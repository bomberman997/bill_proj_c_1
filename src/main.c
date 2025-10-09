#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database_file_path>\n", argv[0]);
    printf("\t -n - create new database file\n");
    printf("\t -f - (required) path to database file\n");
    printf("\t -a - add a new employee. Usage: -a \"<name>,<address>,<hours>\"\n");
    printf("\t -r - remove an employee by name. Usage: -r \"<name>\"\n");
    printf("\t -u - remove an employee by name. Usage: -u \"<name>\" -h <hours>\n");
    printf("\t -h - used with -u to update hours\n");

    return;
}

int main(int argc, char *argv[]) { 
    char *filepath = NULL;
    char *addstring = NULL;
    bool newfile = false;
    bool list = false; 
    char *name_to_remove = NULL;
    char *update = NULL; 
    char *hours = NULL;
    int c;

    int dbfd = -1;
    struct dbheader_t *header = NULL;
    struct employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:lr:u:h:")) != -1) {
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg; 
                break;
            case 'a':
                addstring = optarg;
                break;
            case 'l':
                list = true;
                break;
            case 'r':
                name_to_remove = optarg;
                break;
            case 'u':
                update = optarg;
                break;
            case 'h':
                hours = optarg;
                break;
            case '?':
                printf("Unkown option -%c\n", c);
                break;
            default:
                return -1;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);

        return 0;
    }

    if (newfile) {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return -1;
        }

        if (create_db_header(dbfd, &header) == STATUS_ERROR) {
            printf("Failed to create databse header\n");
            return -1;
        }
    } else {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return -1;
        }

        if (validate_db_header(dbfd, &header) == STATUS_ERROR) {
            printf("Failed to validate database header\n");
            return -1;
        }
    }

    printf("Newfile: %d\n", newfile);
    printf("Filepath: %s\n", filepath);

    if (read_employees(dbfd, header, &employees) == STATUS_ERROR) {
        printf("Failed to read employees\n");
        return -1;
    }

    if (addstring) {
        header->count++;
        employees = realloc(employees, header->count*(sizeof(struct employee_t)));

        add_employee(header, employees, addstring);
    }

    if (name_to_remove) {
        remove_employee(header, employees, name_to_remove);
    }

    if (update) {
        if (hours == NULL) {
            printf("-h option is required when -u option is provided");
            return -1;
        }

        update_employee_hours(header, employees, update, atoi(hours)); 
    }

    if (list) {
        list_employees(header, employees);
    }

    output_file(dbfd, header, employees);

    return 0;
}
