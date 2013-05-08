/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancombotagentry.h
 * Copyright (C) 2008 Aurimas Juška <aurisj@svn.gnome.org>
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

#ifndef __PICMAN_COMBO_TAG_ENTRY_H__
#define __PICMAN_COMBO_TAG_ENTRY_H__

#include "picmantagentry.h"

#define PICMAN_TYPE_COMBO_TAG_ENTRY            (picman_combo_tag_entry_get_type ())
#define PICMAN_COMBO_TAG_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COMBO_TAG_ENTRY, PicmanComboTagEntry))
#define PICMAN_COMBO_TAG_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COMBO_TAG_ENTRY, PicmanComboTagEntryClass))
#define PICMAN_IS_COMBO_TAG_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COMBO_TAG_ENTRY))
#define PICMAN_IS_COMBO_TAG_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COMBO_TAG_ENTRY))
#define PICMAN_COMBO_TAG_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COMBO_TAG_ENTRY, PicmanComboTagEntryClass))


typedef struct _PicmanComboTagEntryClass  PicmanComboTagEntryClass;

struct _PicmanComboTagEntry
{
  PicmanTagEntry    parent_instance;

  GdkPixbuf      *arrow_pixbuf;

  GtkWidget      *popup;
  PangoAttrList  *normal_item_attr;
  PangoAttrList  *selected_item_attr;
  PangoAttrList  *insensitive_item_attr;
  GdkColor        selected_item_color;
};

struct _PicmanComboTagEntryClass
{
  PicmanTagEntryClass  parent_class;
};


GType       picman_combo_tag_entry_get_type (void) G_GNUC_CONST;

GtkWidget * picman_combo_tag_entry_new      (PicmanTaggedContainer *container,
                                           PicmanTagEntryMode     mode);


#endif  /*  __PICMAN_COMBO_TAG_ENTRY_H__  */
