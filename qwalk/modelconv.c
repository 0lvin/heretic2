/*
    QShed <http://www.icculus.org/qshed>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "model.h"

int texwidth = -1;
int texheight = -1;

const char *g_skinpath = NULL;

static model_t *model = NULL;

static bool_t replacetexture(const char *filename)
{
	char *error;
	image_rgba_t *image;
	int i;
	mesh_t *mesh;

	image = image_load_from_file(mem_globalpool, filename, &error);
	if (!image)
	{
		fprintf(stderr, "Failed to load %s: %s.\n", filename, error);
		qfree(error);
		return false;
	}

/* clear old skins */
	model_clear_skins(model);

/* add new skin */
	model->total_skins = 1;
	model->num_skins = 1;
	model->skininfo = (skininfo_t*)qmalloc(sizeof(skininfo_t));
	model->skininfo[0].frametime = 0.1f;
	model->skininfo[0].num_skins = 1;
	model->skininfo[0].skins = (singleskin_t*)qmalloc(sizeof(singleframe_t));
	model->skininfo[0].skins[0].name = copystring(filename);
	model->skininfo[0].skins[0].offset = 0;

	for (i = 0, mesh = model->meshes; i < model->num_meshes; i++, mesh++)
	{
		mesh->skins = (meshskin_t*)qmalloc(sizeof(meshskin_t));
		mesh->skins[0].components[SKIN_DIFFUSE] = image_clone(mem_globalpool, image);
		mesh->skins[0].components[SKIN_FULLBRIGHT] = NULL;
	}

	image_free(&image);
	return true;
}

