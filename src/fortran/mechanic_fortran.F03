!
! FORTRAN 2003 BINDINGS FOR MECHANIC
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

! [F2003BIND_REF]
!
! @idoc modules/modules_fortran/fhello/mechanic_module_fhello.f03 F2003BIND
!
! @subsection f2003ref Fortran 2003 bindings reference
!
! The Fortran 2003 bindings provide the same API as C headers, as follows:
! - Error codes
!   @code
!   @icode fortran/mechanic_fortran.f03 F2003BIND_ERR
!   @endcode
! - ModuleInfo structure
!   @code
!   @icode fortran/mechanic_fortran.f03 F2003BIND_MD
!   @endcode
! - ConfigData structure
!   @code
!   @icode fortran/mechanic_fortran.f03 F2003BIND_CONFIG
!   @endcode
! - MasterData structure
!   @code
!   @icode fortran/mechanic_fortran.f03 F2003BIND_DATA
!   @endcode
! - API functions
!   @code
!   @icode fortran/mechanic_fortran.f03 F2003BIND_API
!   @endcode
!
! [/F2003BIND_REF]

module mechanic_fortran

  use iso_c_binding
  use mpi
  use hdf5
 
  ! [F2003BIND_ERR]
  integer, parameter :: MECHANIC_ERR_MPI_F = 911
  integer, parameter :: MECHANIC_ERR_HDF_F = 912
  integer, parameter :: MECHANIC_ERR_MODULE_F = 913
  integer, parameter :: MECHANIC_ERR_SETUP_F = 914
  integer, parameter :: MECHANIC_ERR_MEM_F = 915
  integer, parameter :: MECHANIC_ERR_CHECKPOINT_F = 916
  integer, parameter :: MECHANIC_ERR_OTHER_F = 999
  ! [/F2003BIND_ERR]

  ! [F2003BIND_MODERR]
  integer, parameter :: MECHANIC_MODULE_ERR_MPI_F = 811
  integer, parameter :: MECHANIC_MODULE_ERR_HDF_F = 812
  integer, parameter :: MECHANIC_MODULE_ERR_MODULE_F = 813
  integer, parameter :: MECHANIC_MODULE_ERR_SETUP_F = 814
  integer, parameter :: MECHANIC_MODULE_ERR_MEM_F = 815
  integer, parameter :: MECHANIC_MODULE_ERR_CHECKPOINT_F = 816
  integer, parameter :: MECHANIC_MODULE_ERR_OTHER_F = 888
  ! [/F2003BIND_MODERR]

  integer, parameter :: MECHANIC_HDF_RANK_F = 2
  integer, parameter :: MECHANIC_STRLEN_F = 256
   
  ! [F2003BIND_MSG]
  enum, bind(c) 
    enumerator MECHANIC_MESSAGE_INFO_F
    enumerator MECHANIC_MESSAGE_ERR_F
    enumerator MECHANIC_MESSAGE_IERR_F
    enumerator MECHANIC_MESSAGE_CONT_F
    enumerator MECHANIC_MESSAGE_WARN_F
    enumerator MECHANIC_MESSAGE_DEBUG_F
  end enum 
  ! [/F2003BIND_MSG]

  ! [F2003BIND_MD]
  type, bind(c) :: moduleInfo
    integer (c_int) :: irl
    integer (c_int) :: mrl
    integer (c_int) :: api
  end type moduleInfo
  ! [/F2003BIND_MD]

  ! [F2003BIND_CONFIG]
  type, bind(c) :: configData
    character(kind = c_char, len = MECHANIC_STRLEN_F) :: p_name 
    character(kind = c_char, len = MECHANIC_STRLEN_F) :: datafile 
    character(kind = c_char, len = MECHANIC_STRLEN_F) :: u_module
    integer (c_int) :: p_name_len
    integer (c_int) :: datafile_len
    integer (c_int) :: u_module_len
    integer (c_int) :: xres
    integer (c_int) :: yres
    integer (c_int) :: checkpoint
    integer (c_int) :: restartmode
    integer (c_int) :: mode
  end type configData
  ! [/F2003BIND_CONFIG]

  ! [F2003BIND_DATA]
  type, bind(c) :: masterData
    type (c_ptr) :: res
    integer (c_int) :: coords(3)
  end type masterData
  ! [/F2003BIND_DATA]

  contains

  ! API functions

  ! [F2003BIND_API]
  ! subroutine mechanic_finalize_f(node)
  ! subroutine mechanic_abort_f(errcode)
  ! subroutine mechanic_error_f(stat)
  ! [/F2003BIND_API]

  ! Implementation of mechanic_finalize()
  subroutine mechanic_finalize_f(node)
    use iso_c_binding
    interface
      integer (c_int) function mechanic_finalize(node) bind(c) result(mstat)
        use iso_c_binding
        integer (c_int) :: node
      end function mechanic_finalize
    end interface

    integer(c_int) :: node, mstat
    mstat = mechanic_finalize(node)
  end subroutine mechanic_finalize_f

  ! Implementation of mechanic_abort()
  subroutine mechanic_abort_f(errcode)
    use iso_c_binding
    interface
      integer (c_int) function mechanic_abort(errcode) bind(c) result(mstat)
        use iso_c_binding
        integer (c_int) :: errcode
      end function mechanic_abort
    end interface

    integer(c_int) :: errcode, mstat
    mstat = mechanic_abort(errcode)
  end subroutine mechanic_abort_f

  ! Implementation of mechanic_error()
  subroutine mechanic_error_f(stat)
    use iso_c_binding
    interface
      subroutine mechanic_error(stat) bind(c)
        use iso_c_binding
        integer (c_int) :: stat
      end subroutine mechanic_error
    end interface

    integer(c_int) :: stat
    call mechanic_error(stat)
  end subroutine mechanic_error_f

  ! Implementation of mechanic_message()
  ! subroutine mechanic_message_f(message_type, message_fmt, ...)
  !  use iso_c_binding
  !  interface
  !    subroutine mechanic_message(message_type, message_fmt, ...) bind(c)
  !      integer (c_int) :: message_type
  !
  !    end subroutine mechanic_message
  !  end interface
  ! end subroutine

end module mechanic_fortran

