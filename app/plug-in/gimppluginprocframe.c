/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginprocframe.c
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

#include "libpicmanbase/picmanbase.h"

#include "plug-in-types.h"

#include "core/picmanprogress.h"

#include "pdb/picmanpdbcontext.h"
#include "pdb/picmanpdberror.h"

#include "picmanplugin.h"
#include "picmanplugin-cleanup.h"
#include "picmanplugin-progress.h"
#include "picmanpluginprocedure.h"

#include "picman-intl.h"


/*  public functions  */

PicmanPlugInProcFrame *
picman_plug_in_proc_frame_new (PicmanContext         *context,
                             PicmanProgress        *progress,
                             PicmanPlugInProcedure *procedure)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_val_if_fail (PICMAN_IS_PDB_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (PICMAN_IS_PLUG_IN_PROCEDURE (procedure), NULL);

  proc_frame = g_slice_new0 (PicmanPlugInProcFrame);

  proc_frame->ref_count = 1;

  picman_plug_in_proc_frame_init (proc_frame, context, progress, procedure);

  return proc_frame;
}

void
picman_plug_in_proc_frame_init (PicmanPlugInProcFrame *proc_frame,
                              PicmanContext         *context,
                              PicmanProgress        *progress,
                              PicmanPlugInProcedure *procedure)
{
  g_return_if_fail (proc_frame != NULL);
  g_return_if_fail (PICMAN_IS_PDB_CONTEXT (context));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (procedure == NULL ||
                    PICMAN_IS_PLUG_IN_PROCEDURE (procedure));

  proc_frame->main_context       = g_object_ref (context);
  proc_frame->context_stack      = NULL;
  proc_frame->procedure          = procedure ? g_object_ref (procedure) : NULL;
  proc_frame->main_loop          = NULL;
  proc_frame->return_vals        = NULL;
  proc_frame->progress           = progress ? g_object_ref (progress) : NULL;
  proc_frame->progress_created   = FALSE;
  proc_frame->progress_cancel_id = 0;
  proc_frame->error_handler      = PICMAN_PDB_ERROR_HANDLER_INTERNAL;

  if (progress)
    picman_plug_in_progress_attach (progress);
}

void
picman_plug_in_proc_frame_dispose (PicmanPlugInProcFrame *proc_frame,
                                 PicmanPlugIn          *plug_in)
{
  g_return_if_fail (proc_frame != NULL);
  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  if (proc_frame->progress)
    {
      picman_plug_in_progress_end (plug_in, proc_frame);

      if (proc_frame->progress)
        {
          g_object_unref (proc_frame->progress);
          proc_frame->progress = NULL;
        }
    }

  if (proc_frame->context_stack)
    {
      g_list_free_full (proc_frame->context_stack,
                        (GDestroyNotify) g_object_unref);
      proc_frame->context_stack = NULL;
    }

  if (proc_frame->main_context)
    {
      g_object_unref (proc_frame->main_context);
      proc_frame->main_context = NULL;
    }

  if (proc_frame->return_vals)
    {
      picman_value_array_unref (proc_frame->return_vals);
      proc_frame->return_vals = NULL;
    }

  if (proc_frame->main_loop)
    {
      g_main_loop_unref (proc_frame->main_loop);
      proc_frame->main_loop = NULL;
    }

  if (proc_frame->image_cleanups || proc_frame->item_cleanups)
    picman_plug_in_cleanup (plug_in, proc_frame);

  if (proc_frame->procedure)
    {
      g_object_unref (proc_frame->procedure);
      proc_frame->procedure = NULL;
    }
}

PicmanPlugInProcFrame *
picman_plug_in_proc_frame_ref (PicmanPlugInProcFrame *proc_frame)
{
  g_return_val_if_fail (proc_frame != NULL, NULL);

  proc_frame->ref_count++;

  return proc_frame;
}

void
picman_plug_in_proc_frame_unref (PicmanPlugInProcFrame *proc_frame,
                               PicmanPlugIn          *plug_in)
{
  g_return_if_fail (proc_frame != NULL);
  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  proc_frame->ref_count--;

  if (proc_frame->ref_count < 1)
    {
      picman_plug_in_proc_frame_dispose (proc_frame, plug_in);
      g_slice_free (PicmanPlugInProcFrame, proc_frame);
    }
}

PicmanValueArray *
picman_plug_in_proc_frame_get_return_values (PicmanPlugInProcFrame *proc_frame)
{
  PicmanValueArray *return_vals;

  g_return_val_if_fail (proc_frame != NULL, NULL);

  if (proc_frame->return_vals)
    {
      if (picman_value_array_length (proc_frame->return_vals) >=
          proc_frame->procedure->num_values + 1)
        {
          return_vals = proc_frame->return_vals;
        }
      else
        {
          /* Allocate new return values of the correct size. */
          return_vals = picman_procedure_get_return_values (proc_frame->procedure,
                                                          TRUE, NULL);

          /* Copy all of the arguments we can. */
          memcpy (picman_value_array_index (return_vals, 0),
                  picman_value_array_index (proc_frame->return_vals, 0),
                  sizeof (GValue) *
                  picman_value_array_length (proc_frame->return_vals));

          /* Free the old arguments. */
          memset (picman_value_array_index (proc_frame->return_vals, 0), 0,
                  sizeof (GValue) *
                  picman_value_array_length (proc_frame->return_vals));
          picman_value_array_unref (proc_frame->return_vals);
        }

      /* We have consumed any saved values, so clear them. */
      proc_frame->return_vals = NULL;
    }
  else
    {
      PicmanProcedure *procedure = proc_frame->procedure;
      GError        *error;

      error = g_error_new (PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_RETURN_VALUE,
                           _("Procedure '%s' returned no return values"),
                           picman_object_get_name (procedure));

      return_vals = picman_procedure_get_return_values (procedure, FALSE,
                                                      error);
      g_error_free (error);
    }

  return return_vals;
}
