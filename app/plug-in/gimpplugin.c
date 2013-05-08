/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanplugin.c
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

#ifndef _WIN32
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include <gegl.h>

#if defined(G_OS_WIN32) || defined(G_WITH_CYGWIN)

#define STRICT
#include <windows.h>
#include <process.h>

#ifdef G_OS_WIN32
#include <fcntl.h>
#include <io.h>

#ifndef pipe
#define pipe(fds) _pipe(fds, 4096, _O_BINARY)
#endif

#endif

#ifdef G_WITH_CYGWIN
#define O_TEXT    0x0100  /* text file   */
#define _O_TEXT   0x0100  /* text file   */
#define O_BINARY  0x0200  /* binary file */
#define _O_BINARY 0x0200  /* binary file */
#endif

#endif /* G_OS_WIN32 || G_WITH_CYGWIN */

#include "libpicmanbase/picmanbase.h"
#include "libpicmanbase/picmanprotocol.h"
#include "libpicmanbase/picmanwire.h"

#include "plug-in-types.h"

#include "core/picman.h"
#include "core/picmanprogress.h"

#include "pdb/picmanpdbcontext.h"

#include "picmanenvirontable.h"
#include "picmaninterpreterdb.h"
#include "picmanplugin.h"
#include "picmanplugin-message.h"
#include "picmanplugin-progress.h"
#include "picmanplugindebug.h"
#include "picmanplugindef.h"
#include "picmanpluginmanager.h"
#include "picmanpluginmanager-help-domain.h"
#include "picmanpluginmanager-locale-domain.h"
#include "picmantemporaryprocedure.h"
#include "plug-in-params.h"

#include "picman-intl.h"


static void       picman_plug_in_finalize      (GObject      *object);

static gboolean   picman_plug_in_write         (GIOChannel   *channel,
                                              const guint8 *buf,
                                              gulong        count,
                                              gpointer      data);
static gboolean   picman_plug_in_flush         (GIOChannel   *channel,
                                              gpointer      data);

static gboolean   picman_plug_in_recv_message  (GIOChannel   *channel,
                                              GIOCondition  cond,
                                              gpointer      data);

#if !defined(G_OS_WIN32) && !defined (G_WITH_CYGWIN)
static void       picman_plug_in_prep_for_exec (gpointer      data);
#else
#define           picman_plug_in_prep_for_exec  NULL
#endif


G_DEFINE_TYPE (PicmanPlugIn, picman_plug_in, PICMAN_TYPE_OBJECT)

#define parent_class picman_plug_in_parent_class


static void
picman_plug_in_class_init (PicmanPlugInClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = picman_plug_in_finalize;

  /* initialize the picman protocol library and set the read and
   *  write handlers.
   */
  gp_init ();
  picman_wire_set_writer (picman_plug_in_write);
  picman_wire_set_flusher (picman_plug_in_flush);
}

static void
picman_plug_in_init (PicmanPlugIn *plug_in)
{
  plug_in->manager            = NULL;
  plug_in->prog               = NULL;

  plug_in->call_mode          = PICMAN_PLUG_IN_CALL_NONE;
  plug_in->open               = FALSE;
  plug_in->hup                = FALSE;
  plug_in->pid                = 0;

  plug_in->my_read            = NULL;
  plug_in->my_write           = NULL;
  plug_in->his_read           = NULL;
  plug_in->his_write          = NULL;

  plug_in->input_id           = 0;
  plug_in->write_buffer_index = 0;

  plug_in->temp_procedures    = NULL;

  plug_in->ext_main_loop      = NULL;

  plug_in->temp_proc_frames   = NULL;

  plug_in->plug_in_def        = NULL;
}

