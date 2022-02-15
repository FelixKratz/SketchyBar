#include "mach.h"
#include <mach/message.h>
#include <stdint.h>

bool mach_send_message(char* message, uint32_t len) {
  if (!message || len < 1) return false;
  mach_port_name_t task = mach_task_self();

  mach_port_t bs_port;
  if (task_get_special_port(task,
                            TASK_BOOTSTRAP_PORT,
                            &bs_port            ) != KERN_SUCCESS) {
    return NULL;
  }

  mach_port_t port;
  if (bootstrap_look_up(bs_port,
                        "git.felix.sketchybar",
                        &port                  ) != KERN_SUCCESS) {
    return NULL;
  }

  struct mach_message msg = { 0 };
  msg.header.msgh_remote_port = port;

  msg.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND,
                                            0,
                                            0,
                                            MACH_MSGH_BITS_COMPLEX  );
  msg.header.msgh_size = sizeof(struct mach_message);

  msg.msgh_descriptor_count = 1;
  msg.descriptor.address = message;
  msg.descriptor.size = len * sizeof(char);
  msg.descriptor.copy = MACH_MSG_VIRTUAL_COPY;
  msg.descriptor.deallocate = true;
  msg.descriptor.type = MACH_MSG_OOL_DESCRIPTOR;

  mach_msg_return_t msg_return = mach_msg(&msg.header, MACH_SEND_MSG,
                                          sizeof(struct mach_message),
                                          0,
                                          MACH_PORT_NULL,
                                          MACH_MSG_TIMEOUT_NONE,
                                          MACH_PORT_NULL              );

  return msg_return == MACH_MSG_SUCCESS;
}

char* mach_receive_message(struct mach_server* mach_server) {
  mach_server->buffer = (struct mach_buffer) { 0 };
  mach_msg_return_t msg_return = mach_msg(&mach_server->buffer.message.header,
                                          MACH_RCV_MSG,
                                          0,
                                          sizeof(struct mach_buffer),
                                          mach_server->port,
                                          MACH_MSG_TIMEOUT_NONE,
                                          MACH_PORT_NULL             );
  if (msg_return != MACH_MSG_SUCCESS) {
    return NULL;
  }

  return mach_server->buffer.message.descriptor.address;
}

static void* mach_connection_handler(void *context) {
    struct mach_server* server = context;
    while (server->is_running) {
        server->handler(mach_receive_message(server));
    }

  return NULL;
}

bool mach_server_begin(struct mach_server* mach_server, mach_handler handler) {
  mach_server->task = mach_task_self();

  if (mach_port_allocate(mach_server->task,
                         MACH_PORT_RIGHT_RECEIVE,
                         &mach_server->port      ) != KERN_SUCCESS) {
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

  if (bootstrap_register(mach_server->bs_port,
                         "git.felix.sketchybar",
                          mach_server->port     ) != KERN_SUCCESS) {
    return false;
  }

  mach_server->handler = handler;
  mach_server->is_running = true;
  pthread_create(&mach_server->thread, NULL, &mach_connection_handler, mach_server);

  return true;
}
