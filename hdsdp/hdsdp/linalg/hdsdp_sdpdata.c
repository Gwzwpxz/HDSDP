/** @file hdsdp\_sdpdata.h
 *  @brief Implenent the SDP Coeffcient data operations
 *  @date 11/24/2022
 */

#include "interface/hdsdp_utils.h"

#include "linalg/hdsdp_sdpdata.h"
#include "linalg/vec_opts.h"
#include "linalg/sparse_opts.h"
#include "linalg/dense_opts.h"
#include "linalg/r1_opts.h"

#include "external/hdsdp_cs.h"

#include <math.h>

static hdsdp_retcode dataMatCreateZeroImpl( void **pA, int nSDPCol, int dataMatNnz, int *dataMatIdx, double *dataMatElem ) {
    
    hdsdp_retcode retcode = HDSDP_RETCODE_OK;
    
    if ( !pA ) {
        retcode = HDSDP_RETCODE_FAILED;
        goto exit_cleanup;
    }
    
    sdp_coeff_zero *zero = NULL;
    HDSDP_INIT(zero, sdp_coeff_zero, 1);
    HDSDP_MEMCHECK(zero);
    
    zero->nSDPCol = nSDPCol;
    *pA = (void *) zero;
    
exit_cleanup:
    
    return retcode;
}

static void dataMatViewZeroImpl( void *A );
static void dataMatViewSparseImpl( void *A );
static void dataMatViewDenseImpl( void *A );
static void dataMatViewRankOneSparseImpl( void *A );
static void dataMatViewRankOneDenseImpl( void *A );

static hdsdp_retcode dataMatCreateSparseImpl( void **pA, int nSDPCol, int dataMatNnz, int *dataMatIdx, double *dataMatElem ) {
    
    hdsdp_retcode retcode = HDSDP_RETCODE_OK;
    
    if ( !pA ) {
        retcode = HDSDP_RETCODE_FAILED;
        goto exit_cleanup;
    }
    
    sdp_coeff_sparse *sparse = NULL;
    HDSDP_INIT(sparse, sdp_coeff_sparse, 1);
    HDSDP_MEMCHECK(sparse);
    
    sparse->nSDPCol = nSDPCol;
    sparse->nTriMatElem = dataMatNnz;
    
    HDSDP_INIT(sparse->triMatRow, int, dataMatNnz);
    HDSDP_INIT(sparse->triMatCol, int, dataMatNnz);
    HDSDP_INIT(sparse->triMatElem, double, dataMatNnz);
    
    HDSDP_MEMCHECK(sparse->triMatRow);
    HDSDP_MEMCHECK(sparse->triMatCol);
    HDSDP_MEMCHECK(sparse->triMatElem);
    
    tsp_decompress(sparse->nSDPCol, sparse->nTriMatElem, dataMatIdx, dataMatElem,
                   sparse->triMatRow, sparse->triMatCol, sparse->triMatElem);
    
    *pA = (void *) sparse;
    
exit_cleanup:
    
    return retcode;
}

static hdsdp_retcode dataMatCreateDenseImpl( void **pA, int nSDPCol, int dataMatNnz, int *dataMatIdx, double *dataMatElem ) {
    
    hdsdp_retcode retcode = HDSDP_RETCODE_OK;
    
    if ( !pA ) {
        retcode = HDSDP_RETCODE_FAILED;
        goto exit_cleanup;
    }
    
    sdp_coeff_dense *dense = NULL;
    HDSDP_INIT(dense, sdp_coeff_dense, 1);
    HDSDP_MEMCHECK(dense);
    
    dense->nSDPCol = nSDPCol;
    HDSDP_INIT(dense->dsMatElem, double, PACK_NNZ(nSDPCol));
    HDSDP_MEMCHECK(dense->dsMatElem);
    
    pds_decompress(dataMatNnz, dataMatIdx, dataMatElem, dense->dsMatElem);

    *pA = (void *) dense;
    
exit_cleanup:
               
    return retcode;
}

