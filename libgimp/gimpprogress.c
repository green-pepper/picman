/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanprogress.c
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

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#undef PICMAN_DISABLE_DEPRECATED
#include "picmanprogress.h"
#define PICMAN_DISABLE_DEPRECATED

#include "picman.h"


typedef struct
{
  gchar              *progress_callback;
  PicmanProgressVtable  vtable;
  gpointer            data;
} PicmanProgressData;


/*  local function prototypes  */

static void   picman_progress_data_free (PicmanProgressData *data);

static void   picman_temp_progress_run  (const gchar      *name,
                                       gint              nparams,
                                       const PicmanParam  *param,
                                       gint             *nreturn_vals,
                                       PicmanParam       **return_vals);


/*  private variables  */

static GHashTable    * picman_progress_ht      = NULL;
static gdouble         picman_progress_current = 0.0;
static const gdouble   picman_progress_step    = (1.0 / 256.0);


/*  public functions  */

/**
 * picman_progress_install:
 * @start_callback: the function to call when progress starts
 * @end_callback:   the function to call when progress finishes
 * @text_callback:  the function to call to change the text
 * @value_callback: the function to call to change the value
 * @user_data:      a pointer that is returned when uninstalling the progress
 *
 * Note that since PICMAN 2.4, @value_callback can be called with
 * negative values. This is triggered by calls to picman_progress_pulse().
 * The callback should then implement a progress indicating business,
 * e.g. by calling gtk_progress_bar_pulse().
 *
 * Return value: the name of the temporary procedure that's been installed
 *
 * Since: PICMAN 2.2
 **/
const gchar *
picman_progress_install (PicmanProgressStartCallback start_callback,
                       PicmanProgressEndCallback   end_callback,
                       PicmanProgressTextCallback  text_callback,
                       PicmanProgressValueCallback value_callback,
                       gpointer                  user_data)
{
  PicmanProgressVtable vtable = { 0, };

  g_return_val_if_fail (start_callback != NULL, NULL);
  g_return_val_if_fail (end_callback != NULL, NULL);
  g_return_val_if_fail (text_callback != NULL, NULL);
  g_return_val_if_fail (value_callback != NULL, NULL);

  vtable.start     = start_callback;
  vtable.end       = end_callback;
  vtable.set_text  = text_callback;
  vtable.set_value = value_callback;

  return picman_progress_install_vtable (&vtable, user_data);
}

/**
 * picman_progress_install_vtable:
 * @vtable:    a pointer to a @PicmanProgressVtable.
 * @user_data: a pointer that is passed as user_data to all vtable functions.
 *
 * Return value: the name of the temporary procedure that's been installed
 *
 * Since: PICMAN 2.4
 **/
const gchar *
picman_progress_install_vtable (const PicmanProgressVtable *vtable,
                              gpointer                  user_data)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,  "command", "" },
    { PICMAN_PDB_STRING, "text",    "" },
    { PICMAN_PDB_FLOAT,  "value",   "" }
  };

  static const PicmanParamDef values[] =
  {
    { PICMAN_PDB_FLOAT,  "value",   "" }
  };

  gchar *progress_callback;

  g_return_val_if_fail (vtable != NULL, NULL);
  g_return_val_if_fail (vtable->start != NULL, NULL);
  g_return_val_if_fail (vtable->end != NULL, NULL);
  g_return_val_if_fail (vtable->set_text != NULL, NULL);
  g_return_val_if_fail (vtable->set_value != NULL, NULL);

  progress_callback = picman_procedural_db_temp_name ();

  picman_install_temp_proc (progress_callback,
                          "Temporary progress callback procedure",
                          "",
                          "",
                          "",
                          "",
                          NULL,
                          "",
                          PICMAN_TEMPORARY,
                          G_N_ELEMENTS (args), G_N_ELEMENTS (values),
                          args, values,
                          picman_temp_progress_run);

  if (_picman_progress_install (progress_callback))
    {
      PicmanProgressData *progress_data;

      picman_extension_enable (); /* Allow callbacks to be watched */

      /* Now add to hash table so we can find it again */
      if (! picman_progress_ht)
        {
          picman_progress_ht =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   g_free,
                                   (GDestroyNotify) picman_progress_data_free);
        }

      progress_data = g_slice_new0 (PicmanProgressData);

      progress_data->progress_callback = progress_callback;
      progress_data->vtable.start      = vtable->start;
      progress_data->vtable.end        = vtable->end;
      progress_data->vtable.set_text   = vtable->set_text;
      progress_data->vtable.set_value  = vtable->set_value;
      progress_data->vtable.pulse      = vtable->pulse;
      progress_data->vtable.get_window = vtable->get_window;
      progress_data->data              = user_data;

      g_hash_table_insert (picman_progress_ht, progress_callback, progress_data);

      return progress_callback;
    }

  picman_uninstall_temp_proc (progress_callback);
  g_free (progress_callback);

  return NULL;
}

/**
 * picman_progress_uninstall:
 * @progress_callback: the name of the temporary procedure to uninstall
 *
 * Uninstalls a temporary progress procedure that was installed using
 * picman_progress_install().
 *
 * Return value: the @user_data that was passed to picman_progress_install().
 *
 * Since: PICMAN 2.2
 **/
