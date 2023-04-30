//==============================================================================
//
// m_corvus_anim.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "../src/common/header/common.h"
#include "g_local.h"

#include "c_siernan2_anim.h"
#include "c_siernan2.h"

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
animframe_t siernan2_frames_c_action1 [] =
{
	FRAME_Minions1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions23,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions24,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions25,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions26,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions27,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions28,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions29,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions30,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions31,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions32,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions33,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions34,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions35,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions36,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions37,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions38,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions39,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions40,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions41,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions42,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions43,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions44,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions45,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions46,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions47,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions48,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions49,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions50,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions51,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions52,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions53,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions54,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions55,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions56,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions57,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions58,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions59,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions60,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions61,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions62,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions63,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions64,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions65,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions66,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions67,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions68,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions69,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions70,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions71,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions72,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions73,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions74,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions75,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions76,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions77,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions78,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions79,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions80,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions81,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions82,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions83,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions84,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions85,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions86,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions87,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions88,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions89,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions90,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions91,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions92,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions93,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions94,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions95,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions96,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions97,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions98,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions99,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions100, ai_c_move, 0, 0, 0, NULL, 0, NULL,

	FRAME_Minions101,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions102,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions103,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions104,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions105,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions106,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions107,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions108,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions109,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions110,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions111,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions112,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions113,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions114,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions115,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions116,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions117,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions118,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions119,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions120,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions121,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions122,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions123,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions124,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions125,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions126,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions127,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions128,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions129,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions130,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan2_move_c_action1 = {130, siernan2_frames_c_action1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan
-----------------------------------------------------------------------*/
animframe_t siernan2_frames_c_action2 [] =
{
	FRAME_Minions1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions10,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions11,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions12,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions13,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions14,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions15,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions16,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions17,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions18,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions19,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions20,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions21,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions22,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions23,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions24,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions25,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions26,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions27,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions28,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions29,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions30,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions31,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions32,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions33,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions34,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions35,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions36,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions37,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions38,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions39,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions40,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions41,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions42,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions43,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions44,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions45,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions46,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions47,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions48,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions49,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions50,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions51,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions52,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions53,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions54,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions55,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions56,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions57,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions58,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions59,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions60,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions61,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions62,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions63,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions64,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions65,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions66,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions67,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions68,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions69,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions70,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions71,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions72,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions73,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions74,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions75,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions76,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions77,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions78,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions79,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions80,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions81,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions82,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions83,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions84,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions85,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions86,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions87,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions88,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions89,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions90,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions91,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions92,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions93,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions94,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions95,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions96,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions97,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions98,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions99,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions100, ai_c_move, 0, 0, 0, NULL, 0, NULL,

	FRAME_Minions101,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions102,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions103,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions104,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions105,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions106,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions107,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions108,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions109,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions110,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions111,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions112,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions113,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions114,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions115,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions116,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions117,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions118,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions119,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions120,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan2_move_c_action2 = {120, siernan2_frames_c_action2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Siernan
-----------------------------------------------------------------------*/
animframe_t siernan2_frames_c_idle1 [] =
{
	FRAME_Minions1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Minions1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};

animmove_t siernan2_move_c_idle1 = {6, siernan2_frames_c_idle1, ai_c_cycleend};




