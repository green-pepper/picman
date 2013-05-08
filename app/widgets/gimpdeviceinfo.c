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

#undef GSEAL_ENABLE

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancurve.h"
#include "core/picmancurve-map.h"
#include "core/picmandatafactory.h"
#include "core/picmanmarshal.h"
#include "core/picmanparamspecs.h"
#include "core/picmantoolinfo.h"

#include "picmandeviceinfo.h"


#define PICMAN_DEVICE_INFO_DATA_KEY "picman-device-info"


enum
{
  CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_DEVICE,
  PROP_DISPLAY,
  PROP_MODE,
  PROP_AXES,
  PROP_KEYS,
  PROP_PRESSURE_CURVE
};


/*  local function prototypes  */

static void   picman_device_info_constructed  (GObject        *object);
static void   picman_device_info_finalize     (GObject        *object);
static void   picman_device_info_set_property (GObject        *object,
                                             guint           property_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void   picman_device_info_get_property (GObject        *object,
                                             guint           property_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static void   picman_device_info_guess_icon   (PicmanDeviceInfo *info);


G_DEFINE_TYPE (PicmanDeviceInfo, picman_device_info, PICMAN_TYPE_CONTEXT)

#define parent_class picman_device_info_parent_class

static guint device_info_signals[LAST_SIGNAL] = { 0 };


static void
picman_device_info_class_init (PicmanDeviceInfoClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);
  GParamSpec        *param_spec;

  device_info_signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDeviceInfoClass, changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->constructed        = picman_device_info_constructed;
  object_class->finalize           = picman_device_info_finalize;
  object_class->set_property       = picman_device_info_set_property;
  object_class->get_property       = picman_device_info_get_property;

  viewable_class->default_stock_id = PICMAN_STOCK_INPUT_DEVICE;

  g_object_class_install_property (object_class, PROP_DEVICE,
                                   g_param_spec_object ("device",
                                                        NULL, NULL,
                                                        GDK_TYPE_DEVICE,
                                                        PICMAN_PARAM_STATIC_STRINGS |
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_DISPLAY,
                                   g_param_spec_object ("display",
                                                        NULL, NULL,
                                                        GDK_TYPE_DISPLAY,
                                                        PICMAN_PARAM_STATIC_STRINGS |
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_MODE, "mode", NULL,
                                 GDK_TYPE_INPUT_MODE,
                                 GDK_MODE_DISABLED,
                                 PICMAN_PARAM_STATIC_STRINGS);

  param_spec = g_param_spec_enum ("axis",
                                  NULL, NULL,
                                  GDK_TYPE_AXIS_USE,
                                  GDK_AXIS_IGNORE,
                                  PICMAN_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_AXES,
                                   picman_param_spec_value_array ("axes",
                                                                NULL, NULL,
                                                                param_spec,
                                                                PICMAN_PARAM_STATIC_STRINGS |
                                                                PICMAN_CONFIG_PARAM_FLAGS));

  param_spec = g_param_spec_string ("key",
                                    NULL, NULL,
                                    NULL,
                                    PICMAN_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_KEYS,
                                   picman_param_spec_value_array ("keys",
                                                                NULL, NULL,
                                                                param_spec,
                                                                PICMAN_PARAM_STATIC_STRINGS |
                                                                PICMAN_CONFIG_PARAM_FLAGS));

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_PRESSURE_CURVE,
                                   "pressure-curve", NULL,
                                   PICMAN_TYPE_CURVE,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);
}

static void
picman_device_info_init (PicmanDeviceInfo *info)
{
  info->device   = NULL;
  info->display  = NULL;
  info->mode     = GDK_MODE_DISABLED;
  info->n_axes   = 0;
  info->axes     = NULL;
  info->n_keys   = 0;
  info->keys     = NULL;

  info->pressure_curve = PICMAN_CURVE (picman_curve_new ("pressure curve"));

  g_signal_connect (info, "notify::name",
                    G_CALLBACK (picman_device_info_guess_icon),
                    NULL);
}

static void
picman_device_info_constructed (GObject *object)
{
  PicmanDeviceInfo *info = PICMAN_DEVICE_INFO (object);
  Picman           *picman;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert ((info->device == NULL         && info->display == NULL) ||
            (GDK_IS_DEVICE (info->device) && GDK_IS_DISPLAY (info->display)));

  picman = PICMAN_CONTEXT (object)->picman;

  if (info->device)
    {
      g_object_set_data (G_OBJECT (info->device), PICMAN_DEVICE_INFO_DATA_KEY,
                         info);

      picman_object_set_name (PICMAN_OBJECT (info), info->device->name);

      info->mode    = info->device->mode;
      info->n_axes  = info->device->num_axes;
      info->n_keys  = info->device->num_keys;
    }

  picman_context_define_properties (PICMAN_CONTEXT (object),
                                  PICMAN_DEVICE_INFO_CONTEXT_MASK,
                                  FALSE);
  picman_context_copy_properties (picman_get_user_context (picman),
                                PICMAN_CONTEXT (object),
                                PICMAN_DEVICE_INFO_CONTEXT_MASK);

  picman_context_set_serialize_properties (PICMAN_CONTEXT (object),
                                         PICMAN_DEVICE_INFO_CONTEXT_MASK);

  /*  FIXME: this is ugly and needs to be done via "notify" once
   *  the contexts' properties are dynamic.
   */
  g_signal_connect (object, "foreground-changed",
                    G_CALLBACK (picman_device_info_changed),
                    NULL);
  g_signal_connect (object, "background-changed",
                    G_CALLBACK (picman_device_info_changed),
                    NULL);
  g_signal_connect (object, "tool-changed",
                    G_CALLBACK (picman_device_info_changed),
                    NULL);
  g_signal_connect (object, "brush-changed",
                    G_CALLBACK (picman_device_info_changed),
                    NULL);
  g_signal_connect (object, "pattern-changed",
                    G_CALLBACK (picman_device_info_changed),
                    NULL);
  g_signal_connect (object, "gradient-changed",
                    G_CALLBACK (picman_device_info_changed),
                    NULL);
}

static void
picman_device_info_finalize (GObject *object)
{
  PicmanDeviceInfo *info = PICMAN_DEVICE_INFO (object);

  if (info->axes)
    {
      g_free (info->axes);
      info->axes = NULL;
    }

  if (info->keys)
    {
      g_free (info->keys);
      info->keys = NULL;
    }

  if (info->pressure_curve)
    {
      g_object_unref (info->pressure_curve);
      info->pressure_curve = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_device_info_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanDeviceInfo *info       = PICMAN_DEVICE_INFO (object);
  GdkDevice      *device     = info->device;
  PicmanCurve      *src_curve  = NULL;
  PicmanCurve      *dest_curve = NULL;

  switch (property_id)
    {
    case PROP_DEVICE:
      info->device = g_value_get_object (value);
      break;

    case PROP_DISPLAY:
      info->display = g_value_get_object (value);
      break;

    case PROP_MODE:
      picman_device_info_set_mode (info, g_value_get_enum (value));
      break;

    case PROP_AXES:
      {
        PicmanValueArray *array = g_value_get_boxed (value);

        if (array)
          {
            gint i;
            gint n_device_values;

            if (device)
              {
                n_device_values = MIN (picman_value_array_length (array),
                                       device->num_axes);
              }
            else
              {
                n_device_values = picman_value_array_length (array);

                info->n_axes = n_device_values;
                info->axes   = g_renew (GdkAxisUse, info->axes, info->n_axes);
                memset (info->axes, 0, info->n_axes * sizeof (GdkAxisUse));
              }

            for (i = 0; i < n_device_values; i++)
              {
                GdkAxisUse axis_use;

                axis_use = g_value_get_enum (picman_value_array_index (array, i));

                picman_device_info_set_axis_use (info, i, axis_use);
              }
          }
      }
      break;

    case PROP_KEYS:
      {
        PicmanValueArray *array = g_value_get_boxed (value);

        if (array)
          {
            gint i;
            gint n_device_values;

            if (device)
              {
                n_device_values = MIN (picman_value_array_length (array),
                                       device->num_keys);
              }
            else
              {
                n_device_values = picman_value_array_length (array);

                info->n_keys = n_device_values;
                info->keys   = g_renew (GdkDeviceKey, info->keys, info->n_keys);
                memset (info->keys, 0, info->n_keys * sizeof (GdkDeviceKey));
              }

            for (i = 0; i < n_device_values; i++)
              {
                const gchar     *accel;
                guint            keyval;
                GdkModifierType  modifiers;

                accel = g_value_get_string (picman_value_array_index (array, i));

                gtk_accelerator_parse (accel, &keyval, &modifiers);

                picman_device_info_set_key (info, i, keyval, modifiers);
              }
          }
      }
      break;

    case PROP_PRESSURE_CURVE:
      src_curve  = g_value_get_object (value);
      dest_curve = info->pressure_curve;
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }

  if (src_curve && dest_curve)
    {
      picman_config_copy (PICMAN_CONFIG (src_curve),
                        PICMAN_CONFIG (dest_curve),
                        PICMAN_CONFIG_PARAM_SERIALIZE);
    }
}

static void
picman_device_info_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanDeviceInfo *info = PICMAN_DEVICE_INFO (object);

  switch (property_id)
    {
    case PROP_DEVICE:
      g_value_set_object (value, info->device);
      break;

    case PROP_DISPLAY:
      g_value_set_object (value, info->display);
      break;

    case PROP_MODE:
      g_value_set_enum (value, picman_device_info_get_mode (info));
      break;

    case PROP_AXES:
      {
        PicmanValueArray *array;
        GValue          enum_value = { 0, };
        gint            n_axes;
        gint            i;

        array = picman_value_array_new (6);
        g_value_init (&enum_value, GDK_TYPE_AXIS_USE);

        n_axes = picman_device_info_get_n_axes (info);

        for (i = 0; i < n_axes; i++)
          {
            g_value_set_enum (&enum_value,
                              picman_device_info_get_axis_use (info, i));

            picman_value_array_append (array, &enum_value);
          }

        g_value_unset (&enum_value);

        g_value_take_boxed (value, array);
      }
      break;

    case PROP_KEYS:
      {
        PicmanValueArray *array;
        GValue          string_value = { 0, };
        gint            n_keys;
        gint            i;

        array = picman_value_array_new (32);
        g_value_init (&string_value, G_TYPE_STRING);

        n_keys = picman_device_info_get_n_keys (info);

        for (i = 0; i < n_keys; i++)
          {
            guint           keyval;
            GdkModifierType modifiers;

            picman_device_info_get_key (info, i, &keyval, &modifiers);

            if (keyval)
              {
                gchar *accel;
                gchar *escaped;

                accel = gtk_accelerator_name (keyval, modifiers);
                escaped = g_strescape (accel, NULL);
                g_free (accel);

                g_value_set_string (&string_value, escaped);
                g_free (escaped);
              }
            else
              {
                g_value_set_string (&string_value, "");
              }

            picman_value_array_append (array, &string_value);
          }

        g_value_unset (&string_value);

        g_value_take_boxed (value, array);
      }
      break;

    case PROP_PRESSURE_CURVE:
      g_value_set_object (value, info->pressure_curve);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_device_info_guess_icon (PicmanDeviceInfo *info)
{
  PicmanViewable *viewable = PICMAN_VIEWABLE (info);

  if (picman_object_get_name (viewable) &&
      ! strcmp (picman_viewable_get_stock_id (viewable),
                PICMAN_VIEWABLE_GET_CLASS (viewable)->default_stock_id))
    {
      const gchar *stock_id = NULL;
      gchar       *down     = g_ascii_strdown (picman_object_get_name (viewable),
                                               -1);

      if (strstr (down, "eraser"))
        {
          stock_id = PICMAN_STOCK_TOOL_ERASER;
        }
      else if (strstr (down, "pen"))
        {
          stock_id = PICMAN_STOCK_TOOL_PAINTBRUSH;
        }
      else if (strstr (down, "airbrush"))
        {
          stock_id = PICMAN_STOCK_TOOL_AIRBRUSH;
        }
      else if (strstr (down, "cursor")   ||
               strstr (down, "mouse")    ||
               strstr (down, "pointer")  ||
               strstr (down, "touchpad") ||
               strstr (down, "trackpoint"))
        {
          stock_id = PICMAN_STOCK_CURSOR;
        }

      g_free (down);

      if (stock_id)
        picman_viewable_set_stock_id (viewable, stock_id);
    }
}



/*  public functions  */

PicmanDeviceInfo *
picman_device_info_new (Picman       *picman,
                      GdkDevice  *device,
                      GdkDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (GDK_IS_DEVICE (device), NULL);
  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);

  return g_object_new (PICMAN_TYPE_DEVICE_INFO,
                       "picman",    picman,
                       "device",  device,
                       "display", display,
                       NULL);
}

GdkDevice *
picman_device_info_get_device (PicmanDeviceInfo  *info,
                             GdkDisplay     **display)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_INFO (info), NULL);

  if (display)
    *display = info->display;

  return info->device;
}

void
picman_device_info_set_device (PicmanDeviceInfo *info,
                             GdkDevice      *device,
                             GdkDisplay     *display)
{
  gint i;

  g_return_if_fail (PICMAN_IS_DEVICE_INFO (info));
  g_return_if_fail ((device == NULL && display == NULL) ||
                    (GDK_IS_DEVICE (device) && GDK_IS_DISPLAY (display)));
  g_return_if_fail ((info->device == NULL && GDK_IS_DEVICE (device)) ||
                    (GDK_IS_DEVICE (info->device) && device == NULL));
  g_return_if_fail (device == NULL ||
                    strcmp (device->name,
                            picman_object_get_name (info)) == 0);

  if (device)
    {
      info->device  = device;
      info->display = display;

      g_object_set_data (G_OBJECT (device), PICMAN_DEVICE_INFO_DATA_KEY, info);

      picman_device_info_set_mode (info, info->mode);

      if (info->n_axes != device->num_axes)
        g_printerr ("%s: stored 'num-axes' for device '%s' doesn't match "
                    "number of axes present in device\n",
                    G_STRFUNC, device->name);

      for (i = 0; i < MIN (info->n_axes, device->num_axes); i++)
        picman_device_info_set_axis_use (info, i,
                                       info->axes[i]);

      if (info->n_keys != device->num_keys)
        g_printerr ("%s: stored 'num-keys' for device '%s' doesn't match "
                    "number of keys present in device\n",
                    G_STRFUNC, device->name);

      for (i = 0; i < MIN (info->n_keys, device->num_keys); i++)
        picman_device_info_set_key (info, i,
                                  info->keys[i].keyval,
                                  info->keys[i].modifiers);
    }
  else
    {
      device  = info->device;
      display = info->display;

      info->device  = NULL;
      info->display = NULL;

      g_object_set_data (G_OBJECT (device), PICMAN_DEVICE_INFO_DATA_KEY, NULL);

      picman_device_info_set_mode (info, device->mode);

      info->n_axes = device->num_axes;
      info->axes   = g_renew (GdkAxisUse, info->axes, info->n_axes);
      memset (info->axes, 0, info->n_axes * sizeof (GdkAxisUse));

      for (i = 0; i < device->num_axes; i++)
        picman_device_info_set_axis_use (info, i,
                                       device->axes[i].use);

      info->n_keys = device->num_keys;
      info->keys   = g_renew (GdkDeviceKey, info->keys, info->n_keys);
      memset (info->keys, 0, info->n_keys * sizeof (GdkDeviceKey));

      for (i = 0; i < MIN (info->n_keys, device->num_keys); i++)
        picman_device_info_set_key (info, i,
                                  device->keys[i].keyval,
                                  device->keys[i].modifiers);
    }

  /*  sort order depends on device presence  */
  picman_object_name_changed (PICMAN_OBJECT (info));

  g_object_notify (G_OBJECT (info), "device");
  picman_device_info_changed (info);
}

void
picman_device_info_set_default_tool (PicmanDeviceInfo *info)
{
  g_return_if_fail (PICMAN_IS_DEVICE_INFO (info));

  if (info->device &&
      gdk_device_get_source (info->device) == GDK_SOURCE_ERASER)
    {
      PicmanContainer *tools = PICMAN_CONTEXT (info)->picman->tool_info_list;
      PicmanToolInfo  *eraser;

      eraser =
        PICMAN_TOOL_INFO (picman_container_get_child_by_name (tools,
                                                          "picman-eraser-tool"));

      if (eraser)
        picman_context_set_tool (PICMAN_CONTEXT (info), eraser);
    }
}

GdkInputMode
picman_device_info_get_mode (PicmanDeviceInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_INFO (info), GDK_MODE_DISABLED);

  if (info->device)
    return info->device->mode;
  else
    return info->mode;
}

