#include "mach.h"
#include <mach/mach_port.h>
#include <mach/message.h>
#include <stdint.h>
#include <CoreFoundation/CoreFoundation.h>

mach_port_t mach_get_bs_port(char* bs_name) {
  mach_port_name_t task = mach_task_self();

  mach_port_t bs_port;
  if (task_get_special_port(task,
                            TASK_BOOTSTRAP_PORT,
                            &bs_port            ) != KERN_SUCCESS) {
    return 0;
  }

  mach_port_t port;
  if (bootstrap_look_up(bs_port,
                        bs_name,
                        &port   ) != KERN_SUCCESS) {
    return 0;
  }

  return port;
}

void mach_receive_message(mach_port_t port, struct mach_buffer* buffer, bool timeout) {
  *buffer = (struct mach_buffer) { 0 };
  mach_msg_return_t msg_return;
  if (timeout)
    msg_return = mach_msg(&buffer->message.header,
                          MACH_RCV_MSG | MACH_RCV_TIMEOUT,
                          0,
                          sizeof(struct mach_buffer),
                          port,
                          100,
                          MACH_PORT_NULL                  );
  else 
    msg_return = mach_msg(&buffer->message.header,
                          MACH_RCV_MSG,
                          0,
                          sizeof(struct mach_buffer),
                          port,
                          MACH_MSG_TIMEOUT_NONE,
                          MACH_PORT_NULL            );

  if (msg_return != MACH_MSG_SUCCESS) {
    buffer->message.descriptor.address = NULL;
  }
}

char* mach_send_message(mach_port_t port, char* message, uint32_t len, bool await_response) {
  if (!message || !port) return NULL;

  mach_port_t response_port;
    mach_port_name_t task = mach_task_self();
  if (await_response) {
    if (mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE,
                                 &response_port          ) != KERN_SUCCESS) {
      return NULL;
    }

    if (mach_port_insert_right(task, response_port,
                                     response_port,
                                     MACH_MSG_TYPE_MAKE_SEND)!= KERN_SUCCESS) {
      return NULL;
    }
  }

  struct mach_message msg = { 0 };
  msg.header.msgh_remote_port = port;
  if (await_response) {
    msg.header.msgh_local_port = response_port;
    msg.header.msgh_id = response_port;
    msg.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND,
                                              MACH_MSG_TYPE_MAKE_SEND,
                                              0,
                                              MACH_MSGH_BITS_COMPLEX  );
  } else {
    msg.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND
                                              & MACH_MSGH_BITS_REMOTE_MASK,
                                              0,
                                              0,
                                              MACH_MSGH_BITS_COMPLEX       );
  }

  msg.header.msgh_size = sizeof(struct mach_message);

  msg.msgh_descriptor_count = 1;
  msg.descriptor.address = message;
  msg.descriptor.size = len * sizeof(char);
  msg.descriptor.copy = MACH_MSG_VIRTUAL_COPY;
  msg.descriptor.deallocate = false;
  msg.descriptor.type = MACH_MSG_OOL_DESCRIPTOR;

  mach_msg(&msg.header,
           MACH_SEND_MSG,
           sizeof(struct mach_message),
           0,
           MACH_PORT_NULL,
           MACH_MSG_TIMEOUT_NONE,
           MACH_PORT_NULL             );

  if (await_response) {
    struct mach_buffer buffer = { 0 };
    mach_receive_message(response_port, &buffer, true);
    char* rsp = NULL;
    if (buffer.message.descriptor.address) {
      rsp = malloc(strlen(buffer.message.descriptor.address) + 1);
      memcpy(rsp, buffer.message.descriptor.address,
                  strlen(buffer.message.descriptor.address) + 1);
    } else {
      rsp = malloc(1);
      *rsp = '\0';
    }

    mach_msg_destroy(&buffer.message.header);
    mach_port_mod_refs(task, response_port, MACH_PORT_RIGHT_RECEIVE, -1);
    mach_port_deallocate(task, response_port);

    return rsp;
  }

  return NULL;
}

void mach_message_callback(CFMachPortRef port, void* message, CFIndex size, void* context) {
  struct mach_server* mach_server = context;
  struct mach_buffer buffer;
  buffer.message = *(struct mach_message*)message;
  mach_server->handler(&buffer);
  mach_msg_destroy(&buffer.message.header);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
extern char g_name[256];
bool mach_server_begin(struct mach_server* mach_server, mach_handler handler) {
  mach_server->task = mach_task_self();

  if (mach_port_allocate(mach_server->task,
                         MACH_PORT_RIGHT_RECEIVE,
                         &mach_server->port      ) != KERN_SUCCESS) {
    return false;
  }

  struct mach_port_limits limits = {};
  limits.mpl_qlimit = MACH_PORT_QLIMIT_LARGE;

  if (mach_port_set_attributes(mach_server->task,
                               mach_server->port,
                               MACH_PORT_LIMITS_INFO,
                               (mach_port_info_t)&limits,
                               MACH_PORT_LIMITS_INFO_COUNT) != KERN_SUCCESS) {
    return false;
  }

  if (mach_port_insert_right(mach_server->task,
                             mach_server->port,
                             mach_server->port,
                             MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS) {
    return false;
  }

  if (task_get_special_port(mach_server->task,
                            TASK_BOOTSTRAP_PORT,
                            &mach_server->bs_port) != KERN_SUCCESS) {
    return false;
  }

  char bs_name[256];
  snprintf(bs_name, 256, MACH_BS_NAME_FMT, g_name);

  if (bootstrap_register(mach_server->bs_port,
                         bs_name,
                         mach_server->port    ) != KERN_SUCCESS) {
    return false;
  }

  mach_server->handler = handler;
  mach_server->is_running = true;

  CFMachPortContext context = {0, (void*)mach_server};

  CFMachPortRef cf_mach_port = CFMachPortCreateWithPort(NULL,
                                                        mach_server->port,
                                                        mach_message_callback,
                                                        &context,
                                                        false                );

  CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(NULL,
                                                            cf_mach_port,
                                                            0            );

  CFRunLoopAddSource(CFRunLoopGetMain(), source, kCFRunLoopDefaultMode);
  CFRelease(source);
  CFRelease(cf_mach_port);
  return true;
}
#pragma clang diagnostic pop
