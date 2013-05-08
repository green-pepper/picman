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
#include <stdlib.h>

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"

#include "core/core-types.h"

#include "core/picman.h"
#include "core/picmanparamspecs.h"

#include "batch.h"

#include "pdb/picmanpdb.h"
#include "pdb/picmanprocedure.h"

#include "picman-intl.h"


#define BATCH_DEFAULT_EVAL_PROC   "plug-in-script-fu-eval"


static void  batch_exit_after_callback (Picman          *picman) G_GNUC_NORETURN;

static void  batch_run_cmd             (Picman          *picman,
                                        const gchar   *proc_name,
                                        PicmanProcedure *procedure,
                                        PicmanRunMode    run_mode,
                                        const gchar   *cmd);


void
batch_run (Picman         *picman,
           const gchar  *batch_interpreter,
           const gchar **batch_commands)
{
  gulong  exit_id;

  if (! batch_commands || ! batch_commands[0])
    return;

  exit_id = g_signal_connect_after (picman, "exit",
                                    G_CALLBACK (batch_exit_after_callback),
                                    NULL);

  if (! batch_interpreter)
    {
      batch_interpreter = g_getenv ("PICMAN_BATCH_INTERPRETER");

      if (! batch_interpreter)
        {
          batch_interpreter = BATCH_DEFAULT_EVAL_PROC;

          if (picman->be_verbose)
            g_printerr (_("No batch interpreter specified, using the default "
                          "'%s'.\n"), batch_interpreter);
        }
    }

  /*  script-fu text console, hardcoded for backward compatibility  */

  if (strcmp (batch_interpreter, "plug-in-script-fu-eval") == 0 &&
      strcmp (batch_commands[0], "-") == 0)
    {
      const gchar   *proc_name = "plug-in-script-fu-text-console";
      PicmanProcedure *procedure = picman_pdb_lookup_procedure (picman->pdb,
                                                            proc_name);

      if (procedure)
        batch_run_cmd (picman, proc_name, procedure,
                       PICMAN_RUN_NONINTERACTIVE, NULL);
      else
        g_message (_("The batch interpreter '%s' is not available. "
                     "Batch mode disabled."), proc_name);
    }
  else
    {
      PicmanProcedure *eval_proc = picman_pdb_lookup_procedure (picman->pdb,
                                                            batch_interpreter);

      if (eval_proc)
        {
          gint i;

          for (i = 0; batch_commands[i]; i++)
            batch_run_cmd (picman, batch_interpreter, eval_proc,
                           PICMAN_RUN_NONINTERACTIVE, batch_commands[i]);
        }
      else
        {
          g_message (_("The batch interpreter '%s' is not available. "
                       "Batch mode disabled."), batch_interpreter);
        }
    }

  g_signal_handler_disconnect (picman, exit_id);
}


/*
 * The purpose of this handler is to exit PICMAN cleanly when the batch
 * procedure calls the picman-exit procedure. Without this callback, the
 * message "batch command experienced an execution error" would appear
 * and picman would hang forever.
 */
static void
batch_exit_after_callback (Picman *picman)
{
  if (picman->be_verbose)
    g_print ("EXIT: %s\n", G_STRFUNC);

  gegl_exit ();

  exit (EXIT_SUCCESS);
}

static void
batch_run_cmd (Picman          *picman,
               const gchar   *proc_name,
               PicmanProcedure *procedure,
               PicmanRunMode    run_mode,
               const gchar   *cmd)
{
  PicmanValueArray *args;
  PicmanValueArray *return_vals;
  GError         *error = NULL;
  gint            i     = 0;

  args = picman_procedure_get_arguments (procedure);

  if (procedure->num_args > i &&
      PICMAN_IS_PARAM_SPEC_INT32 (procedure->args[i]))
    g_value_set_int (picman_value_array_index (args, i++), run_mode);

  if (procedure->num_args > i &&
      PICMAN_IS_PARAM_SPEC_STRING (procedure->args[i]))
    g_value_set_static_string (picman_value_array_index (args, i++), cmd);

  return_vals =
    picman_pdb_execute_procedure_by_name_args (picman->pdb,
                                             picman_get_user_context (picman),
                                             NULL, &error,
                                             proc_name, args);

  switch (g_value_get_enum (picman_value_array_index (return_vals, 0)))
    {
    case PICMAN_PDB_EXECUTION_ERROR:
      if (error)
        {
          g_printerr ("batch command experienced an execution error:\n"
                      "%s\n", error->message);
        }
      else
        {
          g_printerr ("batch command experienced an execution error\n");
        }
      break;

    case PICMAN_PDB_CALLING_ERROR:
      if (error)
        {
          g_printerr ("batch command experienced a calling error:\n"
                      "%s\n", error->message);
        }
      else
        {
          g_printerr ("batch command experienced a calling error\n");
        }
      break;

    case PICMAN_PDB_SUCCESS:
      g_printerr ("batch command executed successfully\n");
      break;
    }

  picman_value_array_unref (return_vals);
  picman_value_array_unref (args);

  if (error)
    g_error_free (error);

  return;
}
