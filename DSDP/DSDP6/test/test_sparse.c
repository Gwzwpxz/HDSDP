#include "test.h"
static char etype[] = "Test sparse";
/* CSC data
 
 10.0000         0         0         0         0   -2.1321         0         0         0   -0.5607
       0   10.0000   -0.5046   -0.1768    2.1778    1.1454   -2.4969         0    0.0479    0.0173
       0   -0.5046   10.0000   -1.4286         0    0.1370   -0.7629         0         0    0.8257
       0   -0.1768   -1.4286    9.8528         0         0         0   -2.1237    1.1385         0
       0    2.1778         0         0    9.8681   -0.6428         0         0         0         0
 -2.1321    1.1454    0.1370         0   -0.6428   10.0000         0   -1.0149         0         0
       0   -2.4969   -0.7629         0         0         0   11.0078         0   -1.2038         0
       0         0         0   -2.1237         0   -1.0149         0   10.0000         0   -0.9300
       0    0.0479         0    1.1385         0         0   -1.2038         0   10.0000   -0.6291
 -0.5607    0.0173    0.8257         0         0         0         0   -0.9300   -0.6291   10.0000
 
 */

/* rhs =
    -0.9087
    -0.2099
    -1.6989
     0.6076
    -0.1178
     0.6992
     0.2696
     0.4943
    -1.4831
    -1.0203
 
  sol =
    -0.0825
    -0.0374
    -0.1554
     0.0707
     0.0005
     0.0650
    -0.0127
     0.0619
    -0.1639
    -0.0983
 
 B =

 0         0         0         0   -0.6568         0         0         0         0    1.2616
 0         0         0         0         0         0    0.4754         0         0         0
 0         0   -0.3373         0         0    0.1555         0         0   -1.4814         0
 0         0         0         0         0         0         0         0         0         0
-0.6568    0         0         0         0         0    0.8842         0         0         0
 0         0    0.1555         0         0         0    0.1269         0         0         0
 0    0.4754         0         0    0.8842    0.1269         0         0         0         0
 0         0         0         0         0         0         0         0         0    1.1287
 0         0   -1.4814         0         0         0         0         0         0         0
1.2616     0         0         0         0         0         0    1.1287         0         0
 
 A \ B
 
 0.0064   -0.0004    0.0036         0   -0.0699   -0.0003    0.0033    0.0071    0.0015    0.1363
 0.0160    0.0128   -0.0087         0    0.0260    0.0047    0.0292   -0.0012   -0.0127   -0.0056
-0.0094    0.0043   -0.0336         0    0.0086    0.0173    0.0019   -0.0096   -0.1543    0.0013
 0.0005    0.0002    0.0135         0    0.0000    0.0025    0.0011    0.0001   -0.0236    0.0272
-0.0703   -0.0029    0.0031         0   -0.0070   -0.0011    0.0842    0.0005    0.0030    0.0040
-0.0037   -0.0018    0.0184         0   -0.0187   -0.0009    0.0156    0.0029    0.0037    0.0426
 0.0039    0.0470   -0.0210         0    0.0879    0.0139    0.0068   -0.0001   -0.0133   -0.0014
 0.0117   -0.0002    0.0042         0   -0.0023    0.0003    0.0018    0.0111   -0.0034    0.1247
 0.0084    0.0056   -0.1526         0    0.0102    0.0013    0.0006    0.0072    0.0019   -0.0020
 0.1289   -0.0001   -0.0062         0   -0.0042   -0.0013    0.0002    0.1156    0.0126    0.0190
 
 Ainv * B * Ainv
 
 0.0014    0.0018   -0.0006    0.0003   -0.0075   -0.0002    0.0008    0.0020    0.0011    0.0140
 0.0018    0.0013   -0.0006    0.0000    0.0024    0.0008    0.0028   -0.0001   -0.0010   -0.0005
-0.0006   -0.0006   -0.0033    0.0012    0.0011    0.0017   -0.0019   -0.0006   -0.0158   -0.0007
 0.0003    0.0000    0.0012    0.0005    0.0000    0.0003   -0.0001    0.0004   -0.0023    0.0025
-0.0075    0.0024    0.0011    0.0000   -0.0014   -0.0021    0.0084   -0.0002    0.0013   -0.0000
-0.0002    0.0008    0.0017    0.0003   -0.0021   -0.0003    0.0018    0.0007    0.0008    0.0042
 0.0008    0.0028   -0.0019   -0.0001    0.0084    0.0018    0.0010    0.0002   -0.0012   -0.0000
 0.0020   -0.0001   -0.0006    0.0004   -0.0002    0.0007    0.0002    0.0025    0.0004    0.0129
 0.0011   -0.0010   -0.0158   -0.0023    0.0013    0.0008   -0.0012    0.0004    0.0004    0.0012
 0.0140   -0.0005   -0.0007    0.0025   -0.0000    0.0042   -0.0000    0.0129    0.0012    0.0040
 
 */

