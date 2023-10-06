#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

/* Include polybench common header. */
#include "polybench.h"

/* Include benchmark-specific header. */
/* Default data type is int, default size is 50. */
#include "reg_detect.h"

/* Number of threads */
#define NUM_THREADS 6

/* Array initialization. */
static
void init_array(int maxgrid,
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

/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int maxgrid,
                 DATA_TYPE POLYBENCH_2D(path, MAXGRID, MAXGRID, maxgrid, maxgrid))
{
    int i, j;

    for (i = 0; i < maxgrid; i++)
        for (j = 0; j < maxgrid; j++) {
            fprintf(stderr, DATA_PRINTF_MODIFIER, path[i][j]);
            if ((i * maxgrid + j) % 20 == 0) fprintf(stderr, "\n");
        }
    fprintf(stderr, "\n");
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
/* Source (modified): http://www.cs.uic.edu/~iluican/reg_detect.c */
static
void kernel_reg_detect(int niter, int maxgrid, int length,
                       DATA_TYPE POLYBENCH_2D(sum_tang, MAXGRID, MAXGRID, maxgrid, maxgrid),
                       DATA_TYPE POLYBENCH_2D(mean, MAXGRID, MAXGRID, maxgrid, maxgrid),
                       DATA_TYPE POLYBENCH_2D(path, MAXGRID, MAXGRID, maxgrid, maxgrid),
                       DATA_TYPE POLYBENCH_3D(diff, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length),
                       DATA_TYPE POLYBENCH_3D(sum_diff, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length),
                       pthread_barrier_t *barrier)
{
    int t, i, j, cnt;

#pragma scop

    // Estrutura para passar argumentos para as threads
    struct ThreadArgs {
        int maxgrid;
        DATA_TYPE (*sum_tang)[MAXGRID];
        DATA_TYPE (*mean)[MAXGRID];
        DATA_TYPE (*path)[MAXGRID];
        DATA_TYPE (*diff)[MAXGRID][LENGTH];
        DATA_TYPE (*sum_diff)[MAXGRID][LENGTH];
        int inicio;
        int fim;
        pthread_barrier_t *barrier;
    };

    // Função executada por cada thread
    void *thread_function(void *arg) {
        struct ThreadArgs *args = (struct ThreadArgs *)arg;
        int maxgrid = args->maxgrid;
        DATA_TYPE (*sum_tang)[MAXGRID] = args->sum_tang;
        DATA_TYPE (*mean)[MAXGRID] = args->mean;
        DATA_TYPE (*path)[MAXGRID] = args->path;
        DATA_TYPE (*diff)[MAXGRID][LENGTH] = args->diff;
        DATA_TYPE (*sum_diff)[MAXGRID][LENGTH] = args->sum_diff;
        int inicio = args->inicio;
        int fim = args->fim;
        pthread_barrier_t *barrier = args->barrier;

        int t, i, j, cnt;

        for (t = 0; t < _PB_NITER; t++) {
            for (j = inicio; j <= fim - 1; j++) {
                for (i = j; i <= fim - 1; i++) {
                    for (cnt = 0; cnt <= _PB_LENGTH - 1; cnt++)
                        diff[j][i][cnt] = sum_tang[j][i];
                }
            }

            for (j = inicio; j <= fim - 1; j++) {
                for (i = j; i <= fim - 1; i++) {
                    sum_diff[j][i][0] = diff[j][i][0];
                    for (cnt = 1; cnt <= _PB_LENGTH - 1; cnt++)
                        sum_diff[j][i][cnt] = sum_diff[j][i][cnt - 1] + diff[j][i][cnt];
                    mean[j][i] = sum_diff[j][i][_PB_LENGTH - 1];
                }
            }

            for (i = inicio; i <= fim - 1; i++)
                path[0][i] = mean[0][i];

            /* Add a barrier here to synchronize all threads before continuing */
            pthread_barrier_wait(barrier);

            for (j = inicio + 1; j <= fim - 1; j++) {
                for (i = j; i <= fim - 1; i++) {
                    path[j][i] = path[j - 1][i - 1] + mean[j][i];
                }
            }

            /* Add another barrier here to synchronize all threads before exiting the function */
            pthread_barrier_wait(barrier);
        }

        return NULL;
    }

    /* Create threads */
    pthread_t threads[NUM_THREADS];
    struct ThreadArgs thread_args[NUM_THREADS];

    /* Initialize argument structure and create threads */
    for (int t = 0; t < NUM_THREADS; t++) {
        thread_args[t].maxgrid = maxgrid;
        thread_args[t].sum_tang = sum_tang;
        thread_args[t].mean = mean;
        thread_args[t].path = path;
        thread_args[t].diff = diff;
        thread_args[t].sum_diff = sum_diff;
        thread_args[t].inicio = t * (maxgrid) / NUM_THREADS;
        thread_args[t].fim = (t + 1) * (maxgrid) / NUM_THREADS;
        thread_args[t].barrier = barrier;
        pthread_create(&threads[t], NULL, thread_function, &thread_args[t]);
    }

    /* Wait for threads to finish */
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

#pragma endscop
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
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
    /* Retrieve problem size. */
    int niter = NITER;
    int maxgrid = MAXGRID;
    int length = LENGTH;

    /* Variable declaration/allocation. */
    POLYBENCH_2D_ARRAY_DECL(sum_tang, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
    POLYBENCH_2D_ARRAY_DECL(mean, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
    POLYBENCH_2D_ARRAY_DECL(path, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
    POLYBENCH_3D_ARRAY_DECL(diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length);
    POLYBENCH_3D_ARRAY_DECL(sum_diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length);

    /* Initialize array(s). */
    init_array(maxgrid,
               POLYBENCH_ARRAY(sum_tang),
               POLYBENCH_ARRAY(mean),
               POLYBENCH_ARRAY(path));

    /* Initialize barrier */
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    /* Start timer. */
    polybench_start_instruments;

    /* Run kernel. */
    kernel_reg_detect(niter, maxgrid, length,
                      POLYBENCH_ARRAY(sum_tang),
                      POLYBENCH_ARRAY(mean),
                      POLYBENCH_ARRAY(path),
                      POLYBENCH_ARRAY(diff),
                      POLYBENCH_ARRAY(sum_diff),
                      &barrier);

    /* Stop and print timer. */
    polybench_stop_instruments;
    polybench_print_instruments;

    /* Prevent dead-code elimination. All live-out data must be printed
       by the function call in argument. */
    polybench_prevent_dce(print_array(maxgrid, POLYBENCH_ARRAY(path)));

    /* Be clean. */
    POLYBENCH_FREE_ARRAY(sum_tang);
    POLYBENCH_FREE_ARRAY(mean);
    POLYBENCH_FREE_ARRAY(path);
    POLYBENCH_FREE_ARRAY(diff);
    POLYBENCH_FREE_ARRAY(sum_diff);

    /* Destroy barrier */
    pthread_barrier_destroy(&barrier);

    return 0;
}
