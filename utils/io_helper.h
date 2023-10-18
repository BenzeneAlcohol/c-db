#ifndef IO_HELPERS_H
#define IO_HELPERS_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../input_buffer.h"

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100 // Maximum Pages in the table

typedef enum { META_COMMAND_SUCCESS,
               META_COMMAND_UNRECOGNIZED } MetaCommandResult;

typedef enum { PREPARE_SUCCESS,
               PREPARE_UNRECOGNIZED_STATEMENT } PrepareResult;

typedef enum { EXECUTE_SUCCESS, 
               EXECUTE_TABLE_FULL } ExecuteResult;

typedef enum { STATEMENT_INSERT,
               STATEMENT_SELECT,
               STATEMENT_NONE } StatementType;

typedef struct {
  __uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
  StatementType type;
  Row row_details;
} Statement;

typedef struct {
  __uint32_t file_descriptor;
  __uint32_t file_length;
  void* pages[TABLE_MAX_PAGES];
} Pager;


typedef struct {
  __uint32_t num_rows;
  Pager* pager;
} Table;

// Table functions
Table* db_connect(const char* filename);
Pager* pager_open(const char* filename);
void free_table(Table* table);
void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);
__uint32_t* row_slot(Table* table, __uint32_t row_num);
void print_row(Row* row);
void* get_page(Pager* pager, uint32_t page_num);
void db_close(Table* table);
void pager_flush(Pager* pager, uint32_t page_num, uint32_t size);

// IO functions
bool is_meta_command(InputBuffer *input_buffer);
MetaCommandResult execute_meta_command(InputBuffer *input_buffer, Table *table);
PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement);
ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement *statement, Table *table);
ExecuteResult execute_statement(InputBuffer *input_buffer, Statement *statement, Table *table);

#endif  // IO_HELPERS_H