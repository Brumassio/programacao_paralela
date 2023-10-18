#include <stdio.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

/* Include polybench common header. */
#include "polybench.h"
/* Include benchmark-specific header. */
/* Default data type is int, default size is 50. */
#include "reg_detect.h"
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
                       int start_iter, int end_iter)
{
    int t, i, j, cnt;

    for (t = start_iter; t < end_iter; t++) {
        for (j = 0; j < maxgrid; j++) {
            for (i = j; i < maxgrid; i++) {
                for (cnt = 0; cnt < _PB_LENGTH; cnt++)
                    diff[j][i][cnt] = sum_tang[j][i];
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        for (j = 0; j < maxgrid; j++) {
            for (i = j; i < maxgrid; i++) {
                sum_diff[j][i][0] = diff[j][i][0];
                for (cnt = 1; cnt < _PB_LENGTH; cnt++)
                    sum_diff[j][i][cnt] = sum_diff[j][i][cnt - 1] + diff[j][i][cnt];
                mean[j][i] = sum_diff[j][i][_PB_LENGTH - 1];
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        for (i = 0; i < maxgrid; i++)
            path[0][i] = mean[0][i];

        for (j = 1; j < maxgrid; j++) {
            for (i = j; i < maxgrid; i++) {
                path[j][i] = path[j - 1][i - 1] + mean[j][i];
            }
        }
    }
}

int main(int argc, char **argv)
{
    /* Initialize MPI */
    MPI_Init(&argc, &argv);

    /* Retrieve the rank and size of the MPI process. */
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
    /* Retrieve problem size. */
    int niter = NITER;
    int maxgrid = MAXGRID;
    int length = LENGTH;

    /* Calculate workload per process */
    int work_per_process = niter / size;
    int extra_work = niter % size;

    int start_iter = rank * work_per_process + ((rank < extra_work) ? rank : extra_work);
    int end_iter = start_iter + work_per_process + ((rank < extra_work) ? 1 : 0);

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

    /* Start timer. */
    polybench_start_instruments;

    /* Run kernel. */
    kernel_reg_detect(niter, maxgrid, length,
                      POLYBENCH_ARRAY(sum_tang),
                      POLYBENCH_ARRAY(mean),
                      POLYBENCH_ARRAY(path),
                      POLYBENCH_ARRAY(diff),
                      POLYBENCH_ARRAY(sum_diff),
                      start_iter, end_iter);

    MPI_Barrier(MPI_COMM_WORLD);

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

    /* Finalize MPI */
    MPI_Finalize();

    return 0;
}
