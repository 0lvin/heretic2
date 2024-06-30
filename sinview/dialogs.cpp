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
#include "dialogs.h"
#include "resource.h"

extern "C" {

BOOL CALLBACK AboutDlgProc(HWND hDlg,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
	switch (iMsg) {

		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			switch(wParam) {
				case ST_DIALOG_ABOUT_OK:
					EndDialog(hDlg,0);
					return TRUE;
			}
			break;

		case WM_SYSCOMMAND:
			switch (wParam) {
				case SC_CLOSE:
					EndDialog(hDlg,0);
					return TRUE;
			}
			break;
	}
	return FALSE;
}

}