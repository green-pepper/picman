/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpatternselect.c
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
  gchar                  *pattern_callback;
  guint                   idle_id;
  gchar                  *pattern_name;
  gint                    width;
  gint                    height;
  gint                    bytes;
  guchar                 *pattern_mask_data;
  PicmanRunPatternCallback  callback;
  gboolean                closing;
  gpointer                data;
} PicmanPatternData;


/*  local function prototypes  */

static void      picman_pattern_data_free     (PicmanPatternData  *data);

static void      picman_temp_pattern_run      (const gchar      *name,
                                             gint              nparams,
                                             const PicmanParam  *param,
                                             gint             *nreturn_vals,
                                             PicmanParam       **return_vals);
static gboolean  picman_temp_pattern_run_idle (PicmanPatternData  *pattern_data);


/*  private variables  */

static GHashTable *picman_pattern_select_ht = NULL;


/*  public functions  */

const gchar *
picman_pattern_select_new (const gchar            *title,
                         const gchar            *pattern_name,
                         PicmanRunPatternCallback  callback,
                         gpointer                data)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_STRING,   "str",           "String"                      },
    { PICMAN_PDB_INT32,    "mask width",    "Pattern width"               },
    { PICMAN_PDB_INT32,    "mask height",   "Pattern heigth"              },
    { PICMAN_PDB_INT32,    "mask bpp",      "Pattern bytes per pixel"     },
    { PICMAN_PDB_INT32,    "mask len",      "Length of pattern mask data" },
    { PICMAN_PDB_INT8ARRAY,"mask data",     "The pattern mask data"       },
    { PICMAN_PDB_INT32,    "dialog status", "If the dialog was closing "
                                          "[0 = No, 1 = Yes]"           }
  };

  gchar *pattern_callback = picman_procedural_db_temp_name ();

  picman_install_temp_proc (pattern_callback,
                          "Temporary pattern popup callback procedure",
                          "",
                          "",
                          "",
                          "",
                          NULL,
                          "",
                          PICMAN_TEMPORARY,
                          G_N_ELEMENTS (args), 0,
                          args, NULL,
                          picman_temp_pattern_run);

  if (picman_patterns_popup (pattern_callback, title, pattern_name))
    {
      PicmanPatternData *pattern_data;

      picman_extension_enable (); /* Allow callbacks to be watched */

      /* Now add to hash table so we can find it again */
      if (! picman_pattern_select_ht)
        {
          picman_pattern_select_ht =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   g_free,
                                   (GDestroyNotify) picman_pattern_data_free);
        }

      pattern_data = g_slice_new0 (PicmanPatternData);

      pattern_data->pattern_callback = pattern_callback;
      pattern_data->callback         = callback;
      pattern_data->data             = data;

      g_hash_table_insert (picman_pattern_select_ht,
                           pattern_callback, pattern_data);

      return pattern_callback;
    }

  picman_uninstall_temp_proc (pattern_callback);
  g_free (pattern_callback);

  return NULL;
}

void
picman_pattern_select_destroy (const gchar *pattern_callback)
{
  PicmanPatternData *pattern_data;

  g_return_if_fail (pattern_callback != NULL);
  g_return_if_fail (picman_pattern_select_ht != NULL);

  pattern_data = g_hash_table_lookup (picman_pattern_select_ht,
                                      pattern_callback);

  if (! pattern_data)
    {
      g_warning ("Can't find internal pattern data");
      return;
    }

  if (pattern_data->idle_id)
    g_source_remove (pattern_data->idle_id);

  g_free (pattern_data->pattern_name);
  g_free (pattern_data->pattern_mask_data);

  if (pattern_data->pattern_callback)
    picman_patterns_close_popup (pattern_data->pattern_callback);

  picman_uninstall_temp_proc (pattern_callback);

  g_hash_table_remove (picman_pattern_select_ht, pattern_callback);
}


/*  private functions  */

static void
picman_pattern_data_free (PicmanPatternData *data)
{
  g_slice_free (PicmanPatternData, data);
}

static void
picman_temp_pattern_run (const gchar      *name,
                       gint              nparams,
                       const PicmanParam  *param,
                       gint             *nreturn_vals,
                       PicmanParam       **return_vals)
{
  static PicmanParam  values[1];
  PicmanPatternData  *pattern_data;

  pattern_data = g_hash_table_lookup (picman_pattern_select_ht, name);

  if (! pattern_data)
    {
      g_warning ("Can't find internal pattern data");
    }
  else
    {
      g_free (pattern_data->pattern_name);
      g_free (pattern_data->pattern_mask_data);

      pattern_data->pattern_name      = g_strdup (param[0].data.d_string);
      pattern_data->width             = param[1].data.d_int32;
      pattern_data->height            = param[2].data.d_int32;
      pattern_data->bytes             = param[3].data.d_int32;
      pattern_data->pattern_mask_data = g_memdup (param[5].data.d_int8array,
                                                  param[4].data.d_int32);
      pattern_data->closing           = param[6].data.d_int32;

      if (! pattern_data->idle_id)
        pattern_data->idle_id = g_idle_add ((GSourceFunc) picman_temp_pattern_run_idle,
                                            pattern_data);
    }

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_SUCCESS;
}

static gboolean
picman_temp_pattern_run_idle (PicmanPatternData *pattern_data)
{
  pattern_data->idle_id = 0;

  if (pattern_data->callback)
    pattern_data->callback (pattern_data->pattern_name,
                            pattern_data->width,
                            pattern_data->height,
                            pattern_data->bytes,
                            pattern_data->pattern_mask_data,
                            pattern_data->closing,
                            pattern_data->data);

  if (pattern_data->closing)
    {
      gchar *pattern_callback = pattern_data->pattern_callback;

      pattern_data->pattern_callback = NULL;
      picman_pattern_select_destroy (pattern_callback);
    }

  return FALSE;
}
