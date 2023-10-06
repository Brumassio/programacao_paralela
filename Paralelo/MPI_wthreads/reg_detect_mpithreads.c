#include <stdio.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <pthread.h>

#include "polybench.h"
#include "reg_detect.h"

#define NUM_THREADS 4

pthread_barrier_t barr;
pthread_mutex_t lock;

typedef struct {
    int niter;
    int maxgrid;
    int length;
    DATA_TYPE (*sum_tang)[MAXGRID];
    DATA_TYPE (*mean)[MAXGRID];
    DATA_TYPE (*path)[MAXGRID];
    DATA_TYPE (*diff)[MAXGRID][LENGTH];
    DATA_TYPE (*sum_diff)[MAXGRID][LENGTH];
} ThreadArgs;

static void init_array(int maxgrid,
                       DATA_TYPE POLYBENCH_2D(sum_tang, MAXGRID, MAXGRID, maxgrid, maxgrid),
                       DATA_TYPE POLYBENCH_2D(mean, MAXGRID, MAXGRID, maxgrid, maxgrid),
                       DATA_TYPE POLYBENCH_2D(path, MAXGRID, MAXGRID, maxgrid, maxgrid))
{
    int i, j;

    for (i = 0; i < maxgrid; i++)
        for (j = 0; j < maxgrid; j++) {
            sum_tang[i][j] = (DATA_TYPE)((i + 1) * (j + 1));
            mean[i][j] = ((DATA_TYPE)i - j) / maxgrid;
            path[i][j] = ((DATA_TYPE)i * (j - 1)) / maxgrid;
        }
}

static void print_array(int maxgrid,
                        DATA_TYPE POLYBENCH_2D(path, MAXGRID, MAXGRID, maxgrid, maxgrid))
{
    int i, j;

    for (i = 0; i < maxgrid; i++)
        for (j = 0; j < maxgrid; j++) {
            fprintf(stderr, DATA_PRINTF_MODIFIER, path[i][j]);
            if ((i * maxgrid + j) % 20 == 0)
                fprintf(stderr, "\n");
        }
    fprintf(stderr, "\n");
}

static void *thread_kernel(void *args) {
    ThreadArgs *thread_args = (ThreadArgs *)args;

    int t, i, j, cnt;
    int length = thread_args->length;

    for (t = 0; t < thread_args->niter; t++) {
        for (j = 0; j < thread_args->maxgrid; j++) {
            for (i = j; i < thread_args->maxgrid; i++) {
                for (cnt = 0; cnt < length; cnt++)
                    thread_args->diff[j][i][cnt] = thread_args->sum_tang[j][i];
            }
        }

        pthread_barrier_wait(&barr);

        for (j = 0; j < thread_args->maxgrid; j++) {
            for (i = j; i < thread_args->maxgrid; i++) {
                thread_args->sum_diff[j][i][0] = thread_args->diff[j][i][0];
                for (cnt = 1; cnt < length; cnt++)
                    thread_args->sum_diff[j][i][cnt] =
                        thread_args->sum_diff[j][i][cnt - 1] + thread_args->diff[j][i][cnt];
                thread_args->mean[j][i] = thread_args->sum_diff[j][i][length - 1];
            }
        }

        pthread_barrier_wait(&barr);

        for (i = 0; i < thread_args->maxgrid; i++)
            thread_args->path[0][i] = thread_args->mean[0][i];

        for (j = 1; j < thread_args->maxgrid; j++) {
            for (i = j; i < thread_args->maxgrid; i++) {
                thread_args->path[j][i] = thread_args->path[j - 1][i - 1] + thread_args->mean[j][i];
            }
        }

        pthread_barrier_wait(&barr);
    }

    return NULL;
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc == 2 && strcmp(argv[1], "-h") == 0 && rank == 0) {
        printf("Como compilar:\n");
        printf("make DATASET_SIZE=*TAMANHO DESEJADO*\n");
        printf("Tamanhos possiveis: -DSMALL_DATASET, -DSTANDARD_DATASET, -DLARGE_DATASET, -DEXTRALARGE_DATASET\n");
        printf("Exemplo de compilacao:\n");
        printf("make DATASET_SIZE=-DEXTRALARGE_DATASET\n");
        printf("Exemplo de execucao\n");
        printf("O nome gerado do executavel e atax_time\n");
        printf("./atax_time\n");
        return 0;
    }
    int niter = NITER;
    int maxgrid = MAXGRID;
    int length = LENGTH;

    int work_per_thread = niter / NUM_THREADS;
    int extra_work = niter % NUM_THREADS;

    POLYBENCH_2D_ARRAY_DECL(sum_tang, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
    POLYBENCH_2D_ARRAY_DECL(mean, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
    POLYBENCH_2D_ARRAY_DECL(path, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
    POLYBENCH_3D_ARRAY_DECL(diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length);
    POLYBENCH_3D_ARRAY_DECL(sum_diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length);

    init_array(maxgrid, POLYBENCH_ARRAY(sum_tang), POLYBENCH_ARRAY(mean), POLYBENCH_ARRAY(path));

    pthread_barrier_init(&barr, NULL, NUM_THREADS);
    pthread_mutex_init(&lock, NULL);

    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];
    polybench_start_instruments;

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].niter = work_per_thread + ((i < extra_work) ? 1 : 0);
        thread_args[i].maxgrid = maxgrid;
        thread_args[i].length = length;
        thread_args[i].sum_tang = POLYBENCH_ARRAY(sum_tang);
        thread_args[i].mean = POLYBENCH_ARRAY(mean);
        thread_args[i].path = POLYBENCH_ARRAY(path);
        thread_args[i].diff = POLYBENCH_ARRAY(diff);
        thread_args[i].sum_diff = POLYBENCH_ARRAY(sum_diff);

        pthread_create(&threads[i], NULL, thread_kernel, (void *)&thread_args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barr);

    pthread_mutex_destroy(&lock);

    MPI_Barrier(MPI_COMM_WORLD);
    polybench_stop_instruments;
        polybench_print_instruments;
    // if (rank == 0) {
    //     print_array(maxgrid, POLYBENCH_ARRAY(path));
    // }

    POLYBENCH_FREE_ARRAY(sum_tang);
    POLYBENCH_FREE_ARRAY(mean);
    POLYBENCH_FREE_ARRAY(path);
    POLYBENCH_FREE_ARRAY(diff);
    POLYBENCH_FREE_ARRAY(sum_diff);

    MPI_Finalize();

    return 0;
}
