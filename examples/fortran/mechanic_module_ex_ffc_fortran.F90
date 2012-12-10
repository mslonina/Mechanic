!
! C and Fortran interface, the Fortran part
!
! Please refer to the F2003 standard, i.e.
! Metcalf et al, "Fortran 95/2003 explained", Oxford University Press, 2008
!
module ffc

  use iso_c_binding
  implicit none

contains

  subroutine integrator(cfunc, n, ctrl, x, f, pstatus) bind(c)
    use iso_c_binding
    implicit none

    integer(c_int), intent(in), value :: n
    real(c_double), intent(inout), dimension(10) :: ctrl
    real(c_double), intent(out), dimension(24) :: x
    real(c_double), intent(out) :: f
    integer(c_int), intent(out) :: pstatus

    ! The interface for C-function
    interface
      function cfunc(n, x) result(cval) bind(c)
        use iso_c_binding
        implicit none
        integer(c_int) , intent(in), value :: n
        real(c_double), intent(inout), dimension(24) :: x
        integer(c_int) :: cval
      end function cfunc
    end interface

    x(8) = 77.668
    f = f + 1.0d0
      
    print *, n, ctrl(8), x(22), f

    ! Call C-function here
    pstatus = cfunc(n,x)
  end subroutine integrator

end module ffc
