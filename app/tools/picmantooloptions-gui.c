/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picmantooloptions.h"

#include "picmantooloptions-gui.h"


/*  public functions  */

GtkWidget *
picman_tool_options_gui (PicmanToolOptions *tool_options)
{
  GtkWidget *vbox;

  g_return_val_if_fail (PICMAN_IS_TOOL_OPTIONS (tool_options), NULL);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);

  return vbox;
}
