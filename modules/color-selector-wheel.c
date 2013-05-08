/* PICMAN Wheel ColorSelector
 * Copyright (C) 2008  Michael Natterer <mitch@picman.org>
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

#include <stdlib.h>
#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanmodule/picmanmodule.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "picmancolorwheel.h"

#include "libpicman/libpicman-intl.h"


#define COLORSEL_TYPE_WHEEL            (colorsel_wheel_get_type ())
#define COLORSEL_WHEEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), COLORSEL_TYPE_WHEEL, ColorselWheel))
#define COLORSEL_WHEEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), COLORSEL_TYPE_WHEEL, ColorselWheelClass))
#define COLORSEL_IS_WHEEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), COLORSEL_TYPE_WHEEL))
#define COLORSEL_IS_WHEEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), COLORSEL_TYPE_WHEEL))


typedef struct _ColorselWheel      ColorselWheel;
typedef struct _ColorselWheelClass ColorselWheelClass;

struct _ColorselWheel
{
  PicmanColorSelector  parent_instance;

  GtkWidget         *hsv;
};

struct _ColorselWheelClass
{
  PicmanColorSelectorClass  parent_class;
};


static GType  colorsel_wheel_get_type      (void);

static void   colorsel_wheel_set_color     (PicmanColorSelector *selector,
                                            const PicmanRGB     *rgb,
                                            const PicmanHSV     *hsv);
static void   colorsel_wheel_changed       (PicmanColorWheel    *hsv,
                                            PicmanColorSelector *selector);

static const PicmanModuleInfo colorsel_wheel_info =
{
  PICMAN_MODULE_ABI_VERSION,
  N_("HSV color wheel"),
  "Michael Natterer <mitch@picman.org>",
  "v1.0",
  "(c) 2008, released under the GPL",
  "08 Aug 2008"
};


G_DEFINE_DYNAMIC_TYPE (ColorselWheel, colorsel_wheel,
                       PICMAN_TYPE_COLOR_SELECTOR)


G_MODULE_EXPORT const PicmanModuleInfo *
picman_module_query (GTypeModule *module)
{
  return &colorsel_wheel_info;
}

G_MODULE_EXPORT gboolean
picman_module_register (GTypeModule *module)
{
  colorsel_wheel_register_type (module);

  return TRUE;
}

static void
colorsel_wheel_class_init (ColorselWheelClass *klass)
{
  PicmanColorSelectorClass *selector_class = PICMAN_COLOR_SELECTOR_CLASS (klass);

  selector_class->name      = _("Wheel");
  selector_class->help_id   = "picman-colorselector-triangle";
  selector_class->stock_id  = PICMAN_STOCK_COLOR_TRIANGLE;
  selector_class->set_color = colorsel_wheel_set_color;
}

static void
colorsel_wheel_class_finalize (ColorselWheelClass *klass)
{
}

static void
colorsel_wheel_init (ColorselWheel *wheel)
{
  GtkWidget *frame;

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (wheel), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  wheel->hsv = picman_color_wheel_new ();
  gtk_container_add (GTK_CONTAINER (frame), wheel->hsv);
  gtk_widget_show (wheel->hsv);

  g_signal_connect (wheel->hsv, "changed",
                    G_CALLBACK (colorsel_wheel_changed),
                    wheel);
}

static void
colorsel_wheel_set_color (PicmanColorSelector *selector,
                          const PicmanRGB     *rgb,
                          const PicmanHSV     *hsv)
{
  ColorselWheel *wheel = COLORSEL_WHEEL (selector);

  picman_color_wheel_set_color (PICMAN_COLOR_WHEEL (wheel->hsv),
                              hsv->h, hsv->s, hsv->v);
}

static void
colorsel_wheel_changed (PicmanColorWheel    *hsv,
                        PicmanColorSelector *selector)
{
  picman_color_wheel_get_color (hsv,
                              &selector->hsv.h,
                              &selector->hsv.s,
                              &selector->hsv.v);
  picman_hsv_to_rgb (&selector->hsv, &selector->rgb);

  picman_color_selector_color_changed (selector);
}
