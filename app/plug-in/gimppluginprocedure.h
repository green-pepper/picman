/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginprocedure.h
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

#ifndef __PICMAN_PLUG_IN_PROCEDURE_H__
#define __PICMAN_PLUG_IN_PROCEDURE_H__

#include <time.h>      /* time_t */

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "pdb/picmanprocedure.h"


#define PICMAN_TYPE_PLUG_IN_PROCEDURE            (picman_plug_in_procedure_get_type ())
#define PICMAN_PLUG_IN_PROCEDURE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PLUG_IN_PROCEDURE, PicmanPlugInProcedure))
#define PICMAN_PLUG_IN_PROCEDURE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PLUG_IN_PROCEDURE, PicmanPlugInProcedureClass))
#define PICMAN_IS_PLUG_IN_PROCEDURE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PLUG_IN_PROCEDURE))
#define PICMAN_IS_PLUG_IN_PROCEDURE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PLUG_IN_PROCEDURE))
#define PICMAN_PLUG_IN_PROCEDURE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PLUG_IN_PROCEDURE, PicmanPlugInProcedureClass))


typedef struct _PicmanPlugInProcedureClass PicmanPlugInProcedureClass;

struct _PicmanPlugInProcedure
{
  PicmanProcedure        parent_instance;

  /*  common members  */
  gchar               *prog;
  GQuark               locale_domain;
  GQuark               help_domain;
  gchar               *menu_label;
  GList               *menu_paths;
  gchar               *label;
  PicmanIconType         icon_type;
  gint                 icon_data_length;
  guint8              *icon_data;
  gchar               *image_types;
  PicmanPlugInImageType  image_types_val;
  time_t               mtime;
  gboolean             installed_during_init;

  /*  file proc specific members  */
  gboolean             file_proc;
  gchar               *extensions;
  gchar               *prefixes;
  gchar               *magics;
  gchar               *mime_type;
  gboolean             handles_uri;
  GSList              *extensions_list;
  GSList              *prefixes_list;
  GSList              *magics_list;
  gchar               *thumb_loader;
};

struct _PicmanPlugInProcedureClass
{
  PicmanProcedureClass parent_class;

  /*  virtual functions  */
  const gchar * (* get_progname)    (const PicmanPlugInProcedure *procedure);

  /*  signals  */
  void          (* menu_path_added) (PicmanPlugInProcedure       *procedure,
                                     const gchar               *menu_path);
};


GType           picman_plug_in_procedure_get_type      (void) G_GNUC_CONST;

PicmanProcedure * picman_plug_in_procedure_new           (PicmanPDBProcType            proc_type,
                                                      const gchar               *prog);

PicmanPlugInProcedure * picman_plug_in_procedure_find    (GSList                    *list,
                                                      const gchar               *proc_name);

const gchar * picman_plug_in_procedure_get_progname    (const PicmanPlugInProcedure *proc);

void          picman_plug_in_procedure_set_locale_domain (PicmanPlugInProcedure     *proc,
                                                        const gchar             *locale_domain);
const gchar * picman_plug_in_procedure_get_locale_domain (const PicmanPlugInProcedure *proc);

void          picman_plug_in_procedure_set_help_domain (PicmanPlugInProcedure       *proc,
                                                      const gchar               *help_domain);
const gchar * picman_plug_in_procedure_get_help_domain (const PicmanPlugInProcedure *proc);

gboolean      picman_plug_in_procedure_add_menu_path   (PicmanPlugInProcedure       *proc,
                                                      const gchar               *menu_path,
                                                      GError                   **error);

const gchar * picman_plug_in_procedure_get_label       (PicmanPlugInProcedure       *proc);
const gchar * picman_plug_in_procedure_get_blurb       (const PicmanPlugInProcedure *proc);

void          picman_plug_in_procedure_set_icon        (PicmanPlugInProcedure       *proc,
                                                      PicmanIconType               type,
                                                      const guint8              *data,
                                                      gint                       data_length);
const gchar * picman_plug_in_procedure_get_stock_id    (const PicmanPlugInProcedure *proc);
GdkPixbuf   * picman_plug_in_procedure_get_pixbuf      (const PicmanPlugInProcedure *proc);

gchar       * picman_plug_in_procedure_get_help_id     (const PicmanPlugInProcedure *proc);

gboolean      picman_plug_in_procedure_get_sensitive   (const PicmanPlugInProcedure *proc,
                                                      PicmanDrawable              *drawable);

void          picman_plug_in_procedure_set_image_types (PicmanPlugInProcedure       *proc,
                                                      const gchar               *image_types);
void          picman_plug_in_procedure_set_file_proc   (PicmanPlugInProcedure       *proc,
                                                      const gchar               *extensions,
                                                      const gchar               *prefixes,
                                                      const gchar               *magics);
void          picman_plug_in_procedure_set_mime_type   (PicmanPlugInProcedure       *proc,
                                                      const gchar               *mime_ype);
void          picman_plug_in_procedure_set_handles_uri (PicmanPlugInProcedure       *proc);
void          picman_plug_in_procedure_set_thumb_loader(PicmanPlugInProcedure       *proc,
                                                      const gchar               *thumbnailer);

void     picman_plug_in_procedure_handle_return_values (PicmanPlugInProcedure       *proc,
                                                      Picman                      *picman,
                                                      PicmanProgress              *progress,

                                                      PicmanValueArray            *return_vals);


#endif /* __PICMAN_PLUG_IN_PROCEDURE_H__ */