static void
picman_plug_in_finalize (GObject *object)
{
  PicmanPlugIn *plug_in = PICMAN_PLUG_IN (object);

  g_free (plug_in->prog);

  picman_plug_in_proc_frame_dispose (&plug_in->main_proc_frame, plug_in);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


/*  public functions  */

PicmanPlugIn *
picman_plug_in_new (PicmanPlugInManager   *manager,
                  PicmanContext         *context,
                  PicmanProgress        *progress,
                  PicmanPlugInProcedure *procedure,
                  const gchar         *prog)
{
  PicmanPlugIn *plug_in;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), NULL);
  g_return_val_if_fail (PICMAN_IS_PDB_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (procedure == NULL ||
                        PICMAN_IS_PLUG_IN_PROCEDURE (procedure), NULL);
  g_return_val_if_fail (prog == NULL || g_path_is_absolute (prog), NULL);
  g_return_val_if_fail ((procedure != NULL || prog != NULL) &&
                        ! (procedure != NULL && prog != NULL), NULL);

  plug_in = g_object_new (PICMAN_TYPE_PLUG_IN, NULL);

  if (! prog)
    prog = picman_plug_in_procedure_get_progname (procedure);

  picman_object_take_name (PICMAN_OBJECT (plug_in),
                         g_filename_display_basename (prog));

  plug_in->manager = manager;
  plug_in->prog    = g_strdup (prog);

  picman_plug_in_proc_frame_init (&plug_in->main_proc_frame,
                                context, progress, procedure);

  return plug_in;
}

