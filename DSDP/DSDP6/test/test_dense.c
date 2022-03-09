#include <time.h>
#include "test.h"
#include "dsdpcg.h"
static char etype[] = "Test dense";
/* Packed data
 
 145.0902  107.4532   -2.4046   -0.6691  -11.3401    9.7020  137.9264  188.1493   22.3598   81.2420
 107.4532  495.2369   32.9430   25.1972    4.0607    2.8208  190.7778  122.7086   80.0744   -2.2327
  -2.4046   32.9430   29.9940   11.3035   14.2333   12.8114   -0.2192    1.9181   23.7690   -2.8766
  -0.6691   25.1972   11.3035   87.7535   71.1435    5.6926    4.3115    5.4869    3.2918   -0.2061
 -11.3401    4.0607   14.2333   71.1435  254.9858   44.6697    1.8421   43.1823    0.0087    7.7144
   9.7020    2.8208   12.8114    5.6926   44.6697   70.8160    3.0342   67.2678    1.4728   16.5201
 137.9264  190.7778   -0.2192    4.3115    1.8421    3.0342  172.6808  188.3668   24.0036   41.3076
 188.1493  122.7086    1.9181    5.4869   43.1823   67.2678  188.3668  406.5596    7.6074  214.3802
  22.3598   80.0744   23.7690    3.2918    0.0087    1.4728   24.0036    7.6074   61.4641  -22.6640
  81.2420   -2.2327   -2.8766   -0.2061    7.7144   16.5201   41.3076  214.3802  -22.6640  349.4797

*/
static void fastTranspose( double *A, double *AT, DSDP_INT row,
                           DSDP_INT col, DSDP_INT m, DSDP_INT n, DSDP_INT N ) {
    
    // Cache oblivious transposition of A matrix
    // Adapted from https://github.com/iainkfraser/cache_transpose
    if (m > 4 || n > 4) {
        if (n >= m) {
            DSDP_INT half = n / 2;
            if (n % 2 == 0) {
                fastTranspose(A, AT, row, col, m, half, N);
                fastTranspose(A, AT, row, col + half, m, half, N);
            } else {
                fastTranspose(A, AT, row, col, m, half, N);
                fastTranspose(A, AT, row, col + half, m, n - half, N);
            }
            
        } else {
            DSDP_INT half = m / 2;
            if (m % 2 == 0) {
                fastTranspose(A, AT, row, col, half, n, N);
                fastTranspose(A, AT, row + half, col, half, n, N);
            } else {
                fastTranspose(A, AT, row, col, half, n, N);
                fastTranspose(A, AT, row + half, col, m - half, n, N);
            }
        }
    } else {
        DSDP_INT r = row + m;
        DSDP_INT c = col + n;
        for (DSDP_INT i = row; i < r; ++i) {
            for (DSDP_INT j = col; j < c; ++j) {
                AT[N * j + i] = A[N * i + j];
            }
        }
    }
}

static void fastTranspose2( double *A, DSDP_INT row, DSDP_INT col, DSDP_INT m, DSDP_INT n, DSDP_INT N ) {
    
    // Cache oblivious transposition of A matrix
    // Adapted from https://github.com/iainkfraser/cache_transpose
    if (m > 4 || n > 4) {
        if (n >= m) {
            DSDP_INT half = n / 2;
            fastTranspose2(A, row, col, m, half, N);
            fastTranspose2(A, row, col + half, m, half, N);
            
        } else {
            DSDP_INT half = m / 2;
            fastTranspose2(A, row, col, half, n, N);
            if (m % 2 == 0) {
                fastTranspose2(A, row + half, col, half, n, N);
            } else {
                fastTranspose2(A, row + half, col, m - half, n, N);
            }
        }
    } else {
        double tmp = 0.0;
        DSDP_INT r = row + m;
        DSDP_INT c = col + n;
        for (DSDP_INT i = row; i < r; ++i) {
            for (DSDP_INT j = col; j < c; ++j) {
                tmp = A[N * j + i];
                A[N * i + j] = A[N * j + i];
                A[N * j + i] = tmp;
            }
        }
    }
}

