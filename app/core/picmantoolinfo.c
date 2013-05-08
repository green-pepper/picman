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

#include "config.h"

#include <string.h>

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman.h"
#include "picmandatafactory.h"
#include "picmanfilteredcontainer.h"
#include "picmanpaintinfo.h"
#include "picmantoolinfo.h"
#include "picmantooloptions.h"
#include "picmantoolpreset.h"


enum
{
  PROP_0,
  PROP_VISIBLE
};


static void    picman_tool_info_dispose         (GObject       *object);
static void    picman_tool_info_finalize        (GObject       *object);
static void    picman_tool_info_get_property    (GObject       *object,
                                               guint          property_id,
                                               GValue        *value,
                                               GParamSpec    *pspec);
static void    picman_tool_info_set_property    (GObject       *object,
                                               guint          property_id,
                                               const GValue  *value,
                                               GParamSpec    *pspec);
static gchar * picman_tool_info_get_description (PicmanViewable  *viewable,
                                               gchar        **tooltip);


G_DEFINE_TYPE (PicmanToolInfo, picman_tool_info, PICMAN_TYPE_VIEWABLE)

#define parent_class picman_tool_info_parent_class


static void
picman_tool_info_class_init (PicmanToolInfoClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->dispose           = picman_tool_info_dispose;
  object_class->finalize          = picman_tool_info_finalize;
  object_class->get_property      = picman_tool_info_get_property;
  object_class->set_property      = picman_tool_info_set_property;

  viewable_class->get_description = picman_tool_info_get_description;

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_VISIBLE, "visible",
                                    NULL, TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_tool_info_init (PicmanToolInfo *tool_info)
{
  tool_info->picman              = NULL;

  tool_info->tool_type         = G_TYPE_NONE;
  tool_info->tool_options_type = G_TYPE_NONE;
  tool_info->context_props     = 0;

  tool_info->blurb             = NULL;
  tool_info->help              = NULL;

  tool_info->menu_label        = NULL;
  tool_info->menu_accel        = NULL;

  tool_info->help_domain       = NULL;
  tool_info->help_id           = NULL;

  tool_info->visible           = TRUE;
  tool_info->tool_options      = NULL;
  tool_info->paint_info        = NULL;
}

static void
picman_tool_info_dispose (GObject *object)
{
  PicmanToolInfo *tool_info = PICMAN_TOOL_INFO (object);

  if (tool_info->tool_options)
    {
      g_object_run_dispose (G_OBJECT (tool_info->tool_options));
      g_object_unref (tool_info->tool_options);
      tool_info->tool_options = NULL;
    }

  if (tool_info->presets)
    {
      g_object_unref (tool_info->presets);
      tool_info->presets = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_tool_info_finalize (GObject *object)
{
  PicmanToolInfo *tool_info = PICMAN_TOOL_INFO (object);

  if (tool_info->blurb)
    {
      g_free (tool_info->blurb);
      tool_info->blurb = NULL;
    }
  if (tool_info->help)
    {
      g_free (tool_info->help);
      tool_info->help = NULL;
    }

  if (tool_info->menu_label)
    {
      g_free (tool_info->menu_label);
      tool_info->menu_label = NULL;
    }
  if (tool_info->menu_accel)
    {
      g_free (tool_info->menu_accel);
      tool_info->menu_accel = NULL;
    }

  if (tool_info->help_domain)
    {
      g_free (tool_info->help_domain);
      tool_info->help_domain = NULL;
    }
  if (tool_info->help_id)
    {
      g_free (tool_info->help_id);
      tool_info->help_id = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_tool_info_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PicmanToolInfo *tool_info = PICMAN_TOOL_INFO (object);

  switch (property_id)
    {
    case PROP_VISIBLE:
      g_value_set_boolean (value, tool_info->visible);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_tool_info_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PicmanToolInfo *tool_info = PICMAN_TOOL_INFO (object);

  switch (property_id)
    {
    case PROP_VISIBLE:
      tool_info->visible = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gchar *
picman_tool_info_get_description (PicmanViewable  *viewable,
                                gchar        **tooltip)
{
  PicmanToolInfo *tool_info = PICMAN_TOOL_INFO (viewable);

  return g_strdup (tool_info->blurb);
}

static gboolean
picman_tool_info_filter_preset (const PicmanObject *object,
                              gpointer          user_data)
{
  PicmanToolPreset *preset    = PICMAN_TOOL_PRESET (object);
  PicmanToolInfo   *tool_info = user_data;

  return preset->tool_options->tool_info == tool_info;
}

PicmanToolInfo *
picman_tool_info_new (Picman                *picman,
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
                    const gchar         *stock_id)
{
  PicmanPaintInfo *paint_info;
  PicmanToolInfo  *tool_info;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);
  g_return_val_if_fail (blurb != NULL, NULL);
  g_return_val_if_fail (help != NULL, NULL);
  g_return_val_if_fail (menu_label != NULL, NULL);
  g_return_val_if_fail (help_id != NULL, NULL);
  g_return_val_if_fail (paint_core_name != NULL, NULL);
  g_return_val_if_fail (stock_id != NULL, NULL);

  paint_info = (PicmanPaintInfo *)
    picman_container_get_child_by_name (picman->paint_info_list, paint_core_name);

  g_return_val_if_fail (PICMAN_IS_PAINT_INFO (paint_info), NULL);

  tool_info = g_object_new (PICMAN_TYPE_TOOL_INFO,
                            "name",     identifier,
                            "stock-id", stock_id,
                            NULL);

  tool_info->picman              = picman;
  tool_info->tool_type         = tool_type;
  tool_info->tool_options_type = tool_options_type;
  tool_info->context_props     = context_props;

  tool_info->blurb             = g_strdup (blurb);
  tool_info->help              = g_strdup (help);

  tool_info->menu_label        = g_strdup (menu_label);
  tool_info->menu_accel        = g_strdup (menu_accel);

  tool_info->help_domain       = g_strdup (help_domain);
  tool_info->help_id           = g_strdup (help_id);

  tool_info->paint_info        = paint_info;

  if (tool_info->tool_options_type == paint_info->paint_options_type)
    {
      tool_info->tool_options = g_object_ref (paint_info->paint_options);
    }
  else
    {
      tool_info->tool_options = g_object_new (tool_info->tool_options_type,
                                              "picman", picman,
                                              "name", identifier,
                                              NULL);
    }

  g_object_set (tool_info->tool_options,
                "tool",      tool_info,
                "tool-info", tool_info, NULL);

  if (tool_info->tool_options_type != PICMAN_TYPE_TOOL_OPTIONS)
    {
      PicmanContainer *presets;

      presets = picman_data_factory_get_container (picman->tool_preset_factory);

      tool_info->presets =
        picman_filtered_container_new (presets,
                                     picman_tool_info_filter_preset,
                                     tool_info);
    }

  return tool_info;
}

void
picman_tool_info_set_standard (Picman         *picman,
                             PicmanToolInfo *tool_info)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (! tool_info || PICMAN_IS_TOOL_INFO (tool_info));

  if (tool_info != picman->standard_tool_info)
    {
      if (picman->standard_tool_info)
        g_object_unref (picman->standard_tool_info);

      picman->standard_tool_info = tool_info;

      if (picman->standard_tool_info)
        g_object_ref (picman->standard_tool_info);
    }
}

PicmanToolInfo *
picman_tool_info_get_standard (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return picman->standard_tool_info;
}

gchar *
picman_tool_info_build_options_filename (PicmanToolInfo *tool_info,
                                       const gchar  *suffix)
{
  const gchar *name;
  gchar       *filename;
  gchar       *basename;

  g_return_val_if_fail (PICMAN_IS_TOOL_INFO (tool_info), NULL);

  name = picman_object_get_name (tool_info);

  if (suffix)
    basename = g_strconcat (name, suffix, NULL);
  else
    basename = g_strdup (name);

  filename = g_build_filename (picman_directory (),
                               "tool-options",
                               basename,
                               NULL);
  g_free (basename);

  return filename;
}
