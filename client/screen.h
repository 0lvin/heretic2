// screen.h
#ifndef CLIENT_SCREEN_H
#define CLIENT_SCREEN_H
#include "../src/common/header/common.h"
#include "vid.h"

#ifdef __cplusplus
extern "C" {
#endif
void	SCR_Init (void);
void	SCR_UpdateScreen (void);
void	SCR_CenterPrint (char *str /*, PalIdx_t colour*/);
void	SCR_BeginLoadingPlaque (void);
void	SCR_EndLoadingPlaque (void);
void	SCR_TouchPics (void);
void	SCR_RunConsole (void);

extern	float		scr_con_current;
extern	float		scr_conlines;		// lines of console to display
extern	int			sb_lines;

extern cvar_t* scr_viewsize;
extern	cvar_t		*crosshair;

extern	vrect_t		scr_vrect;		// position of render window

void SCR_AddDirtyPoint (int x, int y);
void SCR_DirtyScreen (void);

//
// scr_cin.c
//
void SCR_PlayCinematic (char *name);
void SCR_RunCinematic (void);
void SCR_FinishCinematic (void);
void SCR_StopCinematic();
qboolean CIN_IsCinematicRunning(void);

#ifdef __cplusplus
} //end extern "C"
#endif


#endif