/*
 * -----------------------------------------------------------------
 * $Revision: 1.26 $
 * $Date: 2004-09-14 21:23:47 $
 * ----------------------------------------------------------------- 
 * Programmers: Alan C. Hindmarsh and Radu Serban @ LLNL
 * -----------------------------------------------------------------
 * Copyright (c) 2002, The Regents of the University of California  
 * Produced at the Lawrence Livermore National Laboratory
 * All rights reserved
 * For details, see sundials/ida/LICENSE
 * -----------------------------------------------------------------
 * This is the implementation file for the main IDA solver.
 * It is independent of the linear solver in use.
 * -----------------------------------------------------------------
 */

/************************************************************/
/******************* BEGIN Imports **************************/
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "ida_impl.h"
#include "sundialstypes.h"
#include "nvector.h"
#include "sundialsmath.h"

/************************************************************/
/******************** END Imports ***************************/
/************************************************************/


/***************************************************************/
/*********************** BEGIN Macros **************************/
/**************************************************************/

/* Macro: loop */

#define loop for(;;)

/***************************************************************/
/************************ END Macros ***************************/
/***************************************************************/



/************************************************************/
/************** BEGIN IDA Private Constants ***************/
/************************************************************/

#define ZERO       RCONST(0.0)    /* real 0.0    */
#define HALF       RCONST(0.5)    /* real 0.5    */
#define QUARTER    RCONST(0.25)   /* real 0.25   */
#define TWOTHIRDS  RCONST(0.667)  /* real 2/3 for default steptol */
#define ONE        RCONST(1.0)    /* real 1.0    */
#define ONEPT5     RCONST(1.5)    /* real 1.5    */
#define TWO        RCONST(2.0)    /* real 2.0    */
#define TWOPT5     RCONST(2.5)    /* real 2.5    */
#define TEN        RCONST(10.0)   /* real 10.0   */
#define TWELVE     RCONST(12.0)   /* real 12.0   */
#define TWENTY     RCONST(20.0)   /* real 20.0   */
#define HUNDRED    RCONST(100.0)  /* real 100.0  */
#define PT9        RCONST(0.9)    /* real 0.9    */
#define PT99       RCONST(0.99)   /* real 0.99   */
#define PT1        RCONST(0.1)    /* real 0.1    */
#define PT01       RCONST(0.01)   /* real 0.01   */
#define PT001      RCONST(0.001)  /* real 0.001  */
#define PT0001     RCONST(0.0001) /* real 0.0001 */

/***************************************************************/
/************** BEGIN Default Constants ************************/
/***************************************************************/

#define MXSTEP_DEFAULT   500   /* mxstep default value   */
#define MAXORD_DEFAULT   5     /* maxord default value   */
#define MXNCF           10     /* max number of convergence failures allowed */
#define MXNEF           10     /* max number of error test failures allowed  */
#define EPCON      RCONST(0.33)   /* Newton convergence test constant */
#define MAXNH      5    /* max. number of h tries in IC calc. */
#define MAXNJ      4    /* max. number of J tries in IC calc. */
#define MAXNI      10   /* max. Newton iterations in IC calc. */

/***************************************************************/
/*************** END Default Constants *************************/
/***************************************************************/


/***************************************************************/
/************ BEGIN Routine-Specific Constants *****************/
/***************************************************************/

 /* IDASolve return values are defined by an enum statement in the 
    include file ida.h.  They are: 
 
       (successful returns:)
 
       NORMAL_RETURN
       INTERMEDIATE_RETURN
       TSTOP_RETURN
 
       (failed returns:)

       IDA_NO_MEM
       ILL_INPUT
       TOO_MUCH_WORK
       TOO_MUCH_ACC
       ERR_FAILURE
       CONV_FAILURE
       SETUP_FAILURE
       SOLVE_FAILURE
       CONSTR_FAILURE   
       REP_RES_REC_ERR
       RES_NONREC_ERR
                       */

/* IDAStep return values */

/* The first three IDAStep return values are defined in an enum statement
   given in file ida.h. They are listed below for continuity with the other
   return values. The next one is defined here as an alias for the value
   defined in ida.h (LSOLVE_ERROR_NONRECVR) */

/* SUCCESS                      0 */
/* RES_ERROR_NONRECVR          -1 */
/* LSETUP_ERROR_NONRECVR       -2 */

#define CONV_FAIL_LINR_NONRECVR  LSOLVE_ERROR_NONRECVR   /* = -3 */

#define REP_ERR_FAIL           -4 
#define CONSTR_FAIL            -5
#define REP_CONV_FAIL          -6
#define REP_RES_ERR            -7


/* IDACalcIC control constants */

#define ICRATEMAX  RCONST(0.9)    /* max. Newton conv. rate */
#define ALPHALS    RCONST(0.0001) /* alpha in linesearch conv. test */


/* IDAStep control constants */

#define PREDICT_AGAIN    20


/* IDANewtonIter constants */

#define RATEMAX RCONST(0.9)
#define MAXIT    4


/* Return values from various low-level routines */

/* Return values for lower level routines used by IDACalcIC */

enum { IC_FAIL_RECOV = 1,  IC_CONSTR_FAILED = 2,  IC_LINESRCH_FAILED = 3,
       IC_CONV_FAIL =  4,  IC_SLOW_CONVRG =   5 };

/* The following two return values are set by an enumeration in ida.h.
   They are included here for clarity. The values are potentially
   encountered in:  'res', 'lsetup', 'lsolve', IDANewtonIter, IDAnls,
   IDAHandleNFlag, IDAStep, and IDASolve. The third value is defined
   as an alias for the value returned from lsolve (LSOLVE_ERROR_RECVR) */

/*RES_ERROR_RECVR              +1    */
/*LSETUP_ERROR_RECVR           +2    */

#define CONV_FAIL_LINR_RECVR         LSOLVE_ERROR_RECVR  /*  = +3  */

#define CONV_FAIL_NLINR_RECVR  +4 /* IDANewtonIter, IDAnls                   */
#define CONSTRAINT_FAIL_RECVR  +5 /* IDAnls                                  */
#define CONTINUE_STEPS        +99 /* IDASolve, IDAStopTest1, IDAStopTest2    */

#define UNSET -1    /* IDACompleteStep */
#define LOWER 1     /* IDACompleteStep */
#define RAISE 2     /* IDACompleteStep */
#define MAINTAIN 3  /* IDACompleteStep */

#define ERROR_TEST_FAIL       +7

#define XRATE                RCONST(0.25)        


/***************************************************************/
/*************** END Routine-Specific Constants  ***************/
/***************************************************************/


/***************************************************************/
/***************** BEGIN Error Messages ************************/
/***************************************************************/

/* IDACreate error messages */

#define MSG_IDAMEM_FAIL      "IDACreate-- Allocation of ida_mem failed. \n\n"

/* IDASet* error messages */

#define MSG_IDAS_NO_MEM      "ida_mem=NULL in an IDASet routine illegal. \n\n"

#define MSG_IDAS_NEG_MAXORD  "IDASetMaxOrd-- maxord<=0 illegal. \n\n"

#define MSG_IDAS_BAD_MAXORD1 "IDASetMaxOrd-- Illegal attempt to increase "
#define MSG_IDAS_BAD_MAXORD2 "maximum method order from %d to %d.\n\n"
#define MSG_IDAS_BAD_MAXORD  MSG_IDAS_BAD_MAXORD1 MSG_IDAS_BAD_MAXORD2 

#define MSG_IDAS_NEG_MXSTEPS "IDASetMaxNumSteps-- mxsteps<=0 illegal. \n\n"

#define MSG_IDAS_NEG_HMAX    "IDASetMaxStep-- hmax<=0 illegal. \n\n"

#define MSG_IDAS_NEG_EPCON   "IDASetNonlinConvCoef-- epcon < 0.0 illegal. \n\n"

#define MSG_IDAS_BAD_EPICCON "IDASetNonlinConvcoefIC-- epiccon < 0.0 illegal.\n\n"

#define MSG_IDAS_BAD_MAXNH   "IDASetMaxNumStepsIC-- maxnh < 0 illegal.\n\n"

#define MSG_IDAS_BAD_MAXNJ   "IDASetMaxNumJacsIC-- maxnj < 0 illegal.\n\n"

#define MSG_IDAS_BAD_MAXNIT  "IDASetMaxNumItersIC-- maxnit < 0 illegal.\n\n"

#define MSG_IDAS_BAD_STEPTOL "IDASetLineSearchOffIC-- steptol < 0.0 illegal.\n\n"

/* IDAMalloc/IDAReInit error messages */

#define IDAM               "IDAMalloc/IDAReInit-- "

#define MSG_IDAM_NO_MEM    IDAM "ida_mem = NULL illegal.\n\n"

#define MSG_Y0_NULL        IDAM "y0 = NULL illegal.\n\n"
#define MSG_YP0_NULL       IDAM "yp0 = NULL illegal.\n\n"

#define MSG_BAD_ITOL1      IDAM "itol = %d illegal.\n"
#define MSG_BAD_ITOL2      "The legal values are SS = %d and SV = %d.\n\n"
#define MSG_BAD_ITOL       MSG_BAD_ITOL1 MSG_BAD_ITOL2

#define MSG_RES_NULL       IDAM "res = NULL illegal.\n\n"

#define MSG_RTOL_NULL      IDAM "rtol = NULL illegal.\n\n"

#define MSG_BAD_RTOL       IDAM "*rtol = %g < 0 illegal.\n\n"

#define MSG_ATOL_NULL      IDAM "atol = NULL illegal.\n\n"

#define MSG_BAD_ATOL       IDAM "Some atol component < 0.0 illegal.\n\n"

#define MSG_BAD_NVECTOR    IDAM "A required vector operation is not implemented.\n\n"

#define MSG_MEM_FAIL       IDAM "A memory request failed.\n\n"

#define MSG_REI_NO_MALLOC  "IDAReInit-- Attempt to call before IDAMalloc. \n\n"

/* IDAInitialSetup error messages -- called from IDACalcIC or IDASolve */

#define IDAIS              "Initial setup-- "

#define MSG_MISSING_ID      IDAIS "id = NULL but suppressalg option on.\n\n"

#define MSG_BAD_EWT         IDAIS "Some initial ewt component = 0.0 illegal.\n\n"

#define MSG_BAD_CONSTRAINTS IDAIS "illegal values in constraints vector.\n\n"

#define MSG_Y0_FAIL_CONSTR  IDAIS "y0 fails to satisfy constraints.\n\n"

#define MSG_LINIT_NULL      IDAIS "The linear solver's init routine is NULL.\n\n"

#define MSG_LSETUP_NULL     IDAIS "The linear solver's setup routine is NULL.\n\n"

#define MSG_LSOLVE_NULL     IDAIS "The linear solver's solve routine is NULL.\n\n"

#define MSG_LFREE_NULL      IDAIS "The linear solver's free routine is NULL.\n\n"

#define MSG_LINIT_FAIL      IDAIS "The linear solver's init routine failed.\n\n"

/* IDACalcIC error messages */

#define IDAIC              "IDACalcIC-- "

#define MSG_IC_IDA_NO_MEM  IDAIC "IDA_mem = NULL illegal.\n\n"

#define MSG_IC_NO_MALLOC   IDAIC "Attempt to call before IDAMalloc. \n\n"
 
#define MSG_BAD_ICOPT      IDAIC "icopt = %d is illegal.\n\n"

#define MSG_IC_MISSING_ID  IDAIC "id = NULL conflicts with icopt.\n\n"

#define MSG_IC_BAD_ID      IDAIC "id has illegal values.\n\n"

#define MSG_IC_TOO_CLOSE1  IDAIC "tout1 = %g too close to t0 = %g to attempt"
#define MSG_IC_TOO_CLOSE2  " initial condition calculation.\n\n"
#define MSG_IC_TOO_CLOSE   MSG_IC_TOO_CLOSE1 MSG_IC_TOO_CLOSE2

#define MSG_IC_LINIT_FAIL  IDAIC "The linear solver's init routine failed.\n\n"

#define MSG_IC_BAD_EWT     IDAIC "Some ewt component = 0.0 illegal.\n\n"

#define MSG_IC_RES_NONR1   IDAIC "Non-recoverable error return from"
#define MSG_IC_RES_NONR2   " ResFn residual routine. \n\n"
#define MSG_IC_RES_NONREC  MSG_IC_RES_NONR1 MSG_IC_RES_NONR2

#define MSG_IC_RES_FAIL1   IDAIC "Recoverable error in first call to"
#define MSG_IC_RES_FAIL2   " ResFn residual routine. Cannot recover. \n\n"
#define MSG_IC_RES_FAIL    MSG_IC_RES_FAIL1 MSG_IC_RES_FAIL2

#define MSG_IC_SETUP_FL1   IDAIC "The linear solver setup routine"
#define MSG_IC_SETUP_FL2   " failed non-recoverably.\n\n"
#define MSG_IC_SETUP_FAIL  MSG_IC_SETUP_FL1 MSG_IC_SETUP_FL2

#define MSG_IC_SOLVE_FL1   IDAIC "The linear solver solve routine"
#define MSG_IC_SOLVE_FL2   " failed non-recoverably.\n\n"
#define MSG_IC_SOLVE_FAIL  MSG_IC_SOLVE_FL1 MSG_IC_SOLVE_FL2

#define MSG_IC_NO_RECOV1   IDAIC "The residual routine or the linear"
#define MSG_IC_NO_RECOV2   " setup or solve routine had a recoverable"
#define MSG_IC_NO_RECOV3   " error, but IDACalcIC was unable to recover.\n\n"
#define MSG_IC_NO_RECOVERY MSG_IC_NO_RECOV1 MSG_IC_NO_RECOV2 MSG_IC_NO_RECOV3

#define MSG_IC_FAIL_CON1   IDAIC "Unable to satisfy the inequality"
#define MSG_IC_FAIL_CON2   " constraints.\n\n"
#define MSG_IC_FAIL_CONSTR MSG_IC_FAIL_CON1 MSG_IC_FAIL_CON2

#define MSG_IC_FAILED_LS1  IDAIC "The Linesearch algorithm failed"
#define MSG_IC_FAILED_LS2  " with too small a step.\n\n"
#define MSG_IC_FAILED_LINS MSG_IC_FAILED_LS1 MSG_IC_FAILED_LS2

#define MSG_IC_CONV_FAIL1  IDAIC "Failed to get convergence in"
#define MSG_IC_CONV_FAIL2  " Newton/Linesearch algorithm.\n\n"
#define MSG_IC_CONV_FAILED MSG_IC_CONV_FAIL1 MSG_IC_CONV_FAIL2

/* IDASolve error messages */

#define IDASLV             "IDASolve-- "

#define MSG_IDA_NO_MEM     IDASLV "IDA_mem = NULL illegal.\n\n"

#define MSG_NO_MALLOC      IDASLV "Attempt to call before IDAMalloc. \n\n"
 
