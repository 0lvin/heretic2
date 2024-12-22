/*
    QShed <http://www.icculus.org/qshed>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

unsigned short SwapShort(unsigned short v);
unsigned int SwapLong(unsigned int v);
float SwapFloat(float v);
bool_t yesno(void);
FILE *openfile_write(const char *filename, char **out_error);

#endif
