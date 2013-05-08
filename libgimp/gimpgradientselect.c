/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmangradientselect.c
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
  gchar                   *gradient_callback;
  guint                    idle_id;
  gchar                   *gradient_name;
  gint                     width;
  gdouble                 *gradient_data;
  PicmanRunGradientCallback  callback;
  gboolean                 closing;
  gpointer                 data;
} PicmanGradientData;


/*  local function prototypes  */

static void      picman_gradient_data_free     (PicmanGradientData  *data);

static void      picman_temp_gradient_run      (const gchar       *name,
                                              gint               nparams,
                                              const PicmanParam   *param,
                                              gint              *nreturn_vals,
                                              PicmanParam        **return_vals);
static gboolean  picman_temp_gradient_run_idle (PicmanGradientData  *gradient_data);


/*  private variables  */

static GHashTable *picman_gradient_select_ht = NULL;


/*  public functions  */

const gchar *
picman_gradient_select_new (const gchar             *title,
                          const gchar             *gradient_name,
                          gint                     sample_size,
                          PicmanRunGradientCallback  callback,
                          gpointer                 data)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_STRING,    "str",            "String"                     },
    { PICMAN_PDB_INT32,     "gradient width", "Gradient width"             },
    { PICMAN_PDB_FLOATARRAY,"gradient data",  "The gradient mask data"     },
    { PICMAN_PDB_INT32,     "dialog status",  "If the dialog was closing "
                                            "[0 = No, 1 = Yes]"          }
  };

  gchar *gradient_callback = picman_procedural_db_temp_name ();

  picman_install_temp_proc (gradient_callback,
                          "Temporary gradient popup callback procedure",
                          "",
                          "",
                          "",
                          "",
                          NULL,
                          "",
                          PICMAN_TEMPORARY,
                          G_N_ELEMENTS (args), 0,
                          args, NULL,
                          picman_temp_gradient_run);

  if (picman_gradients_popup (gradient_callback, title, gradient_name,
                            sample_size))
    {
      PicmanGradientData *gradient_data;

      picman_extension_enable (); /* Allow callbacks to be watched */

      /* Now add to hash table so we can find it again */
      if (! picman_gradient_select_ht)
        {
          picman_gradient_select_ht =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   g_free,
                                   (GDestroyNotify) picman_gradient_data_free);
        }

      gradient_data = g_slice_new0 (PicmanGradientData);

      gradient_data->gradient_callback = gradient_callback;
      gradient_data->callback          = callback;
      gradient_data->data              = data;

      g_hash_table_insert (picman_gradient_select_ht,
                           gradient_callback, gradient_data);

      return gradient_callback;
    }

  picman_uninstall_temp_proc (gradient_callback);
  g_free (gradient_callback);

  return NULL;
}

void
picman_gradient_select_destroy (const gchar *gradient_callback)
{
  PicmanGradientData *gradient_data;

  g_return_if_fail (gradient_callback != NULL);
  g_return_if_fail (picman_gradient_select_ht != NULL);

  gradient_data = g_hash_table_lookup (picman_gradient_select_ht,
                                       gradient_callback);

  if (! gradient_data)
    {
      g_warning ("Can't find internal gradient data");
      return;
    }

  if (gradient_data->idle_id)
    g_source_remove (gradient_data->idle_id);

  g_free (gradient_data->gradient_name);
  g_free (gradient_data->gradient_data);

  if (gradient_data->gradient_callback)
    picman_gradients_close_popup (gradient_data->gradient_callback);

  picman_uninstall_temp_proc (gradient_callback);

  g_hash_table_remove (picman_gradient_select_ht, gradient_callback);
}


/*  private functions  */

static void
picman_gradient_data_free (PicmanGradientData *data)
{
  g_slice_free (PicmanGradientData, data);
}

static void
picman_temp_gradient_run (const gchar      *name,
                        gint              nparams,
                        const PicmanParam  *param,
                        gint             *nreturn_vals,
                        PicmanParam       **return_vals)
{
  static PicmanParam  values[1];
  PicmanGradientData *gradient_data;

  gradient_data = g_hash_table_lookup (picman_gradient_select_ht, name);

  if (! gradient_data)
    {
      g_warning ("Can't find internal gradient data");
    }
  else
    {
      g_free (gradient_data->gradient_name);
      g_free (gradient_data->gradient_data);

      gradient_data->gradient_name = g_strdup (param[0].data.d_string);
      gradient_data->width         = param[1].data.d_int32;
      gradient_data->gradient_data = g_memdup (param[2].data.d_floatarray,
                                               param[1].data.d_int32 *
                                               sizeof (gdouble));
      gradient_data->closing       = param[3].data.d_int32;

      if (! gradient_data->idle_id)
        gradient_data->idle_id =
          g_idle_add ((GSourceFunc) picman_temp_gradient_run_idle,
                      gradient_data);
    }

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_SUCCESS;
}

static gboolean
picman_temp_gradient_run_idle (PicmanGradientData *gradient_data)
{
  gradient_data->idle_id = 0;

  if (gradient_data->callback)
    gradient_data->callback (gradient_data->gradient_name,
                             gradient_data->width,
                             gradient_data->gradient_data,
                             gradient_data->closing,
                             gradient_data->data);

  if (gradient_data->closing)
    {
      gchar *gradient_callback = gradient_data->gradient_callback;

      gradient_data->gradient_callback = NULL;
      picman_gradient_select_destroy (gradient_callback);
    }

  return FALSE;
}
