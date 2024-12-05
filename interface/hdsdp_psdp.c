#ifdef HEADERPATH
#include "interface/hdsdp_psdp.h"
#include "interface/hdsdp_utils.h"
#include "interface/hdsdp_conic.h"
#include "interface/def_hdsdp.h"
#include "linalg/dense_opts.h"
#include "linalg/vec_opts.h"
#else
#include "hdsdp_psdp.h"
#include "hdsdp_utils.h"
#include "hdsdp_conic.h"
#include "def_hdsdp.h"
#include "dense_opts.h"
#include "vec_opts.h"
#endif

#include <math.h>

static void sdpPrimalConeILanczosMultiply( void *Hpsdp, double *dLhsVec, double *dRhsVec ) {
    
    /* Ratio test subroutine */
    hdsdp_psdp *psdp = (hdsdp_psdp *) Hpsdp;
    int iLanczos = psdp->iLanczos;
    HFpLinsysBSolve(psdp->XFactors[iLanczos], 1, dLhsVec, dRhsVec);
    fds_symv(HConeGetDim(psdp->HCones[iLanczos]), -1.0, psdp->dPrimalXStep[iLanczos], dRhsVec, 0.0, psdp->dPrimalAuxiVec1);
    HFpLinsysFSolve(psdp->XFactors[iLanczos], 1, psdp->dPrimalAuxiVec1, dRhsVec);
    
    return;
}

static hdsdp_retcode HPSDPIRecover( hdsdp_psdp *Hpsdp ) {
    
    /* Restore the dual solution in case of failure
       Barrier parameter and primal bound will be kept */
    hdsdp_retcode retcode = HDSDP_RETCODE_OK;
    double *dDualBackUp = Hpsdp->dPrimalAuxiVec2;
    
    HDSDP_MEMCPY(Hpsdp->dRowDual, dDualBackUp, double, Hpsdp->nRows);
    
    int isInterior = 0;
    for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
        HDSDP_CALL(HConeCheckIsInterior(Hpsdp->HCones[iCone], 1.0, Hpsdp->dRowDual, &isInterior));
    }
    
exit_cleanup:
    return retcode;
}

extern hdsdp_retcode HPSDPCreate( hdsdp_psdp **pHpsdp ) {
    
    hdsdp_retcode retcode = HDSDP_RETCODE_OK;
    HDSDP_NULLCHECK(pHpsdp);
    
    hdsdp_psdp *Hpsdp = NULL;
    HDSDP_INIT(Hpsdp, hdsdp_psdp, 1);
    HDSDP_MEMCHECK(Hpsdp);
    HDSDP_ZERO(Hpsdp, hdsdp_psdp, 1);
    
    *pHpsdp = Hpsdp;
    
exit_cleanup:
    return retcode;
}

