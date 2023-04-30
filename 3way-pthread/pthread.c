#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <pthread.h>

#include "sys/types.h"
#include "sys/sysinfo.h"

#define NUM_THREADS 4

typedef struct {
    uint32_t virtualMem;
    uint32_t physicalMem;
} processMem_t;

typedef struct {
    int start;
    int end;
    int *results;
    char **lines;
} thread_data_t;

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

void *findMaxValueThread(void *thread_data) {
    thread_data_t *data = (thread_data_t *)thread_data;
    int start = data->start;
    int end = data->end;
    int i, nchars;

    for (i = start; i < end; i++) {
        char line[2001];
        strncpy(line, data->lines[i], 2001);
        nchars = strlen(line);
        data->results[i] = findMaxValue(line, nchars);
    }

    pthread_exit(NULL);
}

int main() {
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
    
     //GetProcessMemory(&afterRead);
    char *slurm_ntasks_env = getenv("SLURM_NTASKS");
    int num_threads = NUM_THREADS;
    
    if (slurm_ntasks_env) {
        num_threads = atoi(slurm_ntasks_env);
    }
    
    int lines_per_thread = nlines / num_threads;
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];
    int rc;
    long t;

    for (t = 0; t < num_threads; t++) {
        thread_data[t].start = t * lines_per_thread;
        thread_data[t].end = (t == num_threads - 1) ? nlines : (t + 1) * lines_per_thread;
        thread_data[t].results = results;
        thread_data[t].lines = lines;
        rc = pthread_create(&threads[t], NULL, findMaxValueThread, (void *)&thread_data[t]);
        if (rc) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
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
	printf("DATATIME(ms), %d, %s, %f\n", myVersion, getenv("SLURM_NTASKS"),  elapsedTime);
	//printf("DATAREADMEM(vMemKB)(pMemKB), %u, %u\n", afterRead.virtualMem, afterRead.physicalMem);
	printf("DATACOMPMEM(vMemKB)(pMemKB), %s, %u, %u\n", getenv("SLURM_NTASKS"), afterComp.virtualMem, afterComp.physicalMem);
    
    return 0;
}



