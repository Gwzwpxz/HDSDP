#ifndef dsdpsolver_h
#define dsdpsolver_h

#include "stepheur.h"
#include "residualsetup.h"
#include "structs.h"
#include "dsdpdata.h"
#include "dsdpparam.h"

#define IterStep 20
#define nEvent   10

typedef struct {
    
    // Problem data
    sdpMat   **sdpData;   // SDP data A after transformation.
    DSDP_INT *isSDPset;
    
    lpMat    *lpData;     // LP data A after transformation
    vec      *lpObj;        // Linear coefficient
    DSDP_INT isLPset;
    
    vec      *dObj;       // Dual objective b
    
    // Dimension data
    DSDP_INT m;           // Dimension of dual
    DSDP_INT nBlock;      // Number of SDP blocks ( = 1) currently
    DSDP_INT lpDim;       // Dimension of LP
    
    // Iteration Monitor
    DSDP_INT iterProgress[IterStep];
    DSDP_INT eventMonitor[nEvent];
    
    // Iterator
    double  pObjVal;      // Primal objective
    double  dObjVal;      // Dual objective
    double  mu;           // Duality gap
    
    spsMat   **S;         // SDP dual iteration matrix
    vec      *s;          // LP dual iteration matrix
    vec      *x;          // LP primal iteration vector
    
    dsMat    **SinvASinv; // Store Sinv * A * Sinv for different A_i
    vec      **asinv;     // Store trace(A * Sinv) for different A_i (including C),
                          // setup as a by product
    double   *csinv;      // Store csinv for different C_i and Sinv_i
    
    dsMat    *Msdp;       // Schur matrix for SDP (dense)
    spsMat   *Mlp;        // Schur matrix for LP  (sparse)
    vec      *u;          // A_ls^(-2)c + AS^(-1)CS^(-1)
    vec      *d1;         // Temporary iteration array
    vec      *d2;         // Temporary iteration array
    
    vec      *y;          // y
    double   tau;         // tau
    
    double   alpha;       // Stepsize
    
    // Step matrix
    spsMat   **dS;        // SDP step matrix
    vec      *ds;         // LP step matrix
    double   dtau;        // Tau step
    double   dkappa;      // Kappa step
    
    // Solver parameter
    hsdParam *param;      // Solver parameters
    
    // Solver status
    DSDP_INT insStatus;   // Solver instance status
    DSDP_INT solStatus;   // Solver solution status
    
    // Logging
    DSDP_INT verbosity;   // Solver information
    
    // Primal variable
    vec      *pScaler;    // Primal scaling coefficient
    dsMat    **X;         // Primal solution matrix
    DSDP_INT isXComputed; // Whether X has been computed
    
} HSDSolver;



#endif /* dsdpsolver_h */
