/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpatternmenu.c
 * Copyright (C) 1998 Andy Thomas
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
#include "picmanpatternmenu.h"
#include "picmanpatternselectbutton.h"


/**
 * SECTION: picmanpatternmenu
 * @title: picmanpatternmenu
 * @short_description: A widget for selecting patterns.
 *
 * A widget for selecting patterns.
 **/


typedef struct
{
  PicmanRunPatternCallback callback;
  gpointer               data;
} CompatCallbackData;


static void compat_callback           (PicmanPatternSelectButton *pattern_button,
                                       const gchar             *pattern_name,
                                       gint                     width,
                                       gint                     height,
                                       gint                     bytes,
                                       const guchar            *mask_data,
                                       gboolean                 dialog_closing,
                                       CompatCallbackData      *data);
static void compat_callback_data_free (CompatCallbackData      *data);


/**
 * picman_pattern_select_widget_new:
 * @title:        Title of the dialog to use or %NULL to use the default title.
 * @pattern_name: Initial pattern name or %NULL to use current selection.
 * @callback:     A function to call when the selected pattern changes.
 * @data:         A pointer to arbitrary data to be used in the call to @callback.
 *
 * Creates a new #GtkWidget that completely controls the selection of
 * a pattern. This widget is suitable for placement in a table in a
 * plug-in dialog.
 *
 * Returns: A #GtkWidget that you can use in your UI.
 */
GtkWidget *
picman_pattern_select_widget_new (const gchar            *title,
                                const gchar            *pattern_name,
                                PicmanRunPatternCallback  callback,
                                gpointer                data)
{
  GtkWidget          *pattern_button;
  CompatCallbackData *compat_data;

  g_return_val_if_fail (callback != NULL, NULL);

  pattern_button = picman_pattern_select_button_new (title, pattern_name);

  compat_data = g_slice_new (CompatCallbackData);

  compat_data->callback = callback;
  compat_data->data     = data;

  g_signal_connect_data (pattern_button, "pattern-set",
                         G_CALLBACK (compat_callback),
                         compat_data,
                         (GClosureNotify) compat_callback_data_free, 0);

  return pattern_button;
}

/**
 * picman_pattern_select_widget_close:
 * @widget: A pattern select widget.
 *
 * Closes the popup window associated with @widget.
 */
void
picman_pattern_select_widget_close (GtkWidget *widget)
{
  g_return_if_fail (widget != NULL);

  picman_select_button_close_popup (PICMAN_SELECT_BUTTON (widget));
}

/**
 * picman_pattern_select_widget_set:
 * @widget:       A pattern select widget.
 * @pattern_name: Pattern name to set. NULL means no change.
 *
 * Sets the current pattern for the pattern select widget.  Calls the
 * callback function if one was supplied in the call to
 * picman_pattern_select_widget_new().
 */
void
picman_pattern_select_widget_set (GtkWidget   *widget,
                                const gchar *pattern_name)
{
  g_return_if_fail (widget != NULL);

  picman_pattern_select_button_set_pattern (PICMAN_PATTERN_SELECT_BUTTON (widget),
                                          pattern_name);
}


static void
compat_callback (PicmanPatternSelectButton *pattern_button,
                 const gchar             *pattern_name,
                 gint                     width,
                 gint                     height,
                 gint                     bpp,
                 const guchar            *mask_data,
                 gboolean                 dialog_closing,
                 CompatCallbackData      *data)
{
  data->callback (pattern_name, width, height, bpp, mask_data,
                  dialog_closing, data->data);
}

static void
compat_callback_data_free (CompatCallbackData *data)
{
  g_slice_free (CompatCallbackData, data);
}
