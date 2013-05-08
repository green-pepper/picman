/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpaletteview.h
 * Copyright (C) 2005 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_PALETTE_VIEW_H__
#define __PICMAN_PALETTE_VIEW_H__

#include "picmanview.h"


#define PICMAN_TYPE_PALETTE_VIEW            (picman_palette_view_get_type ())
#define PICMAN_PALETTE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PALETTE_VIEW, PicmanPaletteView))
#define PICMAN_PALETTE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PALETTE_VIEW, PicmanPaletteViewClass))
#define PICMAN_IS_PALETTE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_PALETTE_VIEW))
#define PICMAN_IS_PALETTE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PALETTE_VIEW))
#define PICMAN_PALETTE_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PALETTE_VIEW, PicmanPaletteViewClass))


typedef struct _PicmanPaletteViewClass  PicmanPaletteViewClass;

struct _PicmanPaletteView
{
  PicmanView          parent_instance;

  PicmanPaletteEntry *selected;
  PicmanPaletteEntry *dnd_entry;
};

struct _PicmanPaletteViewClass
{
  PicmanViewClass  parent_class;

  void (* entry_clicked)   (PicmanPaletteView  *view,
                            PicmanPaletteEntry *entry,
                            GdkModifierType   state);
  void (* entry_selected)  (PicmanPaletteView  *view,
                            PicmanPaletteEntry *entry);
  void (* entry_activated) (PicmanPaletteView  *view,
                            PicmanPaletteEntry *entry);
  void (* entry_context)   (PicmanPaletteView  *view,
                            PicmanPaletteEntry *entry);
  void (* color_dropped)   (PicmanPaletteView  *view,
                            PicmanPaletteEntry *entry,
                            const PicmanRGB    *color);
};


GType   picman_palette_view_get_type     (void) G_GNUC_CONST;

void    picman_palette_view_select_entry (PicmanPaletteView  *view,
                                        PicmanPaletteEntry *entry);


#endif /* __PICMAN_PALETTE_VIEW_H__ */
