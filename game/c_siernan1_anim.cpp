//==============================================================================
//
// m_corvus_anim.c
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

#include "c_siernan1_anim.h"
#include "c_siernan1.h"

#include "g_monster.h"
#include "c_ai.h"


/************************************************************************
/************************************************************************
// 
//  Cinematic Frames
// 
/************************************************************************
/*************************************************************************/
/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action1 [] =
{
	FRAME_A1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_A21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action1 = {21, siernan1_frames_c_action1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action2 [] =
{
	FRAME_B1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_B20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action2 = {20, siernan1_frames_c_action2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action3 [] =
{
	FRAME_C1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_C16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action3 = {16, siernan1_frames_c_action3, ai_c_cycleend};


/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action4 [] =
{
	FRAME_D1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_D11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action4 = {11, siernan1_frames_c_action4, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action5 [] =
{
	FRAME_E1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E23,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E24,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E25,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E26,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E27,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E28,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E29,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E30,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E31,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E32,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E33,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E34,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E35,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E36,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_E37,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action5 = {37, siernan1_frames_c_action5, ai_c_cycleend};



/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action6 [] =
{
	FRAME_uhave1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave23,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave24,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave25,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave26,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave27,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave28,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave29,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave30,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave31,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave32,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave33,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave34,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave35,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave36,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave37,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave38,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave39,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave40,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave41,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave42,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave43,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave44,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave45,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave46,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave47,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave48,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave49,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave50,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave51,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave52,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave53,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave54,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave55,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave56,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave57,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave58,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave59,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave60,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave61,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave62,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave63,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave64,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave65,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave66,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave67,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave68,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave69,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave70,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave71,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave72,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave73,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave74,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave75,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave76,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave77,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave78,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave79,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave80,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave81,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave82,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave83,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave84,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave85,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave86,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave87,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave88,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave89,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave90,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave91,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave92,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave93,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave94,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave95,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave96,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave97,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave98,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave99,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave100, ai_c_move, 0, 0, 0, NULL, 0, NULL,

	FRAME_uhave101,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave102,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave103,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave104,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave105,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave106,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave107,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave108,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave109,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave110,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave111,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave112,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave113,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave114,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave115,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave116,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave117,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave118,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave119,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave120,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave121,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave122,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave123,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave124,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave125,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave126,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave127,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave128,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave129,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave130,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave131,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave132,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave133,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave134,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave135,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_uhave136,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action6 = {136, siernan1_frames_c_action6, ai_c_cycleend};


/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action7 [] =
{
	FRAME_rember1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember23,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember24,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember25,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember26,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember27,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember28,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember29,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember30,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember31,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember32,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember33,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember34,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember35,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember36,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember37,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember38,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember39,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember40,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember41,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember42,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember43,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember44,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember45,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember46,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember47,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember48,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember49,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember50,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember51,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember52,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember53,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember54,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember55,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember56,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember57,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember58,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember59,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember60,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember61,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember62,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember63,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember64,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember65,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember66,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember67,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rember68,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action7 = {68, siernan1_frames_c_action7, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action8 [] =
{
	FRAME_noknow1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow23,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow24,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow25,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow26,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow27,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow28,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow29,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow30,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow31,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow32,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow33,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow34,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow35,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow36,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow37,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow38,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow39,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow40,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow41,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow42,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow43,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow44,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow45,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow46,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow47,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow48,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow49,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow50,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow51,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow52,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow53,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow54,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow55,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow56,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow57,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow58,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow59,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow60,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow61,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow62,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow63,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow64,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow65,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow66,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow67,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow68,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow69,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow70,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow71,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow72,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow73,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow74,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow75,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow76,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow77,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow78,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow79,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow80,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_noknow81,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action8 = {81, siernan1_frames_c_action8, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action9 [] =
{
	FRAME_ifear1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear23,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear24,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear25,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear26,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear27,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear28,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear29,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear30,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear31,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear32,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear33,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear34,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear35,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear36,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear37,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear38,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear39,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear40,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear41,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear42,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear43,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear44,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear45,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear46,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear47,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear48,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear49,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear50,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear51,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear52,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear53,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear54,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear55,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear56,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear57,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear58,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear59,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear60,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear61,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear62,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear63,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear64,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear65,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear66,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear67,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear68,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear69,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear70,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear71,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear72,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear73,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear74,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear75,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear76,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear77,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear78,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear79,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear80,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear81,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear82,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear83,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear84,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear85,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear86,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear87,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear88,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear89,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear90,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear91,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear92,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear93,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear94,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear95,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear96,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear97,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear98,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear99,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear100, ai_c_move, 0, 0, 0, NULL, 0, NULL,

	FRAME_ifear101,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear102,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear103,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear104,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear105,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear106,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear107,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear108,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear109,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear110,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear111,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear112,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear113,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear114,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear115,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear116,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ifear117,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action9 = {117, siernan1_frames_c_action9, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action10 [] =
{
	FRAME_notin1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin23,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin24,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin25,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin26,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin27,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin28,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin29,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin30,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin31,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin32,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin33,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin34,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin35,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin36,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin37,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin38,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin39,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin40,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin41,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin42,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin43,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin44,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin45,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin46,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin47,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin48,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin49,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin50,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin51,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin52,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin53,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin54,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin55,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin56,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin57,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin58,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin59,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin60,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin61,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin62,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin63,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin64,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin65,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin66,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin67,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin68,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin69,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin70,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin71,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin72,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin73,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin74,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin75,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin76,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin77,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin78,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin79,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin80,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin81,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin82,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin83,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin84,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin85,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin86,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin87,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin88,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin89,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin90,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin91,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin92,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin93,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin94,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin95,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin96,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin97,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin98,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin99,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin100, ai_c_move, 0, 0, 0, NULL, 0, NULL,

	FRAME_notin101,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin102,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin103,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin104,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin105,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin106,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin107,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin108,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin109,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin110,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin111,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin112,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin113,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin114,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin115,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin116,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin117,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin118,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin119,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin120,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin121,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin122,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin123,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin124,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin125,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin126,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin127,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin128,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin129,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin130,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin131,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin132,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin133,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin134,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin135,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin136,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin137,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin138,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin139,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin140,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin141,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin142,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_notin143,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action10 = {143, siernan1_frames_c_action10, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action11 [] =
{
	FRAME_yes1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes23,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes24,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes25,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes26,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes27,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes28,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes29,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes30,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes31,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes32,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes33,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes34,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes35,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes36,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes37,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes38,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes39,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes40,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes41,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes42,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes43,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes44,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes45,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes46,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes47,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes48,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes49,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes50,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes51,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes52,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes53,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes54,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes55,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes56,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes57,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes58,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes59,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes60,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes61,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes62,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes63,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes64,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes65,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes66,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes67,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes68,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes69,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes70,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes71,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes72,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes73,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes74,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes75,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes76,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes77,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes78,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes79,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes80,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes81,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes82,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes83,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes84,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes85,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes86,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes87,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes88,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes89,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes90,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes91,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes92,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes93,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes94,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes95,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes96,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes97,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes98,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes99,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_yes100,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action11 = {100, siernan1_frames_c_action11, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action12 [] =
{
	FRAME_siping1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action12 = {22, siernan1_frames_c_action12, ai_c_cycleend};


/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_action13 [] =
{
	FRAME_accept1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_action13 = {9, siernan1_frames_c_action13, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_idle1 [] =
{
	FRAME_Idle1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle20, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle21, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle22, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle23, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle24, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t siernan1_move_c_idle1 = {24, siernan1_frames_c_idle1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_idle2 [] =
{
	FRAME_Idle24, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Idle24, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t siernan1_move_c_idle2 = {2, siernan1_frames_c_idle2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_idle3 [] =
{
	FRAME_accept1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_idle3 = {2, siernan1_frames_c_idle3, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_idle4 [] =
{
	FRAME_accept9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_accept9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_idle4 = {2, siernan1_frames_c_idle4, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_idle5 [] =
{
	FRAME_siping22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_siping22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan1_move_c_idle5 = {2, siernan1_frames_c_idle5, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_walk1 [] =
{
	FRAME_walk1, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk2, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk3, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk4, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk5, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk6, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk7, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk8, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk9, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk10, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk11, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
	FRAME_walk12, ai_c_move, 2, 0, 0,  NULL, 0, NULL, 
};

animmove_t siernan1_move_c_walk1 = {12, siernan1_frames_c_walk1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_walkstart [] =
{
	FRAME_strtwalk1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_strtwalk2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_strtwalk3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_strtwalk4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_strtwalk5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_strtwalk6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t siernan1_move_c_walkstart = {6, siernan1_frames_c_walkstart, ai_c_cycleend};


/*----------------------------------------------------------------------
  Siernan 
-----------------------------------------------------------------------*/
animframe_t siernan1_frames_c_walkstop1 [] =
{
	FRAME_stpwalk1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_stpwalk2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_stpwalk3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_stpwalk4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_stpwalk5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t siernan1_move_c_walkstop1 = {5, siernan1_frames_c_walkstop1, ai_c_cycleend};


