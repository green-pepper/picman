/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanfontselect.c
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
  gchar               *font_callback;
  guint                idle_id;
  gchar               *font_name;
  PicmanRunFontCallback  callback;
  gboolean             closing;
  gpointer             data;
} PicmanFontData;


/*  local function prototypes  */

static void      picman_font_data_free     (PicmanFontData     *data);

static void      picman_temp_font_run      (const gchar      *name,
                                          gint              nparams,
                                          const PicmanParam  *param,
                                          gint             *nreturn_vals,
                                          PicmanParam       **return_vals);
static gboolean  picman_temp_font_run_idle (PicmanFontData     *font_data);


/*  private variables  */

static GHashTable *picman_font_select_ht = NULL;


/*  public functions  */

const gchar *
picman_font_select_new (const gchar         *title,
                      const gchar         *font_name,
                      PicmanRunFontCallback  callback,
                      gpointer             data)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_STRING, "str",           "String"                     },
    { PICMAN_PDB_INT32,  "dialog status", "If the dialog was closing "
                                        "[0 = No, 1 = Yes]"          }
  };

  gchar *font_callback = picman_procedural_db_temp_name ();

  picman_install_temp_proc (font_callback,
                          "Temporary font popup callback procedure",
                          "",
                          "",
                          "",
                          "",
                          NULL,
                          "",
                          PICMAN_TEMPORARY,
                          G_N_ELEMENTS (args), 0,
                          args, NULL,
                          picman_temp_font_run);

  if (picman_fonts_popup (font_callback, title, font_name))
    {
      PicmanFontData *font_data;

      picman_extension_enable (); /* Allow callbacks to be watched */

      /* Now add to hash table so we can find it again */
      if (! picman_font_select_ht)
        {
          picman_font_select_ht =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   g_free,
                                   (GDestroyNotify) picman_font_data_free);
        }

      font_data = g_slice_new0 (PicmanFontData);

      font_data->font_callback = font_callback;
      font_data->callback      = callback;
      font_data->data          = data;

      g_hash_table_insert (picman_font_select_ht, font_callback, font_data);

      return font_callback;
    }

  picman_uninstall_temp_proc (font_callback);
  g_free (font_callback);

  return NULL;
}

void
picman_font_select_destroy (const gchar *font_callback)
{
  PicmanFontData *font_data;

  g_return_if_fail (font_callback != NULL);
  g_return_if_fail (picman_font_select_ht != NULL);

  font_data = g_hash_table_lookup (picman_font_select_ht, font_callback);

  if (! font_data)
    {
      g_warning ("Can't find internal font data");
      return;
    }

  if (font_data->idle_id)
    g_source_remove (font_data->idle_id);

  g_free (font_data->font_name);

  if (font_data->font_callback)
    picman_fonts_close_popup (font_data->font_callback);

  picman_uninstall_temp_proc (font_callback);

  g_hash_table_remove (picman_font_select_ht, font_callback);
}


/*  private functions  */

static void
picman_font_data_free (PicmanFontData *data)
{
  g_slice_free (PicmanFontData, data);
}

static void
picman_temp_font_run (const gchar      *name,
                    gint              nparams,
                    const PicmanParam  *param,
                    gint             *nreturn_vals,
                    PicmanParam       **return_vals)
{
  static PicmanParam  values[1];
  PicmanFontData     *font_data;

  font_data = g_hash_table_lookup (picman_font_select_ht, name);

  if (! font_data)
    {
      g_warning ("Can't find internal font data");
    }
  else
    {
      g_free (font_data->font_name);

      font_data->font_name = g_strdup (param[0].data.d_string);
      font_data->closing   = param[1].data.d_int32;

      if (! font_data->idle_id)
        font_data->idle_id = g_idle_add ((GSourceFunc) picman_temp_font_run_idle,
                                         font_data);
    }

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_SUCCESS;
}

static gboolean
picman_temp_font_run_idle (PicmanFontData *font_data)
{
  font_data->idle_id = 0;

  if (font_data->callback)
    font_data->callback (font_data->font_name,
                         font_data->closing,
                         font_data->data);

  if (font_data->closing)
    {
      gchar *font_callback = font_data->font_callback;

      font_data->font_callback = NULL;
      picman_font_select_destroy (font_callback);
    }

  return FALSE;
}