gboolean
picman_plug_in_open (PicmanPlugIn         *plug_in,
                   PicmanPlugInCallMode  call_mode,
                   gboolean            synchronous)
{
  gint          my_read[2];
  gint          my_write[2];
  gchar       **envp;
  const gchar  *args[9];
  gchar       **argv;
  gint          argc;
  gchar        *interp, *interp_arg;
  gchar        *read_fd, *write_fd;
  const gchar  *mode;
  gchar        *stm;
  GError       *error = NULL;
  gboolean      debug;
  guint         debug_flag;
  guint         spawn_flags;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (plug_in->call_mode == PICMAN_PLUG_IN_CALL_NONE, FALSE);

  /* Open two pipes. (Bidirectional communication).
   */
  if ((pipe (my_read) == -1) || (pipe (my_write) == -1))
    {
      picman_message (plug_in->manager->picman, NULL, PICMAN_MESSAGE_ERROR,
                    "Unable to run plug-in \"%s\"\n(%s)\n\npipe() failed: %s",
                    picman_object_get_name (plug_in),
                    picman_filename_to_utf8 (plug_in->prog),
                    g_strerror (errno));
      return FALSE;
    }

#if defined(G_WITH_CYGWIN)
  /* Set to binary mode */
  setmode (my_read[0], _O_BINARY);
  setmode (my_write[0], _O_BINARY);
  setmode (my_read[1], _O_BINARY);
  setmode (my_write[1], _O_BINARY);
#endif

#ifdef G_OS_WIN32
  /* Prevent the plug-in from inheriting our ends of the pipes */
  SetHandleInformation ((HANDLE) _get_osfhandle (my_read[0]), HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation ((HANDLE) _get_osfhandle (my_write[1]), HANDLE_FLAG_INHERIT, 0);
#endif

  plug_in->my_read   = g_io_channel_unix_new (my_read[0]);
  plug_in->my_write  = g_io_channel_unix_new (my_write[1]);
  plug_in->his_read  = g_io_channel_unix_new (my_write[0]);
  plug_in->his_write = g_io_channel_unix_new (my_read[1]);

  g_io_channel_set_encoding (plug_in->my_read, NULL, NULL);
  g_io_channel_set_encoding (plug_in->my_write, NULL, NULL);
  g_io_channel_set_encoding (plug_in->his_read, NULL, NULL);
  g_io_channel_set_encoding (plug_in->his_write, NULL, NULL);

  g_io_channel_set_buffered (plug_in->my_read, FALSE);
  g_io_channel_set_buffered (plug_in->my_write, FALSE);
  g_io_channel_set_buffered (plug_in->his_read, FALSE);
  g_io_channel_set_buffered (plug_in->his_write, FALSE);

  g_io_channel_set_close_on_unref (plug_in->my_read, TRUE);
  g_io_channel_set_close_on_unref (plug_in->my_write, TRUE);
  g_io_channel_set_close_on_unref (plug_in->his_read, TRUE);
  g_io_channel_set_close_on_unref (plug_in->his_write, TRUE);

  /* Remember the file descriptors for the pipes.
   */
  read_fd  = g_strdup_printf ("%d",
                              g_io_channel_unix_get_fd (plug_in->his_read));
  write_fd = g_strdup_printf ("%d",
                              g_io_channel_unix_get_fd (plug_in->his_write));

  switch (call_mode)
    {
    case PICMAN_PLUG_IN_CALL_QUERY:
      mode = "-query";
      debug_flag = PICMAN_DEBUG_WRAP_QUERY;
      break;

    case PICMAN_PLUG_IN_CALL_INIT:
      mode = "-init";
      debug_flag = PICMAN_DEBUG_WRAP_INIT;
      break;

    case PICMAN_PLUG_IN_CALL_RUN:
      mode = "-run";
      debug_flag = PICMAN_DEBUG_WRAP_RUN;
      break;

    default:
      g_assert_not_reached ();
    }

  stm = g_strdup_printf ("%d", plug_in->manager->picman->stack_trace_mode);

  interp = picman_interpreter_db_resolve (plug_in->manager->interpreter_db,
                                        plug_in->prog, &interp_arg);

  argc = 0;

  if (interp)
    args[argc++] = interp;

  if (interp_arg)
    args[argc++] = interp_arg;

  args[argc++] = plug_in->prog;
  args[argc++] = "-picman";
  args[argc++] = read_fd;
  args[argc++] = write_fd;
  args[argc++] = mode;
  args[argc++] = stm;
  args[argc++] = NULL;

  argv = (gchar **) args;
  envp = picman_environ_table_get_envp (plug_in->manager->environ_table);
  spawn_flags = (G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
                 G_SPAWN_DO_NOT_REAP_CHILD      |
                 G_SPAWN_CHILD_INHERITS_STDIN);

  debug = FALSE;

  if (plug_in->manager->debug)
    {
      gchar **debug_argv = picman_plug_in_debug_argv (plug_in->manager->debug,
                                                    plug_in->prog,
                                                    debug_flag, args);

      if (debug_argv)
        {
          debug = TRUE;
          argv = debug_argv;
          spawn_flags |= G_SPAWN_SEARCH_PATH;
        }
    }

  /* Fork another process. We'll remember the process id so that we
   * can later use it to kill the filter if necessary.
   */
  if (! g_spawn_async (NULL, argv, envp, spawn_flags,
                       picman_plug_in_prep_for_exec, plug_in,
                       &plug_in->pid,
                       &error))
    {
      picman_message (plug_in->manager->picman, NULL, PICMAN_MESSAGE_ERROR,
                    "Unable to run plug-in \"%s\"\n(%s)\n\n%s",
                    picman_object_get_name (plug_in),
                    picman_filename_to_utf8 (plug_in->prog),
                    error->message);
      g_error_free (error);
      goto cleanup;
    }

  g_io_channel_unref (plug_in->his_read);
  plug_in->his_read = NULL;

  g_io_channel_unref (plug_in->his_write);
  plug_in->his_write = NULL;

  if (! synchronous)
    {
      GSource *source;

      source = g_io_create_watch (plug_in->my_read,
                                  G_IO_IN  | G_IO_PRI | G_IO_ERR | G_IO_HUP);

      g_source_set_callback (source,
                             (GSourceFunc) picman_plug_in_recv_message, plug_in,
                             NULL);

      g_source_set_can_recurse (source, TRUE);

      plug_in->input_id = g_source_attach (source, NULL);
      g_source_unref (source);
    }

  plug_in->open      = TRUE;
  plug_in->call_mode = call_mode;

  picman_plug_in_manager_add_open_plug_in (plug_in->manager, plug_in);

 cleanup:

  if (debug)
    g_free (argv);

  g_free (read_fd);
  g_free (write_fd);
  g_free (stm);
  g_free (interp);
  g_free (interp_arg);

  return plug_in->open;
}

void
picman_plug_in_close (PicmanPlugIn *plug_in,
                    gboolean    kill_it)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));
  g_return_if_fail (plug_in->open);

  plug_in->open = FALSE;

  if (plug_in->pid)
    {
#ifndef G_OS_WIN32
      gint status;
#endif

      /*  Ask the filter to exit gracefully,
          but not if it is closed because of a broken pipe.  */
      if (kill_it && ! plug_in->hup)
        {
          gp_quit_write (plug_in->my_write, plug_in);

          /*  give the plug-in some time (10 ms)  */
          g_usleep (10000);
        }

      /* If necessary, kill the filter. */

#ifndef G_OS_WIN32

      if (kill_it)
        {
          if (plug_in->manager->picman->be_verbose)
            g_print ("Terminating plug-in: '%s'\n",
                     picman_filename_to_utf8 (plug_in->prog));

          /*  If the plug-in opened a process group, kill the group instead
           *  of only the plug-in, so we kill the plug-in's children too
           */
          if (getpgid (0) != getpgid (plug_in->pid))
            status = kill (- plug_in->pid, SIGKILL);
          else
            status = kill (plug_in->pid, SIGKILL);
        }

      /* Wait for the process to exit. This will happen
       * immediately if it was just killed.
       */
      waitpid (plug_in->pid, &status, 0);

#else /* G_OS_WIN32 */

      if (kill_it)
        {
          /* Trying to avoid TerminateProcess (does mostly work).
           * Otherwise some of our needed DLLs may get into an
           * unstable state (see Win32 API docs).
           */
          DWORD dwExitCode = STILL_ACTIVE;
          DWORD dwTries    = 10;

          while (dwExitCode == STILL_ACTIVE &&
                 GetExitCodeProcess ((HANDLE) plug_in->pid, &dwExitCode) &&
                 (dwTries > 0))
            {
              Sleep (10);
              dwTries--;
            }

          if (dwExitCode == STILL_ACTIVE)
            {
              if (plug_in->manager->picman->be_verbose)
                g_print ("Terminating plug-in: '%s'\n",
                         picman_filename_to_utf8 (plug_in->prog));

              TerminateProcess ((HANDLE) plug_in->pid, 0);
            }
        }

#endif /* G_OS_WIN32 */

      g_spawn_close_pid (plug_in->pid);
      plug_in->pid = 0;
    }

  /* Remove the input handler. */
  if (plug_in->input_id)
    {
      g_source_remove (plug_in->input_id);
      plug_in->input_id = 0;
    }

  /* Close the pipes. */
  if (plug_in->my_read != NULL)
    {
      g_io_channel_unref (plug_in->my_read);
      plug_in->my_read = NULL;
    }
  if (plug_in->my_write != NULL)
    {
      g_io_channel_unref (plug_in->my_write);
      plug_in->my_write = NULL;
    }
  if (plug_in->his_read != NULL)
    {
      g_io_channel_unref (plug_in->his_read);
      plug_in->his_read = NULL;
    }
  if (plug_in->his_write != NULL)
    {
      g_io_channel_unref (plug_in->his_write);
      plug_in->his_write = NULL;
    }

  picman_wire_clear_error ();

  while (plug_in->temp_proc_frames)
    {
      PicmanPlugInProcFrame *proc_frame = plug_in->temp_proc_frames->data;

#ifdef PICMAN_UNSTABLE
      g_printerr ("plug-in '%s' aborted before sending its "
                  "temporary procedure return values\n",
                  picman_object_get_name (plug_in));
#endif

      if (proc_frame->main_loop &&
          g_main_loop_is_running (proc_frame->main_loop))
        {
          g_main_loop_quit (proc_frame->main_loop);
        }

      /* pop the frame here, because normally this only happens in
       * picman_plug_in_handle_temp_proc_return(), which can't
       * be called after plug_in_close()
       */
      picman_plug_in_proc_frame_pop (plug_in);
    }

  if (plug_in->main_proc_frame.main_loop &&
      g_main_loop_is_running (plug_in->main_proc_frame.main_loop))
    {
#ifdef PICMAN_UNSTABLE
      g_printerr ("plug-in '%s' aborted before sending its "
                  "procedure return values\n",
                  picman_object_get_name (plug_in));
#endif

      g_main_loop_quit (plug_in->main_proc_frame.main_loop);
    }

  if (plug_in->ext_main_loop &&
      g_main_loop_is_running (plug_in->ext_main_loop))
    {
#ifdef PICMAN_UNSTABLE
      g_printerr ("extension '%s' aborted before sending its "
                  "extension_ack message\n",
                  picman_object_get_name (plug_in));
#endif

      g_main_loop_quit (plug_in->ext_main_loop);
    }

  /* Unregister any temporary procedures. */
  while (plug_in->temp_procedures)
    picman_plug_in_remove_temp_proc (plug_in, plug_in->temp_procedures->data);

  picman_plug_in_manager_remove_open_plug_in (plug_in->manager, plug_in);
}

