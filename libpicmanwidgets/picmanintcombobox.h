/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanintcombobox.h
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

#ifndef __PICMAN_INT_COMBO_BOX_H__
#define __PICMAN_INT_COMBO_BOX_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_INT_COMBO_BOX            (picman_int_combo_box_get_type ())
#define PICMAN_INT_COMBO_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_INT_COMBO_BOX, PicmanIntComboBox))
#define PICMAN_INT_COMBO_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_INT_COMBO_BOX, PicmanIntComboBoxClass))
#define PICMAN_IS_INT_COMBO_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_INT_COMBO_BOX))
#define PICMAN_IS_INT_COMBO_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_INT_COMBO_BOX))
#define PICMAN_INT_COMBO_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_INT_COMBO_BOX, PicmanIntComboBoxClass))


typedef struct _PicmanIntComboBoxClass  PicmanIntComboBoxClass;

struct _PicmanIntComboBox
{
  GtkComboBox       parent_instance;

  /*< private >*/
  gpointer          priv;

  /* Padding for future expansion (should have gone to the class) */
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};

struct _PicmanIntComboBoxClass
{
  GtkComboBoxClass  parent_class;
};


typedef  gboolean (* PicmanIntSensitivityFunc) (gint      value,
                                              gpointer  data);



GType       picman_int_combo_box_get_type        (void) G_GNUC_CONST;

GtkWidget * picman_int_combo_box_new             (const gchar     *first_label,
                                                gint             first_value,
                                                ...) G_GNUC_NULL_TERMINATED;
GtkWidget * picman_int_combo_box_new_valist      (const gchar     *first_label,
                                                gint             first_value,
                                                va_list          values);

GtkWidget * picman_int_combo_box_new_array       (gint             n_values,
                                                const gchar     *labels[]);

void        picman_int_combo_box_prepend         (PicmanIntComboBox *combo_box,
                                                ...);
void        picman_int_combo_box_append          (PicmanIntComboBox *combo_box,
                                                ...);

gboolean    picman_int_combo_box_set_active      (PicmanIntComboBox *combo_box,
                                                gint             value);
gboolean    picman_int_combo_box_get_active      (PicmanIntComboBox *combo_box,
                                                gint            *value);

gulong      picman_int_combo_box_connect         (PicmanIntComboBox *combo_box,
                                                gint             value,
                                                GCallback        callback,
                                                gpointer         data);

void        picman_int_combo_box_set_sensitivity (PicmanIntComboBox        *combo_box,
                                                PicmanIntSensitivityFunc  func,
                                                gpointer                data,
                                                GDestroyNotify          destroy);


G_END_DECLS

#endif  /* __PICMAN_INT_COMBO_BOX_H__ */
