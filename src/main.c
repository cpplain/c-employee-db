#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

void print_usage(char *argv[]) {
    printf("Usage: %s [-n] -f <database_file>\n", argv[0]);
    printf("    -n    create new database file\n");
    printf("    -f    path to database file (required)\n");
    return;
}

int main(int argc, char *argv[]) {
    int ch;
    bool newfile = false;
    char *filepath = NULL;

    while ((ch = getopt(argc, argv, "nf:")) != -1) {
        switch (ch) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case '?':
            default:
                printf("Unknown option\n");
                print_usage(argv);
                return 0;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required arguement\n");
        print_usage(argv);
        return 0;
    }

    return 0;
}
