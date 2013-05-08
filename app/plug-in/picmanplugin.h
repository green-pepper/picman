/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanplugin.h
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

#ifndef __PICMAN_PLUG_IN_H__
#define __PICMAN_PLUG_IN_H__


#include "core/picmanobject.h"
#include "picmanpluginprocframe.h"


#define WRITE_BUFFER_SIZE  512


#define PICMAN_TYPE_PLUG_IN            (picman_plug_in_get_type ())
#define PICMAN_PLUG_IN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PLUG_IN, PicmanPlugIn))
#define PICMAN_PLUG_IN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PLUG_IN, PicmanPlugInClass))
#define PICMAN_IS_PLUG_IN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PLUG_IN))
#define PICMAN_IS_PLUG_IN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PLUG_IN))


typedef struct _PicmanPlugInClass PicmanPlugInClass;

struct _PicmanPlugIn
{
  PicmanObject           parent_instance;

  PicmanPlugInManager   *manager;
  gchar               *prog;            /*  Plug-in's full path name          */

  PicmanPlugInCallMode   call_mode;       /*  QUERY, INIT or RUN                */
  guint                open : 1;        /*  Is the plug-in open?              */
  guint                hup : 1;         /*  Did we receive a G_IO_HUP         */
  guint                precision : 1;   /*  True drawable precision enabled   */
  GPid                 pid;             /*  Plug-in's process id              */

  GIOChannel          *my_read;         /*  App's read and write channels     */
  GIOChannel          *my_write;
  GIOChannel          *his_read;        /*  Plug-in's read and write channels */
  GIOChannel          *his_write;

  guint                input_id;        /*  Id of input proc                  */

  gchar                write_buffer[WRITE_BUFFER_SIZE]; /* Buffer for writing */
  gint                 write_buffer_index;              /* Buffer index       */

  GSList              *temp_procedures; /*  Temporary procedures              */

  GMainLoop           *ext_main_loop;   /*  for waiting for extension_ack     */

  PicmanPlugInProcFrame  main_proc_frame;

  GList               *temp_proc_frames;

  PicmanPlugInDef       *plug_in_def;     /*  Valid during query() and init()   */
};

struct _PicmanPlugInClass
{
  PicmanObjectClass  parent_class;
};


GType         picman_plug_in_get_type          (void) G_GNUC_CONST;

PicmanPlugIn  * picman_plug_in_new               (PicmanPlugInManager      *manager,
                                              PicmanContext            *context,
                                              PicmanProgress           *progress,
                                              PicmanPlugInProcedure    *procedure,
                                              const gchar            *prog);

gboolean      picman_plug_in_open              (PicmanPlugIn             *plug_in,
                                              PicmanPlugInCallMode      call_mode,
                                              gboolean                synchronous);
void          picman_plug_in_close             (PicmanPlugIn             *plug_in,
                                              gboolean                kill_it);

PicmanPlugInProcFrame *
              picman_plug_in_get_proc_frame    (PicmanPlugIn             *plug_in);

PicmanPlugInProcFrame *
              picman_plug_in_proc_frame_push   (PicmanPlugIn             *plug_in,
                                              PicmanContext            *context,
                                              PicmanProgress           *progress,
                                              PicmanTemporaryProcedure *procedure);
void          picman_plug_in_proc_frame_pop    (PicmanPlugIn             *plug_in);

void          picman_plug_in_main_loop         (PicmanPlugIn             *plug_in);
void          picman_plug_in_main_loop_quit    (PicmanPlugIn             *plug_in);

const gchar * picman_plug_in_get_undo_desc     (PicmanPlugIn             *plug_in);

gboolean      picman_plug_in_menu_register     (PicmanPlugIn             *plug_in,
                                              const gchar            *proc_name,
                                              const gchar            *menu_path);

void          picman_plug_in_add_temp_proc     (PicmanPlugIn             *plug_in,
                                              PicmanTemporaryProcedure *procedure);
void          picman_plug_in_remove_temp_proc  (PicmanPlugIn             *plug_in,
                                              PicmanTemporaryProcedure *procedure);

void          picman_plug_in_set_error_handler (PicmanPlugIn             *plug_in,
                                              PicmanPDBErrorHandler     handler);
PicmanPDBErrorHandler
              picman_plug_in_get_error_handler (PicmanPlugIn             *plug_in);

void          picman_plug_in_enable_precision  (PicmanPlugIn             *plug_in);
gboolean      picman_plug_in_precision_enabled (PicmanPlugIn             *plug_in);


#endif /* __PICMAN_PLUG_IN_H__ */
