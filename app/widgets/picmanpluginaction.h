/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginaction.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_PLUG_IN_ACTION_H__
#define __PICMAN_PLUG_IN_ACTION_H__


#include "picmanaction.h"


#define PICMAN_TYPE_PLUG_IN_ACTION            (picman_plug_in_action_get_type ())
#define PICMAN_PLUG_IN_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PLUG_IN_ACTION, PicmanPlugInAction))
#define PICMAN_PLUG_IN_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PLUG_IN_ACTION, PicmanPlugInActionClass))
#define PICMAN_IS_PLUG_IN_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PLUG_IN_ACTION))
#define PICMAN_IS_PLUG_IN_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), PICMAN_TYPE_PLUG_IN_ACTION))
#define PICMAN_PLUG_IN_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PICMAN_TYPE_PLUG_IN_ACTION, PicmanPlugInActionClass))


typedef struct _PicmanPlugInActionClass PicmanPlugInActionClass;

struct _PicmanPlugInAction
{
  PicmanAction           parent_instance;

  PicmanPlugInProcedure *procedure;
};

struct _PicmanPlugInActionClass
{
  PicmanActionClass parent_class;

  void (* selected) (PicmanPlugInAction    *action,
                     PicmanPlugInProcedure *proc);
};


GType              picman_plug_in_action_get_type (void) G_GNUC_CONST;

PicmanPlugInAction * picman_plug_in_action_new      (const gchar         *name,
                                                 const gchar         *label,
                                                 const gchar         *tooltip,
                                                 const gchar         *stock_id,
                                                 PicmanPlugInProcedure *procedure);
void               picman_plug_in_action_selected (PicmanPlugInAction    *action,
                                                 PicmanPlugInProcedure *procedure);


#endif  /* __PICMAN_PLUG_IN_ACTION_H__ */
