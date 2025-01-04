//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef	EFFECTFLAGS_H
#define EFFECTFLAGS_H

// ************************************************************************************************
// CEF_XXX
// -------
// Flags specified when a client-effect is created; don't even think about expanding this beyond 1
// byte! This is 'cos only the frist byte is sent across the net (and only a flag is actually set).
// ************************************************************************************************

#define CEF_OWNERS_ORIGIN			0x00000001  // Use the owner's origin only, with no additional
												// displacment.
#define CEF_BROADCAST				0x00000002	// sent to all client's
#define CEF_MULTICAST				0x00000008	// places the effect into the world buffer
												// instead of the owner's buffer (no effect
												// on independent effects), resulting in the
												// effect being more reliably sent at the
												// expense of an extra byte or two
#define CEF_DONT_LINK				0x00000010	// used to stop cleint effects from being linked to their
												// owner's movement
												// In this case, CEF_OWNERS_ORIGIN causes the owner's origin
												// to be used for initialization only
#define CEF_FLAG6					0x00000020	//
#define CEF_FLAG7					0x00000040	//
#define CEF_FLAG8					0x00000080	//

// Client-effect Flags relevant only in the Client Effects DLL.

#define CEF_VIEWSTATUSCHANGED		0x00020000	// If this flag is set, do not think when the CEF_CULLED flag is set.
#define CEF_USE_VELOCITY2			0x00040000	// Sprite lines.  Read and apply the velocity2 and acceleration2 fields of the line
#define CEF_USE_SCALE2				0x00080000	// Sprite lines.  Read and apply the scale2 value to the endpoint
#define CEF_AUTO_ORIGIN				0x00100000	// Sprite lines.  Read just the origin of the line to the centerpoint after any movement

#define CEF_PULSE_ALPHA				0x00200000	// Particle/fx d_alpha: when hits 1.0 alpha, reverse and start fading out.
#define CEF_ABSOLUTE_PARTS			0x00400000	// Particle origins represent absolute positions.
#define	CEF_ADDITIVE_PARTS			0x00800000	// Particles are additively transparent (temporary)

#define CEF_DROPPED					0x01000000	// entity was dropped from the view due to an excessive number of entites in the view
#define CEF_NOMOVE					0x02000000	// velocity and acceleration are not applied to origin in update
												// acceleration is not applied to velocity in update
												// allows vel and accel to be used for something else
												// for static entities
#define CEF_CULLED					0x04000000	// Culled from view this frame (set or unset) in AddEffectsToView().
#define CEF_CLIP_TO_WORLD			0x08000000	// Turns on collision detection with the world. Additionally, the
												// entity needs to have a message handler in order to recieve MSG_COLLISION.
#define CEF_DISAPPEARED				0x20000000	// Alpha faded out, or scaled to nothing needs to be turned off if entity
												// later scales up or fades back in.
#define CEF_CHECK_OWNER				0x40000000	// if we are owned, then check to see if our owner has been server culled before it gets to the client
#define CEF_NO_DRAW					0x80000000	// Doesn't get added to the render list.

#endif // EFFECTFLAGS_H
