#ifndef QCOMMON_CDADIO_H
#define QCOMMON_CDADIO_H

#include "qcommon.h"
#ifdef __cplusplus
extern "C" {
#endif
int		CDAudio_Init(void);
void	CDAudio_Shutdown(void);
void	CDAudio_Play(int track, qboolean looping);
void	CDAudio_Stop(void);
void	CDAudio_Update(void);
void	CDAudio_Activate (qboolean active);
#ifdef __cplusplus
} //end extern "C"
#endif

#endif