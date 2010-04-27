!
! Fortran 2003 sample module for MECHANIC
!

module ff

  use iso_c_binding
  use mechanic_fortran

contains

  ! Implementation of module_init()
  integer (c_int) function ff_init(md) &
    bind(c, name = 'ff_init') result(errcode)

    implicit none
    type(moduleInfo), intent(inout) :: md

    md%mrl = 3

    errcode = 0
  end function ff_init 

  ! Implementation of module_cleanup()
  integer (c_int) function ff_cleanup(md) &
    bind(c, name = 'ff_cleanup') result(errcode)

    implicit none
    type(moduleInfo), intent(in) :: md

    write(*,*) "End module fortran:)"
    errcode = 0

  end function ff_cleanup

  ! Implementation of module_pixelCompute()
  integer (c_int) function ff_pixelCompute(node, md, d, r) &
    bind(c, name = 'ff_pixelCompute') result(errcode)

    implicit none
    integer(c_int), intent(in) :: node
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d
    type(masterData), intent(inout) :: r
    integer :: res_rank(1)

    real (c_double), pointer :: res_array(:)
    
    res_rank = md%mrl
    call c_f_pointer(r%res, res_array, res_rank)

    res_array(1) = 22.0d0
    res_array(2) = 32.0d0
    res_array(3) = 42.0d0

    errcode = 0
  end function ff_pixelCompute

end module ff
