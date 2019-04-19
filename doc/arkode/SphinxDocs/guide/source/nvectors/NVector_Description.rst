..
   Programmer(s): Daniel R. Reynolds @ SMU
   ----------------------------------------------------------------
   SUNDIALS Copyright Start
   Copyright (c) 2002-2019, Lawrence Livermore National Security
   and Southern Methodist University.
   All rights reserved.

   See the top-level LICENSE and NOTICE files for details.

   SPDX-License-Identifier: BSD-3-Clause
   SUNDIALS Copyright End
   ----------------------------------------------------------------

:tocdepth: 3


.. _NVectors.Description:

Description of the NVECTOR Modules
======================================

The SUNDIALS solvers are written in a data-independent manner. They
all operate on generic vectors (of type ``N_Vector``) through a set of
operations defined by, and specific to, the particular NVECTOR
implementation. Users can provide a custom implementation of the
NVECTOR module or use one of four provided within SUNDIALS -- a serial
and three parallel implementations.  The generic operations are
described below.  In the sections following, the implementations
provided with SUNDIALS are described.

The generic ``N_Vector`` type is a pointer to a structure that has an
implementation-dependent *content* field containing the description
and actual data of the vector, and an *ops* field pointing to a
structure with generic vector operations. The type ``N_Vector`` is
defined as:

.. code-block:: c

   typedef struct _generic_N_Vector *N_Vector;

   struct _generic_N_Vector {
      void *content;
      struct _generic_N_Vector_Ops *ops;
   };

Here, the ``_generic_N_Vector_Op`` structure is essentially a list of
function pointers to the various actual vector operations, and is
defined as

