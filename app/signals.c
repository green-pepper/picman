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

#include <signal.h>

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "core/core-types.h"

#include "errors.h"
#include "signals.h"


#ifndef G_OS_WIN32
static void  picman_sigfatal_handler (gint sig_num) G_GNUC_NORETURN;
#endif


void
picman_init_signal_handlers (PicmanStackTraceMode stack_trace_mode)
{
#ifndef G_OS_WIN32
  /* No use catching these on Win32, the user won't get any stack
   * trace from glib anyhow. It's better to let Windows inform about
   * the program error, and offer debugging (if the user has installed
   * MSVC or some other compiler that knows how to install itself as a
   * handler for program errors).
   */

  /* Handle fatal signals */

  /* these are handled by picman_terminate() */
  picman_signal_private (SIGHUP,  picman_sigfatal_handler, 0);
  picman_signal_private (SIGINT,  picman_sigfatal_handler, 0);
  picman_signal_private (SIGQUIT, picman_sigfatal_handler, 0);
  picman_signal_private (SIGABRT, picman_sigfatal_handler, 0);
  picman_signal_private (SIGTERM, picman_sigfatal_handler, 0);

  if (stack_trace_mode != PICMAN_STACK_TRACE_NEVER)
    {
      /* these are handled by picman_fatal_error() */
      picman_signal_private (SIGBUS,  picman_sigfatal_handler, 0);
      picman_signal_private (SIGSEGV, picman_sigfatal_handler, 0);
      picman_signal_private (SIGFPE,  picman_sigfatal_handler, 0);
    }

  /* Ignore SIGPIPE because plug_in.c handles broken pipes */
  picman_signal_private (SIGPIPE, SIG_IGN, 0);

  /* Restart syscalls on SIGCHLD */
  picman_signal_private (SIGCHLD, SIG_DFL, SA_RESTART);

#endif /* ! G_OS_WIN32 */
}


#ifndef G_OS_WIN32

/* picman core signal handler for fatal signals */

static void
picman_sigfatal_handler (gint sig_num)
{
  switch (sig_num)
    {
    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGABRT:
    case SIGTERM:
      picman_terminate (g_strsignal (sig_num));
      break;

    case SIGBUS:
    case SIGSEGV:
    case SIGFPE:
    default:
      picman_fatal_error (g_strsignal (sig_num));
      break;
    }
}

#endif /* ! G_OS_WIN32 */
