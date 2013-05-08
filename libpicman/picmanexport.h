/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1999 Peter Mattis and Spencer Kimball
 *
 * picmanexport.h
 * Copyright (C) 1999-2000 Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_UI_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picmanui.h> can be included directly."
#endif

#ifndef __PICMAN_EXPORT_H__
#define __PICMAN_EXPORT_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


typedef enum
{
  PICMAN_EXPORT_CAN_HANDLE_RGB                 = 1 << 0,
  PICMAN_EXPORT_CAN_HANDLE_GRAY                = 1 << 1,
  PICMAN_EXPORT_CAN_HANDLE_INDEXED             = 1 << 2,
  PICMAN_EXPORT_CAN_HANDLE_BITMAP              = 1 << 3,
  PICMAN_EXPORT_CAN_HANDLE_ALPHA               = 1 << 4,
  PICMAN_EXPORT_CAN_HANDLE_LAYERS              = 1 << 5,
  PICMAN_EXPORT_CAN_HANDLE_LAYERS_AS_ANIMATION = 1 << 6,
  PICMAN_EXPORT_CAN_HANDLE_LAYER_MASKS         = 1 << 7,
  PICMAN_EXPORT_NEEDS_ALPHA                    = 1 << 8
} PicmanExportCapabilities;

typedef enum
{
  PICMAN_EXPORT_CANCEL,
  PICMAN_EXPORT_IGNORE,
  PICMAN_EXPORT_EXPORT
} PicmanExportReturn;

PicmanExportReturn   picman_export_image                   (gint32                 *image_ID,
                                                        gint32                 *drawable_ID,
                                                        const gchar            *format_name,
                                                        PicmanExportCapabilities  capabilities);
GtkWidget        * picman_export_dialog_new              (const gchar            *format_name,
                                                        const gchar            *role,
                                                        const gchar            *help_id);
GtkWidget        * picman_export_dialog_get_content_area (GtkWidget              *dialog);


G_END_DECLS

#endif /* __PICMAN_EXPORT_H__ */