static gboolean
picman_plug_in_recv_message (GIOChannel   *channel,
                           GIOCondition  cond,
                           gpointer      data)
{
  PicmanPlugIn *plug_in     = data;
  gboolean    got_message = FALSE;

#ifdef G_OS_WIN32
  /* Workaround for GLib bug #137968: sometimes we are called for no
   * reason...
   */
  if (cond == 0)
    return TRUE;
#endif

  if (plug_in->my_read == NULL)
    return TRUE;

  g_object_ref (plug_in);

  if (cond & (G_IO_IN | G_IO_PRI))
    {
      PicmanWireMessage msg;

      memset (&msg, 0, sizeof (PicmanWireMessage));

      if (! picman_wire_read_msg (plug_in->my_read, &msg, plug_in))
        {
          picman_plug_in_close (plug_in, TRUE);
        }
      else
        {
          picman_plug_in_handle_message (plug_in, &msg);
          picman_wire_destroy (&msg);
          got_message = TRUE;
        }
    }

  if (cond & (G_IO_ERR | G_IO_HUP))
    {
      if (cond & G_IO_HUP)
        plug_in->hup = TRUE;

      if (plug_in->open)
        picman_plug_in_close (plug_in, TRUE);
    }

  if (! got_message)
    {
      PicmanPlugInProcFrame *frame    = picman_plug_in_get_proc_frame (plug_in);
      PicmanProgress        *progress = frame ? frame->progress : NULL;

      picman_message (plug_in->manager->picman, G_OBJECT (progress),
                    PICMAN_MESSAGE_ERROR,
                    _("Plug-in crashed: \"%s\"\n(%s)\n\n"
                      "The dying plug-in may have messed up PICMAN's internal "
                      "state. You may want to save your images and restart "
                      "PICMAN to be on the safe side."),
                    picman_object_get_name (plug_in),
                    picman_filename_to_utf8 (plug_in->prog));
    }

  g_object_unref (plug_in);

  return TRUE;
}

