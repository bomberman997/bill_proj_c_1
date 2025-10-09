#include <stdio.h>

#define HEADER_MAGIC 0x4c4c4144

struct dbheader_t {
    unsigned int   magic;
    unsigned short version;
    unsigned short count;
    unsigned int   filesize;
};

int main() {
    printf("sizeof(struct dbheader_t) = %zu\n", sizeof(struct dbheader_t));
    printf("sizeof(unsigned int) = %zu\n", sizeof(unsigned int));
    printf("sizeof(unsigned short) = %zu\n", sizeof(unsigned short));
    return 0;
}
