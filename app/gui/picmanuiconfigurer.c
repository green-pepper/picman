/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanuiconfigurer.c
 * Copyright (C) 2009 Martin Nordholts <martinn@src.gnome.org>
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
#include <gtk/gtk.h>

#include "gui-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmandockcolumns.h"
#include "widgets/picmandockcontainer.h"
#include "widgets/picmandockwindow.h"
#include "widgets/picmantoolbox.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-appearance.h"
#include "display/picmanimagewindow.h"

#include "menus/menus.h"

#include "picmanuiconfigurer.h"


enum
{
  PROP_0,
  PROP_PICMAN
};


struct _PicmanUIConfigurerPrivate
{
  Picman *picman;
};


static void              picman_ui_configurer_set_property                (GObject           *object,
                                                                         guint              property_id,
                                                                         const GValue      *value,
                                                                         GParamSpec        *pspec);
static void              picman_ui_configurer_get_property                (GObject           *object,
                                                                         guint              property_id,
                                                                         GValue            *value,
                                                                         GParamSpec        *pspec);
static void              picman_ui_configurer_move_docks_to_columns       (PicmanUIConfigurer  *ui_configurer,
                                                                         PicmanImageWindow   *uber_image_window);
static void              picman_ui_configurer_move_shells                 (PicmanUIConfigurer  *ui_configurer,
                                                                         PicmanImageWindow   *source_image_window,
                                                                         PicmanImageWindow   *target_image_window);
static void              picman_ui_configurer_separate_docks              (PicmanUIConfigurer  *ui_configurer,
                                                                         PicmanImageWindow   *source_image_window);
static void              picman_ui_configurer_move_docks_to_window        (PicmanUIConfigurer  *ui_configurer,
                                                                         PicmanDockColumns   *dock_columns,
                                                                         PicmanAlignmentType  screen_side_destination);
static void              picman_ui_configurer_separate_shells             (PicmanUIConfigurer  *ui_configurer,
                                                                         PicmanImageWindow   *source_image_window);
static void              picman_ui_configurer_configure_for_single_window (PicmanUIConfigurer  *ui_configurer);
static void              picman_ui_configurer_configure_for_multi_window  (PicmanUIConfigurer  *ui_configurer);
static PicmanImageWindow * picman_ui_configurer_get_uber_window             (PicmanUIConfigurer  *ui_configurer);


G_DEFINE_TYPE (PicmanUIConfigurer, picman_ui_configurer, PICMAN_TYPE_OBJECT)

#define parent_class picman_ui_configurer_parent_class


static void
picman_ui_configurer_class_init (PicmanUIConfigurerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_ui_configurer_set_property;
  object_class->get_property = picman_ui_configurer_get_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman", NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_type_class_add_private (klass,
                            sizeof (PicmanUIConfigurerPrivate));
}

static void
picman_ui_configurer_init (PicmanUIConfigurer *ui_configurer)
{
  ui_configurer->p = G_TYPE_INSTANCE_GET_PRIVATE (ui_configurer,
                                                  PICMAN_TYPE_UI_CONFIGURER,
                                                  PicmanUIConfigurerPrivate);
}