/* Rank-one matrices cannot be directly created */
static hdsdp_retcode dataMatCreateRankOneSparseImpl( void **pA, int nSDPCol, int dataMatNnz, int *dataMatIdx, double *dataMatElem ) {
    
    return HDSDP_RETCODE_FAILED;
}

static hdsdp_retcode dataMatCreateRankOneDenseImpl( void **pA, int nSDPCol, int dataMatNnz, int *dataMatIdx, double *dataMatElem ) {
    
    return HDSDP_RETCODE_FAILED;
}

static double dataMatDotFullZeroImpl( void *A, double *dFullMatrix ) {
    
    return 0.0;
}

static double dataMatDotFullSparseImpl( void *A, double *dFullMatrix ) {
    
    /* TODO: */
    return 0.0;
}

static double dataMatDotFullDenseImpl( void *A, double *dFullMatrix ) {
    
    /* TODO: */
    return 0.0;
}

static double dataMatDotFullRankOneSparseImpl( void *A, double *dFullMatrix ) {
    
    /* TODO: */
    return 0.0;
}

static double dataMatDotFullRankOneDenseImpl( void *A, double *dFullMatrix ) {
    
    /* TODO: */
    return 0.0;
}

static void dataMatScalZeroImpl( void *A, double alpha ) {
    
    return;
}

/** @brief A = alpha \* A for sparse matrix A
 *  @param[in] A sdpSparseData pointer
 *  @param[in] alpha Scale parameter
 */
static void dataMatScalSparseImpl( void *A, double alpha ) {
    
    sdp_coeff_sparse *spA = (sdp_coeff_sparse *) A;
    int incx = 1;
    
    scal(&spA->nTriMatElem, &alpha, spA->triMatElem, &incx);
    
    return;
}

/** @brief A = alpha \* A for dense matrix A
 *  @param[in] A sdpDenseData pointer
 *  @param[in] alpha Scale parameter
 */
static void dataMatScalDenseImpl( void *A, double alpha ) {
    
    sdp_coeff_dense *dsA = (sdp_coeff_dense *) A;
    int nElem = PACK_NNZ(dsA->nSDPCol);
    int incx = 1;
    
    scal(&nElem, &alpha, dsA->dsMatElem, &incx);
    
    return;
}

/** @brief A = alpha \* A for rank one sparse matrix A
 *  @param[in] A sdpRankOneSparseData pointer
 *  @param[in] alpha Scale parameter
 */
static void dataMatScalRankOneSparseImpl( void *A, double alpha ) {
    
    /* Avoid alpha = 0.0 case */
    assert( fabs(alpha) >= 1e-08 );
    
    sdp_coeff_spr1 *spR1A = (sdp_coeff_spr1 *) A;
    
    spR1A->spR1FactorSign = spR1A->spR1FactorSign * alpha;
    
    return;
}

/** @brief A = alpha \* A for rank one dense matrix A
 *  @param[in] A sdpRankOneDenseData pointer
 *  @param[in] alpha Scale parameter
 */
static void dataMatScalRankOneDenseImpl( void *A, double alpha ) {
    
    /* Avoid alpha = 0.0 case */
    assert( fabs(alpha) >= 1e-08 );
    
    sdp_coeff_dsr1 *dsR1A = (sdp_coeff_dsr1 *) A;
    dsR1A->r1FactorSign = dsR1A->r1FactorSign * alpha;
    
    return;
}

static double dataMatNormZeroImpl( void *A, int type ) {
    
    return 0.0;
}

/** @brief Calculate norm of sparse matrix A
 *  @param[in] A sdpSparseData pointer
 *  @param[in] type Type of norm
 *  @return Norm of A
 */
