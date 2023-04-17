#ifndef CLIENT_INPUT_H
#define CLIENT_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

// input.h -- external (non-keyboard) input devices

void IN_Init (void);

void IN_Shutdown (void);

void IN_Commands (void);
// oportunity for devices to stick commands on the script buffer

void IN_Frame (void);

void IN_Move (usercmd_t *cmd);
// add additional movement on top of the keyboard move cmd

void IN_Activate (qboolean active);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif