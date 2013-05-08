/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorprofilecombobox.h
 * Copyright (C) 2007  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_COLOR_PROFILE_COMBO_BOX_H__
#define __PICMAN_COLOR_PROFILE_COMBO_BOX_H__

G_BEGIN_DECLS

#define PICMAN_TYPE_COLOR_PROFILE_COMBO_BOX            (picman_color_profile_combo_box_get_type ())
#define PICMAN_COLOR_PROFILE_COMBO_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_PROFILE_COMBO_BOX, PicmanColorProfileComboBox))
#define PICMAN_COLOR_PROFILE_COMBO_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_PROFILE_COMBO_BOX, PicmanColorProfileComboBoxClass))
#define PICMAN_IS_COLOR_PROFILE_COMBO_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_PROFILE_COMBO_BOX))
#define PICMAN_IS_COLOR_PROFILE_COMBO_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_PROFILE_COMBO_BOX))
#define PICMAN_COLOR_PROFILE_COMBO_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_PROFILE_COMBO_BOX, PicmanColorProfileComboBoxClass))


typedef struct _PicmanColorProfileComboBoxClass  PicmanColorProfileComboBoxClass;

struct _PicmanColorProfileComboBox
{
  GtkComboBox       parent_instance;

  GtkWidget        *dialog;
};

struct _PicmanColorProfileComboBoxClass
{
  GtkComboBoxClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_color_profile_combo_box_get_type         (void) G_GNUC_CONST;

GtkWidget * picman_color_profile_combo_box_new              (GtkWidget    *dialog,
                                                           const gchar  *history);
GtkWidget * picman_color_profile_combo_box_new_with_model   (GtkWidget    *dialog,
                                                           GtkTreeModel *model);

void        picman_color_profile_combo_box_add              (PicmanColorProfileComboBox *combo,
                                                           const gchar              *filename,
                                                           const gchar              *label);
void        picman_color_profile_combo_box_set_active       (PicmanColorProfileComboBox *combo,
                                                           const gchar              *filename,
                                                           const gchar              *label);
gchar *     picman_color_profile_combo_box_get_active       (PicmanColorProfileComboBox *combo);


G_END_DECLS

#endif  /* __PICMAN_COLOR_PROFILE_COMBO_BOX_H__ */
