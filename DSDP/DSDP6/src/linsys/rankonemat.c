#include <stdio.h>
#include "rankonemat.h"
#include "sparsemat.h"
#include "dsdplapack.h"

// Define constants involving Lapack and Blas
static DSDP_INT one = 1;
static double done = 1.0;
static double dzero = 0.0;
static char uplolow = 'L';

extern DSDP_INT r1MatInit( r1Mat *x ) {
    // Initialize rank one matrix
    x->x = NULL; x->dim = 0; x->sign = 0;
    x->nnz = 0; x->nzIdx = NULL;
    return DSDP_RETCODE_OK;
}

extern DSDP_INT r1MatAlloc( r1Mat *x, const DSDP_INT n ) {
    // Allocate memory for vec
    assert( x->dim == 0 );
    x->dim = n; x->sign = 1.0;
    x->x = (double *) calloc(n, sizeof(double));
    return DSDP_RETCODE_OK;
}

extern DSDP_INT r1MatSetData( r1Mat *x, double eigval, double *array ) {
    // Set rank-one data
    assert(x->dim > 0);
    memcpy(x->x, array, sizeof(double) * x->dim);
    x->sign = eigval; r1MatCountNnz(x);
    return DSDP_RETCODE_OK;
}

/* M2 Technique */
extern double r1Matr1Trace( r1Mat *x, r1Mat *y ) {
    // Compute the inner product of two rank one matrices. The second one is assumed dense
    double res = 0.0; DSDP_INT n = x->dim;
    if (x->nnz < 0.5 * n) {
        // Otherwise do sparse computation
        double *xdata = x->x, *ydata = y->x;
        DSDP_INT *dataidx = x->nzIdx, i;
        for (i = 0; i < x->nnz; ++i) {
            res += xdata[*dataidx] * ydata[*dataidx]; ++dataidx;
        }
    } else {
        // Use dense dot product if rank-one matrix is dense
        res = dot(&x->dim, x->x, &one, y->x, &one);
    }
    
    return (res * res * x->sign * y->sign);
}

extern void r1MatdenseTrace( r1Mat *x, dsMat *A, double *trace ) {
    // Compute the inner product of a rank-1 matrix and dense A
    DSDP_INT n = x->dim, nnz = x->nnz;
    
    // Two ways of implementation
    double res = 0.0, *Adata = A->array, *xdata = x->x, cval = 0.0, csum;
    if (nnz >= 0.7 * n) {
        // Use usual matrix vector product
        for (DSDP_INT i = 0, j; i < n; ++i) {
            cval = xdata[i];
            csum = 0.5 * packIdx(Adata, n, i, i) * cval;
            for (j = i + 1; j < n; ++j) {
                csum += packIdx(Adata, n, j, i) * xdata[j];
            }
            res += csum * cval;
        }
        /*
         TODO: try the following implementation that avoids casting
         for (DSDP_INT i = 0, j = 0, k = 0; i < n; ++i) {
             cval = xdata[i]; csum = 0.5 * Adata[j] * cval; k = j + 1;
             for (j = k; j < k + n - i; ++j) {
                 csum += Adata[j] * xdata[j - k];
             }
             res += csum * cval;
         }
        */
    } else {
        DSDP_INT cidx, ridx, *nzidx = x->nzIdx;
        // x' * A * x = \sum_{i, j} a_{i,j} * x_i * x_j
        for (DSDP_INT col = 0, row; col < nnz; ++col) {
            cidx = nzidx[col]; cval = xdata[cidx];
            csum = 0.5 * packIdx(Adata, n, cidx, cidx) * cval;
            for (row = col + 1; row < nnz; ++row) {
                ridx = nzidx[row];
                csum += packIdx(Adata, n, ridx, cidx) * xdata[ridx];
            }
            res += csum * cval;
        }
    }
    *trace = 2.0 * res * x->sign;
}

