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

module fsunlinsol_spfgmr_mod
 use, intrinsic :: ISO_C_BINDING
 use fsunlinsol_mod
 use fsundials_types
 use fnvector_mod
 use fsundials_types
 implicit none
 private

 ! PUBLIC METHODS AND TYPES
 public :: FSUNLinSol_SPFGMR
 public :: FSUNLinSol_SPFGMRSetPrecType
 public :: FSUNLinSol_SPFGMRSetGSType
 public :: FSUNLinSol_SPFGMRSetMaxRestarts
 public :: FSUNSPFGMR
 public :: FSUNSPFGMRSetPrecType
 public :: FSUNSPFGMRSetGSType
 public :: FSUNSPFGMRSetMaxRestarts
 public :: FSUNLinSolGetType_SPFGMR
 public :: FSUNLinSolInitialize_SPFGMR
 public :: FSUNLinSolSetATimes_SPFGMR
 public :: FSUNLinSolSetPreconditioner_SPFGMR
 public :: FSUNLinSolSetScalingVectors_SPFGMR
 public :: FSUNLinSolSetup_SPFGMR
 public :: FSUNLinSolSolve_SPFGMR
 public :: FSUNLinSolNumIters_SPFGMR
 public :: FSUNLinSolResNorm_SPFGMR
 public :: FSUNLinSolResid_SPFGMR
 public :: FSUNLinSolLastFlag_SPFGMR
 public :: FSUNLinSolSpace_SPFGMR
 public :: FSUNLinSolFree_SPFGMR

 ! PARAMETERS
 integer(C_INT), parameter, public :: SUNSPFGMR_MAXL_DEFAULT = 5_C_INT
 integer(C_INT), parameter, public :: SUNSPFGMR_MAXRS_DEFAULT = 0_C_INT

 ! WRAPPER DECLARATIONS
 interface
function FSUNLinSol_SPFGMR(y, pretype, maxl) &
bind(C, name="SUNLinSol_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: y
integer(C_INT), value :: pretype
integer(C_INT), value :: maxl
type(C_PTR) :: fresult
end function

function FSUNLinSol_SPFGMRSetPrecType(s, pretype) &
bind(C, name="SUNLinSol_SPFGMRSetPrecType") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT), value :: pretype
integer(C_INT) :: fresult
end function

function FSUNLinSol_SPFGMRSetGSType(s, gstype) &
bind(C, name="SUNLinSol_SPFGMRSetGSType") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT), value :: gstype
integer(C_INT) :: fresult
end function

function FSUNLinSol_SPFGMRSetMaxRestarts(s, maxrs) &
bind(C, name="SUNLinSol_SPFGMRSetMaxRestarts") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT), value :: maxrs
integer(C_INT) :: fresult
end function

function FSUNSPFGMR(y, pretype, maxl) &
bind(C, name="SUNSPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: y
integer(C_INT), value :: pretype
integer(C_INT), value :: maxl
type(C_PTR) :: fresult
end function

function FSUNSPFGMRSetPrecType(s, pretype) &
bind(C, name="SUNSPFGMRSetPrecType") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT), value :: pretype
integer(C_INT) :: fresult
end function

function FSUNSPFGMRSetGSType(s, gstype) &
bind(C, name="SUNSPFGMRSetGSType") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT), value :: gstype
integer(C_INT) :: fresult
end function

function FSUNSPFGMRSetMaxRestarts(s, maxrs) &
bind(C, name="SUNSPFGMRSetMaxRestarts") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT), value :: maxrs
integer(C_INT) :: fresult
end function

function FSUNLinSolGetType_SPFGMR(s) &
bind(C, name="SUNLinSolGetType_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT) :: fresult
end function

function FSUNLinSolInitialize_SPFGMR(s) &
bind(C, name="SUNLinSolInitialize_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT) :: fresult
end function

function FSUNLinSolSetATimes_SPFGMR(s, a_data, atimes) &
bind(C, name="SUNLinSolSetATimes_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
type(C_PTR), value :: a_data
type(C_FUNPTR), value :: atimes
integer(C_INT) :: fresult
end function

function FSUNLinSolSetPreconditioner_SPFGMR(s, p_data, pset, psol) &
bind(C, name="SUNLinSolSetPreconditioner_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
type(C_PTR), value :: p_data
type(C_FUNPTR), value :: pset
type(C_FUNPTR), value :: psol
integer(C_INT) :: fresult
end function

function FSUNLinSolSetScalingVectors_SPFGMR(s, s1, s2) &
bind(C, name="SUNLinSolSetScalingVectors_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
type(C_PTR), value :: s1
type(C_PTR), value :: s2
integer(C_INT) :: fresult
end function

function FSUNLinSolSetup_SPFGMR(s, a) &
bind(C, name="SUNLinSolSetup_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
type(C_PTR), value :: a
integer(C_INT) :: fresult
end function

function FSUNLinSolSolve_SPFGMR(s, a, x, b, tol) &
bind(C, name="SUNLinSolSolve_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
type(C_PTR), value :: a
type(C_PTR), value :: x
type(C_PTR), value :: b
real(C_DOUBLE), value :: tol
integer(C_INT) :: fresult
end function

function FSUNLinSolNumIters_SPFGMR(s) &
bind(C, name="SUNLinSolNumIters_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT) :: fresult
end function

function FSUNLinSolResNorm_SPFGMR(s) &
bind(C, name="SUNLinSolResNorm_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
real(C_DOUBLE) :: fresult
end function

function FSUNLinSolResid_SPFGMR(s) &
bind(C, name="SUNLinSolResid_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
type(C_PTR) :: fresult
end function

function FSUNLinSolLastFlag_SPFGMR(s) &
bind(C, name="SUNLinSolLastFlag_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_LONG) :: fresult
end function

function FSUNLinSolSpace_SPFGMR(s, lenrwls, leniwls) &
bind(C, name="SUNLinSolSpace_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_LONG) :: lenrwls
integer(C_LONG) :: leniwls
integer(C_INT) :: fresult
end function

function FSUNLinSolFree_SPFGMR(s) &
bind(C, name="SUNLinSolFree_SPFGMR") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: s
integer(C_INT) :: fresult
end function

 end interface


end module
