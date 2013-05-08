/* PICMAN - The GNU Image Manipulation Program
 *
 * picmancageoptions.h
 * Copyright (C) 2010 Michael Muré <batolettre@gmail.com>
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

#ifndef __PICMAN_CAGE_OPTIONS_H__
#define __PICMAN_CAGE_OPTIONS_H__


#include "core/picmantooloptions.h"


#define PICMAN_TYPE_CAGE_OPTIONS            (picman_cage_options_get_type ())
#define PICMAN_CAGE_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CAGE_OPTIONS, PicmanCageOptions))
#define PICMAN_CAGE_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CAGE_OPTIONS, PicmanCageOptionsClass))
#define PICMAN_IS_CAGE_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CAGE_OPTIONS))
#define PICMAN_IS_CAGE_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CAGE_OPTIONS))
#define PICMAN_CAGE_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CAGE_OPTIONS, PicmanCageOptionsClass))


typedef struct _PicmanCageOptions      PicmanCageOptions;
typedef struct _PicmanCageOptionsClass PicmanCageOptionsClass;

struct _PicmanCageOptions
{
  PicmanToolOptions  parent_instance;

  PicmanCageMode     cage_mode;
  gboolean         fill_plain_color;
};

struct _PicmanCageOptionsClass
{
  PicmanToolOptionsClass  parent_class;
};


GType       picman_cage_options_get_type (void) G_GNUC_CONST;

GtkWidget * picman_cage_options_gui      (PicmanToolOptions *tool_options);


#endif  /*  __PICMAN_CAGE_OPTIONS_H__  */