DSDP_INT spsAdim = 10;
DSDP_INT Ap[] = {0,  3, 11, 17, 22, 25, 31, 35, 39, 44, 50};
DSDP_INT Ai[] = {0, 5, 9, 1, 2, 3, 4, 5, 6, 8, 9, 1, 2, 3,
                 5, 6, 9, 1, 2, 3, 7, 8, 1, 4, 5, 0, 1, 2,
                 4, 5, 7, 1, 2, 6, 8, 3, 5, 7, 9, 1, 3, 6,
                 8, 9, 0, 1, 2, 7, 8, 9};
double Ax[] = {10.        , -2.1320946 , -0.560665  , 10.        , -0.50458641,
               -0.17683027,  2.17777871,  1.14536171, -2.4968865 ,  0.04787387,
                0.01734614, -0.50458641, 10.        , -1.42864686,  0.13702487,
               -0.76293329,  0.82572715, -0.17683027, -1.42864686,  9.85279854,
               -2.12365546,  1.13846539,  2.17777871,  9.86806213, -0.64277281,
               -2.1320946 ,  1.14536171,  0.13702487, -0.64277281, 10.        ,
               -1.01494364, -2.4968865 , -0.76293329, 11.00777341, -1.20384997,
               -2.12365546, -1.01494364, 10.        , -0.92996156,  0.04787387,
                1.13846539, -1.20384997, 10.        , -0.62909076, -0.560665  ,
                0.01734614,  0.82572715, -0.92996156, -0.62909076, 10.        };

DSDP_INT packAp[] = {0,  3, 11, 16, 19, 21, 23, 25, 27, 29, 30};
DSDP_INT packAi[] = {0, 5, 9, 1, 2, 3, 4, 5, 6, 8, 9, 2, 3, 5,
                     6, 9, 3, 7, 8, 4, 5, 5, 7, 6, 8, 7, 9, 8, 9, 9};
double packAx[]  =  {10.        , -2.1320946 , -0.560665  , 10.        , -0.50458641,
                     -0.17683027,  2.17777871,  1.14536171, -2.4968865 ,  0.04787387,
                      0.01734614, 10.        , -1.42864686,  0.13702487, -0.76293329,
                      0.82572715,  9.85279854, -2.12365546,  1.13846539,  9.86806213,
                     -0.64277281, 10.        , -1.01494364, 11.00777341, -1.20384997,
                     10.        , -0.92996156, 10.        , -0.62909076, 10.        };

DSDP_INT packBp[] = {0, 2, 3, 6, 6, 7, 8, 8, 9, 9, 9};
DSDP_INT packBi[] = {4, 9, 6, 2, 5, 8, 6, 6, 9};
double packBx[]   = {-0.65681593,  1.26155072,  0.47542481, -0.33733642,  0.155489,
                     -1.48139907,  0.88415371,  0.12694707,  1.12873645};

double AinvB[] = {0.0064, 0.0160, -0.0094, 0.0005, -0.0703, -0.0037, 0.0039, 0.0117, 0.0084, 0.1289,
                 -0.0004, 0.0128, 0.0043, 0.0002, -0.0029, -0.0018, 0.0470, -0.0002, 0.0056, -0.0001,
                  0.0036, -0.0087, -0.0336, 0.0135, 0.0031, 0.0184, -0.0210, 0.0042, -0.1526, -0.0062,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 -0.0699, 0.0260, 0.0086, 0.0000, -0.0070, -0.0187, 0.0879, -0.0023, 0.0102, -0.0042,
                 -0.0003, 0.0047, 0.0173, 0.0025, -0.0011, -0.0009, 0.0139, 0.0003, 0.0013, -0.0013,
                  0.0033, 0.0292, 0.0019, 0.0011, 0.0842, 0.0156, 0.0068, 0.0018, 0.0006, 0.0002,
                  0.0071, -0.0012, -0.0096, 0.0001, 0.0005, 0.0029, -0.0001, 0.0111, 0.0072, 0.1156,
                  0.0015, -0.0127, -0.1543, -0.0236, 0.0030, 0.0037, -0.0133, -0.0034, 0.0019, 0.0126,
                  0.1363, -0.0056, 0.0013, 0.0272, 0.0040, 0.0426, -0.0014, 0.1247, -0.0020, 0.0190};