static gboolean
picman_plug_in_write (GIOChannel   *channel,
                    const guint8 *buf,
                    gulong        count,
                    gpointer      data)
{
  PicmanPlugIn *plug_in = data;
  gulong      bytes;

  while (count > 0)
    {
      if ((plug_in->write_buffer_index + count) >= WRITE_BUFFER_SIZE)
        {
          bytes = WRITE_BUFFER_SIZE - plug_in->write_buffer_index;
          memcpy (&plug_in->write_buffer[plug_in->write_buffer_index],
                  buf, bytes);
          plug_in->write_buffer_index += bytes;
          if (! picman_wire_flush (channel, plug_in))
            return FALSE;
        }
      else
        {
          bytes = count;
          memcpy (&plug_in->write_buffer[plug_in->write_buffer_index],
                  buf, bytes);
          plug_in->write_buffer_index += bytes;
        }

      buf += bytes;
      count -= bytes;
    }

  return TRUE;
}

static gboolean
picman_plug_in_flush (GIOChannel *channel,
                    gpointer    data)
{
  PicmanPlugIn *plug_in = data;

  if (plug_in->write_buffer_index > 0)
    {
      GIOStatus  status;
      GError    *error = NULL;
      gint       count;
      gsize      bytes;

      count = 0;
      while (count != plug_in->write_buffer_index)
        {
          do
            {
              bytes = 0;
              status = g_io_channel_write_chars (channel,
                                                 &plug_in->write_buffer[count],
                                                 (plug_in->write_buffer_index - count),
                                                 &bytes,
                                                 &error);
            }
          while (status == G_IO_STATUS_AGAIN);

          if (status != G_IO_STATUS_NORMAL)
            {
              if (error)
                {
                  g_warning ("%s: plug_in_flush(): error: %s",
                             picman_filename_to_utf8 (g_get_prgname ()),
                             error->message);
                  g_error_free (error);
                }
              else
                {
                  g_warning ("%s: plug_in_flush(): error",
                             picman_filename_to_utf8 (g_get_prgname ()));
                }

              return FALSE;
            }

          count += bytes;
        }

      plug_in->write_buffer_index = 0;
    }

  return TRUE;
}

#if !defined(G_OS_WIN32) && !defined (G_WITH_CYGWIN)

static void
picman_plug_in_prep_for_exec (gpointer data)
{
  PicmanPlugIn *plug_in = data;

  g_io_channel_unref (plug_in->my_read);
  plug_in->my_read  = NULL;

  g_io_channel_unref (plug_in->my_write);
  plug_in->my_write  = NULL;
}

#endif

PicmanPlugInProcFrame *
picman_plug_in_get_proc_frame (PicmanPlugIn *plug_in)
{
  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), NULL);

  if (plug_in->temp_proc_frames)
    return plug_in->temp_proc_frames->data;
  else
    return &plug_in->main_proc_frame;
}

