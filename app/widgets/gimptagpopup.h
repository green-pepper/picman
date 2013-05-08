/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantagpopup.h
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

#ifndef __PICMAN_TAG_POPUP_H__
#define __PICMAN_TAG_POPUP_H__


#define PICMAN_TYPE_TAG_POPUP            (picman_tag_popup_get_type ())
#define PICMAN_TAG_POPUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TAG_POPUP, PicmanTagPopup))
#define PICMAN_TAG_POPUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TAG_POPUP, PicmanTagPopupClass))
#define PICMAN_IS_TAG_POPUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TAG_POPUP))
#define PICMAN_IS_TAG_POPUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TAG_POPUP))
#define PICMAN_TAG_POPUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TAG_POPUP, PicmanTagPopupClass))


typedef struct _PicmanTagPopupClass  PicmanTagPopupClass;

typedef struct _PopupTagData PopupTagData;

struct _PicmanTagPopup
{
  GtkWindow          parent_instance;

  PicmanComboTagEntry *combo_entry;

  GtkWidget         *frame;
  GtkWidget         *alignment;
  GtkWidget         *tag_area;

  PangoContext      *context;
  PangoLayout       *layout;

  PopupTagData      *tag_data;
  gint               tag_count;

  PopupTagData      *prelight;

  gboolean           single_select_disabled;

  guint              scroll_timeout_id;
  gint               scroll_height;
  gint               scroll_y;
  gint               scroll_step;
  gint               scroll_arrow_height;
  gboolean           scroll_fast;
  gboolean           arrows_visible;
  gboolean           upper_arrow_prelight;
  gboolean           lower_arrow_prelight;
  GtkStateType       upper_arrow_state;
  GtkStateType       lower_arrow_state;
};

struct _PicmanTagPopupClass
{
  GtkWindowClass  parent_class;
};


GType       picman_tag_popup_get_type (void) G_GNUC_CONST;

GtkWidget * picman_tag_popup_new      (PicmanComboTagEntry *entry);

void        picman_tag_popup_show     (PicmanTagPopup      *popup);


#endif  /*  __PICMAN_TAG_POPUP_H__  */
