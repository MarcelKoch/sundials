/*
 * -----------------------------------------------------------------
 * Programmer(s): Radu Serban and Aaron Collier @ LLNL
 * -----------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2019, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 * -----------------------------------------------------------------
 * This file (companion of nvector_spcparallel.c) contains the
 * definitions needed for the initialization of the spcparallel
 * vector operations in Fortran.
 * -----------------------------------------------------------------
 */

#ifndef _FNVECTOR_SPCPARALLEL_H
#define _FNVECTOR_SPCPARALLEL_H

#include <nvector/nvector_spcparallel.h>
#include <sundials/sundials_fnvector.h>

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#if defined(F77_FUNC)

#define FNV_INITSPCP    F77_FUNC(fnvinitspcp, FNVINITSPCP)
#define FNV_INITSPCP_Q  F77_FUNC_(fnvinitspcp_q, FNVINITSPCP_Q)
#define FNV_INITSPCP_S  F77_FUNC_(fnvinitspcp_s, FNVINITSPCP_S)
#define FNV_INITSPCP_B  F77_FUNC_(fnvinitspcp_b, FNVINITSPCP_B)
#define FNV_INITSPCP_QB F77_FUNC_(fnvinitspcp_qb, FNVINITSPCP_QB)

#else

#define FNV_INITSPCP    fnvinitspcp_
#define FNV_INITSPCP_Q  fnvinitspcp_q_
#define FNV_INITSPCP_S  fnvinitspcp_s_
#define FNV_INITSPCP_B  fnvinitspcp_b_
#define FNV_INITSPCP_QB fnvinitspcp_qb_

#endif

/* Declarations of global variables */

extern N_Vector F2C_CVODE_vec;
extern N_Vector F2C_CVODE_vecQ;
extern N_Vector *F2C_CVODE_vecS;
extern N_Vector F2C_CVODE_vecB;
extern N_Vector F2C_CVODE_vecQB;

extern N_Vector F2C_IDA_vec;
extern N_Vector F2C_IDA_vecQ;
extern N_Vector *F2C_IDA_vecS;
extern N_Vector F2C_IDA_vecB;
extern N_Vector F2C_IDA_vecQB;

extern N_Vector F2C_KINSOL_vec;

extern N_Vector F2C_ARKODE_vec;

/*
 * Prototypes of exported functions
 *
 * FNV_INITSPCP    - initializes parallel vector operations for main problem
 * FNV_INITSPCP_Q  - initializes parallel vector operations for quadratures
 * FNV_INITSPCP_S  - initializes parallel vector operations for sensitivities
 * FNV_INITSPCP_B  - initializes parallel vector operations for adjoint problem
 * FNV_INITSPCP_QB - initializes parallel vector operations for adjoint quadratures
 *
 */

#ifndef SUNDIALS_MPI_COMM_F2C
#define MPI_Fint int
#endif

void FNV_INITSPCP(MPI_Fint *comm, int *code,
		  int *Ngrp, int *Nspc,
		  long int *Nx, long int *Ny, long int *Nz,
		  long int *NGx, long int *NGy, long int *NGz,
		  int *ier);
void FNV_INITSPCP_Q(MPI_Fint *comm, int *code,
		    int *NgrpQ, int *NspcQ,
		    long int *NxQ, long int *NyQ, long int *NzQ,
		    long int *NGxQ, long int *NGyQ, long int *NGzQ,
		    int *ier);
void FNV_INITSPCP_B(MPI_Fint *comm, int *code,
		    int *NgrpB, int *NspcB,
		    long int *NxB, long int *NyB, long int *NzB,
		    long int *NGxB, long int *NGyB, long int *NGzB,
		    int *ier);
void FNV_INITSPCP_QB(MPI_Fint *comm, int *code,
		     int *NgrpQB, int *NspcQB,
		     long int *NxQB, long int *NyQB, long int *NzQB,
		     long int *NGxQB, long int *NGyQB, long int *NGzQB,
		     int *ier);
void FNV_INITSPCP_S(int *code, int *Ns, int *ier);

#ifdef __cplusplus
}
#endif

#endif
