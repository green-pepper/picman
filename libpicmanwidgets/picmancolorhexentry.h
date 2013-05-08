/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorhexentry.h
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_COLOR_HEX_ENTRY_H__
#define __PICMAN_COLOR_HEX_ENTRY_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_COLOR_HEX_ENTRY            (picman_color_hex_entry_get_type ())
#define PICMAN_COLOR_HEX_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_HEX_ENTRY, PicmanColorHexEntry))
#define PICMAN_COLOR_HEX_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_HEX_ENTRY, PicmanColorHexEntryClass))
#define PICMAN_IS_COLOR_HEX_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_HEX_ENTRY))
#define PICMAN_IS_COLOR_HEX_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_HEX_ENTRY))
#define PICMAN_COLOR_HEX_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_HEX_AREA, PicmanColorHexEntryClass))


typedef struct _PicmanColorHexEntryClass  PicmanColorHexEntryClass;

struct _PicmanColorHexEntry
{
  GtkEntry        parent_instance;

  PicmanRGB         color;
};

struct _PicmanColorHexEntryClass
{
  GtkEntryClass   parent_class;

  void (* color_changed) (PicmanColorHexEntry *entry);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_color_hex_entry_get_type  (void) G_GNUC_CONST;

GtkWidget * picman_color_hex_entry_new       (void);

void        picman_color_hex_entry_set_color (PicmanColorHexEntry *entry,
                                            const PicmanRGB     *color);
void        picman_color_hex_entry_get_color (PicmanColorHexEntry *entry,
                                            PicmanRGB           *color);


G_END_DECLS

#endif /* __PICMAN_COLOR_HEX_ENTRY_H__ */
