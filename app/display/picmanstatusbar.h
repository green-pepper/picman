/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_STATUSBAR_H__
#define __PICMAN_STATUSBAR_H__

G_BEGIN_DECLS


/*  maximal length of the format string for the cursor-coordinates  */
#define CURSOR_FORMAT_LENGTH 32


#define PICMAN_TYPE_STATUSBAR            (picman_statusbar_get_type ())
#define PICMAN_STATUSBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_STATUSBAR, PicmanStatusbar))
#define PICMAN_STATUSBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_STATUSBAR, PicmanStatusbarClass))
#define PICMAN_IS_STATUSBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_STATUSBAR))
#define PICMAN_IS_STATUSBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_STATUSBAR))
#define PICMAN_STATUSBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_STATUSBAR, PicmanStatusbarClass))

typedef struct _PicmanStatusbarClass PicmanStatusbarClass;

struct _PicmanStatusbar
{
  GtkStatusbar         parent_instance;

  PicmanDisplayShell    *shell;

  GSList              *messages;
  GHashTable          *context_ids;
  guint                seq_context_id;

  GdkPixbuf           *icon;

  guint                temp_context_id;
  guint                temp_timeout_id;
  PicmanMessageSeverity  temp_severity;

  gchar                cursor_format_str[CURSOR_FORMAT_LENGTH];
  gchar                cursor_format_str_f[CURSOR_FORMAT_LENGTH];
  gchar                length_format_str[CURSOR_FORMAT_LENGTH];

  GtkWidget           *cursor_label;
  GtkWidget           *unit_combo;
  GtkWidget           *scale_combo;
  GtkWidget           *label; /* same as GtkStatusbar->label */

  GtkWidget           *progressbar;
  GtkWidget           *cancel_button;
  gboolean             progress_active;
  gboolean             progress_shown;
  gdouble              progress_value;
};

struct _PicmanStatusbarClass
{
  GtkStatusbarClass parent_class;
};


GType       picman_statusbar_get_type              (void) G_GNUC_CONST;
GtkWidget * picman_statusbar_new                   (void);

void        picman_statusbar_set_shell             (PicmanStatusbar       *statusbar,
                                                  PicmanDisplayShell    *shell);

gboolean    picman_statusbar_get_visible           (PicmanStatusbar       *statusbar);
void        picman_statusbar_set_visible           (PicmanStatusbar       *statusbar,
                                                  gboolean             visible);
void        picman_statusbar_empty                 (PicmanStatusbar       *statusbar);
void        picman_statusbar_fill                  (PicmanStatusbar       *statusbar);

void        picman_statusbar_override_window_title (PicmanStatusbar       *statusbar);
void        picman_statusbar_restore_window_title  (PicmanStatusbar       *statusbar);

void        picman_statusbar_push                  (PicmanStatusbar       *statusbar,
                                                  const gchar         *context,
                                                  const gchar         *stock_id,
                                                  const gchar         *format,
                                                  ...) G_GNUC_PRINTF(4,5);
void        picman_statusbar_push_valist           (PicmanStatusbar       *statusbar,
                                                  const gchar         *context,
                                                  const gchar         *stock_id,
                                                  const gchar         *format,
                                                  va_list              args);
void        picman_statusbar_push_coords           (PicmanStatusbar       *statusbar,
                                                  const gchar         *context,
                                                  const gchar         *stock_id,
                                                  PicmanCursorPrecision  precision,
                                                  const gchar         *title,
                                                  gdouble              x,
                                                  const gchar         *separator,
                                                  gdouble              y,
                                                  const gchar         *help);
void        picman_statusbar_push_length           (PicmanStatusbar       *statusbar,
                                                  const gchar         *context,
                                                  const gchar         *stock_id,
                                                  const gchar         *title,
                                                  PicmanOrientationType  axis,
                                                  gdouble              value,
                                                  const gchar         *help);
void        picman_statusbar_replace               (PicmanStatusbar       *statusbar,
                                                  const gchar         *context,
                                                  const gchar         *stock_id,
                                                  const gchar         *format,
                                                  ...) G_GNUC_PRINTF(4,5);
void        picman_statusbar_replace_valist        (PicmanStatusbar       *statusbar,
                                                  const gchar         *context,
                                                  const gchar         *stock_id,
                                                  const gchar         *format,
                                                  va_list              args);
const gchar * picman_statusbar_peek                (PicmanStatusbar       *statusbar,
                                                  const gchar         *context);
void        picman_statusbar_pop                   (PicmanStatusbar       *statusbar,
                                                  const gchar         *context);

void        picman_statusbar_push_temp             (PicmanStatusbar       *statusbar,
                                                  PicmanMessageSeverity  severity,
                                                  const gchar         *stock_id,
                                                  const gchar         *format,
                                                  ...) G_GNUC_PRINTF(4,5);
void        picman_statusbar_push_temp_valist      (PicmanStatusbar       *statusbar,
                                                  PicmanMessageSeverity  severity,
                                                  const gchar         *stock_id,
                                                  const gchar         *format,
                                                  va_list              args);
void        picman_statusbar_pop_temp              (PicmanStatusbar       *statusbar);

void        picman_statusbar_update_cursor         (PicmanStatusbar       *statusbar,
                                                  PicmanCursorPrecision  precision,
                                                  gdouble              x,
                                                  gdouble              y);
void        picman_statusbar_clear_cursor          (PicmanStatusbar       *statusbar);


G_END_DECLS

#endif /* __PICMAN_STATUSBAR_H__ */
