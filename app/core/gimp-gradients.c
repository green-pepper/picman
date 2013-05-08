/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2002 Spencer Kimball, Peter Mattis, and others
 *
 * picman-gradients.c
 * Copyright (C) 2002 Michael Natterer  <mitch@picman.org>
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

#include <gegl.h>

#include "core-types.h"

#include "picman.h"
#include "picman-gradients.h"
#include "picmancontext.h"
#include "picmancontainer.h"
#include "picmandatafactory.h"
#include "picmangradient.h"

#include "picman-intl.h"


#define FG_BG_RGB_KEY      "picman-gradient-fg-bg-rgb"
#define FG_BG_HARDEDGE_KEY "picman-gradient-fg-bg-rgb"
#define FG_BG_HSV_CCW_KEY  "picman-gradient-fg-bg-hsv-ccw"
#define FG_BG_HSV_CW_KEY   "picman-gradient-fg-bg-hsv-cw"
#define FG_TRANSPARENT_KEY "picman-gradient-fg-transparent"


/*  local function prototypes  */

static PicmanGradient * picman_gradients_add_gradient (Picman        *picman,
                                                   const gchar *name,
                                                   const gchar *id);


/*  public functions  */

void
picman_gradients_init (Picman *picman)
{
  PicmanGradient *gradient;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  /* FG to BG (RGB) */
  gradient = picman_gradients_add_gradient (picman,
                                          _("FG to BG (RGB)"),
                                          FG_BG_RGB_KEY);
  gradient->segments->left_color_type  = PICMAN_GRADIENT_COLOR_FOREGROUND;
  gradient->segments->right_color_type = PICMAN_GRADIENT_COLOR_BACKGROUND;
  picman_context_set_gradient (picman->user_context, gradient);

  /* FG to BG (Hardedge) */
  gradient = picman_gradients_add_gradient (picman,
                                          _("FG to BG (Hardedge)"),
                                          FG_BG_HARDEDGE_KEY);
  gradient->segments->left                   = 0.00;
  gradient->segments->middle                 = 0.25;
  gradient->segments->right                  = 0.50;
  gradient->segments->left_color_type        = PICMAN_GRADIENT_COLOR_FOREGROUND;
  gradient->segments->right_color_type       = PICMAN_GRADIENT_COLOR_FOREGROUND;
  gradient->segments->next                   = picman_gradient_segment_new ();
  gradient->segments->next->prev             = gradient->segments;
  gradient->segments->next->left             = 0.50;
  gradient->segments->next->middle           = 0.75;
  gradient->segments->next->right            = 1.00;
  gradient->segments->next->left_color_type  = PICMAN_GRADIENT_COLOR_BACKGROUND;
  gradient->segments->next->right_color_type = PICMAN_GRADIENT_COLOR_BACKGROUND;

  /* FG to BG (HSV counter-clockwise) */
  gradient = picman_gradients_add_gradient (picman,
                                          _("FG to BG (HSV counter-clockwise)"),
                                          FG_BG_HSV_CCW_KEY);
  gradient->segments->left_color_type  = PICMAN_GRADIENT_COLOR_FOREGROUND;
  gradient->segments->right_color_type = PICMAN_GRADIENT_COLOR_BACKGROUND;
  gradient->segments->color            = PICMAN_GRADIENT_SEGMENT_HSV_CCW;

  /* FG to BG (HSV clockwise hue) */
  gradient = picman_gradients_add_gradient (picman,
                                          _("FG to BG (HSV clockwise hue)"),
                                          FG_BG_HSV_CW_KEY);
  gradient->segments->left_color_type  = PICMAN_GRADIENT_COLOR_FOREGROUND;
  gradient->segments->right_color_type = PICMAN_GRADIENT_COLOR_BACKGROUND;
  gradient->segments->color            = PICMAN_GRADIENT_SEGMENT_HSV_CW;

  /* FG to Transparent */
  gradient = picman_gradients_add_gradient (picman,
                                          _("FG to Transparent"),
                                          FG_TRANSPARENT_KEY);
  gradient->segments->left_color_type  = PICMAN_GRADIENT_COLOR_FOREGROUND;
  gradient->segments->right_color_type = PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT;
}


/*  private functions  */

static PicmanGradient *
picman_gradients_add_gradient (Picman        *picman,
                             const gchar *name,
                             const gchar *id)
{
  PicmanGradient *gradient;

  gradient = PICMAN_GRADIENT (picman_gradient_new (picman_get_user_context (picman),
                                               name));

  picman_data_make_internal (PICMAN_DATA (gradient), id);

  picman_container_add (picman_data_factory_get_container (picman->gradient_factory),
                      PICMAN_OBJECT (gradient));
  g_object_unref (gradient);

  g_object_set_data (G_OBJECT (picman), id, gradient);

  return gradient;
}
