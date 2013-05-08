/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansettingsbox.h
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_SETTINGS_BOX_H__
#define __PICMAN_SETTINGS_BOX_H__


#define PICMAN_TYPE_SETTINGS_BOX            (picman_settings_box_get_type ())
#define PICMAN_SETTINGS_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SETTINGS_BOX, PicmanSettingsBox))
#define PICMAN_SETTINGS_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SETTINGS_BOX, PicmanSettingsBoxClass))
#define PICMAN_IS_SETTINGS_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SETTINGS_BOX))
#define PICMAN_IS_SETTINGS_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SETTINGS_BOX))
#define PICMAN_SETTINGS_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SETTINGS_BOX, PicmanSettingsBoxClass))


typedef struct _PicmanSettingsBoxClass PicmanSettingsBoxClass;

struct _PicmanSettingsBox
{
  GtkBox  parent_instance;
};

struct _PicmanSettingsBoxClass
{
  GtkBoxClass  parent_class;

  /*  signals  */
  void (* file_dialog_setup) (PicmanSettingsBox      *box,
                              GtkFileChooserDialog *dialog,
                              gboolean              export);
  void (* import)            (PicmanSettingsBox      *box,
                              const gchar          *filename);
  void (* export)            (PicmanSettingsBox      *box,
                              const gchar          *filename);
};


GType       picman_settings_box_get_type    (void) G_GNUC_CONST;

GtkWidget * picman_settings_box_new         (Picman            *picman,
                                           GObject         *config,
                                           PicmanContainer   *container,
                                           const gchar     *filename,
                                           const gchar     *import_dialog_title,
                                           const gchar     *export_dialog_title,
                                           const gchar     *file_dialog_help_id,
                                           const gchar     *default_folder,
                                           const gchar     *last_filename);

void        picman_settings_box_add_current (PicmanSettingsBox *box,
                                           gint             max_recent);

GtkWidget * picman_settings_box_get_combo   (PicmanSettingsBox *box);


#endif  /*  __PICMAN_SETTINGS_BOX_H__  */
