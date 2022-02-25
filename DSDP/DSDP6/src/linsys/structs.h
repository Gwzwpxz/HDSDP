#ifndef structs_h
#define structs_h

#include "cs.h"
#include "dsdphsd.h"
#include "dsdppardiso.h"

/* In DSDP rank-1 matrix is represented by d or -d * a * a' and a is used to represent the matrix */
typedef struct {
    
    double   sign;   // The sign before the vector
    DSDP_INT dim;    // Dimension of the rank 1 matrix
    double   *x;     // Vector a
    DSDP_INT *nzIdx; // Index of nonzero elements
    DSDP_INT nnz;    // Number of nonzero elements in a
    
} r1Mat;

/* In DSDP rank-k matrix is represented by the sum of multiple rank-1 matrices */
typedef struct {
    
    DSDP_INT dim;
    DSDP_INT isdata;
    DSDP_INT rank;
    r1Mat    **data;
    
} rkMat;

typedef struct {
    
    DSDP_INT dim;                       // Dimension of the sparse matrix
    DSDP_INT isFactorized;              // Whether the sparse matrix has been factorized
    DSDP_INT *p;                        // Column pointer
    DSDP_INT *i;                        // Row index
    double   *x;                        // Sparse data
    DSDP_INT nnz;                       // Nonzeros entries
    DSDP_INT *nzHash;                   // Position of non-zero entry in packed format
    
    rkMat    *factor;                   // Eigenvalue decomposition
    void     *pdsWorker[PARDISOINDEX];  // Pardiso working array
    
} spsMat;


typedef struct {
    
    DSDP_INT dim;          // Dimension of the matrix
    DSDP_INT isFactorized; // Whether the dense matrix is factorized
    DSDP_INT isillCond;    // Whether the matrix suffers from ill conditioning
    
    rkMat    *factor;      // Eigenvalue decomposition
    double   *array;       // The (dim + 1) * dim / 2 array
    double   *lfactor;     // The packed Cholesky factor returned by dppsv or other Lapack routines
    DSDP_INT *ipiv;        // Array in case of indefinite Schur matrix
    
} dsMat;


typedef struct {

    DSDP_INT dim;  // Dimension of the vectorß
    double   *x;   // Array storing the data
    
} vec;


#define packIdx(P, n, i, j) (P[(DSDP_INT)((2 * (n) - (j) - 1) * (j) / 2) + (i)])

#endif /* structs_h */
