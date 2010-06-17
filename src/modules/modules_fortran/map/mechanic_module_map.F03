!
! Sample dynamical map module for MECHANIC
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

! Dynamical map is the simplest case of using Mechanic. Here, we set initial
! conditon on master node, and modify it on each slave node according to the
! pixel coordinates. 

module map

  use iso_c_binding
  use mechanic_fortran

contains

  !
  ! STEP 0: Required functions
  !

  ! Implementation of module_init()
  integer (c_int) function map_init(md) &
    bind(c, name = 'map_init') result(errcode)

    implicit none
    type(moduleInfo), intent(inout) :: md

    md%irl = 3 ! Initial condition length
    md%mrl = 6 ! Result length

    errcode = 0
  end function map_init 

  ! Implementation of module_cleanup()
  integer (c_int) function map_cleanup(md) &
    bind(c, name = 'map_cleanup') result(errcode)

    implicit none
    type(moduleInfo), intent(in) :: md

    ! Do nothing
    errcode = 0

  end function map_cleanup

  ! 
  ! STEP 1: Initial conditions
  !

  ! Implementation of module_node_in()
  ! Here we read initial condition for all pixels
  integer (c_int) function map_master_in(mpi_size, node, md, d, ic) &
    bind(c, name = 'map_master_in') result(errcode)

    implicit none
    integer(c_int), intent(in), value :: mpi_size
    integer(c_int), intent(in), value :: node
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d
    type(masterData), intent(inout) :: ic
    integer :: ic_rank(1)
    real (c_double), pointer :: ic_array(:)

    ! Bind C arrays
    ic_rank = md%irl
    call c_f_pointer(ic%res, ic_array, ic_rank)

    ! Initial condition
    ! Here we assign some dummy values,
    ! but it is easy to read something from file
    ic_array(1) = 1.0d0
    ic_array(2) = 2.0d0
    ic_array(3) = 3.0d0

    errcode = 0
  end function map_master_in

  !
  ! STEP 2: Modify initial condition
  !

  ! Implementation of node_preparePixel()
  ! Each slave node will receive initial condition from master_in function.
  ! Here we can modify it according to the coordinates of the pixel or leave it
  ! empty and do this task in processPixel() function. 
  integer (c_int) function map_slave_preparePixel(node, md, d, ic, r) &
    bind(c, name = 'map_slave_preparePixel') result(errcode)

    implicit none
    integer(c_int), intent(in), value :: node
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d
    type(masterData), intent(inout) :: ic
    type(masterData), intent(inout) :: r

    ! Bind C arrays
    integer :: res_rank(1)
    integer :: ic_rank(1)

    integer (c_int) :: ic_coords(3)
    real (c_double), pointer :: ic_array(:)
    real (c_double), pointer :: res_array(:)
    
    ic_rank = md%irl
    res_rank = md%mrl

    call c_f_pointer(ic%res, ic_array, ic_rank)
    call c_f_pointer(r%res, res_array, res_rank)

    ! Do something here or leave empty and go to processPixel()

    ic_array(1) = ic_array(1) + ic%coords(1) 
    ic_array(2) = ic_array(2) + ic%coords(2) 
    ic_array(3) = ic_array(3) + ic%coords(3) 

    errcode = 0
  end function map_slave_preparePixel

  !
  ! STEP 3: Compute solution for given (modified) initial condition
  !

  ! Implementation of module_processPixel()
  integer (c_int) function map_processPixel(node, md, d, ic, r) &
    bind(c, name = 'map_processPixel') result(errcode)

    implicit none
    integer(c_int), intent(in), value :: node
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d
    type(masterData), intent(in) :: ic
    type(masterData), intent(inout) :: r

    ! Bind C arrays
    integer :: res_rank(1)
    integer :: ic_rank(1)

    integer (c_int) :: ic_coords(3)
    real (c_double), pointer :: ic_array(:)
    real (c_double), pointer :: res_array(:)
    
    ic_rank = md%irl
    res_rank = md%mrl

    call c_f_pointer(ic%res, ic_array, ic_rank)
    call c_f_pointer(r%res, res_array, res_rank)

    ! Do some computations here,
    ! i.e assign pixel coordinates and initial condition
    res_array(1) = ic%coords(1)
    res_array(2) = ic%coords(2)
    res_array(3) = ic%coords(3)

    res_array(4) = ic_array(1)
    res_array(5) = ic_array(2)
    res_array(6) = ic_array(3)

    errcode = 0
  end function map_processPixel

end module map
