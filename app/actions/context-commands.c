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
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmanbrushgenerated.h"
#include "core/picmancontext.h"
#include "core/picmandatafactory.h"
#include "core/picmanlist.h"
#include "core/picmantoolinfo.h"

#include "paint/picmanpaintoptions.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmanpaletteeditor.h"
#include "widgets/picmancolormapeditor.h"

#include "actions.h"
#include "context-commands.h"

#include "picman-intl.h"


static const PicmanLayerModeEffects paint_modes[] =
{
  PICMAN_NORMAL_MODE,
  PICMAN_DISSOLVE_MODE,
  PICMAN_BEHIND_MODE,
  PICMAN_COLOR_ERASE_MODE,
  PICMAN_MULTIPLY_MODE,
  PICMAN_DIVIDE_MODE,
  PICMAN_SCREEN_MODE,
  PICMAN_OVERLAY_MODE,
  PICMAN_DODGE_MODE,
  PICMAN_BURN_MODE,
  PICMAN_HARDLIGHT_MODE,
  PICMAN_SOFTLIGHT_MODE,
  PICMAN_GRAIN_EXTRACT_MODE,
  PICMAN_GRAIN_MERGE_MODE,
  PICMAN_DIFFERENCE_MODE,
  PICMAN_ADDITION_MODE,
  PICMAN_SUBTRACT_MODE,
  PICMAN_DARKEN_ONLY_MODE,
  PICMAN_LIGHTEN_ONLY_MODE,
  PICMAN_HUE_MODE,
  PICMAN_SATURATION_MODE,
  PICMAN_COLOR_MODE,
  PICMAN_VALUE_MODE
};


/*  local function prototypes  */

static void     context_select_object    (PicmanActionSelectType  select_type,
                                          PicmanContext          *context,
                                          PicmanContainer        *container);
static gint     context_paint_mode_index (PicmanLayerModeEffects  paint_mode);

static void     context_select_color     (PicmanActionSelectType  select_type,
                                          PicmanRGB              *color,
                                          gboolean              use_colormap,
                                          gboolean              use_palette);

static gint     context_get_color_index  (gboolean              use_colormap,
                                          gboolean              use_palette,
                                          const PicmanRGB        *color);
static gint     context_max_color_index  (gboolean              use_colormap,
                                          gboolean              use_palette);
static gboolean context_set_color_index  (gint                  index,
                                          gboolean              use_colormap,
                                          gboolean              use_palette,
                                          PicmanRGB              *color);

static PicmanPaletteEditor  * context_get_palette_editor  (void);
static PicmanColormapEditor * context_get_colormap_editor (void);


/*  public functions  */

void
context_colors_default_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanContext *context;
  return_if_no_context (context, data);

  picman_context_set_default_colors (context);
}

void
context_colors_swap_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanContext *context;
  return_if_no_context (context, data);

  picman_context_swap_colors (context);
}

#define SELECT_COLOR_CMD_CALLBACK(name, fgbg, use_colormap, use_palette) \
void \
context_##name##_##fgbg##ground_cmd_callback (GtkAction *action, \
                                              gint       value, \
                                              gpointer   data) \
{ \
  PicmanContext *context; \
  PicmanRGB      color; \
  return_if_no_context (context, data); \
\
  picman_context_get_##fgbg##ground (context, &color); \
  context_select_color ((PicmanActionSelectType) value, &color, \
                        use_colormap, use_palette); \
  picman_context_set_##fgbg##ground (context, &color); \
}

SELECT_COLOR_CMD_CALLBACK (palette,  fore, FALSE, TRUE)
SELECT_COLOR_CMD_CALLBACK (palette,  back, FALSE, TRUE)
SELECT_COLOR_CMD_CALLBACK (colormap, fore, TRUE,  FALSE)
SELECT_COLOR_CMD_CALLBACK (colormap, back, TRUE,  FALSE)
SELECT_COLOR_CMD_CALLBACK (swatch,   fore, TRUE,  TRUE)
SELECT_COLOR_CMD_CALLBACK (swatch,   back, TRUE,  TRUE)

