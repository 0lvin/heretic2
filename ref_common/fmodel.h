
/*
========================================================================

.FM triangle flexible model file format

========================================================================
*/

#ifndef __FMODEL_HEADER
#define __FMODEL_HEADER

#include "../src/common/header/shared.h"
#include "../src/common/header/files.h"

#define	MAX_FM_TRIANGLES	2048
#define MAX_FM_VERTS		2048
#define MAX_FM_FRAMES		2048
#define MAX_FM_SKINS		64
#define	MAX_FM_SKINNAME		64
#define MAX_FM_MESH_NODES	16		// also defined in game/qshared.h

// the glcmd format:
// a positive integer starts a tristrip command, followed by that many
// vertex structures.
// a negative integer starts a trifan command, followed by -x vertexes
// a zero indicates the end of the command list.
// a vertex consists of a floating point s, a floating point t,
// and an integer vertex index.


// Initial Header
#define FM_HEADER_NAME	"header"
#define FM_HEADER_VER	2

// Skin Header
#define FM_SKIN_NAME	"skin"
#define FM_SKIN_VER		1


// ST Coord Header
#define FM_ST_NAME		"st coord"
#define FM_ST_VER		1

typedef struct
{
	short	s;
	short	t;
} fmstvert_t;


// Tri Header
#define FM_TRI_NAME		"tris"
#define FM_TRI_VER		1

typedef struct
{
	short	index_xyz[3];
	short	index_st[3];
} fmtriangle_t;


// Frame Header
#define FM_FRAME_NAME	"frames"
#define FM_FRAME_VER	1

// Frame for compression, just the names
#define FM_SHORT_FRAME_NAME	"short frames"
#define FM_SHORT_FRAME_VER	1

// Normals for compressed frames
#define FM_NORMAL_NAME	"normals"
#define FM_NORMAL_VER	1

// Compressed Frame Data
#define FM_COMP_NAME	"comp data"
#define FM_COMP_VER	1

// GL Cmds Header
#define FM_GLCMDS_NAME	"glcmds"
#define FM_GLCMDS_VER	1


// Mesh Nodes Header
#define FM_MESH_NAME	"mesh nodes"
#define FM_MESH_VER		3

// Skeleton Header
#define FM_SKELETON_NAME "skeleton"
#define FM_SKELETON_VER	1

// References Header
#define FM_REFERENCES_NAME "references"
#define FM_REFERENCES_VER	1

typedef struct
{

	union
	{

	byte	tris[MAX_FM_TRIANGLES>>3];

	struct {
// jmarshall - 64bit
	//short	*triIndicies;
	int triIndicies; // Array of shorts?
// jmarshall end
	int		num_tris;
	};

	};

	byte	verts[MAX_FM_VERTS>>3];
	short	start_glcmds, num_glcmds;
} fmmeshnode_t;

//=================================================================

// Frame info
typedef struct
{
	byte	v[3];			// scaled byte to fit in frame mins/maxs
	byte	lightnormalindex;
} fmtrivertx_t;

typedef struct
{
	float			scale[3];		// multiply byte verts by this
	float			translate[3];	// then add this
	char			name[16];		// frame name from grabbing
	fmtrivertx_t	verts[1];		// variable sized
} fmaliasframe_t;

#ifndef _TOOL // the following is only relevant to the game code

#define MAX_COMP_DOF 25

#define FRAME_NAME_LEN (16)

typedef struct fmdl_s
{
	fmheader_t				header;
	fmstvert_t				*st_verts;
	fmtriangle_t			*tris;
	fmaliasframe_t			*frames;
	int						*glcmds;
	char					*skin_names;
	fmmeshnode_t			*mesh_nodes;
	//compression stuff
	int						ngroups;
	int						*frame_to_group;
	char					*framenames;
	byte					*lightnormalindex;
	//end comp stuff

	int						skeletalType;
	int						rootCluster;
	struct ModelSkeleton_s	*skeletons;

	int						referenceType;
} fmdl_t;

/*****************************************************
 * Flex Model Loading Routines
 *****************************************************/

extern fmdl_t *fmodel;

void Mod_LoadFlexModel (struct model_s *mod, void *buffer, int length);
void R_DrawFlexModel (entity_t *e);

// jmarshall
enum
{
	FM_BLOCK_HEADER,
	FM_BLOCK_SKIN,
	FM_BLOCK_ST,
	FM_BLOCK_TRIS,
	FM_BLOCK_FRAMES,
	FM_BLOCK_GLCMDS,
	FM_BLOCK_MESHNODES,
	FM_BLOCK_SHORTFRAMES,
	FM_BLOCK_NORMAL,
	FM_BLOCK_COMP,
	FM_BLOCK_SKELETON,
	FM_BLOCK_REFERENCES,
};

typedef struct
{
	char		blockid[32];
	int			blocktype;
} fmblock_t;
// jmarshall end

#endif


#endif // #define __FMODEL_HEADER
