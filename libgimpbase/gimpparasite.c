/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanparasite.c
 * Copyright (C) 1998 Jay Cox <jaycox@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>

#include <glib-object.h>

#ifdef G_OS_WIN32
#include <process.h>                /* For _getpid() */
#endif

#include "picmanbasetypes.h"

#include "picmanparasite.h"


/**
 * SECTION: picmanparasite
 * @title: picmanparasite
 * @short_description: Arbitrary pieces of data which can be attached
 *                     to various PICMAN objects.
 * @see_also: picman_image_parasite_attach(),
 *            picman_drawable_parasite_attach(), picman_parasite_attach()
 *            and their related functions.
 *
 * Arbitrary pieces of data which can be attached to various PICMAN objects.
 **/


/*
 * PICMAN_TYPE_PARASITE
 */

GType
picman_parasite_get_type (void)
{
  static GType type = 0;

  if (! type)
    type = g_boxed_type_register_static ("PicmanParasite",
                                         (GBoxedCopyFunc) picman_parasite_copy,
                                         (GBoxedFreeFunc) picman_parasite_free);

  return type;
}


/*
 * PICMAN_TYPE_PARAM_PARASITE
 */

#define PICMAN_PARAM_SPEC_PARASITE(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_PARASITE, PicmanParamSpecParasite))

typedef struct _PicmanParamSpecParasite PicmanParamSpecParasite;

struct _PicmanParamSpecParasite
{
  GParamSpecBoxed parent_instance;
};

static void       picman_param_parasite_class_init  (GParamSpecClass *class);
static void       picman_param_parasite_init        (GParamSpec      *pspec);
static gboolean   picman_param_parasite_validate    (GParamSpec      *pspec,
                                                   GValue          *value);
static gint       picman_param_parasite_values_cmp  (GParamSpec      *pspec,
                                                   const GValue    *value1,
                                                   const GValue    *value2);

GType
picman_param_parasite_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo type_info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_parasite_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecParasite),
        0,
        (GInstanceInitFunc) picman_param_parasite_init
      };

      type = g_type_register_static (G_TYPE_PARAM_BOXED,
                                     "PicmanParamParasite",
                                     &type_info, 0);
    }

  return type;
}

static void
picman_param_parasite_class_init (GParamSpecClass *class)
{
  class->value_type     = PICMAN_TYPE_PARASITE;
  class->value_validate = picman_param_parasite_validate;
  class->values_cmp     = picman_param_parasite_values_cmp;
}

static void
picman_param_parasite_init (GParamSpec *pspec)
{
}

static gboolean
picman_param_parasite_validate (GParamSpec *pspec,
                              GValue     *value)
{
  PicmanParasite *parasite = value->data[0].v_pointer;

  if (! parasite)
    {
      return TRUE;
    }
  else if (parasite->name == NULL                          ||
           ! g_utf8_validate (parasite->name, -1, NULL)    ||
           (parasite->size == 0 && parasite->data != NULL) ||
           (parasite->size >  0 && parasite->data == NULL))
    {
      g_value_set_boxed (value, NULL);
      return TRUE;
    }

  return FALSE;
}

static gint
picman_param_parasite_values_cmp (GParamSpec   *pspec,
                                const GValue *value1,
                                const GValue *value2)
{
  PicmanParasite *parasite1 = value1->data[0].v_pointer;
  PicmanParasite *parasite2 = value2->data[0].v_pointer;

  /*  try to return at least *something*, it's useless anyway...  */

  if (! parasite1)
    return parasite2 != NULL ? -1 : 0;
  else if (! parasite2)
    return parasite1 != NULL;
  else
    return picman_parasite_compare (parasite1, parasite2);
}

GParamSpec *
picman_param_spec_parasite (const gchar *name,
                          const gchar *nick,
                          const gchar *blurb,
                          GParamFlags  flags)
{
  PicmanParamSpecParasite *parasite_spec;

  parasite_spec = g_param_spec_internal (PICMAN_TYPE_PARAM_PARASITE,
                                         name, nick, blurb, flags);

  return G_PARAM_SPEC (parasite_spec);
}


#ifdef DEBUG
static void
picman_parasite_print (PicmanParasite *parasite)
{
  if (parasite == NULL)
    {
      g_print ("pid %d: attempt to print a null parasite\n", getpid ());
      return;
    }

  g_print ("pid %d: parasite: %p\n", getpid (), parasite);

  if (parasite->name)
    g_print ("\tname: %s\n", parasite->name);
  else
    g_print ("\tname: NULL\n");

  g_print ("\tflags: %d\n", parasite->flags);
  g_print ("\tsize: %d\n", parasite->size);
  if (parasite->size > 0)
    g_print ("\tdata: %p\n", parasite->data);
}
#endif

PicmanParasite *
picman_parasite_new (const gchar    *name,
                   guint32         flags,
                   guint32         size,
                   gconstpointer   data)
{
  PicmanParasite *parasite;

  if (!name)
    return NULL;

  parasite = g_slice_new (PicmanParasite);
  parasite->name  = g_strdup (name);
  parasite->flags = (flags & 0xFF);
  parasite->size  = size;

  if (size)
    parasite->data = g_memdup (data, size);
  else
    parasite->data = NULL;

  return parasite;
}

void
picman_parasite_free (PicmanParasite *parasite)
{
  if (parasite == NULL)
    return;

  if (parasite->name)
    g_free (parasite->name);

  if (parasite->data)
    g_free (parasite->data);

  g_slice_free (PicmanParasite, parasite);
}

gboolean
picman_parasite_is_type (const PicmanParasite *parasite,
                       const gchar        *name)
{
  if (!parasite || !parasite->name)
    return FALSE;

  return (strcmp (parasite->name, name) == 0);
}

PicmanParasite *
picman_parasite_copy (const PicmanParasite *parasite)
{
  if (parasite == NULL)
    return NULL;

  return picman_parasite_new (parasite->name, parasite->flags,
                            parasite->size, parasite->data);
}

gboolean
picman_parasite_compare (const PicmanParasite *a,
                       const PicmanParasite *b)
{
  if (a && b &&
      a->name && b->name &&
      strcmp (a->name, b->name) == 0 &&
      a->flags == b->flags &&
      a->size == b->size)
    {
      if (a->data == NULL && b->data == NULL)
        return TRUE;
      else if (a->data && b->data && memcmp (a->data, b->data, a->size) == 0)
        return TRUE;
    }

  return FALSE;
}

gulong
picman_parasite_flags (const PicmanParasite *parasite)
{
  if (parasite == NULL)
    return 0;

  return parasite->flags;
}

gboolean
picman_parasite_is_persistent (const PicmanParasite *parasite)
{
  if (parasite == NULL)
    return FALSE;

  return (parasite->flags & PICMAN_PARASITE_PERSISTENT);
}

gboolean
picman_parasite_is_undoable (const PicmanParasite *parasite)
{
  if (parasite == NULL)
    return FALSE;

  return (parasite->flags & PICMAN_PARASITE_UNDOABLE);
}

gboolean
picman_parasite_has_flag (const PicmanParasite *parasite,
                        gulong              flag)
{
  if (parasite == NULL)
    return FALSE;

  return (parasite->flags & flag);
}

const gchar *
picman_parasite_name (const PicmanParasite *parasite)
{
  if (parasite)
    return parasite->name;

  return NULL;
}

gconstpointer
picman_parasite_data (const PicmanParasite *parasite)
{
  if (parasite)
    return parasite->data;

  return NULL;
}

glong
picman_parasite_data_size (const PicmanParasite *parasite)
{
  if (parasite)
    return parasite->size;

  return 0;
}
