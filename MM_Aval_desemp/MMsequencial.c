#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>


double **mat1;
double **mat2;
double **res;



void randvalues(int len);
void MMseq(int len);
float time_diff(struct timeval *start, struct timeval *end);

int main(int argc, char *argv[]){
    struct timeval start;
    struct timeval end;
    int len = atoi(argv[1]);

    mat1 = malloc(len * sizeof(double));
    mat2 = malloc(len * sizeof(double));
    res = malloc(len * sizeof(double));

    for(int i =0; i<len;i++){
        mat1[i] = malloc (len * sizeof(double));
        mat2[i] = malloc (len * sizeof(double));
        res[i] = malloc(len * sizeof(double));
    }

    randvalues(len);
    gettimeofday(&start, NULL); 
    MMseq(len);
    gettimeofday(&end, NULL); 

    printf("MMseq with len %d time spent: %0.8f sec\n",len, time_diff(&start, &end));

    return 0;
}

void MMseq(int len){
    for(int i=0;i<len;i++){
        for(int j=0;j<len;j++){
            res[i][j] = 0;
            for(int k=0; k<len; k++){
                res[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
}

void randvalues(int len){
    for(int i=0;i<len;i++){
        for(int j=0;j<len;j++){
            mat1[i][j] = rand() % 100;
            mat2[i][j] = rand() % 100;
        }
    }
}


float time_diff(struct timeval *start, struct timeval *end){
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}