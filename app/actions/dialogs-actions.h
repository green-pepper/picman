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

#ifndef __DIALOGS_ACTIONS_H__
#define __DIALOGS_ACTIONS_H__


/*  this check is needed for the extern declaration below to be correct  */
#ifndef __PICMAN_ACTION_GROUP_H__
#error "widgets/picmanactiongroup.h must be included prior to dialogs-actions.h"
#endif

extern const PicmanStringActionEntry dialogs_dockable_actions[];
extern gint                        n_dialogs_dockable_actions;


void       dialogs_actions_setup          (PicmanActionGroup *group);
void       dialogs_actions_update         (PicmanActionGroup *group,
                                           gpointer         data);

gboolean   dialogs_actions_toolbox_exists (Picman            *picman);


#endif /* __DIALOGS_ACTIONS_H__ */
