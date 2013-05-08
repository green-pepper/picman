/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanplugin-progress.h
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

#ifndef __PICMAN_PLUG_IN_PROGRESS_H__
#define __PICMAN_PLUG_IN_PROGRESS_H__


gint       picman_plug_in_progress_attach        (PicmanProgress        *progress);
gint       picman_plug_in_progress_detach        (PicmanProgress        *progress);

void       picman_plug_in_progress_start         (PicmanPlugIn          *plug_in,
                                                const gchar         *message,
                                                PicmanObject          *display);
void       picman_plug_in_progress_end           (PicmanPlugIn          *plug_in,
                                                PicmanPlugInProcFrame *proc_frame);
void       picman_plug_in_progress_set_text      (PicmanPlugIn          *plug_in,
                                                const gchar         *message);
void       picman_plug_in_progress_set_value     (PicmanPlugIn          *plug_in,
                                                gdouble              percentage);
void       picman_plug_in_progress_pulse         (PicmanPlugIn          *plug_in);
guint32    picman_plug_in_progress_get_window_id (PicmanPlugIn          *plug_in);

gboolean   picman_plug_in_progress_install       (PicmanPlugIn          *plug_in,
                                                const gchar         *progress_callback);
gboolean   picman_plug_in_progress_uninstall     (PicmanPlugIn          *plug_in,
                                                const gchar         *progress_callback);
gboolean   picman_plug_in_progress_cancel        (PicmanPlugIn          *plug_in,
                                                const gchar         *progress_callback);


#endif /* __PICMAN_PLUG_IN_PROGRESS_H__ */
