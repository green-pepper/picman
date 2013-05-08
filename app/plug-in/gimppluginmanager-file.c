/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginmanager-file.c
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

#include "file/file-procedure.h"

#include "picmanplugin.h"
#include "picmanplugindef.h"
#include "picmanpluginmanager.h"
#include "picmanpluginmanager-file.h"
#include "picmanpluginprocedure.h"


/*  public functions  */

gboolean
picman_plug_in_manager_register_load_handler (PicmanPlugInManager *manager,
                                            const gchar       *name,
                                            const gchar       *extensions,
                                            const gchar       *prefixes,
                                            const gchar       *magics)
{
  PicmanPlugInProcedure *file_proc;
  PicmanProcedure       *procedure;
  GSList              *list;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), FALSE);
  g_return_val_if_fail (name != NULL, FALSE);

  if (manager->current_plug_in && manager->current_plug_in->plug_in_def)
    list = manager->current_plug_in->plug_in_def->procedures;
  else
    list = manager->plug_in_procedures;

  file_proc = picman_plug_in_procedure_find (list, name);

  if (! file_proc)
    {
      picman_message (manager->picman, NULL, PICMAN_MESSAGE_ERROR,
                    "attempt to register nonexistent load handler \"%s\"",
                    name);
      return FALSE;
    }

  procedure = PICMAN_PROCEDURE (file_proc);

  if ((procedure->num_args   < 3)                        ||
      (procedure->num_values < 1)                        ||
      ! PICMAN_IS_PARAM_SPEC_INT32    (procedure->args[0]) ||
      ! G_IS_PARAM_SPEC_STRING      (procedure->args[1]) ||
      ! G_IS_PARAM_SPEC_STRING      (procedure->args[2]) ||
      ! PICMAN_IS_PARAM_SPEC_IMAGE_ID (procedure->values[0]))
    {
      picman_message (manager->picman, NULL, PICMAN_MESSAGE_ERROR,
                    "load handler \"%s\" does not take the standard "
                    "load handler args", name);
      return FALSE;
    }

  picman_plug_in_procedure_set_file_proc (file_proc,
                                        extensions, prefixes, magics);

  if (! g_slist_find (manager->load_procs, file_proc))
    manager->load_procs = g_slist_prepend (manager->load_procs, file_proc);

  return TRUE;
}

gboolean
picman_plug_in_manager_register_save_handler (PicmanPlugInManager *manager,
                                            const gchar       *name,
                                            const gchar       *extensions,
                                            const gchar       *prefixes)
{
  PicmanPlugInProcedure *file_proc;
  PicmanProcedure       *procedure;
  GSList              *list;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), FALSE);
  g_return_val_if_fail (name != NULL, FALSE);

  if (manager->current_plug_in && manager->current_plug_in->plug_in_def)
    list = manager->current_plug_in->plug_in_def->procedures;
  else
    list = manager->plug_in_procedures;

  file_proc = picman_plug_in_procedure_find (list, name);

  if (! file_proc)
    {
      picman_message (manager->picman, NULL, PICMAN_MESSAGE_ERROR,
                    "attempt to register nonexistent save handler \"%s\"",
                    name);
      return FALSE;
    }

  procedure = PICMAN_PROCEDURE (file_proc);

  if ((procedure->num_args < 5)                             ||
      ! PICMAN_IS_PARAM_SPEC_INT32       (procedure->args[0]) ||
      ! PICMAN_IS_PARAM_SPEC_IMAGE_ID    (procedure->args[1]) ||
      ! PICMAN_IS_PARAM_SPEC_DRAWABLE_ID (procedure->args[2]) ||
      ! G_IS_PARAM_SPEC_STRING         (procedure->args[3]) ||
      ! G_IS_PARAM_SPEC_STRING         (procedure->args[4]))
    {
      picman_message (manager->picman, NULL, PICMAN_MESSAGE_ERROR,
                    "save handler \"%s\" does not take the standard "
                    "save handler args", name);
      return FALSE;
    }

  picman_plug_in_procedure_set_file_proc (file_proc,
                                        extensions, prefixes, NULL);

  if (file_procedure_in_group (file_proc, FILE_PROCEDURE_GROUP_SAVE))
    {
      if (! g_slist_find (manager->save_procs, file_proc))
        manager->save_procs = g_slist_prepend (manager->save_procs, file_proc);
    }

  if (file_procedure_in_group (file_proc, FILE_PROCEDURE_GROUP_EXPORT))
    {
      if (! g_slist_find (manager->export_procs, file_proc))
        manager->export_procs = g_slist_prepend (manager->export_procs, file_proc);
    }

  return TRUE;
}

gboolean
picman_plug_in_manager_register_mime_type (PicmanPlugInManager *manager,
                                         const gchar       *name,
                                         const gchar       *mime_type)
{
  PicmanPlugInProcedure *file_proc;
  GSList              *list;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), FALSE);
  g_return_val_if_fail (name != NULL, FALSE);
  g_return_val_if_fail (mime_type != NULL, FALSE);

  if (manager->current_plug_in && manager->current_plug_in->plug_in_def)
    list = manager->current_plug_in->plug_in_def->procedures;
  else
    list = manager->plug_in_procedures;

  file_proc = picman_plug_in_procedure_find (list, name);

  if (! file_proc)
    return FALSE;

  picman_plug_in_procedure_set_mime_type (file_proc, mime_type);

  return TRUE;
}

gboolean
picman_plug_in_manager_register_handles_uri (PicmanPlugInManager *manager,
                                           const gchar       *name)
{
  PicmanPlugInProcedure *file_proc;
  GSList              *list;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), FALSE);
  g_return_val_if_fail (name != NULL, FALSE);

  if (manager->current_plug_in && manager->current_plug_in->plug_in_def)
    list = manager->current_plug_in->plug_in_def->procedures;
  else
    list = manager->plug_in_procedures;

  file_proc = picman_plug_in_procedure_find (list, name);

  if (! file_proc)
    return FALSE;

  picman_plug_in_procedure_set_handles_uri (file_proc);

  return TRUE;
}

gboolean
picman_plug_in_manager_register_thumb_loader (PicmanPlugInManager *manager,
                                            const gchar       *load_proc,
                                            const gchar       *thumb_proc)
{
  PicmanPlugInProcedure *file_proc;
  GSList              *list;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), FALSE);
  g_return_val_if_fail (load_proc, FALSE);
  g_return_val_if_fail (thumb_proc, FALSE);

  if (manager->current_plug_in && manager->current_plug_in->plug_in_def)
    list = manager->current_plug_in->plug_in_def->procedures;
  else
    list = manager->plug_in_procedures;

  file_proc = picman_plug_in_procedure_find (list, load_proc);

  if (! file_proc)
    return FALSE;

  picman_plug_in_procedure_set_thumb_loader (file_proc, thumb_proc);

  return TRUE;
}

gboolean
picman_plug_in_manager_uri_has_exporter (PicmanPlugInManager *manager,
                                       const gchar       *uri)
{
  return file_procedure_find (manager->export_procs, uri, NULL) != NULL;
}
