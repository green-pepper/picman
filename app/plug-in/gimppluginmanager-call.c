/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginmanager-call.c
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
#include "libpicmanbase/picmanprotocol.h"
#include "libpicmanbase/picmanwire.h"

#include "plug-in-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmanprogress.h"

#include "pdb/picmanpdbcontext.h"

#include "picmanplugin.h"
#include "picmanplugin-message.h"
#include "picmanplugindef.h"
#include "picmanpluginerror.h"
#include "picmanpluginmanager.h"
#define __YES_I_NEED_PICMAN_PLUG_IN_MANAGER_CALL__
#include "picmanpluginmanager-call.h"
#include "picmanpluginshm.h"
#include "picmantemporaryprocedure.h"
#include "plug-in-params.h"

#include "picman-intl.h"


/*  public functions  */

void
picman_plug_in_manager_call_query (PicmanPlugInManager *manager,
                                 PicmanContext       *context,
                                 PicmanPlugInDef     *plug_in_def)
{
  PicmanPlugIn *plug_in;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_PDB_CONTEXT (context));
  g_return_if_fail (PICMAN_IS_PLUG_IN_DEF (plug_in_def));

  plug_in = picman_plug_in_new (manager, context, NULL,
                              NULL, plug_in_def->prog);

  if (plug_in)
    {
      plug_in->plug_in_def = plug_in_def;

      if (picman_plug_in_open (plug_in, PICMAN_PLUG_IN_CALL_QUERY, TRUE))
        {
          while (plug_in->open)
            {
              PicmanWireMessage msg;

              if (! picman_wire_read_msg (plug_in->my_read, &msg, plug_in))
                {
                  picman_plug_in_close (plug_in, TRUE);
                }
              else
                {
                  picman_plug_in_handle_message (plug_in, &msg);
                  picman_wire_destroy (&msg);
                }
            }
        }

      g_object_unref (plug_in);
    }
}

void
picman_plug_in_manager_call_init (PicmanPlugInManager *manager,
                                PicmanContext       *context,
                                PicmanPlugInDef     *plug_in_def)
{
  PicmanPlugIn *plug_in;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_PDB_CONTEXT (context));
  g_return_if_fail (PICMAN_IS_PLUG_IN_DEF (plug_in_def));

  plug_in = picman_plug_in_new (manager, context, NULL,
                              NULL, plug_in_def->prog);

  if (plug_in)
    {
      plug_in->plug_in_def = plug_in_def;

      if (picman_plug_in_open (plug_in, PICMAN_PLUG_IN_CALL_INIT, TRUE))
        {
          while (plug_in->open)
            {
              PicmanWireMessage msg;

              if (! picman_wire_read_msg (plug_in->my_read, &msg, plug_in))
                {
                  picman_plug_in_close (plug_in, TRUE);
                }
              else
                {
                  picman_plug_in_handle_message (plug_in, &msg);
                  picman_wire_destroy (&msg);
                }
            }
        }

      g_object_unref (plug_in);
    }
}

