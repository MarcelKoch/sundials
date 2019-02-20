! This file was automatically generated by SWIG (http://www.swig.org).
! Version 4.0.0
!
! Do not make changes to this file unless you know what you are doing--modify
! the SWIG interface file instead.

! ---------------------------------------------------------------
! Programmer(s): Auto-generated by swig.
! ---------------------------------------------------------------
! SUNDIALS Copyright Start
! Copyright (c) 2002-2019, Lawrence Livermore National Security
! and Southern Methodist University.
! All rights reserved.
!
! See the top-level LICENSE and NOTICE files for details.
!
! SPDX-License-Identifier: BSD-3-Clause
! SUNDIALS Copyright End
! ---------------------------------------------------------------

module fida_mod
 use, intrinsic :: ISO_C_BINDING
 use fsundials_types
 implicit none
 private

 ! PUBLIC METHODS AND TYPES
 public :: FIDACreate
 public :: FIDAInit
 public :: FIDAReInit
 public :: FIDASStolerances
 public :: FIDASVtolerances
 public :: FIDAWFtolerances
 public :: FIDACalcIC
 public :: FIDASetNonlinConvCoefIC
 public :: FIDASetMaxNumStepsIC
 public :: FIDASetMaxNumJacsIC
 public :: FIDASetMaxNumItersIC
 public :: FIDASetLineSearchOffIC
 public :: FIDASetStepToleranceIC
 public :: FIDASetMaxBacksIC
 public :: FIDASetErrHandlerFn
 public :: FIDASetErrFile
 public :: FIDASetUserData
 public :: FIDASetMaxOrd
 public :: FIDASetMaxNumSteps
 public :: FIDASetInitStep
 public :: FIDASetMaxStep
 public :: FIDASetStopTime
 public :: FIDASetNonlinConvCoef
 public :: FIDASetMaxErrTestFails
 public :: FIDASetMaxNonlinIters
 public :: FIDASetMaxConvFails
 public :: FIDASetSuppressAlg
 public :: FIDASetId
 public :: FIDASetConstraints
 public :: FIDASetNonlinearSolver
 public :: FIDARootInit
 public :: FIDASetRootDirection
 public :: FIDASetNoInactiveRootWarn
 public :: FIDASolve
 public :: FIDAGetDky
 public :: FIDAGetWorkSpace
 public :: FIDAGetNumSteps
 public :: FIDAGetNumResEvals
 public :: FIDAGetNumLinSolvSetups
 public :: FIDAGetNumErrTestFails
 public :: FIDAGetNumBacktrackOps
 public :: FIDAGetConsistentIC
 public :: FIDAGetLastOrder
 public :: FIDAGetCurrentOrder
 public :: FIDAGetActualInitStep
 public :: FIDAGetLastStep
 public :: FIDAGetCurrentStep
 public :: FIDAGetCurrentTime
 public :: FIDAGetTolScaleFactor
 public :: FIDAGetErrWeights
 public :: FIDAGetEstLocalErrors
 public :: FIDAGetNumGEvals
 public :: FIDAGetRootInfo
 public :: FIDAGetIntegratorStats
 public :: FIDAGetNumNonlinSolvIters
 public :: FIDAGetNumNonlinSolvConvFails
 public :: FIDAGetNonlinSolvStats
 public :: FIDAGetReturnFlagName
 public :: FIDAFree
 public :: FIDABBDPrecInit
 public :: FIDABBDPrecReInit
 public :: FIDABBDPrecGetWorkSpace
 public :: FIDABBDPrecGetNumGfnEvals
 public :: FIDASetLinearSolver
 public :: FIDASetJacFn
 public :: FIDASetPreconditioner
 public :: FIDASetJacTimes
 public :: FIDASetEpsLin
 public :: FIDASetIncrementFactor
 public :: FIDAGetLinWorkSpace
 public :: FIDAGetNumJacEvals
 public :: FIDAGetNumPrecEvals
 public :: FIDAGetNumPrecSolves
 public :: FIDAGetNumLinIters
 public :: FIDAGetNumLinConvFails
 public :: FIDAGetNumJTSetupEvals
 public :: FIDAGetNumJtimesEvals
 public :: FIDAGetNumLinResEvals
 public :: FIDAGetLastLinFlag
 public :: FIDAGetLinReturnFlagName

 ! PARAMETERS
 integer(C_INT), parameter, public :: IDA_NORMAL = 1_C_INT
 integer(C_INT), parameter, public :: IDA_ONE_STEP = 2_C_INT
 integer(C_INT), parameter, public :: IDA_YA_YDP_INIT = 1_C_INT
 integer(C_INT), parameter, public :: IDA_Y_INIT = 2_C_INT
 integer(C_INT), parameter, public :: IDA_SUCCESS = 0_C_INT
 integer(C_INT), parameter, public :: IDA_TSTOP_RETURN = 1_C_INT
 integer(C_INT), parameter, public :: IDA_ROOT_RETURN = 2_C_INT
 integer(C_INT), parameter, public :: IDA_WARNING = 99_C_INT
 integer(C_INT), parameter, public :: IDA_TOO_MUCH_WORK = -1_C_INT
 integer(C_INT), parameter, public :: IDA_TOO_MUCH_ACC = -2_C_INT
 integer(C_INT), parameter, public :: IDA_ERR_FAIL = -3_C_INT
 integer(C_INT), parameter, public :: IDA_CONV_FAIL = -4_C_INT
 integer(C_INT), parameter, public :: IDA_LINIT_FAIL = -5_C_INT
 integer(C_INT), parameter, public :: IDA_LSETUP_FAIL = -6_C_INT
 integer(C_INT), parameter, public :: IDA_LSOLVE_FAIL = -7_C_INT
 integer(C_INT), parameter, public :: IDA_RES_FAIL = -8_C_INT
 integer(C_INT), parameter, public :: IDA_REP_RES_ERR = -9_C_INT
 integer(C_INT), parameter, public :: IDA_RTFUNC_FAIL = -10_C_INT
 integer(C_INT), parameter, public :: IDA_CONSTR_FAIL = -11_C_INT
 integer(C_INT), parameter, public :: IDA_FIRST_RES_FAIL = -12_C_INT
 integer(C_INT), parameter, public :: IDA_LINESEARCH_FAIL = -13_C_INT
 integer(C_INT), parameter, public :: IDA_NO_RECOVERY = -14_C_INT
 integer(C_INT), parameter, public :: IDA_NLS_INIT_FAIL = -15_C_INT
 integer(C_INT), parameter, public :: IDA_NLS_SETUP_FAIL = -16_C_INT
 integer(C_INT), parameter, public :: IDA_MEM_NULL = -20_C_INT
 integer(C_INT), parameter, public :: IDA_MEM_FAIL = -21_C_INT
 integer(C_INT), parameter, public :: IDA_ILL_INPUT = -22_C_INT
 integer(C_INT), parameter, public :: IDA_NO_MALLOC = -23_C_INT
 integer(C_INT), parameter, public :: IDA_BAD_EWT = -24_C_INT
 integer(C_INT), parameter, public :: IDA_BAD_K = -25_C_INT
 integer(C_INT), parameter, public :: IDA_BAD_T = -26_C_INT
 integer(C_INT), parameter, public :: IDA_BAD_DKY = -27_C_INT
 integer(C_INT), parameter, public :: IDA_VECTOROP_ERR = -28_C_INT
 integer(C_INT), parameter, public :: IDA_UNRECOGNIZED_ERROR = -99_C_INT
 integer(C_INT), parameter, public :: IDALS_SUCCESS = 0_C_INT
 integer(C_INT), parameter, public :: IDALS_MEM_NULL = -1_C_INT
 integer(C_INT), parameter, public :: IDALS_LMEM_NULL = -2_C_INT
 integer(C_INT), parameter, public :: IDALS_ILL_INPUT = -3_C_INT
 integer(C_INT), parameter, public :: IDALS_MEM_FAIL = -4_C_INT
 integer(C_INT), parameter, public :: IDALS_PMEM_NULL = -5_C_INT
 integer(C_INT), parameter, public :: IDALS_JACFUNC_UNRECVR = -6_C_INT
 integer(C_INT), parameter, public :: IDALS_JACFUNC_RECVR = -7_C_INT
 integer(C_INT), parameter, public :: IDALS_SUNMAT_FAIL = -8_C_INT
 integer(C_INT), parameter, public :: IDALS_SUNLS_FAIL = -9_C_INT

 ! WRAPPER DECLARATIONS
 interface
function FIDACreate() &
bind(C, name="IDACreate") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR) :: fresult
end function

