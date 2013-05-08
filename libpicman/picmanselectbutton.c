/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanselectbutton.c
 * Copyright (C) 2003  Sven Neumann  <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "picman.h"

#include "picmanuitypes.h"
#include "picmanselectbutton.h"


/**
 * SECTION: picmanselectbutton
 * @title: PicmanSelectButton
 * @short_description: The base class of the data select buttons.
 *
 * The base class of the brush, pattern, gradient, palette and font
 * select buttons.
 **/


/*  local function prototypes  */

static void   picman_select_button_dispose (GObject *object);


G_DEFINE_TYPE (PicmanSelectButton, picman_select_button, GTK_TYPE_BOX)

static void
picman_select_button_class_init (PicmanSelectButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = picman_select_button_dispose;
}

static void
picman_select_button_init (PicmanSelectButton *select_button)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (select_button),
                                  GTK_ORIENTATION_HORIZONTAL);

  select_button->temp_callback = NULL;
}

static void
picman_select_button_dispose (GObject *object)
{
  picman_select_button_close_popup (PICMAN_SELECT_BUTTON (object));

  G_OBJECT_CLASS (picman_select_button_parent_class)->dispose (object);
}

/**
 * picman_select_button_close_popup:
 * @button: A #PicmanSelectButton
 *
 * Closes the popup window associated with @button.
 *
 * Since: PICMAN 2.4
 */
void
picman_select_button_close_popup (PicmanSelectButton *button)
{
  g_return_if_fail (PICMAN_IS_SELECT_BUTTON (button));

  if (button->temp_callback)
    {
      PicmanSelectButtonClass *klass = PICMAN_SELECT_BUTTON_GET_CLASS (button);

      klass->select_destroy (button->temp_callback);

      button->temp_callback = NULL;
    }
}
