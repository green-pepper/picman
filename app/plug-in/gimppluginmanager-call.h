/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginmanager-call.h
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

#ifndef __PICMAN_PLUG_IN_MANAGER_CALL_H__
#define __PICMAN_PLUG_IN_MANAGER_CALL_H__

#ifndef __YES_I_NEED_PICMAN_PLUG_IN_MANAGER_CALL__
#error Do not use picman_plug_in_manager_call_run*(), use picman_procedure_execute*() instead.
#endif


/*  Call the plug-in's query() function
 */
void             picman_plug_in_manager_call_query    (PicmanPlugInManager      *manager,
                                                     PicmanContext            *context,
                                                     PicmanPlugInDef          *plug_in_def);

/*  Call the plug-in's init() function
 */
void             picman_plug_in_manager_call_init     (PicmanPlugInManager      *manager,
                                                     PicmanContext            *context,
                                                     PicmanPlugInDef          *plug_in_def);

/*  Run a plug-in as if it were a procedure database procedure
 */
PicmanValueArray * picman_plug_in_manager_call_run      (PicmanPlugInManager      *manager,
                                                     PicmanContext            *context,
                                                     PicmanProgress           *progress,
                                                     PicmanPlugInProcedure    *procedure,
                                                     PicmanValueArray         *args,
                                                     gboolean                synchronous,
                                                     PicmanObject             *display);

/*  Run a temp plug-in proc as if it were a procedure database procedure
 */
PicmanValueArray * picman_plug_in_manager_call_run_temp (PicmanPlugInManager      *manager,
                                                     PicmanContext            *context,
                                                     PicmanProgress           *progress,
                                                     PicmanTemporaryProcedure *procedure,
                                                     PicmanValueArray         *args);


#endif /* __PICMAN_PLUG_IN_MANAGER_CALL_H__ */
