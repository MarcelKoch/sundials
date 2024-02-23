/* ----------------------------------------------------------------
 * Programmer(s): Daniel R. Reynolds @ SMU
 * ----------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2024, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 * ----------------------------------------------------------------
 * Routine to test an MRI method's embedding-based error
 * estimate.  Uses a nonlinear Kvaerno-Prothero-Robinson ODE test
 * problem with analytical solution,
 *
 *    [u]' =  [ G  e ] [(u^2-p-1)/(2u)] +  [ p'(t)/(2u) ]
 *    [v]     [ e -1 ] [(v^2-q-2)/(2v)]    [ q'(t)/(2v) ]
 *
 * where p(t) = cos(t), and q(t) = cos(omega*t*(1+exp(-(t-2)^2))).
 * This problem has analytical solution given by
 *    u(t) = sqrt(2+p(t)),  v(t) = sqrt(2+q(t)).
 * We use the parameters: e = 0.1 and G = -10 [default]
 *
 * The stiffness of the problem is essentially determined
 * by G, for |G| > 50 it is "stiff" and ideally suited to an
 * implicit method.
 *
 * Coupling between the two components is determined by e, with
 * coupling strength proportional to |e|.
 *
 * The "fast" variable, v, oscillates at a frequency "omega" times
 * faster than u.
 *
 * We partition the full time integration interval, 0 < t < 5, into
 * Npart pieces.  We then run a single time step starting at the
 * beginning of each partition, using a variety of slow step sizes,
 * H = {hmax, hmax/2, hmax/4, hmax/8, hmax/16} with
 * hmax=(t_f-t_0)/Npart.
 *
 * We place the entire ODE in the "slow" RHS partition.  All tests
 * use ARKODE's default fifth-order ERK method, with relative and
 * absolute tolerances set to 1e-10 and 1e-12, respectively.
 *
 * We select the slow integrator based on a command-line argument,
 * with the default being ARKODE_MRI_GARK_ERK33a.
 *
 * The program should be run with arguments in the following order:
 *   $ a.out method Npart G e omega
 * Not all arguments are required, but these must be omitted from
 * end-to-beginning, i.e. any one of
 *   $ a.out method Npart G e
 *   $ a.out method Npart G
 *   $ a.out method Npart
 *   $ a.out method
 *   $ a.out
 * are acceptable.  We require:
 *   * method = string corresponding to a valid embedded ARKODE_MRITableID
 *   * Npart > 0
 *   * G < 0.0
 *   * omega > 0.0
 * ----------------------------------------------------------------*/

// Header files
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <cmath>
#include <vector>
#include <sundials/sundials_core.hpp>
#include <arkode/arkode_arkstep.h>
#include <arkode/arkode_erkstep.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunnonlinsol/sunnonlinsol_newton.h>

#if defined(SUNDIALS_EXTENDED_PRECISION)
#define GSYM "Lg"
#define ESYM "Le"
#define FSYM "Lf"
#else
#define GSYM "g"
#define ESYM "e"
#define FSYM "f"
#endif

#define ZERO SUN_RCONST(0.0)
#define ONE  SUN_RCONST(1.0)
#define TWO  SUN_RCONST(2.0)

using namespace std;

// User data structure
struct UserData
{
  sunrealtype G;
  sunrealtype e;
  sunrealtype omega;
  int Npart;
};

