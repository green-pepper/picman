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

#include "config.h"

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picmanprogress.h"

#include "widgets/picmanhelp.h"

#include "actions.h"
#include "help-commands.h"


void
help_help_cmd_callback (GtkAction *action,
                        gpointer   data)
{
  Picman        *picman;
  PicmanDisplay *display;
  return_if_no_picman (picman, data);
  return_if_no_display (display, data);

  picman_help_show (picman, PICMAN_PROGRESS (display), NULL, NULL);
}

void
help_context_help_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  GtkWidget *widget;
  return_if_no_widget (widget, data);

  picman_context_help (widget);
}
