/*  SinView - Sin Pak File Viewer
    Copyright (C) 1998

    Trey Harrison trey@u.washington.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _MATRIX_ASM_H_
#define _MATRIX_ASM_H_

#ifdef USE_TRANSFORM_ASM
class matrix_class;

inline void trans_4x3(matrix_class *transforming_matrix, vec3 *src_vector, vec3 *dst_vector)
{
  _asm
  {
    mov     eax, transforming_matrix
    mov     ecx, src_vector
    mov     edx, dst_vector

    //compute src.x * matrix.x.x
    fld dword ptr  [ecx+0]     ;starts & ends on cycle 0
    fmul dword ptr [eax+0]     ;starts on cycle 1

    //compute src.x * matrix.x.y
    fld dword ptr  [ecx+0]     ;starts & ends on cycle 2
    fmul dword ptr [eax+4]     ;starts on cycle 3

    //compute src.x * matrix.x.z
    fld dword ptr  [ecx+0]     ;starts & ends on cycle 4
    fmul dword ptr [eax+8]     ;starts on cycle 5

    //compute src.y * matrix.y.x
    fld dword ptr  [ecx+4]     ;starts & ends on cycle 6
    fmul dword ptr [eax+16]    ;starts on cycle 7

    //compute src.y * matrix.y.y
    fld dword ptr  [ecx+4]     ;starts & ends on cycle 8
    fmul dword ptr [eax+20]    ;starts on cycle 9

    //compute src.y * matrix.y.z
    fld dword ptr  [ecx+4]     ;starts & ends on cycle 10
    fmul dword ptr [eax+24]    ;starts on cycle 11


    //add up those results
    fxch           st(2)       ;no cost
    faddp          st(5),st(0) ;starts on cycle 12
    faddp          st(3),st(0) ;starts on cycle 13
    faddp          st(1),st(0) ;starts on cycle 14

    //compute src.z * matrix.z.x
    fld dword ptr  [ecx+8]     ;starts & ends on cycle 15
    fmul dword ptr [eax+32]    ;starts on cycle 16

    //compute src.z * matrix.z.y
    fld dword ptr  [ecx+8]     ;starts & ends on cycle 17
    fmul dword ptr [eax+36]    ;starts on cycle 18

    //compute src.z * matrix.z.z
    fld dword ptr  [ecx+8]     ;starts & ends on cycle 19
    fmul dword ptr [eax+40]    ;starts on cycle 20

    //add up those results
    fxch           st(2)       ;no cost
    faddp          st(5),st(0) ;starts on cycle 21
    faddp          st(3),st(0) ;starts on cycle 22
    faddp          st(1),st(0) ;starts on cycle 23
    fxch           st(2)       ;no cost

    //add in matrix.t.x
    fadd dword ptr [eax+48]    ;starts on cycle 24
    fxch           st(1)       ;starts on cycle 25

    //add in matrix.t.y
    fadd dword ptr [eax+52]    ;starts on cycle 26
    fxch           st(2)       ;no cost

    //add in matrix.t.z
    fadd dword ptr [eax+56]    ;starts on cycle 27
    fxch           st(1)       ;no cost

    //store the results in dst
    fstp dword ptr [edx+0] ;starts on cycle 28, ends on cycle 29 //store x
    fstp dword ptr [edx+8] ;starts on cycle 30, ends on cycle 31 //store y
    fstp dword ptr [edx+4] ;starts on cycle 32, ends on cycle 33 //store z
  }
}

inline void trans_3x3(matrix_class *transforming_matrix, vec3 *src_vector, vec3 *dst_vector)
{
  _asm
  {
    mov     eax, transforming_matrix
    mov     ecx, src_vector
    mov     edx, dst_vector

    //compute src.x * matrix.x.x
    fld dword ptr  [ecx+0]     ;starts & ends on cycle 0
    fmul dword ptr [eax+0]     ;starts on cycle 1

    //compute src.x * matrix.x.y
    fld dword ptr  [ecx+0]     ;starts & ends on cycle 2
    fmul dword ptr [eax+4]     ;starts on cycle 3

    //compute src.x * matrix.x.z
    fld dword ptr  [ecx+0]     ;starts & ends on cycle 4
    fmul dword ptr [eax+8]     ;starts on cycle 5

    //compute src.y * matrix.y.x
    fld dword ptr  [ecx+4]     ;starts & ends on cycle 6
    fmul dword ptr [eax+16]    ;starts on cycle 7

    //compute src.y * matrix.y.y
    fld dword ptr  [ecx+4]     ;starts & ends on cycle 8
    fmul dword ptr [eax+20]    ;starts on cycle 9

    //compute src.y * matrix.y.z
    fld dword ptr  [ecx+4]     ;starts & ends on cycle 10
    fmul dword ptr [eax+24]    ;starts on cycle 11

    //add up those results
    fxch           st(2)       ;no cost
    faddp          st(5),st(0) ;starts on cycle 12
    faddp          st(3),st(0) ;starts on cycle 13
    faddp          st(1),st(0) ;starts on cycle 14

    //compute src.z * matrix.z.x
    fld dword ptr  [ecx+8]     ;starts & ends on cycle 15
    fmul dword ptr [eax+32]    ;starts on cycle 16

    //compute src.z * matrix.z.y
    fld dword ptr  [ecx+8]     ;starts & ends on cycle 17
    fmul dword ptr [eax+36]    ;starts on cycle 18

    //compute src.z * matrix.z.z
    fld dword ptr  [ecx+8]     ;starts & ends on cycle 19
    fmul dword ptr [eax+40]    ;starts on cycle 20

    //add up those results
    fxch           st(2)       ;no cost
    faddp          st(5),st(0) ;starts on cycle 21
    faddp          st(3),st(0) ;starts on cycle 22
    faddp          st(1),st(0) ;starts on cycle 23
    fxch           st(2)       ;no cost

    fstp dword ptr [edx+0] //store x
    fstp dword ptr [edx+4] //store y
    fstp dword ptr [edx+8] //store z
  }
}
#endif //#ifdef USE_TRANSFORM_ASM

#endif