void
picman_device_info_set_mode (PicmanDeviceInfo *info,
                           GdkInputMode    mode)
{
  g_return_if_fail (PICMAN_IS_DEVICE_INFO (info));

  if (mode != picman_device_info_get_mode (info))
    {
      if (info->device)
        gdk_device_set_mode (info->device, mode);
      else
        info->mode = mode;

      g_object_notify (G_OBJECT (info), "mode");
      picman_device_info_changed (info);
    }
}

gboolean
picman_device_info_has_cursor (PicmanDeviceInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_INFO (info), FALSE);

  if (info->device)
    return info->device->has_cursor;

  return FALSE;
}

gint
picman_device_info_get_n_axes (PicmanDeviceInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_INFO (info), 0);

  if (info->device)
    return info->device->num_axes;
  else
    return info->n_axes;
}

GdkAxisUse
picman_device_info_get_axis_use (PicmanDeviceInfo *info,
                               gint            axis)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_INFO (info), GDK_AXIS_IGNORE);
  g_return_val_if_fail (axis >= 0 && axis < picman_device_info_get_n_axes (info),
                        GDK_AXIS_IGNORE);

  if (info->device)
    return info->device->axes[axis].use;
  else
    return info->axes[axis];
}

void
picman_device_info_set_axis_use (PicmanDeviceInfo *info,
                               gint            axis,
                               GdkAxisUse      use)
{
  g_return_if_fail (PICMAN_IS_DEVICE_INFO (info));
  g_return_if_fail (axis >= 0 && axis < picman_device_info_get_n_axes (info));

  if (use != picman_device_info_get_axis_use (info, axis))
    {
      if (info->device)
        gdk_device_set_axis_use (info->device, axis, use);
      else
        info->axes[axis] = use;

      g_object_notify (G_OBJECT (info), "axes");
    }
}