function FIDAInit(ida_mem, res, t0, yy0, yp0) &
bind(C, name="IDAInit") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_FUNPTR), value :: res
real(C_DOUBLE), value :: t0
type(C_PTR), value :: yy0
type(C_PTR), value :: yp0
integer(C_INT) :: fresult
end function

function FIDAReInit(ida_mem, t0, yy0, yp0) &
bind(C, name="IDAReInit") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: t0
type(C_PTR), value :: yy0
type(C_PTR), value :: yp0
integer(C_INT) :: fresult
end function

function FIDASStolerances(ida_mem, reltol, abstol) &
bind(C, name="IDASStolerances") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: reltol
real(C_DOUBLE), value :: abstol
integer(C_INT) :: fresult
end function

function FIDASVtolerances(ida_mem, reltol, abstol) &
bind(C, name="IDASVtolerances") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: reltol
type(C_PTR), value :: abstol
integer(C_INT) :: fresult
end function

function FIDAWFtolerances(ida_mem, efun) &
bind(C, name="IDAWFtolerances") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_FUNPTR), value :: efun
integer(C_INT) :: fresult
end function

function FIDACalcIC(ida_mem, icopt, tout1) &
bind(C, name="IDACalcIC") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: icopt
real(C_DOUBLE), value :: tout1
integer(C_INT) :: fresult
end function