#define MSG_BAD_HINIT      IDASLV "hinit=%g and tout-t0=%g inconsistent.\n\n"

#define MSG_BAD_TOUT1      IDASLV "Trouble interpolating at tout = %g.\n"
#define MSG_BAD_TOUT2      "tout too far back in direction of integration.\n\n"
#define MSG_BAD_TOUT       MSG_BAD_TOUT1 MSG_BAD_TOUT2

#define MSG_BAD_TSTOP1     IDASLV "tstop = %g is behind  current t = %g \n"
#define MSG_BAD_TSTOP2     "in the direction of integration.\n\n"
#define MSG_BAD_TSTOP      MSG_BAD_TSTOP1 MSG_BAD_TSTOP2


#define MSG_MAX_STEPS1     IDASLV "At t=%g, mxstep=%d steps taken on "
#define MSG_MAX_STEPS2     "this call before\nreaching tout=%g.\n\n"
#define MSG_MAX_STEPS      MSG_MAX_STEPS1 MSG_MAX_STEPS2

#define MSG_EWT_NOW_BAD1   IDASLV "At t=%g, "
#define MSG_EWT_NOW_BAD2   "some ewt component has become <= 0.0.\n\n"
#define MSG_EWT_NOW_BAD    MSG_EWT_NOW_BAD1 MSG_EWT_NOW_BAD2

#define MSG_TOO_MUCH_ACC   IDASLV "At t=%g, too much accuracy requested.\n\n"

#define MSG_ERR_FAILS1     IDASLV "At t=%g and step size h=%g, the error test\n"
#define MSG_ERR_FAILS2     "failed repeatedly or with |h| = hmin.\n\n"
#define MSG_ERR_FAILS      MSG_ERR_FAILS1 MSG_ERR_FAILS2

#define MSG_CONV_FAILS1    IDASLV "At t=%g and step size h=%g, the corrector\n"
#define MSG_CONV_FAILS2    "convergence failed repeatedly.\n\n"
#define MSG_CONV_FAILS     MSG_CONV_FAILS1 MSG_CONV_FAILS2

#define MSG_SETUP_FAILED1  IDASLV "At t=%g, the linear solver setup routine "
#define MSG_SETUP_FAILED2  "failed in an unrecoverable manner.\n\n"
#define MSG_SETUP_FAILED   MSG_SETUP_FAILED1 MSG_SETUP_FAILED2

#define MSG_SOLVE_FAILED1  IDASLV "At t=%g, the linear solver solve routine "
#define MSG_SOLVE_FAILED2  "failed in an unrecoverable manner.\n\n"
#define MSG_SOLVE_FAILED   MSG_SOLVE_FAILED1 MSG_SOLVE_FAILED2

#define MSG_TOO_CLOSE1     IDASLV "tout=%g too close to t0=%g to start"
#define MSG_TOO_CLOSE2     " integration.\n\n"
#define MSG_TOO_CLOSE      MSG_TOO_CLOSE1 MSG_TOO_CLOSE2

#define MSG_YRET_NULL      IDASLV "yret=NULL illegal.\n\n"
#define MSG_YPRET_NULL     IDASLV "ypret=NULL illegal.\n\n"
#define MSG_TRET_NULL      IDASLV "tret=NULL illegal.\n\n"

#define MSG_BAD_ITASK      IDASLV "itask=%d illegal.\n\n"

#define MSG_NO_TSTOP1      IDASLV "itask = NORMAL_TSTOP or itask = ONE_STEP_TSTOP "
#define MSG_NO_TSTOP2      "but tstop was not set.\n\n"
#define MSG_NO_TSTOP       MSG_NO_TSTOP1 MSG_NO_TSTOP2

#define MSG_REP_RES_ERR1   IDASLV "At t = %g, repeated recoverable error \n"
#define MSG_REP_RES_ERR2   "returns from ResFn residual function. \n\n"
#define MSG_REP_RES_ERR    MSG_REP_RES_ERR1 MSG_REP_RES_ERR2

#define MSG_RES_NONRECOV1  IDASLV "At t = %g, nonrecoverable error \n"
#define MSG_RES_NONRECOV2  "return from ResFn residual function. \n\n"
#define MSG_RES_NONRECOV   MSG_RES_NONRECOV1 MSG_RES_NONRECOV2

#define MSG_FAILED_CONSTR1 IDASLV "At t = %g, unable to satisfy \n"
#define MSG_FAILED_CONSTR2 "inequality constraints. \n\n"
#define MSG_FAILED_CONSTR  MSG_FAILED_CONSTR1 MSG_FAILED_CONSTR2

/* IDAGet Error Messages */

#define MSG_IDAG_NO_MEM "ida_mem=NULL in an IDAGet routine illegal. \n\n"

#define MSG_BAD_T1      "IDAGetSolution-- t=%g illegal.\n"
#define MSG_BAD_T2      "t not in interval tcur-hu=%g to tcur=%g.\n\n"
#define MSG_BAD_T       MSG_BAD_T1 MSG_BAD_T2

/***************************************************************/
/****************** END Error Messages *************************/
/***************************************************************/


/************************************************************/
/*************** END IDA Private Constants ****************/
/************************************************************/


/**************************************************************/
/********* BEGIN Private Helper Functions Prototypes **********/
/**************************************************************/

static booleantype IDACheckNvector(N_Vector tmpl);

static booleantype IDAAllocVectors(IDAMem IDA_mem, N_Vector tmpl);
static void IDAFreeVectors(IDAMem IDA_mem);

static int IDAnlsIC (IDAMem IDA_mem);
static int IDANewtonIC (IDAMem IDA_mem);
static int IDALineSrch (IDAMem IDA_mem, realtype *delnorm, realtype *fnorm);
static int IDAfnorm (IDAMem IDA_mem, realtype *fnorm);
static int IDANewyyp (IDAMem IDA_mem, realtype lambda);
static int IDANewy (IDAMem IDA_mem);
static int IDAICFailFlag (IDAMem IDA_mem, int retval);

static int IDAInitialSetup(IDAMem IDA_mem);

static booleantype IDAEwtSet(IDAMem IDA_mem, N_Vector ycur);
static booleantype IDAEwtSetSS(IDAMem IDA_mem, N_Vector ycur);
static booleantype IDAEwtSetSV(IDAMem IDA_mem, N_Vector ycur);

static int IDAStopTest1(IDAMem IDA_mem, realtype tout,realtype *tret, 
                        N_Vector yret, N_Vector ypret, int itask);
static int IDAStopTest2(IDAMem IDA_mem, realtype tout, realtype *tret, 
                        N_Vector yret, N_Vector ypret, int itask);
static int IDAHandleFailure(IDAMem IDA_mem, int sflag);

static int IDAStep(IDAMem IDA_mem);
static void IDASetCoeffs(IDAMem IDA_mem, realtype *ck);
static int IDAnls(IDAMem IDA_mem);
static int IDAPredict(IDAMem IDA_mem);
static int IDANewtonIter(IDAMem IDA_mem);
static int IDATestError(IDAMem IDA_mem, realtype *ck, realtype *est,
                        realtype *terk, realtype *terkm1, realtype *erkm1);
static int IDAHandleNFlag(IDAMem IDA_mem, int nflag, realtype saved_t,
                          int *ncfPtr, int *nefPtr, realtype *est);
static int IDACompleteStep(IDAMem IDA_mem, realtype *est, 
                           realtype *terk, realtype *terkm1, realtype *erkm1);

static realtype IDAWrmsNorm(IDAMem IDA_mem, N_Vector x, N_Vector w, 
                            booleantype mask);

/**************************************************************/
/********** END Private Helper Functions Prototypes ***********/
/**************************************************************/



/***************************************************************/
/************* BEGIN IDA Implementation ************************/
/***************************************************************/


/***************************************************************/
/********* BEGIN Exported Functions Implementation *************/
/***************************************************************/

/*------------------     IDACreate     --------------------------*/
/* 
   IDACreate creates an internal memory block for a problem to 
   be solved by IDA.
   If successful, IDACreate returns a pointer to the problem memory. 
   This pointer should be passed to IDAMalloc.  
   If an initialization error occurs, IDACreate prints an error 
   message to standard err and returns NULL. 
*/
/*-----------------------------------------------------------------*/

void *IDACreate(void)
{
  IDAMem IDA_mem;

  IDA_mem = (IDAMem) malloc(sizeof(struct IDAMemRec));
  if (IDA_mem == NULL) {
    fprintf(stderr, MSG_MEM_FAIL);
    return (NULL);
  }

  /* Set unit roundoff in IDA_mem */
  IDA_mem->ida_uround = UNIT_ROUNDOFF;

  /* Set default values for integrator optional inputs */
  IDA_mem->ida_rdata       = NULL;
  IDA_mem->ida_errfp       = stderr;
  IDA_mem->ida_maxord      = MAXORD_DEFAULT;
  IDA_mem->ida_mxstep      = MXSTEP_DEFAULT;
  IDA_mem->ida_hmax_inv    = ZERO;
  IDA_mem->ida_hin         = ZERO;
  IDA_mem->ida_epcon       = EPCON;
  IDA_mem->ida_maxnef      = MXNEF;
  IDA_mem->ida_maxncf      = MXNCF;
  IDA_mem->ida_maxcor      = MAXIT;
  IDA_mem->ida_suppressalg = FALSE;
  IDA_mem->ida_id          = NULL;
  IDA_mem->ida_constraints = NULL;
  IDA_mem->ida_tstopset    = FALSE;

  /* Set default values for IC optional inputs */
  IDA_mem->ida_epiccon = PT01 * EPCON;
  IDA_mem->ida_maxnh   = MAXNH;
  IDA_mem->ida_maxnj   = MAXNJ;
  IDA_mem->ida_maxnit  = MAXNI;
  IDA_mem->ida_lsoff   = FALSE;
  IDA_mem->ida_steptol = RPowerR(IDA_mem->ida_uround, TWOTHIRDS);

  /* No mallocs have been done yet */
  IDA_mem->ida_MallocDone = FALSE;

  /* Return pointer to IDA memory block */
  return((void *)IDA_mem);
}


/*-----------------------------------------------------------------*/

int IDASetRdata(void *ida_mem, void *rdata)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_rdata = rdata;

  return(SUCCESS);
}

#define rdata (IDA_mem->ida_rdata)

/*-----------------------------------------------------------------*/

int IDASetErrFile(void *ida_mem, FILE *errfp)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_errfp = errfp;

  return(SUCCESS);
}

#define errfp (IDA_mem->ida_errfp)

/*-----------------------------------------------------------------*/

int IDASetMaxOrd(void *ida_mem, int maxord)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  if (maxord <= 0) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_NEG_MAXORD);
    return(IDAS_ILL_INPUT);
  }

  if (maxord > IDA_mem->ida_maxord) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_BAD_MAXORD, IDA_mem->ida_maxord, maxord);
    return(IDAS_ILL_INPUT);
  }  

  IDA_mem->ida_maxord = maxord;

  return(SUCCESS);
}

#define maxord (IDA_mem->ida_maxord)

/*-----------------------------------------------------------------*/

int IDASetMaxNumSteps(void *ida_mem, long int mxsteps)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  if (mxsteps <= 0) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_NEG_MXSTEPS);
    return(IDAS_ILL_INPUT);
  }

  IDA_mem->ida_mxstep = mxsteps;

  return(SUCCESS);
}

#define mxstep (IDA_mem->ida_mxstep)

/*-----------------------------------------------------------------*/

int IDASetInitStep(void *ida_mem, realtype hin)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_hin = hin;

  return(SUCCESS);
}

#define hin (IDA_mem->ida_hin)

/*-----------------------------------------------------------------*/

int IDASetMaxStep(void *ida_mem, realtype hmax)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  if (hmax <= 0) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_NEG_HMAX);
    return(IDAS_ILL_INPUT);
  }

  IDA_mem->ida_hmax_inv = ONE/hmax;

  return(SUCCESS);
}

#define hmax_inv (IDA_mem->ida_hmax_inv)

/*-----------------------------------------------------------------*/

int IDASetStopTime(void *ida_mem, realtype tstop)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_tstop = tstop;
  IDA_mem->ida_tstopset = TRUE;

  return(SUCCESS);
}

#define tstop (IDA_mem->ida_tstop)
#define tstopset (IDA_mem->ida_tstopset)

/*-----------------------------------------------------------------*/

int IDASetNonlinConvCoef(void *ida_mem, realtype epcon)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  if (epcon < ZERO) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_NEG_EPCON);
    return(IDAS_ILL_INPUT);
  }

  IDA_mem->ida_epcon = epcon;

  return(SUCCESS);
}

#define epcon (IDA_mem->ida_epcon)

/*-----------------------------------------------------------------*/

int IDASetMaxErrTestFails(void *ida_mem, int maxnef)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return (IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_maxnef = maxnef;

  return(SUCCESS);
}

#define maxnef (IDA_mem->ida_maxnef)

/*-----------------------------------------------------------------*/

int IDASetMaxConvFails(void *ida_mem, int maxncf)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return (IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_maxncf = maxncf;

  return(SUCCESS);
}

#define maxncf (IDA_mem->ida_maxncf)

/*-----------------------------------------------------------------*/

int IDASetMaxNonlinIters(void *ida_mem, int maxcor)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return (IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_maxcor = maxcor;

  return(SUCCESS);
}

#define maxcor (IDA_mem->ida_maxcor)

/*-----------------------------------------------------------------*/

int IDASetSuppressAlg(void *ida_mem, booleantype suppressalg)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_suppressalg = suppressalg;

  return(SUCCESS);
}

#define suppressalg (IDA_mem->ida_suppressalg)

/*-----------------------------------------------------------------*/

int IDASetId(void *ida_mem, N_Vector id)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_id = id;

  return(SUCCESS);
}

#define id (IDA_mem->ida_id)

/*-----------------------------------------------------------------*/

int IDASetConstraints(void *ida_mem, N_Vector constraints)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_constraints = constraints;

  return(SUCCESS);
}

#define constraints (IDA_mem->ida_constraints)

/******************** IDAMalloc *******************************

 IDAMalloc allocates and initializes memory for a problem. All
 problem specification inputs are checked for errors. If any
 error occurs during initialization, it is reported to the file
 whose file pointer is errfp and an error flag is returned. 
 
*****************************************************************/

