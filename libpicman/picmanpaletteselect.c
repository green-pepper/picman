/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpaletteselect.c
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "picman.h"


typedef struct
{
  gchar                  *palette_callback;
  guint                   idle_id;
  gchar                  *palette_name;
  gint                    num_colors;
  PicmanRunPaletteCallback  callback;
  gboolean                closing;
  gpointer                data;
} PicmanPaletteData;


/*  local function prototypes  */

static void      picman_palette_data_free     (PicmanPaletteData  *data);

static void      picman_temp_palette_run      (const gchar      *name,
                                             gint              nparams,
                                             const PicmanParam  *param,
                                             gint             *nreturn_vals,
                                             PicmanParam       **return_vals);
static gboolean  picman_temp_palette_run_idle (PicmanPaletteData  *palette_data);


/*  private variables  */

static GHashTable *picman_palette_select_ht = NULL;


/*  public functions  */

const gchar *
picman_palette_select_new (const gchar            *title,
                         const gchar            *palette_name,
                         PicmanRunPaletteCallback  callback,
                         gpointer                data)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_STRING, "str",           "String"                      },
    { PICMAN_PDB_INT32,  "num colors",    "Number of colors"            },
    { PICMAN_PDB_INT32,  "dialog status", "If the dialog was closing "
                                        "[0 = No, 1 = Yes]"           }
  };

  gchar *palette_callback = picman_procedural_db_temp_name ();

  picman_install_temp_proc (palette_callback,
                          "Temporary palette popup callback procedure",
                          "",
                          "",
                          "",
                          "",
                          NULL,
                          "",
                          PICMAN_TEMPORARY,
                          G_N_ELEMENTS (args), 0,
                          args, NULL,
                          picman_temp_palette_run);

  if (picman_palettes_popup (palette_callback, title, palette_name))
    {
      PicmanPaletteData *palette_data;

      picman_extension_enable (); /* Allow callbacks to be watched */

      /* Now add to hash table so we can find it again */
      if (! picman_palette_select_ht)
        {
          picman_palette_select_ht =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   g_free,
                                   (GDestroyNotify) picman_palette_data_free);
        }

      palette_data = g_slice_new0 (PicmanPaletteData);

      palette_data->palette_callback = palette_callback;
      palette_data->callback      = callback;
      palette_data->data          = data;

      g_hash_table_insert (picman_palette_select_ht,
                           palette_callback, palette_data);

      return palette_callback;
    }

  picman_uninstall_temp_proc (palette_callback);
  g_free (palette_callback);

  return NULL;
}

void
picman_palette_select_destroy (const gchar *palette_callback)
{
  PicmanPaletteData *palette_data;

  g_return_if_fail (palette_callback != NULL);
  g_return_if_fail (picman_palette_select_ht != NULL);

  palette_data = g_hash_table_lookup (picman_palette_select_ht, palette_callback);

  if (! palette_data)
    {
      g_warning ("Can't find internal palette data");
      return;
    }

  if (palette_data->idle_id)
    g_source_remove (palette_data->idle_id);

  g_free (palette_data->palette_name);

  if (palette_data->palette_callback)
    picman_palettes_close_popup (palette_data->palette_callback);

  picman_uninstall_temp_proc (palette_callback);

  g_hash_table_remove (picman_palette_select_ht, palette_callback);
}


/*  private functions  */

static void
picman_palette_data_free (PicmanPaletteData *data)
{
  g_slice_free (PicmanPaletteData, data);
}

static void
picman_temp_palette_run (const gchar      *name,
                       gint              nparams,
                       const PicmanParam  *param,
                       gint             *nreturn_vals,
                       PicmanParam       **return_vals)
{
  static PicmanParam  values[1];
  PicmanPaletteData  *palette_data;

  palette_data = g_hash_table_lookup (picman_palette_select_ht, name);

  if (! palette_data)
    {
      g_warning ("Can't find internal palette data");
    }
  else
    {
      g_free (palette_data->palette_name);

      palette_data->palette_name = g_strdup (param[0].data.d_string);
      palette_data->num_colors   = param[1].data.d_int32;
      palette_data->closing      = param[2].data.d_int32;

      if (! palette_data->idle_id)
        palette_data->idle_id = g_idle_add ((GSourceFunc) picman_temp_palette_run_idle,
                                            palette_data);
    }

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_SUCCESS;
}

static gboolean
picman_temp_palette_run_idle (PicmanPaletteData *palette_data)
{
  palette_data->idle_id = 0;

  if (palette_data->callback)
    palette_data->callback (palette_data->palette_name,
                            palette_data->closing,
                            palette_data->data);

  if (palette_data->closing)
    {
      gchar *palette_callback = palette_data->palette_callback;

      palette_data->palette_callback = NULL;
      picman_palette_select_destroy (palette_callback);
    }

  return FALSE;
}
