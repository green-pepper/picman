/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantemporaryprocedure.c
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

#include "libpicmanbase/picmanbase.h"

#include "plug-in-types.h"

#include "core/picman.h"

#include "picmanplugin.h"
#define __YES_I_NEED_PICMAN_PLUG_IN_MANAGER_CALL__
#include "picmanpluginmanager-call.h"
#include "picmantemporaryprocedure.h"

#include "picman-intl.h"


static void             picman_temporary_procedure_finalize (GObject        *object);

static PicmanValueArray * picman_temporary_procedure_execute  (PicmanProcedure  *procedure,
                                                           Picman           *picman,
                                                           PicmanContext    *context,
                                                           PicmanProgress   *progress,
                                                           PicmanValueArray *args,
                                                           GError        **error);
static void        picman_temporary_procedure_execute_async (PicmanProcedure  *procedure,
                                                           Picman           *picman,
                                                           PicmanContext    *context,
                                                           PicmanProgress   *progress,
                                                           PicmanValueArray *args,
                                                           PicmanObject     *display);

const gchar       * picman_temporary_procedure_get_progname (const PicmanPlugInProcedure *procedure);


G_DEFINE_TYPE (PicmanTemporaryProcedure, picman_temporary_procedure,
               PICMAN_TYPE_PLUG_IN_PROCEDURE)

#define parent_class picman_temporary_procedure_parent_class


static void
picman_temporary_procedure_class_init (PicmanTemporaryProcedureClass *klass)
{
  GObjectClass             *object_class = G_OBJECT_CLASS (klass);
  PicmanProcedureClass       *proc_class   = PICMAN_PROCEDURE_CLASS (klass);
  PicmanPlugInProcedureClass *plug_class   = PICMAN_PLUG_IN_PROCEDURE_CLASS (klass);

  object_class->finalize    = picman_temporary_procedure_finalize;

  proc_class->execute       = picman_temporary_procedure_execute;
  proc_class->execute_async = picman_temporary_procedure_execute_async;

  plug_class->get_progname  = picman_temporary_procedure_get_progname;
}

static void
picman_temporary_procedure_init (PicmanTemporaryProcedure *proc)
{
  PICMAN_PROCEDURE (proc)->proc_type = PICMAN_TEMPORARY;
}

static void
picman_temporary_procedure_finalize (GObject *object)
{
  /* PicmanTemporaryProcedure *proc = PICMAN_TEMPORARY_PROCEDURE (object); */

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static PicmanValueArray *
picman_temporary_procedure_execute (PicmanProcedure   *procedure,
                                  Picman            *picman,
                                  PicmanContext     *context,
                                  PicmanProgress    *progress,
                                  PicmanValueArray  *args,
                                  GError         **error)
{
  return picman_plug_in_manager_call_run_temp (picman->plug_in_manager,
                                             context, progress,
                                             PICMAN_TEMPORARY_PROCEDURE (procedure),
                                             args);
}

static void
picman_temporary_procedure_execute_async (PicmanProcedure  *procedure,
                                        Picman           *picman,
                                        PicmanContext    *context,
                                        PicmanProgress   *progress,
                                        PicmanValueArray *args,
                                        PicmanObject     *display)
{
  PicmanTemporaryProcedure *temp_procedure = PICMAN_TEMPORARY_PROCEDURE (procedure);
  PicmanValueArray         *return_vals;

  return_vals = picman_plug_in_manager_call_run_temp (picman->plug_in_manager,
                                                    context, progress,
                                                    temp_procedure,
                                                    args);

  if (return_vals)
    {
      PicmanPlugInProcedure *proc = PICMAN_PLUG_IN_PROCEDURE (procedure);

      picman_plug_in_procedure_handle_return_values (proc,
                                                   picman, progress,
                                                   return_vals);
      picman_value_array_unref (return_vals);
    }
}

const gchar *
picman_temporary_procedure_get_progname (const PicmanPlugInProcedure *procedure)
{
  return PICMAN_TEMPORARY_PROCEDURE (procedure)->plug_in->prog;
}


/*  public functions  */

PicmanProcedure *
picman_temporary_procedure_new (PicmanPlugIn *plug_in)
{
  PicmanTemporaryProcedure *proc;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), NULL);

  proc = g_object_new (PICMAN_TYPE_TEMPORARY_PROCEDURE, NULL);

  proc->plug_in = plug_in;

  PICMAN_PLUG_IN_PROCEDURE (proc)->prog = g_strdup ("none");

  return PICMAN_PROCEDURE (proc);
}
