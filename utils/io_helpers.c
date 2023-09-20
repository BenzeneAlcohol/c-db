#include "../input_buffer.h"

bool is_metacommand(InputBuffer* input_buffer) {
  return (input_buffer->buffer[0] == '.');
}
