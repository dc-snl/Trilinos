!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! Zoltan Dynamic Load-Balancing Library for Parallel Applications            !
! For more info, see the README file in the top-level Zoltan directory.      ! 
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!  CVS File Information :
!     $RCSfile$
!     $Author$
!     $Date$
!     $Revision$
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "sppr_header"
module zoltan_types

!--------------------------------------------------------------------------
! kind parameters to match Fortran types with C types.
! This might have to be adjusted on some machines.

integer, parameter :: &
   LB_INT = selected_int_kind(9), &
#ifdef PTR_64BIT
   LB_INT_PTR = selected_int_kind(17), &
#else
   LB_INT_PTR = selected_int_kind(9), &
#endif
   LB_FLOAT = selected_real_kind(6), &
   LB_DOUBLE = selected_real_kind(15)

#ifdef PTR_64BIT
integer, parameter :: LB_PTR_LENGTH = 8
#else
integer, parameter :: LB_PTR_LENGTH = 4
#endif

type LB_PTR
   sequence
   character(len=LB_PTR_LENGTH) :: addr
end type LB_PTR

type(LB_PTR), parameter :: &
   LB_NULL_PTR = LB_PTR( &
#ifdef PTR_64BIT
                 char(0)//char(0)//char(0)//char(0)// &
#endif
                 char(0)//char(0)//char(0)//char(0))

interface operator(==)
   module procedure ptrcompare
end interface

private :: ptrcompare

contains

! comparison operator for C pointers

function ptrcompare(p1,p2)
logical :: ptrcompare
type(LB_PTR), intent(in) :: p1, p2
integer :: i
! should be able to compare the strings with a single statement
!   ptrcompare = (p1%addr == p2%addr)
! but bugs in PGI 3.1-2 and NAG 4.0 require comparing one character
! at a time
ptrcompare = .true.
do i=1,LB_PTR_LENGTH
   ptrcompare = ptrcompare .and. (p1%addr(i:i) == p2%addr(i:i))
end do
end function ptrcompare

end module zoltan_types
