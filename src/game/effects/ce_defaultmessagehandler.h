//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef	CE_DEFAULTMESSAGEHANDLER_H
#define	CE_DEFAULTMESSAGEHANDLER_H

#include "../../common/header/common.h"
#include "ce_message.h"

extern CE_MsgReceiver_t CE_DefaultMessageReceivers[NUM_MESSAGES];
extern void CE_DefaultMsgHandler(struct client_entity_s *self, CE_Message_t *msg);

#endif
