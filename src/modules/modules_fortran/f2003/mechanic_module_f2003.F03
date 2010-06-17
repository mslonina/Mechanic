!
! Full F2003 template module for MECHANIC
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

module f2003

  use iso_c_binding
  use mechanic_fortran

contains

  ! Implementation of module_init()
  integer (c_int) function f2003_init(md) &
    bind(c, name = 'f2003_init') result(errcode)

    implicit none
    type(moduleInfo), intent(inout) :: md

    md%irl = 3 ! Initial condition length
    md%mrl = 6 ! Result length

    errcode = 0
  end function f2003_init 

  ! Implementation of module_cleanup()
  integer (c_int) function f2003_cleanup(md) &
    bind(c, name = 'f2003_cleanup') result(errcode)

    implicit none
    type(moduleInfo), intent(in) :: md

    ! Do nothing
    errcode = 0

  end function f2003_cleanup

  ! Implementation of module_query()
  integer (c_int) function f2003_query(md) &
    bind(c, name = 'f2003_query') result(errcode)

    implicit none
    type(moduleInfo), intent(in) :: md

    errcode = 0

  end function f2003_query

  ! Implementation of module_farmResolution() (method 6)
  integer (c_int) function f2003_farmResolution(x, y, md, d) &
    bind(c, name = 'f2003_farmResolution' result(resolution)

    implicit none
    integer (c_int), intent(in), value :: x
    integer (c_int), intent(in), value :: y
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d

    resolution = x * y

  end function f2003_farmResolution

  ! Implementation of module_pixelCoordsMap() (method 6) [TODO]
  integer (c_int) function f2003_pixelCoordsMap(t, numofpx, xres, yres, md, d) &
    bind(c, name = 'f2003_pixelCoordsMap') result(errcode)

    implicit none
    integer (c_int), intent(in), value :: numofpx
    integer (c_int), intent(in), value :: xres
    integer (c_int), intent(in), value :: yres
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d

    errcode = 0
  end function f2003_pixelCoordsMap

  ! Implementation of module_pixelCoords() [TODO]

  ! Implementation of module_node_preparePixel() 
  integer (c_int) function f2003_node_preparePixel(node, md, d, ic, r) &
    bind(c, name = 'f2003_node_preparePixel') result(errcode)

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

    errcode = 0
  end function f2003_node_preparePixel

  ! Implementation of module_node_beforeProcessPixel()
  integer (c_int) function f2003_node_beforeProcessPixel(node, md, d, ic, r) &
    bind(c, name = 'f2003_node_beforeProcessPixel') result(errcode)

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

    errcode = 0

  end function f2003_node_beforeProcessPixel

  ! Implementation of module_node_afterProcessPixel()
  integer (c_int) function f2003_node_afterProcessPixel(node, md, d, ic, r) &
    bind(c, name = 'f2003_node_afterProcessPixel') result(errcode)

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

    errcode = 0

  end function f2003_node_afterProcessPixel

  ! Implementation of module_node_in()
  integer (c_int) function f2003_node_in(mpi_size, node, md, d, ic) &
    bind(c, name = 'f2003_node_in') result(errcode)
    
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
    
    errcode = 0
  end function f2003_master_in

  ! Implementation of module_node_out()
  integer (c_int) function f2003_node_out(mpi_size, node, md, d, ic, r) &
    bind(c, name = 'f2003_node_out') result(errcode)

    implicit none
    integer(c_int), intent(in), value :: mpi_size
    integer(c_int), intent(in), value :: node
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d
    type(masterData), intent(inout) :: ic
    type(masterData), intent(inout) :: r
    integer :: ic_rank(1)
    integer :: r_rank(1)
    real (c_double), pointer :: ic_array(:)
    real (c_double), pointer :: r_array(:)

    ! Bind C arrays
    ic_rank = md%irl
    call c_f_pointer(ic%res, ic_array, ic_rank)

    r_rank = md%mrl
    call c_f_pointer(r%res, r_array, r_rank)

    errcode = 0
  end function f2003_node_out

  ! Implementation of module_node_beforeSend()
  integer (c_int) function f2003_node_beforeSend(node, md, d, ic, r) &
    bind(c, name = 'f2003_node_beforeSend') result(errcode)

    implicit none
    integer(c_int), intent(in), value :: node
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d
    type(masterData), intent(inout) :: ic
    type(masterData), intent(inout) :: r
    integer :: ic_rank(1)
    integer :: r_rank(1)
    real (c_double), pointer :: ic_array(:)
    real (c_double), pointer :: r_array(:)

    ! Bind C arrays
    ic_rank = md%irl
    call c_f_pointer(ic%res, ic_array, ic_rank)

    r_rank = md%mrl
    call c_f_pointer(r%res, r_array, r_rank)

    errcode = 0

  end function f2003_node_beforeSend

  ! Implementation of module_node_afterSend()
  integer (c_int) function f2003_node_afterSend(node, md, d, ic, r) &
    bind(c, name = 'f2003_node_afterSend') result(errcode)

    implicit none
    integer(c_int), intent(in), value :: node
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d
    type(masterData), intent(inout) :: ic
    type(masterData), intent(inout) :: r
    integer :: ic_rank(1)
    integer :: r_rank(1)
    real (c_double), pointer :: ic_array(:)
    real (c_double), pointer :: r_array(:)

    ! Bind C arrays
    ic_rank = md%irl
    call c_f_pointer(ic%res, ic_array, ic_rank)

    r_rank = md%mrl
    call c_f_pointer(r%res, r_array, r_rank)

    errcode = 0

  end function f2003_node_afterSend

  ! Implementation of module_node_beforeReceive()
  integer (c_int) function f2003_node_beforeReceive(node, md, d, ic, r) &
    bind(c, name = 'f2003_node_beforeReceive') result(errcode)

    implicit none
    integer(c_int), intent(in), value :: node
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d
    type(masterData), intent(inout) :: ic
    type(masterData), intent(inout) :: r
    integer :: ic_rank(1)
    integer :: r_rank(1)
    real (c_double), pointer :: ic_array(:)
    real (c_double), pointer :: r_array(:)

    ! Bind C arrays
    ic_rank = md%irl
    call c_f_pointer(ic%res, ic_array, ic_rank)

    r_rank = md%mrl
    call c_f_pointer(r%res, r_array, r_rank)

    errcode = 0

  end function f2003_node_beforeReceive

  ! Implementation of module_node_afterReceive()
  integer (c_int) function f2003_node_afterReceive(node, md, d, ic, r) &
    bind(c, name = 'f2003_node_afterReceive') result(errcode)

    implicit none
    integer(c_int), intent(in), value :: node
    type(moduleInfo), intent(in) :: md
    type(configData), intent(in) :: d
    type(masterData), intent(inout) :: ic
    type(masterData), intent(inout) :: r
    integer :: ic_rank(1)
    integer :: r_rank(1)
    real (c_double), pointer :: ic_array(:)
    real (c_double), pointer :: r_array(:)

    ! Bind C arrays
    ic_rank = md%irl
    call c_f_pointer(ic%res, ic_array, ic_rank)

    r_rank = md%mrl
    call c_f_pointer(r%res, r_array, r_rank)

    errcode = 0

  end function f2003_node_afterReceive

  ! Implementation of module_node_in()
  integer (c_int) function f2003_master_in(mpi_size, node, md, d, ic) &
    bind(c, name = 'f2003_master_in') result(errcode)

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
  end function f2003_master_in

  ! Implementation of node_preparePixel()
  integer (c_int) function f2003_slave_preparePixel(node, md, d, ic, r) &
    bind(c, name = 'f2003_slave_preparePixel') result(errcode)

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
  end function f2003_slave_preparePixel

  ! Implementation of module_processPixel()
  integer (c_int) function f2003_processPixel(node, md, d, ic, r) &
    bind(c, name = 'f2003_processPixel') result(errcode)

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
  end function f2003_processPixel

end module f2003