function FIDASetNonlinConvCoefIC(ida_mem, epiccon) &
bind(C, name="IDASetNonlinConvCoefIC") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: epiccon
integer(C_INT) :: fresult
end function

function FIDASetMaxNumStepsIC(ida_mem, maxnh) &
bind(C, name="IDASetMaxNumStepsIC") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: maxnh
integer(C_INT) :: fresult
end function

function FIDASetMaxNumJacsIC(ida_mem, maxnj) &
bind(C, name="IDASetMaxNumJacsIC") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: maxnj
integer(C_INT) :: fresult
end function

function FIDASetMaxNumItersIC(ida_mem, maxnit) &
bind(C, name="IDASetMaxNumItersIC") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: maxnit
integer(C_INT) :: fresult
end function

function FIDASetLineSearchOffIC(ida_mem, lsoff) &
bind(C, name="IDASetLineSearchOffIC") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
logical(C_BOOL), value :: lsoff
integer(C_INT) :: fresult
end function

function FIDASetStepToleranceIC(ida_mem, steptol) &
bind(C, name="IDASetStepToleranceIC") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: steptol
integer(C_INT) :: fresult
end function

function FIDASetMaxBacksIC(ida_mem, maxbacks) &
bind(C, name="IDASetMaxBacksIC") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: maxbacks
integer(C_INT) :: fresult
end function

function FIDASetErrHandlerFn(ida_mem, ehfun, eh_data) &
bind(C, name="IDASetErrHandlerFn") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_FUNPTR), value :: ehfun
type(C_PTR), value :: eh_data
integer(C_INT) :: fresult
end function

function FIDASetErrFile(ida_mem, errfp) &
bind(C, name="IDASetErrFile") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_PTR), value :: errfp
integer(C_INT) :: fresult
end function

function FIDASetUserData(ida_mem, user_data) &
bind(C, name="IDASetUserData") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_PTR), value :: user_data
integer(C_INT) :: fresult
end function

function FIDASetMaxOrd(ida_mem, maxord) &
bind(C, name="IDASetMaxOrd") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: maxord
integer(C_INT) :: fresult
end function

function FIDASetMaxNumSteps(ida_mem, mxsteps) &
bind(C, name="IDASetMaxNumSteps") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG), value :: mxsteps
integer(C_INT) :: fresult
end function

function FIDASetInitStep(ida_mem, hin) &
bind(C, name="IDASetInitStep") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: hin
integer(C_INT) :: fresult
end function

function FIDASetMaxStep(ida_mem, hmax) &
bind(C, name="IDASetMaxStep") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: hmax
integer(C_INT) :: fresult
end function

