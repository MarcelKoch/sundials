/*
 * -----------------------------------------------------------------
 * $Revision: 1.14 $
 * $Date: 2004-06-02 23:07:46 $
 * -----------------------------------------------------------------
 * Programmer(s): Allan Taylor, Alan Hindmarsh and
 *                Radu Serban @ LLNL
 *  -----------------------------------------------------------------
 * Copyright (c) 2002, The Regents of the University of California
 * Produced at the Lawrence Livermore National Laboratory
 * All rights reserved
 * For details, see sundials/kinsol/LICENSE
 * -----------------------------------------------------------------
 * This is the header file for the KINBBDPRE module, for a
 * band-block-diagonal preconditioner, i.e. a block-diagonal
 * matrix with banded blocks, for use with KINSol, KINSpgmr,
 * and the parallel implementaion of the NVECTOR module.
 *
 * Summary:
 *
 * These routines provide a preconditioner matrix for KINSol that
 * is block-diagonal with banded blocks. The blocking corresponds
 * to the distribution of the dependent variable vector u amongst
 * the processes. Each preconditioner block is generated from
 * the Jacobian of the local part (associated with the current
 * process) of a given function g(u) approximating f(u). The blocks
 * are generated by each process via a difference quotient scheme,
 * utilizing the assumed banded structure with given
 * half-bandwidths.
 *
 * The user's calling program should have the following form:
 *
 *   #include "sundialstypes.h"
 *   #include "sundialsmath.h"
 *   #include "iterative.h"
 *   #include "nvector_parallel.h"
 *   #include "kinsol.h"
 *   #include "kinbbdpre.h"
 *   ...
 *   void *p_data;
 *   ...
 *   MPI_Init(&argc,&argv);
 *   ...
 *   nvSpec = NV_SpecInit_Parallel(...);
 *   ...
 *   kin_mem = KINCreate();
 *   KINMalloc(kin_mem,...,nvSpec);
 *   ...
 *   p_data = KINBBDPrecAlloc(kin_mem,...);
 *   ...
 *   KINBBDSpgmr(kin_mem,...,p_data);
 *   ...
 *   KINSol(kin_mem,...);
 *   ...
 *   KINBBDPrecFree(p_data);
 *   ...
 *   KINFree(kin_mem);
 *   ...
 *   NV_SpecFree_Parallel(nvSpec);
 *   ...
 *   MPI_Finalize();
 *
 * The user-supplied routines required are:
 *
 *  func    the function f(u) defining the system to be solved:
 *          f(u) = 0
 *
 *  glocal  the function defining the approximation g(u) to f(u)
 *
 *  gcomm   the function to do necessary communication for glocal
 *
 * Notes:
 *
 * 1) This header file (kinbbdpre.h) is included by the user for
 *    the definition of the KBBDData data type and for needed
 *    function prototypes.
 *
 * 2) The KINBBDPrecAlloc call includes half-bandwiths mu and ml
 *    to be used in the approximate Jacobian. They need not be
 *    the true half-bandwidths of the Jacobian of the local block
 *    of g, when smaller values may provide greater efficiency.
 *    Also, mu and ml need not be the same for all processes.
 *
 * 3) The actual name of the user's f function is passed to
 *    KINMalloc, and the names of the user's glocal and gcomm
 *    functions are passed to KINBBDPrecAlloc.
 *
 * 4) Optional outputs specific to this module are available by
 *    way of the macros listed below. These include work space sizes
 *    and the cumulative number of glocal calls.
 * -----------------------------------------------------------------
 */

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#ifndef _kbbdpre_h
#define _kbbdpre_h

#include "kinsol.h"
#include "sundialstypes.h"
#include "nvector.h"
#include "band.h"

/*
 * -----------------------------------------------------------------
 * Type : KINCommFn
 * -----------------------------------------------------------------
 * The user must supply a function of type KINCommFn which
 * performs all inter-process communication necessary to
 * evaluate the approximate system function described above.
 *
 * This function takes as input the local vector size Nlocal,
 * the solution vector u, and a pointer to the user-defined
 * data block f_data.
 *
 * The KINCommFn gcomm is expected to save communicated data in
 * space defined with the structure *f_data. Note: A KINCommFn
 * function does not have a return value.
 *
 * Each call to the KINCommFn is preceded by a call to the system
 * function func at the current iterate uu. Thus functions of the
 * type KINCommFn can omit any communications done by f (func) if
 * relevant to the evaluation of the KINLocalFn function.
 * -----------------------------------------------------------------
 */

typedef void (*KINCommFn)(long int Nlocal, N_Vector u, void *f_data);