void
context_foreground_red_cmd_callback (GtkAction *action,
                                     gint       value,
                                     gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  return_if_no_context (context, data);

  picman_context_get_foreground (context, &color);
  color.r = action_select_value ((PicmanActionSelectType) value,
                                 color.r,
                                 0.0, 1.0, 1.0,
                                 1.0 / 255.0, 0.01, 0.1, 0.0, FALSE);
  picman_context_set_foreground (context, &color);
}

void
context_foreground_green_cmd_callback (GtkAction *action,
                                       gint       value,
                                       gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  return_if_no_context (context, data);

  picman_context_get_foreground (context, &color);
  color.g = action_select_value ((PicmanActionSelectType) value,
                                 color.g,
                                 0.0, 1.0, 1.0,
                                 1.0 / 255.0, 0.01, 0.1, 0.0, FALSE);
  picman_context_set_foreground (context, &color);
}

void
context_foreground_blue_cmd_callback (GtkAction *action,
                                      gint       value,
                                      gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  return_if_no_context (context, data);

  picman_context_get_foreground (context, &color);
  color.b = action_select_value ((PicmanActionSelectType) value,
                                 color.b,
                                 0.0, 1.0, 1.0,
                                 1.0 / 255.0, 0.01, 0.1, 0.0, FALSE);
  picman_context_set_foreground (context, &color);
}

void
context_background_red_cmd_callback (GtkAction *action,
                                     gint       value,
                                     gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  return_if_no_context (context, data);

  picman_context_get_background (context, &color);
  color.r = action_select_value ((PicmanActionSelectType) value,
                                 color.r,
                                 0.0, 1.0, 1.0,
                                 1.0 / 255.0, 0.01, 0.1, 0.0, FALSE);
  picman_context_set_background (context, &color);
}

void
context_background_green_cmd_callback (GtkAction *action,
                                       gint       value,
                                       gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  return_if_no_context (context, data);

  picman_context_get_background (context, &color);
  color.g = action_select_value ((PicmanActionSelectType) value,
                                 color.g,
                                 0.0, 1.0, 1.0,
                                 1.0 / 255.0, 0.01, 0.1, 0.0, FALSE);
  picman_context_set_background (context, &color);
}

void
context_background_blue_cmd_callback (GtkAction *action,
                                      gint       value,
                                      gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  return_if_no_context (context, data);

  picman_context_get_background (context, &color);
  color.b = action_select_value ((PicmanActionSelectType) value,
                                 color.b,
                                 0.0, 1.0, 1.0,
                                 1.0 / 255.0, 0.01, 0.1, 0.0, FALSE);
  picman_context_set_background (context, &color);
}

void
context_foreground_hue_cmd_callback (GtkAction *action,
                                     gint       value,
                                     gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  PicmanHSV      hsv;
  return_if_no_context (context, data);

  picman_context_get_foreground (context, &color);
  picman_rgb_to_hsv (&color, &hsv);
  hsv.h = action_select_value ((PicmanActionSelectType) value,
                               hsv.h,
                               0.0, 1.0, 1.0,
                               1.0 / 360.0, 0.01, 0.1, 0.0, FALSE);
  picman_hsv_to_rgb (&hsv, &color);
  picman_context_set_foreground (context, &color);
}

void
context_foreground_saturation_cmd_callback (GtkAction *action,
                                            gint       value,
                                            gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  PicmanHSV      hsv;
  return_if_no_context (context, data);

  picman_context_get_foreground (context, &color);
  picman_rgb_to_hsv (&color, &hsv);
  hsv.s = action_select_value ((PicmanActionSelectType) value,
                               hsv.s,
                               0.0, 1.0, 1.0,
                               0.01, 0.01, 0.1, 0.0, FALSE);
  picman_hsv_to_rgb (&hsv, &color);
  picman_context_set_foreground (context, &color);
}