// User-supplied functions called by the solver
static int fn(sunrealtype t, N_Vector y, N_Vector ydot, void *user_data);
static int Jn(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix J, void *user_data,
              N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

// Private utility functions
static sunrealtype p(sunrealtype t);
static sunrealtype q(sunrealtype t, UserData &udata);
static sunrealtype pdot(sunrealtype t);
static sunrealtype qdot(sunrealtype t, UserData &udata);
static sunrealtype utrue(sunrealtype t);
static sunrealtype vtrue(sunrealtype t, UserData &udata);
static int Ytrue(sunrealtype t, N_Vector y, UserData &udata);
static int computeErrorWeights(N_Vector ycur, N_Vector weight,
                               sunrealtype rtol, sunrealtype atol,
                               N_Vector vtemp);
static int check_retval(void *returnvalue, const char *funcname, int opt);

// Main Program
int main(int argc, char *argv[])
{
  // general problem parameters
  sunrealtype T0 = SUN_RCONST(0.0);   // initial time
  sunrealtype Tf = SUN_RCONST(5.0);   // final time
  sunindextype NEQ = 2;               // number of dependent vars.
  char *method;                       // MRI method name

  // general problem variables
  int retval;                    // reusable error-checking flag
  N_Vector y = NULL;             // empty vector for the computed solution
  void *arkode_mem = NULL;       // empty ARKODE memory structure
  SUNMatrix A = NULL;            // empty system matrix
  SUNLinearSolver LS = NULL;     // empty system linear solver object
  UserData udata;                // user-data structure
  udata.G = SUN_RCONST(-10.0);   // stiffness parameter
  udata.e = SUN_RCONST(0.1);     // coupling strength
  udata.omega = SUN_RCONST(5.0); // time scale ratio
  udata.Npart = 20;              // partition size

  //
  // Initialization
  //

  // Retrieve the command-line options:  method Npart G e omega
  if (argc > 1)  method = argv[1];
  if (argc > 2)  udata.Npart = (int) atoi(argv[2]);
  if (argc > 3)  udata.G = (sunrealtype) atof(argv[3]);
  if (argc > 4)  udata.e = (sunrealtype) atof(argv[4]);
  if (argc > 5)  udata.omega = (sunrealtype) atof(argv[5]);

  // Check arguments for validity
  //   G < 0.0
  //   omega > 0.0
  //   Npart > 0
  if (udata.G >= ZERO) {
    cerr << "ERROR: G must be a negative real number\n";
    return(-1);
  }
  if (udata.omega <= ZERO) {
    cerr << "ERROR: omega must be a positive real number\n";
    return(-1);
  }
  if (udata.Npart < 1) {
    cerr << "ERROR: Npart must be a positive integer\n";
    return(-1);
  }

  // Initial problem output (and set implicit solver tolerances as needed)
  cout << "\nSlow error estimation test (Nonlinear Kvaerno-Prothero-Robinson problem):\n";
  cout << "    time domain:  (" << T0 << "," << Tf << "]\n";
  cout << "    partition size = " << udata.Npart << endl;
  cout << "    G = " << udata.G << endl;
  cout << "    e = " << udata.e << endl;
  cout << "    omega = " << udata.omega << endl;
  cout << "    MRI method: " << method << endl;

  //
  // Problem Setup
  //

  // Create SUNDIALS context
  sundials::Context ctx;

  // Create and initialize serial vector for the solution
  y = N_VNew_Serial(NEQ, ctx);
  if (check_retval((void *)y, "N_VNew_Serial", 0)) return 1;
  retval = Ytrue(T0, y, udata);
  if (check_retval(&retval, "Ytrue", 1)) return 1;

  // Initialize ARKStep or ERKStep.
  if (rk_type == 0) {    // DIRK method

    arkode_mem = ARKStepCreate(NULL, fn, T0, y, ctx);
    if (check_retval((void *) arkode_mem, "ARKStepCreate", 0)) return 1;

    // Initialize/attach linear solvers (if required)
    A = SUNDenseMatrix(NEQ, NEQ, ctx);
    if (check_retval((void *)A, "SUNDenseMatrix", 0)) return 1;
    LS = SUNLinSol_Dense(y, A, ctx);
    if (check_retval((void *)LS, "SUNLinSol_Dense", 0)) return 1;
    retval = ARKStepSetLinearSolver(arkode_mem, LS, A);
    if (check_retval(&retval, "ARKStepSetLinearSolver", 1)) return(1);
    retval = ARKStepSetJacFn(arkode_mem, Jn);
    if (check_retval(&retval, "ARKStepSetJacFn", 1)) return 1;

    // Set desired solver order
    retval = ARKStepSetOrder(arkode_mem, order);
    if (check_retval(&retval, "ARKStepSetOrder", 1)) return 1;

    // Set the user data pointer
    retval = ARKStepSetUserData(arkode_mem, (void *) &udata);
    if (check_retval(&retval, "ARKStepSetUserData", 1)) return 1;

  } else {               // ERK method

    arkode_mem = ERKStepCreate(fn, T0, y, ctx);
    if (check_retval((void *) arkode_mem, "ERKStepCreate", 0)) return 1;

    // Set maximum stepsize for ERK run
    retval = ERKStepSetMaxStep(arkode_mem, ONE/abs(udata.G));
    if (check_retval(&retval, "ERKStepSetMaxStep", 1)) return(1);

    // Set desired solver order
    retval = ERKStepSetOrder(arkode_mem, order);
    if (check_retval(&retval, "ERKStepSetOrder", 1)) return 1;

    // Set the user data pointer
    retval = ERKStepSetUserData(arkode_mem, (void *) &udata);
    if (check_retval(&retval, "ERKStepSetUserData", 1)) return 1;

  }

  // Integrate ODE, based on run type
  if (adaptive) {
    retval = adaptive_run(arkode_mem, y, T0, Tf, rk_type, order, udata);
    if (check_retval(&retval, "adaptive_run", 1)) return 1;
  } else {
    retval = fixed_run(arkode_mem, y, T0, Tf, rk_type, order, udata);
    if (check_retval(&retval, "fixed_run", 1)) return 1;
  }

  // Clean up and return
  if (rk_type == 0) {                 // Free integrator memory
    ARKStepFree(&arkode_mem);
  } else {
    ERKStepFree(&arkode_mem);
  }
  if (LS != NULL) SUNLinSolFree(LS);  // free system linear solver
  if (A  != NULL) SUNMatDestroy(A);   // free system matrix
  N_VDestroy(y);                      // Free y vector
  return 0;
}

//------------------------------
// Functions called by the solver
//------------------------------

static int fn(sunrealtype t, N_Vector y, N_Vector ydot, void *user_data)
{
  UserData *udata = (UserData *) user_data;
  const sunrealtype u = NV_Ith_S(y,0);
  const sunrealtype v = NV_Ith_S(y,1);
  sunrealtype tmp1, tmp2;

  // fill in the RHS function:
  //   [G  e]*[(-2+u^2-p(t))/(2*u)] + [pdot(t)/(2u)]
  //   [e -1] [(-2+v^2-s(t))/(2*v)]   [qdot(t)/(2v)]
  tmp1 = (-TWO+u*u-p(t))/(TWO*u);
  tmp2 = (-TWO+v*v-q(t,*udata))/(TWO*v);
  NV_Ith_S(ydot,0) = udata->G*tmp1 + udata->e*tmp2 + pdot(t)/(TWO*u);
  NV_Ith_S(ydot,1) = udata->e*tmp1 - tmp2 + qdot(t,*udata)/(TWO*v);

  // Return with success
  return 0;
}

static int Jn(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix J,
              void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  UserData *udata = (UserData *) user_data;
  const sunrealtype u = NV_Ith_S(y,0);
  const sunrealtype v = NV_Ith_S(y,1);
  sunrealtype t11, t22;

  // fill in the Jacobian:
  //   [G  e]*[1-(u^2-p(t)-2)/(2*u^2),  0] + [-r'(t)/(2*u^2),  0]
  //   [e -1] [0,  1-(v^2-s(t)-2)/(2*v^2)]   [0,  -s'(t)/(2*v^2)]
  t11 = ONE-(u*u-p(t)-TWO)/(TWO*u*u);
  t22 = ONE-(v*v-q(t,*udata)-TWO)/(TWO*v*v);
  SM_ELEMENT_D(J,0,0) = udata->G*t11 - pdot(t)/(TWO*u*u);
  SM_ELEMENT_D(J,0,1) = udata->e*t22;
  SM_ELEMENT_D(J,1,0) = udata->e*t11;
  SM_ELEMENT_D(J,1,1) = -t22 - qdot(t,*udata)/(TWO*v*v);

  // Return with success
  return 0;
}



//------------------------------
// Private helper functions
//------------------------------

static int adaptive_run(void *arkode_mem, N_Vector y, sunrealtype T0,
                        sunrealtype Tf, int rk_type, int order, UserData &udata)
{
  // Reused variables
  int retval;
  sunrealtype t;
  sunrealtype hpart = (Tf-T0)/udata.Npart;
  sunrealtype abstol = SUN_RCONST(1.e-12);
  vector<sunrealtype> rtols = {SUN_RCONST(1.e-2), SUN_RCONST(1.e-4), SUN_RCONST(1.e-6)};
  vector<int> accum_types = {0, 1};
  vector<sunrealtype> dsm(udata.Npart);
  vector<sunrealtype> dsm_est(udata.Npart);
  vector<long int> Nsteps(udata.Npart);

  // Loop over tolerances
  cout << "\nAdaptive-step runs:\n";
  for (size_t irtol=0; irtol<rtols.size(); irtol++) {

    // Loop over accumulation types
    for (size_t iaccum=0; iaccum<accum_types.size(); iaccum++) {

      // Loop over partition
      for (size_t ipart=0; ipart<udata.Npart; ipart++) {

        // Reset integrator for this run, and evolve over partition interval
        t = T0 + ipart*hpart;
        retval = Ytrue(t, y, udata);
        if (check_retval(&retval, "Ytrue", 1)) return 1;
        if (rk_type == 0) {  // DIRK
          retval = ARKStepReInit(arkode_mem, NULL, fn, t, y);
          if (check_retval(&retval, "ARKStepReInit", 1)) return 1;
          retval = ARKStepSetAccumulatedErrorType(arkode_mem, accum_types[iaccum]);
          if (check_retval(&retval, "ARKStepSetAccumulatedErrorType", 1)) return 1;
          retval = ARKStepResetAccumulatedError(arkode_mem);
          if (check_retval(&retval, "ARKStepResetAccumulatedError", 1)) return 1;
          retval = ARKStepSStolerances(arkode_mem, rtols[irtol], abstol);
          if (check_retval(&retval, "ARKStepSStolerances", 1)) return 1;
          retval = ARKStepSetStopTime(arkode_mem, t+hpart);
          if (check_retval(&retval, "ARKStepSetStopTime", 1)) return 1;
          retval = ARKStepSetMaxNumSteps(arkode_mem, 1000000);
          if (check_retval(&retval, "ARKStepSetMaxNumSteps", 1)) return(1);
          retval = ARKStepEvolve(arkode_mem, t+hpart, y, &t, ARK_NORMAL);
          if (check_retval(&retval, "ARKStepEvolve", 1)) break;
          retval = ARKStepGetAccumulatedError(arkode_mem, &(dsm_est[ipart]));
          if (check_retval(&retval, "ARKStepGetAccumulatedError", 1)) break;
          retval = ARKStepGetNumSteps(arkode_mem, &(Nsteps[ipart]));
          if (check_retval(&retval, "ARKStepGetNumSteps", 1)) break;
        } else {            // ERK
          retval = ERKStepReInit(arkode_mem, fn, t, y);
          if (check_retval(&retval, "ERKStepReInit", 1)) return 1;
          retval = ERKStepSetAccumulatedErrorType(arkode_mem, accum_types[iaccum]);
          if (check_retval(&retval, "ERKStepSetAccumulatedErrorType", 1)) return 1;
          retval = ERKStepResetAccumulatedError(arkode_mem);
          if (check_retval(&retval, "ERKStepResetAccumulatedError", 1)) return 1;
          retval = ERKStepSStolerances(arkode_mem, rtols[irtol], abstol);
          if (check_retval(&retval, "ERKStepSStolerances", 1)) return 1;
          retval = ERKStepSetStopTime(arkode_mem, t+hpart);
          if (check_retval(&retval, "ERKStepSetStopTime", 1)) return 1;
          retval = ERKStepSetMaxNumSteps(arkode_mem, 1000000);
          if (check_retval(&retval, "ERKStepSetMaxNumSteps", 1)) return(1);
          retval = ERKStepEvolve(arkode_mem, t+hpart, y, &t, ARK_NORMAL);
          if (check_retval(&retval, "ERKStepEvolve", 1)) break;
          retval = ERKStepGetAccumulatedError(arkode_mem, &(dsm_est[ipart]));
          if (check_retval(&retval, "ERKStepGetAccumulatedError", 1)) break;
          retval = ERKStepGetNumSteps(arkode_mem, &(Nsteps[ipart]));
          if (check_retval(&retval, "ERKStepGetNumSteps", 1)) break;
        }

        // Compute/print solution error
        sunrealtype udsm = abs(NV_Ith_S(y,0)-utrue(t))/(abstol + rtols[irtol]*abs(utrue(t)));
        sunrealtype vdsm = abs(NV_Ith_S(y,1)-vtrue(t,udata))/(abstol + rtols[irtol]*abs(vtrue(t,udata)));
        dsm[ipart] = rtols[irtol]*sqrt(0.5*(udsm*udsm + vdsm*vdsm));
        cout << "  rtol " << rtols[irtol]
             << "  rk_type " << rk_type
             << "  order " << order
             << "  acc " << accum_types[iaccum]
             << "  t " << t
             << "  dsm " << dsm[ipart]
             << "  dsm_est " << dsm_est[ipart]
             << "  nsteps " << Nsteps[ipart]
             << endl;
      }
    }
  }

  return(0);
}

static int fixed_run(void *arkode_mem, N_Vector y, sunrealtype T0, sunrealtype Tf,
                     int rk_type, int order, UserData &udata)
{
  // local variables
  int retval;
  sunrealtype hpart = (Tf-T0)/udata.Npart;
  long int nsteps2;
  sunrealtype t, t2;
  sunrealtype reltol = SUN_RCONST(1.e-9);
  sunrealtype abstol = SUN_RCONST(1.e-12);
  N_Vector y2 = N_VClone(y);
  N_Vector ewt = N_VClone(y);
  N_Vector vtemp = N_VClone(y);

  // Set array of fixed step sizes to use, storage for corresponding errors/orders
  sunrealtype hmax = (Tf - T0)/1000;
  if (rk_type == 1) hmax = min(hmax, ONE/abs(udata.G));
  vector<sunrealtype> hvals = {hmax, hmax/4, hmax/16, hmax/64};
  vector<int> accum_types = {0, 1};
  vector<sunrealtype> dsm(udata.Npart);
  vector<sunrealtype> dsm_est(udata.Npart);
  vector<long int> Nsteps(udata.Npart);

  // Loop over step sizes
  cout << "\nFixed-step runs:\n";
  for (size_t ih=0; ih<hvals.size(); ih++) {

    // Loop over built-in accumulation types
    for (size_t iaccum=0; iaccum<accum_types.size(); iaccum++) {

      // Loop over partition
      for (size_t ipart=0; ipart<udata.Npart; ipart++) {

        // Reset integrator for this run, and evolve over partition interval
        t = T0 + ipart*hpart;
        retval = Ytrue(t, y, udata);
        if (check_retval(&retval, "Ytrue", 1)) return 1;
        if (rk_type == 0) {  // DIRK
          retval = ARKStepReInit(arkode_mem, NULL, fn, t, y);
          if (check_retval(&retval, "ARKStepReInit", 1)) return 1;
          retval = ARKStepSetAccumulatedErrorType(arkode_mem, accum_types[iaccum]);
          if (check_retval(&retval, "ARKStepSetAccumulatedErrorType", 1)) return 1;
          retval = ARKStepResetAccumulatedError(arkode_mem);
          if (check_retval(&retval, "ARKStepResetAccumulatedError", 1)) return 1;
          retval = ARKStepSetFixedStep(arkode_mem, hvals[ih]);
          if (check_retval(&retval, "ARKStepSetFixedStep", 1)) return 1;
          retval = ARKStepSetMaxNumSteps(arkode_mem, 1000000);
          if (check_retval(&retval, "ARKStepSetMaxNumSteps", 1)) return(1);
          retval = ARKStepSetStopTime(arkode_mem, t+hpart);
          if (check_retval(&retval, "ARKStepSetStopTime", 1)) return 1;
          retval = ARKStepSStolerances(arkode_mem, reltol, abstol);
          if (check_retval(&retval, "ARKStepSStolerances", 1)) return 1;
          retval = ARKStepSetJacEvalFrequency(arkode_mem, 1);
          if (check_retval(&retval, "ARKStepSetJacEvalFrequency", 1)) return 1;
          retval = ARKStepSetLSetupFrequency(arkode_mem, 1);
          if (check_retval(&retval, "ARKStepSetLSetupFrequency", 1)) return 1;
          retval = ARKStepSetMaxNonlinIters(arkode_mem, 20);
          if (check_retval(&retval, "ARKStepSetMaxNonlinIters", 1)) return 1;
          retval = ARKStepEvolve(arkode_mem, t+hpart, y, &t, ARK_NORMAL);
          if (check_retval(&retval, "ARKStepEvolve", 1)) break;
          retval = ARKStepGetAccumulatedError(arkode_mem, &(dsm_est[ipart]));
          if (check_retval(&retval, "ARKStepGetAccumulatedError", 1)) break;
          retval = ARKStepGetNumSteps(arkode_mem, &(Nsteps[ipart]));
          if (check_retval(&retval, "ARKStepGetNumSteps", 1)) break;
        } else {            // ERK
          retval = ERKStepReInit(arkode_mem, fn, t, y);
          if (check_retval(&retval, "ERKStepReInit", 1)) return 1;
          retval = ERKStepSetAccumulatedErrorType(arkode_mem, accum_types[iaccum]);
          if (check_retval(&retval, "ERKStepSetAccumulatedErrorType", 1)) return 1;
          retval = ERKStepResetAccumulatedError(arkode_mem);
          if (check_retval(&retval, "ERKStepResetAccumulatedError", 1)) return 1;
          retval = ERKStepSetFixedStep(arkode_mem, hvals[ih]);
          if (check_retval(&retval, "ERKStepSetFixedStep", 1)) return 1;
          retval = ERKStepSetMaxNumSteps(arkode_mem, 1000000);
          if (check_retval(&retval, "ERKStepSetMaxNumSteps", 1)) return(1);
          retval = ERKStepSStolerances(arkode_mem, reltol, abstol);
          if (check_retval(&retval, "ERKStepSStolerances", 1)) return 1;
          retval = ERKStepSetStopTime(arkode_mem, t+hpart);
          if (check_retval(&retval, "ERKStepSetStopTime", 1)) return 1;
          retval = ERKStepEvolve(arkode_mem, t+hpart, y, &t, ARK_NORMAL);
          if (check_retval(&retval, "ERKStepEvolve", 1)) break;
          retval = ERKStepGetAccumulatedError(arkode_mem, &(dsm_est[ipart]));
          if (check_retval(&retval, "ERKStepGetAccumulatedError", 1)) break;
          retval = ERKStepGetNumSteps(arkode_mem, &(Nsteps[ipart]));
          if (check_retval(&retval, "ERKStepGetNumSteps", 1)) break;
        }

        // Compute/print solution error
        sunrealtype udsm = abs(NV_Ith_S(y,0)-utrue(t))/(abstol + reltol*abs(utrue(t)));
        sunrealtype vdsm = abs(NV_Ith_S(y,1)-vtrue(t,udata))/(abstol + reltol*abs(vtrue(t,udata)));
        dsm[ipart] = reltol*sqrt(0.5*(udsm*udsm + vdsm*vdsm));
        cout << "  h " << hvals[ih]
             << "  rk_type " << rk_type
             << "  order " << order
             << "  acc " << accum_types[iaccum]
             << "  t " << t
             << "  dsm " << dsm[ipart]
             << "  dsm_est " << dsm_est[ipart]
             << "  nsteps " << Nsteps[ipart]
             << endl;
      }
    }

    // Test double-step error estimator

    // Loop over partition
    for (size_t ipart=0; ipart<udata.Npart; ipart++) {

      // Reset integrator for this run, and evolve over partition interval
      t = t2 = T0 + ipart*hpart;
      retval = Ytrue(t, y, udata);
      if (check_retval(&retval, "Ytrue", 1)) return 1;
      retval = Ytrue(t2, y2, udata);
      if (check_retval(&retval, "Ytrue", 1)) return 1;
      if (rk_type == 0) {  // DIRK
        retval = ARKStepReInit(arkode_mem, NULL, fn, t, y);
        if (check_retval(&retval, "ARKStepReInit", 1)) return 1;
        retval = ARKStepSetAccumulatedErrorType(arkode_mem, -1);
        if (check_retval(&retval, "ARKStepSetAccumulatedErrorType", 1)) return 1;
        retval = ARKStepSetFixedStep(arkode_mem, hvals[ih]);
        if (check_retval(&retval, "ARKStepSetFixedStep", 1)) return 1;
        retval = ARKStepSetMaxNumSteps(arkode_mem, 1000000);
        if (check_retval(&retval, "ARKStepSetMaxNumSteps", 1)) return(1);
        retval = ARKStepSetStopTime(arkode_mem, t+hpart);
        if (check_retval(&retval, "ARKStepSetStopTime", 1)) return 1;
        retval = ARKStepSStolerances(arkode_mem, reltol, abstol);
        if (check_retval(&retval, "ARKStepSStolerances", 1)) return 1;
        retval = ARKStepSetJacEvalFrequency(arkode_mem, 1);
        if (check_retval(&retval, "ARKStepSetJacEvalFrequency", 1)) return 1;
        retval = ARKStepSetLSetupFrequency(arkode_mem, 1);
        if (check_retval(&retval, "ARKStepSetLSetupFrequency", 1)) return 1;
        retval = ARKStepSetMaxNonlinIters(arkode_mem, 20);
        if (check_retval(&retval, "ARKStepSetMaxNonlinIters", 1)) return 1;
        retval = ARKStepEvolve(arkode_mem, t+hpart, y, &t, ARK_NORMAL);
        if (check_retval(&retval, "ARKStepEvolve", 1)) break;
        retval = ARKStepGetNumSteps(arkode_mem, &(Nsteps[ipart]));
        if (check_retval(&retval, "ARKStepGetNumSteps", 1)) break;

        retval = ARKStepReInit(arkode_mem, NULL, fn, t2, y2);
        if (check_retval(&retval, "ARKStepReInit", 1)) return 1;
        retval = ARKStepSetFixedStep(arkode_mem, 2.0*hvals[ih]);
        if (check_retval(&retval, "ARKStepSetFixedStep", 1)) return 1;
        retval = ARKStepSetStopTime(arkode_mem, t2+hpart);
        if (check_retval(&retval, "ARKStepSetStopTime", 1)) return 1;
        retval = ARKStepEvolve(arkode_mem, t2+hpart, y2, &t2, ARK_NORMAL);
        if (check_retval(&retval, "ARKStepEvolve", 1)) break;
        retval = ARKStepGetNumSteps(arkode_mem, &nsteps2);
        if (check_retval(&retval, "ARKStepGetNumSteps", 1)) break;
      } else {            // ERK
        retval = ERKStepReInit(arkode_mem, fn, t, y);
        if (check_retval(&retval, "ERKStepReInit", 1)) return 1;
        retval = ERKStepSetAccumulatedErrorType(arkode_mem, -1);
        if (check_retval(&retval, "ERKStepSetAccumulatedErrorType", 1)) return 1;
        retval = ERKStepSetFixedStep(arkode_mem, hvals[ih]);
        if (check_retval(&retval, "ERKStepSetFixedStep", 1)) return 1;
        retval = ERKStepSetMaxNumSteps(arkode_mem, 1000000);
        if (check_retval(&retval, "ERKStepSetMaxNumSteps", 1)) return(1);
        retval = ERKStepSetStopTime(arkode_mem, t+hpart);
        if (check_retval(&retval, "ERKStepSetStopTime", 1)) return 1;
        retval = ERKStepSStolerances(arkode_mem, reltol, abstol);
        if (check_retval(&retval, "ERKStepSStolerances", 1)) return 1;
        retval = ERKStepEvolve(arkode_mem, t+hpart, y, &t, ARK_NORMAL);
        if (check_retval(&retval, "ERKStepEvolve", 1)) break;
        retval = ERKStepGetNumSteps(arkode_mem, &(Nsteps[ipart]));
        if (check_retval(&retval, "ERKStepGetNumSteps", 1)) break;

        retval = ERKStepReInit(arkode_mem, fn, t2, y2);
        if (check_retval(&retval, "ERKStepReInit", 1)) return 1;
        retval = ERKStepSetFixedStep(arkode_mem, 2.0*hvals[ih]);
        if (check_retval(&retval, "ERKStepSetFixedStep", 1)) return 1;
        retval = ERKStepSetStopTime(arkode_mem, t2+hpart);
        if (check_retval(&retval, "ERKStepSetStopTime", 1)) return 1;
        retval = ERKStepEvolve(arkode_mem, t2+hpart, y2, &t2, ARK_NORMAL);
        if (check_retval(&retval, "ERKStepEvolve", 1)) break;
        retval = ERKStepGetNumSteps(arkode_mem, &nsteps2);
        if (check_retval(&retval, "ERKStepGetNumSteps", 1)) break;
      }
      retval = computeErrorWeights(y2, ewt, reltol, abstol, vtemp);
      if (check_retval(&retval, "computeErrorWeights", 1)) break;
      N_VLinearSum(ONE, y2, -ONE, y, y2);
      dsm_est[ipart] = reltol*N_VWrmsNorm(y2, ewt);
      Nsteps[ipart] += nsteps2;
      sunrealtype udsm = abs(NV_Ith_S(y,0)-utrue(t))/(abstol + reltol*abs(utrue(t)));
      sunrealtype vdsm = abs(NV_Ith_S(y,1)-vtrue(t,udata))/(abstol + reltol*abs(vtrue(t,udata)));
      dsm[ipart] = reltol*sqrt(0.5*(udsm*udsm + vdsm*vdsm));
      cout << "  h " << hvals[ih]
           << "  rk_type " << rk_type
           << "  order " << order
           << "  acc " << 2
           << "  t " << t
           << "  dsm " << dsm[ipart]
           << "  dsm_est " << dsm_est[ipart]
           << "  nsteps " << Nsteps[ipart]
           << endl;
    }
  }

  N_VDestroy(y2);
  N_VDestroy(ewt);
  return(0);
}

static sunrealtype p(sunrealtype t)
{
  return( cos(t) );
}
static sunrealtype q(sunrealtype t, UserData &udata)
{
  return( cos(udata.omega*t*(ONE+exp(-(t-2)*(t-2)))) );
}
static sunrealtype pdot(sunrealtype t)
{
  return( -sin(t) );
}
static sunrealtype qdot(sunrealtype t, UserData &udata)
{
  return( -sin(udata.omega*t*(ONE+exp(-(t-2)*(t-2)))) *
          udata.omega*(ONE+exp(-(t-2)*(t-2)) - t*2*(t-2)*(exp(-(t-2)*(t-2)))) );
}
static sunrealtype utrue(sunrealtype t)
{
  return( SUNRsqrt(TWO+p(t)) );
}
static sunrealtype vtrue(sunrealtype t, UserData &udata)
{
  return( SUNRsqrt(TWO+q(t, udata)) );
}
static int Ytrue(sunrealtype t, N_Vector y, UserData &udata)
{
  NV_Ith_S(y,0) = utrue(t);
  NV_Ith_S(y,1) = vtrue(t, udata);
  return(0);
}
static int computeErrorWeights(N_Vector ycur, N_Vector weight,
                               sunrealtype rtol, sunrealtype atol,
                               N_Vector vtemp)
{
  N_VAbs(ycur, vtemp);
  N_VScale(rtol, vtemp, vtemp);
  N_VAddConst(vtemp, atol, vtemp);
  N_VInv(vtemp, weight);
  return(0);
}

/* Check function return value...
    opt == 0 means SUNDIALS function allocates memory so check if
             returned NULL pointer
    opt == 1 means SUNDIALS function returns a retval so check if
             retval < 0
    opt == 2 means function allocates memory so check if returned
             NULL pointer
*/
static int check_retval(void *returnvalue, const char *funcname, int opt)
{
  int *retval;

  // Check if SUNDIALS function returned NULL pointer - no memory allocated
  if (opt == 0 && returnvalue == NULL) {
    fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return 1; }

  // Check if retval < 0
  else if (opt == 1) {
    retval = (int *) returnvalue;
    if (*retval < 0) {
      fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with retval = %d\n\n",
              funcname, *retval);
      return 1; }}

  // Check if function returned NULL pointer - no memory allocated
  else if (opt == 2 && returnvalue == NULL) {
    fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return 1; }

  return 0;
}


//---- end of file ----
