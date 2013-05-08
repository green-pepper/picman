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

#define _GNU_SOURCE  /* need the POSIX signal API */

#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "core/core-types.h"

#include "core/picman.h"

#include "errors.h"

#ifdef G_OS_WIN32
#include <windows.h>
#endif


/*  private variables  */

static Picman                *the_errors_picman   = NULL;
static gboolean             use_debug_handler = FALSE;
static PicmanStackTraceMode   stack_trace_mode  = PICMAN_STACK_TRACE_QUERY;
static gchar               *full_prog_name    = NULL;


/*  local function prototypes  */

static G_GNUC_NORETURN void  picman_eek (const gchar *reason,
                                       const gchar *message,
                                       gboolean     use_handler);

static void   picman_message_log_func (const gchar        *log_domain,
                                     GLogLevelFlags      flags,
                                     const gchar        *message,
                                     gpointer            data);
static void   picman_error_log_func   (const gchar        *domain,
                                     GLogLevelFlags      flags,
                                     const gchar        *message,
                                     gpointer            data) G_GNUC_NORETURN;



/*  public functions  */

void
errors_init (Picman               *picman,
             const gchar        *_full_prog_name,
             gboolean            _use_debug_handler,
             PicmanStackTraceMode  _stack_trace_mode)
{
  const gchar * const log_domains[] =
  {
    "Picman",
    "Picman-Actions",
    "Picman-Base",
    "Picman-Composite",
    "Picman-Config",
    "Picman-Core",
    "Picman-Dialogs",
    "Picman-Display",
    "Picman-File",
    "Picman-GUI",
    "Picman-Menus",
    "Picman-PDB",
    "Picman-Paint",
    "Picman-Paint-Funcs",
    "Picman-Plug-In",
    "Picman-Text",
    "Picman-Tools",
    "Picman-Vectors",
    "Picman-Widgets",
    "Picman-XCF"
  };
  gint i;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (_full_prog_name != NULL);
  g_return_if_fail (full_prog_name == NULL);

#ifdef PICMAN_UNSTABLE
  g_printerr ("This is a development version of PICMAN.  "
              "Debug messages may appear here.\n\n");
#endif /* PICMAN_UNSTABLE */

  the_errors_picman   = picman;
  use_debug_handler = _use_debug_handler ? TRUE : FALSE;
  stack_trace_mode  = _stack_trace_mode;
  full_prog_name    = g_strdup (_full_prog_name);

  for (i = 0; i < G_N_ELEMENTS (log_domains); i++)
    g_log_set_handler (log_domains[i],
                       G_LOG_LEVEL_MESSAGE,
                       picman_message_log_func, picman);

  g_log_set_handler (NULL,
                     G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL,
                     picman_error_log_func, picman);
}

void
errors_exit (void)
{
  the_errors_picman = NULL;
}

void
picman_fatal_error (const gchar *message)
{
  picman_eek ("fatal error", message, TRUE);
}

void
picman_terminate (const gchar *message)
{
  picman_eek ("terminated", message, use_debug_handler);
}


/*  private functions  */

static void
picman_message_log_func (const gchar    *log_domain,
                       GLogLevelFlags  flags,
                       const gchar    *message,
                       gpointer        data)
{
  Picman *picman = data;

  if (picman)
    {
      picman_show_message (picman, NULL, PICMAN_MESSAGE_WARNING, NULL, message);
    }
  else
    {
      g_printerr ("%s: %s\n\n",
                  picman_filename_to_utf8 (full_prog_name), message);
    }
}

static void
picman_error_log_func (const gchar    *domain,
                     GLogLevelFlags  flags,
                     const gchar    *message,
                     gpointer        data)
{
  picman_fatal_error (message);
}

static void
picman_eek (const gchar *reason,
          const gchar *message,
          gboolean     use_handler)
{
#ifndef G_OS_WIN32
  g_printerr ("%s: %s: %s\n", picman_filename_to_utf8 (full_prog_name),
              reason, message);

  if (use_handler)
    {
      switch (stack_trace_mode)
        {
        case PICMAN_STACK_TRACE_NEVER:
          break;

        case PICMAN_STACK_TRACE_QUERY:
          {
            sigset_t sigset;

            sigemptyset (&sigset);
            sigprocmask (SIG_SETMASK, &sigset, NULL);

            if (the_errors_picman)
              picman_gui_ungrab (the_errors_picman);

            g_on_error_query (full_prog_name);
          }
          break;

        case PICMAN_STACK_TRACE_ALWAYS:
          {
            sigset_t sigset;

            sigemptyset (&sigset);
            sigprocmask (SIG_SETMASK, &sigset, NULL);

            g_on_error_stack_trace (full_prog_name);
          }
          break;

        default:
          break;
        }
    }
#else

  /* g_on_error_* don't do anything reasonable on Win32. */

  MessageBox (NULL, g_strdup_printf ("%s: %s", reason, message),
              full_prog_name, MB_OK|MB_ICONERROR);

#endif /* ! G_OS_WIN32 */

  exit (EXIT_FAILURE);
}