extern hdsdp_retcode HPSDPInit( hdsdp_psdp *Hpsdp, hdsdp *HSolver ) {
    /* Initialize primal solver from dual solver */
    hdsdp_retcode retcode = HDSDP_RETCODE_OK;
    
    /* We need initial dual feasible solution */
    int nLpCones = get_int_feature(HSolver, INT_FEATURE_N_LPCONES);
    
    if ( HSolver->dInfeas > 0.0 || nLpCones > 0 ) {
        retcode = HDSDP_RETCODE_FAILED;
        goto exit_cleanup;
    }
    
    Hpsdp->HSolver = HSolver;
    Hpsdp->HCones = HSolver->HCones;
    
    /* Space in dual solver is reused in PSDP */
    Hpsdp->nRows = HSolver->nRows;
    Hpsdp->nCones = HSolver->nCones;
    Hpsdp->rowRHS = HSolver->rowRHS;
    Hpsdp->HKKT = HSolver->HKKT;
    Hpsdp->dRowDual = HSolver->dRowDual;
    Hpsdp->dRowDualStep = HSolver->dRowDualStep;
    
    Hpsdp->dBarrierMu = HSolver->dBarrierMu;
    
    HDSDP_INIT(Hpsdp->dPrimalAuxiVec1, double, HSolver->nRows);
    HDSDP_MEMCHECK(Hpsdp->dPrimalAuxiVec1);
    HDSDP_INIT(Hpsdp->dPrimalAuxiVec2, double, HSolver->nRows);
    HDSDP_MEMCHECK(Hpsdp->dPrimalAuxiVec2);
    
    HDSDP_INIT(Hpsdp->XFactors, hdsdp_linsys *, Hpsdp->nCones);
    HDSDP_MEMCHECK(Hpsdp->XFactors);
    
    /* Use new Lanczos solver for primal ratio test */
    HDSDP_INIT(Hpsdp->Lanczos, hdsdp_lanczos *, Hpsdp->nCones);
    HDSDP_MEMCHECK(Hpsdp->Lanczos);
    
    for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
        int nCol = HConeGetDim(Hpsdp->HCones[iCone]);
        HDSDP_CALL(HFpLinsysCreate(&Hpsdp->XFactors[iCone], nCol, HDSDP_LINSYS_DENSE_DIRECT));
        HDSDP_CALL(HFpLinsysSymbolic(Hpsdp->XFactors[iCone], NULL, NULL));
        HDSDP_CALL(HLanczosCreate(&Hpsdp->Lanczos[iCone]));
        HDSDP_CALL(HLanczosInit(Hpsdp->Lanczos[iCone], nCol, 50));
        HLanczosSetData(Hpsdp->Lanczos[iCone], Hpsdp, sdpPrimalConeILanczosMultiply);
    }
    
    /* Primal memory allocation */
    HDSDP_INIT(Hpsdp->dPrimalKKTRhs, double, Hpsdp->nRows);
    HDSDP_MEMCHECK(Hpsdp->dPrimalKKTRhs);
    
    HDSDP_INIT(Hpsdp->dPrimalX, double *, Hpsdp->nCones);
    HDSDP_MEMCHECK(Hpsdp->dPrimalX);
    
    HDSDP_INIT(Hpsdp->dPrimalScalX, double *, Hpsdp->nCones);
    HDSDP_MEMCHECK(Hpsdp->dPrimalScalX);
    
    HDSDP_INIT(Hpsdp->dPrimalXStep, double *, Hpsdp->nCones);
    HDSDP_MEMCHECK(Hpsdp->dPrimalXStep);
    
    HDSDP_INIT(Hpsdp->dPrimalMatBuffer, double *, Hpsdp->nCones);
    HDSDP_MEMCHECK(Hpsdp->dPrimalMatBuffer);
    
    HDSDP_INIT(Hpsdp->dPrimalMatBuffer2, double *, Hpsdp->nCones);
    HDSDP_MEMCHECK(Hpsdp->dPrimalMatBuffer2);
    
    for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
        int nCol = HConeGetDim(Hpsdp->HCones[iCone]);
        HDSDP_INIT(Hpsdp->dPrimalX[iCone], double, nCol * nCol);
        HDSDP_MEMCHECK(Hpsdp->dPrimalX[iCone]);
        HDSDP_INIT(Hpsdp->dPrimalScalX[iCone], double, nCol * nCol);
        HDSDP_MEMCHECK(Hpsdp->dPrimalScalX[iCone]);
        HDSDP_INIT(Hpsdp->dPrimalXStep[iCone], double, nCol * nCol);
        HDSDP_MEMCHECK(Hpsdp->dPrimalXStep[iCone]);
        HDSDP_INIT(Hpsdp->dPrimalMatBuffer[iCone], double, nCol * nCol);
        HDSDP_MEMCHECK(Hpsdp->dPrimalMatBuffer[iCone]);
        HDSDP_INIT(Hpsdp->dPrimalMatBuffer2[iCone], double, nCol * nCol);
        HDSDP_MEMCHECK(Hpsdp->dPrimalMatBuffer2[iCone]);
        HDSDPGetConeValues(Hpsdp->HSolver, iCone, Hpsdp->dPrimalX[iCone], NULL, Hpsdp->dPrimalMatBuffer[iCone]);
        HDSDP_MEMCPY(Hpsdp->dPrimalScalX[iCone], Hpsdp->dPrimalX[iCone], double, nCol * nCol);
    }
    
    int isInterior = 0;
    for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
        HFpLinsysPsdCheck(Hpsdp->XFactors[iCone], NULL, NULL, Hpsdp->dPrimalX[iCone], &isInterior);
        if ( !isInterior ) {
            retcode = HDSDP_RETCODE_FAILED;
            goto exit_cleanup;
        }
    }
    
    /* Register primal matrix in KKT solver */
    HKKTRegisterPSDP(Hpsdp->HKKT, Hpsdp->dPrimalScalX);
    hdsdp_printf("HDSDP nearly converges. Primal refinement starts. \n");
    
exit_cleanup:
    return retcode;
}


