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

#include <gegl.h>
#include <gtk/gtk.h>

#include "actions-types.h"

#include "core/picmancontext.h"

#include "text/picman-fonts.h"

#include "widgets/picmanfontview.h"

#include "actions.h"
#include "fonts-commands.h"

#include "picman-intl.h"


/*  public functionss */

void
fonts_refresh_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanContext *context = action_data_get_context (data);

  if (context)
    picman_fonts_load (context->picman);
}
