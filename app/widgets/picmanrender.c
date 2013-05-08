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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"

#include "widgets-types.h"

#include "core/picman.h"

#include "picmanrender.h"


static void   picman_render_setup_notify (gpointer    config,
                                        GParamSpec *param_spec,
                                        Picman       *picman);


static PicmanRGB light;
static PicmanRGB dark;


void
picman_render_init (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  g_signal_connect (picman->config, "notify::transparency-type",
                    G_CALLBACK (picman_render_setup_notify),
                    picman);

  picman_render_setup_notify (picman->config, NULL, picman);
}

void
picman_render_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  g_signal_handlers_disconnect_by_func (picman->config,
                                        picman_render_setup_notify,
                                        picman);
}

const PicmanRGB *
picman_render_light_check_color (void)
{
  return &light;
}

const PicmanRGB *
picman_render_dark_check_color (void)
{
  return &dark;
}

static void
picman_render_setup_notify (gpointer    config,
                          GParamSpec *param_spec,
                          Picman       *picman)
{
  PicmanCheckType check_type;
  guchar        dark_check;
  guchar        light_check;

  g_object_get (config,
                "transparency-type", &check_type,
                NULL);

  picman_checks_get_shades (check_type, &light_check, &dark_check);

  picman_rgba_set_uchar (&light, light_check, light_check, light_check, 255);
  picman_rgba_set_uchar (&dark,  dark_check,  dark_check,  dark_check,  255);
}