void
context_foreground_value_cmd_callback (GtkAction *action,
                                       gint       value,
                                       gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  PicmanHSV      hsv;
  return_if_no_context (context, data);

  picman_context_get_foreground (context, &color);
  picman_rgb_to_hsv (&color, &hsv);
  hsv.v = action_select_value ((PicmanActionSelectType) value,
                               hsv.v,
                               0.0, 1.0, 1.0,
                               0.01, 0.01, 0.1, 0.0, FALSE);
  picman_hsv_to_rgb (&hsv, &color);
  picman_context_set_foreground (context, &color);
}

void
context_background_hue_cmd_callback (GtkAction *action,
                                     gint       value,
                                     gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  PicmanHSV      hsv;
  return_if_no_context (context, data);

  picman_context_get_background (context, &color);
  picman_rgb_to_hsv (&color, &hsv);
  hsv.h = action_select_value ((PicmanActionSelectType) value,
                               hsv.h,
                               0.0, 1.0, 1.0,
                               1.0 / 360.0, 0.01, 0.1, 0.0, FALSE);
  picman_hsv_to_rgb (&hsv, &color);
  picman_context_set_background (context, &color);
}

void
context_background_saturation_cmd_callback (GtkAction *action,
                                            gint       value,
                                            gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  PicmanHSV      hsv;
  return_if_no_context (context, data);

  picman_context_get_background (context, &color);
  picman_rgb_to_hsv (&color, &hsv);
  hsv.s = action_select_value ((PicmanActionSelectType) value,
                               hsv.s,
                               0.0, 1.0, 1.0,
                               0.01, 0.01, 0.1, 0.0, FALSE);
  picman_hsv_to_rgb (&hsv, &color);
  picman_context_set_background (context, &color);
}

void
context_background_value_cmd_callback (GtkAction *action,
                                       gint       value,
                                       gpointer   data)
{
  PicmanContext *context;
  PicmanRGB      color;
  PicmanHSV      hsv;
  return_if_no_context (context, data);

  picman_context_get_background (context, &color);
  picman_rgb_to_hsv (&color, &hsv);
  hsv.v = action_select_value ((PicmanActionSelectType) value,
                               hsv.v,
                               0.0, 1.0, 1.0,
                               0.01, 0.01, 0.1, 0.0, FALSE);
  picman_hsv_to_rgb (&hsv, &color);
  picman_context_set_background (context, &color);
}

void
context_opacity_cmd_callback (GtkAction *action,
                              gint       value,
                              gpointer   data)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  return_if_no_context (context, data);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_TOOL_OPTIONS (tool_info->tool_options))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (tool_info->tool_options),
                              "opacity",
                              1.0 / 255.0, 0.01, 0.1, 0.1, FALSE);
    }
}

void
context_paint_mode_cmd_callback (GtkAction *action,
                                 gint       value,
                                 gpointer   data)
{
  PicmanContext          *context;
  PicmanToolInfo         *tool_info;
  PicmanLayerModeEffects  paint_mode;
  gint                  index;
  return_if_no_context (context, data);

  paint_mode = picman_context_get_paint_mode (context);

  index = action_select_value ((PicmanActionSelectType) value,
                               context_paint_mode_index (paint_mode),
                               0, G_N_ELEMENTS (paint_modes) - 1, 0,
                               0.0, 1.0, 1.0, 0.0, FALSE);
  picman_context_set_paint_mode (context, paint_modes[index]);

  tool_info = picman_context_get_tool (context);

  if (tool_info && PICMAN_IS_TOOL_OPTIONS (tool_info->tool_options))
    {
      PicmanDisplay *display;
      const char  *value_desc;

      picman_enum_get_value (PICMAN_TYPE_LAYER_MODE_EFFECTS, index,
                           NULL, NULL, &value_desc, NULL);

      display = action_data_get_display (data);

      if (value_desc && display)
        {
          action_message (display, G_OBJECT (tool_info->tool_options),
                          _("Paint Mode: %s"), value_desc);
        }
    }
}