extern hdsdp_retcode HPSDPOptimize( hdsdp_psdp *Hpsdp ) {
    
    hdsdp_retcode retcode = HDSDP_RETCODE_OK;
    
    double *dPrimalKKTRhs = Hpsdp->dPrimalKKTRhs;
    
    double **dPrimalX = Hpsdp->dPrimalX;
    double **dPrimalScalX = Hpsdp->dPrimalScalX;
    double **dPrimalXStep = Hpsdp->dPrimalXStep;
    double **dPrimalMatBuffer = Hpsdp->dPrimalMatBuffer;
    double **dPrimalMatBuffer2 = Hpsdp->dPrimalMatBuffer2;
    
    double *dRowDual = Hpsdp->dRowDual;
    double *dRowDualStep = Hpsdp->dRowDualStep;
    double *dDualBackUp = Hpsdp->dPrimalAuxiVec2;
    double dBarrierMu = Hpsdp->dBarrierMu;
    
    hdsdp_cone **cones = Hpsdp->HCones;
    hdsdp *HSolver  = Hpsdp->HSolver;
    
    double *dPrimalInfeas = HSolver->dHAuxiVec1;
    
    double dAbsoptTol = get_dbl_param(HSolver, DBL_PARAM_ABSOPTTOL);
    double dRelfeasTol = get_dbl_param(HSolver, DBL_PARAM_RELFEASTOL);
    double dReloptTol = get_dbl_param(HSolver, DBL_PARAM_RELOPTTOL);
    double dObjScal = get_dbl_feature(HSolver, DBL_FEATURE_OBJSCALING);
    double dRhsScal = get_dbl_feature(HSolver, DBL_FEATURE_RHSSCALING);
    double dSumDims = HSolver->dAllConeDims - HSolver->nRows * 2;
    double pdScal = 1.0 / (dObjScal * dRhsScal);
    
    int nIter = 0;
    int nBadIter = 0;
    int nMaxIter = 100;
    double dPrimalInfeasNorm = 0.0;
    double dPrimalInfeasStart = 0.0;
    
    /* Back up dual solution in case of failure */
    HDSDP_MEMCPY(dDualBackUp, HSolver->dRowDual, double, HSolver->nRows);
    /* Build KKT system */
    HDSDP_CALL(HKKTBuildUp(Hpsdp->HKKT, KKT_TYPE_PRIMAL));
    HKKTRegularize(HSolver->HKKT, 1e-16);
    // hdsdp_printf("KKT Build up: %f s \n", HUtilGetTimeStamp() - HSolver->dTimeBegin);
    HDSDP_CALL(HKKTFactorize(Hpsdp->HKKT));
    // hdsdp_printf("KKT Factorize: %f s \n", HUtilGetTimeStamp() - HSolver->dTimeBegin);
    
    /* Enter primal main loop */
    for ( nIter = 0; nIter < nMaxIter; ++nIter ) {
        
        /* Compute primal infeasibility rp = A * X - b */
        HDSDP_ZERO(dPrimalInfeas, double, Hpsdp->nRows);
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
            HConeComputeATimesXpy(cones[iCone], dPrimalX[iCone], dPrimalInfeas);
        }
        dPrimalInfeasNorm = 0.0;
        for ( int iRow = 0; iRow < Hpsdp->nRows; ++iRow ) {
            double dTmp = Hpsdp->rowRHS[iRow] - dPrimalInfeas[iRow];
            dPrimalInfeasNorm += dTmp * dTmp;
        }
        dPrimalInfeasNorm = sqrt(dPrimalInfeasNorm);
        
        /* Record initial primal infeasibility */
        if ( nIter == 0 ) {
            dPrimalInfeasStart = dPrimalInfeasNorm;
        }
                
        /* Prepare primal KKT RHS */
        /* Get buffer matrix X * S * X */
        int iDualMat = 1;
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone) {
            int nCol = HConeGetDim(cones[iCone]);
            HDSDP_ZERO(dPrimalMatBuffer[iCone], double, nCol * nCol);
            HDSDP_ZERO(dPrimalMatBuffer2[iCone], double, nCol * nCol);
            HConeBuildPrimalXSXDirection(cones[iCone], Hpsdp->HKKT, dPrimalX[iCone], dPrimalMatBuffer[iCone], iDualMat);
        }
        
        /* Build the KKT right-hand-side */
        HDSDP_ZERO(dPrimalKKTRhs, double, Hpsdp->nRows);
        
        /* Process the buffer */
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone) {
            int nCol = HConeGetDim(cones[iCone]);
            int nColSqr = nCol * nCol;
            /* XSX / mu - X */
            rscl(&nColSqr, &dBarrierMu, dPrimalMatBuffer[iCone], &HIntConstantOne);
            axpy(&nColSqr, &HDblConstantMinusOne, dPrimalX[iCone], &HIntConstantOne, dPrimalMatBuffer[iCone], &HIntConstantOne);
            HConeComputeATimesXpy(cones[iCone], dPrimalMatBuffer[iCone], dPrimalKKTRhs);
        }
        
        for ( int iRow = 0; iRow < Hpsdp->nRows; ++iRow ) {
            dPrimalKKTRhs[iRow] += Hpsdp->rowRHS[iRow] - dPrimalInfeas[iRow];
        }
        
        /*
        HDSDP_ZERO(dPrimalKKTRhs, double, Hpsdp->nRows);
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone) {
            HConeComputeATimesXpy(cones[iCone], dPrimalMatBuffer[iCone], dPrimalKKTRhs);
        }
        
        for ( int iRow = 0; iRow < Hpsdp->nRows; ++iRow ) {
            dPrimalKKTRhs[iRow] -= dBarrierMu * (2.0 * dPrimalInfeas[iRow] - Hpsdp->rowRHS[iRow]);
        }
        */
         
        /* Solve the KKT system */
        HDSDP_CALL(HKKTSolve(Hpsdp->HKKT, dPrimalKKTRhs, dRowDualStep));
        
        /* Scale back dy */
        for ( int iRow = 0; iRow < Hpsdp->nRows; ++iRow ) {
            dRowDualStep[iRow] *= dBarrierMu;
        }
        
        /* Get Primal direction */
        /* Compute X * S * X + XScal * dS * XScal */
        /* Dual ratio test: necessary for getting dS from buffer */
        double dMaxDualStep = HDSDP_INFINITY;
        double dDualStep = 0.0;
        double oneOverMu = - 1.0 / dBarrierMu;
        
        iDualMat = 0;
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
            int nCol = HConeGetDim(cones[iCone]);
            int nColSqr = nCol * nCol;
            HDSDP_CALL(HConeRatioTest(cones[iCone], 0.0, dRowDualStep, 1.0, BUFFER_DUALVAR, &dDualStep));
            dMaxDualStep = HDSDP_MIN(dMaxDualStep, dDualStep);
            HDSDP_ZERO(dPrimalXStep[iCone], double, nCol * nCol);
            
            /* dX = - (XSX / mu - X) = X - XSX / mu */
            axpy(&nColSqr, &HDblConstantMinusOne, dPrimalMatBuffer[iCone],
                 &HIntConstantOne, dPrimalXStep[iCone], &HIntConstantOne);
            
            /* XScal * dS * XScal */
            HConeBuildPrimalXSXDirection(cones[iCone], Hpsdp->HKKT, dPrimalScalX[iCone], dPrimalMatBuffer2[iCone], iDualMat);
            
            /* X - 1 / mu * (X * S * X + XScal * dS * XScal) */
            axpy(&nColSqr, &oneOverMu, dPrimalMatBuffer2[iCone],
                 &HIntConstantOne, dPrimalXStep[iCone], &HIntConstantOne);
            
            /* Compute X - 1 / mu * (X * S * X + XScal * dS * XScal) */
            /*
            HConeBuildPrimalXSXDirection(cones[iCone], Hpsdp->HKKT, dPrimalScalX[iCone], dPrimalMatBuffer[iCone], iDualMat);
            HDSDP_MEMCPY(dPrimalXStep[iCone], dPrimalX[iCone], double, nCol * nCol);
            int nElem = nCol * nCol;
            axpy(&nElem, &oneOverMu, dPrimalMatBuffer[iCone], &HIntConstantOne, dPrimalXStep[iCone], &HIntConstantOne);
            
            */
        }
        
        /* Ratio test */
        double dMaxPrimalStep = HDSDP_INFINITY;
        double dPrimalStep = 0.0;
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
            Hpsdp->iLanczos = iCone;
            HDSDP_CALL(HLanczosSolve(Hpsdp->Lanczos[iCone], NULL, &dPrimalStep));
            dMaxPrimalStep = HDSDP_MIN(dMaxPrimalStep, dPrimalStep);
        }
        
        dMaxPrimalStep = HDSDP_MIN(0.5 * dMaxPrimalStep, 1.0);
        dMaxDualStep = HDSDP_MIN(0.5 * dMaxDualStep, 1.0);
        
        /* Take step */
        /* Dual step */
        axpy(&Hpsdp->nRows, &dMaxDualStep, dRowDualStep, &HIntConstantOne, dRowDual, &HIntConstantOne);
        
        /* Primal step */
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
            int nCol = HConeGetDim(cones[iCone]);
            int nElem = nCol * nCol;
            axpy(&nElem, &dMaxPrimalStep, dPrimalXStep[iCone], &HIntConstantOne, dPrimalX[iCone], &HIntConstantOne);
            HConeUpdate(cones[iCone], 1.0, dRowDual);
        }
        
        int isInterior = 0;
        
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
            HDSDP_CALL(HConeCheckIsInterior(cones[iCone], 1.0, dRowDual, &isInterior));
            if ( !isInterior ) {
                retcode = HDSDP_RETCODE_FAILED;
                goto exit_cleanup;
            }
        }
        
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
            HFpLinsysPsdCheck(Hpsdp->XFactors[iCone], NULL, NULL, dPrimalX[iCone], &isInterior);
            if ( !isInterior ) {
                retcode = HDSDP_RETCODE_FAILED;
                goto exit_cleanup;
            }
        }
        
        /* Update barrier parameter */
        double dDualObj = dot(&Hpsdp->nRows, Hpsdp->rowRHS, &HIntConstantOne, dRowDual, &HIntConstantOne);
        double dPrimalObj = 0.0;
        double dCompl = 0.0;
        for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
            dPrimalObj += HConeComputeTraceCX(cones[iCone], dPrimalX[iCone]);
            dCompl += HConeComputeXDotS(cones[iCone], dPrimalX[iCone]);
        }
        
        if ( dPrimalObj < dDualObj ) {
            retcode = HDSDP_RETCODE_FAILED;
            goto exit_cleanup;
        }
        
        double dBarrierTarget = (dPrimalObj - dDualObj) / (2.0 * dSumDims);
        
        if ( dBarrierMu < 1e-09 ) {
            dBarrierTarget = HDSDP_MIN(dBarrierMu, dCompl / dSumDims);
            dBarrierTarget = dBarrierTarget * (1 - 1.0 / sqrt(dSumDims));
        } else {
            dBarrierTarget = dBarrierTarget * (1 - 1.0 / sqrt(dSumDims));
            dBarrierMu = dBarrierTarget;
//            dBarrierMu = HDSDP_MIN((1 - 0.1 * dMaxPrimalStep * dMaxDualStep) * HSolver->dBarrierMu, dBarrierMu);
        }
        
