#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>

double **mat1;
double **mat2;
double **res;

typedef struct {
    int thread_index;
    int start_row;
    int end_row;
    int start_col;
    int end_col;
} ThreadData;

void randvalues(int len);
void *MMparal(void *data);
float time_diff(struct timeval *start, struct timeval *end);

int main(int argc, char *argv[]) {
    struct timeval start;
    struct timeval end;
    int len = atoi(argv[1]);
    int flow = atoi(argv[2]);

    pthread_t threads[flow];
    ThreadData threadData[flow];

    mat1 = malloc(len * sizeof(double *));
    mat2 = malloc(len * sizeof(double *));
    res = malloc(len * sizeof(double *));

    for (int i = 0; i < len; i++) {
        mat1[i] = malloc(len * sizeof(double));
        mat2[i] = malloc(len * sizeof(double));
        res[i] = malloc(len * sizeof(double));
    }

    randvalues(len);
    gettimeofday(&start, NULL); 

    int block_size = len / flow;
    int remaining_rows = len % flow;
    int start_row = 0;
    int end_row = block_size - 1;

    for (int i = 0; i < flow; i++) {
        if (remaining_rows > 0) {
            end_row++;
            remaining_rows--;
        }
        int start_col = 0;
        int end_col = len - 1;

        threadData[i].thread_index = i;
        threadData[i].start_row = start_row;
        threadData[i].end_row = end_row;
        threadData[i].start_col = start_col;
        threadData[i].end_col = end_col;

        pthread_create(&threads[i], NULL, MMparal, (void *)&threadData[i]);

        start_row = end_row;
        end_row += block_size;
    }

    for (int i = 0; i < flow; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);
    printf("MMparalelo with len %d time spent: %0.8f sec\n",len, time_diff(&start, &end));


    
    return 0;
}

void *MMparal(void *data) {
    ThreadData *threadData = (ThreadData *)data;
    int thread_index = threadData->thread_index;
    int start_row = threadData->start_row;
    int end_row = threadData->end_row;
    int start_col = threadData->start_col;
    int end_col = threadData->end_col;
    int len = end_row - start_row + 1; 

    for (int i = start_row; i <= end_row; i++) {
        for (int j = start_col; j <= end_col; j++) {
            res[i][j] = 0;
            for (int k = 0; k < len; k++) {
                res[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }

    pthread_exit(NULL); 
}

void randvalues(int len) {
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            mat1[i][j] = rand() % 100;
            mat2[i][j] = rand() % 100;
        }
    }
}

float time_diff(struct timeval *start, struct timeval *end){
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}