#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int create_db_header(struct dbheader_t **headerOut)
{
    // allocate the header
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));

    if (header == NULL)
    {
        printf("calloc failed to allocate memory for a dbheader_t.\n");
        return STATUS_ERROR;
    }

    // compose the header
    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    // override the header output
    *headerOut = header;

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut)
{
    // check the file descriptor
    if (fd < 0)
    {
        printf("Invalid file desriptor.\n");
        return STATUS_ERROR;
    }

    // allocate the header
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL)
    {
        printf("calloc failed to allocate memory for a dbheader_t.\n");
        return STATUS_ERROR;
    }

    // check the header compatability
    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t))
    {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    // unpack the database header
    // (converts netowrk byte order to host byte order see: man ntohs or man ntohl)
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    // validate the header data
    if (header->version != 1)
    {
        printf("header version is incompatible.\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->magic != HEADER_MAGIC)
    {
        printf("header magic number is incompatible.\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size)
    {
        printf("corrupted database.\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *header, struct employee_t *employees)
{
    // check the file descriptor
    if (fd < 0)
    {
        printf("Invalid file desriptor.\n");
        return STATUS_ERROR;
    }

    int realCount = header->count;

    header->version = htons(header->version);
    header->count = htons(header->count);
    header->magic = htonl(header->magic);
    header->filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realCount));

    lseek(fd, 0, SEEK_SET);
    write(fd, header, sizeof(struct dbheader_t));

    int i = 0;
    for (; i < realCount; i++)
    {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *header, struct employee_t **employeesOut)
{
    // check the file descriptor
    if (fd < 0)
    {
        printf("Invalid file desriptor.\n");
        return STATUS_ERROR;
    }

    int count = header->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL)
    {
        printf("calloc failed to allocate memory for an employee_t.\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    int i = 0;
    for (i = 0; i < count; i++)
    {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;

    return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *header, struct employee_t **employees, char *addstring)
{
    if (NULL == header)
        return STATUS_ERROR;
    if (NULL == employees)
        return STATUS_ERROR;
    if (NULL == *employees)
        return STATUS_ERROR;
    if (NULL == addstring)
        return STATUS_ERROR;

    char *name = strtok(addstring, ",");
    if (NULL == name)
        return STATUS_ERROR;

    char *address = strtok(NULL, ",");
    if (NULL == address)
        return STATUS_ERROR;

    char *hours = strtok(NULL, ",");
    if (NULL == hours)
        return STATUS_ERROR;

    struct employee_t *e = *employees;
    e = realloc(e, sizeof(struct employee_t) * (header->count + 1));
    if (e == NULL)
    {
        printf("Failed to reallocate employees.\n");
        return STATUS_ERROR;
    }

    header->count++;

    strncpy(e[header->count - 1].name, name, sizeof(e[header->count - 1].name) - 1);
    strncpy(e[header->count - 1].address, address, sizeof(e[header->count - 1].address) - 1);
    e[header->count - 1].hours = atoi(hours);

    *employees = e;

    return STATUS_SUCCESS;
}

int list_employees(struct dbheader_t *header, struct employee_t *employees)
{
    if (NULL == header)
        return STATUS_ERROR;
    if (NULL == employees)
        return STATUS_ERROR;

    int i = 0;
    for (i = 0; i < header->count - 1; i++)
    {
        printf("Employee: %d\n", i);
        printf("\tName: %s\n", employees[i].name);
        printf("\tAddress: %s\n", employees[i].address);
        printf("\tHours: %d\n", employees[i].hours);
    }

    return STATUS_SUCCESS;
}