gint
picman_device_info_get_n_keys (PicmanDeviceInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_INFO (info), 0);

  if (info->device)
    return info->device->num_keys;
  else
    return info->n_keys;
}

void
picman_device_info_get_key (PicmanDeviceInfo  *info,
                          gint             key,
                          guint           *keyval,
                          GdkModifierType *modifiers)
{
  g_return_if_fail (PICMAN_IS_DEVICE_INFO (info));
  g_return_if_fail (key >= 0 && key < picman_device_info_get_n_keys (info));
  g_return_if_fail (keyval != NULL);
  g_return_if_fail (modifiers != NULL);

  if (info->device)
    {
      *keyval    = info->device->keys[key].keyval;
      *modifiers = info->device->keys[key].modifiers;
    }
  else
    {
      *keyval    = info->keys[key].keyval;
      *modifiers = info->keys[key].modifiers;
    }
}

void
picman_device_info_set_key (PicmanDeviceInfo *info,
                          gint             key,
                          guint            keyval,
                          GdkModifierType  modifiers)
{
  guint           old_keyval;
  GdkModifierType old_modifiers;

  g_return_if_fail (PICMAN_IS_DEVICE_INFO (info));
  g_return_if_fail (key >= 0 && key < picman_device_info_get_n_keys (info));

  picman_device_info_get_key (info, key, &old_keyval, &old_modifiers);

  if (keyval    != old_keyval ||
      modifiers != old_modifiers)
    {
      if (info->device)
        {
          gdk_device_set_key (info->device, key, keyval, modifiers);
        }
      else
        {
          info->keys[key].keyval    = keyval;
          info->keys[key].modifiers = modifiers;
        }

      g_object_notify (G_OBJECT (info), "keys");
    }
}

