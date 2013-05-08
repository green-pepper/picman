/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * file-open.h
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

#ifndef __FILE_OPEN_H__
#define __FILE_OPEN_H__


PicmanImage * file_open_image                 (Picman                *picman,
                                             PicmanContext         *context,
                                             PicmanProgress        *progress,
                                             const gchar         *uri,
                                             const gchar         *entered_filename,
                                             gboolean             as_new,
                                             PicmanPlugInProcedure *file_proc,
                                             PicmanRunMode          run_mode,
                                             PicmanPDBStatusType   *status,
                                             const gchar        **mime_type,
                                             GError             **error);

PicmanImage * file_open_thumbnail             (Picman                *picman,
                                             PicmanContext         *context,
                                             PicmanProgress        *progress,
                                             const gchar         *uri,
                                             gint                 size,
                                             const gchar        **mime_type,
                                             gint                *image_width,
                                             gint                *image_height,
                                             const Babl         **format,
                                             gint                *num_layers,
                                             GError             **error);
PicmanImage * file_open_with_display          (Picman                *picman,
                                             PicmanContext         *context,
                                             PicmanProgress        *progress,
                                             const gchar         *uri,
                                             gboolean             as_new,
                                             PicmanPDBStatusType   *status,
                                             GError             **error);

PicmanImage * file_open_with_proc_and_display (Picman                *picman,
                                             PicmanContext         *context,
                                             PicmanProgress        *progress,
                                             const gchar         *uri,
                                             const gchar         *entered_filename,
                                             gboolean             as_new,
                                             PicmanPlugInProcedure *file_proc,
                                             PicmanPDBStatusType   *status,
                                             GError             **error);

GList     * file_open_layers                (Picman                *picman,
                                             PicmanContext         *context,
                                             PicmanProgress        *progress,
                                             PicmanImage           *dest_image,
                                             gboolean             merge_visible,
                                             const gchar         *uri,
                                             PicmanRunMode          run_mode,
                                             PicmanPlugInProcedure *file_proc,
                                             PicmanPDBStatusType   *status,
                                             GError             **error);

gboolean    file_open_from_command_line     (Picman                *picman,
                                             const gchar         *filename,
                                             gboolean             as_new);


#endif /* __FILE_OPEN_H__ */
