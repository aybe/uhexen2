/* entities.c
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "entities.h"
#include "light.h"


entity_t	entities[MAX_MAP_ENTITIES];
int			num_entities;


/*
==============================================================================

ENTITY FILE PARSING

If a light has a targetname, generate a unique style in the 32-63 range
==============================================================================
*/

static int		numlighttargets;
static char	lighttargets[32][64];

static int LightStyleForTargetname (char *targetname, qboolean alloc)
{
	int		i;

	for (i = 0 ; i < numlighttargets ; i++)
		if (!strcmp (lighttargets[i], targetname))
			return 32 + i;
	if (!alloc)
		return -1;
	strcpy (lighttargets[i], targetname);
	numlighttargets++;
	return numlighttargets-1 + 32;
}


/*
==================
MatchTargets
==================
*/
static void MatchTargets (void)
{
	int		i, j;

	for (i = 0 ; i < num_entities ; i++)
	{
		if (!entities[i].target[0])
			continue;

		for (j = 0 ; j < num_entities ; j++)
			if (!strcmp(entities[j].targetname, entities[i].target))
			{
				entities[i].targetent = &entities[j];
				break;
			}
		if (j == num_entities)
		{
			printf ("WARNING: entity at (%i,%i,%i) (%s) has unmatched target\n",
					(int)entities[i].origin[0], (int)entities[i].origin[1],
					(int)entities[i].origin[2], entities[i].classname);
			continue;
		}

	// set the style on the source ent for switchable lights
		if (entities[j].style)
		{
			char	s[16];

			entities[i].style = entities[j].style;
			sprintf (s,"%i", entities[i].style);
			SetKeyValue (&entities[i], "style", s);
		}
	}
}


/*
==================
LoadEntities
==================
*/
void LoadEntities (void)
{
	const char	*data;
	entity_t	*entity;
	char		key[64];
	epair_t		*epair;

	data = dentdata;
//
// start parsing
//
	num_entities = 0;

// go through all the entities
	while (1)
	{
	// parse the opening brace
		data = COM_Parse (data);
		if (!data)
			break;
		if (com_token[0] != '{')
			COM_Error ("%s: found %s when expecting {", __thisfunc__, com_token);

		if (num_entities == MAX_MAP_ENTITIES)
			COM_Error ("%s: MAX_MAP_ENTITIES", __thisfunc__);
		entity = &entities[num_entities];
		num_entities++;

	// go through all the keys in this entity
		while (1)
		{
			int		c;

		// parse key
			data = COM_Parse (data);
			if (!data)
				COM_Error ("%s: EOF without closing brace", __thisfunc__);
			if (!strcmp(com_token,"}"))
				break;
			strcpy (key, com_token);

		// parse value
			data = COM_Parse (data);
			if (!data)
				COM_Error ("%s: EOF without closing brace", __thisfunc__);
			c = com_token[0];
			if (c == '}')
				COM_Error ("%s: closing brace without data", __thisfunc__);

			epair = (epair_t *) SafeMalloc (sizeof(epair_t));
			strcpy (epair->key, key);
			strcpy (epair->value, com_token);
			epair->next = entity->epairs;
			entity->epairs = epair;

			if (!strcmp(key, "classname"))
				strcpy (entity->classname, com_token);
			else if (!strcmp(key, "target"))
				strcpy (entity->target, com_token);
			else if (!strcmp(key, "targetname"))
				strcpy (entity->targetname, com_token);
			else if (!strcmp(key, "origin"))
			{
				if (sscanf(com_token, "%lf %lf %lf",
						&entity->origin[0],
						&entity->origin[1],
						&entity->origin[2]) != 3)
					COM_Error ("%s: not 3 values for origin", __thisfunc__);
			}
			else if (!strncmp(key, "light", 5))
			{
				entity->light = atoi(com_token);
			}
			else if (!strcmp(key, "style"))
			{
				entity->style = atoi(com_token);
				if ((unsigned int)entity->style > 254)
					COM_Error ("Bad light style %i (must be 0-254)", entity->style);
			}
			else if (!strcmp(key, "angle"))
			{
				entity->angle = (float)atof(com_token);
			}
		}

	// all fields have been parsed
		if (!strncmp (entity->classname, "light", 5) && !entity->light)
			entity->light = DEFAULTLIGHTLEVEL;

		if (!strncmp (entity->classname, "light", 5))
		{
			if (entity->targetname[0] && !entity->style)
			{
				char	s[16];

				entity->style = LightStyleForTargetname (entity->targetname, true);
				sprintf (s,"%i", entity->style);
				SetKeyValue (entity, "style", s);
			}
		}
	}

	printf ("%d entities read\n", num_entities);

	MatchTargets ();
}


const char *ValueForKey (entity_t *ent, const char *key)
{
	epair_t	*ep;

	for (ep = ent->epairs ; ep ; ep = ep->next)
	{
		if (!strcmp (ep->key, key) )
			return ep->value;
	}

	return "";
}

void SetKeyValue (entity_t *ent, const char *key, const char *value)
{
	epair_t	*ep;

	for (ep = ent->epairs ; ep ; ep = ep->next)
	{
		if (!strcmp (ep->key, key) )
		{
			strcpy (ep->value, value);
			return;
		}
	}

	ep = (epair_t *) SafeMalloc (sizeof(*ep));
	ep->next = ent->epairs;
	ent->epairs = ep;
	strcpy (ep->key, key);
	strcpy (ep->value, value);
}

float FloatForKey (entity_t *ent, const char *key)
{
	const char	*k;

	k = ValueForKey (ent, key);
	return (float)atof(k);
}

void GetVectorForKey (entity_t *ent, const char *key, vec3_t vec)
{
	const char	*k;

	k = ValueForKey (ent, key);
	sscanf (k, "%lf %lf %lf", &vec[0], &vec[1], &vec[2]);
}


/*
================
WriteEntitiesToString
================
*/
void WriteEntitiesToString (void)
{
	char	*buf, *end;
	epair_t	*ep;
	char	line[128];
	int		i;

	buf = dentdata;
	end = buf;
	*end = 0;

	printf ("%i switchable light styles\n", numlighttargets);

	for (i = 0 ; i < num_entities ; i++)
	{
		ep = entities[i].epairs;
		if (!ep)
			continue;	// ent got removed

		strcat (end,"{\n");
		end += 2;

		for (ep = entities[i].epairs ; ep ; ep = ep->next)
		{
			q_snprintf (line, sizeof(line), "\"%s\" \"%s\"\n", ep->key, ep->value);
			strcat (end, line);
			end += strlen(line);
		}
		strcat (end,"}\n");
		end += 2;

		if (end > buf + MAX_MAP_ENTSTRING)
			COM_Error ("Entity text too long");
	}
	entdatasize = end - buf + 1;
}