.. code-block:: c

   struct _generic_N_Vector_Ops {
      N_Vector_ID  (*nvgetvectorid)(N_Vector);
      N_Vector     (*nvclone)(N_Vector);
      N_Vector     (*nvcloneempty)(N_Vector);
      void         (*nvdestroy)(N_Vector);
      void         (*nvspace)(N_Vector, sunindextype *, sunindextype *);
      realtype*    (*nvgetarraypointer)(N_Vector);
      void         (*nvsetarraypointer)(realtype *, N_Vector);
      void*        (*nvgetcommunicator)(N_Vector);
      sunindextype (*nvgetlength)(N_Vector);
      void         (*nvlinearsum)(realtype, N_Vector, realtype, N_Vector, N_Vector);
      void         (*nvconst)(realtype, N_Vector);
      void         (*nvprod)(N_Vector, N_Vector, N_Vector);
      void         (*nvdiv)(N_Vector, N_Vector, N_Vector);
      void  	   (*nvscale)(realtype, N_Vector, N_Vector);
      void  	   (*nvabs)(N_Vector, N_Vector);
      void	   (*nvinv)(N_Vector, N_Vector);
      void	   (*nvaddconst)(N_Vector, realtype, N_Vector);
      realtype	   (*nvdotprod)(N_Vector, N_Vector);
      realtype	   (*nvmaxnorm)(N_Vector);
      realtype	   (*nvwrmsnorm)(N_Vector, N_Vector);
      realtype	   (*nvwrmsnormmask)(N_Vector, N_Vector, N_Vector);
      realtype	   (*nvmin)(N_Vector);
      realtype	   (*nvwl2norm)(N_Vector, N_Vector);
      realtype	   (*nvl1norm)(N_Vector);
      void	   (*nvcompare)(realtype, N_Vector, N_Vector);
      booleantype  (*nvinvtest)(N_Vector, N_Vector);
      booleantype  (*nvconstrmask)(N_Vector, N_Vector, N_Vector);
      realtype	   (*nvminquotient)(N_Vector, N_Vector);
      int          (*nvlinearcombination)(int, realtype *, N_Vector *, N_Vector);
      int          (*nvscaleaddmulti)(int, realtype *, N_Vector, N_Vector *, N_Vector *);
      int          (*nvdotprodmulti)(int, N_Vector, N_Vector *,  realtype *);
      int          (*nvlinearsumvectorarray)(int, realtype, N_Vector *, realtype,
                                             N_Vector *, N_Vector *);
      int          (*nvscalevectorarray)(int, realtype *,  N_Vector *, N_Vector *);
      int          (*nvconstvectorarray)(int, realtype, N_Vector *);
      int          (*nvwrmsnomrvectorarray)(int, N_Vector *, N_Vector *, realtype *);
      int          (*nvwrmsnomrmaskvectorarray)(int, N_Vector *, N_Vector *, N_Vector,
                                                realtype *);
      int          (*nvscaleaddmultivectorarray)(int, int, realtype *, N_Vector *,
                                                 N_Vector **, N_Vector **);
      int          (*nvlinearcombinationvectorarray)(int, int, realtype *, N_Vector **,
                                                     N_Vector *);
      realtype     (*nvdotprodlocal)(N_Vector, N_Vector);
      realtype     (*nvmaxnormlocal)(N_Vector);
      realtype     (*nvminlocal)(N_Vector);
      realtype     (*nvl1normlocal)(N_Vector);
      booleantype  (*nvinvtestlocal)(N_Vector, N_Vector);
      booleantype  (*nvconstrmasklocal)(N_Vector, N_Vector, N_Vector);
      realtype     (*nvminquotientlocal)(N_Vector, N_Vector);
      realtype     (*nvwsqrsumlocal)(N_Vector, N_Vector);
      realtype     (*nvwsqrsummasklocal(N_Vector, N_Vector, N_Vector);
   };


The generic NVECTOR module defines and implements the vector
operations acting on a ``N_Vector``. These routines are nothing but
wrappers for the vector operations defined by a particular NVECTOR
implementation, which are accessed through the *ops* field of the
``N_Vector`` structure. To illustrate this point we show below the
implementation of a typical vector operation from the generic NVECTOR
module, namely ``N_VScale``, which performs the scaling of a vector
``x`` by a scalar ``c``:

.. code-block:: c

   void N_VScale(realtype c, N_Vector x, N_Vector z) {
      z->ops->nvscale(c, x, z);
   }

The subsection :ref:`NVectors.Ops` contains a complete list of all
standard vector operations defined by the generic NVECTOR module.  The
subsections :ref:`NVectors.FusedOps`, :ref:`NVectors.ArrayOps` and
:ref:`NVectors.LocalOps` list *optional* fused, vector array and local
reduction operations respectively.

Fused and vector array operations are intended to increase data reuse, reduce
parallel communication on distributed memory systems, and lower the number of
kernel launches on systems with accelerators. If a particular NVECTOR
implementation defines a fused or vector array operation as ``NULL``, the
generic NVECTOR module will automatically call standard vector operations as
necessary to complete the desired operation. In all SUNDIALS-provided
NVECTOR implementations, all fused and vector array operations are
disabled by default.  However, these implementations provide
additional user-callable functions to enable/disable any or all of the
fused and vector array operations. See the following sections
for the implementation specific functions to enable/disable operations.

Local reduction operations are similarly intended to reduce parallel
communication on distributed memory systems, particularly when
NVECTOR objects are combined together within a NVECTOR_MANYVECTOR
object (see the section :ref:`NVectors.ManyVector`).  If a
particular NVECTOR implementation defines a local reduction
operation as ``NULL``, the NVECTOR_MANYVECTOR module will
automatically call standard vector reduction operations as necessary
to complete the desired operation. All SUNDIALS-provided NVECTOR
implementations include these local reduction operations, which may be
used as templates for user-defined NVECTOR implementations.

Finally, we note that the generic NVECTOR module defines the functions
``N_VCloneVectorArray``, ``N_VCloneVectorArrayEmpty``, and
``N_VDestroyVectorArray``. Both clone functions create (by cloning) an array of
*count* variables of type ``N_Vector``, each of the same type as an existing
``N_Vector``. Their prototypes are:

.. code-block:: c

   N_Vector *N_VCloneVectorArray(int count, N_Vector w);
   N_Vector *N_VCloneVectorArrayEmpty(int count, N_Vector w);

and their definitions are based on the implementation-specific
``N_VClone`` and ``N_VCloneEmpty`` operations, respectively.
An array of variables of type ``N_Vector`` can be destroyed
by calling ``N_VDestroyVectorArray``, whose prototype is

.. code-block:: c

   void N_VDestroyVectorArray(N_Vector *vs, int count);

and whose definition is based on the implementation-specific
``N_VDestroy`` operation.

A particular implementation of the NVECTOR module **must**:

* Specify the *content* field of the ``N_Vector``.

* Define and implement the necessary vector operations. Note that the
  names of these routines should be unique to that implementation in
  order to permit using more than one NVECTOR module (each with
  different ``N_Vector`` internal data representations) in the same
  code.

* Define and implement user-callable constructor and destructor
  routines to create and free a ``N_Vector`` with the new *content*
  field and with *ops* pointing to the new vector operations.

* Optionally, define and implement additional user-callable routines
  acting on the newly defined ``N_Vector`` (e.g., a routine to print the
  *content* for debugging purposes).

* Optionally, provide accessor macros as needed for that particular
  implementation to be used to access different parts in the content
  field of the newly defined ``N_Vector``.

To aid in the creation of custom NVECTOR modules the generic NVECTOR module
provides two utility functions :c:func:`N_VNewEmpty()` and
:c:func:`N_VCopyOps()`. When used in custom NVECTOR constructors and clone
routines these functions will ease the introduction of any new optional vector
operations to the NVECTOR API by ensuring only required operations need to be
set and all operations are copied when cloning a vector.

.. c:function:: N_Vector N_VNewEmpty()

  This allocates a new generic ``N_Vector`` object and initializes its content
  pointer and the function pointers in the operations structure to ``NULL``.

  **Return value:** If successful, this function returns an ``N_Vector``
  object. If an error occurs when allocating the object, then this routine will
  return ``NULL``.

.. c:function:: int N_VCopyOps(N_Vector w, N_Vector v)

  This function copies the function pointers in the ``ops`` structure of ``w``
  into the ``ops`` structure of ``v``. 

   **Arguments:**
      * *w* -- the vector to copy operations from.
      * *v* -- the vector to copy operations to.

   **Return value:**  If successful, this function returns ``0``. If either of
   the inputs are ``NULL`` or the ``ops`` structure of either input is ``NULL``,
   then is function returns a non-zero value.

Each NVECTOR implementation included in SUNDIALS has a unique
identifier specified in enumeration and shown in the table below.
It is recommended that a user supplied NVECTOR implementation use the
``SUNDIALS_NVEC_CUSTOM`` identifier.



.. _NVector.vectorIDs:

Vector Identifications associated with vector kernels supplied with SUNDIALS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. cssclass:: table-bordered

========================  =====================================================  ==============
Vector ID                 Vector type                                            ID Value
========================  =====================================================  ==============
SUNDIALS_NVEC_SERIAL      Serial                                                 0
SUNDIALS_NVEC_PARALLEL    Distributed memory parallel (MPI)                      1
SUNDIALS_NVEC_OPENMP      OpenMP shared memory parallel                          2
SUNDIALS_NVEC_PTHREADS    PThreads shared memory parallel                        3
SUNDIALS_NVEC_PARHYP      *hypre* ParHyp parallel vector                         4
SUNDIALS_NVEC_PETSC       PETSc parallel vector                                  5
SUNDIALS_NVEC_CUDA        CUDA parallel vector                                   6
SUNDIALS_NVEC_RAJA        RAJA parallel vector                                   7
SUNDIALS_NVEC_OPENMPDEV   OpenMP parallel vector with device offloading          8
SUNDIALS_NVEC_TRILINOS    Trilinos Tpetra vector                                 9
SUNDIALS_NVEC_MANYVECTOR  "ManyVector" vector                                    10
SUNDIALS_NVEC_CUSTOM      User-provided custom vector                            11
========================  =====================================================  ==============
