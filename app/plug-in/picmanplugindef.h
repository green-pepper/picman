/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanplugindef.h
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

#ifndef __PICMAN_PLUG_IN_DEF_H__
#define __PICMAN_PLUG_IN_DEF_H__


#include <time.h>

#include "core/picmanobject.h"


#define PICMAN_TYPE_PLUG_IN_DEF            (picman_plug_in_def_get_type ())
#define PICMAN_PLUG_IN_DEF(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PLUG_IN_DEF, PicmanPlugInDef))
#define PICMAN_PLUG_IN_DEF_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PLUG_IN_DEF, PicmanPlugInDefClass))
#define PICMAN_IS_PLUG_IN_DEF(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PLUG_IN_DEF))
#define PICMAN_IS_PLUG_IN_DEF_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PLUG_IN_DEF))


typedef struct _PicmanPlugInDefClass PicmanPlugInDefClass;

struct _PicmanPlugInDef
{
  PicmanObject  parent_instance;

  gchar      *prog;
  GSList     *procedures;
  gchar      *locale_domain_name;
  gchar      *locale_domain_path;
  gchar      *help_domain_name;
  gchar      *help_domain_uri;
  time_t      mtime;
  gboolean    needs_query;  /* Does the plug-in need to be queried ?     */
  gboolean    has_init;     /* Does the plug-in need to be initialized ? */
};

struct _PicmanPlugInDefClass
{
  PicmanObjectClass  parent_class;
};


GType           picman_plug_in_def_get_type (void) G_GNUC_CONST;

PicmanPlugInDef * picman_plug_in_def_new      (const gchar         *prog);

void   picman_plug_in_def_add_procedure     (PicmanPlugInDef       *plug_in_def,
                                           PicmanPlugInProcedure *proc);
void   picman_plug_in_def_remove_procedure  (PicmanPlugInDef       *plug_in_def,
                                           PicmanPlugInProcedure *proc);

void   picman_plug_in_def_set_locale_domain (PicmanPlugInDef       *plug_in_def,
                                           const gchar         *domain_name,
                                           const gchar         *domain_path);

void   picman_plug_in_def_set_help_domain   (PicmanPlugInDef       *plug_in_def,
                                           const gchar         *domain_name,
                                           const gchar         *domain_uri);

void   picman_plug_in_def_set_mtime         (PicmanPlugInDef       *plug_in_def,
                                           time_t               mtime);
void   picman_plug_in_def_set_needs_query   (PicmanPlugInDef       *plug_in_def,
                                           gboolean             needs_query);
void   picman_plug_in_def_set_has_init      (PicmanPlugInDef       *plug_in_def,
                                           gboolean             has_init);


#endif /* __PICMAN_PLUG_IN_DEF_H__ */
