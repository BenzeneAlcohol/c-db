#include "../input_buffer.h"

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED
} MetaCommandResult;

bool is_meta_command(InputBuffer* input_buffer) {
  return (input_buffer->buffer[0] == '.');
}

bool execute_meta_command(InputBuffer* input_buffer) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    exit(EXIT_SUCCESS);
    free_input_buffer(input_buffer);
  } else {
    return META_COMMAND_UNRECOGNIZED;
  }
}