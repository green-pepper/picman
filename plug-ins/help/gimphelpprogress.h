/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * The PICMAN Help plug-in
 * Copyright (C) 1999-2008 Sven Neumann <sven@picman.org>
 *                         Michael Natterer <mitch@picman.org>
 *                         Henrik Brix Andersen <brix@picman.org>
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

#ifndef __PICMAN_HELP_PROGRESS_H__
#define __PICMAN_HELP_PROGRESS_H__


typedef struct
{
  void  (* start)           (const gchar *message,
                             gboolean     cancelable,
                             gpointer     user_data);
  void  (* end)             (gpointer     user_data);
  void  (* set_value)       (gdouble      percentage,
                             gpointer     user_data);

  /* Padding for future expansion. Must be initialized with NULL! */
  void  (* _picman_reserved1) (void);
  void  (* _picman_reserved2) (void);
  void  (* _picman_reserved3) (void);
  void  (* _picman_reserved4) (void);
} PicmanHelpProgressVTable;


PicmanHelpProgress * picman_help_progress_new    (const PicmanHelpProgressVTable *vtable,
                                              gpointer                      user_data);
void               picman_help_progress_free   (PicmanHelpProgress *progress);

void               picman_help_progress_cancel (PicmanHelpProgress *progress);


#endif /* ! __PICMAN_HELP_PROGRESS_H__ */
