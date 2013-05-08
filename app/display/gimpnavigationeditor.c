/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmannavigationeditor.c
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
 *
 * partly based on app/nav_window
 * Copyright (C) 1999 Andy Thomas <alt@picman.org>
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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmandisplayconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"

#include "widgets/picmandocked.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmenufactory.h"
#include "widgets/picmannavigationview.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanviewrenderer.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-scale.h"
#include "picmandisplayshell-scroll.h"
#include "picmannavigationeditor.h"

#include "picman-intl.h"


#define UPDATE_DELAY 300 /* From GtkRange in GTK+ 2.22 */


static void        picman_navigation_editor_docked_iface_init (PicmanDockedInterface  *iface);

static void        picman_navigation_editor_dispose           (GObject              *object);

static void        picman_navigation_editor_set_context       (PicmanDocked           *docked,
                                                             PicmanContext          *context);

static GtkWidget * picman_navigation_editor_new_private       (PicmanMenuFactory      *menu_factory,
                                                             PicmanDisplayShell     *shell);

static void        picman_navigation_editor_set_shell         (PicmanNavigationEditor *editor,
                                                             PicmanDisplayShell     *shell);
static gboolean    picman_navigation_editor_button_release    (GtkWidget            *widget,
                                                             GdkEventButton       *bevent,
                                                             PicmanDisplayShell     *shell);
static void        picman_navigation_editor_marker_changed    (PicmanNavigationView   *view,
                                                             gdouble               x,
                                                             gdouble               y,
                                                             gdouble               width,
                                                             gdouble               height,
                                                             PicmanNavigationEditor *editor);
static void        picman_navigation_editor_zoom              (PicmanNavigationView   *view,
                                                             PicmanZoomType          direction,
                                                             PicmanNavigationEditor *editor);
static void        picman_navigation_editor_scroll            (PicmanNavigationView   *view,
                                                             GdkScrollDirection    direction,
                                                             PicmanNavigationEditor *editor);

static void        picman_navigation_editor_zoom_adj_changed  (GtkAdjustment        *adj,
                                                             PicmanNavigationEditor *editor);

static void        picman_navigation_editor_shell_scaled      (PicmanDisplayShell     *shell,
                                                             PicmanNavigationEditor *editor);
static void        picman_navigation_editor_shell_scrolled    (PicmanDisplayShell     *shell,
                                                             PicmanNavigationEditor *editor);
static void        picman_navigation_editor_shell_reconnect   (PicmanDisplayShell     *shell,
                                                             PicmanNavigationEditor *editor);
static void        picman_navigation_editor_update_marker     (PicmanNavigationEditor *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanNavigationEditor, picman_navigation_editor,
                         PICMAN_TYPE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_navigation_editor_docked_iface_init))

#define parent_class picman_navigation_editor_parent_class


static void
picman_navigation_editor_class_init (PicmanNavigationEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = picman_navigation_editor_dispose;
}

static void
picman_navigation_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  iface->set_context = picman_navigation_editor_set_context;
}

static void
picman_navigation_editor_init (PicmanNavigationEditor *editor)
{
  GtkWidget *frame;

  editor->context       = NULL;
  editor->shell         = NULL;
  editor->scale_timeout = 0;

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (editor), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  editor->view = picman_view_new_by_types (NULL,
                                         PICMAN_TYPE_NAVIGATION_VIEW,
                                         PICMAN_TYPE_IMAGE,
                                         PICMAN_VIEW_SIZE_MEDIUM, 0, TRUE);
  gtk_container_add (GTK_CONTAINER (frame), editor->view);
  gtk_widget_show (editor->view);

  g_signal_connect (editor->view, "marker-changed",
                    G_CALLBACK (picman_navigation_editor_marker_changed),
                    editor);
  g_signal_connect (editor->view, "zoom",
                    G_CALLBACK (picman_navigation_editor_zoom),
                    editor);
  g_signal_connect (editor->view, "scroll",
                    G_CALLBACK (picman_navigation_editor_scroll),
                    editor);

  gtk_widget_set_sensitive (GTK_WIDGET (editor), FALSE);
}

