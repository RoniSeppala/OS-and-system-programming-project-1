#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    FILE *inputFile = stdin, *outputFile = stdout;
    char *inputFileName = NULL, *outputFileName = NULL;

    /* 1) Argument parsing */
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }
    if (argc >= 2)
        inputFileName = argv[1];
    if (argc == 3)
        outputFileName = argv[2];

    /* 2) Input==output check */
    if (inputFileName && outputFileName &&
        strcmp(inputFileName, outputFileName) == 0) {
        fprintf(stderr, "Input and output file must differ\n");
        exit(1);
    }

    /* 3) Open files (or use stdin/stdout) */
    if (inputFileName) {
        inputFile = fopen(inputFileName, "r");
        if (!inputFile) {
            fprintf(stderr, "error: cannot open file '%s'\n", inputFileName);
            exit(1);
        }
    }
    if (outputFileName) {
        outputFile = fopen(outputFileName, "w");
        if (!outputFile) {
            fprintf(stderr, "error: cannot open file '%s'\n", outputFileName);
            if (inputFile != stdin) fclose(inputFile);
            exit(1);
        }
    }

    /* 4) Read all lines into a dynamically resizing array */
    size_t lineCapacity = 16, lineCount = 0;
    char **lines = malloc(lineCapacity * sizeof *lines);
    if (!lines) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    char *buffer = NULL;
    size_t bufferSize = 0;
    ssize_t linelength;
    while ((linelength = getline(&buffer, &bufferSize, inputFile)) >= 0) {
        /* Copy the line (including newline) */
        char *copy = malloc((size_t)linelength + 1);
        if (!copy) {
            fprintf(stderr, "malloc failed\n");
            exit(1);
        }
        memcpy(copy, buffer, (size_t)linelength + 1);

        /* Grow the array if needed */
        if (lineCount >= lineCapacity) {
            size_t newcap = lineCapacity * 2;
            char **tmp = realloc(lines, newcap * sizeof *tmp);
            if (!tmp) {
                fprintf(stderr, "malloc failed\n");
                exit(1);
            }
            lines = tmp;
            lineCapacity = newcap;
        }

        lines[lineCount++] = copy;
    }
    free(buffer);
    if (inputFile != stdin) fclose(inputFile);

    /* 5) Write in reverse order */
    for (ssize_t i = (ssize_t)lineCount - 1; i >= 0; --i) {
        char *line = lines[i];
        size_t len  = strlen(line);

        /* Print the line bytes */
        if (fwrite(line, 1, len, outputFile) < len) {
            /* handle write error */
            fprintf(stderr, "error: writing to output file failed\n");
        }

        /* If it didn’t include a trailing '\n', add one */
        if (len == 0 || line[len - 1] != '\n') {
            if (fputc('\n', outputFile) == EOF) {
                /* handle write error */
                fprintf(stderr, "error: writing newline to output file failed\n");
            }
        }

        free(line);
    }

    if (outputFile != stdout) fclose(outputFile);
    return 0;
}