extern double r1MatSinvSolve( const double *Sinv, r1Mat *x, double *ASinv, double *aux, double *asinv, double Ry ) {
    // Compute a * a' * S^-1.
    DSDP_INT n = x->dim, *Ai = x->nzIdx, i, j;
    double *Ax = x->x, *ASinvi, coeff = 0.0, res = 0.0;
    const double *Sinvj; memset(ASinv, 0, sizeof(double) * n * n);
    if (x->nnz < sqrt(n)) {
        for (i = 0; i < x->nnz; ++i) {
            for (j = 0; j < x->nnz; ++j) {
                coeff = x->sign * Ax[Ai[i]] * Ax[Ai[j]];
                Sinvj = Sinv + Ai[j] * n; ASinvi = ASinv + Ai[i];
                daxpy(&n, &coeff, Sinvj, &one, ASinvi, &n);
            }
        }
    } else {
        // S^-1 * a
        memset(aux, 0, sizeof(double) * n);
        for (i = 0; i < x->nnz; ++i) {
            coeff = x->sign * Ax[Ai[i]]; if (coeff == 0.0) { continue; }
            Sinvj = Sinv + n * Ai[i]; daxpy(&n, &coeff, Sinvj, &one, aux, &one);
        }
        dger(&n, &n, &done, Ax, &one, aux, &one, ASinv, &n); // a * (S^-1 * a)'
    }
    for (i = 0; i < n; ++i) { res += ASinv[i * n + i]; }
    *asinv = res; if (Ry == 0.0) { return 0.0; }
    return Ry * ddot(&n, aux, &one, Ax, &one);
//    for (i = 0; i < n; ++i) {
//        Sinvj = Sinv + n * i; ASinvi = ASinv + n * i;
//        res2 += ddot(&n, Sinvj, &one, ASinvi, &one);
//    }
//
//    return (res2 * Ry);
}

extern double r1MatSinvASinv( const double *Sinv, r1Mat *x, const double *ASinv ) {
    // Compute a * a' * S^-1 * (A_i S^-1)
    DSDP_INT n = x->dim, *Ai = x->nzIdx, i, j, p, q;
    double *Ax = x->x, res = 0.0; const double *Sinvi, *ASinvj;
    for (i = 0; i < x->nnz; ++i) {
        for (j = 0; j < x->nnz; ++j) {
            p = Ai[i]; q = Ai[j]; Sinvi = Sinv + n * p; ASinvj = ASinv + n * q;
            res += x->sign * Ax[p] * Ax[q] * ddot(&n, Sinvi, &one, ASinvj, &one);
        }
    }
    return res;
}

extern double r1RySinv2( r1Mat *B, double *aux, double *Sinv, double *asinv, double Ry ) {
    // Auxiliary routine for dense rank one matrix from r1RySinv
    // trace( a * a' * S^-1 ) = (S^-1 * a)' * a and trace( S^-1 * a * a' * S^-1 * Ry )
    DSDP_INT n = B->dim, i; double *a = B->x, tmp1 = 0.0, tmp2 = 0.0;
    if (B->nnz <= n / 2) {
        memset(aux, 0, sizeof(double) * n);
        for (i = 0; i < B->nnz; ++i) {
            daxpy(&n, &a[B->nzIdx[i]], &Sinv[n * B->nzIdx[i]], &one, aux, &one);
        }
    } else {
        // aux = S^-1 * a
        dsymv(&uplolow, &n, &done, Sinv, &n, a, &one, &dzero, aux, &one);
    }
    // ||S^-1 * a||
    tmp1 = dnrm2(&n, aux, &one);
    // asinv = a' * S^-1 * a
    if (B->nnz <= n / 4) {
        for (i = 0; i < B->nnz; ++i) { tmp2 += a[B->nzIdx[i]] * aux[B->nzIdx[i]]; }
        *asinv = tmp2 * B->sign;
    } else {
        *asinv = ddot(&n, aux, &one, a, &one) * B->sign;
    }
    
    return (tmp1 * tmp1 * Ry * B->sign);
}

extern double r1RySinv( r1Mat *B, double *Sinv, double *asinv, double Ry, double *aux ) {
    // Compute trace( a * a' * S^-1 ) and trace( S^-1 * a * a' * S^-1 * Ry )
    if (B->nnz >= B->dim / 4) {
        return r1RySinv2(B, aux, Sinv, asinv, Ry);
    }
    double res = 0.0, res2 = 0.0, tmp, *Bx = B->x;
    DSDP_INT i, p, q, in, n = B->dim, *Bi = B->nzIdx;
    
    for (p = 0; p < B->nnz; ++p) {
        for (q = 0; q < p; ++q) {
            res += Bx[Bi[p]] * Bx[Bi[q]] * Sinv[Bi[p] * n + Bi[q]];
        }
        i = Bi[p]; res += 0.5 * Bx[i] * Bx[i] * Sinv[i * n + i];
    }
    
    for (i = 0; i < n; ++i) {
        in = i * n;
        for (p = 0; p < B->nnz; ++p) {
            for (q = 0; q < p; ++q) {
                res2 += Bx[Bi[p]] * Bx[Bi[q]] * Sinv[Bi[q] + in] * Sinv[Bi[p] + in];
            }
            q = Bi[p]; tmp = Bx[q] * Sinv[q + in]; res2 += 0.5 * tmp * tmp;
        }
    }
    
    *asinv = (2.0 * res * B->sign); return (2.0 * res2 * B->sign * Ry);
}

extern double r1Sinvr1( r1Mat *A, r1Mat *B, double *Sinv ) {
    // Compute <a * a', S^-1 * b * b' * S^-1>
    double res = 0.0, *Ax = A->x, *Bx = B->x;
    DSDP_INT n = A->dim, i, j, *Ai = A->nzIdx, *Bi = B->nzIdx;
    for (i = 0; i < A->nnz; ++i) {
        for (j = 0; j < B->nnz; ++j) {
            res += Ax[Ai[i]] * Bx[Bi[j]] * Sinv[Ai[i] * n + Bi[j]];
        }
    }
    return (res * res * A->sign * B->sign);
}

extern double r1Sinvsps( spsMat *A, r1Mat *B, double *Sinv ) {
    // For convenience A is sparse and B is rank-one.
    double res = 0.0, tmp, bij, *Ax = A->x, *Bx = B->x;
    DSDP_INT k, p, q, in, jn, n = A->dim, *Ai = A->i, *Aj = A->cidx, *Bi = B->nzIdx;
    
    for (p = 0; p < B->nnz; ++p) {
        for (q = 0; q < p; ++q) {
            bij = Bx[Bi[p]] * Bx[Bi[q]]; in = Bi[p] * n; jn = Bi[q] * n; tmp = 0.0;
            for (k = 0; k < A->nnz; ++k) {
                if (Ai[k] == Aj[k]) {
                    tmp += Ax[k] * Sinv[in + Ai[k]] * Sinv[jn + Aj[k]];
                } else {
                    tmp += Ax[k] * Sinv[in + Ai[k]] * Sinv[jn + Aj[k]];
                    tmp += Ax[k] * Sinv[jn + Ai[k]] * Sinv[in + Aj[k]];
                }
            }
            res += bij * tmp;
        }
        q = p; bij = Bx[Bi[p]] * Bx[Bi[q]]; jn = in = Bi[p] * n; tmp = 0.0;
        for (k = 0; k < A->nnz; ++k) {
            if (Ai[k] == Aj[k]) {
                tmp += Ax[k] * Sinv[in + Ai[k]] * Sinv[jn + Aj[k]];
            } else {
                tmp += Ax[k] * Sinv[in + Ai[k]] * Sinv[jn + Aj[k]];
                tmp += Ax[k] * Sinv[jn + Ai[k]] * Sinv[in + Aj[k]];
            }
        }
        res += 0.5 * bij * tmp;
    }
    
    return (2.0 * res * B->sign);
}

extern DSDP_INT r1MatdiagTrace( r1Mat *x, double diag, double *trace ) {
    // Compute trace(a * a' * diag * I) = diag * norm(a)^2
    DSDP_INT retcode = DSDP_RETCODE_OK;
    if (diag == 0.0) { *trace = 0.0; return retcode; }
    double res = 0.0; retcode = r1MatFnorm(x, &res);
    *trace = (x->sign >= 0) ? diag * res : - diag * res;
    return retcode;
}

extern double r1MatFullTrace( r1Mat *x, double *S, double *aux ) {
    register double res = 0.0, *xdata = x->x;
    DSDP_INT i, j, n = x->dim, *xi = x->nzIdx;
    if (x->nnz <= n / 4) {
        for (i = 0; i < x->nnz; ++i) {
            for (j = 0; j < i; ++j) {
                res += xdata[xi[i]] * xdata[xi[j]] * S[n * xi[i] + xi[j]];
            }
            res += 0.5 * xdata[xi[j]] * xdata[xi[j]] * S[n * xi[j] + xi[j]];
        }
        res = 2.0 * res;
    } else {
        dsymv(&uplolow, &x->dim, &done, S, &x->dim,
              xdata, &one, &dzero, aux, &one);
        res = ddot(&x->dim, aux, &one, xdata, &one);
    }
    return res * x->sign;
}

extern DSDP_INT r1MatCountNnz( r1Mat *x ) {
    // Count the number of nonzero elements and setup nzIdx
    DSDP_INT nnz = 0, n = x->dim, i;
    
    if (!x->nzIdx) {
        x->nzIdx = (DSDP_INT *) calloc(n, sizeof(DSDP_INT));
    } else {
        memset(x->nzIdx, 0, sizeof(DSDP_INT) * n);
    }

    for (i = 0; i < n - 7; ++i) {
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;}
    }
    
    if (i < n - 3) {
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
    }
    
    if (i < n - 1) {
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;} ++i;
    }
    
    if (i < n) {
        if (x->x[i]) {x->nzIdx[nnz] = i; nnz += 1;}
    }
    
    x->nnz = nnz;
    assert( nnz >= 1 );
    return DSDP_RETCODE_OK;
}

