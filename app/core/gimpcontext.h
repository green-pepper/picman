/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontext.h
 * Copyright (C) 1999-2010 Michael Natterer
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

#ifndef __PICMAN_CONTEXT_H__
#define __PICMAN_CONTEXT_H__


#include "picmanviewable.h"


#define PICMAN_TYPE_CONTEXT            (picman_context_get_type ())
#define PICMAN_CONTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTEXT, PicmanContext))
#define PICMAN_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST (klass, PICMAN_TYPE_CONTEXT, PicmanContextClass))
#define PICMAN_IS_CONTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTEXT))
#define PICMAN_IS_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTEXT))
#define PICMAN_CONTEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS (obj, PICMAN_TYPE_CONTEXT, PicmanContextClass))


typedef struct _PicmanContextClass PicmanContextClass;

/**
 * PicmanContext:
 *
 * Holds state such as the active image, active display, active brush,
 * active foreground and background color, and so on. There can many
 * instances of contexts. The user context is what the user sees and
 * interacts with but there can also be contexts for docks and for
 * plug-ins.
 */
struct _PicmanContext
{
  PicmanViewable          parent_instance;

  Picman                 *picman;

  PicmanContext          *parent;

  guint32               defined_props;
  guint32               serialize_props;

  PicmanImage            *image;
  gpointer              display;

  PicmanToolInfo         *tool_info;
  gchar                *tool_name;

  PicmanPaintInfo        *paint_info;
  gchar                *paint_name;

  PicmanRGB               foreground;
  PicmanRGB               background;

  gdouble               opacity;
  PicmanLayerModeEffects  paint_mode;

  PicmanBrush            *brush;
  gchar                *brush_name;

  PicmanDynamics         *dynamics;
  gchar                *dynamics_name;

  PicmanPattern          *pattern;
  gchar                *pattern_name;

  PicmanGradient         *gradient;
  gchar                *gradient_name;

  PicmanPalette          *palette;
  gchar                *palette_name;

  PicmanToolPreset       *tool_preset;
  gchar                *tool_preset_name;

  PicmanFont             *font;
  gchar                *font_name;

  PicmanBuffer           *buffer;
  gchar                *buffer_name;

  PicmanImagefile        *imagefile;
  gchar                *imagefile_name;

  PicmanTemplate         *template;
  gchar                *template_name;
};

struct _PicmanContextClass
{
  PicmanViewableClass  parent_class;

  void (* image_changed)      (PicmanContext          *context,
                               PicmanImage            *image);
  void (* display_changed)    (PicmanContext          *context,
                               gpointer              display);

  void (* tool_changed)       (PicmanContext          *context,
                               PicmanToolInfo         *tool_info);
  void (* paint_info_changed) (PicmanContext          *context,
                               PicmanPaintInfo        *paint_info);

  void (* foreground_changed) (PicmanContext          *context,
                               PicmanRGB              *color);
  void (* background_changed) (PicmanContext          *context,
                               PicmanRGB              *color);
  void (* opacity_changed)    (PicmanContext          *context,
                               gdouble               opacity);
  void (* paint_mode_changed) (PicmanContext          *context,
                               PicmanLayerModeEffects  paint_mode);
  void (* brush_changed)      (PicmanContext          *context,
                               PicmanBrush            *brush);
  void (* dynamics_changed)   (PicmanContext          *context,
                               PicmanDynamics         *dynamics);
  void (* pattern_changed)    (PicmanContext          *context,
                               PicmanPattern          *pattern);
  void (* gradient_changed)   (PicmanContext          *context,
                               PicmanGradient         *gradient);
  void (* palette_changed)    (PicmanContext          *context,
                               PicmanPalette          *palette);
  void (* tool_preset_changed)(PicmanContext          *context,
                               PicmanToolPreset       *tool_preset);
  void (* font_changed)       (PicmanContext          *context,
                               PicmanFont             *font);
  void (* buffer_changed)     (PicmanContext          *context,
                               PicmanBuffer           *buffer);
  void (* imagefile_changed)  (PicmanContext          *context,
                               PicmanImagefile        *imagefile);
  void (* template_changed)   (PicmanContext          *context,
                               PicmanTemplate         *template);
};


