#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Helper method used to take characters within a line and loop through checking for the max value
int findMaxValue(char* line, int nchars) {
    int i;
    int maxVal = 0;

    #pragma omp parallel for    \
    private(i)                  \
    reduction(max:maxVal)
    for (i = 0; i < nchars; i++) {
        
        int asciiVal = (int)line[i]; // need to cast char to int to get ascii value
        //compare and keep max
        if (asciiVal > maxVal) {
            maxVal = asciiVal; // update if a higher val is found
        }
    }

    return maxVal;
}

int main() {
    //omp_set_num_threads(NUM_THREADS);

    int nlines = 0, maxlines = 100;
    int i, err;
    float charsum = 0.0;
    int nchars = 0;
    FILE *fd;
    char *line = (char*)malloc(2001); // no lines larger than 2000 chars

    // Read in the lines from the data file

    fd = fopen("/homes/dan/625/wiki_dump.txt", "r");

    #pragma omp parallel for    \
    default(shared)             \
    private(err, i, line)

    for (i = 0; i < maxlines; i++) {
        char local_line[2001];
        err = fscanf(fd, "%[^\n]\n", local_line);
        if (err == EOF) break;
        int nchars = strlen(local_line);
        int max_val = findMaxValue(local_line, nchars);
        
        #pragma omp critical
        {
            // Print the line number and the maximum value
            printf("%d: %d\n", nlines, max_val);
            nlines++;
        }
    }

    fclose(fd);
}