function FIDASetStopTime(ida_mem, tstop) &
bind(C, name="IDASetStopTime") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: tstop
integer(C_INT) :: fresult
end function

function FIDASetNonlinConvCoef(ida_mem, epcon) &
bind(C, name="IDASetNonlinConvCoef") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: epcon
integer(C_INT) :: fresult
end function

function FIDASetMaxErrTestFails(ida_mem, maxnef) &
bind(C, name="IDASetMaxErrTestFails") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: maxnef
integer(C_INT) :: fresult
end function

function FIDASetMaxNonlinIters(ida_mem, maxcor) &
bind(C, name="IDASetMaxNonlinIters") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: maxcor
integer(C_INT) :: fresult
end function

function FIDASetMaxConvFails(ida_mem, maxncf) &
bind(C, name="IDASetMaxConvFails") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: maxncf
integer(C_INT) :: fresult
end function

function FIDASetSuppressAlg(ida_mem, suppressalg) &
bind(C, name="IDASetSuppressAlg") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
logical(C_BOOL), value :: suppressalg
integer(C_INT) :: fresult
end function

function FIDASetId(ida_mem, id) &
bind(C, name="IDASetId") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_PTR), value :: id
integer(C_INT) :: fresult
end function

function FIDASetConstraints(ida_mem, constraints) &
bind(C, name="IDASetConstraints") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_PTR), value :: constraints
integer(C_INT) :: fresult
end function

function FIDASetNonlinearSolver(ida_mem, nls) &
bind(C, name="IDASetNonlinearSolver") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_PTR), value :: nls
integer(C_INT) :: fresult
end function

function FIDARootInit(ida_mem, nrtfn, g) &
bind(C, name="IDARootInit") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT), value :: nrtfn
type(C_FUNPTR), value :: g
integer(C_INT) :: fresult
end function

function FIDASetRootDirection(ida_mem, rootdir) &
bind(C, name="IDASetRootDirection") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT) :: rootdir
integer(C_INT) :: fresult
end function

function FIDASetNoInactiveRootWarn(ida_mem) &
bind(C, name="IDASetNoInactiveRootWarn") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT) :: fresult
end function

function FIDASolve(ida_mem, tout, tret, yret, ypret, itask) &
bind(C, name="IDASolve") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: tout
real(C_DOUBLE) :: tret
type(C_PTR), value :: yret
type(C_PTR), value :: ypret
integer(C_INT), value :: itask
integer(C_INT) :: fresult
end function

function FIDAGetDky(ida_mem, t, k, dky) &
bind(C, name="IDAGetDky") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: t
integer(C_INT), value :: k
type(C_PTR), value :: dky
integer(C_INT) :: fresult
end function

function FIDAGetWorkSpace(ida_mem, lenrw, leniw) &
bind(C, name="IDAGetWorkSpace") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: lenrw
integer(C_LONG) :: leniw
integer(C_INT) :: fresult
end function

function FIDAGetNumSteps(ida_mem, nsteps) &
bind(C, name="IDAGetNumSteps") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nsteps
integer(C_INT) :: fresult
end function

function FIDAGetNumResEvals(ida_mem, nrevals) &
bind(C, name="IDAGetNumResEvals") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nrevals
integer(C_INT) :: fresult
end function

function FIDAGetNumLinSolvSetups(ida_mem, nlinsetups) &
bind(C, name="IDAGetNumLinSolvSetups") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nlinsetups
integer(C_INT) :: fresult
end function

function FIDAGetNumErrTestFails(ida_mem, netfails) &
bind(C, name="IDAGetNumErrTestFails") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: netfails
integer(C_INT) :: fresult
end function

function FIDAGetNumBacktrackOps(ida_mem, nbacktr) &
bind(C, name="IDAGetNumBacktrackOps") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nbacktr
integer(C_INT) :: fresult
end function

function FIDAGetConsistentIC(ida_mem, yy0_mod, yp0_mod) &
bind(C, name="IDAGetConsistentIC") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_PTR), value :: yy0_mod
type(C_PTR), value :: yp0_mod
integer(C_INT) :: fresult
end function

function FIDAGetLastOrder(ida_mem, klast) &
bind(C, name="IDAGetLastOrder") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT) :: klast
integer(C_INT) :: fresult
end function

