/*
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This is a plug-in for PICMAN.
 *
 * Generates images containing vector type drawings.
 *
 * Copyright (C) 1997 Andy Thomas  alt@picnic.demon.co.uk
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
 *
 */

#ifndef __GFIG_STYLE_H__
#define __GFIG_STYLE_H__

#include <libpicman/picmanui.h>

struct _Style
{
  gchar     *name;
  gchar     *brush_name;
  gint       brush_width;
  gint       brush_height;
  gint       brush_spacing;
  BrushType  brush_type;
  gdouble    brushfade;
  gdouble    brushgradient;
  gdouble    airbrushpressure;
  FillType   fill_type;
  gdouble    fill_opacity;
  gchar     *pattern;
  gchar     *gradient;
  PaintType  paint_type;
  PicmanRGB    foreground;
  PicmanRGB    background;
  gboolean   reverselines;
  gint       ref_count;
};

gboolean gfig_load_style                   (Style                *style,
                                            FILE                 *fp);

gboolean gfig_skip_style                   (Style                *style,
                                            FILE                 *fp);

gboolean gfig_load_styles                  (GFigObj              *gfig,
                                            FILE                 *fp);

void     gfig_save_style                   (Style                *style,
                                            GString              *string);

void     gfig_style_save_as_attributes     (Style                *style,
                                            GString              *string);

void     gfig_save_styles                  (GString              *string);

void     set_foreground_callback           (PicmanColorButton      *button,
                                            gpointer              data);

void     set_background_callback           (PicmanColorButton      *button,
                                            gpointer              data);

void     set_paint_type_callback           (GtkToggleButton      *toggle,
                                            gpointer              data);

void     gfig_brush_changed_callback       (PicmanBrushSelectButton *button,
                                            const gchar          *brush_name,
                                            gdouble               opacity,
                                            gint                  spacing,
                                            PicmanLayerModeEffects  paint_mode,
                                            gint                  width,
                                            gint                  height,
                                            const guchar         *mask_data,
                                            gboolean              dialog_closing,
                                            gpointer              user_data);

void     gfig_pattern_changed_callback     (PicmanPatternSelectButton *button,
                                            const gchar          *pattern_name,
                                            gint                  width,
                                            gint                  height,
                                            gint                  bpp,
                                            const guchar         *mask_data,
                                            gboolean              dialog_closing,
                                            gpointer              user_data);

void     gfig_gradient_changed_callback    (PicmanGradientSelectButton *button,
                                            const gchar          *gradient_name,
                                            gint                  width,
                                            const gdouble        *grad_data,
                                            gboolean              dialog_closing,
                                            gpointer              user_data);

void     gfig_rgba_copy                    (PicmanRGB              *color1,
                                            PicmanRGB              *color2);

void     gfig_style_copy                   (Style                *style1,
                                            Style                *style0,
                                            const gchar          *name);

void     gfig_style_apply                  (Style                *style);

void     gfig_read_picman_style              (Style                *style,
                                            const gchar          *name);

void     gfig_style_set_context_from_style (Style                *style);

void     gfig_style_set_style_from_context (Style                *style);

void     mypicman_brush_info                 (gint                 *width,
                                            gint                 *height);

Style   *gfig_context_get_current_style    (void);

#endif /* __GFIG_STYLE_H__ */
