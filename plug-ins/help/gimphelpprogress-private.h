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

#ifndef __PICMAN_HELP_PROGRESS_PRIVATE_H__
#define __PICMAN_HELP_PROGRESS_PRIVATE_H__


/*  internal API  */

void  _picman_help_progress_start  (PicmanHelpProgress   *progress,
                                  GCancellable       *cancellable,
                                  const gchar        *format,
                                  ...) G_GNUC_PRINTF (3, 4)       G_GNUC_INTERNAL;
void  _picman_help_progress_update (PicmanHelpProgress   *progress,
                                  gdouble             percentage) G_GNUC_INTERNAL;
void  _picman_help_progress_pulse  (PicmanHelpProgress   *progress)   G_GNUC_INTERNAL;
void  _picman_help_progress_finish (PicmanHelpProgress   *progress)   G_GNUC_INTERNAL;


#endif /* ! __PICMAN_HELP_PROGRESS_PRIVATE_H__ */
