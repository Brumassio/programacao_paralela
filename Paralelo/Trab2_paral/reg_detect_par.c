// #include <stdio.h>
// #include <string.h>
// #include <pthread.h>
// #include "polybench.h"
// #include "reg_detect.h"


// typedef struct {
//    pthread_mutex_t mutex;
//    pthread_cond_t cond;
//    int count;
//    int max_count;
// } Barrier;


// void barrier_init(Barrier *barrier, int count) {
//    pthread_mutex_init(&barrier->mutex, NULL);
//    pthread_cond_init(&barrier->cond, NULL);
//    barrier->count = 0;
//    barrier->max_count = count;
// }


// void barrier_wait(Barrier *barrier) {
//    pthread_mutex_lock(&barrier->mutex);
//    barrier->count++;
//    if (barrier->count == barrier->max_count) {
//        barrier->count = 0;
//        pthread_cond_broadcast(&barrier->cond);
//    } else {
//        pthread_cond_wait(&barrier->cond, &barrier->mutex);
//    }
//    pthread_mutex_unlock(&barrier->mutex);
// }


// typedef struct {
//    int thread_id;
//    int num_threads;
//    int niter;
//    int maxgrid;
//    int length;
//    DATA_TYPE (*sum_tang)[MAXGRID];
//    DATA_TYPE (*mean)[MAXGRID];
//    DATA_TYPE (*path)[MAXGRID];
//    DATA_TYPE (*diff)[MAXGRID][LENGTH];
//    DATA_TYPE (*sum_diff)[MAXGRID][LENGTH];
//    Barrier *barrier;
// } ThreadArgs;




// static
// void print_array(int maxgrid,
//         DATA_TYPE POLYBENCH_2D(path,MAXGRID,MAXGRID,maxgrid,maxgrid))
// {
//  int i, j;


//  for (i = 0; i < maxgrid; i++)
//    for (j = 0; j < maxgrid; j++) {
//      fprintf (stderr, DATA_PRINTF_MODIFIER, path[i][j]);
//      if ((i * maxgrid + j) % 20 == 0) fprintf (stderr, "\n");
//    }
//  fprintf (stderr, "\n");
// }




// /* Array initialization. */
// static
// void init_array(int maxgrid,
//        DATA_TYPE POLYBENCH_2D(sum_tang,MAXGRID,MAXGRID,maxgrid,maxgrid),
//        DATA_TYPE POLYBENCH_2D(mean,MAXGRID,MAXGRID,maxgrid,maxgrid),
//        DATA_TYPE POLYBENCH_2D(path,MAXGRID,MAXGRID,maxgrid,maxgrid))
// {
//  int i, j;


//  for (i = 0; i < maxgrid; i++)
//    for (j = 0; j < maxgrid; j++) {
//      sum_tang[i][j] = (DATA_TYPE)((i+1)*(j+1));
//      mean[i][j] = ((DATA_TYPE) i-j) / maxgrid;
//      path[i][j] = ((DATA_TYPE) i*(j-1)) / maxgrid;
//    }
// }






// void *kernel_thread(void *args_ptr) {
//    ThreadArgs *args = (ThreadArgs *)args_ptr;


//    int start_t = (args->niter / args->num_threads) * args->thread_id;
//    int end_t = args->thread_id == args->num_threads - 1 ? args->niter : start_t + (args->niter / args->num_threads);


//    for (int t = start_t; t < end_t; t++) {
//        for (int j = 0; j <= args->maxgrid - 1; j++) {
//            for (int i = j; i <= args->maxgrid - 1; i++) {
//                for (int cnt = 0; cnt <= args->length - 1; cnt++) {
//                    args->diff[j][i][cnt] = args->sum_tang[j][i];
//                }
//            }
//        }


//        barrier_wait(args->barrier);


//        for (int j = 0; j <= args->maxgrid - 1; j++) {
//            for (int i = j; i <= args->maxgrid - 1; i++) {
//                args->sum_diff[j][i][0] = args->diff[j][i][0];
//                for (int cnt = 1; cnt <= args->length - 1; cnt++) {
//                    args->sum_diff[j][i][cnt] = args->sum_diff[j][i][cnt - 1] + args->diff[j][i][cnt];
//                }
//                args->mean[j][i] = args->sum_diff[j][i][args->length - 1];
//            }
//        }


//        barrier_wait(args->barrier);


//         for (int i = 0; i <= args->maxgrid - 1; i++) {
//             args->path[0][i] = args->mean[0][i];
//         }


//        for (int j = 1; j <= args->maxgrid - 1; j++) {
//            for (int i = j; i <= args->maxgrid - 1; i++) {
//                args->path[j][i] = args->path[j - 1][i - 1] + args->mean[j][i];
//            }
//        }


//        barrier_wait(args->barrier);
//    }


//    return NULL;
// }


// int main(int argc, char **argv) {
//    int niter = NITER;
//    int maxgrid = MAXGRID;
//    int length = LENGTH;
//    int num_threads = 4;


//    POLYBENCH_2D_ARRAY_DECL(sum_tang, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
//    POLYBENCH_2D_ARRAY_DECL(mean, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
//    POLYBENCH_2D_ARRAY_DECL(path, DATA_TYPE, MAXGRID, MAXGRID, maxgrid, maxgrid);
//    POLYBENCH_3D_ARRAY_DECL(diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length);
//    POLYBENCH_3D_ARRAY_DECL(sum_diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, maxgrid, maxgrid, length);


//    init_array(maxgrid, POLYBENCH_ARRAY(sum_tang), POLYBENCH_ARRAY(mean), POLYBENCH_ARRAY(path));