double AinvBAinv[] = {0.0014, 0.0018, -0.0006, 0.0003, -0.0075, -0.0002, 0.0008, 0.0020, 0.0011,
                      0.0140, 0.0013, -0.0006, 0.0000, 0.0024, 0.0008, 0.0028, -0.0001, -0.0010,
                     -0.0005, -0.0033, 0.0012, 0.0011, 0.0017, -0.0019, -0.0006, -0.0158, -0.0007,
                      0.0005, 0.0000, 0.0003, -0.0001, 0.0004, -0.0023, 0.0025, -0.0014, -0.0021,
                      0.0084, -0.0002, 0.0013, -0.0000, -0.0003, 0.0018, 0.0007, 0.0008, 0.0042,
                      0.0010, 0.0002, -0.0012, -0.0000, 0.0025, 0.0004, 0.0129, 0.0004, 0.0012, 0.0040};

double b[] = {-0.9087, -0.2099, -1.6989, 0.6076, -0.1178,
               0.6992, 0.2696, 0.4943, -1.4831, -1.0203};
double spsol[] = {-0.0825, -0.0374, -0.1554, 0.0707, 0.0005,
               0.0650, -0.0127, 0.0619, -0.1639, -0.0983};

double spfnrm = 32.347300418814825;
double refeigmax = 13.943109380013954;
double refeigmin = 6.085048173386250;

