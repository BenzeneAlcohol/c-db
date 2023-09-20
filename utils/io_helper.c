#include "../input_buffer.h"

typedef enum { META_COMMAND_SUCCESS,
               META_COMMAND_UNRECOGNIZED } MetaCommandResult;

typedef enum { PREPARE_SUCCESS,
               PREPARE_UNRECOGNIZED_STATEMENT } PrepareResult;
typedef enum { STATEMENT_INSERT,
               STATEMENT_SELECT,
               STATEMENT_NONE } StatementType;

typedef enum { EXECUTE_SUCCESS, 
               EXECUTE_TABLE_FULL } ExecuteResult;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute) // It calculates the size of that attribute within the structure 
#define TABLE_MAX_PAGES 100 // Maximum Pages in the table

///////////////////////////////////
// STRUCTURE OF THE TABLE BEGINS //
///////////////////////////////////

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
  __u_int id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
  StatementType type;
  Row row_details;
} Statement;

typedef struct {
  __uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES]; //Each element in the array can hold the memory address of any type of data (void*)
} Table;

/////////////////////////////////
// STRUCTURE OF THE TABLE ENDS //
/////////////////////////////////

const __uint32_t ID_SIZE = size_of_attribute(Row, id); // Size of the ID attribute according to whatever is the size of the id variable
const __uint32_t USERNAME_SIZE = size_of_attribute(Row, username); // Size of username variable
const __uint32_t EMAIL_SIZE = size_of_attribute(Row, email); // Size of email variable.
const __uint32_t ID_OFFSET = 0;
const __uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const __uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const __uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const __uint32_t PAGE_SIZE = 4096;
const __uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const __uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

///////////////////////////
// TABLE FUNCTIONS BEGIN //
///////////////////////////

// Function to serialize the Row structure so that ALL the elements of the structure align themselves together.

Table* new_table() {
  Table* table = (Table*)malloc(sizeof(Table));
  table->num_rows = 0;
  for (__uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    table->pages[i] = NULL;
  }
  return table;
}

void free_table(Table* table) {
  for (int i = 0; table->pages[i]; i++) {
    free(table->pages[i]);
  }
  free(table);
}

void serialize_row(Row* source, void* destination) {
  memcpy(destination+ID_OFFSET, &(source->id), ID_SIZE); // Copies ID_SIZE bytes from address of (source->id) to destination+ID_OFFSET
  memcpy(destination+USERNAME_OFFSET, &(source->username), USERNAME_SIZE); // Copies USERNAME_SIZE bytes (same as above)
  memcpy(destination+EMAIL_OFFSET, &(source->email), EMAIL_SIZE); // Same as above
}

// Deserialize function
void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

// Exact memory location of where the page exists is created and returned by this function
__uint32_t* row_slot(Table* table, __uint32_t row_num) {
  __uint32_t page_num = row_num / ROWS_PER_PAGE;
  void* page = table->pages[page_num];

  if (page == NULL) {
    page = table->pages[page_num] = malloc(PAGE_SIZE); //When the PAGE we try to access doesn't exist already
  }
  __uint32_t row_offset = row_num % ROWS_PER_PAGE;
  __uint32_t byte_offset = row_offset * ROW_SIZE;
  return page + byte_offset;
}

void print_row(Row* row) {
  printf("%d, %s, %s \n", row->id, row->username, row->email);
}

/////////////////////////
// TABLE FUNCTIONS END //
/////////////////////////


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
    int args_recieved = sscanf(input_buffer->buffer, "INSERT %d %s %s", &(statement->row_details.id),
                               &(statement->row_details.username),
                               &(statement->row_details.email));
    if (args_recieved < 3) {
      return PREPARE_UNRECOGNIZED_STATEMENT;
    }
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

ExecuteResult execute_insert(Statement* statement, Table* table) {
  if (table->num_rows >= TABLE_MAX_ROWS) {
    return EXECUTE_TABLE_FULL;
  }

  Row* row_details = &(statement->row_details);

  serialize_row(row_details, row_slot(table, table->num_rows));
  table->num_rows += 1;

  return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table) {
  Row row;
  for (__uint32_t i = 0; i < table->num_rows; i++) {
    deserialize_row(row_slot(table, i), &row);
    print_row(&row);
  }

  return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(InputBuffer *input_buffer, Statement *statement, Table *table) {
  PrepareResult result = prepare_statement(input_buffer, statement);
  switch (statement->type) {
  case (STATEMENT_INSERT):
    printf("This is where we would do an insert.\n");
    return execute_insert(statement, table);
    break;
  case (STATEMENT_SELECT):
    printf("This is where we would do a select.\n");
    return execute_select(statement, table);
    break;
  }
}