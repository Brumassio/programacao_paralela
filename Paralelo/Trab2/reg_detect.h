/**
 * reg_detect.h: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#ifndef REG_DETECT_H
# define REG_DETECT_H

/* Default to STANDARD_DATASET. */
# if !defined(MINI_DATASET) && !defined(SMALL_DATASET) && !defined(LARGE_DATASET) && !defined(EXTRALARGE_DATASET)
#  define STANDARD_DATASET
# endif

/* Do not define anything if the user manually defines the size. */
# if !defined(NITER) && !defined(LENGTH) && !defined(MAXGRID)
/* Define the possible dataset sizes. */
#  ifdef MINI_DATASET
#   define NITER 10000
#   define LENGTH 1000
#   define MAXGRID 20
#  endif

#  ifdef SMALL_DATASET
#   define NITER 15000
#   define LENGTH 2000
#   define MAXGRID 20
#  endif

#  ifdef STANDARD_DATASET /* Default if unspecified. */
#   define NITER 20000
#   define LENGTH 3000
#   define MAXGRID 20
#  endif

#  ifdef LARGE_DATASET
#   define NITER 25000
#   define LENGTH 4000
#   define MAXGRID 20
#  endif

#  ifdef EXTRALARGE_DATASET
#   define NITER 30000
#   define LENGTH 5000
#   define MAXGRID 20
#  endif
# endif /* !N */

# define _PB_NITER POLYBENCH_LOOP_BOUND(NITER,niter)
# define _PB_LENGTH POLYBENCH_LOOP_BOUND(LENGTH,length)
# define _PB_MAXGRID POLYBENCH_LOOP_BOUND(MAXGRID,maxgrid)

# ifndef DATA_TYPE
#  define DATA_TYPE int
#  define DATA_PRINTF_MODIFIER "%d "
# endif


#endif /* !REG_DETECT */
