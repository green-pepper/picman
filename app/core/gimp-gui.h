/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_GUI_H__
#define __PICMAN_GUI_H__


typedef struct _PicmanGui PicmanGui;

struct _PicmanGui
{
  void           (* ungrab)                (Picman                *picman);

  void           (* threads_enter)         (Picman                *picman);
  void           (* threads_leave)         (Picman                *picman);

  void           (* set_busy)              (Picman                *picman);
  void           (* unset_busy)            (Picman                *picman);

  void           (* show_message)          (Picman                *picman,
                                            GObject             *handler,
                                            PicmanMessageSeverity  severity,
                                            const gchar         *domain,
                                            const gchar         *message);
  void           (* help)                  (Picman                *picman,
                                            PicmanProgress        *progress,
                                            const gchar         *help_domain,
                                            const gchar         *help_id);

  const gchar  * (* get_program_class)     (Picman                *picman);
  gchar        * (* get_display_name)      (Picman                *picman,
                                            gint                 display_ID,
                                            gint                *monitor_number);
  guint32        (* get_user_time)         (Picman                *picman);

  const gchar  * (* get_theme_dir)         (Picman                *picman);

  PicmanObject   * (* get_window_strategy)   (Picman                *picman);
  PicmanObject   * (* get_empty_display)     (Picman                *picman);
  PicmanObject   * (* display_get_by_id)     (Picman                *picman,
                                            gint                 ID);
  gint           (* display_get_id)        (PicmanObject          *display);
  guint32        (* display_get_window_id) (PicmanObject          *display);
  PicmanObject   * (* display_create)        (Picman                *picman,
                                            PicmanImage           *image,
                                            PicmanUnit             unit,
                                            gdouble              scale);
  void           (* display_delete)        (PicmanObject          *display);
  void           (* displays_reconnect)    (Picman                *picman,
                                            PicmanImage           *old_image,
                                            PicmanImage           *new_image);

  PicmanProgress * (* progress_new)          (Picman                *picman,
                                            PicmanObject          *display);
  void           (* progress_free)         (Picman                *picman,
                                            PicmanProgress        *progress);

  gboolean       (* pdb_dialog_new)        (Picman                *picman,
                                            PicmanContext         *context,
                                            PicmanProgress        *progress,
                                            PicmanContainer       *container,
                                            const gchar         *title,
                                            const gchar         *callback_name,
                                            const gchar         *object_name,
                                            va_list              args);
  gboolean       (* pdb_dialog_set)        (Picman                *picman,
                                            PicmanContainer       *container,
                                            const gchar         *callback_name,
                                            const gchar         *object_name,
                                            va_list              args);
  gboolean       (* pdb_dialog_close)      (Picman                *picman,
                                            PicmanContainer       *container,
                                            const gchar         *callback_name);
  gboolean       (* recent_list_add_uri)   (Picman                *picman,
                                            const gchar         *uri,
                                            const gchar         *mime_type);
  void           (* recent_list_load)      (Picman                *picman);

};


void           picman_gui_init              (Picman                *picman);

void           picman_gui_ungrab            (Picman                *picman);

void           picman_threads_enter         (Picman                *picman);
void           picman_threads_leave         (Picman                *picman);

PicmanObject   * picman_get_window_strategy   (Picman                *picman);
PicmanObject   * picman_get_empty_display     (Picman                *picman);
PicmanObject   * picman_get_display_by_ID     (Picman                *picman,
                                           gint                 ID);
gint           picman_get_display_ID        (Picman                *picman,
                                           PicmanObject          *display);
guint32        picman_get_display_window_id (Picman                *picman,
                                           PicmanObject          *display);
PicmanObject   * picman_create_display        (Picman                *picman,
                                           PicmanImage           *image,
                                           PicmanUnit             unit,
                                           gdouble              scale);
void           picman_delete_display        (Picman                *picman,
                                           PicmanObject          *display);
void           picman_reconnect_displays    (Picman                *picman,
                                           PicmanImage           *old_image,
                                           PicmanImage           *new_image);

void           picman_set_busy              (Picman                *picman);
void           picman_set_busy_until_idle   (Picman                *picman);
void           picman_unset_busy            (Picman                *picman);

void           picman_show_message          (Picman                *picman,
                                           GObject             *handler,
                                           PicmanMessageSeverity  severity,
                                           const gchar         *domain,
                                           const gchar         *message);
void           picman_help                  (Picman                *picman,
                                           PicmanProgress        *progress,
                                           const gchar         *help_domain,
                                           const gchar         *help_id);

PicmanProgress * picman_new_progress          (Picman                *picman,
                                           PicmanObject          *display);
void           picman_free_progress         (Picman                *picman,
                                           PicmanProgress        *progress);

const gchar  * picman_get_program_class     (Picman                *picman);
gchar        * picman_get_display_name      (Picman                *picman,
                                           gint                 display_ID,
                                           gint                *monitor_number);
guint32        picman_get_user_time         (Picman                *picman);
const gchar  * picman_get_theme_dir         (Picman                *picman);

gboolean       picman_pdb_dialog_new        (Picman                *picman,
                                           PicmanContext         *context,
                                           PicmanProgress        *progress,
                                           PicmanContainer       *container,
                                           const gchar         *title,
                                           const gchar         *callback_name,
                                           const gchar         *object_name,
                                           ...) G_GNUC_NULL_TERMINATED;
gboolean       picman_pdb_dialog_set        (Picman                *picman,
                                           PicmanContainer       *container,
                                           const gchar         *callback_name,
                                           const gchar         *object_name,
                                           ...) G_GNUC_NULL_TERMINATED;
gboolean       picman_pdb_dialog_close      (Picman                *picman,
                                           PicmanContainer       *container,
                                           const gchar         *callback_name);
gboolean       picman_recent_list_add_uri   (Picman                *picman,
                                           const gchar         *uri,
                                           const gchar         *mime_type);
void           picman_recent_list_load      (Picman                *picman);


#endif  /* __PICMAN_GUI_H__ */
