#include "../input_buffer.h"

typedef enum { META_COMMAND_SUCCESS,
               META_COMMAND_UNRECOGNIZED } MetaCommandResult;

typedef enum { PREPARE_SUCCESS,
               PREPARE_UNRECOGNIZED_STATEMENT } PrepareResult;
typedef enum { STATEMENT_INSERT,
               STATEMENT_SELECT,
               STATEMENT_NONE } StatementType;

typedef struct {
  StatementType type;
} Statement;

bool is_meta_command(InputBuffer *input_buffer) {
  return (input_buffer->buffer[0] == '.');
}

MetaCommandResult execute_meta_command(InputBuffer *input_buffer) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    exit(EXIT_SUCCESS);
    free_input_buffer(input_buffer);
  } else {
    return META_COMMAND_UNRECOGNIZED;
  }
}

PrepareResult prepare_statement(InputBuffer *input_buffer,
                                Statement *statement) {
  if (strncmp(input_buffer->buffer, "INSERT", 6) == 0) {
    printf("Welcome to INSERT statement\n");
    statement->type = STATEMENT_INSERT;
    return PREPARE_SUCCESS;
  }

  else if (strncmp(input_buffer->buffer, "SELECT", 6) == 0) {
    printf("Welcome to SELECT statement\n");
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  else {
    printf("Undefined Statement for now\n");
    statement->type = STATEMENT_NONE;
    return PREPARE_UNRECOGNIZED_STATEMENT;
  }
}

void execute_statement(InputBuffer *input_buffer, Statement *statement) {
  PrepareResult result = prepare_statement(input_buffer, statement);
  switch (statement->type) {
  case (STATEMENT_INSERT):
    printf("This is where we would do an insert.\n");
    break;
  case (STATEMENT_SELECT):
    printf("This is where we would do a select.\n");
    break;
  }
}