GType         picman_context_get_type          (void) G_GNUC_CONST;

PicmanContext * picman_context_new               (Picman                *picman,
                                              const gchar         *name,
                                              PicmanContext         *template);

PicmanContext * picman_context_get_parent        (const PicmanContext   *context);
void          picman_context_set_parent        (PicmanContext         *context,
                                              PicmanContext         *parent);

/*  define / undefinine context properties
 *
 *  the value of an undefined property will be taken from the parent context.
 */
void          picman_context_define_property   (PicmanContext         *context,
                                              PicmanContextPropType  prop,
                                              gboolean             defined);

gboolean      picman_context_property_defined  (PicmanContext         *context,
                                              PicmanContextPropType  prop);

void          picman_context_define_properties (PicmanContext         *context,
                                              PicmanContextPropMask  props_mask,
                                              gboolean             defined);


/*  specify which context properties will be serialized
 */
void   picman_context_set_serialize_properties (PicmanContext         *context,
                                              PicmanContextPropMask  props_mask);

PicmanContextPropMask
       picman_context_get_serialize_properties (PicmanContext         *context);


/*  copying context properties
 */
void          picman_context_copy_property     (PicmanContext         *src,
                                              PicmanContext         *dest,
                                              PicmanContextPropType  prop);

void          picman_context_copy_properties   (PicmanContext         *src,
                                              PicmanContext         *dest,
                                              PicmanContextPropMask  props_mask);


/*  manipulate by GType  */
PicmanContextPropType   picman_context_type_to_property    (GType     type);
const gchar         * picman_context_type_to_prop_name   (GType     type);
const gchar         * picman_context_type_to_signal_name (GType     type);

PicmanObject     * picman_context_get_by_type         (PicmanContext     *context,
                                                   GType            type);
void             picman_context_set_by_type         (PicmanContext     *context,
                                                   GType            type,
                                                   PicmanObject      *object);
void             picman_context_changed_by_type     (PicmanContext     *context,
                                                   GType            type);


/*  image  */
PicmanImage      * picman_context_get_image           (PicmanContext     *context);
void             picman_context_set_image           (PicmanContext     *context,
                                                   PicmanImage       *image);
void             picman_context_image_changed       (PicmanContext     *context);


/*  display  */
gpointer         picman_context_get_display         (PicmanContext     *context);
void             picman_context_set_display         (PicmanContext     *context,
                                                   gpointer         display);
void             picman_context_display_changed     (PicmanContext     *context);


/*  tool  */
PicmanToolInfo   * picman_context_get_tool            (PicmanContext     *context);
void             picman_context_set_tool            (PicmanContext     *context,
                                                   PicmanToolInfo    *tool_info);
void             picman_context_tool_changed        (PicmanContext     *context);


/*  paint info  */
PicmanPaintInfo  * picman_context_get_paint_info      (PicmanContext     *context);
void             picman_context_set_paint_info      (PicmanContext     *context,
                                                   PicmanPaintInfo   *paint_info);
void             picman_context_paint_info_changed  (PicmanContext     *context);


/*  foreground color  */
void             picman_context_get_foreground       (PicmanContext     *context,
                                                    PicmanRGB         *color);
void             picman_context_get_foreground_pixel (PicmanContext     *context,
                                                    const Babl      *pixel_format,
                                                    gpointer         pixel);
void             picman_context_set_foreground       (PicmanContext     *context,
                                                    const PicmanRGB   *color);
void             picman_context_set_foreground_pixel (PicmanContext     *context,
                                                    const Babl      *pixel_format,
                                                    gconstpointer    pixel);
void             picman_context_foreground_changed   (PicmanContext     *context);


/*  background color  */
void             picman_context_get_background       (PicmanContext     *context,
                                                    PicmanRGB         *color);
void             picman_context_get_background_pixel (PicmanContext     *context,
                                                    const Babl      *pixel_format,
                                                    gpointer         pixel);
