/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanlanguagecombobox.h
 * Copyright (C) 2009  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_LANGUAGE_COMBO_BOX_H__
#define __PICMAN_LANGUAGE_COMBO_BOX_H__


#define PICMAN_TYPE_LANGUAGE_COMBO_BOX            (picman_language_combo_box_get_type ())
#define PICMAN_LANGUAGE_COMBO_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_LANGUAGE_COMBO_BOX, PicmanLanguageComboBox))
#define PICMAN_LANGUAGE_COMBO_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_LANGUAGE_COMBO_BOX, PicmanLanguageComboBoxClass))
#define PICMAN_IS_LANGUAGE_COMBO_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_LANGUAGE_COMBO_BOX))
#define PICMAN_IS_LANGUAGE_COMBO_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_LANGUAGE_COMBO_BOX))
#define PICMAN_LANGUAGE_COMBO_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_LANGUAGE_COMBO_BOX, PicmanLanguageComboBoxClass))


typedef struct _PicmanLanguageComboBoxClass  PicmanLanguageComboBoxClass;

struct _PicmanLanguageComboBoxClass
{
  GtkComboBoxClass  parent_class;
};


GType       picman_language_combo_box_get_type (void) G_GNUC_CONST;

GtkWidget * picman_language_combo_box_new      (void);

gchar     * picman_language_combo_box_get_code (PicmanLanguageComboBox *combo);
gboolean    picman_language_combo_box_set_code (PicmanLanguageComboBox *combo,
                                              const gchar          *code);


#endif  /* __PICMAN_LANGUAGE_COMBO_BOX_H__ */