function FIDAGetCurrentOrder(ida_mem, kcur) &
bind(C, name="IDAGetCurrentOrder") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT) :: kcur
integer(C_INT) :: fresult
end function

function FIDAGetActualInitStep(ida_mem, hinused) &
bind(C, name="IDAGetActualInitStep") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE) :: hinused
integer(C_INT) :: fresult
end function

function FIDAGetLastStep(ida_mem, hlast) &
bind(C, name="IDAGetLastStep") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE) :: hlast
integer(C_INT) :: fresult
end function

function FIDAGetCurrentStep(ida_mem, hcur) &
bind(C, name="IDAGetCurrentStep") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE) :: hcur
integer(C_INT) :: fresult
end function

function FIDAGetCurrentTime(ida_mem, tcur) &
bind(C, name="IDAGetCurrentTime") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE) :: tcur
integer(C_INT) :: fresult
end function

function FIDAGetTolScaleFactor(ida_mem, tolsfact) &
bind(C, name="IDAGetTolScaleFactor") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE) :: tolsfact
integer(C_INT) :: fresult
end function

function FIDAGetErrWeights(ida_mem, eweight) &
bind(C, name="IDAGetErrWeights") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_PTR), value :: eweight
integer(C_INT) :: fresult
end function

function FIDAGetEstLocalErrors(ida_mem, ele) &
bind(C, name="IDAGetEstLocalErrors") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_PTR), value :: ele
integer(C_INT) :: fresult
end function

function FIDAGetNumGEvals(ida_mem, ngevals) &
bind(C, name="IDAGetNumGEvals") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: ngevals
integer(C_INT) :: fresult
end function

function FIDAGetRootInfo(ida_mem, rootsfound) &
bind(C, name="IDAGetRootInfo") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT) :: rootsfound
integer(C_INT) :: fresult
end function

function FIDAGetIntegratorStats(ida_mem, nsteps, nrevals, nlinsetups, netfails, qlast, qcur, hinused, hlast, hcur, tcur) &
bind(C, name="IDAGetIntegratorStats") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nsteps
integer(C_LONG) :: nrevals
integer(C_LONG) :: nlinsetups
integer(C_LONG) :: netfails
integer(C_INT) :: qlast
integer(C_INT) :: qcur
real(C_DOUBLE) :: hinused
real(C_DOUBLE) :: hlast
real(C_DOUBLE) :: hcur
real(C_DOUBLE) :: tcur
integer(C_INT) :: fresult
end function

function FIDAGetNumNonlinSolvIters(ida_mem, nniters) &
bind(C, name="IDAGetNumNonlinSolvIters") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nniters
integer(C_INT) :: fresult
end function

function FIDAGetNumNonlinSolvConvFails(ida_mem, nncfails) &
bind(C, name="IDAGetNumNonlinSolvConvFails") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nncfails
integer(C_INT) :: fresult
end function

function FIDAGetNonlinSolvStats(ida_mem, nniters, nncfails) &
bind(C, name="IDAGetNonlinSolvStats") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nniters
integer(C_LONG) :: nncfails
integer(C_INT) :: fresult
end function

function FIDAGetReturnFlagName(flag) &
bind(C, name="IDAGetReturnFlagName") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
integer(C_LONG), value :: flag
type(C_PTR) :: fresult
end function

subroutine FIDAFree(ida_mem) &
bind(C, name="IDAFree")
use, intrinsic :: ISO_C_BINDING
type(C_PTR) :: ida_mem
end subroutine

function FIDABBDPrecInit(ida_mem, nlocal, mudq, mldq, mukeep, mlkeep, dq_rel_yy, gres, gcomm) &
bind(C, name="IDABBDPrecInit") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT64_T), value :: nlocal
integer(C_INT64_T), value :: mudq
integer(C_INT64_T), value :: mldq
integer(C_INT64_T), value :: mukeep
integer(C_INT64_T), value :: mlkeep
real(C_DOUBLE), value :: dq_rel_yy
type(C_FUNPTR), value :: gres
type(C_FUNPTR), value :: gcomm
integer(C_INT) :: fresult
end function

