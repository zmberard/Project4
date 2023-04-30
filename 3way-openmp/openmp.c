#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int findMaxValue(char* line, int nchars) {
    int i;
    int maxVal = 0;

    for (i = 0; i < nchars; i++) {
        int asciiVal = (int)line[i];
        if (asciiVal > maxVal) {
            maxVal = asciiVal;
        }
    }

    return maxVal;
}

int main() {
    const int maxlines = 1000000;
    const int chunk_size = 1000;
    int nlines = 0;
    int i, nchars;
    FILE *fd;
    int *results = (int*)malloc(maxlines * sizeof(int));

    fd = fopen("/homes/dan/625/wiki_dump.txt", "r");

    // Read the entire file into memory
    char **lines = (char**)malloc(maxlines * sizeof(char*));
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    for (i = 0; i < maxlines; i++) {
        read = getline(&line, &len, fd);
        if (read == -1) {
            break;
        }
        lines[i] = (char *)malloc((read + 1) * sizeof(char));
        strncpy(lines[i], line, read + 1);
        nlines++;
    }
    free(line);
    fclose(fd);

    #pragma omp parallel for private(i, nchars) schedule(static)
    for (i = 0; i < nlines; i++) {
        char line[2001];
        strncpy(line, lines[i], 2001);
        nchars = strlen(line);
        results[i] = findMaxValue(line, nchars);
    }

    for (i = 0; i < nlines; i++) {
        printf("%d: %d\n", i, results[i]);
    }

    // Free memory
    for (i = 0; i < maxlines; i++) {
        free(lines[i]);
    }
    free(lines);
    free(results);
    return 0;
}