int IDAMalloc(void *ida_mem, ResFn res,
              realtype t0, N_Vector y0, N_Vector yp0, 
              int itol, realtype *rtol, void *atol)
{
  IDAMem IDA_mem;
  booleantype nvectorOK, allocOK, neg_atol;
  long int lrw1, liw1;

  /* Check ida_mem */
  if (ida_mem == NULL) {
    fprintf(stderr, MSG_IDAM_NO_MEM);
    return(IDAM_NO_MEM);
  }
  IDA_mem = (IDAMem) ida_mem;
  
  /* Check for legal input parameters */
  
  if (y0 == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_Y0_NULL); 
    return(IDAM_ILL_INPUT); 
  }
  
  if (yp0 == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_YP0_NULL); 
    return(IDAM_ILL_INPUT); 
  }

  if ((itol != SS) && (itol != SV)) {
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_ITOL, itol, SS, SV);
    return(IDAM_ILL_INPUT);
  }

  if (res == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_RES_NULL); 
    return(IDAM_ILL_INPUT); 
  }

  if (rtol == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_RTOL_NULL); 
    return(IDAM_ILL_INPUT); 
  }

  if (*rtol < ZERO) { 
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_RTOL, *rtol); 
    return(IDAM_ILL_INPUT); 
  }
   
  if (atol == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_ATOL_NULL); 
    return(IDAM_ILL_INPUT); 
  }

  /* Test if all required vector operations are implemented */
  nvectorOK = IDACheckNvector(y0);
  if(!nvectorOK) {
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_NVECTOR);
    return(IDAM_ILL_INPUT);
  }

  /* Test absolute tolerances */
  if (itol == SS) { 
    neg_atol = (*((realtype *)atol) < ZERO); 
  } else { 
    neg_atol = (N_VMin((N_Vector)atol) < ZERO); 
  }
  if (neg_atol) { 
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_ATOL); 
    return(IDAM_ILL_INPUT); 
  }

  /* Set space requirements for one N_Vector */
  if (y0->ops->nvspace != NULL) {
    N_VSpace(y0, &lrw1, &liw1);
  } else {
    lrw1 = 0;
    liw1 = 0;
  }
  IDA_mem->ida_lrw1 = lrw1;
  IDA_mem->ida_liw1 = liw1;

  /* Allocate the vectors (using y0 as a template) */
  allocOK = IDAAllocVectors(IDA_mem, y0);
  if (!allocOK) {
    if(errfp!=NULL) fprintf(errfp, MSG_MEM_FAIL);
    return(IDAM_MEM_FAIL);
  }
 
  /* All error checking is complete at this point */

  /* Copy the input parameters into IDA memory block */
  IDA_mem->ida_res = res;
  IDA_mem->ida_tn = t0;
  IDA_mem->ida_y0  = y0;
  IDA_mem->ida_yp0 = yp0;
  IDA_mem->ida_itol = itol;
  IDA_mem->ida_rtol = rtol;      
  IDA_mem->ida_atol = atol;  

  /* Set the linear solver addresses to NULL, linitOK to FALSE */
  IDA_mem->ida_linit  = NULL;
  IDA_mem->ida_lsetup = NULL;
  IDA_mem->ida_lsolve = NULL;
  IDA_mem->ida_lperf  = NULL;
  IDA_mem->ida_lfree  = NULL;
  IDA_mem->ida_lmem = NULL;
  IDA_mem->ida_linitOK = FALSE;

  /* Initialize the phi array */
  N_VScale(ONE, y0, IDA_mem->ida_phi[0]);  
  N_VScale(ONE, yp0, IDA_mem->ida_phi[1]);  
 
  /* Initialize all the counters and other optional output values */
  IDA_mem->ida_nst     = 0;
  IDA_mem->ida_nre     = 0;
  IDA_mem->ida_ncfn    = 0;
  IDA_mem->ida_netf    = 0;
  IDA_mem->ida_nni     = 0;
  IDA_mem->ida_nsetups = 0;
  
  IDA_mem->ida_kused = 0;
  IDA_mem->ida_hused = ZERO;
  IDA_mem->ida_tolsf = ONE;

  /* Initial setup not done yet */
  IDA_mem->ida_SetupDone = FALSE;

  /* Problem memory has been successfully allocated */
  IDA_mem->ida_MallocDone = TRUE;
  return(SUCCESS);
}

/******************** IDAReInit ********************************

 IDAReInit re-initializes IDA's memory for a problem, assuming
 it has already beeen allocated in a prior IDAMalloc call.
 All problem specification inputs are checked for errors.
 The problem size Neq is assumed to be unchaged since the call
 to IDAMalloc, and the maximum order maxord must not be larger.
 If any error occurs during reinitialization, it is reported to
 the file whose file pointer is errfp.
 The return value is SUCCESS = 0 if no errors occurred, or
 a negative value otherwise.
 
*****************************************************************/

int IDAReInit(void *ida_mem, ResFn res,
              realtype t0, N_Vector y0, N_Vector yp0,
              int itol, realtype *rtol, void *atol)
{
  IDAMem IDA_mem;
  booleantype neg_atol;

  /* Check for legal input parameters */
  
  if (ida_mem == NULL) {
    fprintf(stderr, MSG_IDAM_NO_MEM);
    return(IDAREI_NO_MEM);
  }
  IDA_mem = (IDAMem) ida_mem;

  /* Check if problem was malloc'ed */
  
  if (IDA_mem->ida_MallocDone == FALSE) {
    if(errfp!=NULL) fprintf(errfp, MSG_REI_NO_MALLOC);
    return(IDAREI_NO_MALLOC);
  }

  /* Check for legal input parameters */
  
  if (y0 == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_Y0_NULL); 
    return(IDAREI_ILL_INPUT); 
  }
  
  if (yp0 == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_YP0_NULL); 
    return(IDAREI_ILL_INPUT); 
  }

  if ((itol != SS) && (itol != SV)) {
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_ITOL, itol, SS, SV);
    return(IDAREI_ILL_INPUT);
  }

  if (res == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_RES_NULL); 
    return(IDAREI_ILL_INPUT); 
  }

  if (rtol == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_RTOL_NULL); 
    return(IDAREI_ILL_INPUT); 
  }

  if (*rtol < ZERO) {
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_RTOL, *rtol); 
    return(IDAREI_ILL_INPUT); 
  }
   
  if (atol == NULL) { 
    if(errfp!=NULL) fprintf(errfp, MSG_ATOL_NULL); 
    return(IDAREI_ILL_INPUT); 
  }

  if (itol == SS) { 
    neg_atol = (*((realtype *)atol) < ZERO); 
  } else { 
    neg_atol = (N_VMin((N_Vector)atol) < ZERO); 
  }
  if (neg_atol) { 
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_ATOL); 
    return(IDAREI_ILL_INPUT); 
  }

  /* All error checking is complete at this point */

  /* Copy the input parameters into IDA memory block */
  IDA_mem->ida_res = res;
  IDA_mem->ida_y0  = y0;
  IDA_mem->ida_yp0 = yp0;
  IDA_mem->ida_tn = t0;
  IDA_mem->ida_itol = itol;
  IDA_mem->ida_rtol = rtol;      
  IDA_mem->ida_atol = atol;  

  /* Set linitOK to FALSE */
  IDA_mem->ida_linitOK = FALSE;

  /* Initialize the phi array */
  N_VScale(ONE, y0, IDA_mem->ida_phi[0]);  
  N_VScale(ONE, yp0, IDA_mem->ida_phi[1]);  
 
  /* Initialize all the counters and other optional output values */
 
  IDA_mem->ida_nst     = 0;
  IDA_mem->ida_nre     = 0;
  IDA_mem->ida_ncfn    = 0;
  IDA_mem->ida_netf    = 0;
  IDA_mem->ida_nni     = 0;
  IDA_mem->ida_nsetups = 0;
  
  IDA_mem->ida_kused = 0;
  IDA_mem->ida_hused = ZERO;
  IDA_mem->ida_tolsf = ONE;

  /* Initial setup not done yet */
  IDA_mem->ida_SetupDone = FALSE;
      
  /* Problem has been successfully re-initialized */

  return(SUCCESS);
}

#define res    (IDA_mem->ida_res)
#define y0     (IDA_mem->ida_y0)
#define yp0    (IDA_mem->ida_yp0)
#define itol   (IDA_mem->ida_itol)
#define rtol   (IDA_mem->ida_rtol)
#define atol   (IDA_mem->ida_atol)
#define nvspec (IDA_mem->ida_nvspec)


/*-----------------------------------------------------------------*/

int IDASetNonlinConvCoefIC(void *ida_mem, realtype epiccon)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  if (epiccon < ZERO) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_BAD_EPICCON);
    return(IDAS_ILL_INPUT);
  }

  IDA_mem->ida_epiccon = epiccon;

  return(SUCCESS);
}

#define epiccon (IDA_mem->ida_epiccon)

/*-----------------------------------------------------------------*/

int IDASetMaxNumStepsIC(void *ida_mem, int maxnh)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  if (maxnh < 0) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_BAD_MAXNH);
    return(IDAS_ILL_INPUT);
  }

  IDA_mem->ida_maxnh = maxnh;

  return(SUCCESS);
}

#define maxnh (IDA_mem->ida_maxnh)

/*-----------------------------------------------------------------*/

int IDASetMaxNumJacsIC(void *ida_mem, int maxnj)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

   if (maxnj < 0) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_BAD_MAXNJ);
    return(IDAS_ILL_INPUT);
  } 

  IDA_mem->ida_maxnj = maxnj;

  return(SUCCESS);
}

#define maxnj (IDA_mem->ida_maxnj)

/*-----------------------------------------------------------------*/

int IDASetMaxNumItersIC(void *ida_mem, int maxnit)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  if (maxnit < 0) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_BAD_MAXNIT);
    return(IDAS_ILL_INPUT);
  }

  IDA_mem->ida_maxnit = maxnit;

  return(SUCCESS);
}

#define maxnit (IDA_mem->ida_maxnit)

/*-----------------------------------------------------------------*/

int IDASetLineSearchOffIC(void *ida_mem, booleantype lsoff)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  IDA_mem->ida_lsoff = lsoff;

  return(SUCCESS);
}

#define lsoff (IDA_mem->ida_lsoff)

/*-----------------------------------------------------------------*/

int IDASetStepToleranceIC(void *ida_mem, realtype steptol)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAS_NO_MEM);
    return(IDAS_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  if (steptol < ZERO) {
    if(errfp!=NULL) fprintf(errfp, MSG_IDAS_BAD_STEPTOL);
    return(IDAS_ILL_INPUT);
  }

  IDA_mem->ida_steptol = steptol;

  return(SUCCESS);
}

#define steptol   (IDA_mem->ida_steptol)

/**************************************************************/
/**************** BEGIN Readability Constants *****************/
/**************************************************************/

#define uround   (IDA_mem->ida_uround)  
#define phi      (IDA_mem->ida_phi) 
#define ewt      (IDA_mem->ida_ewt)  
#define yy       (IDA_mem->ida_yy)
#define yp       (IDA_mem->ida_yp)
#define delta    (IDA_mem->ida_delta)
#define mm       (IDA_mem->ida_mm)
#define ee       (IDA_mem->ida_ee)
#define savres   (IDA_mem->ida_savres)
#define tempv1   (IDA_mem->ida_tempv1)
#define tempv2   (IDA_mem->ida_tempv2) 
#define kk       (IDA_mem->ida_kk)
#define hh       (IDA_mem->ida_hh)
#define h0u      (IDA_mem->ida_h0u)
#define tn       (IDA_mem->ida_tn)
#define tretp    (IDA_mem->ida_tretp)
#define cj       (IDA_mem->ida_cj)
#define cjold    (IDA_mem->ida_cjold)
#define cjratio  (IDA_mem->ida_cjratio)
#define cjlast   (IDA_mem->ida_cjlast)
#define nbacktr  (IDA_mem->ida_nbacktr)
#define nst      (IDA_mem->ida_nst)
#define nre      (IDA_mem->ida_nre)
#define ncfn     (IDA_mem->ida_ncfn)
#define netf     (IDA_mem->ida_netf)
#define nni      (IDA_mem->ida_nni)
#define nsetups  (IDA_mem->ida_nsetups)
#define ns       (IDA_mem->ida_ns)
#define lrw1     (IDA_mem->ida_lrw1)
#define liw1     (IDA_mem->ida_liw1)
#define lrw      (IDA_mem->ida_lrw)
#define liw      (IDA_mem->ida_liw)
#define linit    (IDA_mem->ida_linit)
#define lsetup   (IDA_mem->ida_lsetup)
#define lsolve   (IDA_mem->ida_lsolve) 
#define lperf    (IDA_mem->ida_lperf)
#define lfree    (IDA_mem->ida_lfree) 
#define lmem     (IDA_mem->ida_lmem) 
#define linitOK  (IDA_mem->ida_linitOK)
#define knew     (IDA_mem->ida_knew)
#define kused    (IDA_mem->ida_kused)          
#define hused    (IDA_mem->ida_hused)         
#define tolsf    (IDA_mem->ida_tolsf)      
#define phase    (IDA_mem->ida_phase)
#define epsNewt  (IDA_mem->ida_epsNewt)
#define toldel   (IDA_mem->ida_toldel)
#define ss       (IDA_mem->ida_ss)
#define rr       (IDA_mem->ida_rr)
#define psi      (IDA_mem->ida_psi)
#define alpha    (IDA_mem->ida_alpha)
#define beta     (IDA_mem->ida_beta)
#define sigma    (IDA_mem->ida_sigma)
#define gamma    (IDA_mem->ida_gamma)
#define setupNonNull (IDA_mem->ida_setupNonNull) 
#define constraintsSet (IDA_mem->ida_constraintsSet)

/**************************************************************/
/***************** END Readability Constants ******************/
/**************************************************************/

/******************** IDACalcIC *********************************

 IDACalcIC computes consistent initial conditions, given the 
 user's initial guess for unknown components of y0 and/or yp0.

 The return value is SUCCESS = 0 if no error occurred.

 The error return values (fully described in ida.h) are:
    IC_IDA_NO_MEM      ida_mem is NULL
    IC_NO_MALLOC       ida_mem was not allocated
    IC_ILL_INPUT       bad value for icopt, tout1, or id
    IC_LINIT_FAIL      the linear solver linit routine failed
    IC_BAD_EWT         zero value of some component of ewt
    RES_NONRECOV_ERR   res had a non-recoverable error
    IC_FIRST_RES_FAIL  res failed recoverably on the first call
    SETUP_FAILURE      lsetup had a non-recoverable error
    SOLVE_FAILURE      lsolve had a non-recoverable error
    IC_NO_RECOVERY     res, lsetup, or lsolve had a recoverable
                       error, but IDACalcIC could not recover
    IC_FAILED_CONSTR   the inequality constraints could not be met
    IC_FAILED_LINESRCH the linesearch failed (on steptol test)
    IC_CONV_FAILURE    the Newton iterations failed to converge

 
*****************************************************************/

