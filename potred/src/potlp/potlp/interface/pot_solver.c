#include "pot_def.h"
#include "pot_param.h"
#include "pot_structs.h"
#include "pot_vector.h"
#include "pot_lanczos.h"
#include "pot_utils.h"

#include <math.h>

extern pot_int potLPCreate( pot_solver **ppot ) {
    
    pot_int retcode = RETCODE_OK;
    
    pot_solver *pot = NULL;
    POTLP_INIT(pot, pot_solver, 1);
    
    if (!pot) {
        retcode = RETCODE_FAILED;
        goto exit_cleanup;
    }
    
    memset(pot, 0, sizeof(pot_solver));
    memcpy(pot->dblParams, defaultDblParam, sizeof(double) * NUM_DBL_PARAM);
    memcpy(pot->intParams, defaultIntParam, sizeof(int) * NUM_INT_PARAM);
    
    pot->potVal = POTLP_INFINITY;
    pot->zVal = 0.0;
    
    POT_CALL(potLanczosCreate(&pot->lczTool));
    
    *ppot = pot;
    
exit_cleanup:
    return retcode;
}

extern pot_int potLPInit( pot_solver *pot, pot_int vDim, pot_int vConeDim ) {
    
    pot_int retcode = RETCODE_OK;
    
    if ( pot->potVal != POTLP_INFINITY || pot->xVec || pot->xVecOld ||
         pot->gVec || pot->mkVec || pot->xStepVec || pot->HessMat ||
         pot->auxVec1 || pot->auxVec2 ) {
        goto exit_cleanup;
    }

    POT_CALL(potVecCreate(&pot->xVec));
    POT_CALL(potVecInit(pot->xVec, vDim, vConeDim));
    
    POT_CALL(potVecCreate(&pot->xVecOld));
    POT_CALL(potVecInit(pot->xVecOld, vDim, vConeDim));
    
    POT_CALL(potVecCreate(&pot->gVec));
    POT_CALL(potVecInit(pot->gVec, vDim, vConeDim));
    
    POT_CALL(potVecCreate(&pot->gkVec));
    POT_CALL(potVecInit(pot->gkVec, vDim, vConeDim));
    
    POT_CALL(potVecCreate(&pot->mkVec));
    POT_CALL(potVecInit(pot->mkVec, vDim, vConeDim));
    
    POT_CALL(potVecCreate(&pot->xStepVec));
    POT_CALL(potVecInit(pot->xStepVec, vDim, vConeDim));
    
    POT_CALL(potVecCreate(&pot->auxVec1));
    POT_CALL(potVecInit(pot->auxVec1, vDim, vConeDim));
    
    POT_CALL(potVecCreate(&pot->auxVec2));
    POT_CALL(potVecInit(pot->auxVec2, vDim, vConeDim));
    
    pot->rhoVal = vConeDim + sqrt(vConeDim);
    POT_CALL(potLanczosInit(pot->lczTool, vDim));
    
#ifdef POT_DEBUG
    POTLP_INIT(pot->HessMat, double, vDim * vDim);
    if (!pot->HessMat) {
        retcode = RETCODE_FAILED;
    }
#endif
    
exit_cleanup:
    return retcode;
}

extern pot_int potLPSetObj( pot_solver *pot, pot_fx *objFunc ) {
    
    pot_int retcode = RETCODE_OK;
    
    if (pot->objFunc) {
        retcode = RETCODE_FAILED;
        goto exit_cleanup;
    }
    
    pot->objFunc = objFunc;

exit_cleanup:
    return retcode;
}

extern pot_int potLPSetLinearConstrs( pot_solver *pot, pot_constr_mat *AMat ) {
    
    pot_int retcode = RETCODE_OK;
    
    if (pot->AMat) {
        retcode = RETCODE_FAILED;
        goto exit_cleanup;
    }
    
    pot->AMat = AMat;
    
exit_cleanup:
    return retcode;
}

extern void potLPClear( pot_solver *pot ) {
    
    if (!pot) {
        return;;
    }
    
    pot->objFunc = NULL;
    pot->AMat = NULL;
    pot->fVal = POTLP_INFINITY;
    
    potVecDestroy(&pot->xVec);
    potVecDestroy(&pot->xVecOld);
    potVecDestroy(&pot->gVec);
    potVecDestroy(&pot->mkVec);
    potVecDestroy(&pot->xStepVec);
    potVecDestroy(&pot->auxVec1);
    potVecDestroy(&pot->auxVec2);
    
    pot->objFunc = NULL;
    pot->AMat = NULL;
    
    potLanczosDestroy(&pot->lczTool);
    
    POTLP_FREE(pot->HessMat);
    memset(pot, 0, sizeof(pot_solver));
    
    return;
}

extern void potLPDestroy( pot_solver **ppot ) {
    
    if (!ppot) {
        return;
    }
    
    potLPClear(*ppot);
    POTLP_FREE(*ppot);
    
    return;
}