static double dataMatNormSparseImpl( void *A, int type ) {
    
    sdp_coeff_sparse *spA = (sdp_coeff_sparse *) A;
    double nrmA = 0.0;
    
    if ( type == FRO_NORM ) {
        for ( int i = 0; i < spA->nTriMatElem; ++i ) {
            nrmA += ( spA->triMatCol[i] == spA->triMatRow[i] ) ?
                      spA->triMatElem[i] * spA->triMatElem[i] :
                      spA->triMatElem[i] * spA->triMatElem[i] * 2.0;
        }
        nrmA = sqrt(nrmA);
    } else if ( type == ABS_NORM ) {
        for ( int i = 0; i < spA->nTriMatElem; ++i ) {
            nrmA += ( spA->triMatCol[i] == spA->triMatRow[i] ) ?
                      fabs(spA->triMatElem[i]) :
                      fabs(spA->triMatElem[i]) * 2.0;
        }
    }
    
    return nrmA;
}

/** @brief Calculate norm of dense matrix A
 *  @param[in] A sdpDenseData pointer
 *  @param[in] type Type of norm
 *  @return Norm of A
 */
static double dataMatNormDenseImpl( void *A, int type ) {
    
    sdp_coeff_dense *dsA = (sdp_coeff_dense *) A;
    double nrmA = 0.0;
    int nCol = dsA->nSDPCol;
    int incx = 1;
    int colLen; ///< Exclude diagonal
    double colNrm; ///< Exclude diagonal
    double *p = dsA->dsMatElem;
    
    if ( type == FRO_NORM ) {
        for ( int i = 0; i < nCol; ++i ) {
            nrmA += p[0] * p[0];
            colLen = nCol - i - 1;
            colNrm = nrm2(&colLen, p + 1, &incx);
            nrmA += colNrm * colNrm * 2.0;
            p = p + colLen + 1;
        }
        nrmA = sqrt(nrmA);
    } else if ( type == ABS_NORM ) {
        for ( int i = 0; i < nCol; ++i ) {
            nrmA += fabs(p[0]);
            colLen = nCol - i - 1;
            nrmA += nrm1(&colLen, p + 1, &incx) * 2;
            p = p + colLen + 1;
        }
    }
    
    return nrmA;
}

/** @brief Calculate norm of rank one sparse matrix A
 *  @param[in] A sdpRankOneSparseData pointer
 *  @param[in] type Type of norm
 *  @return Norm of A
 */
static double dataMatNormRankOneSparseImpl( void *A, int type ) {
    
    sdp_coeff_spr1 *spR1A = (sdp_coeff_spr1 *) A;
    double nrmA = 0.0;
    int incx = 1;
    
    if ( type == FRO_NORM ) {
        nrmA = nrm2(&spR1A->nSpR1FactorElem, spR1A->spR1MatElem, &incx);
        nrmA = nrmA * nrmA * fabs(spR1A->spR1FactorSign);
    } else if ( type == ABS_NORM ) {
        nrmA = nrm1(&spR1A->nSpR1FactorElem, spR1A->spR1MatElem, &incx);
        nrmA = nrmA * nrmA * fabs(spR1A->spR1FactorSign);
    }
    
    return nrmA;
}

/** @brief Calculate norm of rank one densematrix A
 *  @param[in] A sdpRankOneDenseData pointer
 *  @param[in] type Type of norm
 *  @return Norm of A
 */
static double dataMatNormRankOneDenseImpl( void *A, int type ) {
    
    sdp_coeff_dsr1 *dsR1A = (sdp_coeff_dsr1 *) A;
    double nrmA = 0.0;
    int incx = 1;
    
    if ( type == FRO_NORM ) {
        nrmA = nrm2(&dsR1A->nSDPCol, dsR1A->r1MatFactor, &incx);
        nrmA = nrmA * nrmA * fabs(dsR1A->r1FactorSign);
    } else if ( type == ABS_NORM ) {
        nrmA = nrm1(&dsR1A->nSDPCol, dsR1A->r1MatFactor, &incx);
        nrmA = nrmA * nrmA * fabs(dsR1A->r1FactorSign);
    }
    
    return nrmA;
}

static int dataMatGetNnzZeroImpl( void *A ) {
    
    return 0;
}

