
//==============================================================================
//
// m_victimSsitra_anim.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "g_local.h"

#pragma hdrstop("g_local.pch")
// PRECOMPILED HEADER ABOVE
// WARNING:  DO NOT CHANGE THE ABOVE HEADERS OR THE PRECOMPILED STUFF WILL BREAK!  
// ADD ANY ADDITIONAL FILES BELOW

#include "c_ssithrascout_anim.h"
#include "c_ssithrascout.h"
#include "c_ai.h"


/************************************************************************
/************************************************************************
// 
//  Cinematic Frames
// 
/************************************************************************
/*************************************************************************/

/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action1[] =
{
	FRAME_ss_recover1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover27, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover28, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover29, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover30, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover31, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover32, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover33, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover34, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover35, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover36, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover37, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover38, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_recover39, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action1 = { 39, scout_frames_c_action1, ai_c_cycleend};


/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action2[] =
{
	FRAME_ss_hecured1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured27, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured28, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured29, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured30, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured31, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured32, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured33, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured34, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured35, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured36, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured37, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured38, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured39, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured40, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured41, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured42, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured43, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured44, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured45, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured46, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured47, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured48, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured49, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured50, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured51, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured52, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured53, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured54, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured55, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured56, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured57, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured58, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured59, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_hecured60, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action2 = { 60, scout_frames_c_action2, ai_c_cycleend};



/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action3[] =
{
	FRAME_ss_talk1A1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A27, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A28, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A29, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A30, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A31, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A32, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A33, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A34, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A35, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A36, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A37, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A38, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A39, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A40, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A41, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A42, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A43, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A44, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A45, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A46, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A47, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A48, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A49, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A50, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A51, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A52, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A53, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A54, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A55, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A56, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A57, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A58, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A59, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A60, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A61, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A62, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A63, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A64, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A65, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A66, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A67, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A68, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A69, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A70, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A71, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A72, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A73, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A74, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A75, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A76, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A77, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A78, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A79, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A80, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A81, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A82, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A83, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A84, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A85, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1A86, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action3 = { 86, scout_frames_c_action3, ai_c_cycleend};



/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action4[] =
{
	FRAME_ss_talk1B1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B27, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B28, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B29, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B30, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B31, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B32, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B33, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B34, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B35, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B36, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B37, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B38, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B39, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B40, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B41, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B42, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B43, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk1B44, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action4 = { 44, scout_frames_c_action4, ai_c_cycleend};


/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action5[] =
{
	FRAME_ss_coughA1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughA26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action5 = { 26, scout_frames_c_action5, ai_c_cycleend};



/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action6[] =
{
	FRAME_ss_noshe1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe27, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe28, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe29, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe30, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe31, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe32, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe33, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_noshe34, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action6 = { 34, scout_frames_c_action6, ai_c_cycleend};



/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action7[] =
{
	FRAME_ss_talk2A1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A27, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A28, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A29, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A30, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A31, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A32, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A33, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A34, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A35, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A36, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A37, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A38, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A39, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A40, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A41, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A42, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A43, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A44, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A45, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A46, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A47, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A48, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A49, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A50, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A51, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A52, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A53, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A54, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A55, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A56, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A57, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A58, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A59, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A60, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A61, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A62, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A63, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A64, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A65, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A66, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A67, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A68, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A69, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2A70, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action7 = { 70, scout_frames_c_action7, ai_c_cycleend};



/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action8[] =
{
	FRAME_ss_talk2B1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B27, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B28, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B29, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B30, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B31, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B32, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B33, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B34, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B35, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B36, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B37, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B38, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B39, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B40, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B41, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B42, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B43, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B44, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B45, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B46, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B47, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B48, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B49, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B50, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B51, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B52, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B53, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B54, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B55, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B56, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B57, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B58, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B59, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B60, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B61, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2B62, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action8 = { 62, scout_frames_c_action8, ai_c_cycleend};



/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action9[] =
{
	FRAME_ss_talk2C1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_talk2C25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action9 = { 25, scout_frames_c_action9, ai_c_cycleend};



/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_action10[] =
{
	FRAME_ss_coughB1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_coughB19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_action10 = { 19, scout_frames_c_action10, ai_c_cycleend};



/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_death1[] =
{
	FRAME_ss_death1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death27, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death28, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death29, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death30, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death31, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death32, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death33, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death34, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death35, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death36, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death37, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death38, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death39, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death40, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death41, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death42, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death43, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death44, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death45, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death46, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death47, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death48, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death49, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death50, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death51, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death52, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death53, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_death54, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_death1 = { 54, scout_frames_c_death1, ai_c_cycleend};


/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_idle1 [] =
{
	FRAME_ss_breath1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath26, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath27, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath28, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath29, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath30, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_breath31, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_idle1 = { 31, scout_frames_c_idle1, ai_c_cycleend};


/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_idle2 [] =
{
	FRAME_ss_idleA1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleA16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_idle2 = { 16, scout_frames_c_idle2, ai_c_cycleend};



/*----------------------------------------------------------------------
  Ssithra Scout - 
-----------------------------------------------------------------------*/
animframe_t scout_frames_c_idle3 [] =
{
	FRAME_ss_idleB1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB20, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB21, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB22, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB23, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB24, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ss_idleB25, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t scout_move_c_idle3 = { 25, scout_frames_c_idle3, ai_c_cycleend};