extern DSDP_INT r1MatFree( r1Mat *x ) {
    // Free the allocated memory in vec structure
    if (x) {
        x->sign = 0; x->dim  = 0; x->nnz = 0;
        DSDP_FREE(x->x); DSDP_FREE(x->nzIdx);
    }
    return DSDP_RETCODE_OK;
}

extern DSDP_INT r1MatNormalize( r1Mat *x ) {
    
    double nrm = dnrm2(&x->dim, x->x, &one);
    drscl(&x->dim, &nrm, x->x, &one);
    x->sign = x->sign * nrm * nrm;
    return DSDP_RETCODE_OK;
}

extern DSDP_INT r1MatFnorm( r1Mat *x, double *fnrm ) {

    if (x->nnz < 0.6 * x->dim) {
        double res = 0.0, *xdata = x->x;
        DSDP_INT idx, i, *nzidx = x->nzIdx;
        for (i = 0; i < x->nnz; ++i) {
            idx = nzidx[i]; res += xdata[idx] * xdata[idx];
        }
        *fnrm = res * fabs(x->sign);
    } else {
        *fnrm = norm(&x->dim, x->x, &one);
        *fnrm = (*fnrm) * (*fnrm) * fabs(x->sign);
    }
    
    return DSDP_RETCODE_OK;
}

extern double r1MatOneNorm( r1Mat *x ) {
    
    double res = 0.0, *xdata = x->x;
    DSDP_INT i, j, *nzIdx = x->nzIdx;
    
    if (x->nnz < 0.6 * x->dim) {
        DSDP_INT nnz = x->nnz;
        for (i = 0; i < nnz; ++i) {
            for (j = 0; j < i; ++j) {
                res += fabs(xdata[nzIdx[i]] * xdata[nzIdx[j]]);
            }
            res += 0.5 * fabs(xdata[nzIdx[i]] * xdata[nzIdx[i]]);
        }
    } else {
        DSDP_INT n = x->dim;
        for (i = 0; i < n; ++i) {
            for (j = 0; j < i; ++j) {
                res += fabs(xdata[i] * xdata[j]);
            }
            res += 0.5 * fabs(xdata[i] * xdata[i]);
        }
    }
    
    return 2 * fabs(x->sign) * res;
}

extern DSDP_INT r1MatScale( r1Mat *x, double a ) {
    
    assert( (x->dim) );
    x->sign = x->sign * a;
    return DSDP_RETCODE_OK;
}

extern DSDP_INT r1MatRscale( r1Mat *x, double r ) {
    
    assert( (x->dim) && (r != 0.0) );
    x->sign = x->sign / r;
    return DSDP_RETCODE_OK;
}

extern void r1MatCheckSparsity( r1Mat *x, DSDP_INT *isdense, double thresh ) {
    if (x->nnz > thresh * x->dim) { *isdense = TRUE; }
}

extern void r1MatGetSymbolic( r1Mat *x, DSDP_INT *hash, DSDP_INT *firstNnz, DSDP_INT *nnzs ) {
    
    DSDP_INT i, j, n = x->dim;
    if (x->nzIdx[0] == 0) *firstNnz = TRUE;
    for (i = 0; i < x->nnz; ++i) {
        for (j = 0; j <= i; ++j) {
            if (packIdx(hash, n, x->nzIdx[i], x->nzIdx[j]) == 0) {
                packIdx(hash, n, x->nzIdx[i], x->nzIdx[j]) = 1; *nnzs = *nnzs + 1;
            }
        }
    }
}

extern DSDP_INT r1MatIsConstant( r1Mat *x ) {
    double *a = x->x, a1 = a[x->nzIdx[0]];
    for (DSDP_INT i = 1; i < x->nnz; ++i) {
        if (a[x->nzIdx[i]] != a1) return FALSE;
    }
    return TRUE;
}

extern DSDP_INT r1MatView( r1Mat *x ) {
    
    printf("Matrix View: \n");
    printf("Matrix dimension "ID" : \n", x->dim);
    for (DSDP_INT i = 0; i < x->dim; ++i) {
        printf("%10.3g, ", x->x[i] * x->sign);
    }
    
    printf("\n");
    return DSDP_RETCODE_OK;
}