static void
picman_navigation_editor_dispose (GObject *object)
{
  PicmanNavigationEditor *editor = PICMAN_NAVIGATION_EDITOR (object);

  if (editor->shell)
    picman_navigation_editor_set_shell (editor, NULL);

  if (editor->scale_timeout)
    {
      g_source_remove (editor->scale_timeout);
      editor->scale_timeout = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_navigation_editor_display_changed (PicmanContext          *context,
                                        PicmanDisplay          *display,
                                        PicmanNavigationEditor *editor)
{
  PicmanDisplayShell *shell = NULL;

  if (display)
    shell = picman_display_get_shell (display);

  picman_navigation_editor_set_shell (editor, shell);
}

static void
picman_navigation_editor_set_context (PicmanDocked  *docked,
                                    PicmanContext *context)
{
  PicmanNavigationEditor *editor  = PICMAN_NAVIGATION_EDITOR (docked);
  PicmanDisplay          *display = NULL;

  if (editor->context)
    {
      g_signal_handlers_disconnect_by_func (editor->context,
                                            picman_navigation_editor_display_changed,
                                            editor);
    }

  editor->context = context;

  if (editor->context)
    {
      g_signal_connect (context, "display-changed",
                        G_CALLBACK (picman_navigation_editor_display_changed),
                        editor);

      display = picman_context_get_display (context);
    }

  picman_view_renderer_set_context (PICMAN_VIEW (editor->view)->renderer,
                                  context);

  picman_navigation_editor_display_changed (editor->context,
                                          display,
                                          editor);
}


/*  public functions  */

GtkWidget *
picman_navigation_editor_new (PicmanMenuFactory *menu_factory)
{
  return picman_navigation_editor_new_private (menu_factory, NULL);
}

void
picman_navigation_editor_popup (PicmanDisplayShell *shell,
                              GtkWidget        *widget,
                              gint              click_x,
                              gint              click_y)
{
  GtkStyle             *style = gtk_widget_get_style (widget);
  PicmanNavigationEditor *editor;
  PicmanNavigationView   *view;
  GdkScreen            *screen;
  gint                  x, y;
  gint                  view_marker_x, view_marker_y;
  gint                  view_marker_width, view_marker_height;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (! shell->nav_popup)
    {
      GtkWidget *frame;

      shell->nav_popup = gtk_window_new (GTK_WINDOW_POPUP);

      frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
      gtk_container_add (GTK_CONTAINER (shell->nav_popup), frame);
      gtk_widget_show (frame);

      editor =
        PICMAN_NAVIGATION_EDITOR (picman_navigation_editor_new_private (NULL,
                                                                    shell));
      gtk_container_add (GTK_CONTAINER (frame), GTK_WIDGET (editor));
      gtk_widget_show (GTK_WIDGET (editor));

      g_signal_connect (editor->view, "button-release-event",
                        G_CALLBACK (picman_navigation_editor_button_release),
                        shell);
    }
  else
    {
      GtkWidget *bin = gtk_bin_get_child (GTK_BIN (shell->nav_popup));

      editor = PICMAN_NAVIGATION_EDITOR (gtk_bin_get_child (GTK_BIN (bin)));
    }

  view = PICMAN_NAVIGATION_VIEW (editor->view);

  /* Set poup screen */
  screen = gtk_widget_get_screen (widget);
  gtk_window_set_screen (GTK_WINDOW (shell->nav_popup), screen);

  picman_navigation_view_get_local_marker (view,
                                         &view_marker_x,
                                         &view_marker_y,
                                         &view_marker_width,
                                         &view_marker_height);
  /* Position the popup */
  {
    gint x_origin, y_origin;
    gint popup_width, popup_height;
    gint border_width, border_height;
    gint screen_click_x, screen_click_y;

    gdk_window_get_origin (gtk_widget_get_window (widget),
                           &x_origin, &y_origin);

    screen_click_x = x_origin + click_x;
    screen_click_y = y_origin + click_y;
    border_width   = 2 * style->xthickness;
    border_height  = 2 * style->ythickness;
    popup_width    = PICMAN_VIEW (view)->renderer->width  - 2 * border_width;
    popup_height   = PICMAN_VIEW (view)->renderer->height - 2 * border_height;

    x = screen_click_x -
        border_width -
        view_marker_x -
        view_marker_width / 2;

    y = screen_click_y -
        border_height -
        view_marker_y -
        view_marker_height / 2;

    /* When the image is zoomed out and overscrolled, the above
     * calculation risks positioning the popup far far away from the
     * click coordinate. We don't want that, so perform some clamping.
     */
    x = CLAMP (x, screen_click_x - popup_width,  screen_click_x);
    y = CLAMP (y, screen_click_y - popup_height, screen_click_y);

    /* If the popup doesn't fit into the screen, we have a problem.
     * We move the popup onscreen and risk that the pointer is not
     * in the square representing the viewable area anymore. Moving
     * the pointer will make the image scroll by a large amount,
     * but then it works as usual. Probably better than a popup that
     * is completely unusable in the lower right of the screen.
     *
     * Warping the pointer would be another solution ...
     */
    x = CLAMP (x, 0, gdk_screen_get_width (screen)  - popup_width);
    y = CLAMP (y, 0, gdk_screen_get_height (screen) - popup_height);

    gtk_window_move (GTK_WINDOW (shell->nav_popup), x, y);
  }

  gtk_widget_show (shell->nav_popup);
  gdk_flush ();

  /* fill in then grab pointer */
  picman_navigation_view_set_motion_offset (view,
                                          view_marker_width  / 2,
                                          view_marker_height / 2);
  picman_navigation_view_grab_pointer (view);
}


/*  private functions  */

static GtkWidget *
picman_navigation_editor_new_private (PicmanMenuFactory  *menu_factory,
                                    PicmanDisplayShell *shell)
{
  PicmanNavigationEditor *editor;

  g_return_val_if_fail (menu_factory == NULL ||
                        PICMAN_IS_MENU_FACTORY (menu_factory), NULL);
  g_return_val_if_fail (shell == NULL || PICMAN_IS_DISPLAY_SHELL (shell), NULL);
  g_return_val_if_fail (menu_factory || shell, NULL);

  if (shell)
    {
      Picman              *picman   = shell->display->picman;
      PicmanDisplayConfig *config = shell->display->config;
      PicmanView          *view;

      editor = g_object_new (PICMAN_TYPE_NAVIGATION_EDITOR, NULL);

      view = PICMAN_VIEW (editor->view);

      picman_view_renderer_set_size (view->renderer,
                                   config->nav_preview_size * 3,
                                   view->renderer->border_width);
      picman_view_renderer_set_context (view->renderer,
                                      picman_get_user_context (picman));

      picman_navigation_editor_set_shell (editor, shell);

    }
  else
    {
      GtkWidget *hscale;
      GtkWidget *hbox;

      editor = g_object_new (PICMAN_TYPE_NAVIGATION_EDITOR,
                             "menu-factory",    menu_factory,
                             "menu-identifier", "<NavigationEditor>",
                             NULL);

      gtk_widget_set_size_request (editor->view,
                                   PICMAN_VIEW_SIZE_HUGE,
                                   PICMAN_VIEW_SIZE_HUGE);
      picman_view_set_expand (PICMAN_VIEW (editor->view), TRUE);

      /* the editor buttons */

      editor->zoom_out_button =
        picman_editor_add_action_button (PICMAN_EDITOR (editor), "view",
                                       "view-zoom-out", NULL);

      editor->zoom_in_button =
        picman_editor_add_action_button (PICMAN_EDITOR (editor), "view",
                                       "view-zoom-in", NULL);

      editor->zoom_100_button =
        picman_editor_add_action_button (PICMAN_EDITOR (editor), "view",
                                       "view-zoom-1-1", NULL);

      editor->zoom_fit_in_button =
        picman_editor_add_action_button (PICMAN_EDITOR (editor), "view",
                                       "view-zoom-fit-in", NULL);

      editor->zoom_fill_button =
        picman_editor_add_action_button (PICMAN_EDITOR (editor), "view",
                                       "view-zoom-fill", NULL);

      editor->shrink_wrap_button =
        picman_editor_add_action_button (PICMAN_EDITOR (editor), "view",
                                       "view-shrink-wrap", NULL);

      /* the zoom scale */

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
      gtk_box_pack_end (GTK_BOX (editor), hbox, FALSE, FALSE, 0);
      gtk_widget_show (hbox);

      editor->zoom_adjustment =
        GTK_ADJUSTMENT (gtk_adjustment_new (0.0, -8.0, 8.0, 0.5, 1.0, 0.0));

      g_signal_connect (editor->zoom_adjustment, "value-changed",
                        G_CALLBACK (picman_navigation_editor_zoom_adj_changed),
                        editor);

      hscale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                              editor->zoom_adjustment);
      gtk_scale_set_draw_value (GTK_SCALE (hscale), FALSE);
      gtk_box_pack_start (GTK_BOX (hbox), hscale, TRUE, TRUE, 0);
      gtk_widget_show (hscale);

      /* the zoom label */

      editor->zoom_label = gtk_label_new ("100%");
      gtk_label_set_width_chars (GTK_LABEL (editor->zoom_label), 7);
      gtk_box_pack_start (GTK_BOX (hbox), editor->zoom_label, FALSE, FALSE, 0);
      gtk_widget_show (editor->zoom_label);
    }

  picman_view_renderer_set_background (PICMAN_VIEW (editor->view)->renderer,
                                     PICMAN_STOCK_TEXTURE);

  return GTK_WIDGET (editor);
}

static void
picman_navigation_editor_set_shell (PicmanNavigationEditor *editor,
                                  PicmanDisplayShell     *shell)
{
  g_return_if_fail (PICMAN_IS_NAVIGATION_EDITOR (editor));
  g_return_if_fail (! shell || PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell == editor->shell)
    return;

  if (editor->shell)
    {
      g_signal_handlers_disconnect_by_func (editor->shell,
                                            picman_navigation_editor_shell_scaled,
                                            editor);
      g_signal_handlers_disconnect_by_func (editor->shell,
                                            picman_navigation_editor_shell_scrolled,
                                            editor);
      g_signal_handlers_disconnect_by_func (editor->shell,
                                            picman_navigation_editor_shell_reconnect,
                                            editor);
    }
  else if (shell)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (editor), TRUE);
    }

  editor->shell = shell;

  if (editor->shell)
    {
      PicmanImage *image = picman_display_get_image (shell->display);

      picman_view_set_viewable (PICMAN_VIEW (editor->view),
                              PICMAN_VIEWABLE (image));

      g_signal_connect (editor->shell, "scaled",
                        G_CALLBACK (picman_navigation_editor_shell_scaled),
                        editor);
      g_signal_connect (editor->shell, "scrolled",
                        G_CALLBACK (picman_navigation_editor_shell_scrolled),
                        editor);
      g_signal_connect (editor->shell, "reconnect",
                        G_CALLBACK (picman_navigation_editor_shell_reconnect),
                        editor);

      picman_navigation_editor_shell_scaled (editor->shell, editor);
    }
  else
    {
      picman_view_set_viewable (PICMAN_VIEW (editor->view), NULL);
      gtk_widget_set_sensitive (GTK_WIDGET (editor), FALSE);
    }

  if (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)))
    picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                            picman_editor_get_popup_data (PICMAN_EDITOR (editor)));
}