PicmanPlugInProcFrame *
picman_plug_in_proc_frame_push (PicmanPlugIn             *plug_in,
                              PicmanContext            *context,
                              PicmanProgress           *progress,
                              PicmanTemporaryProcedure *procedure)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), NULL);
  g_return_val_if_fail (PICMAN_IS_PDB_CONTEXT (context), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);
  g_return_val_if_fail (PICMAN_IS_TEMPORARY_PROCEDURE (procedure), NULL);

  proc_frame = picman_plug_in_proc_frame_new (context, progress,
                                            PICMAN_PLUG_IN_PROCEDURE (procedure));

  plug_in->temp_proc_frames = g_list_prepend (plug_in->temp_proc_frames,
                                              proc_frame);

  return proc_frame;
}

void
picman_plug_in_proc_frame_pop (PicmanPlugIn *plug_in)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));
  g_return_if_fail (plug_in->temp_proc_frames != NULL);

  proc_frame = (PicmanPlugInProcFrame *) plug_in->temp_proc_frames->data;

  picman_plug_in_proc_frame_unref (proc_frame, plug_in);

  plug_in->temp_proc_frames = g_list_remove (plug_in->temp_proc_frames,
                                             proc_frame);
}

void
picman_plug_in_main_loop (PicmanPlugIn *plug_in)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));
  g_return_if_fail (plug_in->temp_proc_frames != NULL);

  proc_frame = (PicmanPlugInProcFrame *) plug_in->temp_proc_frames->data;

  g_return_if_fail (proc_frame->main_loop == NULL);

  proc_frame->main_loop = g_main_loop_new (NULL, FALSE);

  picman_threads_leave (plug_in->manager->picman);
  g_main_loop_run (proc_frame->main_loop);
  picman_threads_enter (plug_in->manager->picman);

  g_main_loop_unref (proc_frame->main_loop);
  proc_frame->main_loop = NULL;
}

void
picman_plug_in_main_loop_quit (PicmanPlugIn *plug_in)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));
  g_return_if_fail (plug_in->temp_proc_frames != NULL);

  proc_frame = (PicmanPlugInProcFrame *) plug_in->temp_proc_frames->data;

  g_return_if_fail (proc_frame->main_loop != NULL);

  g_main_loop_quit (proc_frame->main_loop);
}

const gchar *
picman_plug_in_get_undo_desc (PicmanPlugIn *plug_in)
{
  PicmanPlugInProcFrame *proc_frame;
  const gchar         *undo_desc = NULL;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), NULL);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (proc_frame)
    {
      PicmanPlugInProcedure *proc;

      proc = PICMAN_PLUG_IN_PROCEDURE (proc_frame->procedure);

      if (proc)
        undo_desc = picman_plug_in_procedure_get_label (proc);
    }

  return undo_desc ? undo_desc : picman_object_get_name (plug_in);
}

