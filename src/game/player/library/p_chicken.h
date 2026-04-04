//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef _P_CHICKEN_H_
#define _P_CHICKEN_H_

#include "../../header/local.h"

extern panimmove_t chicken_move_stand1;
extern panimmove_t chicken_move_walk;
extern panimmove_t chicken_move_run;
extern panimmove_t chicken_move_cluck;
extern panimmove_t chicken_move_attack;
extern panimmove_t chicken_move_eat;
extern panimmove_t chicken_move_jump;

//Dummy anim to catch sequence leaks
extern panimmove_t chickenp_move_dummy;

extern panimmove_t chickenp_move_stand;
extern panimmove_t chickenp_move_stand1;
extern panimmove_t chickenp_move_stand2;
extern panimmove_t chickenp_move_walk;
extern panimmove_t chickenp_move_run;
extern panimmove_t chickenp_move_back;
extern panimmove_t chickenp_move_runb;
extern panimmove_t chickenp_move_bite;
extern panimmove_t chickenp_move_strafel;
extern panimmove_t chickenp_move_strafer;
extern panimmove_t chickenp_move_jump;
extern panimmove_t chickenp_move_wjump;
extern panimmove_t chickenp_move_wjumpb;
extern panimmove_t chickenp_move_rjump;
extern panimmove_t chickenp_move_rjumpb;
extern panimmove_t chickenp_move_jump_loop;
extern panimmove_t chickenp_move_attack;
extern panimmove_t chickenp_move_jump_flap;
extern panimmove_t chickenp_move_runattack;
extern panimmove_t chickenp_move_swim_idle;
extern panimmove_t chickenp_move_swim;

void PlayerChickenBite(playerinfo_t *playerinfo);
void PlayerChickenSqueal(playerinfo_t *playerinfo);
int PlayerChickenJump(playerinfo_t *playerinfo);

void PlayerChickenFlap (playerinfo_t *playerinfo);
void PlayerChickenCheckFlap (playerinfo_t *playerinfo);

void ChickenAssert(playerinfo_t *playerinfo);
void PlayerChickenCluck(playerinfo_t *playerinfo, float force);

void ChickenStepSound(playerinfo_t *playerinfo, float value);

#endif // _P_CHICKEN_H_