static gboolean
picman_navigation_editor_button_release (GtkWidget        *widget,
                                       GdkEventButton   *bevent,
                                       PicmanDisplayShell *shell)
{
  if (bevent->button == 1)
    {
      gtk_widget_hide (shell->nav_popup);
    }

  return FALSE;
}

static void
picman_navigation_editor_marker_changed (PicmanNavigationView   *view,
                                       gdouble               x,
                                       gdouble               y,
                                       gdouble               width,
                                       gdouble               height,
                                       PicmanNavigationEditor *editor)
{
  if (editor->shell)
    {
      if (picman_display_get_image (editor->shell->display))
        picman_display_shell_scroll_center_image_coordinate (editor->shell,
                                                           x + width / 2,
                                                           y + height / 2);
    }
}

static void
picman_navigation_editor_zoom (PicmanNavigationView   *view,
                             PicmanZoomType          direction,
                             PicmanNavigationEditor *editor)
{
  g_return_if_fail (direction != PICMAN_ZOOM_TO);

  if (editor->shell)
    {
      if (picman_display_get_image (editor->shell->display))
        picman_display_shell_scale (editor->shell,
                                  direction,
                                  0.0,
                                  PICMAN_ZOOM_FOCUS_BEST_GUESS);
    }
}

static void
picman_navigation_editor_scroll (PicmanNavigationView   *view,
                               GdkScrollDirection    direction,
                               PicmanNavigationEditor *editor)
{
  if (editor->shell)
    {
      GtkAdjustment *adj = NULL;
      gdouble        value;

      switch (direction)
        {
        case GDK_SCROLL_LEFT:
        case GDK_SCROLL_RIGHT:
          adj = editor->shell->hsbdata;
          break;

        case GDK_SCROLL_UP:
        case GDK_SCROLL_DOWN:
          adj = editor->shell->vsbdata;
          break;
        }

      g_assert (adj != NULL);

      value = gtk_adjustment_get_value (adj);

      switch (direction)
        {
        case GDK_SCROLL_LEFT:
        case GDK_SCROLL_UP:
          value -= gtk_adjustment_get_page_increment (adj) / 2;
          break;

        case GDK_SCROLL_RIGHT:
        case GDK_SCROLL_DOWN:
          value += gtk_adjustment_get_page_increment (adj) / 2;
          break;
        }

      value = CLAMP (value,
                     gtk_adjustment_get_lower (adj),
                     gtk_adjustment_get_upper (adj) -
                     gtk_adjustment_get_page_size (adj));

      gtk_adjustment_set_value (adj, value);
    }
}