static void
picman_ui_configurer_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanUIConfigurer *ui_configurer = PICMAN_UI_CONFIGURER (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      ui_configurer->p->picman = g_value_get_object (value); /* don't ref */
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_ui_configurer_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanUIConfigurer *ui_configurer = PICMAN_UI_CONFIGURER (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, ui_configurer->p->picman);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


static void
picman_ui_configurer_get_window_center_pos (GtkWindow *window,
                                          gint      *out_x,
                                          gint      *out_y)
{
  gint x, y, w, h;
  gtk_window_get_position (window, &x, &y);
  gtk_window_get_size (window, &w, &h);

  if (out_x)
    *out_x = x + w / 2;
  if (out_y)
    *out_y = y + h / 2;
}

/**
 * picman_ui_configurer_get_relative_window_pos:
 * @window_a:
 * @window_b:
 *
 * Returns: At what side @window_b is relative to @window_a. Either
 * PICMAN_ALIGN_LEFT or PICMAN_ALIGN_RIGHT.
 **/
static PicmanAlignmentType
picman_ui_configurer_get_relative_window_pos (GtkWindow *window_a,
                                            GtkWindow *window_b)
{
  gint a_x, b_x;

  picman_ui_configurer_get_window_center_pos (window_a, &a_x, NULL);
  picman_ui_configurer_get_window_center_pos (window_b, &b_x, NULL);

  return b_x < a_x ? PICMAN_ALIGN_LEFT : PICMAN_ALIGN_RIGHT;
}

static void
picman_ui_configurer_move_docks_to_columns (PicmanUIConfigurer *ui_configurer,
                                          PicmanImageWindow  *uber_image_window)
{
  GList *dialogs     = NULL;
  GList *dialog_iter = NULL;

  dialogs =
    g_list_copy (picman_dialog_factory_get_open_dialogs (picman_dialog_factory_get_singleton ()));

  for (dialog_iter = dialogs; dialog_iter; dialog_iter = dialog_iter->next)
    {
      PicmanDockWindow    *dock_window;
      PicmanDockContainer *dock_container;
      PicmanDockColumns   *dock_columns;
      GList             *docks;
      GList             *dock_iter;

      if (!PICMAN_IS_DOCK_WINDOW (dialog_iter->data))
        continue;

      dock_window = PICMAN_DOCK_WINDOW (dialog_iter->data);

      /* If the dock window is on the left side of the image window,
       * move the docks to the left side. If the dock window is on the
       * right side, move the docks to the right side of the image
       * window.
       */
      if (picman_ui_configurer_get_relative_window_pos (GTK_WINDOW (uber_image_window),
                                                      GTK_WINDOW (dock_window)) == PICMAN_ALIGN_LEFT)
        dock_columns = picman_image_window_get_left_docks (uber_image_window);
      else
        dock_columns = picman_image_window_get_right_docks (uber_image_window);

      dock_container = PICMAN_DOCK_CONTAINER (dock_window);
      g_object_add_weak_pointer (G_OBJECT (dock_window),
                                 (gpointer) &dock_window);

      docks = picman_dock_container_get_docks (dock_container);
      for (dock_iter = docks; dock_iter; dock_iter = dock_iter->next)
        {
          PicmanDock *dock = PICMAN_DOCK (dock_iter->data);

          /* Move the dock from the image window to the dock columns
           * widget. Note that we need a ref while the dock is parentless
           */
          g_object_ref (dock);
          picman_dock_window_remove_dock (dock_window, dock);
          picman_dock_columns_add_dock (dock_columns, dock, -1);
          g_object_unref (dock);
        }
      g_list_free (docks);

      if (dock_window)
        g_object_remove_weak_pointer (G_OBJECT (dock_window),
                                      (gpointer) &dock_window);

      /* Kill the window if removing the dock didn't destroy it
       * already. This will be the case for the toolbox dock window
       */
      if (GTK_IS_WIDGET (dock_window))
        {
          guint docks_len;

          docks     = picman_dock_container_get_docks (dock_container);
          docks_len = g_list_length (docks);

          if (docks_len == 0)
            {
              picman_dialog_factory_remove_dialog (picman_dialog_factory_get_singleton (),
                                                 GTK_WIDGET (dock_window));
              gtk_widget_destroy (GTK_WIDGET (dock_window));
            }

          g_list_free (docks);
        }
    }

  g_list_free (dialogs);
}

/**
 * picman_ui_configurer_move_shells:
 * @ui_configurer:
 * @source_image_window:
 * @target_image_window:
 *
 * Move all display shells from one image window to the another.
 **/
static void
picman_ui_configurer_move_shells (PicmanUIConfigurer  *ui_configurer,
                                PicmanImageWindow   *source_image_window,
                                PicmanImageWindow   *target_image_window)
{
  while (picman_image_window_get_n_shells (source_image_window) > 0)
    {
      PicmanDisplayShell *shell;

      shell = picman_image_window_get_shell (source_image_window, 0);

      g_object_ref (shell);
      picman_image_window_remove_shell (source_image_window, shell);
      picman_image_window_add_shell (target_image_window, shell);
      g_object_unref (shell);
    }
}

/**
 * picman_ui_configurer_separate_docks:
 * @ui_configurer:
 * @image_window:
 *
 * Move out the docks from the image window.
 **/
static void
picman_ui_configurer_separate_docks (PicmanUIConfigurer  *ui_configurer,
                                   PicmanImageWindow   *image_window)
{
  PicmanDockColumns *left_docks  = NULL;
  PicmanDockColumns *right_docks = NULL;

  left_docks  = picman_image_window_get_left_docks (image_window);
  right_docks = picman_image_window_get_right_docks (image_window);

  picman_ui_configurer_move_docks_to_window (ui_configurer, left_docks, PICMAN_ALIGN_LEFT);
  picman_ui_configurer_move_docks_to_window (ui_configurer, right_docks, PICMAN_ALIGN_RIGHT);
}

/**
 * picman_ui_configurer_move_docks_to_window:
 * @dock_columns:
 * @screen_side_destination: At what side of the screen the dock window
 *                           should be put.
 *
 * Moves docks in @dock_columns into a new #PicmanDockWindow and
 * position it on the screen in a non-overlapping manner.
 */
static void
picman_ui_configurer_move_docks_to_window (PicmanUIConfigurer  *ui_configurer,
                                         PicmanDockColumns   *dock_columns,
                                         PicmanAlignmentType  screen_side_destination)
{
  GdkScreen    *screen           = gtk_widget_get_screen (GTK_WIDGET (dock_columns));
  GList        *docks            = g_list_copy (picman_dock_columns_get_docks (dock_columns));
  GList        *iter             = NULL;
  gboolean      contains_toolbox = FALSE;
  GtkWidget    *dock_window      = NULL;
  GtkAllocation original_size    = { 0, 0, 0, 0 };

  /* Are there docks to move at all? */
  if (! docks)
    return;

  /* Remember the size so we can set the new dock window to the same
   * size
   */
  gtk_widget_get_allocation (GTK_WIDGET (dock_columns), &original_size);

  /* Do we need a toolbox window? */
  for (iter = docks; iter; iter = iter->next)
    {
      PicmanDock *dock = PICMAN_DOCK (iter->data);

      if (PICMAN_IS_TOOLBOX (dock))
        {
          contains_toolbox = TRUE;
          break;
        }
    }

  /* Create a dock window to put the dock in. Checking for
   * PICMAN_IS_TOOLBOX() is kind of ugly but not a disaster. We need
   * the dock window correctly configured if we create it for the
   * toolbox
   */
  dock_window =
    picman_dialog_factory_dialog_new (picman_dialog_factory_get_singleton (),
                                    screen,
                                    NULL /*ui_manager*/,
                                    (contains_toolbox ?
                                     "picman-toolbox-window" :
                                     "picman-dock-window"),
                                    -1 /*view_size*/,
                                    FALSE /*present*/);

  for (iter = docks; iter; iter = iter->next)
    {
      PicmanDock *dock = PICMAN_DOCK (iter->data);

      /* Move the dock to the window */
      g_object_ref (dock);
      picman_dock_columns_remove_dock (dock_columns, dock);
      picman_dock_window_add_dock (PICMAN_DOCK_WINDOW (dock_window), dock, -1);
      g_object_unref (dock);
    }

  /* Position the window */
  if (screen_side_destination == PICMAN_ALIGN_LEFT)
    gtk_window_parse_geometry (GTK_WINDOW (dock_window), "+0+0");
  else if (screen_side_destination == PICMAN_ALIGN_RIGHT)
    gtk_window_parse_geometry (GTK_WINDOW (dock_window), "-0+0");
  else
    g_assert_not_reached ();

  /* Try to keep the same size */
  gtk_window_set_default_size (GTK_WINDOW (dock_window),
                               original_size.width,
                               original_size.height);

  /* Don't forget to show the window */
  gtk_widget_show (dock_window);

  g_list_free (docks);
}

/**
 * picman_ui_configurer_separate_shells:
 * @ui_configurer:
 * @source_image_window:
 *
 * Create one image window per display shell and move it there.
 **/
static void
picman_ui_configurer_separate_shells (PicmanUIConfigurer *ui_configurer,
                                    PicmanImageWindow  *source_image_window)
{
  /* The last display shell remains in its window */
  while (picman_image_window_get_n_shells (source_image_window) > 1)
    {
      PicmanImageWindow  *new_image_window;
      PicmanDisplayShell *shell;

      /* Create a new image window */
      new_image_window = picman_image_window_new (ui_configurer->p->picman,
                                                NULL,
                                                global_menu_factory,
                                                picman_dialog_factory_get_singleton ());
      /* Move the shell there */
      shell = picman_image_window_get_shell (source_image_window, 1);

      g_object_ref (shell);
      picman_image_window_remove_shell (source_image_window, shell);
      picman_image_window_add_shell (new_image_window, shell);
      g_object_unref (shell);

      /* FIXME: If we don't set a size request here the window will be
       * too small. Get rid of this hack and fix it the proper way
       */
      gtk_widget_set_size_request (GTK_WIDGET (new_image_window), 640, 480);

      /* Show after we have added the shell */
      gtk_widget_show (GTK_WIDGET (new_image_window));
    }
}

/**
 * picman_ui_configurer_configure_for_single_window:
 * @ui_configurer:
 *
 * Move docks and display shells into a single window.
 **/
static void
picman_ui_configurer_configure_for_single_window (PicmanUIConfigurer *ui_configurer)
{
  Picman            *picman              = ui_configurer->p->picman;
  GList           *windows           = picman_get_image_windows (picman);
  GList           *iter              = NULL;
  PicmanImageWindow *uber_image_window = NULL;

  /* Get and setup the window to put everything in */
  uber_image_window = picman_ui_configurer_get_uber_window (ui_configurer);

  /* Mve docks to the left and right side of the image window */
  picman_ui_configurer_move_docks_to_columns (ui_configurer,
                                            uber_image_window);

  /* Move image shells from other windows to the uber image window */
  for (iter = windows; iter; iter = g_list_next (iter))
    {
      PicmanImageWindow *image_window = PICMAN_IMAGE_WINDOW (iter->data);

      /* Don't move stuff to itself */
      if (image_window == uber_image_window)
        continue;

      /* Put the displays in the rest of the image windows into
       * the uber image window
       */
      picman_ui_configurer_move_shells (ui_configurer,
                                      image_window,
                                      uber_image_window);

      /* Destroy the window */
      picman_image_window_destroy (image_window);
    }

  g_list_free (windows);
}

/**
 * picman_ui_configurer_configure_for_multi_window:
 * @ui_configurer:
 *
 * Moves all display shells into their own image window.
 **/
static void
picman_ui_configurer_configure_for_multi_window (PicmanUIConfigurer *ui_configurer)
{
  Picman  *picman    = ui_configurer->p->picman;
  GList *windows = picman_get_image_windows (picman);
  GList *iter    = NULL;

  for (iter = windows; iter; iter = g_list_next (iter))
    {
      PicmanImageWindow *image_window = PICMAN_IMAGE_WINDOW (iter->data);

      picman_ui_configurer_separate_docks (ui_configurer, image_window);

      picman_ui_configurer_separate_shells (ui_configurer, image_window);
    }

  g_list_free (windows);
}

/**
 * picman_ui_configurer_get_uber_window:
 * @ui_configurer:
 *
 * Returns: The window to be used as the main window for single-window
 *          mode.
 **/
static PicmanImageWindow *
picman_ui_configurer_get_uber_window (PicmanUIConfigurer *ui_configurer)
{
  Picman             *picman         = ui_configurer->p->picman;
  PicmanDisplay      *display      = picman_get_display_iter (picman)->data;
  PicmanDisplayShell *shell        = picman_display_get_shell (display);
  PicmanImageWindow  *image_window = picman_display_shell_get_window (shell);

  return image_window;
}

/**
 * picman_ui_configurer_update_appearance:
 * @ui_configurer:
 *
 * Updates the appearance of all shells in all image windows, so they
 * do whatever they deem necessary to fit the new UI mode mode.
 **/
static void
picman_ui_configurer_update_appearance (PicmanUIConfigurer *ui_configurer)
{
  Picman  *picman    = ui_configurer->p->picman;
  GList *windows = picman_get_image_windows (picman);
  GList *list;

  for (list = windows; list; list = g_list_next (list))
    {
      PicmanImageWindow *image_window = PICMAN_IMAGE_WINDOW (list->data);
      gint             n_shells;
      gint             i;

      n_shells = picman_image_window_get_n_shells (image_window);

      for (i = 0; i < n_shells; i++)
        {
          PicmanDisplayShell *shell;

          shell = picman_image_window_get_shell (image_window, i);

          picman_display_shell_appearance_update (shell);
        }
    }

  g_list_free (windows);
}

/**
 * picman_ui_configurer_configure:
 * @ui_configurer:
 * @single_window_mode:
 *
 * Configure the UI.
 **/
void
picman_ui_configurer_configure (PicmanUIConfigurer *ui_configurer,
                              gboolean          single_window_mode)
{
  if (single_window_mode)
    picman_ui_configurer_configure_for_single_window (ui_configurer);
  else
    picman_ui_configurer_configure_for_multi_window (ui_configurer);

  picman_ui_configurer_update_appearance (ui_configurer);
}
