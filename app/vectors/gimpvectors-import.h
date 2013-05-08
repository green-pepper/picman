/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanVectors Import
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_VECTORS_IMPORT_H__
#define __PICMAN_VECTORS_IMPORT_H__


gboolean  picman_vectors_import_file   (PicmanImage    *image,
                                      const gchar  *filename,
                                      gboolean      merge,
                                      gboolean      scale,
                                      PicmanVectors  *parent,
                                      gint          position,
                                      GList       **ret_vectors,
                                      GError      **error);
gboolean  picman_vectors_import_buffer (PicmanImage    *image,
                                      const gchar  *buffer,
                                      gsize         len,
                                      gboolean      merge,
                                      gboolean      scale,
                                      PicmanVectors  *parent,
                                      gint          position,
                                      GList       **ret_vectors,
                                      GError      **error);


#endif /* __PICMAN_VECTORS_IMPORT_H__ */
