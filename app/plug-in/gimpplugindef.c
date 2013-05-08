/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanplugindef.c
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>

#include "plug-in-types.h"

#include "core/picman-utils.h"

#include "picmanplugindef.h"
#include "picmanpluginprocedure.h"


static void     picman_plug_in_def_finalize    (GObject    *object);

static gint64   picman_plug_in_def_get_memsize (PicmanObject *object,
                                              gint64     *gui_size);


G_DEFINE_TYPE (PicmanPlugInDef, picman_plug_in_def, PICMAN_TYPE_OBJECT)

#define parent_class picman_plug_in_def_parent_class


static void
picman_plug_in_def_class_init (PicmanPlugInDefClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  object_class->finalize         = picman_plug_in_def_finalize;

  picman_object_class->get_memsize = picman_plug_in_def_get_memsize;
}

static void
picman_plug_in_def_init (PicmanPlugInDef *def)
{
}

static void
picman_plug_in_def_finalize (GObject *object)
{
  PicmanPlugInDef *plug_in_def = PICMAN_PLUG_IN_DEF (object);

  g_free (plug_in_def->prog);
  g_free (plug_in_def->locale_domain_name);
  g_free (plug_in_def->locale_domain_path);
  g_free (plug_in_def->help_domain_name);
  g_free (plug_in_def->help_domain_uri);

  g_slist_free_full (plug_in_def->procedures, (GDestroyNotify) g_object_unref);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_plug_in_def_get_memsize (PicmanObject *object,
                              gint64     *gui_size)
{
  PicmanPlugInDef *plug_in_def = PICMAN_PLUG_IN_DEF (object);
  gint64         memsize     = 0;

  memsize += picman_string_get_memsize (plug_in_def->prog);
  memsize += picman_string_get_memsize (plug_in_def->locale_domain_name);
  memsize += picman_string_get_memsize (plug_in_def->locale_domain_path);
  memsize += picman_string_get_memsize (plug_in_def->help_domain_name);
  memsize += picman_string_get_memsize (plug_in_def->help_domain_uri);

  memsize += picman_g_slist_get_memsize (plug_in_def->procedures, 0);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}


/*  public functions  */

PicmanPlugInDef *
picman_plug_in_def_new (const gchar *prog)
{
  PicmanPlugInDef *plug_in_def;

  g_return_val_if_fail (prog != NULL, NULL);

  plug_in_def = g_object_new (PICMAN_TYPE_PLUG_IN_DEF, NULL);

  plug_in_def->prog = g_strdup (prog);

  return plug_in_def;
}

void
picman_plug_in_def_add_procedure (PicmanPlugInDef       *plug_in_def,
                                PicmanPlugInProcedure *proc)
{
  PicmanPlugInProcedure *overridden;

  g_return_if_fail (PICMAN_IS_PLUG_IN_DEF (plug_in_def));
  g_return_if_fail (PICMAN_IS_PLUG_IN_PROCEDURE (proc));

  overridden = picman_plug_in_procedure_find (plug_in_def->procedures,
                                            picman_object_get_name (proc));

  if (overridden)
    picman_plug_in_def_remove_procedure (plug_in_def, overridden);

  proc->mtime = plug_in_def->mtime;

  picman_plug_in_procedure_set_locale_domain (proc,
                                            plug_in_def->locale_domain_name);
  picman_plug_in_procedure_set_help_domain (proc,
                                          plug_in_def->help_domain_name);

  plug_in_def->procedures = g_slist_append (plug_in_def->procedures,
                                            g_object_ref (proc));
}

void
picman_plug_in_def_remove_procedure (PicmanPlugInDef       *plug_in_def,
                                   PicmanPlugInProcedure *proc)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_DEF (plug_in_def));
  g_return_if_fail (PICMAN_IS_PLUG_IN_PROCEDURE (proc));

  plug_in_def->procedures = g_slist_remove (plug_in_def->procedures, proc);
  g_object_unref (proc);
}

void
picman_plug_in_def_set_locale_domain (PicmanPlugInDef *plug_in_def,
                                    const gchar   *domain_name,
                                    const gchar   *domain_path)
{
  GSList *list;

  g_return_if_fail (PICMAN_IS_PLUG_IN_DEF (plug_in_def));

  if (plug_in_def->locale_domain_name)
    g_free (plug_in_def->locale_domain_name);
  plug_in_def->locale_domain_name = g_strdup (domain_name);

  if (plug_in_def->locale_domain_path)
    g_free (plug_in_def->locale_domain_path);
  plug_in_def->locale_domain_path = g_strdup (domain_path);

  for (list = plug_in_def->procedures; list; list = g_slist_next (list))
    {
      PicmanPlugInProcedure *procedure = list->data;

      picman_plug_in_procedure_set_locale_domain (procedure,
                                                plug_in_def->locale_domain_name);
    }
}

void
picman_plug_in_def_set_help_domain (PicmanPlugInDef *plug_in_def,
                                  const gchar   *domain_name,
                                  const gchar   *domain_uri)
{
  GSList *list;

  g_return_if_fail (PICMAN_IS_PLUG_IN_DEF (plug_in_def));

  if (plug_in_def->help_domain_name)
    g_free (plug_in_def->help_domain_name);
  plug_in_def->help_domain_name = g_strdup (domain_name);

  if (plug_in_def->help_domain_uri)
    g_free (plug_in_def->help_domain_uri);
  plug_in_def->help_domain_uri = g_strdup (domain_uri);

  for (list = plug_in_def->procedures; list; list = g_slist_next (list))
    {
      PicmanPlugInProcedure *procedure = list->data;

      picman_plug_in_procedure_set_help_domain (procedure,
                                              plug_in_def->help_domain_name);
    }
}

void
picman_plug_in_def_set_mtime (PicmanPlugInDef *plug_in_def,
                            time_t         mtime)
{
  GSList *list;

  g_return_if_fail (PICMAN_IS_PLUG_IN_DEF (plug_in_def));

  plug_in_def->mtime = mtime;

  for (list = plug_in_def->procedures; list; list = g_slist_next (list))
    {
      PicmanPlugInProcedure *proc = list->data;

      proc->mtime = plug_in_def->mtime;
    }
}

void
picman_plug_in_def_set_needs_query (PicmanPlugInDef *plug_in_def,
                                  gboolean       needs_query)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_DEF (plug_in_def));

  plug_in_def->needs_query = needs_query ? TRUE : FALSE;
}

void
picman_plug_in_def_set_has_init (PicmanPlugInDef *plug_in_def,
                               gboolean       has_init)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_DEF (plug_in_def));

  plug_in_def->has_init = has_init ? TRUE : FALSE;
}