void             picman_context_set_background       (PicmanContext     *context,
                                                    const PicmanRGB   *color);
void             picman_context_set_background_pixel (PicmanContext     *context,
                                                    const Babl      *pixel_format,
                                                    gconstpointer    pixel);
void             picman_context_background_changed   (PicmanContext     *context);


/*  color utility functions  */
void             picman_context_set_default_colors  (PicmanContext     *context);
void             picman_context_swap_colors         (PicmanContext     *context);


/*  opacity  */
gdouble          picman_context_get_opacity         (PicmanContext     *context);
void             picman_context_set_opacity         (PicmanContext     *context,
                                                   gdouble          opacity);
void             picman_context_opacity_changed     (PicmanContext     *context);


/*  paint mode  */
PicmanLayerModeEffects
                 picman_context_get_paint_mode      (PicmanContext     *context);
void             picman_context_set_paint_mode      (PicmanContext     *context,
                                              PicmanLayerModeEffects  paint_mode);
void             picman_context_paint_mode_changed  (PicmanContext     *context);


/*  brush  */
PicmanBrush      * picman_context_get_brush           (PicmanContext     *context);
void             picman_context_set_brush           (PicmanContext     *context,
                                                   PicmanBrush       *brush);
void             picman_context_brush_changed       (PicmanContext     *context);


/*  dynamics  */
PicmanDynamics   * picman_context_get_dynamics        (PicmanContext     *context);
void             picman_context_set_dynamics        (PicmanContext     *context,
                                                   PicmanDynamics    *dynamics);
void             picman_context_dynamics_changed    (PicmanContext     *context);


/*  pattern  */
PicmanPattern    * picman_context_get_pattern         (PicmanContext     *context);
void             picman_context_set_pattern         (PicmanContext     *context,
                                                   PicmanPattern     *pattern);
void             picman_context_pattern_changed     (PicmanContext     *context);


/*  gradient  */
PicmanGradient   * picman_context_get_gradient        (PicmanContext     *context);
void             picman_context_set_gradient        (PicmanContext     *context,
                                                   PicmanGradient    *gradient);
void             picman_context_gradient_changed    (PicmanContext     *context);


/*  palette  */
PicmanPalette    * picman_context_get_palette         (PicmanContext     *context);
void             picman_context_set_palette         (PicmanContext     *context,
                                                   PicmanPalette     *palette);
void             picman_context_palette_changed     (PicmanContext     *context);


/*  tool_preset  */
PicmanToolPreset * picman_context_get_tool_preset     (PicmanContext     *context);
void             picman_context_set_tool_preset     (PicmanContext     *context,
                                                   PicmanToolPreset  *tool_preset);
void             picman_context_tool_preset_changed (PicmanContext     *context);


/*  font  */
PicmanFont       * picman_context_get_font            (PicmanContext     *context);
void             picman_context_set_font            (PicmanContext     *context,
                                                   PicmanFont        *font);
const gchar    * picman_context_get_font_name       (PicmanContext     *context);
void             picman_context_set_font_name       (PicmanContext     *context,
                                                   const gchar     *name);
void             picman_context_font_changed        (PicmanContext     *context);


/*  buffer  */
PicmanBuffer     * picman_context_get_buffer          (PicmanContext     *context);
void             picman_context_set_buffer          (PicmanContext     *context,
                                                   PicmanBuffer      *palette);
void             picman_context_buffer_changed      (PicmanContext     *context);


/*  imagefile  */
PicmanImagefile  * picman_context_get_imagefile       (PicmanContext     *context);
void             picman_context_set_imagefile       (PicmanContext     *context,
                                                   PicmanImagefile   *imagefile);
void             picman_context_imagefile_changed   (PicmanContext     *context);


/*  template  */
PicmanTemplate   * picman_context_get_template        (PicmanContext     *context);
void             picman_context_set_template        (PicmanContext     *context,
                                                   PicmanTemplate    *template);
void             picman_context_template_changed    (PicmanContext     *context);


#endif /* __PICMAN_CONTEXT_H__ */