static gboolean
picman_navigation_editor_zoom_adj_changed_timeout (gpointer data)
{
  PicmanNavigationEditor *editor = PICMAN_NAVIGATION_EDITOR (data);
  GtkAdjustment        *adj    = editor->zoom_adjustment;

  if (picman_display_get_image (editor->shell->display))
    picman_display_shell_scale (editor->shell,
                              PICMAN_ZOOM_TO,
                              pow (2.0, gtk_adjustment_get_value (adj)),
                              PICMAN_ZOOM_FOCUS_BEST_GUESS);

  editor->scale_timeout = 0;

  return FALSE;
}

static void
picman_navigation_editor_zoom_adj_changed (GtkAdjustment        *adj,
                                         PicmanNavigationEditor *editor)
{
  if (editor->scale_timeout)
    g_source_remove (editor->scale_timeout);

  editor->scale_timeout =
    g_timeout_add (UPDATE_DELAY,
                   picman_navigation_editor_zoom_adj_changed_timeout,
                   editor);
}

static void
picman_navigation_editor_shell_scaled (PicmanDisplayShell     *shell,
                                     PicmanNavigationEditor *editor)
{
  if (editor->zoom_label)
    {
      gchar *str;

      g_object_get (shell->zoom,
                    "percentage", &str,
                    NULL);
      gtk_label_set_text (GTK_LABEL (editor->zoom_label), str);
      g_free (str);
    }

  if (editor->zoom_adjustment)
    {
      gdouble val;

      val = log (picman_zoom_model_get_factor (shell->zoom)) / G_LN2;

      g_signal_handlers_block_by_func (editor->zoom_adjustment,
                                       picman_navigation_editor_zoom_adj_changed,
                                       editor);

      gtk_adjustment_set_value (editor->zoom_adjustment, val);

      g_signal_handlers_unblock_by_func (editor->zoom_adjustment,
                                         picman_navigation_editor_zoom_adj_changed,
                                         editor);
    }

  picman_navigation_editor_update_marker (editor);

  if (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)))
    picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                            picman_editor_get_popup_data (PICMAN_EDITOR (editor)));
}

