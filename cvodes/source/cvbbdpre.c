/*******************************************************************
 * File          : cvbbdpre.c                                      *
 * Programmers   : Michael Wittman, Alan C. Hindmarsh, and         *
 *                 Radu Serban @ LLNL                              *
 * Version of    : 07 February 2004                                *
 *-----------------------------------------------------------------*
 * Copyright (c) 2002, The Regents of the University of California * 
 * Produced at the Lawrence Livermore National Laboratory          *
 * All rights reserved                                             *
 * For details, see sundials/cvodes/LICENSE                        *
 *-----------------------------------------------------------------*
 * This file contains implementations of routines for a            *
 * band-block-diagonal preconditioner, i.e. a block-diagonal       *
 * matrix with banded blocks, for use with CVODE, CVSpgmr, and     *
 * the parallel implementation of NVECTOR.                         *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvodes.h"
#include "cvbbdpre.h"
#include "cvspgmr.h"
#include "sundialsmath.h"
#include "iterative.h"

#define MIN_INC_MULT RCONST(1000.0)
#define ZERO         RCONST(0.0)
#define ONE          RCONST(1.0)

/* Error Messages */
#define CVBBDALLOC     "CVBBDAlloc-- "
#define MSG_CVMEM_NULL CVBBDALLOC "Integrator memory is NULL.\n\n"
#define MSG_WRONG_NVEC CVBBDALLOC "Incompatible NVECTOR implementation.\n\n"
#define MSG_PDATA_NULL "CVBBDPrecGet*-- BBDPrecData is NULL. \n\n"

#define MSG_NO_PDATA   "CVBBDSpgmr-- BBDPrecData is NULL. \n\n"

/* Prototypes of functions CVBBDPrecSetup and CVBBDPrecSolve */