static void slowTranspose( double *A, double *AT, DSDP_INT m, DSDP_INT n ) {
    double tmp = 0.0;
    for (DSDP_INT i = 0; i < n; ++i) {
        for (DSDP_INT j = 0; j <= i; ++j) {
            tmp = A[i * n + j];
            A[i * n + j] = A[j * n + i];
            A[j * n + i] = tmp;
        }
    }
}

DSDP_INT packAdim = 10;
double packA[] = {145.0902, 107.4532, -2.4046, -0.6691, -11.3401, 9.7020, 137.9264,
                  188.1493, 22.3598, 81.2420, 495.2369, 32.9430, 25.1972, 4.0607,
                  2.8208, 190.7778, 122.7086, 80.0744, -2.2327, 29.9940, 11.3035,
                  14.2333, 12.8114, -0.2192, 1.9181, 23.7690, -2.8766, 87.7535,
                  71.1435, 5.6926, 4.3115, 5.4869, 3.2918, -0.2061, 254.9858, 44.6697,
                  1.8421, 43.1823, 0.0087, 7.7144, 70.8160, 3.0342, 67.2678, 1.4728,
                  16.5201, 172.6808, 188.3668, 24.0036, 41.3076, 406.5596, 7.6074,
                  214.3802, 61.4641, -22.6640, 349.4797
};

/* RHS and solution array
 
rhs =
 
 0.9619
 0.0046
 0.7749
 0.8173
 0.8687
 0.0844
 0.3998
 0.2599
 0.8001
 0.4314
 
sol =
 
 0.0241
-0.0042
 0.0293
 0.0051
 0.0033
-0.0011
-0.0027
-0.0094
 0.0012
 0.0020
 
 RHSB =
 
 1.3813   -5.6445  -11.1421   -3.9193   -6.3964   -7.7247  -10.1275  -13.3424  -10.1687  -11.3438
-5.6445   -3.3020  -11.2949   -8.2102   -7.9926   -9.8511  -12.0385  -12.1523   -6.6849  -10.2090
-11.1421  -11.2949   -0.7583  -10.8567   -9.0654   -5.7088  -12.0104  -10.5294  -10.5114  -14.7401
-3.9193   -8.2102  -10.8567    4.0027   -4.8024   -8.7332  -11.4535   -8.9209  -12.4497  -15.8803
-6.3964   -7.9926   -9.0654   -4.8024    0.4790   -6.2991   -8.6763   -8.5075  -13.0240  -15.3705
-7.7247   -9.8511   -5.7088   -8.7332   -6.2991   -1.9128  -10.1878  -11.8567   -9.3978  -10.5077
-10.1275  -12.0385  -12.0104  -11.4535   -8.6763  -10.1878   -6.1471   -9.8811  -11.5331  -10.0475
-13.3424  -12.1523  -10.5294   -8.9209   -8.5075  -11.8567   -9.8811   -6.2374  -13.3602  -11.9546
-10.1687   -6.6849  -10.5114  -12.4497  -13.0240   -9.3978  -11.5331  -13.3602   -3.8410   -6.3700
-11.3438  -10.2090  -14.7401  -15.8803  -15.3705  -10.5077  -10.0475  -11.9546   -6.3700    0.2507
 
 Bsol =
 
 0.8419    0.4879    0.2810    0.6434    0.4119    0.2465   -0.1075   -0.0085    0.0400   -0.1962
 0.2729    0.2401    0.1495    0.2635    0.1949    0.1153    0.0539    0.1328    0.1113    0.0533
-0.2002   -0.3258    0.3567   -0.1920   -0.0788   -0.0346   -0.3039   -0.1582   -0.4122   -0.6212
-0.0764   -0.1106   -0.1862    0.0453   -0.1040   -0.1183   -0.1119   -0.1021   -0.1128   -0.1179
 0.0783    0.0648    0.0366    0.0354    0.0774    0.0287    0.0211    0.0239    0.0141   -0.0019
-0.3022   -0.3456   -0.3547   -0.3994   -0.3140   -0.1111   -0.1917   -0.3990   -0.1938   -0.1038
-1.1090   -0.9252   -0.6121   -1.0943   -0.7395   -0.4227   -0.1301   -0.4761   -0.3879   -0.0997
 0.1756    0.2583    0.2517    0.3210    0.2346    0.0970    0.1432    0.3324    0.1673    0.1086
-0.4077   -0.1999   -0.4530   -0.4108   -0.3889   -0.2573   -0.0854   -0.2316    0.0512    0.1739
-0.2186   -0.1910   -0.1992   -0.2710   -0.2078   -0.1086   -0.0754   -0.1770   -0.0749    0.0029
 
 0.8 * A - 0.2 * B
 
 115.7959   87.0915    0.3047    0.2486   -7.7928    9.3065  112.3666  153.1879   19.9216   67.2624
  87.0915  396.8499   28.6134   21.7998    4.8471    4.2269  155.0299  100.5973   65.3965    0.2556
   0.3047   28.6134   24.1469   11.2141   13.1997   11.3909    2.2267    3.6404   21.1175    0.6467
   0.2486   21.7998   11.2141   69.4023   57.8753    6.3007    5.7399    6.1737    5.1234    3.0112
  -7.7928    4.8471   13.1997   57.8753  203.8928   36.9956    3.2089   36.2473    2.6118    9.2456
   9.3065    4.2269   11.3909    6.3007   36.9956   57.0354    4.4649   56.1856    3.0578   15.3176
 112.3666  155.0299    2.2267    5.7399    3.2089    4.4649  139.3741  152.6697   21.5095   35.0556
 153.1879  100.5973    3.6404    6.1737   36.2473   56.1856  152.6697  326.4952    8.7580  173.8951
  19.9216   65.3965   21.1175    5.1234    2.6118    3.0578   21.5095    8.7580   49.9395  -16.8572
  67.2624    0.2556    0.6467    3.0112    9.2456   15.3176   35.0556  173.8951  -16.8572  279.5336
 
 */
