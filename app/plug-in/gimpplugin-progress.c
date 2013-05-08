/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanplugin-progress.c
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

#include "plug-in-types.h"

#include "core/picman.h"
#include "core/picmanparamspecs.h"
#include "core/picmanpdbprogress.h"
#include "core/picmanprogress.h"

#include "pdb/picmanpdb.h"
#include "pdb/picmanpdberror.h"

#include "picmanplugin.h"
#include "picmanplugin-progress.h"
#include "picmanpluginmanager.h"
#include "picmantemporaryprocedure.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   picman_plug_in_progress_cancel_callback (PicmanProgress *progress,
                                                     PicmanPlugIn   *plug_in);


/*  public functions  */

gint
picman_plug_in_progress_attach (PicmanProgress *progress)
{
  gint attach_count;

  g_return_val_if_fail (PICMAN_IS_PROGRESS (progress), 0);

  attach_count =
    GPOINTER_TO_INT (g_object_get_data (G_OBJECT (progress),
                                        "plug-in-progress-attach-count"));

  attach_count++;

  g_object_set_data (G_OBJECT (progress), "plug-in-progress-attach-count",
                     GINT_TO_POINTER (attach_count));

  return attach_count;
}

gint
picman_plug_in_progress_detach (PicmanProgress *progress)
{
  gint attach_count;

  g_return_val_if_fail (PICMAN_IS_PROGRESS (progress), 0);

  attach_count =
    GPOINTER_TO_INT (g_object_get_data (G_OBJECT (progress),
                                        "plug-in-progress-attach-count"));

  attach_count--;

  g_object_set_data (G_OBJECT (progress), "plug-in-progress-attach-count",
                     GINT_TO_POINTER (attach_count));

  return attach_count;
}

void
picman_plug_in_progress_start (PicmanPlugIn  *plug_in,
                             const gchar *message,
                             PicmanObject  *display)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));
  g_return_if_fail (display == NULL || PICMAN_IS_OBJECT (display));

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (! proc_frame->progress)
    {
      proc_frame->progress = picman_new_progress (plug_in->manager->picman,
                                                display);

      if (proc_frame->progress)
        {
          proc_frame->progress_created = TRUE;

          g_object_ref (proc_frame->progress);

          picman_plug_in_progress_attach (proc_frame->progress);
        }
    }

  if (proc_frame->progress)
    {
      if (! proc_frame->progress_cancel_id)
        proc_frame->progress_cancel_id =
          g_signal_connect (proc_frame->progress, "cancel",
                            G_CALLBACK (picman_plug_in_progress_cancel_callback),
                            plug_in);

      if (picman_progress_is_active (proc_frame->progress))
        {
          if (message)
            picman_progress_set_text (proc_frame->progress, message);

          if (picman_progress_get_value (proc_frame->progress) > 0.0)
            picman_progress_set_value (proc_frame->progress, 0.0);
        }
      else
        {
          picman_progress_start (proc_frame->progress,
                               message ? message : "",
                               TRUE);
        }
    }
}

void
picman_plug_in_progress_end (PicmanPlugIn          *plug_in,
                           PicmanPlugInProcFrame *proc_frame)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));
  g_return_if_fail (proc_frame != NULL);

  if (proc_frame->progress)
    {
      if (proc_frame->progress_cancel_id)
        {
          g_signal_handler_disconnect (proc_frame->progress,
                                       proc_frame->progress_cancel_id);
          proc_frame->progress_cancel_id = 0;
        }

      if (picman_plug_in_progress_detach (proc_frame->progress) < 1 &&
          picman_progress_is_active (proc_frame->progress))
        {
          picman_progress_end (proc_frame->progress);
        }

      if (proc_frame->progress_created)
        {
          picman_free_progress (plug_in->manager->picman, proc_frame->progress);
          g_object_unref (proc_frame->progress);
          proc_frame->progress = NULL;
        }
    }
}

void
picman_plug_in_progress_set_text (PicmanPlugIn  *plug_in,
                                const gchar *message)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (proc_frame->progress)
    picman_progress_set_text (proc_frame->progress, message);
}

