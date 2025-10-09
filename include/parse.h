#include <stdint.h>

#define HEADER_MAGIC 0x4c4c4144
#define NAME_LEN 256
#define ADDRESS_LEN 256

struct dbheader_t {
    uint32_t magic;
    uint16_t version;
    uint16_t count;
    uint32_t filesize;
};

struct employee_t {
    char name[NAME_LEN];
    char address[ADDRESS_LEN];
    uint32_t hours;
};

// Function interfaces (as given)
int create_db_header(struct dbheader_t **headerOut);
int validate_db_header(int fd, struct dbheader_t **headerOut);
int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut);
int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees);

// Likely needed elsewhere in your project/tests
int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring);
int list_employees(struct dbheader_t *dbhdr, struct employee_t *employees);

