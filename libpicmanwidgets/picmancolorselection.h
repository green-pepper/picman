/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorselection.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_COLOR_SELECTION_H__
#define __PICMAN_COLOR_SELECTION_H__

G_BEGIN_DECLS

/* For information look at the html documentation */


#define PICMAN_TYPE_COLOR_SELECTION            (picman_color_selection_get_type ())
#define PICMAN_COLOR_SELECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_SELECTION, PicmanColorSelection))
#define PICMAN_COLOR_SELECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_SELECTION, PicmanColorSelectionClass))
#define PICMAN_IS_COLOR_SELECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_SELECTION))
#define PICMAN_IS_COLOR_SELECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_SELECTION))
#define PICMAN_COLOR_SELECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_SELECTION, PicmanColorSelectionClass))


typedef struct _PicmanColorSelectionClass PicmanColorSelectionClass;

struct _PicmanColorSelection
{
  GtkBox                    parent_instance;

  gboolean                  show_alpha;

  PicmanHSV                   hsv;
  PicmanRGB                   rgb;
  PicmanColorSelectorChannel  channel;

  GtkWidget                *left_vbox;
  GtkWidget                *right_vbox;

  GtkWidget                *notebook;
  GtkWidget                *scales;

  GtkWidget                *new_color;
  GtkWidget                *old_color;
};

struct _PicmanColorSelectionClass
{
  GtkBoxClass  parent_class;

  void (* color_changed) (PicmanColorSelection *selection);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_color_selection_get_type       (void) G_GNUC_CONST;

GtkWidget * picman_color_selection_new            (void);

void        picman_color_selection_set_show_alpha (PicmanColorSelection *selection,
                                                 gboolean           show_alpha);
gboolean    picman_color_selection_get_show_alpha (PicmanColorSelection *selection);

void        picman_color_selection_set_color      (PicmanColorSelection *selection,
                                                 const PicmanRGB      *color);
void        picman_color_selection_get_color      (PicmanColorSelection *selection,
                                                 PicmanRGB            *color);

void        picman_color_selection_set_old_color  (PicmanColorSelection *selection,
                                                 const PicmanRGB      *color);
void        picman_color_selection_get_old_color  (PicmanColorSelection *selection,
                                                 PicmanRGB            *color);

void        picman_color_selection_reset          (PicmanColorSelection *selection);

void        picman_color_selection_color_changed  (PicmanColorSelection *selection);

void        picman_color_selection_set_config     (PicmanColorSelection *selection,
                                                 PicmanColorConfig    *config);


G_END_DECLS

#endif /* __PICMAN_COLOR_SELECTION_H__ */