/** @brief Calculate number of nonzero elements in sparse matrix A
 *  @param[in] A sdpSparseData pointer
 *  @return Number of nonzero elements in A
 */
static int dataMatGetNnzSparseImpl( void *A ) {
    
    sdp_coeff_sparse *spA = (sdp_coeff_sparse *) A;
    
    return spA->nTriMatElem;
}

/** @brief Calculate number of nonzero elements in dense matrix A
 *  @param[in] A sdpDenseData pointer
 *  @return Number of nonzero elements in A
 */
static int dataMatGetNnzDenseImpl( void *A ) {
    
    sdp_coeff_dense *dsA = (sdp_coeff_dense *) A;
    
    return PACK_NNZ(dsA->nSDPCol);
}

/** @brief Calculate number of nonzero elements in rank one sparse matrix A
 *  @param[in] A sdpRankOneSparseData pointer
 *  @return Number of nonzero elements in A
 */
static int dataMatGetNnzRankOneSparseImpl( void *A ) {
    
    sdp_coeff_spr1 *spR1A = (sdp_coeff_spr1 *) A;
    
    return PACK_NNZ(spR1A->nSpR1FactorElem);
}

/** @brief Calculate number of nonzero elements in rank one dense matrix A
 *  @param[in] A sdpRankOneDenseData pointer
 *  @return Number of nonzero elements in A
 */
static int dataMatGetNnzRankOneDenseImpl( void *A ) {
    
    sdp_coeff_dsr1 *dsR1A = (sdp_coeff_dsr1 *) A;
    
    return PACK_NNZ(dsR1A->nSDPCol);
}

static void dataMatDumpZeroImpl( void *A, double *v ) {
    
    sdp_coeff_zero *zeroA = (sdp_coeff_zero *) A;
    HDSDP_ZERO(v, double, zeroA->nSDPCol * zeroA->nSDPCol);
    
    return;
}

/** @brief Construct full matrix from sparse matrix A
 *  @param[in] A sdpSparseData pointer
 *  @param[out] v Store full matrix
 */
static void dataMatDumpSparseImpl( void *A, double *v ) {
    
    sdp_coeff_sparse *spA = (sdp_coeff_sparse *) A;
    HDSDP_ZERO(v, double, spA->nSDPCol * spA->nSDPCol);
    int nCol = spA->nSDPCol;
    
    for ( int i = 0; i < spA->nTriMatElem; ++i ) {
        v[spA->triMatCol[i] * nCol + spA->triMatRow[i]] = spA->triMatElem[i];
        if ( spA->triMatCol[i] != spA->triMatRow[i] ) {
            v[spA->triMatRow[i] * nCol + spA->triMatCol[i]] = spA->triMatElem[i];
        }
    }
    
    return;
}

/** @brief Construct full matrix from dense matrix A
 *  @param[in] A sdpDenseData pointer
 *  @param[out] v Store full matrix
 */
static void dataMatDumpDenseImpl( void *A, double *v ) {
    
    sdp_coeff_dense *dsA = (sdp_coeff_dense *) A;
    int nCol = dsA->nSDPCol;
    int dsIdx; ///< Index for (col, row) = (i, j) in dsMatElem
    
    for ( int i = 0, j; i < nCol; ++i) { // Column index
        dsIdx = i * (nCol * 2 - i + 1) / 2;
        v[i * nCol + i] = dsA->dsMatElem[dsIdx];
        for ( j = i + 1; j < nCol; ++j ) { // Row index
            dsIdx = i * (nCol * 2 - i - 1) / 2 + j;
            v[i * nCol + j] = dsA->dsMatElem[dsIdx];
            v[j * nCol + i] = v[i * nCol + j];
        }
    }
    
    return;
}

/** @brief Construct full matrix from rank one sparse matrix A
 *  @param[in] A sdpRankOneSparseData pointer
 *  @param[out] v Store full matrix
 */
