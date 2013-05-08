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

#include <stdio.h>
#include <errno.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "libpicman/picman.h"

#include "scheme-wrapper.h"
#include "script-fu-text-console.h"

#include "script-fu-intl.h"

void
script_fu_text_console_run (const gchar      *name,
                            gint              nparams,
                            const PicmanParam  *params,
                            gint             *nreturn_vals,
                            PicmanParam       **return_vals)
{
  static PicmanParam  values[1];

  /*  Enable Script-Fu output  */
  ts_register_output_func (ts_stdout_output_func, NULL);

  ts_print_welcome ();

  picman_plugin_set_pdb_error_handler (PICMAN_PDB_ERROR_HANDLER_PLUGIN);

  /*  Run the interface  */
  ts_interpret_stdin ();

  picman_plugin_set_pdb_error_handler (PICMAN_PDB_ERROR_HANDLER_INTERNAL);

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_SUCCESS;

  *nreturn_vals = 1;
  *return_vals  = values;
}
