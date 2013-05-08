/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_H__
#define __PICMAN_H__


#include "picmanobject.h"
#include "picman-gui.h"


#define PICMAN_TYPE_PICMAN            (picman_get_type ())
#define PICMAN(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PICMAN, Picman))
#define PICMAN_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PICMAN, PicmanClass))
#define PICMAN_IS_PICMAN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PICMAN))
#define PICMAN_IS_PICMAN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PICMAN))


typedef struct _PicmanClass PicmanClass;

struct _Picman
{
  PicmanObject              parent_instance;

  PicmanCoreConfig         *config;
  PicmanCoreConfig         *edit_config; /* don't use this one, it's just
                                        * for the preferences dialog
                                        */
  gchar                  *session_name;
  gchar                  *default_folder;

  gboolean                be_verbose;
  gboolean                no_data;
  gboolean                no_fonts;
  gboolean                no_interface;
  gboolean                show_gui;
  gboolean                use_shm;
  PicmanMessageHandlerType  message_handler;
  gboolean                console_messages;
  PicmanStackTraceMode      stack_trace_mode;
  PicmanPDBCompatMode       pdb_compat_mode;

  PicmanGui                 gui;         /* gui vtable */

  gboolean                restored;    /* becomes TRUE in picman_restore() */

  gint                    busy;
  guint                   busy_idle_id;

  GList                  *user_units;
  gint                    n_user_units;

  PicmanParasiteList       *parasites;

  PicmanContainer          *paint_info_list;
  PicmanPaintInfo          *standard_paint_info;

  PicmanModuleDB           *module_db;
  gboolean                write_modulerc;

  PicmanPlugInManager      *plug_in_manager;

  PicmanContainer          *images;
  guint32                 next_guide_ID;
  guint32                 next_sample_point_ID;
  PicmanIdTable            *image_table;

  PicmanIdTable            *item_table;

  PicmanContainer          *displays;
  gint                    next_display_ID;

  GList                  *image_windows;

  PicmanBuffer             *global_buffer;
  PicmanContainer          *named_buffers;

  PicmanContainer          *fonts;

  PicmanDataFactory        *brush_factory;
  PicmanDataFactory        *dynamics_factory;
  PicmanDataFactory        *pattern_factory;
  PicmanDataFactory        *gradient_factory;
  PicmanDataFactory        *palette_factory;
  PicmanDataFactory        *tool_preset_factory;

  PicmanTagCache           *tag_cache;

  PicmanPDB                *pdb;

  PicmanContainer          *tool_info_list;
  PicmanToolInfo           *standard_tool_info;

  /*  the opened and saved images in MRU order  */
  PicmanContainer          *documents;

  /*  image_new values  */
  PicmanContainer          *templates;
  PicmanTemplate           *image_new_last_template;

  /*  the list of all contexts  */
  GList                  *context_list;

  /*  the default context which is initialized from picmanrc  */
  PicmanContext            *default_context;

  /*  the context used by the interface  */
  PicmanContext            *user_context;
};

struct _PicmanClass
{
  PicmanObjectClass  parent_class;

  void     (* initialize)     (Picman               *picman,
                               PicmanInitStatusFunc  status_callback);
  void     (* restore)        (Picman               *picman,
                               PicmanInitStatusFunc  status_callback);
  gboolean (* exit)           (Picman               *picman,
                               gboolean            force);

  void     (* buffer_changed) (Picman               *picman);

  /*  emitted if an image is loaded and opened with a display  */
  void     (* image_opened)   (Picman               *picman,
                               const gchar        *uri);
};


GType          picman_get_type             (void) G_GNUC_CONST;

Picman         * picman_new                  (const gchar         *name,
                                          const gchar         *session_name,
                                          const gchar         *default_folder,
                                          gboolean             be_verbose,
                                          gboolean             no_data,
                                          gboolean             no_fonts,
                                          gboolean             no_interface,
                                          gboolean             use_shm,
                                          gboolean             console_messages,
                                          PicmanStackTraceMode   stack_trace_mode,
                                          PicmanPDBCompatMode    pdb_compat_mode);
void           picman_set_show_gui         (Picman                *picman,
                                          gboolean             show_gui);
gboolean       picman_get_show_gui         (Picman                *picman);

void           picman_load_config          (Picman                *picman,
                                          const gchar         *alternate_system_picmanrc,
                                          const gchar         *alternate_picmanrc);
void           picman_initialize           (Picman                *picman,
                                          PicmanInitStatusFunc   status_callback);
void           picman_restore              (Picman                *picman,
                                          PicmanInitStatusFunc   status_callback);
gboolean       picman_is_restored          (Picman                *picman);

void           picman_exit                 (Picman                *picman,
                                          gboolean             force);

GList        * picman_get_image_iter       (Picman                *picman);
GList        * picman_get_display_iter     (Picman                *picman);
GList        * picman_get_image_windows    (Picman                *picman);
GList        * picman_get_paint_info_iter  (Picman                *picman);
GList        * picman_get_tool_info_iter   (Picman                *picman);

void           picman_set_global_buffer    (Picman                *picman,
                                          PicmanBuffer          *buffer);

PicmanImage    * picman_create_image         (Picman                *picman,
                                          gint                 width,
                                          gint                 height,
                                          PicmanImageBaseType    type,
                                          PicmanPrecision        precision,
                                          gboolean             attach_comment);

void           picman_set_default_context  (Picman                *picman,
                                          PicmanContext         *context);
PicmanContext  * picman_get_default_context  (Picman                *picman);

void           picman_set_user_context     (Picman                *picman,
                                          PicmanContext         *context);
PicmanContext  * picman_get_user_context     (Picman                *picman);

PicmanToolInfo * picman_get_tool_info        (Picman                *picman,
                                          const gchar         *tool_name);

void           picman_message              (Picman                *picman,
                                          GObject             *handler,
                                          PicmanMessageSeverity  severity,
                                          const gchar         *format,
                                          ...) G_GNUC_PRINTF(4,5);
void           picman_message_valist       (Picman                *picman,
                                          GObject             *handler,
                                          PicmanMessageSeverity  severity,
                                          const gchar         *format,
                                          va_list              args);
void           picman_message_literal      (Picman                *picman,
                                          GObject             *handler,
                                          PicmanMessageSeverity  severity,
                                          const gchar         *message);

void           picman_image_opened         (Picman                *picman,
                                          const gchar         *uri);

gchar        * picman_get_temp_filename    (Picman                *picman,
                                          const gchar         *extension);


#endif  /* __PICMAN_H__ */
