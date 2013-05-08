/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_TOOL_PRESET_H__
#define __PICMAN_TOOL_PRESET_H__


#include "picmandata.h"


#define PICMAN_TYPE_TOOL_PRESET            (picman_tool_preset_get_type ())
#define PICMAN_TOOL_PRESET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TOOL_PRESET, PicmanToolPreset))
#define PICMAN_TOOL_PRESET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TOOL_PRESET, PicmanToolPresetClass))
#define PICMAN_IS_TOOL_PRESET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TOOL_PRESET))
#define PICMAN_IS_TOOL_PRESET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TOOL_PRESET))
#define PICMAN_TOOL_PRESET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TOOL_PRESET, PicmanToolPresetClass))


typedef struct _PicmanToolPresetClass PicmanToolPresetClass;

struct _PicmanToolPreset
{
  PicmanData         parent_instance;

  Picman            *picman;
  PicmanToolOptions *tool_options;

  gboolean         use_fg_bg;
  gboolean         use_brush;
  gboolean         use_dynamics;
  gboolean         use_gradient;
  gboolean         use_pattern;
  gboolean         use_palette;
  gboolean         use_font;
};

struct _PicmanToolPresetClass
{
  PicmanDataClass  parent_class;
};


GType                 picman_tool_preset_get_type      (void) G_GNUC_CONST;

PicmanData            * picman_tool_preset_new           (PicmanContext    *context,
                                                      const gchar    *unused);

PicmanContextPropMask   picman_tool_preset_get_prop_mask (PicmanToolPreset *preset);


#endif  /*  __PICMAN_TOOL_PRESET_H__  */
