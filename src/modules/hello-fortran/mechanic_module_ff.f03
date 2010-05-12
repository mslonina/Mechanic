!
! Fortran 2003 sample module for MECHANIC
!
! MECHANIC
! Copyright (c) 2010, Mariusz Slonina (Nicolaus Copernicus University)
! All rights reserved.
!
! This file is part of MECHANIC code.
!
! MECHANIC was created to help solving many numerical problems by providing
! tools for improving scalability and functionality of the code. MECHANIC was
! released in belief it will be useful. If you are going to use this code, or
! its parts, please consider referring to the authors either by the website
! or the user guide reference.
!
! http://mechanics.astri.umk.pl/projects/mechanic
!
! User guide should be provided with the package or
! http://mechanics.astri.umk.pl/projects/mechanic/mechanic_userguide.pdf
!
! Redistribution and use in source and binary forms,
! with or without modification, are permitted provided
! that the following conditions are met:
!
! - Redistributions of source code must retain the above copyright notice,
!   this list of conditions and the following disclaimer.
! - Redistributions in binary form must reproduce the above copyright notice,
!   this list of conditions and the following disclaimer in the documentation
!   and/or other materials provided with the distribution.
! - Neither the name of the Nicolaus Copernicus University nor the names of
!   its contributors may be used to endorse or promote products derived from
!   this software without specific prior written permission.
!
! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
! POSSIBILITY OF SUCH DAMAGE.
!

! [F2003BIND]
! @section fortran The Fortran 2003 Bindings
! You can create Fortran 2003 module for @M using provided Fortran bindings.
! Below is an example of Fortran module.
!
! @code
! @icode modules/hello-fortran/mechanic_module_ff.f03 F2003MODULE
! @endcode
! [/F2003BIND]

! [F2003MODULE]
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

  ! Implementation of module_processPixel()
  integer (c_int) function ff_processPixel(node, md, d, r) &
    bind(c, name = 'ff_processPixel') result(errcode)

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
  end function ff_processPixel

end module ff
! [/F2003MODULE]
