#ifndef dsdpparam_h
#define dsdpparam_h

/* Implement the parameter interface for DSDP */
#include "dsdphsd.h"

#define DBL_PARAM_ASIGMA         0
#define DBL_PARAM_BSIGMA         1
#define DBL_PARAM_RHO            2
#define DBL_PARAM_RHON           3
#define DBL_PARAM_INIT_POBJ      4
#define DBL_PARAM_INIT_BETA      5
#define DBL_PARAM_INIT_MU        6
#define DBL_PARAM_INIT_TAU       7
#define DBL_PARAM_INIT_KAPPA     8
#define DBL_PARAM_AALPHA         9
#define DBL_PARAM_NRM_THRESH    10
#define DBL_PARAM_INFEAS_THRESH 11
#define DBL_PARAM_ABS_OPTTOL    12
#define DBL_PARAM_REL_OPTTOL    13
#define DBL_PARAM_ABS_FEASTOL   14
#define DBL_PARAM_REL_FEASTOL   15
#define DBL_PARAM_PRLX_PENTALTY 16
#define DBL_PARAM_BOUND_X       17
#define DBL_PARAM_OBJ_WEIGHT    18

#define INT_PARAM_ACORRECTOR     0
#define INT_PARAM_BCORRECTOR     1
#define INT_PARAM_INITMETHOD     2
#define INIT_METHOD_FRO         20
#define INT_PARAM_AATTEMPT       3
#define AATEMPT_AGGRESSIVE      30
#define AATEMPT_MILD            31
#define AATEMT_CONSERVATIVE     32
#define INT_PARAM_CG_REUSE       4
#define INT_PARAM_PRESOLVE       5
#define PRESOLVE_AGGRESSIVE     50
#define PRESOLVE_CONSERVATIVE   51
#define INT_PARAM_AMAXITER       6
#define INT_PARAM_BMAXITER       7
#define INT_PARAM_GOLDSEARCH     8
#define INT_PARAM_PRELAX         9

#define NUM_DBL_PARAM  (DBL_PARAM_OBJ_WEIGHT + 1)
#define NUM_INT_PARAM  (INT_PARAM_PRELAX     + 1)

typedef struct {
    
    DSDP_INT *intParams;
    double   *dblParams;
    
} hsdParam;

static DSDP_INT defaultIntParam[NUM_INT_PARAM] = {
    4,     // INT_PARAM_ACORRECTOR
    0,     // INT_PARAM_BCORRECTOR
    INIT_METHOD_FRO,
           // INT_PARAM_INITMETHOD
    AATEMT_CONSERVATIVE,
           // INT_PARAM_AATTEMPT
    100,    // INT_PARAM_CG_REUSE
    PRESOLVE_AGGRESSIVE,
           // INT_PARAM_PRESOLVE
    500,   // INT_PARAM_AMAXITER
    500,   // INT_PARAM_BMAXITER
    FALSE,  // INT_PARAM_GOLDSEARCH
    TRUE  // INT_PARAM_PRELAX
};

static double defaultDblParam[NUM_DBL_PARAM] = {
    
    0.7,   // DBL_PARAM_ASIGMA
    0.8,   // DBL_PARAM_BSIGMA
    4.0,   // DBL_PARAM_RHO
    4.0,   // DBL_PARAM_RHON
    1e+10, // DBL_PARAM_INIT_POBJ
    
    1e+06, // DBL_PARAM_INIT_BETA
    1e+15, // DBL_PARAM_INIT_MU
    1.0, // DBL_PARAM_INIT_TAU
    
    1.0,   // DBL_PARAM_INIT_KAPPA
    0.9,  // DBL_PARAM_AALPHA // 0.99995 is good
    1e+08, // DBL_PARAM_NRM_THRESH
    1e+08, // DBL_PARAM_INFEAS_THRESH
    1e-06, // DBL_PARAM_ABS_OPTTOL
    1e-06, // DBL_PARAM_REL_OPTTOL
    1e-07, // DBL_PARAM_ABS_FEASTOL
    1e-07, // DBL_PARAM_REL_FEASTOL
    1e+07, // DBL_PARAM_PRLX_PENTALTY
    1e+08, // DBL_PARAM_BOUND_X
    0.0   // DBL_PARAM_OBJ_WEIGHT
};

static hsdParam defaultParam =
{
    defaultIntParam,
    defaultDblParam
};

extern DSDP_INT setDblParam ( hsdParam *param, DSDP_INT pName, double    dblVal );
extern DSDP_INT getDblParam ( hsdParam *param, DSDP_INT pName, double   *dblVal );
extern DSDP_INT setIntParam ( hsdParam *param, DSDP_INT pName, DSDP_INT  intVal );
extern DSDP_INT getIntParam ( hsdParam *param, DSDP_INT pName, DSDP_INT *intVal );
extern void     printParams ( hsdParam *param                                   );
extern void     DSDPParamPrint ( hsdParam *param );

#endif /* dsdpparam_h */
