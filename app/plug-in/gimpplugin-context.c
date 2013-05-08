/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanplugin-context.c
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

#include "plug-in-types.h"

#include "core/picman.h"

#include "pdb/picmanpdbcontext.h"

#include "picmanplugin.h"
#include "picmanplugin-context.h"
#include "picmanpluginmanager.h"


gboolean
picman_plug_in_context_push (PicmanPlugIn *plug_in)
{
  PicmanPlugInProcFrame *proc_frame;
  PicmanContext         *parent;
  PicmanContext         *context;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (proc_frame->context_stack)
    parent = proc_frame->context_stack->data;
  else
    parent = proc_frame->main_context;

  context = picman_pdb_context_new (plug_in->manager->picman, parent, FALSE);

  proc_frame->context_stack = g_list_prepend (proc_frame->context_stack,
                                              context);

  return TRUE;
}

gboolean
picman_plug_in_context_pop (PicmanPlugIn *plug_in)
{
  PicmanPlugInProcFrame *proc_frame;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);

  if (proc_frame->context_stack)
    {
      PicmanContext *context = proc_frame->context_stack->data;

      proc_frame->context_stack = g_list_remove (proc_frame->context_stack,
                                                 context);
      g_object_unref (context);

      return TRUE;
    }

  return FALSE;
}
