#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]);

int main(int argc, char *argv[])
{

    char *filepath = NULL;
    char *addstring = NULL;

    bool newfile = false;
    bool list = false;
    int c;

    int dbfd = -1;
    struct dbheader_t *dbheader = NULL;
    struct employee_t *employees = NULL;

    // argument parsing loop
    while ((c = getopt(argc, argv, "nf:a:l")) != -1)
    {
        switch (c)
        {
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
        case '?':
            printf("Unknown option -%c\n", c);
            break;
        default:
            return -1;
        }
    }

    // check filepath argument
    if (filepath == NULL)
    {
        printf("Filepath os a required argument.\n");
        print_usage(argv);
        return 0;
    }

    // check newfile argument
    if (newfile)
    {
        // create a database file
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR)
        {
            printf("Unable to create database file.\n");
            return -1;
        }
        // create a database header
        if (create_db_header(&dbheader) == STATUS_ERROR)
        {
            printf("Failed to create database header.\n");
            return -1;
        }
    }
    else
    {
        // open a database file
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR)
        {
            printf("Unable to open databse file.\n");
            return -1;
        }

        // validate the database header
        if (validate_db_header(dbfd, &dbheader) == STATUS_ERROR)
        {
            printf("Failed to validate database header.\n");
            return -1;
        }
    }

    // read database entries
    if (read_employees(dbfd, dbheader, &employees) == STATUS_ERROR)
    {
        printf("Failed to read employee data from the database.\n");
        return 0;
    }

    // handle addstring input
    if (addstring)
    {
        // add a new employee
        if (add_employee(dbheader, &employees, addstring) == STATUS_ERROR)
        {
            printf("Failed to add new employee.\n");
            return 0;
        }
    }

    // list the database records
    if (list)
    {
        list_employees(dbheader, employees);
    }

    // write the header to the database file
    output_file(dbfd, dbheader, employees);

    return 0;
}

void print_usage(char *argv[])
{
    printf("Usage: %s -n -f <database file> -a \"<name>,<address>,<hours>\"\n", argv[0]);
    printf("\t-n  - create new database file\n");
    printf("\t-f  - (required) path to database file\n");
    printf("\t-a  - add employee record\n");
    return;
}