PicmanCurve *
picman_device_info_get_curve (PicmanDeviceInfo *info,
                            GdkAxisUse      use)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_INFO (info), NULL);

  switch (use)
    {
    case GDK_AXIS_PRESSURE:
      return info->pressure_curve;
      break;

    default:
      return NULL;
    }
}

gdouble
picman_device_info_map_axis (PicmanDeviceInfo *info,
                           GdkAxisUse      use,
                           gdouble         value)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_INFO (info), value);

  /* CLAMP() the return values be safe against buggy XInput drivers */

  switch (use)
    {
    case GDK_AXIS_PRESSURE:
      return picman_curve_map_value (info->pressure_curve, value);

    case GDK_AXIS_XTILT:
      return CLAMP (value, PICMAN_COORDS_MIN_TILT, PICMAN_COORDS_MAX_TILT);

    case GDK_AXIS_YTILT:
      return CLAMP (value, PICMAN_COORDS_MIN_TILT, PICMAN_COORDS_MAX_TILT);

    case GDK_AXIS_WHEEL:
      return CLAMP (value, PICMAN_COORDS_MIN_WHEEL, PICMAN_COORDS_MAX_WHEEL);

    default:
      break;
    }

  return value;
}

void
picman_device_info_changed (PicmanDeviceInfo *info)
{
  g_return_if_fail (PICMAN_IS_DEVICE_INFO (info));

  g_signal_emit (info, device_info_signals[CHANGED], 0);
}

PicmanDeviceInfo *
picman_device_info_get_by_device (GdkDevice *device)
{
  g_return_val_if_fail (GDK_IS_DEVICE (device), NULL);

  return g_object_get_data (G_OBJECT (device), PICMAN_DEVICE_INFO_DATA_KEY);
}

gint
picman_device_info_compare (PicmanDeviceInfo *a,
                          PicmanDeviceInfo *b)
{
  if (a->device && a->display &&
      a->device == gdk_display_get_core_pointer (a->display))
    {
      return -1;
    }
  else if (b->device && b->display &&
           b->device == gdk_display_get_core_pointer (b->display))
    {
      return 1;
    }
  else if (a->device && ! b->device)
    {
      return -1;
    }
  else if (! a->device && b->device)
    {
      return 1;
    }
  else
    {
      return picman_object_name_collate ((PicmanObject *) a,
                                       (PicmanObject *) b);
    }
}
