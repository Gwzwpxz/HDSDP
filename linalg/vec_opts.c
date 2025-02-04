#ifdef HEADERPATH
#include "external/lapack_names.h"
#include "interface/hdsdp_utils.h"
#include "linalg/vec_opts.h"
#else
#include "lapack_names.h"
#include "hdsdp_utils.h"
#include "vec_opts.h"
#endif

#include <math.h>

/* Blas functions */
extern double dnrm2( int *n, double *x, int *incx );
extern void daxpy( int *n, double *a, double *x, int *incx, double *y, int *incy );
extern double ddot( int *n, double *x, int *incx, double *y, int *incy );
extern void dscal( int *n, double *sa, double *sx, int *incx );
extern void drscl( int *n, double *sa, double *sx, int *incx );
extern void dsyr( char *uplo, int *n, double *alpha, double *x, int *incx, double *a, int *lda );
extern int idamax( int *n, double *x, int *incx );
extern double nrm2( int *n, double *x, int *incx ) {
#ifdef MYBLAS
    assert( *incx == 1 );
    
    double nrm = 0.0;
    
    for ( int i = 0; i < *n; ++i ) {
        nrm += x[i] * x[i];
    }
    
    return sqrt(nrm);
#else
    return dnrm2(n, x, incx);
#endif
}

extern void axpy( int *n, double *a, double *x, int *incx, double *y, int *incy ) {
#ifdef MYBLAS
    assert( *incx == 1 && *incy == 1 );
    
    for ( int i = 0; i < *n; ++i ) {
        y[i] += (*a) * x[i];
    }
#else
    daxpy(n, a, x, incx, y, incy);
#endif
    return;
}

extern void axpby( int *n, double *a, double *x, int *incx, double *b, double *y, int *incy ) {
    
    double aval = *a;
    double bval = *b;
    
    for ( int i = 0; i < *n; ++i ) {
        y[i] = aval * x[i] + bval * y[i];
    }
    
    return;
}

extern double dot( int *n, double *x, int *incx, double *y, int *incy ) {
#ifdef MYBLAS
    assert( *incx == 1 && *incy == 1 );
    
    double dres = 0.0;
    
    for ( int i = 0; i < *n; ++i ) {
        dres += x[i] * y[i];
    }
    
    return dres;
#else
    return ddot(n, x, incx, y, incy);
#endif
}

extern void scal( int *n, double *sa, double *sx, int *incx ) {
#ifdef MYBLAS
    assert( *incx == 1 );
    double a = *sa;
    
    if ( a == 1.0 ) {
        return;
    }
    
    for ( int i = 0; i < *n; ++i ) {
        sx[i] = sx[i] * a;
    }
#else
    dscal(n, sa, sx, incx);
#endif
    return;
}

/* Use standard Blas for this sensitive operation */
extern void rscl( int *n, double *sa, double *sx, int *incx ) {
#if 0
    assert( *incx == 1 );
    double a = *sa;
    
    assert( a != 0.0 );
    assert( a > 0.0 );
    
    if ( a == 1.0 ) {
        return;
    }
    
    if ( fabs(a) < 1e-16 ) {
        a = (a > 0) ? 1e-16 : -1e-16;
    }
    
    for ( int i = 0; i < *n; ++i ) {
        sx[i] = sx[i] / a;
    }
#else
    drscl(n, sa, sx, incx);
#endif
    return;
}

extern void syr( char *uplo, int *n, double *alpha, double *x, int *incx, double *a, int *lda ) {
    
    dsyr(uplo, n, alpha, x, incx, a, lda);
    
    return;
}

extern int idamax( int *n, double *x, int *incx ) {
    
    int idmax = 0;
    double damax = 0.0;
    
    for ( int i = 0; i < *n; ++i ) {
        double ax = fabs(x[i]);
        if ( ax > damax ) {
            damax = ax; idmax = i;
        }
    }
    
    return idmax;
}

extern int idmin( int *n, double *x, int *incx ) {
    
    int imin = 0;
    double dmin = HDSDP_INFINITY;
    
    for ( int i = 0; i < *n; ++i ) {
        double dx = x[i];
        if ( dx < dmin ) {
            dmin = dx; imin = i;
        }
    }
    
    return imin;
}


extern double sumlogdet( int *n, double *x ) {
    
    double logdet = 0.0;
    for ( int i = 0; i < *n; ++i ) {
        logdet += log(x[i]);
    }
    
    return logdet;
}

extern void vvscl( int *n, double *s, double *x ) {
    
    for ( int i = 0; i < *n; ++i ) {
        x[i] = x[i] * s[i];
    }
    
    return;
}

extern void vvrscl( int *n, double *s, double *x ) {
    
    for ( int i = 0; i < *n; ++i ) {
        x[i] = x[i] / s[i];
    }
    
    return;
}

extern double nrm1( int *n, double *x, int *incx ) {

    assert( *incx == 1 );
    
    double nrm = 0.0;
    
    for ( int i = 0; i < *n; ++i ) {
        nrm += fabs(x[i]);
    }
    
    return nrm;
}

extern double normalize( int *n, double *a ) {
    
    double norm = nrm2(n, a, &HIntConstantOne);
    
    if ( norm > 0.0 ) {
        drscl(n, &norm, a, &HIntConstantOne);
    } else {
        norm = 0.0;
        HDSDP_ZERO(a, double, *n);
    }
    
    return norm;
}