//        hdsdp_printf("Barrier target: %e \n", dBarrierMu);
        
        /* Synchronize data to HSolver */
        HSolver->pObjInternal = dPrimalObj;
        HSolver->dObjInternal = dDualObj;
        
        HSolver->dObjVal = HSolver->dObjInternal * pdScal;
        HSolver->pObjVal = HSolver->pObjInternal * pdScal;
        HSolver->pInfeas = dPrimalInfeasNorm / (1 + get_dbl_feature(HSolver, DBL_FEATURE_RHSONENORM));
        HSolver->dBarrierMu = dBarrierMu;
        HSolver->dDStep = dMaxDualStep;
        
        double elapsedTime = HUtilGetTimeStamp() - HSolver->dTimeBegin;
        HSolver->nIterCount += 1;
        
        hdsdp_printf("    %5d  %+15.8e  %+15.8e  %8.2e  %8.2e  P:%5.2f D:%5.2f %4.1f \n", Hpsdp->HSolver->nIterCount + 1,
               HSolver->pObjVal, HSolver->dObjVal, HSolver->pInfeas, HSolver->dBarrierMu,
               dMaxPrimalStep, HSolver->dDStep, elapsedTime);
        
        if ( HSolver->comp < (fabs(HSolver->pObjVal) + fabs(HSolver->dObjVal) + 1.0) * dReloptTol &&
            HSolver->comp < dAbsoptTol / pdScal ) {
            HSolver->HStatus = HDSDP_PRIMAL_DUAL_OPTIMAL;
            break;
        }
        
        if ( nBadIter > 2 ) {
            break;
        }
        
        if ( (dMaxPrimalStep < 1e-02 && dMaxDualStep < 1e-02) || dMaxPrimalStep < 1e-03 ) {
            isInterior = 0;
            for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
                int nCol = HConeGetDim(Hpsdp->HCones[iCone]);
//                HUtilMatSymmetrize(nCol, Hpsdp->dPrimalX[iCone]);
                HDSDP_MEMCPY(Hpsdp->dPrimalScalX[iCone], Hpsdp->dPrimalX[iCone], double, nCol * nCol);
                HFpLinsysPsdCheck(Hpsdp->XFactors[iCone], NULL, NULL, Hpsdp->dPrimalX[iCone], &isInterior);
                if ( !isInterior ) {
                    retcode = HDSDP_RETCODE_FAILED;
                    goto exit_cleanup;
                }
            }
            
            HDSDP_CALL(HKKTBuildUp(Hpsdp->HKKT, KKT_TYPE_PRIMAL));
            HKKTRegularize(HSolver->HKKT, 1e-16);
            HDSDP_CALL(HKKTFactorize(Hpsdp->HKKT));
            
            nBadIter += 1;
        }
        
        if ( dCompl > 10 * HSolver->comp ) {
            break;
        }
        
        if ( HSolver->pInfeas > 1e-06 && HSolver->pInfeas > 1e-08 ) {
            retcode = HDSDP_RETCODE_FAILED;
            break;
        }
        
        HSolver->comp = dCompl;
    }
    
