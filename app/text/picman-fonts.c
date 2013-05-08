/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis and others
 *
 * text.c
 * Copyright (C) 2003 Manish Singh <yosh@picman.org>
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

#include <glib-object.h>

#include <fontconfig/fontconfig.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "text-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"

#include "picman-fonts.h"
#include "picmanfontlist.h"


#define CONF_FNAME "fonts.conf"


static gboolean picman_fonts_load_fonts_conf (FcConfig    *config,
                                            gchar       *fonts_conf);
static void     picman_fonts_add_directories (FcConfig    *config,
                                            const gchar *path_str);


void
picman_fonts_init (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman->fonts = picman_font_list_new (72.0, 72.0);
  picman_object_set_name (PICMAN_OBJECT (picman->fonts), "fonts");

  g_signal_connect_swapped (picman->config, "notify::font-path",
                            G_CALLBACK (picman_fonts_load), picman);
}

void
picman_fonts_load (Picman *picman)
{
  FcConfig *config;
  gchar    *fonts_conf;
  gchar    *path;

  g_return_if_fail (PICMAN_IS_FONT_LIST (picman->fonts));

  picman_set_busy (picman);

  if (picman->be_verbose)
    g_print ("Loading fonts\n");

  picman_container_freeze (PICMAN_CONTAINER (picman->fonts));

  picman_container_clear (PICMAN_CONTAINER (picman->fonts));

  config = FcInitLoadConfig ();

  if (! config)
    goto cleanup;

  fonts_conf = picman_personal_rc_file (CONF_FNAME);
  if (! picman_fonts_load_fonts_conf (config, fonts_conf))
    goto cleanup;

  fonts_conf = g_build_filename (picman_sysconf_directory (), CONF_FNAME, NULL);
  if (! picman_fonts_load_fonts_conf (config, fonts_conf))
    goto cleanup;

  path = picman_config_path_expand (picman->config->font_path, TRUE, NULL);
  picman_fonts_add_directories (config, path);
  g_free (path);

  if (! FcConfigBuildFonts (config))
    {
      FcConfigDestroy (config);
      goto cleanup;
    }

  FcConfigSetCurrent (config);

  picman_font_list_restore (PICMAN_FONT_LIST (picman->fonts));

 cleanup:
  picman_container_thaw (PICMAN_CONTAINER (picman->fonts));
  picman_unset_busy (picman);
}

void
picman_fonts_reset (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman->no_fonts)
    return;

  /* Reinit the library with defaults. */
  FcInitReinitialize ();
}

static gboolean
picman_fonts_load_fonts_conf (FcConfig *config,
                            gchar    *fonts_conf)
{
  gboolean ret = TRUE;

  if (! FcConfigParseAndLoad (config, (const guchar *) fonts_conf, FcFalse))
    {
      FcConfigDestroy (config);
      ret = FALSE;
    }

  g_free (fonts_conf);

  return ret;
}

static void
picman_fonts_add_directories (FcConfig    *config,
                            const gchar *path_str)
{
  GList *path;
  GList *list;

  g_return_if_fail (config != NULL);
  g_return_if_fail (path_str != NULL);

  path = picman_path_parse (path_str, 256, TRUE, NULL);

  for (list = path; list; list = list->next)
    FcConfigAppFontAddDir (config, (const guchar *) list->data);

  picman_path_free (path);
}