static void dataMatDumpRankOneSparseImpl( void *A, double *v ) {
    
    sdp_coeff_spr1 *spR1A = (sdp_coeff_spr1 *) A;
    HDSDP_ZERO(v, double, spR1A->nSDPCol * spR1A->nSDPCol);
    int nCol = spR1A->nSDPCol;
    int *spR1MatIdx = spR1A->spR1MatIdx;
    
    for ( int i = 0, j; i < spR1A->nSpR1FactorElem; ++i ) { // Column index
        v[spR1MatIdx[i] * nCol + spR1MatIdx[i]] = \
        spR1A->spR1FactorSign * spR1A->spR1MatElem[i] * spR1A->spR1MatElem[i];
        for ( j = i + 1; j < spR1A->nSpR1FactorElem; ++j ) { // Row index
            v[spR1MatIdx[i] * nCol + spR1MatIdx[j]] = \
            spR1A->spR1FactorSign * spR1A->spR1MatElem[i] * spR1A->spR1MatElem[j];
            v[spR1MatIdx[j] * nCol + spR1MatIdx[i]] = \
            v[spR1MatIdx[i] * nCol + spR1MatIdx[j]];
        }
    }
    
    return;
}

/** @brief Construct full matrix from rank one dense matrix A
 *  @param[in] A sdpRankOneDenseData pointer
 *  @param[out] v Store full matrix
 */
static void dataMatDumpRankOneDenseImpl( void *A, double *v ) {
    
    sdp_coeff_dsr1 *dsR1A = (sdp_coeff_dsr1 *) A;
    
    for ( int i = 0, j; i < dsR1A->nSDPCol; ++i ) { // Column index
        v[i * dsR1A->nSDPCol + i] = \
        dsR1A->r1FactorSign * dsR1A->r1MatFactor[i] * dsR1A->r1MatFactor[i];
        for ( j = i + 1; j < dsR1A->nSDPCol; ++j ) { // Row index
            v[i * dsR1A->nSDPCol + j] = \
            dsR1A->r1FactorSign * dsR1A->r1MatFactor[i] * dsR1A->r1MatFactor[j];
            v[j * dsR1A->nSDPCol + i] = v[i * dsR1A->nSDPCol + j];
        }
    }
    
    return;
}

static void dataMatClearZeroImpl( void *A ) {
    
    if ( !A ) {
        return;
    }
    
    HDSDP_ZERO(A, sdp_coeff_zero, 1);
    
    return;
}

static void dataMatClearSparseImpl( void *A ) {
    
    if ( !A ) {
        return;
    }
    
    sdp_coeff_sparse *sparse = (sdp_coeff_sparse *) A;
    
    HDSDP_FREE(sparse->triMatRow);
    HDSDP_FREE(sparse->triMatCol);
    HDSDP_FREE(sparse->triMatElem);
    HDSDP_ZERO(A, sdp_coeff_sparse, 1);
    
    return;
}

static void dataMatClearDenseImpl( void *A ) {
    
    if ( !A ) {
        return;
    }
    
    sdp_coeff_dense *dense = (sdp_coeff_dense *) A;
    
    HDSDP_FREE(dense->dsMatElem);
    HDSDP_ZERO(A, sdp_coeff_dense, 1);
    
    return;
}

static void dataMatClearRankOneSparseImpl( void *A ) {
    
    if ( !A ) {
        return;
    }
    
    sdp_coeff_spr1 *spr1 = (sdp_coeff_spr1 *) A;
    
    HDSDP_FREE(spr1->spR1MatElem);
    HDSDP_FREE(spr1->spR1MatIdx);
    
    HDSDP_ZERO(A, sdp_coeff_spr1, 1);
    
    return;
}

static void dataMatClearRankOneDenseImpl( void *A ) {
    
    if ( !A ) {
        return;
    }
    
    sdp_coeff_dsr1 *dsr1 = (sdp_coeff_dsr1 *) A;
    
    HDSDP_FREE(dsr1->r1MatFactor);
    HDSDP_ZERO(A, sdp_coeff_dsr1, 1);
    
    return;
}