#if 0
    double dPDGap = HSolver->pObjVal - HSolver->dObjVal;
    dPDGap = dPDGap / (fabs(HSolver->pObjVal) + fabs(HSolver->dObjVal) + 1.0);
    double dPrimalInf = dPrimalInfeasNorm / (1.0 + get_dbl_feature(HSolver, DBL_FEATURE_RHSONENORM));
    double dCompl = HSolver->comp;
    dCompl = dCompl / (fabs(HSolver->pObjVal) + fabs(HSolver->dObjVal) + 1.0);
    
    hdsdp_printf("Primal solver ends. Primal. inf: %10.5e  Gap: %10.5e  Compl: %10.5e \n", dPrimalInf, dPDGap, dCompl);
#endif
    
exit_cleanup:
    
    if ( retcode != HDSDP_RETCODE_OK ) {
        hdsdp_printf("Primal method fails. Switch back to dual method.\n");
        HPSDPIRecover(Hpsdp);
    }
    
    return retcode;
}

extern void HPSDPGetSolution( hdsdp_psdp *Hpsdp, int iCone, double *dPrimalX ) {

    int nCol = HConeGetDim(Hpsdp->HCones[iCone]);
    HDSDP_MEMCPY(dPrimalX, Hpsdp->dPrimalX[iCone], double, nCol * nCol);
    
    return;
}

