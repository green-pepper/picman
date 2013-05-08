/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanprefsbox.h
 * Copyright (C) 2013 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_PREFS_BOX_H__
#define __PICMAN_PREFS_BOX_H__


#define PICMAN_TYPE_PREFS_BOX            (picman_prefs_box_get_type ())
#define PICMAN_PREFS_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PREFS_BOX, PicmanPrefsBox))
#define PICMAN_PREFS_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PREFS_BOX, PicmanPrefsBoxClass))
#define PICMAN_IS_PREFS_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PREFS_BOX))
#define PICMAN_IS_PREFS_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PREFS_BOX))
#define PICMAN_PREFS_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PREFS_BOX, PicmanPrefsBoxClass))


typedef struct _PicmanPrefsBoxClass PicmanPrefsBoxClass;

struct _PicmanPrefsBox
{
  GtkBox  parent_instance;
};

struct _PicmanPrefsBoxClass
{
  GtkBoxClass  parent_class;
};


GType       picman_prefs_box_get_type      (void) G_GNUC_CONST;

GtkWidget * picman_prefs_box_new           (void);

GtkWidget * picman_prefs_box_add_page      (PicmanPrefsBox *box,
                                          const gchar  *notebook_label,
                                          GdkPixbuf    *notebook_icon,
                                          const gchar  *tree_label,
                                          GdkPixbuf    *tree_icon,
                                          const gchar  *help_id,
                                          GtkTreeIter  *parent,
                                          GtkTreeIter  *iter);

GtkWidget * picman_prefs_box_get_tree_view (PicmanPrefsBox *box);
GtkWidget * picman_prefs_box_get_notebook  (PicmanPrefsBox *box);
GtkWidget * picman_prefs_box_get_image     (PicmanPrefsBox *box);


#endif  /*  __PICMAN_PREFS_BOX_H__  */