int IDACalcIC (void *ida_mem, int icopt, realtype tout1)
{
  booleantype ewtsetOK;
  int ier, nwt, nh, mxnh, icret, retval=0;
  realtype tdist, troundoff, minid, hic, ypnorm;
  IDAMem IDA_mem;

  /* Check if IDA memory exists */

  if (ida_mem == NULL) {
    fprintf(stderr, MSG_IC_IDA_NO_MEM);
    return(IC_IDA_NO_MEM);
  }
  IDA_mem = (IDAMem) ida_mem;

  /* Check if problem was malloc'ed */
  
  if (IDA_mem->ida_MallocDone == FALSE) {
    if(errfp!=NULL) fprintf(errfp, MSG_IC_NO_MALLOC);
    return(IC_NO_MALLOC);
  }

  /* Check inputs to IDA for correctness and consistency */

  ier = IDAInitialSetup(IDA_mem);
  if (ier != SUCCESS) return(IC_ILL_INPUT);
  IDA_mem->ida_SetupDone = TRUE;

  /* Check legality of input arguments, and set IDA memory copies. */

  if (icopt < CALC_YA_YDP_INIT || icopt > CALC_Y_INIT) {
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_ICOPT, icopt);
    return(IC_ILL_INPUT);
  }
  IDA_mem->ida_icopt = icopt;

  if (icopt == CALC_YA_YDP_INIT && (id == NULL)) {
    if(errfp!=NULL) fprintf(errfp, MSG_IC_MISSING_ID);
    return(IC_ILL_INPUT);
  }

  tdist = ABS(tout1 - tn);
  troundoff = TWO*uround*(ABS(tn) + ABS(tout1));    
  if (tdist < troundoff) {
    if(errfp!=NULL) fprintf(errfp, MSG_IC_TOO_CLOSE, tout1, tn);
    return(IC_ILL_INPUT);
  }

  /* For use in the CALC_YA_YP_INIT case, set sysindex and tscale. */

  IDA_mem->ida_sysindex = 1;
  IDA_mem->ida_tscale   = tdist;
  if (icopt == CALC_YA_YDP_INIT) {
    minid = N_VMin(id);
    if (minid < ZERO) {
      if(errfp!=NULL) fprintf(errfp, MSG_IC_BAD_ID);
      return(IC_ILL_INPUT);
    }
    if (minid > HALF) IDA_mem->ida_sysindex = 0;
  }

  /* Set the test constant in the Newton convergence test */

  IDA_mem->ida_epsNewt = epiccon;

  /* Initializations: cjratio = 1 (for use in direct linear solvers); 
     set nbacktr = 0; call linit routine. */

  cjratio = ONE;
  nbacktr = 0;
  linitOK = (linit(IDA_mem) == LINIT_OK);
  if (!linitOK) {
    if(errfp!=NULL) fprintf(errfp, MSG_IC_LINIT_FAIL);
    return(IC_LINIT_FAIL);
  }

  /* Set hic, hh, cj, and mxnh. */
  hic = PT001*tdist;
  ypnorm = IDAWrmsNorm(IDA_mem, yp0, ewt, suppressalg);
  if (ypnorm > HALF/hic) hic = HALF/ypnorm;
  if( tout1 < tn) hic = -hic;
  hh = hic;
  if (icopt == CALC_YA_YDP_INIT) {
    cj = ONE/hic;
    mxnh = maxnh;
  }
  else {
    cj = ZERO;
    mxnh = 1;
  }

  /* Loop over nwt = number of evaluations of ewt vector. */

  for (nwt = 1; nwt <= 2; nwt++) {
 
    /* Loop over nh = number of h values. */
    for (nh = 1; nh <= mxnh; nh++) {

      /* Call the IC nonlinear solver function. */
      retval = IDAnlsIC(IDA_mem);

      /* Cut h and loop on recoverable CALC_YA_YDP_INIT failure; else break. */
      if (retval == SUCCESS) break;
      ncfn++;
      if (retval < 0) break;
      if (nh == mxnh) break;
      /* If looping to try again, reset y0 and yp0 if not converging. */
      if (retval != IC_SLOW_CONVRG) {
        N_VScale (ONE, phi[0], y0);
        N_VScale (ONE, phi[1], yp0);
      }
      hic *= PT1;
      cj = ONE/hic;
      hh = hic;
    }   /* End of nh loop */

    /* Break on failure; else reset ewt, save y0,yp0 in phi, and loop. */
    if (retval != SUCCESS) break;
    ewtsetOK = IDAEwtSet(IDA_mem, y0);
    if (!ewtsetOK) { retval = IC_BAD_EWT; break; }
    N_VScale (ONE, y0,  phi[0]);
    N_VScale (ONE, yp0, phi[1]);

  }   /* End of nwt loop */


  /* Load the optional outputs. */
  if (icopt == CALC_YA_YDP_INIT)   hused = hic;

  /* On any failure, print message and return proper flag. */
  if (retval != SUCCESS) {
    icret = IDAICFailFlag(IDA_mem, retval);
    return(icret);
  }

  /* Otherwise return success flag. */
  return(SUCCESS);

}

/********************* IDASolve ***********************************

 This routine is the main driver of the IDA package. 

 It integrates over an independent variable interval defined by the user, 
 by calling IDAStep to take internal independent variable steps.

 The first time that IDASolve is called for a successfully initialized
 problem, it computes a tentative initial step size.

 IDASolve supports four modes, specified by itask:
    NORMAL,  ONE_STEP,  NORMAL_TSTOP,  and  ONE_STEP_TSTOP.
 In the NORMAL and NORMAL-TSTOP modes, the solver steps until it 
 passes tout and then interpolates to obtain y(tout) and yp(tout).
 In the ONE_STEP and ONE_STEP_TSTOP modes, it takes one internal step
 and returns.  In the NORMAL_TSTOP and ONE_STEP_TSTOP modes, it also
 takes steps so as to reach tstop exactly and never to go past it.

 IDASolve returns integer values corresponding to success and failure as below:

       (successful returns:)     (failed returns:)
 
       NORMAL_RETURN             ILL_INPUT               TOO_MUCH_WORK
       INTERMEDIATE_RETURN       IDA_NO_MEM              TOO_MUCH_ACC
       TSTOP_RETURN              CONV_FAILURE            SETUP_FAILURE
                                 SOLVE_FAILURE           CONSTR_FAILURE
                                 ERR_FAILURE             REP_RES_REC_ERR
                                 RES_NONREC_ERR

********************************************************************/

int IDASolve(void *ida_mem, realtype tout, realtype *tret,
             N_Vector yret, N_Vector ypret, int itask)
{
  long int nstloc;
  int sflag, istate, ier;
  realtype tdist, troundoff, ypnorm, rh, nrm;
  booleantype istop, ewtsetOK;
  IDAMem IDA_mem;

  /* Check for legal inputs in all cases. */

  if (ida_mem == NULL) {
    fprintf(stderr, MSG_IDA_NO_MEM);
    return(IDA_NO_MEM);
  }
  IDA_mem = (IDAMem) ida_mem;

  /* Check if problem was malloc'ed */
  
  if (IDA_mem->ida_MallocDone == FALSE) {
    if(errfp!=NULL) fprintf(errfp, MSG_NO_MALLOC);
    return(NO_MALLOC);
  }

  /* Check for legal arguments */

  if (yret == NULL) {
    if(errfp!=NULL) fprintf(errfp, MSG_YRET_NULL);       
    return(ILL_INPUT);
  }
  yy = yret;  

  if (ypret == NULL) {
    if(errfp!=NULL) fprintf(errfp, MSG_YPRET_NULL);       
    return(ILL_INPUT);
  }
  yp = ypret;
  
  if (tret == NULL) {
    if(errfp!=NULL) fprintf(errfp, MSG_TRET_NULL);
    return(ILL_INPUT);
  }
  *tret = tretp = tn; /* Set tret now in case of illegal-input return. */

  if ((itask < NORMAL) || (itask > ONE_STEP_TSTOP)) {
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_ITASK, itask);
    return(ILL_INPUT);
  }
  
  if ( (itask == NORMAL_TSTOP) || (itask == ONE_STEP_TSTOP) ) {
    if ( tstopset == FALSE ) {
      if(errfp!=NULL) fprintf(errfp, MSG_NO_TSTOP);
      return(ILL_INPUT);
    }
    istop = TRUE;
  } else {
      istop = FALSE;
  }


  if (nst == 0) {       /* THIS IS THE FIRST CALL */

    /* Check inputs to the IDA for correctness and consistency */

    if (IDA_mem->ida_SetupDone == FALSE) {
      ier = IDAInitialSetup(IDA_mem);
      if(ier != SUCCESS) return(ILL_INPUT);
      IDA_mem->ida_SetupDone = TRUE;
    }

    /* On the first call, check for tout - tn too small,
       set initial hh,
       check for approach to tstop, and scale phi[1] by hh. */

    tdist = ABS(tout - tn);
    troundoff = TWO*uround*(ABS(tn) + ABS(tout));    
    if (tdist < troundoff) {
      if(errfp!=NULL) fprintf(errfp, MSG_TOO_CLOSE, tout, tn);
      return(ILL_INPUT);
    }

    hh = hin;
    if ( (hh != ZERO) && ((tout-tn)*hh < ZERO) ) {
      if(errfp!=NULL) fprintf(errfp, MSG_BAD_HINIT, hh, tout-tn);
      return(ILL_INPUT);
    }

    if (hh == ZERO) {
      hh = PT001*tdist;
      ypnorm = IDAWrmsNorm(IDA_mem, phi[1], ewt, suppressalg);
      if (ypnorm > HALF/hh) hh = HALF/ypnorm;
      if(tout < tn) hh = -hh;
    }

    rh = ABS(hh)*hmax_inv;
    if (rh > ONE) hh /= rh;

    if(istop) {
      if ( (tstop - tn)*hh < ZERO) {
        if(errfp!=NULL) fprintf(errfp, MSG_BAD_TSTOP, tstop, tn);
        return(ILL_INPUT);
      }
      if ( (tn + hh - tstop)*hh > ZERO) hh = tstop - tn;
    }

    h0u = hh;

    N_VScale(hh, phi[1], phi[1]);
    kk = 0; kused = 0;  /* set in case of an error return before a step */

    /* Set the convergence test constants epsNewt and toldel */

    epsNewt = epcon;
    toldel = PT0001 * epsNewt;

  } /* end of first-call block. */

  /* Call lperf function and set nstloc for later performance testing. */

  if (lperf != NULL) lperf(IDA_mem, 0);
  nstloc = 0;

  /* If not the first call, check for stop conditions. */

  if (nst > 0) {
    istate = IDAStopTest1(IDA_mem, tout, tret, yret, ypret, itask);
    if (istate != CONTINUE_STEPS) return(istate);
  }

  /* Looping point for internal steps. */

  loop {
   
    /* Check for too many steps taken. */
    
    if (nstloc >= mxstep) {
      if(errfp!=NULL) fprintf(errfp, MSG_MAX_STEPS, tn, mxstep, tout);
      istate = TOO_MUCH_WORK;
      *tret = tretp = tn;
      break; /* Here yy=yret and yp=ypret already have the current solution. */
    }

    /* Call lperf to generate warnings of poor performance. */

    if (lperf != NULL) lperf(IDA_mem, 1);

    /* Reset and check ewt (if not first call). */

    if (nst > 0) {
      ewtsetOK = IDAEwtSet(IDA_mem, phi[0]);
      if (!ewtsetOK) {
        if(errfp!=NULL) fprintf(errfp, MSG_EWT_NOW_BAD, tn);
        istate = ILL_INPUT;
        ier = IDAGetSolution(IDA_mem, tn, yret, ypret);
        *tret = tretp = tn;
        break;
      }
    }
    
    /* Check for too much accuracy requested. */
    
    nrm = IDAWrmsNorm(IDA_mem, phi[0], ewt, suppressalg);
    tolsf = uround * nrm;
    if (tolsf > ONE) {
      tolsf *= TEN;
      if(errfp!=NULL) fprintf(errfp, MSG_TOO_MUCH_ACC, tn);
      istate = TOO_MUCH_ACC;
      *tret = tretp = tn;
      if (nst > 0) ier = IDAGetSolution(IDA_mem, tn, yret, ypret);
      break;
    }

    /* Call IDAStep to take a step. */

    sflag = IDAStep(IDA_mem);

    /* Process all failed-step cases, and exit loop. */
   
    if (sflag != SUCCESS) {
      istate = IDAHandleFailure(IDA_mem, sflag);
      *tret = tretp = tn;
      ier = IDAGetSolution(IDA_mem, tn, yret, ypret);
      break;
    }
    
    nstloc++;

    /* After successful step, check for stop conditions; continue or break. */

    istate = IDAStopTest2(IDA_mem, tout, tret, yret, ypret, itask);
    if (istate != CONTINUE_STEPS) break;

  } /* End of step loop */

  return(istate);    
}

/*------------------                     --------------------------*/
/* 
   This routine evaluates y(t) and y'(t) as the value and derivative of 
   the interpolating polynomial at the independent variable t, and stores
   the results in the vectors yret and ypret.  It uses the current
   independent variable value, tn, and the method order last used, kused.
   This function is called by IDASolve with t = tout, t = tn, or t = tstop.
   
   If kused = 0 (no step has been taken), or if t = tn, then the order used
   here is taken to be 1, giving yret = phi[0], ypret = phi[1]/psi[0].
   
   The return values are:
     OKAY  if t is legal, or
     BAD_T if t is not within the interval of the last step taken.
*/
/*-----------------------------------------------------------------*/

