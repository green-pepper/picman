/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanpluginmanager.h
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

#ifndef __PICMAN_PLUG_IN_MANAGER_H__
#define __PICMAN_PLUG_IN_MANAGER_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_PLUG_IN_MANAGER            (picman_plug_in_manager_get_type ())
#define PICMAN_PLUG_IN_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PLUG_IN_MANAGER, PicmanPlugInManager))
#define PICMAN_PLUG_IN_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PLUG_IN_MANAGER, PicmanPlugInManagerClass))
#define PICMAN_IS_PLUG_IN_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PLUG_IN_MANAGER))
#define PICMAN_IS_PLUG_IN_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PLUG_IN_MANAGER))


typedef struct _PicmanPlugInManagerClass PicmanPlugInManagerClass;

struct _PicmanPlugInManager
{
  PicmanObject         parent_instance;

  Picman              *picman;

  GSList            *plug_in_defs;
  gboolean           write_pluginrc;

  GSList            *plug_in_procedures;

  GSList            *load_procs;
  GSList            *save_procs;
  GSList            *export_procs;

  GSList            *menu_branches;
  GSList            *locale_domains;
  GSList            *help_domains;

  PicmanPlugIn        *current_plug_in;
  GSList            *open_plug_ins;
  GSList            *plug_in_stack;
  GSList            *history;

  PicmanPlugInShm     *shm;
  PicmanInterpreterDB *interpreter_db;
  PicmanEnvironTable  *environ_table;
  PicmanPlugInDebug   *debug;
  GList             *data_list;
};

struct _PicmanPlugInManagerClass
{
  PicmanObjectClass  parent_class;

  void (* plug_in_opened)    (PicmanPlugInManager *manager,
                              PicmanPlugIn        *plug_in);
  void (* plug_in_closed)    (PicmanPlugInManager *manager,
                              PicmanPlugIn        *plug_in);

  void (* menu_branch_added) (PicmanPlugInManager *manager,
                              const gchar       *prog_name,
                              const gchar       *menu_path,
                              const gchar       *menu_label);
  void (* history_changed)   (PicmanPlugInManager *manager);
};


GType               picman_plug_in_manager_get_type (void) G_GNUC_CONST;

PicmanPlugInManager * picman_plug_in_manager_new      (Picman                *picman);

void    picman_plug_in_manager_initialize           (PicmanPlugInManager   *manager,
                                                   PicmanInitStatusFunc   status_callback);
void    picman_plug_in_manager_exit                 (PicmanPlugInManager   *manager);

/* Register a plug-in. This function is public for file load-save
 * handlers, which are organized around the plug-in data structure.
 * This could all be done a little better, but oh well.  -josh
 */
void    picman_plug_in_manager_add_procedure        (PicmanPlugInManager   *manager,
                                                   PicmanPlugInProcedure *procedure);

void    picman_plug_in_manager_add_temp_proc        (PicmanPlugInManager      *manager,
                                                   PicmanTemporaryProcedure *procedure);
void    picman_plug_in_manager_remove_temp_proc     (PicmanPlugInManager      *manager,
                                                   PicmanTemporaryProcedure *procedure);

void    picman_plug_in_manager_add_open_plug_in     (PicmanPlugInManager   *manager,
                                                   PicmanPlugIn          *plug_in);
void    picman_plug_in_manager_remove_open_plug_in  (PicmanPlugInManager   *manager,
                                                   PicmanPlugIn          *plug_in);

void    picman_plug_in_manager_plug_in_push         (PicmanPlugInManager   *manager,
                                                   PicmanPlugIn          *plug_in);
void    picman_plug_in_manager_plug_in_pop          (PicmanPlugInManager   *manager);

void    picman_plug_in_manager_history_changed      (PicmanPlugInManager   *manager);


#endif  /* __PICMAN_PLUG_IN_MANAGER_H__ */
