/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanplugin-cleanup.h
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

#ifndef __PICMAN_PLUG_IN_CLEANUP_H__
#define __PICMAN_PLUG_IN_CLEANUP_H__


gboolean   picman_plug_in_cleanup_undo_group_start (PicmanPlugIn          *plug_in,
                                                  PicmanImage           *image);
gboolean   picman_plug_in_cleanup_undo_group_end   (PicmanPlugIn          *plug_in,
                                                  PicmanImage           *image);

gboolean   picman_plug_in_cleanup_add_shadow       (PicmanPlugIn          *plug_in,
                                                  PicmanDrawable        *drawable);
gboolean   picman_plug_in_cleanup_remove_shadow    (PicmanPlugIn          *plug_in,
                                                  PicmanDrawable        *drawable);

void       picman_plug_in_cleanup                  (PicmanPlugIn          *plug_in,
                                                  PicmanPlugInProcFrame *proc_frame);


#endif /* __PICMAN_PLUG_IN_CLEANUP_H__ */