int IDAGetSolution(void *ida_mem, realtype t, N_Vector yret, N_Vector ypret)
{
  IDAMem IDA_mem;
  realtype tfuzz, tp, delt, c, d, gam;
  int j, kord;

  if (ida_mem == NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return (IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem; 
  /* Check t for legality.  Here tn - hused is t_{n-1}. */
 
  tfuzz = HUNDRED * uround * (tn + hh);
  tp = tn - hused - tfuzz;
  if ( (t - tp)*hh < ZERO) {
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_T, t, tn-hused, tn);
    return(BAD_T);
  }

  /* Initialize yret = phi[0], ypret = 0, and kord = (kused or 1). */

  N_VScale (ONE, phi[0], yret);
  N_VConst (ZERO, ypret);
  kord = kused; 
  if (kused == 0) kord = 1;

 /* Accumulate multiples of columns phi[j] into yret and ypret. */

  delt = t - tn;
  c = ONE; d = ZERO;
  gam = delt/psi[0];
  for (j=1; j <= kord; j++) {
    d = d*gam + c/psi[j-1];
    c = c*gam;
    gam = (delt + psi[j-1])/psi[j];
    N_VLinearSum(ONE,  yret, c, phi[j],  yret);
    N_VLinearSum(ONE, ypret, d, phi[j], ypret);
  }
  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetIntWorkSpace(void *ida_mem, long int *leniw)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *leniw = liw;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetRealWorkSpace(void *ida_mem, long int *lenrw)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *lenrw = lrw;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetNumSteps(void *ida_mem, long int *nsteps)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *nsteps = nst;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetNumResEvals(void *ida_mem, long int *nrevals)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *nrevals = nre;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetNumLinSolvSetups(void *ida_mem, long int *nlinsetups)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *nlinsetups = nsetups;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetNumErrTestFails(void *ida_mem, long int *netfails)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *netfails = netf;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetNumBacktrackOps(void *ida_mem, long int *nbacktracks)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *nbacktracks = nbacktr;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetLastOrder(void *ida_mem, int *klast)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *klast = kused;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetCurrentOrder(void *ida_mem, int *kcur)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *kcur = kk;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetActualInitStep(void *ida_mem, realtype *hinused)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *hinused = h0u;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetLastStep(void *ida_mem, realtype *hlast)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *hlast = hused;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetCurrentStep(void *ida_mem, realtype *hcur)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *hcur = hh;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetCurrentTime(void *ida_mem, realtype *tcur)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *tcur = tn;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetTolScaleFactor(void *ida_mem, realtype *tolsfact)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *tolsfact = tolsf;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetErrWeights(void *ida_mem, N_Vector *eweight)
{
  IDAMem IDA_mem;
  
  if (ida_mem == NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return (IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem; 

  *eweight = ewt;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetWorkSpace(void *ida_mem, long int *leniw, long int *lenrw)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *leniw = liw;
  *lenrw = lrw;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetIntegratorStats(void *ida_mem, long int *nsteps, long int *nrevals, 
                          long int *nlinsetups, long int *netfails,
                          int *klast, int *kcur, realtype *hlast, 
                          realtype *hcur, realtype *tcur)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *nsteps     = nst;
  *nrevals    = nre;
  *nlinsetups = nsetups;
  *netfails   = netf;
  *klast      = kused;
  *kcur       = kk;
  *hlast      = hused;
  *hcur       = hh;  
  *tcur       = tn;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetNumNonlinSolvIters(void *ida_mem, long int *nniters)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *nniters = nni;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetNumNonlinSolvConvFails(void *ida_mem, long int *nncfails)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *nncfails = ncfn;

  return(OKAY);
}

/*-----------------------------------------------------------------*/

int IDAGetNonlinSolvStats(void *ida_mem, long int *nniters, long int *nncfails)
{
  IDAMem IDA_mem;

  if (ida_mem==NULL) {
    fprintf(stderr, MSG_IDAG_NO_MEM);
    return(IDAG_NO_MEM);
  }

  IDA_mem = (IDAMem) ida_mem;

  *nniters  = nni;
  *nncfails = ncfn;

  return(OKAY);
}

/********************* IDAFree **********************************

 This routine frees the problem memory allocated by IDAMalloc
 Such memory includes all the vectors allocated by IDAAllocVectors,
 and the memory lmem for the linear solver (deallocated by a call
 to lfree).

*******************************************************************/

void IDAFree(void *ida_mem)
{
  IDAMem IDA_mem;

  if (ida_mem == NULL) return;

  IDA_mem = (IDAMem) ida_mem;
  
  IDAFreeVectors(IDA_mem);
  lfree(IDA_mem);
  free(IDA_mem);
}


/***************************************************************/
/********** END Exported Functions Implementation **************/
/***************************************************************/


/*******************************************************************/
/******** BEGIN Private Helper Functions Implementation ************/
/*******************************************************************/
 
/****************** IDACheckNvector **********************************
 This routine checks if all required vector operations are present.
 If any of them is missing it returns FALSE.
**********************************************************************/

static booleantype IDACheckNvector(N_Vector tmpl)
{
  if((tmpl->ops->nvclone        == NULL) ||
     (tmpl->ops->nvdestroy      == NULL) ||
     (tmpl->ops->nvlinearsum    == NULL) ||
     (tmpl->ops->nvconst        == NULL) ||
     (tmpl->ops->nvprod         == NULL) ||
     (tmpl->ops->nvscale        == NULL) ||
     (tmpl->ops->nvabs          == NULL) ||
     (tmpl->ops->nvinv          == NULL) ||
     (tmpl->ops->nvaddconst     == NULL) ||
     (tmpl->ops->nvwrmsnorm     == NULL) ||
     (tmpl->ops->nvmin          == NULL))
    return(FALSE);
  else
    return(TRUE);
}

/****************** IDAAllocVectors ***********************************

 This routine allocates the IDA vectors ewt, tempv1, tempv2, and
 phi[0], ..., phi[maxord]. The length of the vectors is the input
 parameter Neq and the maximum order (needed to allocate phi) is the
 input parameter maxord. If all memory allocations are successful,
 IDAAllocVectors returns TRUE. Otherwise all allocated memory is freed
 and IDAAllocVectors returns FALSE.
 This routine also sets the optional outputs lrw and liw, which are
 (respectively) the lengths of the real and integer work spaces
 allocated here.

**********************************************************************/

static booleantype IDAAllocVectors(IDAMem IDA_mem, N_Vector tmpl)
{
  int i, j, maxcol;

  /* Allocate ewt, ee, delta, tempv1, tempv2 */
  
  ewt = N_VClone(tmpl);
  if (ewt == NULL) return(FALSE);

  ee = N_VClone(tmpl);
  if (ee == NULL) {
    N_VDestroy(ewt);
    return(FALSE);
  }
  delta = N_VClone(tmpl);
  if (delta == NULL) {
    N_VDestroy(ewt);
    N_VDestroy(ee);
    return(FALSE);
  }
  tempv1 = N_VClone(tmpl);
  if (tempv1 == NULL) {
    N_VDestroy(ewt);
    N_VDestroy(ee);
    N_VDestroy(delta);
    return(FALSE);
  }
  tempv2= N_VClone(tmpl);
  if (tempv2 == NULL) {
    N_VDestroy(ewt);
    N_VDestroy(ee);
    N_VDestroy(delta);
    N_VDestroy(tempv1);
    return(FALSE);
  }

  savres = tempv1;

  /* Allocate phi[0] ... phi[maxord].  Make sure phi[2] and phi[3] are
  allocated (for use as temporary vectors), regardless of maxord.       */

  maxcol = MAX(maxord,3);
  for (j=0; j <= maxcol; j++) {
    phi[j] = N_VClone(tmpl);
    if (phi[j] == NULL) {
      N_VDestroy(ewt);
      N_VDestroy(ee);
      N_VDestroy(delta);
      N_VDestroy(tempv1);
      N_VDestroy(tempv2);
      for (i=0; i < j; i++) N_VDestroy(phi[i]);
      return(FALSE);
    }
  }

  /* Set solver workspace lengths  */

  lrw = (maxcol + 6) * lrw1;
  liw = (maxcol + 6) * liw1;

  return(TRUE);
}


/***************** IDAFreeVectors *********************************
  
 This routine frees the IDA vectors allocated in IDAAllocVectors.

******************************************************************/

static void IDAFreeVectors(IDAMem IDA_mem)
{
  int j, maxcol;
  
  N_VDestroy(ewt);
  N_VDestroy(ee);
  N_VDestroy(delta);
  N_VDestroy(tempv1);
  N_VDestroy(tempv2);
  maxcol = MAX(maxord,3);
  for(j=0; j <= maxcol; j++) N_VDestroy(phi[j]);
}


/**************************************************************/
/** BEGIN Support routines for Initial Conditions calculation */
/**************************************************************/

#define icopt    (IDA_mem->ida_icopt)
#define sysindex (IDA_mem->ida_sysindex)
#define tscale   (IDA_mem->ida_tscale)
#define ynew     (IDA_mem->ida_ynew)
#define ypnew    (IDA_mem->ida_ypnew)
#define delnew   (IDA_mem->ida_delnew)
#define dtemp    (IDA_mem->ida_dtemp)

/******************** IDAnlsIC **********************************

 IDAnlsIC solves a nonlinear system for consistent initial 
 conditions.  It calls IDANewtonIC to do most of the work.

 The return value is SUCCESS = 0 if no error occurred.
 The error return values (positive) considered recoverable are:
    IC_FAIL_RECOV      if res, lsetup, or lsolve failed recoverably
    IC_CONSTR_FAILED   if the constraints could not be met
    IC_LINESRCH_FAILED if the linesearch failed (on steptol test)
    IC_CONV_FAIL       if the Newton iterations failed to converge
    IC_SLOW_CONVRG     if the iterations are converging slowly
                       (failed the convergence test, but showed
                       norm reduction or convergence rate < 1)
 The error return values (negative) considered non-recoverable are:
    RES_NONRECOV_ERR   if res had a non-recoverable error
    IC_FIRST_RES_FAIL  if res failed recoverably on the first call
    SETUP_FAILURE      if lsetup had a non-recoverable error
    SOLVE_FAILURE      if lsolve had a non-recoverable error
 
*****************************************************************/

static int IDAnlsIC (IDAMem IDA_mem)
{
  int retval, nj;
  N_Vector tv1, tv2, tv3;

  tv1 = ee;
  tv2 = tempv2;
  tv3 = phi[2];

  retval = res (tn, y0, yp0, delta, rdata);
  nre++;
  if(retval < 0) return(RES_NONRECOV_ERR);
  if(retval > 0) return(IC_FIRST_RES_FAIL);

  N_VScale (ONE, delta, savres);

  /* Loop over nj = number of linear solve Jacobian setups. */

  for (nj = 1; nj <= maxnj; nj++) {

    /* If there is a setup routine, call it. */
    if (setupNonNull) {
      nsetups++;
      retval = lsetup (IDA_mem, y0, yp0, delta, tv1, tv2, tv3);
      if(retval < 0) return(SETUP_FAILURE);
      if(retval > 0) return(IC_FAIL_RECOV);
    }

    /* Call the Newton iteration routine, and return if successful.  */
    retval = IDANewtonIC(IDA_mem);
    if (retval == SUCCESS) return(SUCCESS);

    /* If converging slowly and lsetup is nontrivial, retry. */
    if (retval == IC_SLOW_CONVRG && setupNonNull) {
      N_VScale (ONE, savres, delta);
      continue;
    }

    else return(retval);

  }   /* End of nj loop */

  /* No convergence after maxnj tries; return failure flag. */
  return(retval);

}

/******************** IDANewtonIC ************************************

 IDANewtonIC performs the Newton iteration to solve for consistent
 initial conditions.  It calls IDALineSrch within each iteration.
 On return, savres contains the current residual vector.

 The return value is SUCCESS = 0 if no error occurred.
 The error return values (positive) considered recoverable are:
    IC_FAIL_RECOV      if res or lsolve failed recoverably
    IC_CONSTR_FAILED   if the constraints could not be met
    IC_LINESRCH_FAILED if the linesearch failed (on steptol test)
    IC_CONV_FAIL       if the Newton iterations failed to converge
    IC_SLOW_CONVRG     if the iterations appear to be converging slowly.
                       They failed the convergence test, but showed 
                       an overall norm reduction (by a factor of < 0.1)
                       or a convergence rate <= ICRATEMAX).
 The error return values (negative) considered non-recoverable are:
    RES_NONRECOV_ERR   if res had a non-recoverable error
    SOLVE_FAILURE      if lsolve had a non-recoverable error
 
    **********************************************************************/

static int IDANewtonIC (IDAMem IDA_mem)
{
  int retval, mnewt;
  realtype delnorm, fnorm, fnorm0, oldfnrm, rate;

  /* Set pointer for vector delnew */
  delnew = phi[2];

  /* Call the linear solve function to get the Newton step, delta. */
  retval = lsolve(IDA_mem, delta, ewt, y0, yp0, savres);
  if(retval < 0) return(SOLVE_FAILURE);
  if(retval > 0) return(IC_FAIL_RECOV);

  /* Compute the norm of the step; return now if this is small. */
  fnorm = IDAWrmsNorm(IDA_mem, delta, ewt, FALSE);
  if (sysindex == 0) fnorm *= tscale*abs(cj);
  if (fnorm <= epsNewt) return(SUCCESS);
  fnorm0 = fnorm;

  /* Newton iteration loop */

  for (mnewt = 0; mnewt < maxnit; mnewt++) {

    nni++;
    delnorm = fnorm;
    oldfnrm = fnorm;

    /* Call the Linesearch function and return if it failed. */
    retval = IDALineSrch(IDA_mem, &delnorm, &fnorm);
    if (retval != SUCCESS) return(retval);

    /* Set the observed convergence rate and test for convergence. */
    rate = fnorm/oldfnrm;
    if (fnorm <= epsNewt) return(SUCCESS);

    /* If not converged, copy new step vector, and loop. */
    N_VScale (ONE, delnew, delta);

  }   /* End of Newton iteration loop */

  /* Return either IC_SLOW_CONVRG or recoverable fail flag. */
  if (rate <= ICRATEMAX || fnorm < PT1*fnorm0) return(IC_SLOW_CONVRG);
  return(IC_CONV_FAIL);

}


/******************** IDALineSrch *******************************

 IDALineSrch performs the Linesearch algorithm with the 
 calculation of consistent initial conditions.

 On entry, y0 and yp0 are the current values of y and y', the 
 Newton step is delta, the current residual vector F is savres,
 delnorm is WRMS-norm(delta), and fnorm is the norm of the vector
 J-inverse F.

 On a successful return, y0, yp0, and savres have been updated, 
 delnew contains the current value of J-inverse F, and fnorm is
 WRMS-norm(delnew).
 
 The return value is SUCCESS = 0 if no error occurred.
 The error return values (positive) considered recoverable are:
    IC_FAIL_RECOV      if res or lsolve failed recoverably
    IC_CONSTR_FAILED   if the constraints could not be met
    IC_LINESRCH_FAILED if the linesearch failed (on steptol test)
 The error return values (negative) considered non-recoverable are:
    RES_NONRECOV_ERR   if res had a non-recoverable error
    SOLVE_FAILURE      if lsolve had a non-recoverable error
 
*****************************************************************/

static int IDALineSrch (IDAMem IDA_mem, realtype *delnorm, realtype *fnorm)
{
  booleantype conOK;
  int retval;
  realtype f1norm, fnormp, f1normp, ratio, lambda, minlam, slpi;
  N_Vector mc;

  /* Initialize work space pointers, f1norm, ratio.
     (Use of mc in constraint check does not conflict with ypnew.) */
  mc = ee;
  dtemp = phi[3];
  ynew = tempv2;
  ypnew = ee;
  f1norm = (*fnorm)*(*fnorm)*HALF;
  ratio = ONE;

  /* If there are constraints, check and reduce step if necessary. */
  if (constraintsSet) {

    /* Update y and check constraints. */
    IDANewy(IDA_mem);
    conOK = N_VConstrMask (constraints, ynew, mc);

    if (!conOK) {
      /* Not satisfied.  Compute scaled step to satisfy constraints. */
      N_VProd (mc, delta, dtemp);
      ratio = PT99*N_VMinQuotient (y0, dtemp);
      (*delnorm) *= ratio;
      if ((*delnorm) <= steptol) return(IC_CONSTR_FAILED);
      N_VScale (ratio, delta, delta);
    }

  } /* End of constraints check */

  slpi = -TWO*f1norm*ratio;
  minlam = steptol/(*delnorm);
  lambda = ONE;

  /* In CALC_Y_INIT case, set ypnew = yp0 (fixed) for linesearch. */
  if (icopt == CALC_Y_INIT) N_VScale (ONE, yp0, ypnew);

  /* Loop on linesearch variable lambda. */

  loop {

    /* Get new (y,y') = (ynew,ypnew) and norm of new function value. */
    IDANewyyp(IDA_mem, lambda);
    retval = IDAfnorm(IDA_mem, &fnormp);
    if (retval != SUCCESS) return(retval);

    /* If lsoff option is on, break out. */
    if (lsoff) break;

    /* Do alpha-condition test. */
    f1normp = fnormp*fnormp*HALF;
    if (f1normp <= f1norm + ALPHALS*slpi*lambda) break;
    if (lambda < minlam) return(IC_LINESRCH_FAILED);
    lambda /= TWO;
    nbacktr++;

  }  /* End of breakout linesearch loop */

  /* Update y0, yp0, and fnorm, then return. */
  N_VScale (ONE, ynew,  y0);
  if (icopt == CALC_YA_YDP_INIT) N_VScale (ONE, ypnew, yp0);
  *fnorm = fnormp;
  return(SUCCESS);

}

/******************** IDAfnorm **********************************

 IDAfnorm computes the norm of the current function value, by
 evaluating the DAE residual function, calling the linear 
 system solver, and computing a WRMS-norm.
 
 On return, savres contains the current residual vector F, and
 delnew contains J-inverse F.

 The return value is SUCCESS = 0 if no error occurred, or
    IC_FAIL_RECOV    if res or lsolve failed recoverably, or
    RES_NONRECOV_ERR if res had a non-recoverable error, or
    SOLVE_FAILURE    if lsolve had a non-recoverable error.
 
*****************************************************************/

static int IDAfnorm (IDAMem IDA_mem, realtype *fnorm)
{

  int retval;

  /* Get residual vector F, return if failed, and save F in savres. */
  retval = res (tn, ynew, ypnew, delnew, rdata);
  nre++;
  if(retval < 0) return(RES_NONRECOV_ERR);
  if(retval > 0) return(IC_FAIL_RECOV);

  N_VScale (ONE, delnew, savres);

  /* Call the linear solve function to get J-inverse F; return if failed. */
  retval = lsolve(IDA_mem, delnew, ewt, ynew, ypnew, savres);
  if(retval < 0) return(SOLVE_FAILURE);
  if(retval > 0) return(IC_FAIL_RECOV);

  /* Compute the WRMS-norm; rescale if index = 0. */
  *fnorm = IDAWrmsNorm(IDA_mem, delnew, ewt, FALSE);
  if (sysindex == 0) (*fnorm) *= tscale*abs(cj);

  return(SUCCESS);

}

/******************** IDANewyyp *********************************

 IDANewyyp updates the vectors ynew and ypnew from y0 and yp0,
 using the current step vector lambda*delta, in a manner
 depending on icopt and the input id vector.
 
 The return value is always SUCCESS = 0.
 
*****************************************************************/

static int IDANewyyp (IDAMem IDA_mem, realtype lambda)
{
  
  /* CALC_YA_YDP_INIT case: ynew = y0  - lambda*delta    where id_i = 0
                           ypnew = yp0 - cj*lambda*delta where id_i = 1. */
  if (icopt == CALC_YA_YDP_INIT) {
    N_VProd (id, delta, dtemp);
    N_VLinearSum (ONE, yp0, -cj*lambda, dtemp, ypnew);
    N_VLinearSum (ONE, delta, -ONE, dtemp, dtemp);
    N_VLinearSum (ONE, y0, -lambda, dtemp, ynew);
    return(SUCCESS);
  }

  /* CALC_Y_INIT case: ynew = y0 - lambda*delta. (ypnew = yp0 preset.) */
  N_VLinearSum (ONE, y0, -lambda, delta, ynew);
  return(SUCCESS);

}


/******************** IDANewy ***********************************

 IDANewy updates the vector ynew from y0,
 using the current step vector delta, in a manner
 depending on icopt and the input id vector.
 
 The return value is always SUCCESS = 0.
 
*****************************************************************/

static int IDANewy (IDAMem IDA_mem)
{
  
  /* CALC_YA_YDP_INIT case: ynew = y0  - delta    where id_i = 0. */
  if (icopt == CALC_YA_YDP_INIT) {
    N_VProd (id, delta, dtemp);
    N_VLinearSum (ONE, delta, -ONE, dtemp, dtemp);
    N_VLinearSum (ONE, y0, -ONE, dtemp, ynew);
    return(SUCCESS);
  }

  /* CALC_Y_INIT case: ynew = y0 - delta. */
  N_VLinearSum (ONE, y0, -ONE, delta, ynew);
  return(SUCCESS);

}


/******************** IDAICFailFlag *****************************

 IDAICFailFlag prints a message and sets the IDACalcIC return
 value appropriate to the flag retval returned by IDAnlsIC.
 
*****************************************************************/

static int IDAICFailFlag (IDAMem IDA_mem, int retval)
{

  /* Depending on retval, print error message and return error flag. */
  switch (retval) {

    case RES_NONRECOV_ERR:  if(errfp!=NULL) fprintf(errfp, MSG_IC_RES_NONREC);
                         return(RES_NONRECOV_ERR);

    case IC_FIRST_RES_FAIL:  if(errfp!=NULL) fprintf(errfp, MSG_IC_RES_FAIL);
                         return(IC_FIRST_RES_FAIL);

    case SETUP_FAILURE:  if(errfp!=NULL) fprintf(errfp, MSG_IC_SETUP_FAIL);
                         return(SETUP_FAILURE);

    case SOLVE_FAILURE:  if(errfp!=NULL) fprintf(errfp, MSG_IC_SOLVE_FAIL);
                         return(SOLVE_FAILURE);

    case IC_FAIL_RECOV:  if(errfp!=NULL) fprintf(errfp, MSG_IC_NO_RECOVERY);
                         return(IC_NO_RECOVERY);

    case IC_CONSTR_FAILED: if(errfp!=NULL) fprintf(errfp, MSG_IC_FAIL_CONSTR);
                         return(IC_FAILED_CONSTR);

    case IC_LINESRCH_FAILED:  if(errfp!=NULL) fprintf(errfp, MSG_IC_FAILED_LINS);
                         return(IC_FAILED_LINESRCH);

    case IC_CONV_FAIL:   if(errfp!=NULL) fprintf(errfp, MSG_IC_CONV_FAILED);
                         return(IC_CONV_FAILURE);

    case IC_SLOW_CONVRG: if(errfp!=NULL) fprintf(errfp, MSG_IC_CONV_FAILED);
                         return(IC_CONV_FAILURE);

    case IC_BAD_EWT:     if(errfp!=NULL) fprintf(errfp, MSG_IC_BAD_EWT);
                         return(IC_BAD_EWT);

  }
  return -99;
}

/**************************************************************/
/** END Support routines for Initial Conditions calculation   */
/**************************************************************/

/********************** IDAInitialSetup *********************************

 This routine is called by IDASolve once at the first step. It performs
 all checks on optional inputs and inputs to IDAMalloc/IDAReInit that
 could not be done before.

 If no merror is encountered, IDAInitialSetup returns SUCCESS. Otherwise,
 it returns an error flag and prints a message to errfp.
*************************************************************************/

static int IDAInitialSetup(IDAMem IDA_mem)
{
  realtype temptest;
  booleantype ewtsetOK, conOK;
  
  /* Test for more vector operations, depending on options */

  if (suppressalg)
    if (id->ops->nvwrmsnormmask == NULL) {
      if(errfp!=NULL) fprintf(errfp, MSG_BAD_NVECTOR);
      return(ILL_INPUT);
  }

  if (constraints != NULL)
    if (constraints->ops->nvdiv         == NULL ||
        constraints->ops->nvmaxnorm     == NULL ||
        constraints->ops->nvcompare     == NULL ||
        constraints->ops->nvconstrmask  == NULL ||
        constraints->ops->nvminquotient == NULL) {
      if(errfp!=NULL) fprintf(errfp, MSG_BAD_NVECTOR);
      return(ILL_INPUT);
    }

  /* Test id vector for legality */
  
  if(suppressalg && (id==NULL)){ 
    if(errfp!=NULL) fprintf(errfp, MSG_MISSING_ID); 
    return(ILL_INPUT); 
  }

  /* Load ewt */

  ewtsetOK = IDAEwtSet(IDA_mem, y0);
  if (!ewtsetOK) {
    if(errfp!=NULL) fprintf(errfp, MSG_BAD_EWT);
    return(ILL_INPUT);
  }

  /*  Check the constraints pointer and vector */
  
  if (constraints == NULL) 
    constraintsSet = FALSE;
  else {
    constraintsSet = TRUE;
    temptest = N_VMaxNorm(constraints);
    if(temptest > TWOPT5){ 
      if(errfp!=NULL) fprintf(errfp, MSG_BAD_CONSTRAINTS); 
      return(ILL_INPUT); 
    } else if(temptest < HALF) constraintsSet = FALSE; /* constraints empty */
  }

  /* Check to see if y0 satisfies constraints. */

  if (constraintsSet) {
    conOK = N_VConstrMask (constraints, y0, tempv2);
    if (!conOK) { 
      if(errfp!=NULL) fprintf(errfp, MSG_Y0_FAIL_CONSTR); 
      return(ILL_INPUT); 
    }
  }

  /* Check linear solver functions and call linit function. */

  if (linit == NULL) {
    if(errfp!=NULL) fprintf(errfp, MSG_LINIT_NULL);
    return(ILL_INPUT);
  }
  if (lsetup == NULL) {
    if(errfp!=NULL) fprintf(errfp, MSG_LSETUP_NULL);
    return(ILL_INPUT);
  }
  if (lsolve == NULL) {
    if(errfp!=NULL) fprintf(errfp, MSG_LSOLVE_NULL);
    return(ILL_INPUT);
  }
  if (lfree == NULL) {
    if(errfp!=NULL) fprintf(errfp, MSG_LFREE_NULL);
    return(ILL_INPUT);
  }
  /* Call linit if not already successfully called by IDACalcIC */
  if (!linitOK) {
    linitOK = (linit(IDA_mem) == LINIT_OK);
    if (!linitOK) {
      if(errfp!=NULL) fprintf(errfp, MSG_LINIT_FAIL);
      return(ILL_INPUT);
    }
  }
  
  return(SUCCESS);

}

/*********************** IDAEwtSet **************************************
  
 This routine is responsible for loading the error weight vector
 ewt, according to itol, as follows:
 (1) ewt[i] = 1 / (*rtol * ABS(ycur[i]) + *atol), i=0,...,Neq-1
     if itol = SS
 (2) ewt[i] = 1 / (*rtol * ABS(ycur[i]) + atol[i]), i=0,...,Neq-1
     if itol = SV

  IDAEwtSet returns TRUE if ewt is successfully set as above to a
  positive vector and FALSE otherwise. In the latter case, ewt is
  considered undefined after the FALSE return from IDAEwtSet.

  All the real work is done in the routines IDAEwtSetSS, IDAEwtSetSV.
 
***********************************************************************/

static booleantype IDAEwtSet(IDAMem IDA_mem, N_Vector ycur)
{
  booleantype ewtsetOK=TRUE;

  switch(itol) {
  case SS: 
    ewtsetOK = IDAEwtSetSS(IDA_mem, ycur); 
    break;
  case SV: 
    ewtsetOK = IDAEwtSetSV(IDA_mem, ycur); 
    break;
  }
  return(ewtsetOK);
}

/*********************** IDAEwtSetSS *********************************

 This routine sets ewt as decribed above in the case itol=SS.
 It tests for non-positive components before inverting. IDAEwtSetSS
 returns TRUE if ewt is successfully set to a positive vector
 and FALSE otherwise. In the latter case, ewt is considered
 undefined after the FALSE return from IDAEwtSetSS.

********************************************************************/

static booleantype IDAEwtSetSS(IDAMem IDA_mem, N_Vector ycur)
{
  realtype rtoli, *atoli;
  
  rtoli = *rtol;
  atoli = (realtype *)atol;
  N_VAbs(ycur, tempv1);
  N_VScale(rtoli, tempv1, tempv1);
  N_VAddConst(tempv1, *atoli, tempv1);
  if (N_VMin(tempv1) <= ZERO) return(FALSE);
  N_VInv(tempv1, ewt);
  return(TRUE);
}

/*********************** IDAEwtSetSV *********************************

 This routine sets ewt as decribed above in the case itol=SV.
 It tests for non-positive components before inverting. IDAEwtSetSV
 returns TRUE if ewt is successfully set to a positive vector
 and FALSE otherwise. In the latter case, ewt is considered
 undefined after the FALSE return from IDAEwtSetSV.

********************************************************************/

static booleantype IDAEwtSetSV(IDAMem IDA_mem, N_Vector ycur)
{
  realtype rtoli;
  N_Vector atoli;
  
  rtoli = *rtol;
  atoli = (N_Vector)atol;
  N_VAbs(ycur, tempv1);
  N_VLinearSum(rtoli, tempv1, ONE, atoli, tempv1);
  if (N_VMin(tempv1) <= ZERO) return(FALSE);
  N_VInv(tempv1, ewt);
  return(TRUE);
}



/********************* IDAStopTest1 ********************************

 This routine tests for stop conditions before taking a step.
 The tests depend on the value of itask.
 The variable tretp is the previously returned value of tret.

 The return values are:
   CONTINUE_STEPS       if no stop conditions were found
   NORMAL_RETURN        for a normal return to the user
   INTERMEDIATE_RETURN  for an intermediate-output return to the user
   TSTOP_RETURN         for a tstop-reached return to the user
   ILL_INPUT            for an illegal-input return to the user 

 In the tstop cases, this routine may adjust the stepsize hh to cause
 the next step to reach tstop exactly.

********************************************************************/

static int IDAStopTest1(IDAMem IDA_mem, realtype tout, realtype *tret, 
                        N_Vector yret, N_Vector ypret, int itask)
{

  int ier;
  realtype troundoff;

  switch (itask) {
    
  case NORMAL:  
    /* Test for tout = tretp, and for tn past tout. */
    if (tout == tretp) {
      *tret = tretp = tout;
      return(NORMAL_RETURN);
    }
    if ( (tn - tout)*hh >= ZERO) {
      ier = IDAGetSolution(IDA_mem, tout, yret, ypret);
      if (ier != OKAY) {
        if(errfp!=NULL) fprintf(errfp,MSG_BAD_TOUT, tout);
        return(ILL_INPUT);
      }
      *tret = tretp = tout;
      return(NORMAL_RETURN);
    }
    return(CONTINUE_STEPS);
    
  case ONE_STEP:
    /* Test for tn past tretp. */
    if ( (tn - tretp)*hh > ZERO) {
      ier = IDAGetSolution(IDA_mem, tn, yret, ypret);
      *tret = tretp = tn;
      return(INTERMEDIATE_RETURN);
    }
    return(CONTINUE_STEPS);
    
  case NORMAL_TSTOP:
    /* Test for tn past tstop, tn = tretp, tn past tout, tn near tstop. */
    if ( (tn - tstop)*hh > ZERO) {
      if(errfp!=NULL) fprintf(errfp, MSG_BAD_TSTOP, tstop, tn);
      return(ILL_INPUT);
    }
    if (tout == tretp) {
      *tret = tretp = tout;
      return(NORMAL_RETURN);
    }
    if ( (tn - tout)*hh >= ZERO) {
      ier = IDAGetSolution(IDA_mem, tout, yret, ypret);
      if (ier != OKAY) {
        if(errfp!=NULL) fprintf(errfp, MSG_BAD_TOUT, tout);
        return(ILL_INPUT);
      }
      *tret = tretp = tout;
      return(NORMAL_RETURN);
    }
    troundoff = HUNDRED*uround*(ABS(tn) + ABS(hh));
    if ( ABS(tn - tstop) <= troundoff) {
      ier = IDAGetSolution(IDA_mem, tstop, yret, ypret);
      if (ier != OKAY) {
        if(errfp!=NULL) fprintf(errfp, MSG_BAD_TSTOP, tstop, tn);
        return(ILL_INPUT);
      }
      *tret = tretp = tstop;
      return(TSTOP_RETURN);
    }
    if ( (tn + hh - tstop)*hh > ZERO) hh = tstop - tn;
    return(CONTINUE_STEPS);
    
  case ONE_STEP_TSTOP:
    /* Test for tn past tstop, tn past tretp, and tn near tstop. */
    if ( (tn - tstop)*hh > ZERO) {
      if(errfp!=NULL) fprintf(errfp, MSG_BAD_TSTOP, tstop, tn);
      return(ILL_INPUT);
    }
    if ( (tn - tretp)*hh > ZERO) {
      ier = IDAGetSolution(IDA_mem, tn, yret, ypret);
      *tret = tretp = tn;
      return(INTERMEDIATE_RETURN);
    }
    troundoff = HUNDRED*uround*(ABS(tn) + ABS(hh));
    if ( ABS(tn - tstop) <= troundoff) {
      ier = IDAGetSolution(IDA_mem, tstop, yret, ypret);
      if (ier != OKAY) {
        if(errfp!=NULL) fprintf(errfp, MSG_BAD_TSTOP, tstop, tn);
        return(ILL_INPUT);
      }
      *tret = tretp = tstop;
      return(TSTOP_RETURN);
    }
    if ( (tn + hh - tstop)*hh > ZERO) hh = tstop - tn;
    return(CONTINUE_STEPS);
    
  }
  return -99;
}

/********************* IDAStopTest2 ********************************

 This routine tests for stop conditions after taking a step.
 The tests depend on the value of itask.

 The return values are:
   CONTINUE_STEPS       if no stop conditions were found
   NORMAL_RETURN        for a normal return to the user
   INTERMEDIATE_RETURN  for an intermediate-output return to the user
   TSTOP_RETURN         for a tstop-reached return to the user

 In the two cases with tstop, this routine may reset the stepsize hh
 to cause the next step to reach tstop exactly.

 In the two cases with ONE_STEP mode, no interpolation to tn is needed
 because yret and ypret already contain the current y and y' values.

 Note: No test is made for an error return from IDAGetSolution here,
 because the same test was made prior to the step.

********************************************************************/

static int IDAStopTest2(IDAMem IDA_mem, realtype tout, realtype *tret, 
                        N_Vector yret, N_Vector ypret, int itask)
{

  int ier;
  realtype troundoff;

  switch (itask) {

    case NORMAL:  
      /* Test for tn past tout. */
      if ( (tn - tout)*hh >= ZERO) {
        ier = IDAGetSolution(IDA_mem, tout, yret, ypret);
        *tret = tretp = tout;
        return(NORMAL_RETURN);
      }
      return(CONTINUE_STEPS);

    case ONE_STEP:
      *tret = tretp = tn;
      return(INTERMEDIATE_RETURN);

    case NORMAL_TSTOP:
      /* Test for tn at tstop, for tn past tout, and for tn near tstop. */
      troundoff = HUNDRED*uround*(ABS(tn) + ABS(hh));
      if ( ABS(tn - tstop) <= troundoff) {
        ier = IDAGetSolution(IDA_mem, tstop, yret, ypret);
        *tret = tretp = tstop;
        return(TSTOP_RETURN);
      }
      if ( (tn - tout)*hh >= ZERO) {
        ier = IDAGetSolution(IDA_mem, tout, yret, ypret);
        *tret = tretp = tout;
        return(NORMAL_RETURN);
      }
      if ( (tn + hh - tstop)*hh > ZERO) hh = tstop - tn;
      return(CONTINUE_STEPS);

    case ONE_STEP_TSTOP:
      /* Test for tn at tstop. */
      troundoff = HUNDRED*uround*(ABS(tn) + ABS(hh));
      if ( ABS(tn - tstop) <= troundoff) {
        ier = IDAGetSolution(IDA_mem, tstop, yret, ypret);
        *tret = tretp = tstop;
        return(TSTOP_RETURN);
      }
      if ( (tn + hh - tstop)*hh > ZERO) hh = tstop - tn;
      *tret = tretp = tn;
      return(INTERMEDIATE_RETURN);

  }
  return -99;
}


/****************** IDAHandleFailure ******************************

 This routine prints error messages for all cases of failure by
 IDAStep.  It returns to IDASolve the value that it is to return to
 the user.

*****************************************************************/

static int IDAHandleFailure(IDAMem IDA_mem, int sflag)
{

  /* Depending on sflag, print error message and return error flag */
  switch (sflag) {

    case REP_ERR_FAIL:  if(errfp!=NULL) fprintf(errfp, MSG_ERR_FAILS, tn, hh);
                        return(ERR_FAILURE);

    case REP_CONV_FAIL: if(errfp!=NULL) fprintf(errfp, MSG_CONV_FAILS, tn, hh);
                        return(CONV_FAILURE);

    case LSETUP_ERROR_NONRECVR:  if(errfp!=NULL) fprintf(errfp, MSG_SETUP_FAILED, tn);
                        return(SETUP_FAILURE);

    case CONV_FAIL_LINR_NONRECVR:  if(errfp!=NULL) fprintf(errfp, MSG_SOLVE_FAILED, tn);
                        return(SOLVE_FAILURE);

    case REP_RES_ERR:   if(errfp!=NULL) fprintf(errfp, MSG_REP_RES_ERR, tn);
                        return(REP_RES_REC_ERR);

    case RES_ERROR_NONRECVR:  if(errfp!=NULL) fprintf(errfp, MSG_RES_NONRECOV, tn);
                        return(RES_NONRECOV_ERR);

    case CONSTR_FAIL:   if(errfp!=NULL) fprintf(errfp, MSG_FAILED_CONSTR, tn);
                        return(CONSTR_FAILURE);

  }
  return -99;
}

/********************* IDAStep **************************************
 
 This routine performs one internal IDA step, from tn to tn + hh.
 It calls other routines to do all the work.

 It solves a system of differential/algebraic equations of the form
       F(t,y,y') = 0, for one step. In IDA, tt is used for t,
 yy is used for y, and yp is used for y'. The function F is supplied as 'res'
 by the user.

 The methods used are modified divided difference, fixed leading 
 coefficient forms of backward differentiation formulas.
 The code adjusts the stepsize and order to control the local error per step.

 The main operations done here are as follows:
  * initialize various quantities;
  * setting of multistep method coefficients;
  * solution of the nonlinear system for yy at t = tn + hh;
  * deciding on order reduction and testing the local error;
  * attempting to recover from failure in nonlinear solver or error test;
  * resetting stepsize and order for the next step.
  * updating phi and other state data if successful;

 On a failure in the nonlinear system solution or error test, the
 step may be reattempted, depending on the nature of the failure.

 Variables or arrays (all in the IDAMem structure) used in IDAStep are:

 tt -- Independent variable.
 yy -- Solution vector at tt.
 yp -- Derivative of solution vector after successful stelp.
 res -- User-supplied function to evaluate the residual. See the 
        description given in file ida.h .
 lsetup -- Routine to prepare for the linear solver call. It may either
        save or recalculate quantities used by lsolve. (Optional)
 lsolve -- Routine to solve a linear system. A prior call to lsetup
        may be required. 
 hh  -- Appropriate step size for next step.
 ewt -- Vector of weights used in all convergence tests.
 phi -- Array of divided differences used by IDAStep. This array is composed 
       of  (maxord+1) nvectors (each of size Neq). (maxord+1) is the maximum 
       order for the problem, maxord, plus 1.
 
       Return values are:
       SUCCESS       RES_ERROR_NONRECVR        LSETUP_ERROR_NONRECVR       
                     CONV_FAIL_LINR_NONRECVR   REP_ERR_FAIL            
                     CONSTR_FAIL               REP_CONV_FAIL          
                     REP_RES_ERR            

********************************************************************/

static int IDAStep(IDAMem IDA_mem)
{
  realtype saved_t, ck, est;
  realtype terk, terkm1, erkm1;
  int ncf, nef, nflag, kflag;
  
  saved_t = tn;
  ncf = nef = 0;

  if(nst == ZERO){
    kk = 1;
    kused = 0;
    hused = ZERO;
    psi[0] = hh;
    cj = ONE/hh;
    phase = 0;
    ns = 0;
  }
  
  /* Looping point for attempts to take a step */

  loop {  
    IDASetCoeffs(IDA_mem, &ck);
    kflag = SUCCESS;

    nflag = IDAnls(IDA_mem);

    if(nflag == SUCCESS) nflag=IDATestError(IDA_mem, &ck, &est,
                                            &terk, &terkm1, &erkm1);

    if(nflag != SUCCESS) kflag=IDAHandleNFlag(IDA_mem, nflag, saved_t, 
                                              &ncf, &nef, &est);
    if (kflag == PREDICT_AGAIN) continue;
    else if(kflag == SUCCESS) break;
    else return(kflag);
  }

  /* Nonlinear system solve and error test were both successful;
     update data, and consider change of step and/or order       */

  IDACompleteStep(IDA_mem, &est, &terk, &terkm1, &erkm1);

  return(SUCCESS);
}


/********************* IDASetCoeffs ********************************

  This routine computes the coefficients relevant to the current step.
  The counter ns counts the number of consecutive steps taken at
  constant stepsize h and order k, up to a maximum of k + 2.
  Then the first ns components of beta will be one, and on a step  
  with ns = k + 2, the coefficients alpha, etc. need not be reset here.
  Also, IDACompleteStep prohibits an order increase until ns = k + 2.

***********************************************************************/

static void IDASetCoeffs(IDAMem IDA_mem, realtype *ck)
{
  int i;
  realtype temp1, temp2, alpha0, alphas;

  /* Set coefficients for the current stepsize h */

  if(hh != hused || kk != kused) ns = 0;
  ns = MIN(ns+1,kused+2);
  if(kk+1 >= ns){
    beta[0] = ONE;
    alpha[0] = ONE;
    temp1 = hh;
    gamma[0] = ZERO;
    sigma[0] = ONE;
    for(i=1;i<=kk;i++){
      temp2 = psi[i-1];
      psi[i-1] = temp1;
      beta[i] = beta[i-1] * psi[i-1] / temp2;
      temp1 = temp2 + hh;
      alpha[i] = hh / temp1;
      sigma[i] = i * sigma[i-1] * alpha[i]; 
      gamma[i] = gamma[i-1] + alpha[i-1] / hh;
   }
    psi[kk] = temp1;
  }
  /* compute alphas, alpha0 */
  alphas = ZERO;
  alpha0 = ZERO;
  for(i=0;i<kk;i++){
    alphas = alphas - ONE/(i+1);
    alpha0 = alpha0 - alpha[i];
  }

  /* compute leading coefficient cj  */
  cjlast = cj;
  cj = -alphas/hh;
  
  /* compute variable stepsize error coefficient ck */

    *ck = ABS(alpha[kk] + alphas - alpha0);
    *ck = MAX(*ck, alpha[kk]);

 /* change phi to phi-star  */

  for(i=ns;i<=kk;i++) N_VScale(beta[i], phi[i], phi[i]);

  /* update independent variable */

  tn = tn + hh;
}


/****************** IDAnls *****************************************

 This routine attempts to solve the nonlinear system using the linear
 solver specified. NOTE: this routine uses N_Vector ee as the scratch
 vector tempv3 passed to lsetup.

  Possible return values:

  SUCCESS

  RES_ERROR_RECVR       RES_ERROR_NONRECVR
  LSETUP_ERROR_RECVR    LSETUP_ERROR_NONRECVR
  CONV_FAIL_LINR_RECVR  CONV_FAIL_LINR_NONRECVR

  CONSTRAINT_FAIL_RECVR
  CONV_FAIL_NLINR_RECVR


**********************************************************************/

static int IDAnls(IDAMem IDA_mem)
{
  int retval, ier;
  booleantype constraintsPassed, callSetup, tryAgain;
  realtype temp1, temp2, vnorm;
  N_Vector tempv3;

  callSetup = FALSE;

  /* Initialize if the first time called */

  if(nst == 0){
    cjold = cj;
    ss = TWENTY;
    if(setupNonNull) callSetup = TRUE;
  }

  mm = tempv2;
  tempv3 = ee;


  /* Decide if lsetup is to be called */

  if(setupNonNull){
    cjratio = cj / cjold;
    temp1 = (ONE - XRATE) / (ONE + XRATE);
    temp2 = ONE/temp1;
    {if(cjratio < temp1 || cjratio > temp2) callSetup = TRUE;}
    {if(cj != cjlast) ss=HUNDRED;}
  }

  /* Begin the main loop. This loop is traversed at most twice. 
     The second pass only occurs when the first pass had a recoverable
     failure with old Jacobian data */
  loop{


    /* Compute predicted values for yy and yp, and compute residual there. */
    ier = IDAPredict(IDA_mem);

    retval = res(tn, yy, yp, delta, rdata);
    nre++;
    if(retval != SUCCESS) break;

    /* If indicated, call linear solver setup function and reset parameters. */
    if(callSetup){
      nsetups++;

      retval = lsetup(IDA_mem, yy, yp, delta, tempv1, tempv2, tempv3);

      cjold = cj;
      cjratio = ONE;
      ss = TWENTY;
      if(retval != SUCCESS) break;
    }

    /* Call the Newton iteration routine.  */

    retval = IDANewtonIter(IDA_mem);


    /* Retry the current step on recoverable failure with old Jacobian data. */

    tryAgain = (retval>0)&&(setupNonNull) &&(!callSetup);

    if(tryAgain){
      callSetup = TRUE;
      continue;
    }
    else break;

  }  /* end of loop */

  if(retval != SUCCESS) return(retval);


  /* If otherwise successful, check and enforce inequality constraints. */

  if(constraintsSet){  /* Check constraints and get mask vector mm, 
                          set where constraints failed */
    constraintsPassed = N_VConstrMask(constraints,yy,mm);
    if(constraintsPassed) return(SUCCESS);
    else {
      N_VCompare(ONEPT5, constraints, tempv1);  
      /* a , where a[i] =1. when |c[i]| = 2 ,  c the vector of constraints */
      N_VProd(tempv1, constraints, tempv1);       /* a * c */
      N_VDiv(tempv1, ewt, tempv1);                /* a * c * wt */
      N_VLinearSum(ONE, yy, -PT1, tempv1, tempv1);/* y - 0.1 * a * c * wt */
      N_VProd(tempv1, mm, tempv1);               /*  v = mm*(y-.1*a*c*wt) */
      vnorm = IDAWrmsNorm(IDA_mem, tempv1, ewt, FALSE); /*  ||v|| */
      
      /* If vector v of constraint corrections is small
         in norm, correct and accept this step */      
      if(vnorm <= epsNewt){  
        N_VLinearSum(ONE, ee, -ONE, tempv1, ee);  /* ee <- ee - v */
        return(SUCCESS);
      }
      else {
        /* Constraints not met -- reduce h by computing rr = h'/h */
        N_VLinearSum(ONE, phi[0], -ONE, yy, tempv1);
        N_VProd(mm, tempv1, tempv1);
        rr = PT9*N_VMinQuotient(phi[0], tempv1);
        rr = MAX(rr,PT1);
        return(CONSTRAINT_FAIL_RECVR);
      }
    }
  }
  return(SUCCESS);
}


/***************** IDAPredict ********************************************

  This routine predicts the new values for vectors yy and yp.

*************************************************************************/

static int IDAPredict(IDAMem IDA_mem)
{
  int j;

  N_VScale(ONE, phi[0], yy);
  N_VConst(ZERO, yp);
  
  for(j=1; j<=kk; j++) {
    N_VLinearSum(ONE,      phi[j], ONE, yy, yy);
    N_VLinearSum(gamma[j], phi[j], ONE, yp, yp);
  }

  return(SUCCESS);
}


/********************** IDANewtonIter *********************************

 This routine performs the Newton iteration.  
 It assumes that delta contains the initial residual vector on entry.
 If the iteration succeeds, it returns the value SUCCESS = 0.
 If not, it returns either:
   a positive value (for a recoverable failure), namely one of:
     RES_ERROR_RECOVR
     CONV_FAIL_LINR_RECVR
     CONV_FAIL_NLINR_RECVR
 or
   a negative value (for a nonrecoverable failure), namely one of:
     RES_ERROR_NONRECOVR
     CONV_FAIL_LINR_NONRECVR

     NOTE: This routine uses N_Vector savres, which is preset to tempv1.

***********************************************************************/

static int IDANewtonIter(IDAMem IDA_mem)
{
  int mnewt, retval;
  realtype delnrm, oldnrm, rate;

  /* Initialize counter mnewt and cumulative correction vector ee. */
  mnewt = 0;
  N_VConst (ZERO, ee);

  /* Looping point for Newton iteration.  Break out on any error. */
  loop {

    /* Save a copy of the residual vector in savres. */
    N_VScale(ONE, delta, savres);

    /* Call the lsolve function to get correction vector delta. */
    retval = lsolve(IDA_mem, delta, ewt, yy, yp, savres); 
    nni++;
    if (retval != SUCCESS) break;

    /* Apply delta to yy, yp, and ee, and get norm(delta). */
    N_VLinearSum(ONE, yy, -ONE, delta, yy);
    N_VLinearSum(ONE, ee, -ONE, delta, ee);
    N_VLinearSum(ONE, yp, -cj,  delta, yp);
    delnrm = IDAWrmsNorm(IDA_mem, delta, ewt, FALSE);

    /* Test for convergence, first directly, then with rate estimate. */

    if (mnewt == 0){ 
       oldnrm = delnrm;
       if (delnrm <= toldel) return(SUCCESS);
    }
    else {
      rate = RPowerR( delnrm/oldnrm, ONE/mnewt );
      if (rate > RATEMAX) { retval = CONV_FAIL_NLINR_RECVR; break;}
      ss = rate/(ONE - rate);
    }

    if (ss*delnrm <= epsNewt) return(SUCCESS);

    /* Not yet converged.  Increment mnewt and test for max allowed. */
    mnewt++;
    if (mnewt >= maxcor) {retval = CONV_FAIL_NLINR_RECVR; break;}

    /* Call res for new residual and check error flag from res. */
    retval = res(tn, yy, yp, delta, rdata);
    nre++;
    if (retval != SUCCESS) break;

    /* Loop for next iteration. */

  } /* end of Newton iteration loop */

  /* All error returns exit here. */
  return(retval);

}


/******************* IDATestError ********************************

 This routine estimates errors at orders k, k-1, k-2, decides whether 
 or not to reduce order, and performs the local error test. 

 IDATestError returns either  SUCCESS   or ERROR_TEST_FAIL

******************************************************************/

static int IDATestError(IDAMem IDA_mem, realtype *ck, realtype *est,
                        realtype *terk, realtype *terkm1, realtype *erkm1)
{
  int retval;
  realtype enorm;
  realtype terkm2;
  realtype erk, erkm2;

  /* Compute error for order k. */
  enorm = IDAWrmsNorm(IDA_mem, ee, ewt, suppressalg);
  erk = sigma[kk] * enorm;
  *terk = (kk+1) * erk;
  *est = erk;
  knew = kk;

  /* Now compute the errors for orders k-1 and k-2, and decide whether to 
     reduce the order k to k-1 */
  
  if(kk > 1){
    N_VLinearSum(ONE, phi[kk], ONE, ee, delta);
    *erkm1 = sigma[kk-1] * IDAWrmsNorm(IDA_mem, delta, ewt, suppressalg);
    *terkm1 = kk * *erkm1;
    {
      if(kk > 2){
        N_VLinearSum(ONE, phi[kk-1], ONE, delta, delta);
        erkm2 = sigma[kk-2] * IDAWrmsNorm(IDA_mem, delta, ewt, suppressalg);
        terkm2 = (kk-1) * erkm2;
        if(MAX(*terkm1, terkm2) > *terk) goto evaltest;
      }
      
      else if(*terkm1 > 0.5*(*terk)) goto evaltest; /* executed for kk=2 only */
    }
    /* end of "kk>2" if/else block */
    
    knew = kk-1;
    *est = *erkm1;
    
  } /* end kk>1 if block */ 
  
  
 evaltest:
  retval = SUCCESS;
  
  if ((*ck * enorm) > ONE) retval = ERROR_TEST_FAIL;
  return(retval);
}


/********************** IDAHandleNFlag *******************************

This routine handles failures indicated by the input variable nflag. 
Positive values indicate various recoverable failures while negative
values indicate nonrecoverable failures. This routine adjusts the
step size for recoverable failures. The possible return values are:

(recoverable)
PREDICT_AGAIN

(nonrecoverable)
CONSTR_FAIL      REP_RES_ERR        RES_ERROR_NONRECVR      [from IDAnls] 
REP_ERR_FAIL     REP_CONV_FAIL      LSETUP_ERROR_NONRECVR   [from IDAnls]
                                    CONV_FAIL_LINR_NONRECVR [from IDAnls]
     
**********************************************************************/

static int IDAHandleNFlag(IDAMem IDA_mem, int nflag, realtype saved_t,
                          int *ncfPtr, int *nefPtr, realtype *est)
{
  int j, retval;
  int *ncf, *nef;
  
  ncf = ncfPtr; nef = nefPtr;
  phase = 1;
  
  tn = saved_t; /* restore tn */
  
  /* restore phi and psi */
  
  for (j = ns; j<=kk; j++) N_VScale(ONE/beta[j], phi[j], phi[j]);
  
  for (j = 1; j <= kk; j++) psi[j-1] = psi[j] - hh;
  
  loop{  /* begin 'breakout' loop */
    
    if (nflag < 0) {    /*  nonrecoverable failure cases */
      retval = nflag; ncfn++;
      break;
    }
    
    /* Only positive nflag values (apparently recoverable) will appear here*/
    
    else if (nflag != ERROR_TEST_FAIL) {   /*  misc. recoverable cases  */
      (*ncf)++; ncfn++;
      if (nflag != CONSTRAINT_FAIL_RECVR) rr = QUARTER;
      hh *= rr;
      if (*ncf < maxncf){
        retval = PREDICT_AGAIN;
        break;
      }
      else if (nflag == RES_ERROR_RECVR) {
        retval= REP_RES_ERR;
        break;
      }
      else if (nflag == CONSTRAINT_FAIL_RECVR) {
        retval = CONSTR_FAIL;
        break;
      }
      else {
        retval = REP_CONV_FAIL;
        break;
      }
    }
    else {    /* error test failed case */
      (*nef)++; netf++;
      
      if (*nef == 1){
        /* On first error test failure, keep current order or lower order 
           by one. Compute new stepsize based on differences of the solution. */
        kk = knew;
        
        rr = PT9 * RPowerR( TWO*(*est) + PT0001,(-ONE/(kk+1)) );
        rr = MAX(QUARTER, MIN(PT9,rr));
        hh *=rr;  /* adjust step size */
        
        retval = PREDICT_AGAIN;
        break;
      }
      else if (*nef == 2){
        /* On second error test failure, use current order or decrease order 
           by one. Reduce stepsize by factor of 1/4. */
        kk = knew;
        rr = QUARTER;
        hh *= rr;
        
        retval = PREDICT_AGAIN;
        break;
      }
      else if (*nef > 2){
        /* On third and subsequent error test failures, set order to 1 and
           reduce stepsize h by factor of 1/4. */
        if (*nef<maxnef){
          kk = 1;
          rr = QUARTER;
          hh *= rr;
          retval = PREDICT_AGAIN;
          break;
        }
        else {
          retval = REP_ERR_FAIL;
          break;
        }
      }
    } /* end of nflag  if block */
    
  } /* end of 'breakout loop' */
  
  if (retval < 0) return(retval);
  
  if (retval == PREDICT_AGAIN) {
    if (nst == 0){
      psi[0] = hh;
      N_VScale(rr, phi[1], phi[1]);
    }
    return(PREDICT_AGAIN);
  }
  return -99;  
}


/********************* IDACompleteStep *********************************

 This routine completes a successful step.  It increments nst,
 saves the stepsize and order used, makes the final selection of
 stepsize and order for the next step, and updates the phi array.
 Its return value is SUCCESS= 0.

***********************************************************************/

static int IDACompleteStep(IDAMem IDA_mem, realtype *est, 
                           realtype *terk, realtype *terkm1, realtype *erkm1)
{
  int j, kdiff, action;
  realtype terkp1, erkp1, temp, hnew;

  nst++;
  kdiff = kk - kused;
  kused = kk;
  hused = hh;

  if ( (knew == kk-1) || (kk == maxord) ) phase = 1;

  /* For the first few steps, until either a step fails, or the order is 
  reduced, or the order reaches its maximum, we raise the order and double 
  the stepsize. During these steps, phase = 0. Thereafter, phase = 1, and
  stepsize and order are set by the usual local error algorithm. 

  Note that, after the first step, the order is not increased, as not all
  of the neccessary information is available yet.
  */
  
  if (phase == 0) {

    if (nst > 1) {
      kk++;
      hnew = TWO * hh;
      hh = hnew;
    }

  } else {

    action = UNSET;
    
    /* Set action = LOWER/MAINTAIN/RAISE to specify order decision */
    
    if (knew == kk-1)   {action = LOWER; goto takeaction;}
    if (kk == maxord)   {action = MAINTAIN; goto takeaction;}
    if ( (kk+1 >= ns ) || (kdiff == 1)) {action = MAINTAIN;goto takeaction;}
    
    /* Estimate the error at order k+1, unless already decided to
       reduce order, or already using maximum order, or stepsize has not
       been constant, or order was just raised. */
    
    N_VLinearSum (ONE, ee, -ONE, phi[kk+1], delta);
    terkp1 = IDAWrmsNorm(IDA_mem, delta, ewt, suppressalg);
    erkp1= terkp1/(kk+2);
    
    /* Choose among orders k-1, k, k+1 using local truncation error norms. */
    
    if (kk == 1) {
      if (terkp1 >= HALF * (*terk)) {action = MAINTAIN; goto takeaction;}
      else                          {action = RAISE;    goto takeaction;}
    }
    else {
      if (*terkm1 <= MIN(*terk, terkp1)) {action = LOWER;    goto takeaction;}
      if (terkp1  >= *terk)             {action = MAINTAIN; goto takeaction;}
      action = RAISE;
      goto takeaction;
    }
    
  takeaction:
    
    /* On change of order, reset kk and the estimated error norm. */
    
    if (action == RAISE) { kk++; *est = erkp1;}
    else if (action == LOWER) { kk--; *est = *erkm1;}
    
    /* Compute rr = tentative ratio hnew/hh from error norm.
       Reduce hh if rr <= 1, double hh if rr >= 2, else leave hh as is.
       If hh is reduced, hnew/hh is restricted to be between .5 and .9. */
    
    hnew = hh;
    rr = RPowerR( (TWO * (*est) + PT0001) , (-ONE/(kk+1) ) );
    
    if (rr >= TWO) {
      hnew = TWO * hh;
      if( (temp = ABS(hnew)*hmax_inv) > ONE ) hnew /= temp;
    }
    else if (rr <= ONE ) { 
      rr = MAX(HALF, MIN(PT9,rr));
      hnew = hh * rr;
    }
    
    hh = hnew;
    
  } /* end of phase if block */
  
  /* Save ee for possible order increase on next step; update phi array. */
  
  if (kused < maxord) N_VScale(ONE, ee, phi[kused+1]);
  
  N_VLinearSum(ONE, ee, ONE, phi[kused], phi[kused]);
  for (j= kused-1; j>=0; j--)
    N_VLinearSum(ONE, phi[j], ONE, phi[j+1], phi[j]);

  return (SUCCESS);
}

/*-----------------------  IDAWrmsNorm   --------------------------*/
/*
  Returns the WRMS norm of vector x with weights w.
  If mask = TRUE, the weight vector w is masked by id, i.e.,
      nrm = N_VWrmsNormMask(x,w,id);
  Otherwise,
      nrm = N_VWrmsNorm(x,w);
  
  mask = FALSE       when the call is made from the nonlinear solver.
  mask = suppressalg otherwise.
*/
/*-----------------------------------------------------------------*/

static realtype IDAWrmsNorm(IDAMem IDA_mem, 
                            N_Vector x, N_Vector w, 
                            booleantype mask)
{
  realtype nrm;

  if (mask) nrm = N_VWrmsNormMask(x, w, id);
  else      nrm = N_VWrmsNorm(x, w);

  return(nrm);
}

/********************************************************************/
/********* END Private Helper Functions Implementation **************/
/********************************************************************/


/********************************************************************/
/************** END IDA Implementation ******************************/
/********************************************************************/
