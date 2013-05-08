/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewabledialog.h
 * Copyright (C) 2000 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_VIEWABLE_DIALOG_H__
#define __PICMAN_VIEWABLE_DIALOG_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_VIEWABLE_DIALOG            (picman_viewable_dialog_get_type ())
#define PICMAN_VIEWABLE_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VIEWABLE_DIALOG, PicmanViewableDialog))
#define PICMAN_VIEWABLE_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VIEWABLE_DIALOG, PicmanViewableDialogClass))
#define PICMAN_IS_VIEWABLE_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_VIEWABLE_DIALOG))
#define PICMAN_IS_VIEWABLE_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VIEWABLE_DIALOG))
#define PICMAN_VIEWABLE_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VIEWABLE_DIALOG, PicmanViewableDialogClass))


typedef struct _PicmanViewableDialogClass  PicmanViewableDialogClass;

struct _PicmanViewableDialog
{
  PicmanDialog   parent_instance;

  PicmanContext *context;

  GtkWidget   *icon;
  GtkWidget   *view;
  GtkWidget   *desc_label;
  GtkWidget   *viewable_label;
};

struct _PicmanViewableDialogClass
{
  PicmanDialogClass  parent_class;
};


GType       picman_viewable_dialog_get_type (void) G_GNUC_CONST;

GtkWidget * picman_viewable_dialog_new      (PicmanViewable       *viewable,
                                           PicmanContext        *context,
                                           const gchar        *title,
                                           const gchar        *role,
                                           const gchar        *stock_id,
                                           const gchar        *desc,
                                           GtkWidget          *parent,
                                           PicmanHelpFunc        help_func,
                                           const gchar        *help_id,
                                           ...) G_GNUC_NULL_TERMINATED;

void    picman_viewable_dialog_set_viewable (PicmanViewableDialog *dialog,
                                           PicmanViewable       *viewable,
                                           PicmanContext        *context);


G_END_DECLS

#endif /* __PICMAN_VIEWABLE_DIALOG_H__ */