double rhs[] = {0.9619,  0.0046, 0.7749, 0.8173, 0.8687, 0.0844, 0.3998,  0.2599, 0.8001, 0.4314};
double sol[] = {0.0241, -0.0042, 0.0293, 0.0051, 0.0033, -0.0011, -0.0027, -0.0094, 0.0012, 0.0020};
double diag[] = {145.0902, 495.2369, 29.9940, 87.7535, 254.9858, 70.8160, 172.6808, 406.5596, 61.4641, 349.4797};
double fnrm  = 1.063155486894704e+03;
double cholL[] = {12.0453, 8.9207, -0.1996, -0.0556, -0.9415, 0.8055, 11.4506, 15.6201, 1.8563,
                  6.7447, 20.3877, 1.7032, 1.2602, 0.6111, -0.2141, 4.3472, -0.8159, 3.1154,
                  -3.0607, 5.2013, 1.7584, 2.5003, 2.5641, -1.0262, 1.2354, 3.6210, 0.7080,
                  9.1143, 7.2331, 0.1644, 0.1397, 0.5717, -0.7569, 0.3051, 13.9699, 2.7172,
                  0.8247, 3.6624, -0.2667, 0.8560, 7.4924, -0.6527, 5.5121, -1.0398, 0.8330,
                  4.5263, 3.2742, -1.6421, -4.8817, 10.2732, -1.0515, 11.0714, 5.4474, -4.2703,
                  11.3118};

