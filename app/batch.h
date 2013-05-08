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

#ifndef __BATCH_H__
#define __BATCH_H__

#ifndef PICMAN_APP_GLUE_COMPILATION
#error You must not #include "batch.h" from an app/ subdir
#endif


void   batch_run (Picman         *picman,
                  const gchar  *batch_interpreter,
                  const gchar **batch_commands);


#endif /* __BATCH_H__ */
