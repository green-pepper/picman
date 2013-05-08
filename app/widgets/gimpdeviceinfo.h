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

#ifndef __PICMAN_DEVICE_INFO_H__
#define __PICMAN_DEVICE_INFO_H__


#include "core/picmancontext.h"


G_BEGIN_DECLS


#define PICMAN_DEVICE_INFO_CONTEXT_MASK (PICMAN_CONTEXT_TOOL_MASK       | \
                                       PICMAN_CONTEXT_PAINT_INFO_MASK | \
                                       PICMAN_CONTEXT_FOREGROUND_MASK | \
                                       PICMAN_CONTEXT_BACKGROUND_MASK | \
                                       PICMAN_CONTEXT_BRUSH_MASK      | \
                                       PICMAN_CONTEXT_DYNAMICS_MASK   | \
                                       PICMAN_CONTEXT_PATTERN_MASK    | \
                                       PICMAN_CONTEXT_GRADIENT_MASK)


#define PICMAN_TYPE_DEVICE_INFO            (picman_device_info_get_type ())
#define PICMAN_DEVICE_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DEVICE_INFO, PicmanDeviceInfo))
#define PICMAN_DEVICE_INFO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DEVICE_INFO, PicmanDeviceInfoClass))
#define PICMAN_IS_DEVICE_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DEVICE_INFO))
#define PICMAN_IS_DEVICE_INFO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DEVICE_INFO))
#define PICMAN_DEVICE_INFO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DEVICE_INFO, PicmanDeviceInfoClass))


typedef struct _PicmanDeviceInfoClass PicmanDeviceInfoClass;

struct _PicmanDeviceInfo
{
  PicmanContext    parent_instance;

  GdkDevice     *device;
  GdkDisplay    *display;

  /*  either "device" or the options below are set  */

  GdkInputMode   mode;
  gint           n_axes;
  GdkAxisUse    *axes;
  gint           n_keys;
  GdkDeviceKey  *keys;

  /*  curves  */

  PicmanCurve     *pressure_curve;
};

struct _PicmanDeviceInfoClass
{
  PicmanContextClass  parent_class;

  void (* changed) (PicmanDeviceInfo *device_info);
};


GType            picman_device_info_get_type          (void) G_GNUC_CONST;

PicmanDeviceInfo * picman_device_info_new               (Picman            *picman,
                                                     GdkDevice       *device,
                                                     GdkDisplay      *display);

GdkDevice      * picman_device_info_get_device        (PicmanDeviceInfo  *info,
                                                     GdkDisplay     **display);
void             picman_device_info_set_device        (PicmanDeviceInfo  *info,
                                                     GdkDevice       *device,
                                                     GdkDisplay      *display);

void             picman_device_info_set_default_tool  (PicmanDeviceInfo  *info);

GdkInputMode     picman_device_info_get_mode          (PicmanDeviceInfo  *info);
void             picman_device_info_set_mode          (PicmanDeviceInfo  *info,
                                                     GdkInputMode     mode);

gboolean         picman_device_info_has_cursor        (PicmanDeviceInfo  *info);

gint             picman_device_info_get_n_axes        (PicmanDeviceInfo  *info);
GdkAxisUse       picman_device_info_get_axis_use      (PicmanDeviceInfo  *info,
                                                     gint             axis);
void             picman_device_info_set_axis_use      (PicmanDeviceInfo  *info,
                                                     gint             axis,
                                                     GdkAxisUse       use);

gint             picman_device_info_get_n_keys        (PicmanDeviceInfo  *info);
void             picman_device_info_get_key           (PicmanDeviceInfo  *info,
                                                     gint             key,
                                                     guint           *keyval,
                                                     GdkModifierType *modifiers);
void             picman_device_info_set_key           (PicmanDeviceInfo  *info,
                                                     gint             key,
                                                     guint            keyval,
                                                     GdkModifierType  modifiers);

PicmanCurve      * picman_device_info_get_curve         (PicmanDeviceInfo  *info,
                                                     GdkAxisUse       use);
gdouble          picman_device_info_map_axis          (PicmanDeviceInfo  *info,
                                                     GdkAxisUse       use,
                                                     gdouble          value);

void             picman_device_info_changed           (PicmanDeviceInfo  *info);

PicmanDeviceInfo * picman_device_info_get_by_device     (GdkDevice       *device);

gint             picman_device_info_compare           (PicmanDeviceInfo  *a,
                                                     PicmanDeviceInfo  *b);


G_END_DECLS

#endif /* __PICMAN_DEVICE_INFO_H__ */