static void
picman_navigation_editor_shell_scrolled (PicmanDisplayShell     *shell,
                                       PicmanNavigationEditor *editor)
{
  picman_navigation_editor_update_marker (editor);

  if (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)))
    picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                            picman_editor_get_popup_data (PICMAN_EDITOR (editor)));
}

static void
picman_navigation_editor_shell_reconnect (PicmanDisplayShell     *shell,
                                        PicmanNavigationEditor *editor)
{
  PicmanImage *image = picman_display_get_image (shell->display);

  picman_view_set_viewable (PICMAN_VIEW (editor->view),
                          PICMAN_VIEWABLE (image));

  if (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)))
    picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                            picman_editor_get_popup_data (PICMAN_EDITOR (editor)));
}

static void
picman_navigation_editor_update_marker (PicmanNavigationEditor *editor)
{
  PicmanViewRenderer *renderer = PICMAN_VIEW (editor->view)->renderer;
  PicmanDisplayShell *shell    = editor->shell;

  if (renderer->dot_for_dot != shell->dot_for_dot)
    picman_view_renderer_set_dot_for_dot (renderer, shell->dot_for_dot);

  if (renderer->viewable)
    {
      PicmanNavigationView *view = PICMAN_NAVIGATION_VIEW (editor->view);
      gdouble             x, y;
      gdouble             w, h;

      picman_display_shell_scroll_get_viewport (shell, &x, &y, &w, &h);

      picman_navigation_view_set_marker (view, x, y, w, h);
    }
}
