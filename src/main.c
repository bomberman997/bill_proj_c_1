#include <stdio.h#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

static void print_usage(char *argv0) {
  printf("Usage: %s -f <database file> [-n] [-a \"Name,Address,Hours\"] [-l]\n", argv0);
  printf("  -n  Create new database file\n");
  printf("  -f  Path to database file (required)\n");
  printf("  -a  Add one employee: \"Name,Address,Hours\"\n");
  printf("  -l  List employees\n");
}

int main(int argc, char *argv[]) {
  int c;
  char *filepath   = NULL;
  bool newfile     = false;
  bool list        = false;
  char *addstring  = NULL;

  int dbfd = -1;
  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  // getopt: -n -f <path> -a "Name,Addr,Hours" -l
  while ((c = getopt(argc, argv, "nf:a:l")) != -1) {
    switch (c) {
      case 'n': newfile = true; break;
      case 'f': filepath = optarg; break;
      case 'a': addstring = optarg; break;
      case 'l': list = true; break;
      case '?':
      default:
        print_usage(argv[0]);
        return -1;
    }
  }

  if (filepath == NULL) {
    printf("Filepath is a required argument\n");
    print_usage(argv[0]);
    return -1;
  }

  if (newfile) {
    dbfd = create_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to create database file\n");
      return -1;
    }
    // Grader prototype: no fd param
    if (create_db_header(&dbhdr) == STATUS_ERROR) {
      printf("Failed to create database header\n");
      return -1;
    }
  } else {
    dbfd = open_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to open database file\n");
      return -1;
    }
    if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Failed to validate database header\n");
      return -1;
    }
  }

  // Load existing employees from file (count comes from dbhdr)
  if (read_employees(dbfd, dbhdr, &employees) != STATUS_SUCCESS) {
    printf("Failed to read employees\n");
    // Still try to write at least the header back out if newfile
    output_file(dbfd, dbhdr, employees);
    return -1;
  }

  // Optionally add one employee from -a "Name,Address,Hours"
  if (addstring) {
    unsigned short newcount = (unsigned short)(dbhdr->count + 1);
    struct employee_t *tmp = realloc(employees, newcount * sizeof(struct employee_t));
    if (!tmp) {
      perror("realloc");
      free(employees);
      return -1;
    }
    employees = tmp;
    dbhdr->count = newcount;

    if (add_employee(dbhdr, employees, addstring) != STATUS_SUCCESS) {
      printf("Failed to add employee\n");
      free(employees);
      return -1;
    }
  }

  // Optionally list to stdout
  if (list) {
    list_employees(dbhdr, employees);
  }

  // Serialize header + employees back to the file
  if (output_file(dbfd, dbhdr, employees) != STATUS_SUCCESS) {
    printf("Failed to write database\n");
    free(employees);
    return -1;
  }

  free(employees);
  // (If your file.c doesn’t close in helpers, you could close(dbfd) here.)
  return 0;
}