/*
 * -----------------------------------------------------------------
 * Type : KINLocalFn
 * -----------------------------------------------------------------
 * The user must supply a function g(u) which approximates the
 * function f for the system f(u) = 0, and which is computed
 * locally (without inter-process communication). Note: The case
 * where g is mathematically identical to f is allowed.
 *
 * The implementation of this function must have type KINLocalFn
 * and take as input the local vector size Nlocal, the local
 * solution vector uu, the returned local g values vector, and a
 * pointer to the user-defined data block f_data. It is to
 * compute the local part of g(u) and store the result in the
 * vector gval. (Note: Memory for uu and gval is handled within the
 * preconditioner module.) It is expected that this routine will
 * save communicated data in work space defined by the user and
 * made available to the preconditioner function for the problem.
 *
 * Note: A KINLocalFn function does not have a return value.
 * -----------------------------------------------------------------
 */

typedef void (*KINLocalFn)(long int Nlocal, N_Vector uu,
                           N_Vector gval, void *f_data);

/*
 * -----------------------------------------------------------------
 * Function : KINBBDPrecAlloc
 * -----------------------------------------------------------------
 * KINBBDPrecAlloc allocates and initializes a KBBDData data
 * structure to be passed to KINSpgmr (and then used by
 * KINBBDPrecSetup and KINBBDPrecSolve).
 *
 * The parameters of KINBBDPrecAlloc are as follows:
 *
 * kinmem  is a pointer to the KINSol memory block.
 *
 * Nlocal  is the length of the local block of the vectors
 *         on the current process.
 *
 * mu, ml  are the upper and lower half-bandwidths to be used
 *         in the computation of the local Jacobian blocks.
 *
 * dq_rel_uu  is the relative error to be used in the difference
 *            quotient Jacobian calculation in the preconditioner.
 *            The default is sqrt(unit roundoff), obtained by
 *            passing 0.
 *
 * gloc    is the name of the user-supplied function g(u) that
 *         approximates f and whose local Jacobian blocks are
 *         to form the preconditioner.
 *
 * gcomm   is the name of the user-defined function that performs
 *         necessary inter-process communication for the
 *         execution of gloc.
 *
 * Note: KINBBDPrecAlloc returns a pointer to the storage allocated,
 * or NULL if the request for storage cannot be satisfied.
 * -----------------------------------------------------------------
 */

void *KINBBDPrecAlloc(void *kinmem, long int Nlocal, 
		      long int mu, long int ml,
		      realtype dq_rel_uu, 
		      KINLocalFn gloc, KINCommFn gcomm);

/*
 * -----------------------------------------------------------------
 * Function : KINBBDSpgmr
 * -----------------------------------------------------------------
 * KINBBDSpgmr links the KINBBDPRE preconditioner to the KINSPGMR
 * linear solver. It performs the following actions:
 *  1) Calls the KINSPGMR specification routine and attaches the
 *     KINSPGMR linear solver to the KINSOL solver;
 *  2) Sets the preconditioner data structure for KINSPGMR
 *  3) Sets the preconditioner setup routine for KINSPGMR
 *  4) Sets the preconditioner solve routine for KINSPGMR
 *
 * Its first 2 arguments are the same as for KINSpgmr (see
 * kinspgmr.h). The last argument is the pointer to the KBBDPRE
 * memory block returned by KINBBDPrecAlloc.
 *
 * Possible return values are:
 *   (from kinsol.h) SUCCESS
 *                   LIN_NO_MEM
 *                   LMEM_FAIL
 *                   LIN_NO_LMEM
 *                   LIN_ILL_INPUT
 *
 *   Additionaly, if KINBBDPrecAlloc was not previously called,
 *   KINBBDSpgmr returns BBDP_NO_PDATA (defined below).
 * -----------------------------------------------------------------
 */

int KINBBDSpgmr(void *kinmem, int maxl, void *p_data);

/*
 * -----------------------------------------------------------------
 * Function : KINBBDPrecFree
 * -----------------------------------------------------------------
 * KINBBDPrecFree frees the memory block p_data allocated by the
 * call to KINBBDPrecAlloc.
 * -----------------------------------------------------------------
 */

void KINBBDPrecFree(void *p_data);

/*
 * -----------------------------------------------------------------
 * Function : KINBBDPrecGet*
 * -----------------------------------------------------------------
 */

int KINBBDPrecGetIntWorkSpace(void *p_data, long int *leniwBBDP);
int KINBBDPrecGetRealWorkSpace(void *p_data, long int *lenrwBBDP);
int KINBBDPrecGetNumGfnEvals(void *p_data, long int *ngevalsBBDP);

/* return values for KINBBDPrecGet* functions */
/* Note: OKAY = 0 */

enum { BBDP_NO_PDATA = -11 };

/* prototypes for functions KINBBDPrecSetup and KINBBDPrecSolve */

int KINBBDPrecSetup(N_Vector uu, N_Vector uscale,
		    N_Vector fval, N_Vector fscale, 
		    void *p_data,
		    N_Vector vtemp1, N_Vector vtemp2);

int KINBBDPrecSolve(N_Vector uu, N_Vector uscale,
		    N_Vector fval, N_Vector fscale, 
		    N_Vector vv, void *p_data,
		    N_Vector vtemp);

#endif

#ifdef __cplusplus
}
#endif
