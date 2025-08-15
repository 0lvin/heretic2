//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../../common/header/common.h"
#include "ce_defaultmessagehandler.h"
#include "client_entities.h"

CE_MsgReceiver_t CE_DefaultMessageReceivers[NUM_MESSAGES] =
{
	NULL,
};

void CE_DefaultMsgHandler(client_entity_t *self, CE_Message_t *msg)
{
	CE_MsgReceiver_t receiver;

	receiver = ce_classStatics[self->classID].msgReceivers[msg->ID];

	if (receiver)
	{
		receiver(self, msg);
	}
	else
	{
		// if and when there are a good number of defaults, change the NULL to be an Empty
		// function, overall that should be faster to just always call the function then
		// do the check
		receiver = CE_DefaultMessageReceivers[msg->ID];

		if (receiver)
		{
			CE_DefaultMessageReceivers[msg->ID](self, msg);
		}
	}
}