static void dataMatDestroyZeroImpl( void **pA ) {
    
    if ( !pA ) {
        return;
    }
    
    dataMatClearZeroImpl(*pA);
    HDSDP_FREE(*pA);
    
    return;
}

static void dataMatDestroySparseImpl( void **pA ) {
    
    if ( !pA ) {
        return;
    }
    
    dataMatClearSparseImpl(*pA);
    HDSDP_FREE(*pA);
    
    return;
}

static void dataMatDestroyDenseImpl( void **pA ) {
    
    if ( !pA ) {
        return;
    }
    
    dataMatClearDenseImpl(*pA);
    HDSDP_FREE(*pA);
    
    return;
}

static void dataMatDestroyRankOneSparseImpl( void **pA ) {
    
    if ( !pA ) {
        return;
    }
    
    dataMatClearRankOneSparseImpl(*pA);
    HDSDP_FREE(*pA);
    
    return;
}

static void dataMatDestroyRankOneDenseImpl( void **pA ) {
    
    if ( !pA ) {
        return;
    }
    
    dataMatClearRankOneDenseImpl(*pA);
    HDSDP_FREE(*pA);
    
    return;
}

static void dataMatViewZeroImpl( void *A ) {
    
    printf("Zero matrix L1 = [%5.3e] L2 = [%5.3e] \n", 0.0, 0.0);
    
    return;
}

static void dataMatViewSparseImpl( void *A ) {
    
    sdp_coeff_sparse *sparse = (sdp_coeff_sparse *) A;
    
    printf("Sparse matrix L1 = [%5.3e] L2 = [%5.3e]. \n", dataMatNormSparseImpl(sparse, ABS_NORM),
           dataMatNormSparseImpl(A, FRO_NORM));
    
#if 0
    dcs dcsMat;
    dcsMat.n = sparse->nSDPCol;
    dcsMat.m = sparse->nSDPCol;
    dcsMat.nz = sparse->nTriMatElem;
    dcsMat.nzmax = sparse->nTriMatElem;
    dcsMat.p = sparse->triMatCol;
    dcsMat.i = sparse->triMatRow;
    dcsMat.x = sparse->triMatElem;
    dcs_print(&dcsMat, 1);
#endif
    
    return;
}

static void dataMatViewDenseImpl( void *A ) {
    
    sdp_coeff_dense *dense = (sdp_coeff_dense *) A;
    printf("Dense matrix L1 = [%5.3e] L2 = [%5.3e]. \n", dataMatNormDenseImpl(dense, ABS_NORM),
           dataMatNormDenseImpl(dense, FRO_NORM));
    
    return;
}

static void dataMatViewRankOneSparseImpl( void *A ) {
    
    sdp_coeff_spr1 *spr1 = (sdp_coeff_spr1 *) A;
    printf("Sparse rank-one matrix L1 = [%5.3e] L2 = [%5.3e]. \n", dataMatNormRankOneSparseImpl(spr1, ABS_NORM),
           dataMatNormRankOneSparseImpl(spr1, FRO_NORM));
    
    return;
}

static void dataMatViewRankOneDenseImpl( void *A ) {
    
    sdp_coeff_dsr1 *dsr1 = (sdp_coeff_dsr1 *) A;
    printf("Dense rank-one matrix L1 = [%5.3e] L2 = [%5.3e]. \n", dataMatNormRankOneDenseImpl(dsr1, ABS_NORM),
           dataMatNormRankOneDenseImpl(dsr1, FRO_NORM));
    
    return;
}