DSDP_INT test_sparse(void) {
    // Test the sparse routines
    
    DSDP_INT retcode  = DSDP_RETCODE_OK;
    DSDP_INT packAnnz = packAp[spsAdim];
    DSDP_INT packBnnz = packBp[spsAdim];
    
    spsMat *data  = NULL;
    spsMat *data2 = NULL;
    spsMat *B     = NULL;
    vec *rhs      = NULL;
    vec *mysol    = NULL;
    double *fullSol = NULL;
    dsMat *dataInvBdataInv = NULL;
    
    data    = (spsMat *) calloc(1, sizeof(spsMat));
    data2   = (spsMat *) calloc(1, sizeof(spsMat));
    B       = (spsMat *) calloc(1, sizeof(spsMat));
    rhs     = (vec *) calloc(1, sizeof(vec));
    mysol   = (vec *) calloc(1, sizeof(vec));
    dataInvBdataInv = (dsMat *) calloc(1, sizeof(dsMat));
    fullSol = (double *) calloc(spsAdim * spsAdim, sizeof(double));

    retcode = vec_init(rhs);
    retcode = vec_alloc(rhs, spsAdim);
    retcode = vec_init(mysol);
    retcode = vec_alloc(mysol, spsAdim); checkCodeFree;
    
    retcode = spsMatInit(data); checkCodeFree;
    retcode = spsMatAllocData(data, spsAdim, packAnnz);
    retcode = spsMatInit(data2); checkCodeFree;
    retcode = spsMatAllocData(data2, spsAdim, packAnnz);
    retcode = spsMatInit(B); checkCodeFree;
    retcode = spsMatAllocData(B, spsAdim, packBnnz);
    
    retcode = denseMatInit(dataInvBdataInv);
    retcode = denseMatAlloc(dataInvBdataInv, spsAdim, FALSE);
    
    memcpy(data->p, packAp, sizeof(DSDP_INT) * (spsAdim + 1));
    memcpy(data->i, packAi, sizeof(DSDP_INT) * packAnnz);
    memcpy(data->x, packAx, sizeof(double)   * packAnnz);
    
    memcpy(data2->p, packAp, sizeof(DSDP_INT) * (spsAdim + 1));
    memcpy(data2->i, packAi, sizeof(DSDP_INT) * packAnnz);
    memcpy(data2->x, packAx, sizeof(double)   * packAnnz);
    
    memcpy(B->p, packBp, sizeof(DSDP_INT) * (spsAdim + 1));
    memcpy(B->i, packBi, sizeof(DSDP_INT) * packBnnz);
    memcpy(B->x, packBx, sizeof(double)   * packBnnz);
    
    double diff = 0.0;
    double err  = 0.0;
    
    // Rscale
    double r = 0.5;
    retcode = spsMatRscale(data, r); checkCodeFree;
    
    // Fnorm
    double myfnrm = 0.0;
    retcode = spsMatFnorm(data, &myfnrm); checkCodeFree;
    
    if (fabs(myfnrm * r - spfnrm) < 1e-08) {
        passed("Rscale");
        passed("Fnorm");
    }
    
    // Scatter
    for (DSDP_INT i = 0; i < spsAdim; ++i) {
        retcode = spsMatScatter(data, rhs, i); checkCodeFree;
        vec_print(rhs);
    }
    
    // Factorization
    retcode = spsMatSymbolic(data); checkCodeFree;
    retcode = spsMatFactorize(data); checkCodeFree;
    
    // Vector solve
    memcpy(rhs->x, b, sizeof(double) * spsAdim);
    retcode = spsMatVecSolve(data, rhs, mysol->x); checkCodeFree;
    
    for (DSDP_INT i = 0; i < spsAdim; ++i) {
        diff = mysol->x[i] - spsol[i] * r;
        err += diff * diff;
    }
    
    if (sqrt(err) < 1e-04) {
        passed("Vector solve");
    }
    
    // Sp-Sp solve
    err = 0.0;
    retcode = spsMatSpSolve(data, B, fullSol); checkCodeFree;
    
    for (DSDP_INT i = 0; i < spsAdim * spsAdim; ++i) {
        diff = fullSol[i] - AinvB[i] * r;
        err += diff * diff;
    }
    
    if (sqrt(err) < 1e-08) {
        passed("Sp-Sp solve");
    }
    
    // SinvSpSinv
    err = 0.0;
    double asinv = 0.0;
    retcode = spsSinvSpSinvSolve(data, B, dataInvBdataInv, &asinv);
    
    for (DSDP_INT i = 0; i < nsym(spsAdim); ++i) {
        diff = dataInvBdataInv->array[i] - AinvBAinv[i] * r * r;
        err += diff * diff;
    }
    
    if (sqrt(err) < 1e-04) {
        passed("Spinv Sp Spinv");
    }
    
    // Hash sum
    retcode = spsMatAllocSumMat(data);
    DSDP_INT idx = 0;
    for (DSDP_INT i = 0; i < data->dim; ++i) {
        for (DSDP_INT j = data->p[i]; j < data->p[i + 1]; ++j) {
            packIdx(data->sumHash, data->dim, data->i[j], i) = idx;
            idx += 1;
        }
    }
    
    retcode = spsMataXpbY(2.0, data2, -1.0, data); checkCodeFree;
    retcode = spsMatFnorm(data, &myfnrm); checkCodeFree;
    
    if (myfnrm < 1e-15) {
        passed("Hash sum");
    }
    
    // Eigen value routines
    double maxEig = 0.0;
    double minEig = 0.0;
    
    retcode = spsMatMaxEig(data2, &maxEig);
    retcode = spsMatMinEig(data2, &minEig);
    
    if (fabs(refeigmax - maxEig) < 1e-06) {
        passed("Maximum eigenvalue");
    }
    
    if (fabs(refeigmin - minEig) < 1e-06) {
        passed("Minimum eigenvalue");
    }
    
    // Check positive-definiteness
    retcode = spsMatSymbolic(data2); checkCodeFree;
    DSDP_INT ispd = FALSE;
    spsMatIspd(data, &ispd);
    if (!ispd) {
        passed("PSD check 1");
    }
    spsMatIspd(data2, &ispd);
    if (ispd) {
        passed("PSD check 2");
    }
    
    // Take SDP step
    spsMatLspLSolve(data2, B, data);
    
clean_up:
    
    spsMatFree(data);
    DSDP_FREE(data);
    
    spsMatFree(data2);
    DSDP_FREE(data2);
    
    spsMatFree(B);
    DSDP_FREE(B);
    
    denseMatFree(dataInvBdataInv);
    DSDP_FREE(dataInvBdataInv);
    
    vec_free(rhs);
    DSDP_FREE(rhs);
    
    vec_free(mysol);
    DSDP_FREE(mysol);
    
    DSDP_FREE(fullSol);
    
    return retcode;
}
