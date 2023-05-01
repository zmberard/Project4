#include <mpi.h>
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

int main(int argc, char* argv[]) {
    // MPI initialization
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    //Analysis variables
    
    // Time
    struct timeval t1, t2;
    double elapsedTime;
    int myVersion = 1;
    
    // Memory
    //processMem_t afterRead; //Not sure if I want to keep, peak is after comp
    processMem_t afterComp;
    

    
    //Program variables
    const int maxlines = 1000000;
    //const int chunk_size = 1000;
    int nlines = 0;
    int i, nchars;
    FILE *fd;
    int *results = (int*)malloc(maxlines * sizeof(int));
    
    //Analysis setup
    gettimeofday(&t1, NULL);
    
    char **lines = NULL;
    // Program start
    if (rank == 0) {
        fd = fopen("/homes/dan/625/wiki_dump.txt", "r");

        // Read the entire file into memory
        lines = (char**)malloc(maxlines * sizeof(char*));
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
    }
    
     //GetProcessMemory(&afterRead);
     
    // Broadcast the number of lines to all processes
    MPI_Bcast(&nlines, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast the contents of lines to all processes
    if (rank == 0) {
        for (int i = 0; i < nlines; i++) {
            int line_length = strlen(lines[i]) + 1;
            MPI_Bcast(&line_length, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(lines[i], line_length, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
    } else {
        for (int i = 0; i < nlines; i++) {
            int line_length;
            MPI_Bcast(&line_length, 1, MPI_INT, 0, MPI_COMM_WORLD);
            lines[i] = (char *)malloc(line_length * sizeof(char));
            MPI_Bcast(lines[i], line_length, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
    }
    // Calculate the workload distribution
    int local_nlines = nlines / size;
    int start_line = rank * local_nlines;
    int end_line = (rank == size - 1) ? nlines : (rank + 1) * local_nlines;

    // Process the lines
    for (i = start_line; i < end_line; i++) {
        char line[2001];
        strncpy(line, lines[i], 2001);
        nchars = strlen(line);
        results[i] = findMaxValue(line, nchars);
    }
    
    // Print the results only on the master process
    if (rank == 0) {
        GetProcessMemory(&afterComp);
        for (i = 0; i < nlines; i++) {
            printf("%d: %d\n", i, results[i]);
        }
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
	printf("DATATIME(ms), %d, %s, %s, %f\n", myVersion, getenv("SLURM_NTASKS"), getenv("SLURM_NNODES"), elapsedTime);
	//printf("DATAREADMEM(vMemKB)(pMemKB), %u, %u\n", afterRead.virtualMem, afterRead.physicalMem);
	printf("DATACOMPMEM(vMemKB)(pMemKB), %s, %s, %u, %u\n", getenv("SLURM_NTASKS"), getenv("SLURM_NNODES"), afterComp.virtualMem, afterComp.physicalMem);
	
	MPI_Finalize();
    
    return 0;
}


