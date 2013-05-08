/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmaniconpicker.h
 * Copyright (C) 2011 Michael Natterer <mitch@picman.org>
 *               2012 Daniel Sabo
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

#ifndef __PICMAN_ICON_PICKER_H__
#define __PICMAN_ICON_PICKER_H__


#define PICMAN_TYPE_ICON_PICKER            (picman_icon_picker_get_type ())
#define PICMAN_ICON_PICKER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ICON_PICKER, PicmanIconPicker))
#define PICMAN_ICON_PICKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ICON_PICKER, PicmanIconPickerClass))
#define PICMAN_IS_ICON_PICKER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ICON_PICKER))
#define PICMAN_IS_ICON_PICKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ICON_PICKER))
#define PICMAN_ICON_PICKER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ICON_PICKER, PicmanIconPickerClass))


typedef struct _PicmanIconPickerClass PicmanIconPickerClass;

struct _PicmanIconPicker
{
  GtkBox  parent_instance;
};

struct _PicmanIconPickerClass
{
  GtkBoxClass   parent_class;
};


GType          picman_icon_picker_get_type    (void) G_GNUC_CONST;

GtkWidget   * picman_icon_picker_new          (Picman           *picman);

const gchar * picman_icon_picker_get_stock_id (PicmanIconPicker *picker);
void          picman_icon_picker_set_stock_id (PicmanIconPicker *picker,
                                             const gchar    *stock_id);

GdkPixbuf   * picman_icon_picker_get_icon_pixbuf (PicmanIconPicker *picker);
void          picman_icon_picker_set_icon_pixbuf (PicmanIconPicker *picker,
                                                GdkPixbuf      *value);


#endif  /*  __PICMAN_ICON_PICKER_H__  */