static void sdpDataMatIChooseType( sdp_coeff *sdpCoeff, sdp_coeff_type dataType ) {
    
    sdpCoeff->dataType = dataType;
     
    switch ( dataType ) {
        case SDP_COEFF_ZERO:
            sdpCoeff->create = dataMatCreateZeroImpl;
            sdpCoeff->dot = dataMatDotFullZeroImpl;
            sdpCoeff->scal = dataMatScalZeroImpl;
            sdpCoeff->norm = dataMatNormZeroImpl;
            sdpCoeff->eig = NULL;
            sdpCoeff->getNnz = dataMatGetNnzZeroImpl;
            sdpCoeff->dump = dataMatDumpZeroImpl;
            sdpCoeff->destroy = dataMatDestroyZeroImpl;
            sdpCoeff->view = dataMatViewZeroImpl;
            break;
        case SDP_COEFF_SPARSE:
            sdpCoeff->create = dataMatCreateSparseImpl;
            sdpCoeff->dot = dataMatDotFullSparseImpl;
            sdpCoeff->scal = dataMatScalSparseImpl;
            sdpCoeff->norm = dataMatNormSparseImpl;
            sdpCoeff->eig = NULL;
            sdpCoeff->getNnz = dataMatGetNnzSparseImpl;
            sdpCoeff->dump = dataMatDumpSparseImpl;
            sdpCoeff->destroy = dataMatDestroySparseImpl;
            sdpCoeff->view = dataMatViewSparseImpl;
            break;
        case SDP_COEFF_DENSE:
            sdpCoeff->create = dataMatCreateDenseImpl;
            sdpCoeff->dot = dataMatDotFullDenseImpl;
            sdpCoeff->scal = dataMatScalDenseImpl;
            sdpCoeff->norm = dataMatNormDenseImpl;
            sdpCoeff->eig = NULL;
            sdpCoeff->getNnz = dataMatGetNnzDenseImpl;
            sdpCoeff->dump = dataMatDumpDenseImpl;
            sdpCoeff->destroy = dataMatDestroyDenseImpl;
            sdpCoeff->view = dataMatViewDenseImpl;
            break;
        case SDP_COEFF_SPR1:
            sdpCoeff->create = dataMatCreateRankOneSparseImpl;
            sdpCoeff->dot = dataMatDotFullRankOneSparseImpl;
            sdpCoeff->scal = dataMatScalRankOneSparseImpl;
            sdpCoeff->norm = dataMatNormRankOneSparseImpl;
            sdpCoeff->eig = NULL;
            sdpCoeff->getNnz = dataMatGetNnzRankOneSparseImpl;
            sdpCoeff->dump = dataMatDumpRankOneSparseImpl;
            sdpCoeff->destroy = dataMatDestroyRankOneSparseImpl;
            sdpCoeff->view = dataMatViewRankOneSparseImpl;
            break;
        case SDP_COEFF_DSR1:
            sdpCoeff->create = dataMatCreateRankOneDenseImpl;
            sdpCoeff->dot = dataMatDotFullRankOneDenseImpl;
            sdpCoeff->scal = dataMatScalRankOneDenseImpl;
            sdpCoeff->norm = dataMatNormRankOneDenseImpl;
            sdpCoeff->eig = NULL;
            sdpCoeff->getNnz = dataMatGetNnzRankOneDenseImpl;
            sdpCoeff->dump = dataMatDumpRankOneDenseImpl;
            sdpCoeff->destroy = dataMatDestroyRankOneDenseImpl;
            sdpCoeff->view = dataMatViewRankOneDenseImpl;
            break;
        default:
            assert( 0 );
            break;
    }
    
    return;
}

extern hdsdp_retcode sdpDataMatCreate( sdp_coeff **psdpCoeff ) {
    
    hdsdp_retcode retcode = HDSDP_RETCODE_OK;
    
    if ( !psdpCoeff ) {
        retcode = HDSDP_RETCODE_FAILED;
        goto exit_cleanup;
    }
    
    sdp_coeff *sdpCoeff = NULL;
    HDSDP_INIT(sdpCoeff, sdp_coeff, 1);
    HDSDP_MEMCHECK(sdpCoeff);
    
    /* -1 means there is no eigen-decomposition available */
    sdpCoeff->eigRank = -1;
    *psdpCoeff = sdpCoeff;
    
exit_cleanup:
    
    return retcode;
}

/** @brief Set SDP data matrix. This routine selects data type and assiciate structure with their operations
 * 
 *
 */