//    polybench_start_instruments;


//    Barrier barrier;
//    barrier_init(&barrier, num_threads);


//    pthread_t threads[num_threads];
//    ThreadArgs thread_args[num_threads];
//    for (int i = 0; i < num_threads; i++) {
//        thread_args[i].thread_id = i;
//        thread_args[i].num_threads = num_threads;
//        thread_args[i].niter = niter;
//        thread_args[i].maxgrid = maxgrid;
//        thread_args[i].length = length;
//        thread_args[i].sum_tang = sum_tang;
//        thread_args[i].mean = mean;
//        thread_args[i].path = path;
//        thread_args[i].diff = diff;
//        thread_args[i].sum_diff = sum_diff;
//        thread_args[i].barrier = &barrier;


//        pthread_create(&threads[i], NULL, kernel_thread, &thread_args[i]);
//    }


//    for (int i = 0; i < num_threads; i++) {
//        pthread_join(threads[i], NULL);
//    }


//    polybench_stop_instruments;
//    polybench_print_instruments;


//    polybench_prevent_dce(print_array(maxgrid, POLYBENCH_ARRAY(path)));


//    POLYBENCH_FREE_ARRAY(sum_tang);
//    POLYBENCH_FREE_ARRAY(mean);
//    POLYBENCH_FREE_ARRAY(path);
//    POLYBENCH_FREE_ARRAY(diff);
//    POLYBENCH_FREE_ARRAY(sum_diff);


//    return 0;
// }


/**
 * reg_detect.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include "polybench.h"

/* Include benchmark-specific header. */
/* Default data type is int, default size is 50. */
#include "reg_detect.h"


/* Array initialization. */
static
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


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
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


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
/* Source (modified): http://www.cs.uic.edu/~iluican/reg_detect.c */
static void kernel_reg_detect(int niter, int maxgrid, int length,
               DATA_TYPE POLYBENCH_2D(sum_tang,MAXGRID,MAXGRID,maxgrid,maxgrid),
               DATA_TYPE POLYBENCH_2D(mean,MAXGRID,MAXGRID,maxgrid,maxgrid),
               DATA_TYPE POLYBENCH_2D(path,MAXGRID,MAXGRID,maxgrid,maxgrid),
               DATA_TYPE POLYBENCH_3D(diff,MAXGRID,MAXGRID,LENGTH,maxgrid,maxgrid,length),
               DATA_TYPE POLYBENCH_3D(sum_diff,MAXGRID,MAXGRID,LENGTH,maxgrid,maxgrid,length))
{
  int t, i, j, cnt;

#pragma scop
/*
  for (t = 0; t < _PB_NITER; t++)
    {
      for (j = 0; j <= _PB_MAXGRID - 1; j++)
    for (i = j; i <= _PB_MAXGRID - 1; i++)
      for (cnt = 0; cnt <= _PB_LENGTH - 1; cnt++)
        diff[j][i][cnt] = sum_tang[j][i];

      for (j = 0; j <= _PB_MAXGRID - 1; j++)
        {
      for (i = j; i <= _PB_MAXGRID - 1; i++)
            {
          sum_diff[j][i][0] = diff[j][i][0];
          for (cnt = 1; cnt <= _PB_LENGTH - 1; cnt++)
        sum_diff[j][i][cnt] = sum_diff[j][i][cnt - 1] + diff[j][i][cnt];
          mean[j][i] = sum_diff[j][i][_PB_LENGTH - 1];
            }
        }

      for (i = 0; i <= _PB_MAXGRID - 1; i++)
    path[0][i] = mean[0][i];

      for (j = 1; j <= _PB_MAXGRID - 1; j++)
    for (i = j; i <= _PB_MAXGRID - 1; i++)
      path[j][i] = path[j - 1][i - 1] + mean[j][i];
    }

*/  

for (t = 0; t < _PB_NITER; t++) {
    for (j = 0; j <= _PB_MAXGRID - 1; j++) {
        for (i = j; i <= _PB_MAXGRID - 1; i++) {
            for (cnt = 0; cnt <= _PB_LENGTH - 1; cnt++) {
                diff[j][i][cnt] = sum_tang[j][i];
            }
        }
    }

    for (j = 0; j <= _PB_MAXGRID - 1; j++) {
        for (i = j; i <= _PB_MAXGRID - 1; i++) {
            sum_diff[j][i][0] = diff[j][i][0];
            for (cnt = 1; cnt <= _PB_LENGTH - 1; cnt++) {
                sum_diff[j][i][cnt] = sum_diff[j][i][cnt - 1] + diff[j][i][cnt];
            }
            mean[j][i] = sum_diff[j][i][_PB_LENGTH - 1];
        }
    }

    for (i = 0; i <= _PB_MAXGRID - 1; i++) {
        path[0][i] = mean[0][i];
    }

    for (j = 1; j <= _PB_MAXGRID - 1; j++) {
        for (i = j; i <= _PB_MAXGRID - 1; i++) {
            path[j][i] = path[j - 1][i - 1] + mean[j][i];
        }
    }
}

#pragma endscop

}



int main(int argc, char** argv)
{
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
  init_array (maxgrid,
	      POLYBENCH_ARRAY(sum_tang),
	      POLYBENCH_ARRAY(mean),
	      POLYBENCH_ARRAY(path));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_reg_detect (niter, maxgrid, length,
		     POLYBENCH_ARRAY(sum_tang),
		     POLYBENCH_ARRAY(mean),
		     POLYBENCH_ARRAY(path),
		     POLYBENCH_ARRAY(diff),
		     POLYBENCH_ARRAY(sum_diff));

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

  return 0;
}