PicmanValueArray *
picman_plug_in_manager_call_run (PicmanPlugInManager   *manager,
                               PicmanContext         *context,
                               PicmanProgress        *progress,
                               PicmanPlugInProcedure *procedure,
                               PicmanValueArray      *args,
                               gboolean             synchronous,
                               PicmanObject          *display)
{
  PicmanValueArray *return_vals = NULL;
  PicmanPlugIn     *plug_in;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), NULL);
  g_return_val_if_fail (PICMAN_IS_PDB_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (PICMAN_IS_PLUG_IN_PROCEDURE (procedure), NULL);
  g_return_val_if_fail (args != NULL, NULL);
  g_return_val_if_fail (display == NULL || PICMAN_IS_OBJECT (display), NULL);

  plug_in = picman_plug_in_new (manager, context, progress, procedure, NULL);

  if (plug_in)
    {
      PicmanCoreConfig    *core_config    = manager->picman->config;
      PicmanDisplayConfig *display_config = PICMAN_DISPLAY_CONFIG (core_config);
      PicmanGuiConfig     *gui_config     = PICMAN_GUI_CONFIG (core_config);
      GPConfig           config;
      GPProcRun          proc_run;
      gint               display_ID;
      gint               monitor;

      if (! picman_plug_in_open (plug_in, PICMAN_PLUG_IN_CALL_RUN, FALSE))
        {
          const gchar *name  = picman_object_get_name (plug_in);
          GError      *error = g_error_new (PICMAN_PLUG_IN_ERROR,
                                            PICMAN_PLUG_IN_EXECUTION_FAILED,
                                            _("Failed to run plug-in \"%s\""),
                                            name);

          g_object_unref (plug_in);

          return_vals = picman_procedure_get_return_values (PICMAN_PROCEDURE (procedure),
                                                          FALSE, error);
          g_error_free (error);

          return return_vals;
        }

      display_ID = display ? picman_get_display_ID (manager->picman, display) : -1;

      config.version          = PICMAN_PROTOCOL_VERSION;
      config.tile_width       = PICMAN_PLUG_IN_TILE_WIDTH;
      config.tile_height      = PICMAN_PLUG_IN_TILE_HEIGHT;
      config.shm_ID           = (manager->shm ?
                                 picman_plug_in_shm_get_ID (manager->shm) : -1);
      config.check_size       = display_config->transparency_size;
      config.check_type       = display_config->transparency_type;
      config.show_help_button = (gui_config->use_help &&
                                 gui_config->show_help_button);
#ifdef __GNUC__
#warning FIXME what to do with config.use_cpu_accel
#endif
      config.use_cpu_accel    = FALSE;
      config.picman_reserved_5  = 0;
      config.picman_reserved_6  = 0;
      config.picman_reserved_7  = 0;
      config.picman_reserved_8  = 0;
      config.install_cmap     = FALSE;
      config.show_tooltips    = gui_config->show_tooltips;
      config.min_colors       = 144;
      config.gdisp_ID         = display_ID;
      config.app_name         = (gchar *) g_get_application_name ();
      config.wm_class         = (gchar *) picman_get_program_class (manager->picman);
      config.display_name     = picman_get_display_name (manager->picman,
                                                       display_ID, &monitor);
      config.monitor_number   = monitor;
      config.timestamp        = picman_get_user_time (manager->picman);

      proc_run.name    = PICMAN_PROCEDURE (procedure)->original_name;
      proc_run.nparams = picman_value_array_length (args);
      proc_run.params  = plug_in_args_to_params (args, FALSE);

      if (! gp_config_write (plug_in->my_write, &config, plug_in)     ||
          ! gp_proc_run_write (plug_in->my_write, &proc_run, plug_in) ||
          ! picman_wire_flush (plug_in->my_write, plug_in))
        {
          const gchar *name  = picman_object_get_name (plug_in);
          GError      *error = g_error_new (PICMAN_PLUG_IN_ERROR,
                                            PICMAN_PLUG_IN_EXECUTION_FAILED,
                                            _("Failed to run plug-in \"%s\""),
                                            name);

          g_free (config.display_name);
          g_free (proc_run.params);

          g_object_unref (plug_in);

          return_vals = picman_procedure_get_return_values (PICMAN_PROCEDURE (procedure),
                                                          FALSE, error);
          g_error_free (error);

          return return_vals;
        }

      g_free (config.display_name);
      g_free (proc_run.params);

      /* If this is an extension,
       * wait for an installation-confirmation message
       */
      if (PICMAN_PROCEDURE (procedure)->proc_type == PICMAN_EXTENSION)
        {
          plug_in->ext_main_loop = g_main_loop_new (NULL, FALSE);

          picman_threads_leave (manager->picman);
          g_main_loop_run (plug_in->ext_main_loop);
          picman_threads_enter (manager->picman);

          /*  main_loop is quit in picman_plug_in_handle_extension_ack()  */

          g_main_loop_unref (plug_in->ext_main_loop);
          plug_in->ext_main_loop = NULL;
        }

      /* If this plug-in is requested to run synchronously,
       * wait for its return values
       */
      if (synchronous)
        {
          PicmanPlugInProcFrame *proc_frame = &plug_in->main_proc_frame;

          proc_frame->main_loop = g_main_loop_new (NULL, FALSE);

          picman_threads_leave (manager->picman);
          g_main_loop_run (proc_frame->main_loop);
          picman_threads_enter (manager->picman);

          /*  main_loop is quit in picman_plug_in_handle_proc_return()  */

          g_main_loop_unref (proc_frame->main_loop);
          proc_frame->main_loop = NULL;

          return_vals = picman_plug_in_proc_frame_get_return_values (proc_frame);
        }

      g_object_unref (plug_in);
    }

  return return_vals;
}

PicmanValueArray *
picman_plug_in_manager_call_run_temp (PicmanPlugInManager      *manager,
                                    PicmanContext            *context,
                                    PicmanProgress           *progress,
                                    PicmanTemporaryProcedure *procedure,
                                    PicmanValueArray         *args)
{
  PicmanValueArray *return_vals = NULL;
  PicmanPlugIn     *plug_in;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), NULL);
  g_return_val_if_fail (PICMAN_IS_PDB_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (PICMAN_IS_TEMPORARY_PROCEDURE (procedure), NULL);
  g_return_val_if_fail (args != NULL, NULL);

  plug_in = procedure->plug_in;

  if (plug_in)
    {
      PicmanPlugInProcFrame *proc_frame;
      GPProcRun            proc_run;

      proc_frame = picman_plug_in_proc_frame_push (plug_in, context, progress,
                                                 procedure);

      proc_run.name    = PICMAN_PROCEDURE (procedure)->original_name;
      proc_run.nparams = picman_value_array_length (args);
      proc_run.params  = plug_in_args_to_params (args, FALSE);

      if (! gp_temp_proc_run_write (plug_in->my_write, &proc_run, plug_in) ||
          ! picman_wire_flush (plug_in->my_write, plug_in))
        {
          const gchar *name  = picman_object_get_name (plug_in);
          GError      *error = g_error_new (PICMAN_PLUG_IN_ERROR,
                                            PICMAN_PLUG_IN_EXECUTION_FAILED,
                                            _("Failed to run plug-in \"%s\""),
                                            name);

          g_free (proc_run.params);
          picman_plug_in_proc_frame_pop (plug_in);

          return_vals = picman_procedure_get_return_values (PICMAN_PROCEDURE (procedure),
                                                          FALSE, error);
          g_error_free (error);

          return return_vals;
        }

      g_free (proc_run.params);

      g_object_ref (plug_in);
      picman_plug_in_proc_frame_ref (proc_frame);

      picman_plug_in_main_loop (plug_in);

      /*  main_loop is quit and proc_frame is popped in
       *  picman_plug_in_handle_temp_proc_return()
       */

      return_vals = picman_plug_in_proc_frame_get_return_values (proc_frame);

      picman_plug_in_proc_frame_unref (proc_frame, plug_in);
      g_object_unref (plug_in);
    }

  return return_vals;
}