void
context_tool_select_cmd_callback (GtkAction *action,
                                  gint       value,
                                  gpointer   data)
{
  PicmanContext *context;
  return_if_no_context (context, data);

  context_select_object ((PicmanActionSelectType) value,
                         context, context->picman->tool_info_list);
}

void
context_brush_select_cmd_callback (GtkAction *action,
                                   gint       value,
                                   gpointer   data)
{
  PicmanContext *context;
  return_if_no_context (context, data);

  context_select_object ((PicmanActionSelectType) value,
                         context, picman_data_factory_get_container (context->picman->brush_factory));
}

void
context_pattern_select_cmd_callback (GtkAction *action,
                                     gint       value,
                                     gpointer   data)
{
  PicmanContext *context;
  return_if_no_context (context, data);

  context_select_object ((PicmanActionSelectType) value,
                         context, picman_data_factory_get_container (context->picman->pattern_factory));
}

void
context_palette_select_cmd_callback (GtkAction *action,
                                     gint       value,
                                     gpointer   data)
{
  PicmanContext *context;
  return_if_no_context (context, data);

  context_select_object ((PicmanActionSelectType) value,
                         context, picman_data_factory_get_container (context->picman->palette_factory));
}

void
context_gradient_select_cmd_callback (GtkAction *action,
                                      gint       value,
                                      gpointer   data)
{
  PicmanContext *context;
  return_if_no_context (context, data);

  context_select_object ((PicmanActionSelectType) value,
                         context, picman_data_factory_get_container (context->picman->gradient_factory));
}

void
context_font_select_cmd_callback (GtkAction *action,
                                  gint       value,
                                  gpointer   data)
{
  PicmanContext *context;
  return_if_no_context (context, data);

  context_select_object ((PicmanActionSelectType) value,
                         context, context->picman->fonts);
}

void
context_brush_spacing_cmd_callback (GtkAction *action,
                                    gint       value,
                                    gpointer   data)
{
  PicmanContext *context;
  PicmanBrush   *brush;
  return_if_no_context (context, data);

  brush = picman_context_get_brush (context);

  if (PICMAN_IS_BRUSH (brush) && picman_data_is_writable (PICMAN_DATA (brush)))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (brush),
                              "spacing",
                              1.0, 5.0, 20.0, 0.1, FALSE);
    }
}

void
context_brush_shape_cmd_callback (GtkAction *action,
                                  gint       value,
                                  gpointer   data)
{
  PicmanContext *context;
  PicmanBrush   *brush;
  return_if_no_context (context, data);

  brush = picman_context_get_brush (context);

  if (PICMAN_IS_BRUSH_GENERATED (brush) &&
      picman_data_is_writable (PICMAN_DATA (brush)))
    {
      PicmanBrushGenerated *generated = PICMAN_BRUSH_GENERATED (brush);
      PicmanDisplay        *display;
      const char         *value_desc;

      picman_brush_generated_set_shape (generated,
                                      (PicmanBrushGeneratedShape) value);

      picman_enum_get_value (PICMAN_TYPE_BRUSH_GENERATED_SHAPE, value,
                           NULL, NULL, &value_desc, NULL);
      display = action_data_get_display (data);

      if (value_desc && display)
        {
          action_message (display, G_OBJECT (brush),
                          _("Brush Shape: %s"), value_desc);
        }
    }
}

