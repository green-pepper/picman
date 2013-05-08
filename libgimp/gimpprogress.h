/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanprogress.h
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picman.h> can be included directly."
#endif

#ifndef __PICMAN_PROGRESS_H__
#define __PICMAN_PROGRESS_H__

G_BEGIN_DECLS


typedef struct _PicmanProgressVtable PicmanProgressVtable;

struct _PicmanProgressVtable
{
  void    (* start)        (const gchar *message,
                            gboolean     cancelable,
                            gpointer     user_data);
  void    (* end)          (gpointer     user_data);
  void    (* set_text)     (const gchar *message,
                            gpointer     user_data);
  void    (* set_value)    (gdouble      percentage,
                            gpointer     user_data);
  void    (* pulse)        (gpointer     user_data);

  guint32 (* get_window)   (gpointer     user_data);

  /* Padding for future expansion. Must be initialized with NULL! */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
  void (* _picman_reserved5) (void);
  void (* _picman_reserved6) (void);
  void (* _picman_reserved7) (void);
  void (* _picman_reserved8) (void);
};


const gchar * picman_progress_install_vtable  (const PicmanProgressVtable *vtable,
                                             gpointer                  user_data);
gpointer      picman_progress_uninstall       (const gchar              *progress_callback);

gboolean      picman_progress_init            (const gchar              *message);
gboolean      picman_progress_init_printf     (const gchar              *format,
                                             ...) G_GNUC_PRINTF (1, 2);

gboolean      picman_progress_set_text_printf (const gchar              *format,
                                             ...) G_GNUC_PRINTF (1, 2);

gboolean      picman_progress_update          (gdouble                   percentage);


#ifndef PICMAN_DISABLE_DEPRECATED
typedef void (* PicmanProgressStartCallback) (const gchar *message,
                                            gboolean     cancelable,
                                            gpointer     user_data);
typedef void (* PicmanProgressEndCallback)   (gpointer     user_data);
typedef void (* PicmanProgressTextCallback)  (const gchar *message,
                                            gpointer     user_data);
typedef void (* PicmanProgressValueCallback) (gdouble      percentage,
                                            gpointer     user_data);

PICMAN_DEPRECATED_FOR(picman_progress_install_vtable)
const gchar * picman_progress_install       (PicmanProgressStartCallback  start_callback,
                                           PicmanProgressEndCallback    end_callback,
                                           PicmanProgressTextCallback   text_callback,
                                           PicmanProgressValueCallback  value_callback,
                                           gpointer                   user_data);
#endif /* PICMAN_DISABLE_DEPRECATED */


G_END_DECLS

#endif /* __PICMAN_PROGRESS_H__ */