extern void HPSDPClear( hdsdp_psdp *Hpsdp ) {
    
    if ( !Hpsdp ) {
        return;
    }
    
    HDSDP_FREE(Hpsdp->dPrimalKKTRhs);
    
    for ( int iCone = 0; iCone < Hpsdp->nCones; ++iCone ) {
        HLanczosDestroy(&Hpsdp->Lanczos[iCone]);
        HFpLinsysDestroy(&Hpsdp->XFactors[iCone]);
        HDSDP_FREE(Hpsdp->dPrimalX[iCone]);
        HDSDP_FREE(Hpsdp->dPrimalXStep[iCone]);
        HDSDP_FREE(Hpsdp->dPrimalScalX[iCone]);
        HDSDP_FREE(Hpsdp->dPrimalMatBuffer[iCone]);
        HDSDP_FREE(Hpsdp->dPrimalMatBuffer2[iCone]);
    }
    
    HDSDP_FREE(Hpsdp->dPrimalX);
    HDSDP_FREE(Hpsdp->dPrimalScalX);
    HDSDP_FREE(Hpsdp->dPrimalXStep);
    HDSDP_FREE(Hpsdp->dPrimalMatBuffer);
    HDSDP_FREE(Hpsdp->dPrimalMatBuffer2);
    
    HDSDP_FREE(Hpsdp->dPrimalAuxiVec1);
    HDSDP_FREE(Hpsdp->dPrimalAuxiVec2);
    HDSDP_FREE(Hpsdp->Lanczos);
    HDSDP_FREE(Hpsdp->XFactors);
    
    return;
}

extern void HPSDPDestroy( hdsdp_psdp **pHpsdp ) {
    
    if ( !pHpsdp ) {
        return;
    }
    
    HPSDPClear(*pHpsdp);
    HDSDP_FREE(*pHpsdp);
    
    return;
}