void
context_brush_radius_cmd_callback (GtkAction *action,
                                   gint       value,
                                   gpointer   data)
{
  PicmanContext *context;
  PicmanBrush   *brush;
  return_if_no_context (context, data);

  brush = picman_context_get_brush (context);

  if (PICMAN_IS_BRUSH_GENERATED (brush) &&
      picman_data_is_writable (PICMAN_DATA (brush)))
    {
      PicmanBrushGenerated *generated = PICMAN_BRUSH_GENERATED (brush);
      PicmanDisplay        *display;
      gdouble             radius;
      gdouble             min_radius;

      radius = picman_brush_generated_get_radius (generated);

      /* If the user uses a high precision radius adjustment command
       * then we allow a minimum radius of 0.1 px, otherwise we set the
       * minimum radius to 1.0 px and adjust the radius to 1.0 px if it
       * is less than 1.0 px. This prevents irritating 0.1, 1.1, 2.1 etc
       * radius sequences when 1.0 px steps are used.
       */
      switch ((PicmanActionSelectType) value)
        {
        case PICMAN_ACTION_SELECT_SMALL_PREVIOUS:
        case PICMAN_ACTION_SELECT_SMALL_NEXT:
        case PICMAN_ACTION_SELECT_PERCENT_PREVIOUS:
        case PICMAN_ACTION_SELECT_PERCENT_NEXT:
          min_radius = 0.1;
          break;

        default:
          min_radius = 1.0;

          if (radius < 1.0)
            radius = 1.0;
          break;
        }

      radius = action_select_value ((PicmanActionSelectType) value,
                                    radius,
                                    min_radius, 4000.0, min_radius,
                                    0.1, 1.0, 10.0, 0.05, FALSE);
      picman_brush_generated_set_radius (generated, radius);

      display = action_data_get_display (data);

      if (display)
        {
          action_message (action_data_get_display (data), G_OBJECT (brush),
                          _("Brush Radius: %2.2f"), radius);
        }
    }
}

void
context_brush_spikes_cmd_callback (GtkAction *action,
                                   gint       value,
                                   gpointer   data)
{
  PicmanContext *context;
  PicmanBrush   *brush;
  return_if_no_context (context, data);

  brush = picman_context_get_brush (context);

  if (PICMAN_IS_BRUSH_GENERATED (brush) &&
      picman_data_is_writable (PICMAN_DATA (brush)))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (brush),
                              "spikes",
                              0.0, 1.0, 4.0, 0.1, FALSE);
    }
}

void
context_brush_hardness_cmd_callback (GtkAction *action,
                                     gint       value,
                                     gpointer   data)
{
  PicmanContext *context;
  PicmanBrush   *brush;
  return_if_no_context (context, data);

  brush = picman_context_get_brush (context);

  if (PICMAN_IS_BRUSH_GENERATED (brush) &&
      picman_data_is_writable (PICMAN_DATA (brush)))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (brush),
                              "hardness",
                              0.001, 0.01, 0.1, 0.1, FALSE);
    }
}

void
context_brush_aspect_cmd_callback (GtkAction *action,
                                   gint       value,
                                   gpointer   data)
{
  PicmanContext *context;
  PicmanBrush   *brush;
  return_if_no_context (context, data);

  brush = picman_context_get_brush (context);

  if (PICMAN_IS_BRUSH_GENERATED (brush) &&
      picman_data_is_writable (PICMAN_DATA (brush)))
    {
      action_select_property ((PicmanActionSelectType) value,
                              action_data_get_display (data),
                              G_OBJECT (brush),
                              "aspect-ratio",
                              0.1, 1.0, 4.0, 0.1, FALSE);
    }
}

void
context_brush_angle_cmd_callback (GtkAction *action,
                                  gint       value,
                                  gpointer   data)
{
  PicmanContext *context;
  PicmanBrush   *brush;
  return_if_no_context (context, data);

  brush = picman_context_get_brush (context);

  if (PICMAN_IS_BRUSH_GENERATED (brush) &&
      picman_data_is_writable (PICMAN_DATA (brush)))
    {
      PicmanBrushGenerated *generated = PICMAN_BRUSH_GENERATED (brush);
      PicmanDisplay        *display;
      gdouble             angle;

      angle = picman_brush_generated_get_angle (generated);

      if (value == PICMAN_ACTION_SELECT_FIRST)
        angle = 0.0;
      else if (value == PICMAN_ACTION_SELECT_LAST)
        angle = 90.0;
      else
        angle = action_select_value ((PicmanActionSelectType) value,
                                     angle,
                                     0.0, 180.0, 0.0,
                                     0.1, 1.0, 15.0, 0.1, TRUE);

      picman_brush_generated_set_angle (generated, angle);

      display = action_data_get_display (data);

      if (display)
        {
          action_message (action_data_get_display (data), G_OBJECT (brush),
                          _("Brush Angle: %2.2f"), angle);
        }
    }
}


