/**
 * polybench.h: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
/*
 * Polybench header for instrumentation.
 *
 * Programs must be compiled with `-I utilities utilities/polybench.c'
 *
 * Optionally, one can define:
 *
 * -DPOLYBENCH_TIME, to report the execution time,
 *   OR (exclusive):
 * -DPOLYBENCH_PAPI, to use PAPI H/W counters (defined in polybench.c)
 *
 *
 * See README or utilities/polybench.c for additional options.
 *
 */
#ifndef POLYBENCH_H
# define POLYBENCH_H

# include <stdlib.h>

/* Array padding. By default, none is used. */
# ifndef POLYBENCH_PADDING_FACTOR
/* default: */
#  define POLYBENCH_PADDING_FACTOR 0
# endif


/* C99 arrays in function prototype. By default, do not use. */
# ifdef POLYBENCH_USE_C99_PROTO
#  define POLYBENCH_C99_SELECT(x,y) y
# else
/* default: */
#  define POLYBENCH_C99_SELECT(x,y) x
# endif


/* Scalar loop bounds in SCoPs. By default, use parametric loop bounds. */
# ifdef POLYBENCH_USE_SCALAR_LB
#  define POLYBENCH_LOOP_BOUND(x,y) x
# else
/* default: */
#  define POLYBENCH_LOOP_BOUND(x,y) y
# endif


/* Macros to reference an array. Generic for heap and stack arrays
   (C99).  Each array dimensionality has his own macro, to be used at
   declaration or as a function argument.
   Example:
   int b[x] => POLYBENCH_1D_ARRAY(b, x)
   int A[N][N] => POLYBENCH_2D_ARRAY(A, N, N)
*/
# ifndef POLYBENCH_STACK_ARRAYS
#  define POLYBENCH_ARRAY(x) *x
#  define POLYBENCH_FREE_ARRAY(x) free((void*)x);
#  define POLYBENCH_DECL_VAR(x) (*x)
# else
#  define POLYBENCH_ARRAY(x) x
#  define POLYBENCH_FREE_ARRAY(x)
#  define POLYBENCH_DECL_VAR(x) x
# endif
/* Macros for using arrays in the function prototypes. */
# define POLYBENCH_1D(var, dim1,ddim1) var[POLYBENCH_C99_SELECT(dim1,ddim1) + POLYBENCH_PADDING_FACTOR]
# define POLYBENCH_2D(var, dim1, dim2, ddim1, ddim2) var[POLYBENCH_C99_SELECT(dim1,ddim1) + POLYBENCH_PADDING_FACTOR][POLYBENCH_C99_SELECT(dim2,ddim2) + POLYBENCH_PADDING_FACTOR]
# define POLYBENCH_3D(var, dim1, dim2, dim3, ddim1, ddim2, ddim3) var[POLYBENCH_C99_SELECT(dim1,ddim1) + POLYBENCH_PADDING_FACTOR][POLYBENCH_C99_SELECT(dim2,ddim2) + POLYBENCH_PADDING_FACTOR][POLYBENCH_C99_SELECT(dim3,ddim3) + POLYBENCH_PADDING_FACTOR]


/* Macros to allocate heap arrays.
   Example:
   polybench_alloc_2d_array(N, M, double) => allocates N x M x sizeof(double)
 and returns a pointer to the 2d array
 */
# define POLYBENCH_ALLOC_2D_ARRAY(n1, n2, type) \
  (type(*)[n1 + POLYBENCH_PADDING_FACTOR][n2 + POLYBENCH_PADDING_FACTOR])polybench_alloc_data ((n1 + POLYBENCH_PADDING_FACTOR) * (n2 + POLYBENCH_PADDING_FACTOR), sizeof(type))
# define POLYBENCH_ALLOC_3D_ARRAY(n1, n2, n3, type) \
  (type(*)[n1 + POLYBENCH_PADDING_FACTOR][n2 + POLYBENCH_PADDING_FACTOR][n3 + POLYBENCH_PADDING_FACTOR])polybench_alloc_data ((n1 + POLYBENCH_PADDING_FACTOR) * (n2 + POLYBENCH_PADDING_FACTOR) * (n3 + POLYBENCH_PADDING_FACTOR), sizeof(type))

/* Macros for array declaration. */
# ifndef POLYBENCH_STACK_ARRAYS
#  define POLYBENCH_2D_ARRAY_DECL(var, type, dim1, dim2, ddim1, ddim2) \
  type POLYBENCH_2D(POLYBENCH_DECL_VAR(var), dim1, dim2, ddim1, ddim2); \
  var = POLYBENCH_ALLOC_2D_ARRAY(POLYBENCH_C99_SELECT(dim1, ddim1), POLYBENCH_C99_SELECT(dim2, ddim2), type);
#  define POLYBENCH_3D_ARRAY_DECL(var, type, dim1, dim2, dim3, ddim1, ddim2, ddim3) \
  type POLYBENCH_3D(POLYBENCH_DECL_VAR(var), dim1, dim2, dim3, ddim1, ddim2, ddim3); \
  var = POLYBENCH_ALLOC_3D_ARRAY(POLYBENCH_C99_SELECT(dim1, ddim1), POLYBENCH_C99_SELECT(dim2, ddim2), POLYBENCH_C99_SELECT(dim3, ddim3), type);
# else
#  define POLYBENCH_2D_ARRAY_DECL(var, type, dim1, dim2, ddim1, ddim2) \
  type POLYBENCH_2D(POLYBENCH_DECL_VAR(var), dim1, dim2, ddim1, ddim2);
#  define POLYBENCH_3D_ARRAY_DECL(var, type, dim1, dim2, dim3, ddim1, ddim2, ddim3) \
  type POLYBENCH_3D(POLYBENCH_DECL_VAR(var), dim1, dim2, dim3, ddim1, ddim2, ddim3);
# endif


/* Dead-code elimination macros. Use argc/argv for the run-time check. */
# ifndef POLYBENCH_DUMP_ARRAYS
#  define POLYBENCH_DCE_ONLY_CODE    if (argc > 42 && ! strcmp(argv[0], ""))
# else
#  define POLYBENCH_DCE_ONLY_CODE
# endif

# define polybench_prevent_dce(func) \
  POLYBENCH_DCE_ONLY_CODE \
  func


/* Performance-related instrumentation. See polybench.c */
# define polybench_start_instruments
# define polybench_stop_instruments
# define polybench_print_instruments



/* Timing support. */
# if defined(POLYBENCH_TIME) || defined(POLYBENCH_GFLOPS)
#  undef polybench_start_instruments
#  undef polybench_stop_instruments
#  undef polybench_print_instruments
#  define polybench_start_instruments polybench_timer_start();
#  define polybench_stop_instruments polybench_timer_stop();
#  define polybench_print_instruments polybench_timer_print();
extern double polybench_program_total_flops;
extern void polybench_timer_start();
extern void polybench_timer_stop();
extern void polybench_timer_print();
# endif

/* Function declaration. */
# ifdef POLYBENCH_TIME
extern void polybench_timer_start();
extern void polybench_timer_stop();
extern void polybench_timer_print();
# endif


/* Function prototypes. */
extern void* polybench_alloc_data(unsigned long long int n, int elt_size);


#endif /* !POLYBENCH_H */
