/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanstringaction.h
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

#ifndef __PICMAN_STRING_ACTION_H__
#define __PICMAN_STRING_ACTION_H__


#include "picmanaction.h"


#define PICMAN_TYPE_STRING_ACTION            (picman_string_action_get_type ())
#define PICMAN_STRING_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_STRING_ACTION, PicmanStringAction))
#define PICMAN_STRING_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_STRING_ACTION, PicmanStringActionClass))
#define PICMAN_IS_STRING_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_STRING_ACTION))
#define PICMAN_IS_STRING_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), PICMAN_TYPE_STRING_ACTION))
#define PICMAN_STRING_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PICMAN_TYPE_STRING_ACTION, PicmanStringActionClass))


typedef struct _PicmanStringActionClass PicmanStringActionClass;

struct _PicmanStringAction
{
  PicmanAction  parent_instance;

  gchar      *value;
};

struct _PicmanStringActionClass
{
  PicmanActionClass parent_class;

  void (* selected) (PicmanStringAction *action,
                     const gchar      *value);
};


GType              picman_string_action_get_type (void) G_GNUC_CONST;

PicmanStringAction * picman_string_action_new      (const gchar      *name,
                                                const gchar      *label,
                                                const gchar      *tooltip,
                                                const gchar      *stock_id,
                                                const gchar      *value);
void               picman_string_action_selected (PicmanStringAction *action,
                                                const gchar      *value);


#endif  /* __PICMAN_STRING_ACTION_H__ */
