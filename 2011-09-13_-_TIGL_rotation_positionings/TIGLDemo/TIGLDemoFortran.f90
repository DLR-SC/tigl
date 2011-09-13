! Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
!
! Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
! Changed: $Id: TIGLDemoFortran.f90 4416 2011-02-08 21:03:40Z litz_ma $ 
!
! Version: $Revision: 4416 $
!
! Licensed under the Apache License, Version 2.0 (the "License");
! you may not use this file except in compliance with the License.
! You may obtain a copy of the License at
!
!     http://www.apache.org/licenses/LICENSE-2.0
!
! Unless required by applicable law or agreed to in writing, software
! distributed under the License is distributed on an "AS IS" BASIS,
! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
! See the License for the specific language governing permissions and
! limitations under the License.
!****************************************************************************
!  TIGLDemo Fortran.f90 
!
!  FUNCTIONS:
!  TIGLDemo Fortran      - Demonstrates how to use the TIGL routines
!
!  PROGRAM: TIGLDemo Fortran
!
!  PURPOSE:  Demonstrates how to use the TIGL routines
!****************************************************************************

    program TIGLDemo_Fortran

    implicit none

!   Variables

    character*100 filename    /'../TestData/VFW-614.xml'/    
    character*100 export_name /'../TestData/VFW-614.iges'/    
    integer tixi_handle
    integer cpacs_handle
    integer return_code
    integer f_count
    integer s_count
    integer i_fuselage
    integer i_segment
    integer i_eta
    integer i_zeta
    integer i_section
    integer i_element
    real*8  eta
    real*8  zeta
    real*8  x, y, z
    integer start_segment_count
    integer end_segment_count
    integer n
    integer i_connected
    

!   Body of TIGLDemo Fortran
     
!   Open a CPACS configuration file. First open the CPACS-XML file
!   with TIXI to get a tixi handle and then use this handle to open
!   and read the CPACS configuration.
    call tixi_open_document( filename, tixi_handle, return_code )
    call check_error( return_code, 'tixi_open_document' )
    if ( return_code.ne.0 ) then
        stop "Fatal error when opening input document"
    endif
    
    call tigl_open_cpacs_configuration(tixi_handle, cpacs_handle, return_code)
    call check_error( return_code, 'tigl_open_cpacs_configuration' )
    if ( return_code.ne.0 ) then
        stop "Fatal error when reading CPACS file"
    endif

!    Set the wire algorithm to be used in interpolations. By default this is the
!    bspline interpolation algorithm. The algorithm is used globally for all
!    open CPACS configurations.
    call tigl_use_algorithm(1, return_code)
    call check_error( return_code, 'tigl_use_algorithm' )
    if ( return_code.ne.0 ) then
        stop "Fatal error when setting interpolation algorithm code"
    endif

!   Output some CPACS statistics.
    call tigl_get_fuselage_count(cpacs_handle, f_count, return_code)
    write (*, "('Number of fuselages: ', I6)") f_count

!   Loop over all fuselages and get the number of fuselage segments 
    do i_fuselage = 1, f_count
        call tigl_fuselage_get_segment_count(cpacs_handle, i_fuselage, s_count, return_code)
        write (*, "('Number of segments of fuselage number ', I6,': ', I6)") i_fuselage, s_count
    enddo
    
!   Loop over all fuselages and output some points.
    do i_fuselage = 1, f_count
        write (*, "('Fuselage number ', I6, ':')") i_fuselage
        call tigl_fuselage_get_segment_count(cpacs_handle, i_fuselage, s_count, return_code)
        do i_segment = 1, s_count
            write (*, "('  Segment number ', I6, ':')") i_segment
            do i_eta = 1, 3
                eta = 0.5D0 * ( i_eta - 1 )
                do i_zeta = 1, 3
                    zeta = 0.5D0 * (i_zeta - 1)
                    call tigl_fuselage_get_point(cpacs_handle, i_fuselage, i_segment, eta, zeta, x, y, z, return_code)
                    call check_error( return_code, 'tigl_fuselage_get_point' )
                    if ( return_code.eq.0 ) then
                        write (*, "('    eta = ',D15.6,' zeta = ',D15.6,' point = ',3(D15.6))") eta, zeta, x, y, z
                    endif
                enddo
            enddo
        enddo
    enddo

    write(*,*)

!   Loop over all fuselages and output for every segment the segments connected
!   to the start and end of the current segment.
    do i_fuselage = 1, f_count
        write (*, "('Fuselage number ', I6, ':')") i_fuselage
        call tigl_fuselage_get_segment_count(cpacs_handle, i_fuselage, s_count, return_code)
        do i_segment = 1, s_count
            write (*, "('  Segment number ', I6, ':')") i_segment
            
!           To the start section connected segments 
            call tigl_fuselage_get_start_connected_segment_count(cpacs_handle, i_fuselage, i_segment, start_segment_count, return_code)            
            write (*, "('    Count of to the start section connected segments: ', I6)") start_segment_count
            do n = 1, start_segment_count
                call tigl_fuselage_get_start_connected_segment_index(cpacs_handle, i_fuselage, i_segment, n, i_connected, return_code)
                write (*, "('    Index of the ', I6, '-th to the start section connected segment: ', I6)") n, i_connected
            enddo

!           To the end section connected segments 
            call tigl_fuselage_get_end_connected_segment_count(cpacs_handle, i_fuselage, i_segment, end_segment_count, return_code)            
            write (*, "('    Count of to the end section connected segments: ', I6)") end_segment_count
            do n = 1, end_segment_count
                call tigl_fuselage_get_end_connected_segment_index(cpacs_handle, i_fuselage, i_segment, n, i_connected, return_code)
                write (*, "('    Index of the ', I6, '-th to the end section connected segment: ', I6)") n, i_connected
            enddo

        enddo
    enddo

    write (*,*)
    
!   Loop over all fuselages and output for every segment the section and element indices.
    do i_fuselage = 1, f_count
        write (*, "('Fuselage number ', I6, ':')") i_fuselage
        call tigl_fuselage_get_segment_count(cpacs_handle, i_fuselage, s_count, return_code)
        do i_segment = 1, s_count
            write (*, "('  Segment number ', I6, ':')") i_segment
            
            call tigl_fuselage_get_start_section_and_element_index(cpacs_handle, i_fuselage, i_segment, i_section, i_element, return_code)
            write (*, "('    Start section index:', I6)") i_section            
            write (*, "('    Start element index:', I6)") i_element

            call tigl_fuselage_get_end_section_and_element_index(cpacs_handle, i_fuselage, i_segment, i_section, i_element, return_code)
            write (*, "('    End section index:', I6)") i_section            
            write (*, "('    End element index:', I6)") i_element

        enddo
    enddo

!   Cleanup: First(!) close the CPACS configuration and then the corresponding tixi file handle.
    call tigl_close_cpacs_configuration(cpacs_handle, return_code)
    call tixi_close_document(tixi_handle, return_code)
    
    end program TIGLDemo_Fortran

!****************************************************************************

    subroutine check_error( error, msg )

    implicit none

    character*(*) msg
    integer error

    if (error.ne.0) then
        write (*,*) 'Error ', error, ': ', msg
    endif
    
    end