void
picman_plug_in_progress_set_value (PicmanPlugIn *plug_in,
                                 gdouble     percentage)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (! proc_frame->progress                           ||
      ! picman_progress_is_active (proc_frame->progress) ||
      ! proc_frame->progress_cancel_id)
    {
      picman_plug_in_progress_start (plug_in, NULL, NULL);
    }

  if (proc_frame->progress && picman_progress_is_active (proc_frame->progress))
    picman_progress_set_value (proc_frame->progress, percentage);
}

void
picman_plug_in_progress_pulse (PicmanPlugIn *plug_in)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (! proc_frame->progress                           ||
      ! picman_progress_is_active (proc_frame->progress) ||
      ! proc_frame->progress_cancel_id)
    {
      picman_plug_in_progress_start (plug_in, NULL, NULL);
    }

  if (proc_frame->progress && picman_progress_is_active (proc_frame->progress))
    picman_progress_pulse (proc_frame->progress);
}

guint32
picman_plug_in_progress_get_window_id (PicmanPlugIn *plug_in)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), 0);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (proc_frame->progress)
    return picman_progress_get_window_id (proc_frame->progress);

  return 0;
}

gboolean
picman_plug_in_progress_install (PicmanPlugIn  *plug_in,
                               const gchar *progress_callback)
{
  PicmanPlugInProcFrame *proc_frame;
  PicmanProcedure       *procedure;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (progress_callback != NULL, FALSE);

  procedure = picman_pdb_lookup_procedure (plug_in->manager->picman->pdb,
                                         progress_callback);

  if (! PICMAN_IS_TEMPORARY_PROCEDURE (procedure)                ||
      PICMAN_TEMPORARY_PROCEDURE (procedure)->plug_in != plug_in ||
      procedure->num_args                           != 3       ||
      ! PICMAN_IS_PARAM_SPEC_INT32 (procedure->args[0])          ||
      ! G_IS_PARAM_SPEC_STRING   (procedure->args[1])          ||
      ! G_IS_PARAM_SPEC_DOUBLE   (procedure->args[2]))
    {
      return FALSE;
    }

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (proc_frame->progress)
    {
      picman_plug_in_progress_end (plug_in, proc_frame);

      if (proc_frame->progress)
        {
          g_object_unref (proc_frame->progress);
          proc_frame->progress = NULL;
        }
    }

  proc_frame->progress = g_object_new (PICMAN_TYPE_PDB_PROGRESS,
                                       "pdb",           plug_in->manager->picman->pdb,
                                       "context",       proc_frame->main_context,
                                       "callback-name", progress_callback,
                                       NULL);

  picman_plug_in_progress_attach (proc_frame->progress);

  return TRUE;
}

gboolean
picman_plug_in_progress_uninstall (PicmanPlugIn  *plug_in,
                                 const gchar *progress_callback)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (progress_callback != NULL, FALSE);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (PICMAN_IS_PDB_PROGRESS (proc_frame->progress))
    {
      picman_plug_in_progress_end (plug_in, proc_frame);
      g_object_unref (proc_frame->progress);
      proc_frame->progress = NULL;

      return TRUE;
    }

  return FALSE;
}

gboolean
picman_plug_in_progress_cancel (PicmanPlugIn  *plug_in,
                              const gchar *progress_callback)
{
  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (progress_callback != NULL, FALSE);

  return FALSE;
}


/*  private functions  */

static PicmanValueArray *
get_cancel_return_values (PicmanProcedure *procedure)
{
  PicmanValueArray *return_vals;
  GError         *error;

  error = g_error_new_literal (PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_CANCELLED,
                               _("Cancelled"));
  return_vals = picman_procedure_get_return_values (procedure, FALSE, error);
  g_error_free (error);

  return return_vals;
}

static void
picman_plug_in_progress_cancel_callback (PicmanProgress *progress,
                                       PicmanPlugIn   *plug_in)
{
  PicmanPlugInProcFrame *proc_frame = &plug_in->main_proc_frame;
  GList               *list;

  if (proc_frame->main_loop)
    {
      proc_frame->return_vals =
        get_cancel_return_values (proc_frame->procedure);
    }

  for (list = plug_in->temp_proc_frames; list; list = g_list_next (list))
    {
      proc_frame = list->data;

      if (proc_frame->main_loop)
        {
          proc_frame->return_vals =
            get_cancel_return_values (proc_frame->procedure);
        }
    }

  picman_plug_in_close (plug_in, TRUE);
}
