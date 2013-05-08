/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanstringcombobox.h
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

#ifndef __PICMAN_STRING_COMBO_BOX_H__
#define __PICMAN_STRING_COMBO_BOX_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_STRING_COMBO_BOX            (picman_string_combo_box_get_type ())
#define PICMAN_STRING_COMBO_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_STRING_COMBO_BOX, PicmanStringComboBox))
#define PICMAN_STRING_COMBO_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_STRING_COMBO_BOX, PicmanStringComboBoxClass))
#define PICMAN_IS_STRING_COMBO_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_STRING_COMBO_BOX))
#define PICMAN_IS_STRING_COMBO_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_STRING_COMBO_BOX))
#define PICMAN_STRING_COMBO_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_STRING_COMBO_BOX, PicmanStringComboBoxClass))


typedef struct _PicmanStringComboBoxClass  PicmanStringComboBoxClass;

struct _PicmanStringComboBox
{
  GtkComboBox       parent_instance;

  /*< private >*/
  gpointer          priv;
};

struct _PicmanStringComboBoxClass
{
  GtkComboBoxClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_string_combo_box_get_type   (void) G_GNUC_CONST;

GtkWidget * picman_string_combo_box_new        (GtkTreeModel       *model,
                                              gint                id_column,
                                              gint                label_column);
gboolean    picman_string_combo_box_set_active (PicmanStringComboBox *combo_box,
                                              const gchar        *id);
gchar     * picman_string_combo_box_get_active (PicmanStringComboBox *combo_box);


G_END_DECLS

#endif  /* __PICMAN_STRING_COMBO_BOX_H__ */
