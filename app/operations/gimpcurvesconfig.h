/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancurvesconfig.h
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

#ifndef __PICMAN_CURVES_CONFIG_H__
#define __PICMAN_CURVES_CONFIG_H__


#include "core/picmanimagemapconfig.h"


#define PICMAN_TYPE_CURVES_CONFIG            (picman_curves_config_get_type ())
#define PICMAN_CURVES_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CURVES_CONFIG, PicmanCurvesConfig))
#define PICMAN_CURVES_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_CURVES_CONFIG, PicmanCurvesConfigClass))
#define PICMAN_IS_CURVES_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CURVES_CONFIG))
#define PICMAN_IS_CURVES_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_CURVES_CONFIG))
#define PICMAN_CURVES_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_CURVES_CONFIG, PicmanCurvesConfigClass))


typedef struct _PicmanCurvesConfigClass PicmanCurvesConfigClass;

struct _PicmanCurvesConfig
{
  PicmanImageMapConfig    parent_instance;

  PicmanHistogramChannel  channel;

  PicmanCurve            *curve[5];
};

struct _PicmanCurvesConfigClass
{
  PicmanImageMapConfigClass  parent_class;
};


GType      picman_curves_config_get_type      (void) G_GNUC_CONST;

GObject  * picman_curves_config_new_spline    (gint32             channel,
                                             const guint8      *points,
                                             gint               n_points);
GObject *  picman_curves_config_new_explicit  (gint32             channel,
                                             const guint8      *points,
                                             gint               n_points);

void       picman_curves_config_reset_channel (PicmanCurvesConfig  *config);

gboolean   picman_curves_config_load_cruft    (PicmanCurvesConfig  *config,
                                             gpointer           fp,
                                             GError           **error);
gboolean   picman_curves_config_save_cruft    (PicmanCurvesConfig  *config,
                                             gpointer           fp,
                                             GError           **error);


#endif /* __PICMAN_CURVES_CONFIG_H__ */
