/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanscalecombobox.h
 * Copyright (C) 2004, 2008  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_SCALE_COMBO_BOX_H__
#define __PICMAN_SCALE_COMBO_BOX_H__


#define PICMAN_TYPE_SCALE_COMBO_BOX            (picman_scale_combo_box_get_type ())
#define PICMAN_SCALE_COMBO_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SCALE_COMBO_BOX, PicmanScaleComboBox))
#define PICMAN_SCALE_COMBO_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SCALE_COMBO_BOX, PicmanScaleComboBoxClass))
#define PICMAN_IS_SCALE_COMBO_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SCALE_COMBO_BOX))
#define PICMAN_IS_SCALE_COMBO_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SCALE_COMBO_BOX))
#define PICMAN_SCALE_COMBO_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SCALE_COMBO_BOX, PicmanScaleComboBoxClass))


typedef struct _PicmanScaleComboBoxClass  PicmanScaleComboBoxClass;

struct _PicmanScaleComboBoxClass
{
  GtkComboBoxClass  parent_instance;

  void (* entry_activated) (PicmanScaleComboBox *combo_box);
};

struct _PicmanScaleComboBox
{
  GtkComboBox  parent_instance;

  gdouble      scale;
  GtkTreePath *last_path;
  GList       *mru;
};


GType       picman_scale_combo_box_get_type   (void) G_GNUC_CONST;

GtkWidget * picman_scale_combo_box_new        (void);
void        picman_scale_combo_box_set_scale  (PicmanScaleComboBox *combo_box,
                                             gdouble            scale);
gdouble     picman_scale_combo_box_get_scale  (PicmanScaleComboBox *combo_box);


#endif  /* __PICMAN_SCALE_COMBO_BOX_H__ */
