C File: pvdiagkbf.f
C Diagonal ODE example.  Stiff case, with diagonal preconditioner.
C Uses FCVODE interfaces and FCVBBD interfaces.
C Solves problem twice -- with left and right preconditioning.
C Version of 1 August 2003
C
      IMPLICIT DOUBLE PRECISION (A-H, O-Z)
C
C Include MPI-Fortran header file for MPI_COMM_WORLD, MPI types.

      INCLUDE "mpif.h"
C
      INTEGER*4 IOPT(40)
      DIMENSION Y(1024), ROPT(40)
      DATA ATOL/1.0E-10/, RTOL/1.0E-5/, DTOUT/0.1D0/, NOUT/10/
      DATA LNST/4/, LNFE/5/, LNSETUP/6/, LNNI/7/, LNCF/8/, LNETF/9/,
     1     LNPE/16/, LNLI/17/, LNPS/18/, LNCFL/19/
C
      COMMON /PCOM/ ALPHA, MYPE, NLOCAL
C
C Get NPES and MYPE.  Requires initialization of MPI.
      CALL MPI_INIT(IER)
      IF (IER .NE. 0) THEN
         WRITE(6,5) IER
 5       FORMAT(///' MPI_INIT returned IER =',I5)
         STOP
      ENDIF
      CALL MPI_COMM_SIZE(MPI_COMM_WORLD, NPES, IER)
      IF (IER .NE. 0) THEN
         WRITE(6,6) IER
 6       FORMAT(///' MPI_COMM_SIZE returned IER =',I5)
         STOP
      ENDIF
      CALL MPI_COMM_RANK(MPI_COMM_WORLD, MYPE, IER)
      IF (IER .NE. 0) THEN
         WRITE(6,7) IER
 7       FORMAT(///' MPI_COMM_RANK returned IER =',I5)
         STOP
      ENDIF
      
C
C Set input arguments.
      NLOCAL = 10
      NEQ = NPES*NLOCAL
      T = 0.0D0
      METH = 2
      ITMETH = 2
      IATOL = 1
      INOPT = 0
      ITASK = 0
      IPRE = 1
      IGS = 0
C Set parameter alpha
      ALPHA  = 10.0D0
C
      DO 10 I = 1,NLOCAL
 10     Y(I) = 1.0D0
C
      IF (MYPE .EQ. 0) THEN
        WRITE(6,15) NEQ, ALPHA, RTOL, ATOL, NPES
 15     FORMAT('Diagonal test problem, size NEQ =',I5,
     1         '  parameter alpha = ',F8.3/
     2         '  ydot_i = -alpha*i * y_i (i = 1,...,NEQ)'//
     3         'RTOL, ATOL = ',2E10.1//'Method is BDF/NEWTON/SPGMR'/
     4         'Preconditioner is band-block-diagonal, using CVBBDPRE'//
     5         'Number of processors = ',I3)
      ENDIF
C
      CALL FNVSPECINITP(NLOCAL, NEQ, IER)
C
      IF (IER .NE. 0) THEN
        WRITE(6,20) IER
 20     FORMAT(///' FMENVINITP returned IER =',I5)
        STOP
        ENDIF
C
      CALL FCVMALLOC(T, Y, METH, ITMETH, IATOL, RTOL, ATOL,
     1               INOPT, IOPT, ROPT, IER)
C
      IF (IER .NE. 0) THEN
        WRITE(6,30) IER
 30     FORMAT(///' FCVMALLOC returned IER =',I5)
        STOP
        ENDIF
C
      MUDQ = 0
      MLDQ = 0
      MU = 0
      ML = 0
      CALL FCVBBDIN (NLOCAL, MUDQ, MLDQ, MU,ML, 0.0D0, 
     1               IPRE, IGS, 0, 0.0D0, IER)
      IF (IER .NE. 0) THEN
        WRITE(6,35) IER
 35     FORMAT(///' FCVBBDIN0 returned IER =',I5)
        STOP
        ENDIF
      IF (MYPE.EQ. 0) WRITE(6,38)
 38   FORMAT(///'Preconditioning on left'//)
C
C Looping point for cases IPRE = 1 and 2.
C
 40   CONTINUE
C
C Loop through tout values, call solver, print output, test for failure.
      TOUT = DTOUT
      DO 60 IOUT = 1,NOUT
C
        CALL FCVODE(TOUT, T, Y, ITASK, IER)
C
        IF (MYPE .EQ. 0) WRITE(6,45) T,IOPT(LNST),IOPT(LNFE)
 45     FORMAT(/' t =',E10.2,5X,'no. steps =',I5,'   no. f-s =',I5)
C
        IF (IER .NE. 0) THEN
          WRITE(6,50) IER
 50       FORMAT(///' FCVODE returned IER =',I5)
          STOP
          ENDIF
C
        TOUT = TOUT + DTOUT
 60     CONTINUE
C
C Get max. absolute error in the local vector.
      ERMAX = 0.0D0
      DO 65 I = 1,NLOCAL
        ERRI  = Y(I) - EXP(-ALPHA*(MYPE*NLOCAL + I)*T)
        ERMAX = MAX(ERMAX,ABS(ERRI))
 65     CONTINUE
C Get global max. error from MPI_REDUCE call.
      CALL MPI_REDUCE(ERMAX, GERMAX, 1, MPI_DOUBLE_PRECISION, MPI_MAX,
     1                0, MPI_COMM_WORLD, IER)
      IF (IER .NE. 0) THEN
        WRITE(6,70) IER
 70     FORMAT(///' MPI_REDUCE returned IER =',I5)
        STOP
        ENDIF
      IF (MYPE .EQ. 0) WRITE(6,75) GERMAX
 75   FORMAT(//'Max. absolute error is',E10.2)
C
C Print final statistics.
      IF (MYPE .EQ. 0) THEN
      NST = IOPT(LNST)
      NFE = IOPT(LNFE)
      NPSET = IOPT(LNSETUP)
      NPE = IOPT(LNPE)
      NPS = IOPT(LNPS)
      NNI = IOPT(LNNI)
      NLI = IOPT(LNLI)
      AVDIM = REAL(NLI)/REAL(NNI)
      NCFN = IOPT(LNCF)
      NCFL = IOPT(LNCFL)
      NETF = IOPT(LNETF)
      WRITE (6,80) NST,NFE,NPSET,NPE,NPS,NNI,NLI,AVDIM,NCFN,NCFL,NETF
 80   FORMAT(//'Final statistics..'/
     1 ' number of steps        =',I5,5X,'number of f evals.     =',I5/
     2 ' number of prec. setups =',I5/
     3 ' number of prec. evals. =',I5,5X,'number of prec. solves =',I5/
     4 ' number of nonl. iters. =',I5,5X,'number of lin. iters.  =',I5/
     5 ' average Krylov subspace dimension (NLI/NNI)  =',F8.4/
     6 ' number of conv. failures..  nonlinear =',I3,'  linear =',I3/
     7 ' number of error test failures =',I3)
      CALL FCVBBDOPT(LENRPW, LENIPW, NGE)
      WRITE (6,82) LENRPW, LENIPW, NGE
 82   FORMAT('In CVBBDPRE: real/integer local work space sizes =',2I5/
     1 ' number of g evals.     =',I5)
      ENDIF
C
C If IPRE = 1, re-initialize T, Y, and the solver, and loop for case IPRE = 2.
C Otherwise jump to final block.
      IF (IPRE .EQ. 2) GO TO 99

      T = 0.0D0
      DO 90 I = 1,NLOCAL
 90      Y(I) = 1.0D0
         
      CALL FCVREINIT(T, Y, IATOL, RTOL, ATOL,
     1               INOPT, IOPT, ROPT, IER)
      IF (IER .NE. 0) THEN
         WRITE(6,91) IER
 91      FORMAT(///' FCVREINIT returned IER =',I5)
         STOP
      ENDIF

      IPRE = 2

      CALL FCVREINBBD (NLOCAL, MUDQ, MLDQ, 
     1                 0D0, IPRE, IGS, 
     2                 0.0D0, IER)
      IF (IER .NE. 0) THEN
        WRITE(6,92) IER
 92     FORMAT(///' FCVREINBBD0 returned IER =',I5)
        STOP
      ENDIF
      IF (MYPE .EQ. 0) WRITE (6,95)
 95   FORMAT(///60('-')///'Preconditioning on right'//)
      GO TO 40
C
C Free the memory and finalize MPI.
 99   CALL FCVBBDF
      CALL FCVFREE
      CALL FNVSPECFREEP
      CALL MPI_FINALIZE(IER)
C
      STOP
      END

      SUBROUTINE CVFUN(T, Y, YDOT)
C Routine for right-hand side function f
      IMPLICIT DOUBLE PRECISION (A-H, O-Z)
      DIMENSION Y(*), YDOT(*)
      COMMON /PCOM/ ALPHA, MYPE, NLOCAL
C
      DO 10 I = 1,NLOCAL
  10    YDOT(I) = -ALPHA*(MYPE*NLOCAL + I)*Y(I)
C
      RETURN
      END

      SUBROUTINE CVLOCFN(NLOC, T, YLOC, GLOC)
C Routine to define local approximate function g, here the same as f. 
      IMPLICIT DOUBLE PRECISION (A-H, O-Z)
      DIMENSION YLOC(*), GLOC(*)
C
      CALL CVFUN(T, YLOC, GLOC)
C
      RETURN
      END

      SUBROUTINE CVCOMMF(NLOC, T, YLOC)
C Routine to perform communication required for evaluation of g.
      RETURN
      END
