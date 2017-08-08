/*---------------------------------------------------------------
 * Programmer(s): Daniel R. Reynolds @ SMU
 *---------------------------------------------------------------
 * LLNS/SMU Copyright Start
 * Copyright (c) 2015, Southern Methodist University and 
 * Lawrence Livermore National Security
 *
 * This work was performed under the auspices of the U.S. Department 
 * of Energy by Southern Methodist University and Lawrence Livermore 
 * National Laboratory under Contract DE-AC52-07NA27344.
 * Produced at Southern Methodist University and the Lawrence 
 * Livermore National Laboratory.
 *
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS/SMU Copyright End
 *---------------------------------------------------------------
 * Fortran/C interface routines for ARKODE/ARKDLS, for the case 
 * of a user-supplied Jacobian approximation routine.                
 *--------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "farkode.h"
#include "arkode_impl.h"
#include <arkode/arkode_direct.h>
#include <sunmatrix/sunmatrix_diagonal.h>

/*=============================================================*/

/* Prototype of the Fortran routine */

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

  extern void FARK_DIAGJAC(realtype *T, realtype *Y, 
                           realtype *FY, realtype *DJAC, 
                           realtype *H, long int *IPAR, 
                           realtype *RPAR, realtype *V1, 
                           realtype *V2, realtype *V3, int *ier);

#ifdef __cplusplus
}
#endif

/*=============================================================*/

/* Fortran interface to C routine ARKDlsSetJacFn; see 
   farkode.h for additional information */
void FARK_DIAGSETJAC(int *flag, int *ier)
{
  if (*flag == 0) {
    *ier = ARKDlsSetJacFn(ARK_arkodemem, NULL);
  } else {
    *ier = ARKDlsSetJacFn(ARK_arkodemem, FARKDiagJac);
  }
  return;
}

/*=============================================================*/

/* C interface to user-supplied Fortran routine FARKDIAGJAC; see 
   farkode.h for additional information  */
int FARKDiagJac(realtype t, N_Vector y, N_Vector fy, SUNMatrix J, 
                void *user_data, N_Vector vtemp1, 
                N_Vector vtemp2, N_Vector vtemp3)
{
  int ier;
  realtype *ydata, *fydata, *jacdata, *v1data, *v2data, *v3data;
  realtype h;
  FARKUserData ARK_userdata;

  ARKodeGetLastStep(ARK_arkodemem, &h);
  ydata   = N_VGetArrayPointer(y);
  fydata  = N_VGetArrayPointer(fy);
  v1data  = N_VGetArrayPointer(vtemp1);
  v2data  = N_VGetArrayPointer(vtemp2);
  v3data  = N_VGetArrayPointer(vtemp3);
  jacdata = N_VGetArrayPointer(SUNDiagonalMatrix_Diag(J));
  ARK_userdata = (FARKUserData) user_data;

  FARK_DIAGJAC(&t, ydata, fydata, jacdata, &h, 
               ARK_userdata->ipar, ARK_userdata->rpar, 
               v1data, v2data, v3data, &ier); 
  return(ier);
}

/*===============================================================
   EOF
===============================================================*/
