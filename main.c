#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input_buffer.h"
#include "utils/io_helpers.c"

void print_prompt() {
  printf("mdb > ");
}

int main(int argc, char *argv[]) {
  InputBuffer *input_buffer = new_input_buffer(); // Initializing the structure
  Table *table = new_table(); //Initializing the table
  while (true) {
    print_prompt();
    read_input(input_buffer);
    if(is_meta_command(input_buffer)) {
      switch(execute_meta_command(input_buffer)) {
        case (META_COMMAND_SUCCESS):
          continue;
        case (META_COMMAND_UNRECOGNIZED):
          printf("Unrecognized command !!!%s \n", input_buffer->buffer);
          continue;
      }
    }
    Statement statement;
    switch(execute_statement(input_buffer, &statement, table)) {
      case (EXECUTE_SUCCESS):
        printf("Executed.\n");
        break;
      case (EXECUTE_TABLE_FULL):
        printf("Error: Table FULL.\n");
        break;
    }
  }
}