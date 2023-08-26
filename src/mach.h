#pragma once
#include <bootstrap.h>
#include <mach/mach.h>
#include <mach/message.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define MACH_BS_NAME_FMT "git.felix.%s"

struct mach_message {
  mach_msg_header_t header;
  mach_msg_size_t msgh_descriptor_count;
  mach_msg_ool_descriptor_t descriptor;
};

struct mach_buffer {
  struct mach_message message;
  mach_msg_trailer_t trailer;
};

#define MACH_HANDLER(name) void name(struct mach_buffer* message)
typedef MACH_HANDLER(mach_handler);

struct mach_server {
  bool is_running;
  mach_port_name_t task;
  mach_port_t port;
  mach_port_t bs_port;

  mach_handler* handler;
};

bool mach_server_begin(struct mach_server* mach_server, mach_handler handler);
char* mach_send_message(mach_port_t port, char* message, uint32_t len, bool await_response);
mach_port_t mach_get_bs_port(char* bs_name);
