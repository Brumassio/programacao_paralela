#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <string.h>

#define MAXGRID 1000
#define LENGTH 100
#define NITER 10
#define DATA_TYPE double
#define NUM_THREADS 4

/* Include polybench common header. */
#include "polybench.h"

/* Include benchmark-specific header. */
#include "reg_detect.h"

pthread_barrier_t barrier;

struct ThreadData {
    int thread_id;
    int maxgrid;
    int length;
    DATA_TYPE (*sum_tang)[MAXGRID][MAXGRID];
    DATA_TYPE (*mean)[MAXGRID][MAXGRID];
    DATA_TYPE (*path)[MAXGRID][MAXGRID];
    DATA_TYPE (*diff)[MAXGRID][MAXGRID][LENGTH];
    DATA_TYPE (*sum_diff)[MAXGRID][MAXGRID][LENGTH];
};

#define DATA_PRINTF_MODIFIER "%lf"

void init_array(int maxgrid,
                DATA_TYPE POLYBENCH_2D(sum_tang,MAXGRID,MAXGRID,maxgrid,maxgrid),
                DATA_TYPE POLYBENCH_2D(mean,MAXGRID,MAXGRID,maxgrid,maxgrid),
                DATA_TYPE POLYBENCH_2D(path,MAXGRID,MAXGRID,maxgrid,maxgrid))
{
    int i, j;

    for (i = 0; i < maxgrid; i++)
        for (j = 0; j < maxgrid; j++) {
            sum_tang[i][j] = (DATA_TYPE)((i+1)*(j+1));
            mean[i][j] = ((DATA_TYPE) i-j) / maxgrid;
            path[i][j] = ((DATA_TYPE) i*(j-1)) / maxgrid;
        }
}

void print_array(int maxgrid,
                 DATA_TYPE POLYBENCH_2D(path,MAXGRID,MAXGRID,maxgrid,maxgrid))
{
    int i, j;

    for (i = 0; i < maxgrid; i++)
        for (j = 0; j < maxgrid; j++) {
            fprintf (stderr, DATA_PRINTF_MODIFIER, path[i][j]);
            if ((i * maxgrid + j) % 20 == 0) fprintf (stderr, "\n");
        }
    fprintf (stderr, "\n");
}

void* thread_function(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;
    int maxgrid = data->maxgrid;
    int length = data->length;

    for (int t = 0; t < NITER; t++) {
        for (int j = 0; j < maxgrid; j++)
            for (int i = j; i < maxgrid; i++)
                for (int cnt = 0; cnt < length; cnt++)
                    data->diff[j][i][cnt] = data->sum_tang[j][i][cnt];

        for (int j = 0; j < maxgrid; j++) {
            for (int i = j; i < maxgrid; i++) {
                data->sum_diff[j][i][0] = data->diff[j][i][0];
                for (int cnt = 1; cnt < length; cnt++)
                    data->sum_diff[j][i][cnt] = data->sum_diff[j][i][cnt - 1] + data->diff[j][i][cnt];
                data->mean[j][i] = data->sum_diff[j][i][length - 1];
            }
        }

        for (int i = 0; i < maxgrid; i++)
            data->path[0][i] = data->mean[0][i];

        for (int j = 1; j < maxgrid; j++)
            for (int i = j; i < maxgrid; i++)
                data->path[j][i] = data->path[j - 1][i - 1] + data->mean[j][i];
    }

    pthread_barrier_wait(&barrier);
    return NULL;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int num_procs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int maxgrid = MAXGRID;
    int length = LENGTH;

    /* Variable declaration/allocation. */
    POLYBENCH_2D_ARRAY_DECL(sum_tang, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
    POLYBENCH_2D_ARRAY_DECL(mean, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
    POLYBENCH_2D_ARRAY_DECL(path, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
    POLYBENCH_3D_ARRAY_DECL(diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length);
    POLYBENCH_3D_ARRAY_DECL(sum_diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length);

    init_array(maxgrid, POLYBENCH_ARRAY(sum_tang), POLYBENCH_ARRAY(mean), POLYBENCH_ARRAY(path));

    polybench_start_instruments;

    pthread_t threads[NUM_THREADS];
    struct ThreadData thread_data[NUM_THREADS];

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    int maxgrid_per_thread = maxgrid / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].maxgrid = maxgrid_per_thread;
        thread_data[i].length = length;
        thread_data[i].sum_tang = &sum_tang[i * maxgrid_per_thread];
        thread_data[i].mean = &mean[i * maxgrid_per_thread];
        thread_data[i].path = &path[i * maxgrid_per_thread];
        thread_data[i].diff = &diff[i * maxgrid_per_thread];
        thread_data[i].sum_diff = &sum_diff[i * maxgrid_per_thread];

        pthread_create(&threads[i], NULL, thread_function, (void*)&thread_data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

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

    MPI_Finalize();

    return 0;
}