double BRHS[] = {1.3813, -5.6445, -11.1421, -3.9193, -6.3964, -7.7247, -10.1275, -13.3424, -10.1687,
                 -11.3438, -3.3020, -11.2949, -8.2102, -7.9926, -9.8511, -12.0385, -12.1523, -6.6849,
                 -10.2090, -0.7583, -10.8567, -9.0654, -5.7088, -12.0104, -10.5294, -10.5114, -14.7401,
                 4.0027, -4.8024, -8.7332, -11.4535, -8.9209, -12.4497, -15.8803, 0.4790, -6.2991,
                 -8.6763, -8.5075, -13.0240, -15.3705, -1.9128, -10.1878, -11.8567, -9.3978, -10.5077,
                 -6.1471, -9.8811, -11.5331, -10.0475, -6.2374, -13.3602, -11.9546, -3.8410, -6.3700,
                 0.2507};

double Bsol[] = {0.841, 0.487, 0.281, 0.643, 0.411, 0.246, -0.107, -0.008, 0.040, -0.1962, 0.272,
    0.240, 0.149, 0.263, 0.194, 0.115, 0.053, 0.132, 0.111, 0.0533, -0.200, -0.325, 0.356, -0.192,
    -0.078, -0.034, -0.303, -0.158, -0.412, -0.6212, -0.076, -0.110, -0.186, 0.045, -0.104, -0.118,
    -0.111, -0.102, -0.112, -0.1179, 0.078, 0.064, 0.036, 0.035, 0.077, 0.028, 0.021, 0.023, 0.014,
    -0.0019, -0.302, -0.345, -0.354, -0.399, -0.314, -0.111, -0.191, -0.399, -0.193, -0.1038, -1.109,
    -0.925, -0.612, -1.094, -0.739, -0.422, -0.130, -0.476, -0.387, -0.0997, 0.175, 0.258, 0.251,
    0.321, 0.234, 0.097, 0.143, 0.332, 0.167, 0.1086, -0.407, -0.199, -0.453, -0.410, -0.388, -0.257,
    -0.085, -0.231, 0.051, 0.1739, -0.218, -0.191, -0.199, -0.271, -0.207, -0.108, -0.075, -0.177,
    -0.074, 0.0029};

double aApbB[] = {115.7959, 87.0915, 0.3047, 0.2486, -7.7928, 9.3065, 112.3666, 153.1879, 19.9216,
    67.2624, 396.8499, 28.6134, 21.7998, 4.8471, 4.2269, 155.0299, 100.5973, 65.3965, 0.2556,
    24.1469, 11.2141, 13.1997, 11.3909, 2.2267, 3.6404, 21.1175, 0.6467, 69.4023, 57.8753,
    6.3007, 5.7399, 6.1737, 5.1234, 3.0112, 203.8928, 36.9956, 3.2089, 36.2473, 2.6118,
    9.2456, 57.0354, 4.4649, 56.1856, 3.0578, 15.3176, 139.3741, 152.6697, 21.5095, 35.0556,
    326.4952, 8.7580, 173.8951, 49.9395, -16.8572, 279.5336};

/* Utility functions */
DSDP_INT genDenseMatfromVec1( double *x, dsMat *A );
DSDP_INT genDenseMatfromArray( double *array, dsMat *A );