void dump_txt(const char *filename, const model_t *model)
{
	int i, j, k;
	FILE *fp = fopen(filename, "wt");

	if (!fp)
	{
		printf("Couldn't open %s for writing.\n", filename);
		return;
	}

	fprintf(fp, "Analysis of %s\n\n", filename);

	fprintf(fp, "total_frames = %d\n", model->total_frames);
	fprintf(fp, "num_frames = %d\n", model->num_frames);
	fprintf(fp, "num_tags = %d\n", model->num_tags);
	fprintf(fp, "num_meshes = %d\n", model->num_meshes);
	fprintf(fp, "\n");

	fprintf(fp, "frameinfos:\n");
	if (!model->num_frames)
		fprintf(fp, "(none)\n");
	for (i = 0; i < model->num_frames; i++)
	{
		const frameinfo_t *frameinfo = &model->frameinfo[i];

		if (frameinfo->num_frames == 1)
			fprintf(fp, "%d { name = \"%s\" }\n", i, frameinfo->frames[0].name);
		else
		{
			fprintf(fp, "%d {\n", i);
			fprintf(fp, "\tframetime = %f,\n", frameinfo->frametime);
			for (j = 0; j < frameinfo->num_frames; j++)
			{
				fprintf(fp, "\t{ name = \"%s\", offset = %d }\n", frameinfo->frames[j].name, frameinfo->frames[j].offset);
			}
			fprintf(fp, "}\n");
		}
	}
	fprintf(fp, "\n");

	fprintf(fp, "tags:\n");
	if (!model->num_tags)
		fprintf(fp, "(none)\n");
	for (i = 0; i < model->num_tags; i++)
	{
		const tag_t *tag = &model->tags[i];

		fprintf(fp, "%d {\n", i);
		fprintf(fp, "\tname = \"%s\"\n", tag->name);
		fprintf(fp, "\tframes = {\n");
		for (j = 0; j < model->total_frames; j++)
		{
			fprintf(fp, "\t\t%d { %f %f %f %f, %f %f %f %f, %f %f %f %f, %f %f %f %f }\n", j, tag->matrix[j].m[0][0], tag->matrix[j].m[1][0], tag->matrix[j].m[2][0], tag->matrix[j].m[3][0], tag->matrix[j].m[0][1], tag->matrix[j].m[1][1], tag->matrix[j].m[2][1], tag->matrix[j].m[3][1], tag->matrix[j].m[0][2], tag->matrix[j].m[1][2], tag->matrix[j].m[2][2], tag->matrix[j].m[3][2], tag->matrix[j].m[0][3], tag->matrix[j].m[1][3], tag->matrix[j].m[2][3], tag->matrix[j].m[3][3]);
		}
		fprintf(fp, "\t}\n");
		fprintf(fp, "}\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "meshes:\n");
	if (!model->num_meshes)
		fprintf(fp, "(none)\n");
	for (i = 0; i < model->num_meshes; i++)
	{
		const mesh_t *mesh = &model->meshes[i];

		fprintf(fp, "%d {\n", i);
		fprintf(fp, "\tname = \"%s\"\n", mesh->name);
		fprintf(fp, "\tnum_tris = %d\n", mesh->num_tris);
		fprintf(fp, "\tnum_xyz = %d\n", mesh->num_xyz);
		fprintf(fp, "\n");
		fprintf(fp, "\ttriangle3i = {\n");
		for (j = 0; j < mesh->num_tris; j++)
		{
			fprintf(fp, "\t\t%d => %d %d %d\n", j, mesh->triangle3i[j*3+0], mesh->triangle3i[j*3+1], mesh->triangle3i[j*3+2]);
		}
		fprintf(fp, "\t}");
		fprintf(fp, "\n");
		fprintf(fp, "\ttexcoord2f = {\n");
		for (j = 0; j < mesh->num_xyz; j++)
		{
			fprintf(fp, "\t\t%d => %f %f\n", j, mesh->texcoord2f[j*2+0], mesh->texcoord2f[j*2+1]);
		}
		fprintf(fp, "\t}\n");
		fprintf(fp, "\n");
		fprintf(fp, "\tframes = {\n");
		for (j = 0; j < model->total_frames; j++)
		{
			const float *v = mesh->vertex3f + j * mesh->num_xyz * 3;
			const float *n = mesh->normal3f + j * mesh->num_xyz * 3;
			fprintf(fp, "\t\tframe %d {\n", j);
			for (k = 0; k < mesh->num_xyz; k++)
			{
				fprintf(fp, "\t\t\tvertex %d { v = %f %f %f , n = %f %f %f }\n", k, v[0], v[1], v[2], n[0], n[1], n[2]);
				v += 3;
				n += 3;
			}
			fprintf(fp, "\t\t}\n");
		}
		fprintf(fp, "\t}\n");
		fprintf(fp, "}\n");
	}
	fprintf(fp, "\n");

	fclose(fp);

	printf("Saved %s.\n", filename);
}

int main(int argc, char **argv)
{
	char *error;
	char infilename[1024] = {0};
	char dumpfilename[1024] = {0};
	char outfilename[1024] = {0};
	char texfilename[1024] = {0};
	char skinpath[1024] = {0};
	bool_t notex = false;
	int flags = 0;
	bool_t flags_specified = false;
	int synctype = 0;
	bool_t synctype_specified = false;
	float offsets_x = 0.0f, offsets_y = 0.0f, offsets_z = 0.0f;
	bool_t offsets_specified = false;
	bool_t renormal = false;
	bool_t facet = false;
	bool_t rename_frames = false;
	int i, j, k;

	mem_init();

	set_atexit_final_event(mem_shutdown);
	atexit(call_atexit_events);

	if (argc == 1)
	{
		printf(
"modelconv [options] -i infilename outfilename\n"
"Output format is specified by the file extension of outfilename.\n"
"Options:\n"
"  -i filename        specify the model to load (required).\n"
"  -d filename        specify the txt dump to save.\n"
"  -notex             remove all existing skins from model after importing.\n"
"  -tex filename      replace the model's texture with the given texture. This\n"
"                     is required for any texture to be loaded onto MD2 or MD3\n"
"                     models (the program doesn't automatically load external\n"
"                     skins). Supported formats are PCX, TGA, and JPEG.\n"
"  -texwidth #        see below\n"
"  -texheight #       resample the model's texture to the given dimensions.\n"
"  -skinpath x        specify the path that skins will be exported to when\n"
"                     exporting to md2 (e.g. \"models/players\"). Should not\n"
"                     contain trailing slash. If skinpath is not specified,\n"
"                     skins will be created in the same folder as the model.\n"
"  -flags #           set model flags, such as rocket smoke trail, rotate, etc.\n"
"                     See Quake's defs.qc for a list of all flags.\n"
"  -synctype x        set synctype flag, only used by Quake. The valid values\n"
"                     are sync (default) and rand.\n"
"  -offsets_x #       see below\n"
"  -offsets_y #       see below\n"
"  -offsets_z #       set the offsets vector, which only exists in the MDL\n"
"                     format and is not used by Quake. It's only supported here\n"
"                     for reasons of completeness.\n"
"  -renormal          recalculate vertex normals.\n"
"  -rename_frames     rename all frames to \"frame1\", \"frame2\", etc.\n"
"  -force             force \"yes\" response to all confirmation requests\n"
"                     regarding overwriting existing files or creating\n"
"                     nonexistent paths.\n"
		);
		return 0;
	}

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (!strcmp(argv[i], "-i"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-i'\n", argv[0]);
					return 0;
				}

				strlcpy(infilename, argv[i], sizeof(infilename));
			}
			else if (!strcmp(argv[i], "-d"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-d'\n", argv[0]);
					return 0;
				}

				strlcpy(dumpfilename, argv[i], sizeof(dumpfilename));
			}
			else if (!strcmp(argv[i], "-notex"))
			{
				notex = true;
			}
			else if (!strcmp(argv[i], "-tex"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-tex'\n", argv[0]);
					return 0;
				}

				strlcpy(texfilename, argv[i], sizeof(texfilename));
			}
			else if (!strcmp(argv[i], "-texwidth"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-texwidth'\n", argv[0]);
					return 0;
				}

				texwidth = (int)atoi(argv[i]);

				if (texwidth < 1 || texwidth > 4096)
				{
					printf("%s: invalid value for option '-texwidth'\n", argv[0]);
					return 0;
				}
			}
			else if (!strcmp(argv[i], "-texheight"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-texheight'\n", argv[0]);
					return 0;
				}

				texheight = (int)atoi(argv[i]);

				if (texheight < 1 || texheight > 4096)
				{
					printf("%s: invalid value for option '-texheight'\n", argv[0]);
					return 0;
				}
			}
			else if (!strcmp(argv[i], "-skinpath"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for options '-skinpath'\n", argv[0]);
					return 0;
				}

				strlcpy(skinpath, argv[i], sizeof(skinpath));

			/* parse the path, create missing directories, and normalize the path syntax to something like "folder/folder/folder" */
				if (!makepath(skinpath, &error))
				{
					printf("failed to make skin path: %s\n", error);
					qfree(error);
					return 0;
				}

				g_skinpath = skinpath;
			}
			else if (!strcmp(argv[i], "-flags"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-flags'\n", argv[0]);
					return 0;
				}

				flags = (int)atoi(argv[i]);
				flags_specified = true;
			}
			else if (!strcmp(argv[i], "-synctype"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-synctype'\n", argv[0]);
					return 0;
				}

				if (!strcmp(argv[i], "sync"))
					synctype = 0;
				else if (!strcmp(argv[i], "rand"))
					synctype = 1;
				else
				{
					printf("%s: invalid value for option '-synctype' (accepted values: 'sync' (default), 'rand')\n", argv[0]);
					return 0;
				}

				synctype_specified = true;
			}
			else if (!strcmp(argv[i], "-offsets_x"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-offsets_x'\n", argv[0]);
					return 0;
				}

				offsets_x = (float)atof(argv[i]);
				offsets_specified = true;
			}
			else if (!strcmp(argv[i], "-offsets_y"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-offsets_y'\n", argv[0]);
					return 0;
				}

				offsets_y = (float)atof(argv[i]);
				offsets_specified = true;
			}
			else if (!strcmp(argv[i], "-offsets_z"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-offsets_z'\n", argv[0]);
					return 0;
				}

				offsets_z = (float)atof(argv[i]);
				offsets_specified = true;
			}
			else if (!strcmp(argv[i], "-renormal"))
			{
				renormal = true;
			}
			else if (!strcmp(argv[i], "-facet"))
			{
				facet = true;
			}
			else if (!strcmp(argv[i], "-rename_frames"))
			{
				rename_frames = true;
			}
			else if (!strcmp(argv[i], "-force"))
			{
				g_force_yes = true;
			}
			else
			{
				printf("%s: unrecognized option '%s'\n", argv[0], argv[i]);
				return 0;
			}
		}
		else
		{
			strlcpy(outfilename, argv[i], sizeof(outfilename));
		}
	}

	if (!infilename[0])
	{
		printf("No input file specified.\n");
		return 0;
	}

	model = model_load_from_file(infilename, &error);
	if (!model)
	{
		printf("Failed to load model: %s.\n", error);
		qfree(error);
		return 0;
	}

	printf("Loaded %s.\n", infilename);

	if (flags_specified)
		model->flags = flags;
	if (synctype_specified)
		model->synctype = synctype;
	if (offsets_specified)
	{
		model->offsets[0] = offsets_x;
		model->offsets[1] = offsets_y;
		model->offsets[2] = offsets_z;
	}

	if (notex)
	{
		model_clear_skins(model);
	}

	if (texfilename[0])
	{
		if (!replacetexture(texfilename))
		{
			model_free(model);
			return 1;
		}
	}

	if (texwidth > 0 || texheight > 0)
	{
		mesh_t *mesh;

		for (i = 0, mesh = model->meshes; i < model->num_meshes; i++, mesh++)
		{
			for (j = 0; j < model->total_skins; j++)
			{
				for (k = 0; k < SKIN_NUMTYPES; k++)
				{
					if (mesh->skins[j].components[k])
					{
						image_rgba_t *oldimage = mesh->skins[j].components[k];
						mesh->skins[j].components[k] = image_resize(mem_globalpool, oldimage, (texwidth > 0) ? texwidth : oldimage->width, (texheight > 0) ? texheight : oldimage->height);
						image_free(&oldimage);
					}
				}
			}
		}
	}

	if (renormal)
		model_recalculate_normals(model);

	if (facet)
		model_facetize(model);

	if (rename_frames)
		model_rename_frames(model);

	if (dumpfilename[0])
	{
		dump_txt(dumpfilename, model);
	}

	if (!outfilename[0])
	{
		/* TODO - print brief analysis of input file (further analysis done on option) */
		printf("No output file specified.\n");
	}
	else
	{
		if (!model_save(outfilename, model, &error))
		{
			printf("Failed to save model: %s.\n", error);
			qfree(error);
		}
	}

	model_free(model);
	return 0;
}
