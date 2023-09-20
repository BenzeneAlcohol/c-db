#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

struct InputBuffer_t {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
};

typedef struct InputBuffer_t InputBuffer;


// Defining all the function headers here that are to be exported to other files.

void read_input(InputBuffer* input_buffer);
InputBuffer* new_input_buffer();
void print_prompt();
void free_input_buffer(InputBuffer *input_buffer);
bool is_meta_command(InputBuffer* input_buffer);
bool execute_meta_command(InputBuffer* input_buffer);

#endif