/*  private functions  */

static void
context_select_object (PicmanActionSelectType  select_type,
                       PicmanContext          *context,
                       PicmanContainer        *container)
{
  PicmanObject *current;

  current = picman_context_get_by_type (context,
                                      picman_container_get_children_type (container));

  current = action_select_object (select_type, container, current);

  if (current)
    picman_context_set_by_type (context,
                              picman_container_get_children_type (container), current);
}

static gint
context_paint_mode_index (PicmanLayerModeEffects paint_mode)
{
  gint i = 0;

  while (i < (G_N_ELEMENTS (paint_modes) - 1) && paint_modes[i] != paint_mode)
    i++;

  return i;
}

static void
context_select_color (PicmanActionSelectType  select_type,
                      PicmanRGB              *color,
                      gboolean              use_colormap,
                      gboolean              use_palette)
{
  gint index;
  gint max;

  index = context_get_color_index (use_colormap, use_palette, color);
  max   = context_max_color_index (use_colormap, use_palette);

  index = action_select_value (select_type,
                               index,
                               0, max, 0,
                               0, 1, 4, 0, FALSE);

  context_set_color_index (index, use_colormap, use_palette, color);
}

static gint
context_get_color_index (gboolean       use_colormap,
                         gboolean       use_palette,
                         const PicmanRGB *color)
{
  if (use_colormap)
    {
      PicmanColormapEditor *editor = context_get_colormap_editor ();

      if (editor)
        {
          gint index = picman_colormap_editor_get_index (editor, color);

          if (index != -1)
            return index;
        }
    }

  if (use_palette)
    {
      PicmanPaletteEditor *editor = context_get_palette_editor ();

      if (editor)
        {
          gint index = picman_palette_editor_get_index (editor, color);

          if (index != -1)
            return index;
        }
    }

  return 0;
}

static gint
context_max_color_index (gboolean use_colormap,
                         gboolean use_palette)
{
  if (use_colormap)
    {
      PicmanColormapEditor *editor = context_get_colormap_editor ();

      if (editor)
        {
          gint index = picman_colormap_editor_max_index (editor);

          if (index != -1)
            return index;
        }
    }

  if (use_palette)
    {
      PicmanPaletteEditor *editor = context_get_palette_editor ();

      if (editor)
        {
          gint index = picman_palette_editor_max_index (editor);

          if (index != -1)
            return index;
        }
    }

  return 0;
}

static gboolean
context_set_color_index (gint      index,
                         gboolean  use_colormap,
                         gboolean  use_palette,
                         PicmanRGB  *color)
{
  if (use_colormap)
    {
      PicmanColormapEditor *editor = context_get_colormap_editor ();

      if (editor && picman_colormap_editor_set_index (editor, index, color))
        return TRUE;
    }

  if (use_palette)
    {
      PicmanPaletteEditor *editor = context_get_palette_editor ();

      if (editor && picman_palette_editor_set_index (editor, index, color))
        return TRUE;
    }

  return FALSE;
}

static PicmanPaletteEditor *
context_get_palette_editor (void)
{
  GtkWidget *widget;

  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (picman_dialog_factory_get_singleton ()), NULL);

  widget = picman_dialog_factory_find_widget (picman_dialog_factory_get_singleton (),
                                            "picman-palette-editor");
  if (widget)
    return PICMAN_PALETTE_EDITOR (gtk_bin_get_child (GTK_BIN (widget)));

  return NULL;
}

static PicmanColormapEditor *
context_get_colormap_editor (void)
{
  GtkWidget *widget;

  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (picman_dialog_factory_get_singleton ()), NULL);

  widget = picman_dialog_factory_find_widget (picman_dialog_factory_get_singleton (),
                                            "picman-indexed-palette");
  if (widget)
    return PICMAN_COLORMAP_EDITOR (gtk_bin_get_child (GTK_BIN (widget)));

  return NULL;
}
