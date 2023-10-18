#include "../input_buffer.h"
#include "io_helper.h"

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute) // It calculates the size of that attribute within the structure 

///////////////////////////////////
// STRUCTURE OF THE TABLE BEGINS //
///////////////////////////////////

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

Table* db_connect(const char* filename) {
  Pager* pager = pager_open(filename);
  uint32_t num_rows = pager->file_length / ROW_SIZE;
  Table* table = (Table*)malloc(sizeof(Table));
  table->pager = pager;
  table->num_rows = num_rows;
  return table;
}

Pager* pager_open(const char* filename){
  int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

  if(fd==-1) {
    printf("UNABLE TO OPEN THE DATABASE FILE");
    exit(EXIT_FAILURE);
  }

  off_t file_length = lseek(fd, 0, SEEK_END);
  Pager* pager = malloc(sizeof(Pager));
  pager->file_descriptor = fd;
  pager->file_length = file_length;

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    pager->pages[i] = NULL;
  }

  return pager;
}

void free_table(Table* table) {
  free(table);
}


///////////////////////////
// TABLE FUNCTIONS BEGIN //
///////////////////////////

// Function to serialize the Row structure so that ALL the elements of the structure align themselves together.

// Serialize function
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

  void* page = get_page(table->pager, page_num);

  __uint32_t row_offset = row_num % ROWS_PER_PAGE;
  __uint32_t byte_offset = row_offset * ROW_SIZE;
  return page + byte_offset;
}

void print_row(Row* row) {
  printf("%d, %s, %s \n", row->id, row->username, row->email);
}

void* get_page(Pager* pager, uint32_t page_num) {
  if(page_num > TABLE_MAX_PAGES) {
    printf("Maximum number of pages reached");
    exit(EXIT_FAILURE);
  }

  if (pager->pages[page_num] == NULL) {
    //This means the page number doesn't exist

    void* page = malloc(PAGE_SIZE);

    uint32_t num_pages = pager->file_length / PAGE_SIZE;

    if (pager->file_length % PAGE_SIZE) {
      num_pages += 1;
    }

    if (page_num <= num_pages) {  
      lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);

      if (bytes_read == -1) {
        printf("Error reading file");
        exit(EXIT_FAILURE);
      }
    }
    pager->pages[page_num] = page;
  }
  return pager->pages[page_num];
}

void db_close(Table* table) {
  Pager* pager = table->pager;
  uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

  for (uint32_t i = 0; i < num_full_pages; i++) {
    if (pager->pages[i] == NULL) {
      continue;
    }

    pager_flush(pager, i, PAGE_SIZE);
    free(pager->pages[i]);
    pager->pages[i] = NULL;
  }

  uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;

  if (num_additional_rows > 0) {
    uint32_t page_num = num_full_pages;
    if (pager->pages[page_num] != NULL) {
      pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
      free(pager->pages[page_num]);
      pager->pages[page_num] = NULL;
    }
  }
  int result = close(pager->file_descriptor);

  if (result == -1) {
    printf("Closing db file \n");
    exit(EXIT_FAILURE);
  }

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    void* page = pager->pages[i];

    if (page) {
      free(page);
      pager->pages[i] = NULL;
    }
  }
  free(pager);
  free(table);
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size) { 
  if (pager->pages[page_num] == NULL) {
    printf("Tried to flush null page\n");
    exit(EXIT_FAILURE);
  }

  off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

  if (offset == -1) {
    printf("Error seeking: ");
    exit(EXIT_FAILURE);
  }

  ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], size);

  if (bytes_written == -1) {
    printf("ERROR");
    exit(EXIT_FAILURE);
  }
}

/////////////////////////
// TABLE FUNCTIONS END //
/////////////////////////


bool is_meta_command(InputBuffer *input_buffer) {
  return (input_buffer->buffer[0] == '.');
}

MetaCommandResult execute_meta_command(InputBuffer *input_buffer, Table* table) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    db_close(table);
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