/*  called from the PDB (picman_plugin_menu_register)  */
gboolean
picman_plug_in_menu_register (PicmanPlugIn  *plug_in,
                            const gchar *proc_name,
                            const gchar *menu_path)
{
  PicmanPlugInProcedure *proc  = NULL;
  GError              *error = NULL;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (proc_name != NULL, FALSE);
  g_return_val_if_fail (menu_path != NULL, FALSE);

  if (plug_in->plug_in_def)
    proc = picman_plug_in_procedure_find (plug_in->plug_in_def->procedures,
                                        proc_name);

  if (! proc)
    proc = picman_plug_in_procedure_find (plug_in->temp_procedures, proc_name);

  if (! proc)
    {
      picman_message (plug_in->manager->picman, NULL, PICMAN_MESSAGE_ERROR,
                    "Plug-in \"%s\"\n(%s)\n"
                    "attempted to register the menu item \"%s\" "
                    "for the procedure \"%s\".\n"
                    "It has however not installed that procedure.  This "
                    "is not allowed.",
                    picman_object_get_name (plug_in),
                    picman_filename_to_utf8 (plug_in->prog),
                    menu_path, proc_name);

      return FALSE;
    }

  switch (PICMAN_PROCEDURE (proc)->proc_type)
    {
    case PICMAN_INTERNAL:
      return FALSE;

    case PICMAN_PLUGIN:
    case PICMAN_EXTENSION:
      if (plug_in->call_mode != PICMAN_PLUG_IN_CALL_QUERY &&
          plug_in->call_mode != PICMAN_PLUG_IN_CALL_INIT)
        return FALSE;

    case PICMAN_TEMPORARY:
      break;
    }

  if (! proc->menu_label)
    {
      picman_message (plug_in->manager->picman, NULL, PICMAN_MESSAGE_ERROR,
                    "Plug-in \"%s\"\n(%s)\n"
                    "attempted to register the menu item \"%s\" "
                    "for procedure \"%s\".\n"
                    "The menu label given in picman_install_procedure() "
                    "already contained a path.  To make this work, "
                    "pass just the menu's label to "
                    "picman_install_procedure().",
                    picman_object_get_name (plug_in),
                    picman_filename_to_utf8 (plug_in->prog),
                    menu_path, proc_name);

      return FALSE;
    }

  if (! strlen (proc->menu_label))
    {
      picman_message (plug_in->manager->picman, NULL, PICMAN_MESSAGE_ERROR,
                    "Plug-in \"%s\"\n(%s)\n"
                    "attempted to register the procedure \"%s\" "
                    "in the menu \"%s\", but the procedure has no label.  "
                    "This is not allowed.",
                    picman_object_get_name (plug_in),
                    picman_filename_to_utf8 (plug_in->prog),
                    proc_name, menu_path);

      return FALSE;
    }
  if (! picman_plug_in_procedure_add_menu_path (proc, menu_path, &error))
    {
      picman_message_literal (plug_in->manager->picman, NULL, PICMAN_MESSAGE_ERROR,
			    error->message);
      g_clear_error (&error);

      return FALSE;
    }

  return TRUE;
}

void
picman_plug_in_set_error_handler (PicmanPlugIn          *plug_in,
                                PicmanPDBErrorHandler  handler)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (proc_frame)
    proc_frame->error_handler = handler;
}

PicmanPDBErrorHandler
picman_plug_in_get_error_handler (PicmanPlugIn *plug_in)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in),
                        PICMAN_PDB_ERROR_HANDLER_INTERNAL);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (proc_frame)
    return proc_frame->error_handler;

  return PICMAN_PDB_ERROR_HANDLER_INTERNAL;
}

void
picman_plug_in_add_temp_proc (PicmanPlugIn             *plug_in,
                            PicmanTemporaryProcedure *proc)
{
  PicmanPlugInProcedure *overridden;
  const gchar         *locale_domain;
  const gchar         *help_domain;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));
  g_return_if_fail (PICMAN_IS_TEMPORARY_PROCEDURE (proc));

  overridden = picman_plug_in_procedure_find (plug_in->temp_procedures,
                                            picman_object_get_name (proc));

  if (overridden)
    picman_plug_in_remove_temp_proc (plug_in,
                                   PICMAN_TEMPORARY_PROCEDURE (overridden));

  locale_domain = picman_plug_in_manager_get_locale_domain (plug_in->manager,
                                                          plug_in->prog,
                                                          NULL);
  help_domain = picman_plug_in_manager_get_help_domain (plug_in->manager,
                                                      plug_in->prog,
                                                      NULL);

  picman_plug_in_procedure_set_locale_domain (PICMAN_PLUG_IN_PROCEDURE (proc),
                                            locale_domain);
  picman_plug_in_procedure_set_help_domain (PICMAN_PLUG_IN_PROCEDURE (proc),
                                          help_domain);

  plug_in->temp_procedures = g_slist_prepend (plug_in->temp_procedures,
                                              g_object_ref (proc));
  picman_plug_in_manager_add_temp_proc (plug_in->manager, proc);
}

void
picman_plug_in_remove_temp_proc (PicmanPlugIn             *plug_in,
                               PicmanTemporaryProcedure *proc)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));
  g_return_if_fail (PICMAN_IS_TEMPORARY_PROCEDURE (proc));

  plug_in->temp_procedures = g_slist_remove (plug_in->temp_procedures, proc);

  picman_plug_in_manager_remove_temp_proc (plug_in->manager, proc);
  g_object_unref (proc);
}

void
picman_plug_in_enable_precision (PicmanPlugIn *plug_in)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  plug_in->precision = TRUE;
}

gboolean
picman_plug_in_precision_enabled (PicmanPlugIn *plug_in)
{
  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);

  return plug_in->precision;
}
