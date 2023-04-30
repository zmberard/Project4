#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>

#include "sys/types.h"
#include "sys/sysinfo.h"

typedef struct {
    uint32_t virtualMem;
    uint32_t physicalMem;
} processMem_t;

int parseLine(char *line) {
    int i = strlen(line);
    const char *p = line;
    while (*p < '0' || *p > '9') p++;
    line[i - 3] = '\0';
    i = atoi(p);
    return i;
}

void GetProcessMemory(processMem_t* processMem) {
    FILE *file = fopen("/proc/self/status", "r");
    char line[128];

    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmSize:", 7) == 0) {
            processMem->virtualMem = parseLine(line);
        }

        if (strncmp(line, "VmRSS:", 6) == 0) {
            processMem->physicalMem = parseLine(line);
        }
    }
    fclose(file);
}

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
    //Analysis variables
    
    // Time
    struct timeval t1, t2;
    double elapsedTime;
    int myVersion = 1;
    
    // Memory
    processMem_t afterRead;
    processMem_t afterComp;
    

    
    //Program variables
    const int maxlines = 1000000;
    const int chunk_size = 1000;
    int nlines = 0;
    int i, nchars;
    FILE *fd;
    int *results = (int*)malloc(maxlines * sizeof(int));
    
    //Analysis setup
    gettimeofday(&t1, NULL);
    
    //Program start
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
    
     GetProcessMemory(&afterRead);

    #pragma omp parallel for private(i, nchars) schedule(static)
    for (i = 0; i < nlines; i++) {
        char line[2001];
        strncpy(line, lines[i], 2001);
        nchars = strlen(line);
        results[i] = findMaxValue(line, nchars);
    }
    
    GetProcessMemory(&afterComp);

    for (i = 0; i < nlines; i++) {
        printf("%d: %d\n", i, results[i]);
    }

    // Free memory
    for (i = 0; i < maxlines; i++) {
        free(lines[i]);
    }
    free(lines);
    free(results);
    //End program, start analysis again
    
    gettimeofday(&t2, NULL);
    
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
	printf("DATA, %d, %s, %f\n", myVersion, getenv("SLURM_NTASKS"),  elapsedTime);
	printf("After Read Memory: vMem %u KB, pMem %u KB\n", afterRead.virtualMem, afterRead.physicalMem);
	printf("After Computation Memory: vMem %u KB, pMem %u KB\n", afterComp.virtualMem, afterComp.physicalMem);
    
    return 0;
}