static int CVBBDPrecSetup(realtype t, N_Vector y, N_Vector fy, 
                          booleantype jok, booleantype *jcurPtr, 
                          realtype gamma, void *p_data, 
                          N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

static int CVBBDPrecSolve(realtype t, N_Vector y, N_Vector fy, 
                          N_Vector r, N_Vector z, 
                          realtype gamma, realtype delta,
                          int lr, void *p_data, N_Vector tmp);

/* Prototype for difference quotient Jacobian calculation routine */

static void CVBBDDQJac(CVBBDPrecData pdata, realtype t, 
                       N_Vector y, N_Vector gy, 
                       N_Vector ytemp, N_Vector gtemp);

/*****************************************************************************/

/* Redability replacements */
#define nvspec (cv_mem->cv_nvspec)
#define errfp  (cv_mem->cv_errfp)
#define uround (cv_mem->cv_uround)

/*********** User-Callable Functions: malloc, reinit, and free ***************/

void *CVBBDPrecAlloc(void *cvode_mem, long int Nlocal, 
                     long int mudq, long int mldq,
                     long int mukeep, long int mlkeep, 
                     realtype dqrely, 
                     CVLocalFn gloc, CVCommFn cfn)
{
  CVodeMem cv_mem;
  CVBBDPrecData pdata;
  long int muk, mlk, storage_mu;

  if (cvode_mem == NULL) {
    fprintf(stdout, MSG_CVMEM_NULL);
    return(NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  /* Test if the NVECTOR package is compatible with the BLOCK BAND preconditioner */
  if (nvspec->ops->nvgetdata == NULL || nvspec->ops->nvsetdata == NULL) {
    fprintf(errfp, MSG_WRONG_NVEC);
    return(NULL);
  }

  pdata = (CVBBDPrecData) malloc(sizeof *pdata);  /* Allocate data memory */
  if (pdata == NULL) return(NULL);

  /* Set pointers to gloc and cfn; load half-bandwidths */
  pdata->cvode_mem = cvode_mem;
  pdata->gloc = gloc;
  pdata->cfn = cfn;
  pdata->mudq = MIN( Nlocal-1, MAX(0,mudq) );
  pdata->mldq = MIN( Nlocal-1, MAX(0,mldq) );
  muk = MIN( Nlocal-1, MAX(0,mukeep) );
  mlk = MIN( Nlocal-1, MAX(0,mlkeep) );
  pdata->mukeep = muk;
  pdata->mlkeep = mlk;

  /* Allocate memory for saved Jacobian */
  pdata->savedJ = BandAllocMat(Nlocal, muk, mlk, muk);
  if (pdata->savedJ == NULL) { free(pdata); return(NULL); }

  /* Allocate memory for preconditioner matrix */
  storage_mu = MIN(Nlocal-1, muk + mlk);
  pdata->savedP = BandAllocMat(Nlocal, muk, mlk, storage_mu);
  if (pdata->savedP == NULL) {
    BandFreeMat(pdata->savedJ);
    free(pdata);
    return(NULL);
  }
  /* Allocate memory for pivots */
  pdata->pivots = BandAllocPiv(Nlocal);
  if (pdata->savedJ == NULL) {
    BandFreeMat(pdata->savedP);
    BandFreeMat(pdata->savedJ);
    free(pdata);
    return(NULL);
  }

  /* Set pdata->dqrely based on input dqrely (0 implies default). */
  pdata->dqrely = (dqrely > ZERO) ? dqrely : RSqrt(uround);

  /* Store Nlocal to be used in CVBBDPrecSetup */
  pdata->n_local = Nlocal;

  /* Set work space sizes and initialize nge */
  pdata->rpwsize = Nlocal*(muk + 2*mlk + storage_mu + 2);
  pdata->ipwsize = Nlocal;
  pdata->nge = 0;

  return((void *)pdata);
}

int CVBBDSpgmr(void *cvode_mem, int pretype, int maxl, void *p_data)
{
  int flag;

  if ( p_data == NULL ) {
    fprintf(stdout, MSG_NO_PDATA);
    return(BBDP_NO_PDATA);
  } 

  flag = CVSpgmr(cvode_mem, pretype, maxl);
  if(flag != SUCCESS) return(flag);

  flag = CVSpgmrSetPrecData(cvode_mem, p_data);
  if(flag != SUCCESS) return(flag);

  flag = CVSpgmrSetPrecSetupFn(cvode_mem, CVBBDPrecSetup);
  if(flag != SUCCESS) return(flag);

  flag = CVSpgmrSetPrecSolveFn(cvode_mem, CVBBDPrecSolve);
  if(flag != SUCCESS) return(flag);

  return(SUCCESS);
}

int CVBBDPrecReInit(void *p_data, 
                    long int mudq, long int mldq, 
                    realtype dqrely, 
                    CVLocalFn gloc, CVCommFn cfn)
{
  CVBBDPrecData pdata;
  CVodeMem cv_mem;
  long int Nlocal;

  pdata = (CVBBDPrecData) p_data;
  cv_mem = (CVodeMem) pdata->cvode_mem;

  /* Set pointers to gloc and cfn; load half-bandwidths */
  pdata->gloc = gloc;
  pdata->cfn = cfn;
  Nlocal = pdata->n_local;
  pdata->mudq = MIN( Nlocal-1, MAX(0,mudq) );
  pdata->mldq = MIN( Nlocal-1, MAX(0,mldq) );

  /* Set pdata->dqrely based on input dqrely (0 implies default). */
  pdata->dqrely = (dqrely > ZERO) ? dqrely : RSqrt(uround);

  /* Re-initialize nge */
  pdata->nge = 0;

  return(0);
}

void CVBBDPrecFree(void *p_data)
{
  CVBBDPrecData pdata;
  
  if ( p_data != NULL ) {
    pdata = (CVBBDPrecData) p_data;
    BandFreeMat(pdata->savedJ);
    BandFreeMat(pdata->savedP);
    BandFreePiv(pdata->pivots);
    free(pdata);
  }
}

int CVBBDPrecGetIntWorkSpace(void *p_data, long int *leniwBBDP)
{
  CVBBDPrecData pdata;

  if ( p_data == NULL ) {
    fprintf(stdout, MSG_PDATA_NULL);
    return(BBDP_NO_PDATA);
  } 

  pdata = (CVBBDPrecData) p_data;

  *leniwBBDP = pdata->ipwsize;

  return(OKAY);
}

int CVBBDPrecGetRealWorkSpace(void *p_data, long int *lenrwBBDP)
{
  CVBBDPrecData pdata;

  if ( p_data == NULL ) {
    fprintf(stdout, MSG_PDATA_NULL);
    return(BBDP_NO_PDATA);
  } 

  pdata = (CVBBDPrecData) p_data;

  *lenrwBBDP = pdata->rpwsize;

  return(OKAY);
}

int CVBBDPrecGetNumGfnEvals(void *p_data, long int *ngevalsBBDP)
{
  CVBBDPrecData pdata;

  if ( p_data == NULL ) {
    fprintf(stdout, MSG_PDATA_NULL);
    return(BBDP_NO_PDATA);
  } 

  pdata = (CVBBDPrecData) p_data;

  *ngevalsBBDP = pdata->nge;

  return(OKAY);
}

/***************** Preconditioner setup and solve Functions ****************/
 

/* Readability Replacements */

#define Nlocal    (pdata->n_local)
#define mudq      (pdata->mudq)
#define mldq      (pdata->mldq)
#define mukeep    (pdata->mukeep)
#define mlkeep    (pdata->mlkeep)
#define dqrely    (pdata->dqrely)
#define gloc      (pdata->gloc)
#define cfn       (pdata->cfn)
#define savedJ    (pdata->savedJ)
#define savedP    (pdata->savedP)
#define pivots    (pdata->pivots)
#define nge       (pdata->nge)


/******************************************************************
 * Function : CVBBDPrecSetup                                      *
 *----------------------------------------------------------------*
 * CVBBDPrecSetup generates and factors a banded block of the     *
 * preconditioner matrix on each processor, via calls to the      *
 * user-supplied gloc and cfn functions. It uses difference       *
 * quotient approximations to the Jacobian elements.              *
 *                                                                *
 * CVBBDPrecSetup calculates a new J,if necessary, then calculates*
 * P = I - gamma*J, and does an LU factorization of P.            *
 *                                                                *
 * The parameters of CVBBDPrecSetup used here are as follows:     *
 *                                                                *
 * t       is the current value of the independent variable.      *
 *                                                                *
 * y       is the current value of the dependent variable vector, *
 *         namely the predicted value of y(t).                    *
 *                                                                *
 * fy      is the vector f(t,y).                                  *
 *                                                                *
 * jok     is an input flag indicating whether Jacobian-related   *
 *         data needs to be recomputed, as follows:               *
 *           jok == FALSE means recompute Jacobian-related data   *
 *                  from scratch.                                 *
 *           jok == TRUE  means that Jacobian data from the       *
 *                  previous CVBBDPrecon call can be reused       *
 *                  (with the current value of gamma).            *
 *         A CVBBDPrecon call with jok == TRUE should only occur  *
 *         after a call with jok == FALSE.                        *
 *                                                                *
 * jcurPtr is a pointer to an output integer flag which is        *
 *         set by CVBBDPrecon as follows:                         *
 *           *jcurPtr = TRUE if Jacobian data was recomputed.     *
 *           *jcurPtr = FALSE if Jacobian data was not            * 
 *                      recomputed, but saved data was reused.    *
 *                                                                *
 * gamma   is the scalar appearing in the Newton matrix.          *
 *                                                                *
 * p_data  is a pointer to user data - the same as the P_data     *
 *           parameter passed to CVSpgmr. For CVBBDPrecon, this   *
 *           should be of type CVBBDData.                         *
 *                                                                *
 * tmp1, tmp2, and tmp3 are pointers to memory allocated          *
 *           for NVectors which are be used by CVBBDPrecSetup     *
 *           as temporary storage or work space.                  *
 *                                                                *
 *                                                                *
 * Return value:                                                  *
 * The value returned by this CVBBDPrecSetup function is the int  *
 *   0  if successful,                                            *
 *   1  for a recoverable error (step will be retried).           *
 ******************************************************************/

static int CVBBDPrecSetup(realtype t, N_Vector y, N_Vector fy, 
                          booleantype jok, booleantype *jcurPtr, 
                          realtype gamma, void *p_data, 
                          N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  long int ier;
  CVBBDPrecData pdata;

  pdata = (CVBBDPrecData) p_data;

  if (jok) {
    /* If jok = TRUE, use saved copy of J */
    *jcurPtr = FALSE;
    BandCopy(savedJ, savedP, mukeep, mlkeep);
  } else {
    /* Otherwise call CVBBDDQJac for new J value */
    *jcurPtr = TRUE;
    BandZero(savedJ);
    CVBBDDQJac(pdata, t, y, tmp1, tmp2, tmp3);
    nge += 1 + MIN(mldq + mudq + 1, Nlocal);
    BandCopy(savedJ, savedP, mukeep, mlkeep);
  }
  
  /* Scale and add I to get P = I - gamma*J */
  BandScale(-gamma, savedP);
  BandAddI(savedP);
 
  /* Do LU factorization of P in place */
  ier = BandFactor(savedP, pivots);
 
  /* Return 0 if the LU was complete; otherwise return 1 */
  if (ier > 0) return(1);
  return(0);
}


/******************************************************************
 * Function : CVBBDPrecSolve                                      *
 *----------------------------------------------------------------*
 * CVBBDPrecSolve solves a linear system P z = r, with the        *
 * band-block-diagonal preconditioner matrix P generated and      *
 * factored by CVBBDPrecSetup.                                    *
 *                                                                *
 * The parameters of CVBBDPrecSolve used here are as follows:     *
 *                                                                *
 * r      is the right-hand side vector of the linear system.     *
 *                                                                *
 * P_data is a pointer to the preconditioner data returned by     *
 *        CVBBDPrecAlloc.                                         *
 *                                                                *
 * z      is the output vector computed by CVBBDPrecSolve.        *
 *                                                                *
 * The value returned by the CVBBDPrecSolve function is always 0, *
 * indicating success.                                            *
 ******************************************************************/

static int CVBBDPrecSolve(realtype t, N_Vector y, N_Vector fy, 
                          N_Vector r, N_Vector z, 
                          realtype gamma, realtype delta,
                          int lr, void *p_data, N_Vector tmp)
{
  CVBBDPrecData pdata;
  realtype *zd;

  pdata = (CVBBDPrecData) p_data;

  /* Copy r to z, then do backsolve and return */
  N_VScale(ONE, r, z);
  
  zd = N_VGetData(z);
  BandBacksolve(savedP, pivots, zd);
  N_VSetData(zd, z);

  return(0);
}


/*************** CVBBDDQJac *****************************************

 This routine generates a banded difference quotient approximation to
 the local block of the Jacobian of g(t,y).  It assumes that a band 
 matrix of type BandMat is stored columnwise, and that elements within
 each column are contiguous.  All matrix elements are generated as
 difference quotients, by way of calls to the user routine gloc.
 By virtue of the band structure, the number of these calls is
 bandwidth + 1, where bandwidth = mldq + mudq + 1.
 But the band matrix kept has bandwidth = mlkeep + mukeep + 1.
 This routine also assumes that the local elements of a vector are
 stored contiguously.

**********************************************************************/

#define ewt    (cv_mem->cv_ewt)
#define h      (cv_mem->cv_h)
#define f_data (cv_mem->cv_f_data)

static void CVBBDDQJac(CVBBDPrecData pdata, realtype t, 
                       N_Vector y, N_Vector gy, 
                       N_Vector ytemp, N_Vector gtemp)
{
  CVodeMem cv_mem;
  realtype    gnorm, minInc, inc, inc_inv;
  long int group, i, j, width, ngroups, i1, i2;
  realtype *y_data, *ewt_data, *gy_data, *gtemp_data, *ytemp_data, *col_j;

  cv_mem = (CVodeMem) pdata->cvode_mem;

  /* Obtain pointers to the data for the y and ewt vectors */
  y_data     = N_VGetData(y);
  ewt_data   = N_VGetData(ewt);

  /* Load ytemp with y = predicted solution vector */
  N_VScale(ONE, y, ytemp);
  ytemp_data = N_VGetData(ytemp);

  /* Call cfn and gloc to get base value of g(t,y) */
  cfn (Nlocal, t, y, f_data);
  gloc(Nlocal, t, ytemp, gy, f_data);
  gy_data    = N_VGetData(gy);

  /* Set minimum increment based on uround and norm of g */
  gnorm = N_VWrmsNorm(gy, ewt);
  minInc = (gnorm != ZERO) ?
           (MIN_INC_MULT * ABS(h) * uround * Nlocal * gnorm) : ONE;

  /* Set bandwidth and number of column groups for band differencing */
  width = mldq + mudq + 1;
  ngroups = MIN(width, Nlocal);

  /* Loop over groups */  
  for (group=1; group <= ngroups; group++) {
    
    /* Increment all y_j in group */
    for(j=group-1; j < Nlocal; j+=width) {
      inc = MAX(dqrely*ABS(y_data[j]), minInc/ewt_data[j]);
      ytemp_data[j] += inc;
    }

    /* Evaluate g with incremented y */
    N_VSetData(ytemp_data, ytemp);
    gloc(Nlocal, t, ytemp, gtemp, f_data);
    gtemp_data = N_VGetData(gtemp);

    /* Restore ytemp, then form and load difference quotients */
    for (j=group-1; j < Nlocal; j+=width) {
      ytemp_data[j] = y_data[j];
      col_j = BAND_COL(savedJ,j);
      inc = MAX(dqrely*ABS(y_data[j]), minInc/ewt_data[j]);
      inc_inv = ONE/inc;
      i1 = MAX(0, j-mukeep);
      i2 = MIN(j+mlkeep, Nlocal-1);
      for (i=i1; i <= i2; i++)
        BAND_COL_ELEM(col_j,i,j) =
          inc_inv * (gtemp_data[i] - gy_data[i]);
    }
  }
}