function FIDABBDPrecReInit(ida_mem, mudq, mldq, dq_rel_yy) &
bind(C, name="IDABBDPrecReInit") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_INT64_T), value :: mudq
integer(C_INT64_T), value :: mldq
real(C_DOUBLE), value :: dq_rel_yy
integer(C_INT) :: fresult
end function

function FIDABBDPrecGetWorkSpace(ida_mem, lenrwbbdp, leniwbbdp) &
bind(C, name="IDABBDPrecGetWorkSpace") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: lenrwbbdp
integer(C_LONG) :: leniwbbdp
integer(C_INT) :: fresult
end function

function FIDABBDPrecGetNumGfnEvals(ida_mem, ngevalsbbdp) &
bind(C, name="IDABBDPrecGetNumGfnEvals") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: ngevalsbbdp
integer(C_INT) :: fresult
end function

function FIDASetLinearSolver(ida_mem, ls, a) &
bind(C, name="IDASetLinearSolver") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_PTR), value :: ls
type(C_PTR), value :: a
integer(C_INT) :: fresult
end function

function FIDASetJacFn(ida_mem, jac) &
bind(C, name="IDASetJacFn") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_FUNPTR), value :: jac
integer(C_INT) :: fresult
end function

function FIDASetPreconditioner(ida_mem, pset, psolve) &
bind(C, name="IDASetPreconditioner") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_FUNPTR), value :: pset
type(C_FUNPTR), value :: psolve
integer(C_INT) :: fresult
end function

function FIDASetJacTimes(ida_mem, jtsetup, jtimes) &
bind(C, name="IDASetJacTimes") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
type(C_FUNPTR), value :: jtsetup
type(C_FUNPTR), value :: jtimes
integer(C_INT) :: fresult
end function

function FIDASetEpsLin(ida_mem, eplifac) &
bind(C, name="IDASetEpsLin") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: eplifac
integer(C_INT) :: fresult
end function

function FIDASetIncrementFactor(ida_mem, dqincfac) &
bind(C, name="IDASetIncrementFactor") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
real(C_DOUBLE), value :: dqincfac
integer(C_INT) :: fresult
end function

function FIDAGetLinWorkSpace(ida_mem, lenrwls, leniwls) &
bind(C, name="IDAGetLinWorkSpace") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: lenrwls
integer(C_LONG) :: leniwls
integer(C_INT) :: fresult
end function

function FIDAGetNumJacEvals(ida_mem, njevals) &
bind(C, name="IDAGetNumJacEvals") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: njevals
integer(C_INT) :: fresult
end function

function FIDAGetNumPrecEvals(ida_mem, npevals) &
bind(C, name="IDAGetNumPrecEvals") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: npevals
integer(C_INT) :: fresult
end function

function FIDAGetNumPrecSolves(ida_mem, npsolves) &
bind(C, name="IDAGetNumPrecSolves") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: npsolves
integer(C_INT) :: fresult
end function

function FIDAGetNumLinIters(ida_mem, nliters) &
bind(C, name="IDAGetNumLinIters") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nliters
integer(C_INT) :: fresult
end function

function FIDAGetNumLinConvFails(ida_mem, nlcfails) &
bind(C, name="IDAGetNumLinConvFails") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nlcfails
integer(C_INT) :: fresult
end function

function FIDAGetNumJTSetupEvals(ida_mem, njtsetups) &
bind(C, name="IDAGetNumJTSetupEvals") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: njtsetups
integer(C_INT) :: fresult
end function

function FIDAGetNumJtimesEvals(ida_mem, njvevals) &
bind(C, name="IDAGetNumJtimesEvals") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: njvevals
integer(C_INT) :: fresult
end function

function FIDAGetNumLinResEvals(ida_mem, nrevalsls) &
bind(C, name="IDAGetNumLinResEvals") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: nrevalsls
integer(C_INT) :: fresult
end function

function FIDAGetLastLinFlag(ida_mem, flag) &
bind(C, name="IDAGetLastLinFlag") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: ida_mem
integer(C_LONG) :: flag
integer(C_INT) :: fresult
end function

function FIDAGetLinReturnFlagName(flag) &
bind(C, name="IDAGetLinReturnFlagName") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
integer(C_LONG), value :: flag
type(C_PTR) :: fresult
end function

 end interface


end module