DSDP_INT test_dense(void) {
    // Test the dense packed routines of DSDP
    DSDP_INT retcode = DSDP_RETCODE_OK;
    
    dsMat *data = NULL;
    dsMat *B = NULL;
    double *mysol = NULL;
    vec *vecrhs = NULL;
    double *fullA = NULL;
    double *mat = NULL;
    double *mat2 = NULL;
    
    double myfnorm = 0.0;
    data = (dsMat *) calloc(1, sizeof(dsMat));
    retcode = denseMatAlloc(data, packAdim, TRUE); checkCodeFree;
    retcode = genDenseMatfromArray(packA, data); checkCodeFree;
    retcode = denseMatView(data);
    
    /* Frobenius norm */
    retcode = denseMatFnorm(data, &myfnorm);
    if (fabs(myfnorm - fnrm) / fnrm < 1e-08) {
        passed("Fnorm");
    } else {
        retcode = DSDP_RETCODE_FAILED;
    }
    
    checkCodeFree;
    
    /* Reciprocical scaling */
    double rscale = 10.0;
    retcode = denseMatRscale(data, rscale); checkCodeFree;
    retcode = denseMatFnorm(data, &myfnorm);
    
    if (fabs(myfnorm - fnrm / 10.0) / fnrm < 1e-08) {
        passed("Rscale");
    } else {
        retcode = DSDP_RETCODE_FAILED;
    }
    
    checkCodeFree;
    
    mysol = (double *) calloc(packAdim, sizeof(double));
    vecrhs = (vec *) calloc(1, sizeof(vec));
    retcode = vec_init(vecrhs); checkCodeFree;
    retcode = vec_alloc(vecrhs, packAdim); checkCodeFree;
    
    /* Get diagonal */
    denseMatGetdiag(data, vecrhs); checkCodeFree;
    
    /* Conjugate gradient */
    vec *cgsol = (vec *) calloc(1, sizeof(vec));
    vec_alloc(cgsol, packAdim);

    CGSolver *cgsolver = NULL;
    cgsolver = (CGSolver *) calloc(1, sizeof(CGSolver));
    dsdpCGInit(cgsolver);
    dsdpCGAlloc(cgsolver, packAdim);
    dsdpCGSetM(cgsolver, data);
    dsdpCGSetMaxIter(cgsolver, 100);
    dsdpCGSetTol(cgsolver, 1e-06);
    dsdpCGSetDPre(cgsolver, vecrhs);
    dsdpCGSolve(cgsolver, vecrhs, cgsol);
    
    vec_free(cgsol);
    DSDP_FREE(cgsol);
    dsdpCGFree(cgsolver);
    DSDP_FREE(cgsolver);
    
    
    DSDP_INT npassed = 0;
    for (DSDP_INT i = 0; i < packAdim; ++i) {
        if (fabs(vecrhs->x[i] - diag[i] / 10) / diag[i] < 1e-08) {
            npassed += 1;
        }
    }
    
    if (npassed == packAdim) {
        passed("Get diagonal");
    }
    
    /* Fill */
    fullA = (double *) calloc(packAdim * packAdim, sizeof(double));
    retcode = denseMatFill(data, fullA);
    
    for (DSDP_INT i = 0; i < packAdim; ++i) {
        for (DSDP_INT j = 0; j < packAdim; ++j) {
            printf("%-10.3g ", fullA[i * packAdim + j] * 10);
        }
        printf("\n");
    }
    
    /* Scatter */
    for (DSDP_INT k = 0; k < packAdim; ++k) {
        retcode = denseMatScatter(data, vecrhs, k); checkCodeFree;
        vec_print(vecrhs);
    }
    
    /* Factorize */
    retcode = denseMatRscale(data, 0.1); checkCodeFree;
    retcode = denseMatFactorize(data); checkCodeFree;
    double err = 0.0;
    double diff = 0.0;
    
    for (DSDP_INT i = 0; i < nsym(packAdim); ++i) {
        diff = data->lfactor[i] - cholL[i];
        err += diff * diff;
    }
    
    if (sqrt(err) / fnrm <= 1e-06) {
        passed("Cholesky decomposition");
    }
    
    /* Vector solve */
    err = 0;
    memcpy(vecrhs->x, rhs, sizeof(double) * packAdim);
    retcode = denseVecSolve(data, vecrhs, mysol);
    for (DSDP_INT i = 0; i < packAdim; ++i) {
        diff = mysol[i] - sol[i];
        err += diff * diff;
    }
    
    if (sqrt(err) <= 1e-03) {
        passed("vector solve");
    }
    
    
    err = 0;
    B = (dsMat *) calloc(1, sizeof(dsMat));
    retcode = denseMatInit(B); checkCodeFree;
    retcode = denseMatAlloc(B, packAdim, FALSE); checkCodeFree;
    memcpy(B->array, BRHS, sizeof(double) * nsym(packAdim));
    
    
    /* Dense matrix AXPBY */
    double alpha = 0.8;
    double beta  = -0.2;
    data->isFactorized = FALSE;
    retcode = denseMataXpbY(alpha, data, beta, B); checkCodeFree;
    
    err = 0.0;
    
    for (DSDP_INT i = 0; i < nsym(packAdim); ++i) {
        diff = B->array[i] - aApbB[i];
        err += diff * diff;
    }
    
    retcode = denseMataXpbY(0.0, data, 0.0, B); checkCodeFree;
    
    for (DSDP_INT i = 0; i < nsym(packAdim); ++i) {
        diff = B->array[i];
        err += diff * diff;
    }
    
    retcode = denseMataXpbY(1.0, data, 0.0, B); checkCodeFree;
    
    for (DSDP_INT i = 0; i < nsym(packAdim); ++i) {
        diff = B->array[i] - packA[i];
        err += diff * diff;
    }
    
    memcpy(B->array, BRHS, sizeof(double) * nsym(packAdim));
    retcode = denseMataXpbY(0.0, data, 0.5, B); checkCodeFree; 
    
    for (DSDP_INT i = 0; i < nsym(packAdim); ++i) {
        diff = B->array[i] - 0.5 * BRHS[i];
        err += diff * diff;
    }
    
    if (sqrt(err) / fnrm < 1e-05) {
        passed("Matrix AXPBY");
    }
    
    data->isFactorized = TRUE;
    
    /* Dense matrix solve */    
    for (DSDP_INT i = 0; i < packAdim; ++i) {
        for (DSDP_INT j = 0; j < packAdim; ++j) {
            diff = fullA[i * packAdim + j] - Bsol[i + packAdim * j];
            err += diff * diff;
        }
    }
    
    if (sqrt(err) / fnrm <= 1e-03) {
        passed("Matrix system");
    }
    
    err = 0.0;
    fastTranspose(Bsol, fullA, 0, 0, packAdim, packAdim, packAdim);
    for (DSDP_INT i = 0; i < packAdim; ++i) {
        for (DSDP_INT j = 0; j < packAdim; ++j) {
            err += fabs(fullA[i + j * packAdim] - Bsol[i * packAdim + j]);
            printf("%10.2e ", fullA[i + j * packAdim]);
        }
        printf("\n");
    }
    
    if (err < 1e-15) {
        passed("Matrix transpose");
    }
    
    int size = 1024;
    mat = (double *) calloc(size * size, sizeof(double));
    mat2 = (double *) calloc(size * size, sizeof(double));
    
    for (int i = 0; i < size; ++i) {
        mat[i] = 1.0;
        mat2[i] = 1.0;
    }
    
    double time = 0.0;
    clock_t start = clock();
    
    for (int i = 0; i < 100; ++i) {
        fastTranspose2(mat, 0, 0, size, size, size);
    }
    
    time = ((double) (clock() - start)) / CLOCKS_PER_SEC;
    printf("Cache Oblivious Transpose elapsed time: %3f \n", time);
    
    
    start = clock();
    for (int i = 0; i < 100; ++i) {
        slowTranspose(mat, mat2, size, size);
    }
    time = ((double) (clock() - start)) / CLOCKS_PER_SEC;
    printf("Naive Transpose elapsed time: %3f \n", time);
    
    
clean_up:
    denseMatFree(data);
    DSDP_FREE(data);
    
    denseMatFree(B);
    DSDP_FREE(B);
    
    DSDP_FREE(mysol);
    
    vec_free(vecrhs);
    DSDP_FREE(vecrhs);
    
    DSDP_FREE(fullA);
    DSDP_FREE(mat);
    DSDP_FREE(mat2);
    
    
    return retcode;
}
