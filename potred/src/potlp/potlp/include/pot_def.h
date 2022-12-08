/** @file pot\_def.h
 *  @brief The header file for the user interface of potential reduction algorithm for linear programming
 *  @author Wenzhi Gao, Shanghai University of Finance and Economics
 *  @date Sep 22th, 2022
 *
 * @TODO: Add more detailed comments
 */
#ifndef potlp_h
#define potlp_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef MATLAB_MEX_FILE
#include "mex.h"
#define calloc mxCalloc
#define printf mexPrintf
#define free mxFree
#endif

#ifdef POTLP64
#include <stdint.h>
typedef int64_t pot_int;
#define ID "%ld"
#else
typedef int pot_int;
#define ID "%d"
#endif

//#ifndef MEMWATCH
//#define MEMWATCH
//#include "memwatch.h"
//#endif

// Memory handler
#define POTLP_FREE(var) do {free((var)); (var) = NULL;} while (0)
#define POTLP_INIT(var, type, size) (var) = (type *) calloc(size, sizeof(type))
#define POTLP_MEMCPY(dst, src, type, size) memcpy(dst, src, sizeof(type) * (size))
#define POTLP_ZERO(var, type, size) memset(var, 0, sizeof(type) * (size))

// Return code
#define RETCODE_OK     (0)
#define RETCODE_FAILED (1)

// Algorithm statuss
#define POTLP_UNKNOWN                (99)
#define POTLP_OPTIMAL               (100)
#define POTLP_MAXITER               (101)
#define POTLP_INFEAS_OR_UNBOUNDED   (102)
#define POTLP_TIMELIMIT             (103)
#define POTLP_USER_INTERRUPT        (104)
#define POTLP_INTERNAL_ERROR        (105)
#define POTLP_NUMERICAL             (106)

// Integer Parameters
#define INT_PARAM_MAXITER       0
#define INT_PARAM_MAXRUIZITER   1
#define INT_PARAM_MAXPCITER     2
#define INT_PARAM_L2SCALE       3
#define INT_PARAM_COEFSCALE     4
#define INT_PARAM_CURVATURE     5
#define INT_PARAM_CURVINTERVAL  6
#define INT_PARAM_RSCALFREQ     7
#define INT_PARAM_SCALSIMPLEX   8
#define INT_PARAM_QPWINDOW      9
#define INT_PARAM_RECORDFREQ   10

// Double Parameters
#define DBL_PARAM_RELFEASTOL    0
#define DBL_PARAM_RELOPTTOL     1
#define DBL_PARAM_TIMELIMIT     2

// Version information
#define VERSION_MAJOR           0
#define VERSION_MINOR           1
#define VERSION_TECHNICAL       0

#define BUILD_DATE_YEAR         2022
#define BUILD_DATE_MONTH        10
#define BUILD_DATE_DAY          10

#define POTLP_MAX(x, y) (x) >= (y) ? (x) : (y);
#define POTLP_MIN(x, y) (x) <= (y) ? (x) : (y);

#endif /* pot_def_h */
