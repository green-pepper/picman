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

#ifndef __PICMAN_TOOL_INFO_H__
#define __PICMAN_TOOL_INFO_H__


#include "picmandata.h"


#define PICMAN_TYPE_TOOL_INFO            (picman_tool_info_get_type ())
#define PICMAN_TOOL_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TOOL_INFO, PicmanToolInfo))
#define PICMAN_TOOL_INFO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TOOL_INFO, PicmanToolInfoClass))
#define PICMAN_IS_TOOL_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TOOL_INFO))
#define PICMAN_IS_TOOL_INFO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TOOL_INFO))
#define PICMAN_TOOL_INFO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TOOL_INFO, PicmanToolInfoClass))


typedef struct _PicmanToolInfoClass PicmanToolInfoClass;

struct _PicmanToolInfo
{
  PicmanViewable         parent_instance;

  Picman                *picman;

  GType                tool_type;
  GType                tool_options_type;
  PicmanContextPropMask  context_props;

  gchar               *blurb;
  gchar               *help;

  gchar               *menu_label;
  gchar               *menu_accel;

  gchar               *help_domain;
  gchar               *help_id;

  gboolean             visible;
  PicmanToolOptions     *tool_options;
  PicmanPaintInfo       *paint_info;

  PicmanContainer       *presets;
};

struct _PicmanToolInfoClass
{
  PicmanViewableClass    parent_class;
};


GType          picman_tool_info_get_type     (void) G_GNUC_CONST;

PicmanToolInfo * picman_tool_info_new          (Picman                *picman,
                                            GType                tool_type,
                                            GType                tool_options_type,
                                            PicmanContextPropMask  context_props,
                                            const gchar         *identifier,
                                            const gchar         *blurb,
                                            const gchar         *help,
                                            const gchar         *menu_label,
                                            const gchar         *menu_accel,
                                            const gchar         *help_domain,
                                            const gchar         *help_id,
                                            const gchar         *paint_core_name,
                                            const gchar         *stock_id);

void           picman_tool_info_set_standard (Picman                *picman,
                                            PicmanToolInfo        *tool_info);
PicmanToolInfo * picman_tool_info_get_standard (Picman                *picman);

gchar *
     picman_tool_info_build_options_filename (PicmanToolInfo        *tool_info,
                                            const gchar         *suffix);


#endif  /*  __PICMAN_TOOL_INFO_H__  */