gpointer
picman_progress_uninstall (const gchar *progress_callback)
{
  PicmanProgressData *progress_data;
  gpointer          user_data;

  g_return_val_if_fail (progress_callback != NULL, NULL);
  g_return_val_if_fail (picman_progress_ht != NULL, NULL);

  progress_data = g_hash_table_lookup (picman_progress_ht, progress_callback);

  if (! progress_data)
    {
      g_warning ("Can't find internal progress data");
      return NULL;
    }

  _picman_progress_uninstall (progress_callback);
  picman_uninstall_temp_proc (progress_callback);

  user_data = progress_data->data;

  g_hash_table_remove (picman_progress_ht, progress_callback);

  return user_data;
}


/**
 * picman_progress_init:
 * @message: Message to use in the progress dialog.
 *
 * Initializes the progress bar for the current plug-in.
 *
 * Initializes the progress bar for the current plug-in. It is only
 * valid to call this procedure from a plug-in.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_progress_init (const gchar  *message)
{
  picman_progress_current = 0.0;

  return _picman_progress_init (message, picman_default_display ());
}

/**
 * picman_progress_init_printf:
 * @format: a standard printf() format string
 * @...: arguments for @format
 *
 * Initializes the progress bar for the current plug-in.
 *
 * Initializes the progress bar for the current plug-in. It is only
 * valid to call this procedure from a plug-in.
 *
 * Returns: %TRUE on success.
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_progress_init_printf (const gchar *format,
                           ...)
{
  gchar    *text;
  gboolean  retval;
  va_list   args;

  g_return_val_if_fail (format != NULL, FALSE);

  va_start (args, format);
  text = g_strdup_vprintf (format, args);
  va_end (args);

  retval = picman_progress_init (text);

  g_free (text);

  return retval;
}

/**
 * picman_progress_set_text_printf:
 * @format: a standard printf() format string
 * @...: arguments for @format
 *
 * Changes the text in the progress bar for the current plug-in.
 *
 * This function changes the text in the progress bar for the current
 * plug-in. Unlike picman_progress_init() it does not change the
 * displayed value.
 *
 * Returns: %TRUE on success.
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_progress_set_text_printf (const gchar *format,
                               ...)
{
  gchar    *text;
  gboolean  retval;
  va_list   args;

  g_return_val_if_fail (format != NULL, FALSE);

  va_start (args, format);
  text = g_strdup_vprintf (format, args);
  va_end (args);

  retval = picman_progress_set_text (text);

  g_free (text);

  return retval;
}

/**
 * picman_progress_update:
 * @percentage: Percentage of progress completed (in the range from 0.0 to 1.0).
 *
 * Updates the progress bar for the current plug-in.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_progress_update (gdouble percentage)
{
  gboolean changed;

  if (percentage <= 0.0)
    {
      changed = (picman_progress_current != 0.0);
      percentage = 0.0;
    }
  else if (percentage >= 1.0)
    {
      changed = (picman_progress_current != 1.0);
      percentage = 1.0;
    }
  else
    {
      changed =
        (fabs (picman_progress_current - percentage) > picman_progress_step);

#ifdef PICMAN_UNSTABLE
      if (! changed)
        {
          static gboolean warned = FALSE;
          static gint     count  = 0;

          count++;

          if (count > 3 && ! warned)
            {
              g_printerr ("%s is updating the progress too often\n",
                          g_get_prgname ());
              warned = TRUE;
            }
        }
#endif
    }

  /*  Suppress the update if the change was only marginal.  */
  if (! changed)
    return TRUE;

  picman_progress_current = percentage;

  return _picman_progress_update (picman_progress_current);
}


/*  private functions  */

static void
picman_progress_data_free (PicmanProgressData *data)
{
  g_slice_free (PicmanProgressData, data);
}

static void
picman_temp_progress_run (const gchar      *name,
                        gint              nparams,
                        const PicmanParam  *param,
                        gint             *nreturn_vals,
                        PicmanParam       **return_vals)
{
  static PicmanParam  values[2];
  PicmanProgressData *progress_data;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_SUCCESS;

  progress_data = g_hash_table_lookup (picman_progress_ht, name);

  if (! progress_data)
    {
      g_warning ("Can't find internal progress data");

      values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;
    }
  else
    {
      PicmanProgressCommand command = param[0].data.d_int32;

      switch (command)
        {
        case PICMAN_PROGRESS_COMMAND_START:
          progress_data->vtable.start (param[1].data.d_string,
                                       param[2].data.d_float != 0.0,
                                       progress_data->data);
          break;

        case PICMAN_PROGRESS_COMMAND_END:
          progress_data->vtable.end (progress_data->data);
          break;

        case PICMAN_PROGRESS_COMMAND_SET_TEXT:
          progress_data->vtable.set_text (param[1].data.d_string,
                                          progress_data->data);
          break;

        case PICMAN_PROGRESS_COMMAND_SET_VALUE:
          progress_data->vtable.set_value (param[2].data.d_float,
                                           progress_data->data);
          break;

        case PICMAN_PROGRESS_COMMAND_PULSE:
          if (progress_data->vtable.pulse)
            progress_data->vtable.pulse (progress_data->data);
          else
            progress_data->vtable.set_value (-1, progress_data->data);
          break;

        case PICMAN_PROGRESS_COMMAND_GET_WINDOW:
          *nreturn_vals  = 2;
          values[1].type = PICMAN_PDB_FLOAT;

          if (progress_data->vtable.get_window)
            values[1].data.d_float =
              (gdouble) progress_data->vtable.get_window (progress_data->data);
          else
            values[1].data.d_float = 0;
          break;

        default:
          values[0].data.d_status = PICMAN_PDB_CALLING_ERROR;
          break;
        }
    }
}
