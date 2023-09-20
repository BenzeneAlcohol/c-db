#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct InputBuffer_t {
  char *buffer; // String of your inpur command
  size_t buffer_length;
  ssize_t input_length; // Length of that string  (signed buffer)
};

typedef struct InputBuffer_t InputBuffer; // InputBuffer = struct InputBuffer_t

// lineptr = Pointer to *buffer of input_buffer (which itself is a pointer, hence pointer of pointers)
// size_t is a datatype similar to int, but gaurantees that it can hold the size of the object (in our case, the input length/buffer length)
ssize_t getline(char **lineptr, size_t *n, FILE *stream); // Returns the number of characters read

// Reading the input from the user
void read_input(InputBuffer *input_buffer) {
  ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin); // Number of bytes read is returned (Including newLine)

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  input_buffer->input_length = bytes_read - 1; // Reducing the length of newLine
  input_buffer->buffer[bytes_read - 1] = 0;    // Replacing \n with \0
}

// Returns an InputBuffer* object
// Basic initialization of the object in the memory
InputBuffer *new_input_buffer() {
  InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

void print_prompt() {
  printf("db > ");
}

void free_input_buffer(InputBuffer *input_buffer) {
  free(input_buffer->buffer);
  free(input_buffer);
}

int main(int argc, char *argv[]) {
  InputBuffer *input_buffer = new_input_buffer(); // initializing the structure
  while (true) {
    print_prompt();
    read_input(input_buffer);

    if (strcmp(input_buffer->buffer, ".exit") == 0) {
      free_input_buffer(input_buffer);
      exit(EXIT_SUCCESS);
    } else {
      printf("Unrecognized command '%s'.\n", input_buffer->buffer);
    }
  }
}