extern hdsdp_retcode sdpDataMatSetData( sdp_coeff *sdpCoeff, int nSDPCol, int dataMatNnz, int *dataMatIdx, double *dataMatElem ) {
    
    hdsdp_retcode retcode = HDSDP_RETCODE_OK;
    
    /* Choose data matrix type */
    int nPack = PACK_NNZ(nSDPCol);
    
    /* At this stage, only sparse, zero and dense matrices are classified */
    if ( dataMatNnz == 0 ) {
        sdpDataMatIChooseType(sdpCoeff, SDP_COEFF_ZERO);
    } else if ( dataMatNnz > 0 * nPack ) {
        sdpDataMatIChooseType(sdpCoeff, SDP_COEFF_DENSE);
    } else {
        sdpDataMatIChooseType(sdpCoeff, SDP_COEFF_SPARSE);
    }
    
    /* Create data */
    HDSDP_CALL(sdpCoeff->create(&sdpCoeff->dataMat, nSDPCol,
                                dataMatNnz, dataMatIdx, dataMatElem));
    
exit_cleanup:
    
    return retcode;
}

extern int sdpDataMatGetRank( sdp_coeff *sdpCoeff ) {
    
    if ( sdpCoeff->dataType == SDP_COEFF_ZERO ) {
        return 0;
    } else if ( sdpCoeff->dataType == SDP_COEFF_DSR1 || sdpCoeff->dataType == SDP_COEFF_SPR1 ) {
        return 1;
    } else if ( sdpCoeff->eigRank != -1 ) {
        return sdpCoeff->eigRank;
    }
    
    return sdpCoeff->nSDPCol;
}

extern double sdpDataMatDot( sdp_coeff *sdpCoeff, double *dFullMatrix ) {
    
    return sdpCoeff->dot(sdpCoeff->dataMat, dFullMatrix);
}

extern void sdpDataMatScal( sdp_coeff *sdpCoeff, double scal ) {
    
    sdpCoeff->scal(sdpCoeff->dataMat, scal);
    
    return;
}

extern double sdpDataMatNorm( sdp_coeff *sdpCoeff, int type ) {
    
    return sdpCoeff->norm(sdpCoeff->dataMat, type);
}

/* Used in presolving and low-rank structure detection */
extern hdsdp_retcode sdpDataMatFatorize( sdp_coeff *sdpCoeff, double *dAuxFullMatrix ) {
    
    /* TODO: Add factorization routine */
    
    return HDSDP_RETCODE_OK;
}

extern int sdpDataMatGetNnz( sdp_coeff *sdpCoeff ) {
    
    return sdpCoeff->getNnz(sdpCoeff->dataMat);
}

extern void sdpDataMatDump( sdp_coeff *sdpCoeff, double *dFullMatrix ) {
    
    return sdpCoeff->dump(sdpCoeff->dataMat, dFullMatrix);
}

extern int sdpDataMatGetType( sdp_coeff *sdpCoeff ) {
    
    return sdpCoeff->dataType;
}

extern void sdpDataMatClear( sdp_coeff *sdpCoeff ) {
    
    if ( !sdpCoeff ) {
        return;
    }
    
    sdpCoeff->destroy(&sdpCoeff->dataMat);
    
    if ( sdpCoeff->eigRank != -1 ) {
        HDSDP_FREE(sdpCoeff->eigVals);
        HDSDP_FREE(sdpCoeff->eigVecs);
    }
    
    HDSDP_ZERO(sdpCoeff, sdp_coeff, 1);
    
    return;
}

extern void sdpDataMatDestroy( sdp_coeff **psdpCoeff ) {
    
    if ( !psdpCoeff ) {
        return;
    }
    
    sdpDataMatClear(*psdpCoeff);
    HDSDP_FREE(*psdpCoeff);
    
    return;
}

extern void sdpDataMatView( sdp_coeff *sdpCoeff ) {
    
    sdpCoeff->view(sdpCoeff->dataMat);
    
    return;
}
