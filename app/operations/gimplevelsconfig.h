/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanlevelsconfig.h
 * Copyright (C) 2007 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_LEVELS_CONFIG_H__
#define __PICMAN_LEVELS_CONFIG_H__


#include "core/picmanimagemapconfig.h"


#define PICMAN_TYPE_LEVELS_CONFIG            (picman_levels_config_get_type ())
#define PICMAN_LEVELS_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_LEVELS_CONFIG, PicmanLevelsConfig))
#define PICMAN_LEVELS_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_LEVELS_CONFIG, PicmanLevelsConfigClass))
#define PICMAN_IS_LEVELS_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_LEVELS_CONFIG))
#define PICMAN_IS_LEVELS_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_LEVELS_CONFIG))
#define PICMAN_LEVELS_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_LEVELS_CONFIG, PicmanLevelsConfigClass))


typedef struct _PicmanLevelsConfigClass PicmanLevelsConfigClass;

struct _PicmanLevelsConfig
{
  PicmanImageMapConfig    parent_instance;

  PicmanHistogramChannel  channel;

  gdouble               gamma[5];

  gdouble               low_input[5];
  gdouble               high_input[5];

  gdouble               low_output[5];
  gdouble               high_output[5];
};

struct _PicmanLevelsConfigClass
{
  PicmanImageMapConfigClass  parent_class;
};


GType      picman_levels_config_get_type         (void) G_GNUC_CONST;

void       picman_levels_config_reset_channel    (PicmanLevelsConfig      *config);

void       picman_levels_config_stretch          (PicmanLevelsConfig      *config,
                                                PicmanHistogram         *histogram,
                                                gboolean               is_color);
void       picman_levels_config_stretch_channel  (PicmanLevelsConfig      *config,
                                                PicmanHistogram         *histogram,
                                                PicmanHistogramChannel   channel);
void       picman_levels_config_adjust_by_colors (PicmanLevelsConfig      *config,
                                                PicmanHistogramChannel   channel,
                                                const PicmanRGB         *black,
                                                const PicmanRGB         *gray,
                                                const PicmanRGB         *white);

PicmanCurvesConfig *
           picman_levels_config_to_curves_config (PicmanLevelsConfig      *config);

gboolean   picman_levels_config_load_cruft       (PicmanLevelsConfig      *config,
                                                gpointer               fp,
                                                GError               **error);
gboolean   picman_levels_config_save_cruft       (PicmanLevelsConfig      *config,
                                                gpointer               fp,
                                                GError               **error);


#endif /* __PICMAN_LEVELS_CONFIG_H__ */
