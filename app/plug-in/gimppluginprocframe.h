/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginprocframe.h
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

#ifndef __PICMAN_PLUG_IN_PROC_FRAME_H__
#define __PICMAN_PLUG_IN_PRON_FRAME_H__


struct _PicmanPlugInProcFrame
{
  gint                 ref_count;

  PicmanContext         *main_context;
  GList               *context_stack;

  PicmanProcedure       *procedure;
  GMainLoop           *main_loop;

  PicmanValueArray      *return_vals;

  PicmanProgress        *progress;
  gboolean             progress_created;
  gulong               progress_cancel_id;

  PicmanPDBErrorHandler  error_handler;

  /*  lists of things to clean up on dispose  */
  GList               *image_cleanups;
  GList               *item_cleanups;
};


PicmanPlugInProcFrame * picman_plug_in_proc_frame_new     (PicmanContext         *context,
                                                       PicmanProgress        *progress,
                                                       PicmanPlugInProcedure *procedure);
void                  picman_plug_in_proc_frame_init    (PicmanPlugInProcFrame *proc_frame,
                                                       PicmanContext         *context,
                                                       PicmanProgress        *progress,
                                                       PicmanPlugInProcedure *procedure);

void                  picman_plug_in_proc_frame_dispose (PicmanPlugInProcFrame *proc_frame,
                                                       PicmanPlugIn          *plug_in);

PicmanPlugInProcFrame * picman_plug_in_proc_frame_ref     (PicmanPlugInProcFrame *proc_frame);
void                  picman_plug_in_proc_frame_unref   (PicmanPlugInProcFrame *proc_frame,
                                                       PicmanPlugIn          *plug_in);

PicmanValueArray      * picman_plug_in_proc_frame_get_return_values
                                                      (PicmanPlugInProcFrame *proc_frame);


#endif /* __PICMAN_PLUG_IN_PROC_FRAME_H__ */
