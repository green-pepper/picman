/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanProfileChooserDialog
 * Copyright (C) 2006 Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_PROFILE_CHOOSER_DIALOG_H__
#define __PICMAN_PROFILE_CHOOSER_DIALOG_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_PROFILE_CHOOSER_DIALOG            (picman_profile_chooser_dialog_get_type ())
#define PICMAN_PROFILE_CHOOSER_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PROFILE_CHOOSER_DIALOG, PicmanProfileChooserDialog))
#define PICMAN_PROFILE_CHOOSER_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PROFILE_CHOOSER_DIALOG, PicmanProfileChooserDialogClass))
#define PICMAN_IS_PROFILE_CHOOSER_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PROFILE_CHOOSER_DIALOG))
#define PICMAN_IS_PROFILE_CHOOSER_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PROFILE_CHOOSER_DIALOG))
#define PICMAN_PROFILE_CHOOSER_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PROFILE_CHOOSER_DIALOG, PicmanProfileChooserDialogClass))


typedef struct _PicmanProfileChooserDialogClass  PicmanProfileChooserDialogClass;

struct _PicmanProfileChooserDialog
{
  GtkFileChooserDialog  parent_instance;

  Picman                 *picman;
  GtkTextBuffer        *buffer;

  gchar                *filename;
  gchar                *desc;

  guint                 idle_id;
};

struct _PicmanProfileChooserDialogClass
{
  GtkFileChooserDialogClass  parent_class;
};


GType       picman_profile_chooser_dialog_get_type (void) G_GNUC_CONST;

GtkWidget * picman_profile_chooser_dialog_new      (Picman        *picman,
                                                  const gchar *title);

gchar     * picman_profile_chooser_dialog_get_desc (PicmanProfileChooserDialog *dialog,
                                                  const gchar              *uri);

#endif /* __PICMAN_PROFILE_CHOOSER_DIALOG_H__ */
