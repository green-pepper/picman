/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_PALETTE_H__
#define __PICMAN_PALETTE_H__


#include "picmandata.h"


#define PICMAN_TYPE_PALETTE            (picman_palette_get_type ())
#define PICMAN_PALETTE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PALETTE, PicmanPalette))
#define PICMAN_PALETTE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PALETTE, PicmanPaletteClass))
#define PICMAN_IS_PALETTE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PALETTE))
#define PICMAN_IS_PALETTE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PALETTE))
#define PICMAN_PALETTE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PALETTE, PicmanPaletteClass))


struct _PicmanPaletteEntry
{
  PicmanRGB  color;
  gchar   *name;

  /* EEK */
  gint     position;
};


typedef struct _PicmanPaletteClass PicmanPaletteClass;

struct _PicmanPalette
{
  PicmanData  parent_instance;

  GList    *colors;
  gint      n_colors;

  gint      n_columns;
};

struct _PicmanPaletteClass
{
  PicmanDataClass  parent_class;
};


GType              picman_palette_get_type        (void) G_GNUC_CONST;

PicmanData         * picman_palette_new             (PicmanContext      *context,
                                                 const gchar      *name);
PicmanData         * picman_palette_get_standard    (PicmanContext      *context);

GList            * picman_palette_get_colors      (PicmanPalette      *palette);
gint               picman_palette_get_n_colors    (PicmanPalette      *palette);

PicmanPaletteEntry * picman_palette_add_entry       (PicmanPalette      *palette,
                                                 gint              position,
                                                 const gchar      *name,
                                                 const PicmanRGB    *color);
void               picman_palette_delete_entry    (PicmanPalette      *palette,
                                                 PicmanPaletteEntry *entry);

gboolean           picman_palette_set_entry       (PicmanPalette      *palette,
                                                 gint              position,
                                                 const gchar      *name,
                                                 const PicmanRGB    *color);
gboolean           picman_palette_set_entry_color (PicmanPalette      *palette,
                                                 gint              position,
                                                 const PicmanRGB    *color);
gboolean           picman_palette_set_entry_name  (PicmanPalette      *palette,
                                                 gint              position,
                                                 const gchar      *name);
PicmanPaletteEntry * picman_palette_get_entry       (PicmanPalette      *palette,
                                                 gint              position);

void               picman_palette_set_columns     (PicmanPalette      *palette,
                                                 gint              columns);
gint               picman_palette_get_columns     (PicmanPalette      *palette);

PicmanPaletteEntry * picman_palette_find_entry      (PicmanPalette      *palette,
                                                 const PicmanRGB    *color,
                                                 PicmanPaletteEntry *start_from);


#endif /* __PICMAN_PALETTE_H__ */
