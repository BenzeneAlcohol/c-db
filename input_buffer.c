#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input_buffer.h"


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

void free_input_buffer(InputBuffer *input_buffer) {
  free(input_buffer->buffer);
  free(input_buffer);
}
