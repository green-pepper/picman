/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanenumcombobox.h
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
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_ENUM_COMBO_BOX_H__
#define __PICMAN_ENUM_COMBO_BOX_H__

#include <libpicmanwidgets/picmanintcombobox.h>

G_BEGIN_DECLS

#define PICMAN_TYPE_ENUM_COMBO_BOX            (picman_enum_combo_box_get_type ())
#define PICMAN_ENUM_COMBO_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ENUM_COMBO_BOX, PicmanEnumComboBox))
#define PICMAN_ENUM_COMBO_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ENUM_COMBO_BOX, PicmanEnumComboBoxClass))
#define PICMAN_IS_ENUM_COMBO_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ENUM_COMBO_BOX))
#define PICMAN_IS_ENUM_COMBO_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ENUM_COMBO_BOX))
#define PICMAN_ENUM_COMBO_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ENUM_COMBO_BOX, PicmanEnumComboBoxClass))


typedef struct _PicmanEnumComboBoxClass  PicmanEnumComboBoxClass;

struct _PicmanEnumComboBox
{
  PicmanIntComboBox       parent_instance;
};

struct _PicmanEnumComboBoxClass
{
  PicmanIntComboBoxClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_enum_combo_box_get_type         (void) G_GNUC_CONST;

GtkWidget * picman_enum_combo_box_new              (GType             enum_type);
GtkWidget * picman_enum_combo_box_new_with_model   (PicmanEnumStore    *enum_store);

void        picman_enum_combo_box_set_stock_prefix (PicmanEnumComboBox *combo_box,
                                                  const gchar      *stock_prefix);

G_END_DECLS

#endif  /* __PICMAN_ENUM_COMBO_BOX_H__ */
