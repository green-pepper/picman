/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanprogress.h
 * Copyright (C) 2004  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_PROGRESS_H__
#define __PICMAN_PROGRESS_H__


#define PICMAN_TYPE_PROGRESS               (picman_progress_interface_get_type ())
#define PICMAN_IS_PROGRESS(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PROGRESS))
#define PICMAN_PROGRESS(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PROGRESS, PicmanProgress))
#define PICMAN_PROGRESS_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_PROGRESS, PicmanProgressInterface))


typedef struct _PicmanProgressInterface PicmanProgressInterface;

struct _PicmanProgressInterface
{
  GTypeInterface base_iface;

  /*  virtual functions  */
  PicmanProgress * (* start)         (PicmanProgress        *progress,
                                    const gchar         *message,
                                    gboolean             cancelable);
  void           (* end)           (PicmanProgress        *progress);
  gboolean       (* is_active)     (PicmanProgress        *progress);

  void           (* set_text)      (PicmanProgress        *progress,
                                    const gchar         *message);
  void           (* set_value)     (PicmanProgress        *progress,
                                    gdouble              percentage);
  gdouble        (* get_value)     (PicmanProgress        *progress);
  void           (* pulse)         (PicmanProgress        *progress);

  guint32        (* get_window_id) (PicmanProgress        *progress);

  gboolean       (* message)       (PicmanProgress        *progress,
                                    Picman                *picman,
                                    PicmanMessageSeverity  severity,
                                    const gchar         *domain,
                                    const gchar         *message);

  /*  signals  */
  void           (* cancel)        (PicmanProgress        *progress);
};


GType          picman_progress_interface_get_type (void) G_GNUC_CONST;

PicmanProgress * picman_progress_start              (PicmanProgress        *progress,
                                                 const gchar         *message,
                                                 gboolean             cancelable);
void           picman_progress_end                (PicmanProgress        *progress);
gboolean       picman_progress_is_active          (PicmanProgress        *progress);

void           picman_progress_set_text           (PicmanProgress        *progress,
                                                 const gchar         *message);
void           picman_progress_set_value          (PicmanProgress        *progress,
                                                 gdouble              percentage);
gdouble        picman_progress_get_value          (PicmanProgress        *progress);
void           picman_progress_pulse              (PicmanProgress        *progress);

guint32        picman_progress_get_window_id      (PicmanProgress        *progress);

gboolean       picman_progress_message            (PicmanProgress        *progress,
                                                 Picman                *picman,
                                                 PicmanMessageSeverity  severity,
                                                 const gchar         *domain,
                                                 const gchar         *message);

void           picman_progress_cancel             (PicmanProgress        *progress);

void           picman_progress_update_and_flush   (gint                 min,
                                                 gint                 max,
                                                 gint                 current,
                                                 gpointer             data);


#endif /* __PICMAN_PROGRESS_H__ */
