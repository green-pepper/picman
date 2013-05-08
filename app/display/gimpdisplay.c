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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"

#include "display-types.h"
#include "tools/tools-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmanarea.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanprogress.h"

#include "widgets/picmandialogfactory.h"

#include "tools/picmantool.h"
#include "tools/tool_manager.h"

#include "picmandisplay.h"
#include "picmandisplay-handlers.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-expose.h"
#include "picmandisplayshell-handlers.h"
#include "picmandisplayshell-icon.h"
#include "picmandisplayshell-transform.h"
#include "picmanimagewindow.h"

#include "picman-intl.h"


#define FLUSH_NOW_INTERVAL 20000 /* 20 ms in microseconds */


enum
{
  PROP_0,
  PROP_ID,
  PROP_PICMAN,
  PROP_IMAGE,
  PROP_SHELL
};


typedef struct _PicmanDisplayPrivate PicmanDisplayPrivate;

struct _PicmanDisplayPrivate
{
  gint       ID;           /*  unique identifier for this display  */

  PicmanImage *image;        /*  pointer to the associated image     */
  gint       instance;     /*  the instance # of this display as
                            *  taken from the image at creation    */

  GtkWidget *shell;
  GSList    *update_areas;

  guint64    last_flush_now;
};

#define PICMAN_DISPLAY_GET_PRIVATE(display) \
        G_TYPE_INSTANCE_GET_PRIVATE (display, \
                                     PICMAN_TYPE_DISPLAY, \
                                     PicmanDisplayPrivate)


/*  local function prototypes  */

static void     picman_display_progress_iface_init  (PicmanProgressInterface *iface);

static void     picman_display_set_property           (GObject             *object,
                                                     guint                property_id,
                                                     const GValue        *value,
                                                     GParamSpec          *pspec);
static void     picman_display_get_property           (GObject             *object,
                                                     guint                property_id,
                                                     GValue              *value,
                                                     GParamSpec          *pspec);

static PicmanProgress * picman_display_progress_start   (PicmanProgress        *progress,
                                                     const gchar         *message,
                                                     gboolean             cancelable);
static void     picman_display_progress_end           (PicmanProgress        *progress);
static gboolean picman_display_progress_is_active     (PicmanProgress        *progress);
static void     picman_display_progress_set_text      (PicmanProgress        *progress,
                                                     const gchar         *message);
static void     picman_display_progress_set_value     (PicmanProgress        *progress,
                                                     gdouble              percentage);
static gdouble  picman_display_progress_get_value     (PicmanProgress        *progress);
static void     picman_display_progress_pulse         (PicmanProgress        *progress);
static guint32  picman_display_progress_get_window_id (PicmanProgress        *progress);
static gboolean picman_display_progress_message       (PicmanProgress        *progress,
                                                     Picman                *picman,
                                                     PicmanMessageSeverity  severity,
                                                     const gchar         *domain,
                                                     const gchar         *message);
static void     picman_display_progress_canceled      (PicmanProgress        *progress,
                                                     PicmanDisplay         *display);

static void     picman_display_flush_whenever         (PicmanDisplay         *display,
                                                     gboolean             now);
static void     picman_display_paint_area             (PicmanDisplay         *display,
                                                     gint                 x,
                                                     gint                 y,
                                                     gint                 w,
                                                     gint                 h);


G_DEFINE_TYPE_WITH_CODE (PicmanDisplay, picman_display, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROGRESS,
                                                picman_display_progress_iface_init))

#define parent_class picman_display_parent_class


static void
picman_display_class_init (PicmanDisplayClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_display_set_property;
  object_class->get_property = picman_display_get_property;

  g_object_class_install_property (object_class, PROP_ID,
                                   g_param_spec_int ("id",
                                                     NULL, NULL,
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_IMAGE,
                                   g_param_spec_object ("image",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_IMAGE,
                                                        PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_SHELL,
                                   g_param_spec_object ("shell",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DISPLAY_SHELL,
                                                        PICMAN_PARAM_READABLE));

  g_type_class_add_private (klass, sizeof (PicmanDisplayPrivate));
}

static void
picman_display_init (PicmanDisplay *display)
{
}

static void
picman_display_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start         = picman_display_progress_start;
  iface->end           = picman_display_progress_end;
  iface->is_active     = picman_display_progress_is_active;
  iface->set_text      = picman_display_progress_set_text;
  iface->set_value     = picman_display_progress_set_value;
  iface->get_value     = picman_display_progress_get_value;
  iface->pulse         = picman_display_progress_pulse;
  iface->get_window_id = picman_display_progress_get_window_id;
  iface->message       = picman_display_progress_message;
}

static void
picman_display_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (object);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  switch (property_id)
    {
    case PROP_PICMAN:
      {
        gint ID;

        display->picman = g_value_get_object (value); /* don't ref the picman */
        display->config = PICMAN_DISPLAY_CONFIG (display->picman->config);

        do
          {
            ID = display->picman->next_display_ID++;

            if (display->picman->next_display_ID == G_MAXINT)
              display->picman->next_display_ID = 1;
          }
        while (picman_display_get_by_ID (display->picman, ID));

        private->ID = ID;
      }
      break;

    case PROP_ID:
    case PROP_IMAGE:
    case PROP_SHELL:
      g_assert_not_reached ();
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_display_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (object);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  switch (property_id)
    {
    case PROP_ID:
      g_value_set_int (value, private->ID);
      break;

    case PROP_PICMAN:
      g_value_set_object (value, display->picman);
      break;

    case PROP_IMAGE:
      g_value_set_object (value, private->image);
      break;

    case PROP_SHELL:
      g_value_set_object (value, private->shell);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static PicmanProgress *
picman_display_progress_start (PicmanProgress *progress,
                             const gchar  *message,
                             gboolean      cancelable)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (progress);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->shell)
    return picman_progress_start (PICMAN_PROGRESS (private->shell),
                                message, cancelable);

  return NULL;
}

static void
picman_display_progress_end (PicmanProgress *progress)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (progress);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->shell)
    picman_progress_end (PICMAN_PROGRESS (private->shell));
}

static gboolean
picman_display_progress_is_active (PicmanProgress *progress)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (progress);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->shell)
    return picman_progress_is_active (PICMAN_PROGRESS (private->shell));

  return FALSE;
}

static void
picman_display_progress_set_text (PicmanProgress *progress,
                                const gchar  *message)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (progress);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->shell)
    picman_progress_set_text (PICMAN_PROGRESS (private->shell), message);
}

static void
picman_display_progress_set_value (PicmanProgress *progress,
                                 gdouble       percentage)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (progress);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->shell)
    picman_progress_set_value (PICMAN_PROGRESS (private->shell), percentage);
}

static gdouble
picman_display_progress_get_value (PicmanProgress *progress)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (progress);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->shell)
    return picman_progress_get_value (PICMAN_PROGRESS (private->shell));

  return 0.0;
}

static void
picman_display_progress_pulse (PicmanProgress *progress)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (progress);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->shell)
    picman_progress_pulse (PICMAN_PROGRESS (private->shell));
}

static guint32
picman_display_progress_get_window_id (PicmanProgress *progress)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (progress);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->shell)
    return picman_progress_get_window_id (PICMAN_PROGRESS (private->shell));

  return 0;
}

static gboolean
picman_display_progress_message (PicmanProgress        *progress,
                               Picman                *picman,
                               PicmanMessageSeverity  severity,
                               const gchar         *domain,
                               const gchar         *message)
{
  PicmanDisplay        *display = PICMAN_DISPLAY (progress);
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->shell)
    return picman_progress_message (PICMAN_PROGRESS (private->shell), picman,
                                  severity, domain, message);

  return FALSE;
}

static void
picman_display_progress_canceled (PicmanProgress *progress,
                                PicmanDisplay  *display)
{
  picman_progress_cancel (PICMAN_PROGRESS (display));
}


/*  public functions  */

PicmanDisplay *
picman_display_new (Picman              *picman,
                  PicmanImage         *image,
                  PicmanUnit           unit,
                  gdouble            scale,
                  PicmanMenuFactory   *menu_factory,
                  PicmanUIManager     *popup_manager,
                  PicmanDialogFactory *dialog_factory)
{
  PicmanDisplay        *display;
  PicmanDisplayPrivate *private;
  PicmanImageWindow    *window = NULL;
  PicmanDisplayShell   *shell;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (image == NULL || PICMAN_IS_IMAGE (image), NULL);

  /*  If there isn't an interface, never create a display  */
  if (picman->no_interface)
    return NULL;

  display = g_object_new (PICMAN_TYPE_DISPLAY,
                          "picman", picman,
                          NULL);

  private = PICMAN_DISPLAY_GET_PRIVATE (display);

  /*  refs the image  */
  if (image)
    picman_display_set_image (display, image);

  /*  get an image window  */
  if (PICMAN_GUI_CONFIG (display->config)->single_window_mode)
    {
      PicmanDisplay *active_display;

      active_display = picman_context_get_display (picman_get_user_context (picman));

      if (! active_display)
        {
          active_display =
            PICMAN_DISPLAY (picman_container_get_first_child (picman->displays));
        }

      if (active_display)
        {
          PicmanDisplayShell *shell = picman_display_get_shell (active_display);

          window = picman_display_shell_get_window (shell);
        }
    }

  if (! window)
    {
      window = picman_image_window_new (picman,
                                      private->image,
                                      menu_factory,
                                      dialog_factory);
    }

  /*  create the shell for the image  */
  private->shell = picman_display_shell_new (display, unit, scale,
                                           popup_manager);

  shell = picman_display_get_shell (display);

  picman_image_window_add_shell (window, shell);
  picman_display_shell_present (shell);

  /* make sure the docks are visible, in case all other image windows
   * are iconified, see bug #686544.
   */
  picman_dialog_factory_show_with_display (dialog_factory);

  g_signal_connect (picman_display_shell_get_statusbar (shell), "cancel",
                    G_CALLBACK (picman_display_progress_canceled),
                    display);

  /* add the display to the list */
  picman_container_add (picman->displays, PICMAN_OBJECT (display));

  return display;
}

/**
 * picman_display_delete:
 * @display:
 *
 * Closes the display and removes it from the display list. You should
 * not call this function directly, use picman_display_close() instead.
 */
void
picman_display_delete (PicmanDisplay *display)
{
  PicmanDisplayPrivate *private;
  PicmanTool           *active_tool;

  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  private = PICMAN_DISPLAY_GET_PRIVATE (display);

  /* remove the display from the list */
  picman_container_remove (display->picman->displays, PICMAN_OBJECT (display));

  /*  unrefs the image  */
  picman_display_set_image (display, NULL);

  active_tool = tool_manager_get_active (display->picman);

  if (active_tool && active_tool->focus_display == display)
    tool_manager_focus_display_active (display->picman, NULL);

  /*  free the update area lists  */
  picman_area_list_free (private->update_areas);
  private->update_areas = NULL;

  if (private->shell)
    {
      PicmanDisplayShell *shell  = picman_display_get_shell (display);
      PicmanImageWindow  *window = picman_display_shell_get_window (shell);

      /*  set private->shell to NULL *before* destroying the shell.
       *  all callbacks in picmandisplayshell-callbacks.c will check
       *  this pointer and do nothing if the shell is in destruction.
       */
      private->shell = NULL;

      if (window)
        {
          if (picman_image_window_get_n_shells (window) > 1)
            {
              g_object_ref (shell);

              picman_image_window_remove_shell (window, shell);
              gtk_widget_destroy (GTK_WIDGET (shell));

              g_object_unref (shell);
            }
          else
            {
              picman_image_window_destroy (window);
            }
        }
      else
        {
          g_object_unref (shell);
        }
    }

  g_object_unref (display);
}

/**
 * picman_display_close:
 * @display:
 *
 * Closes the display. If this is the last display, it will remain
 * open, but without an image.
 */
void
picman_display_close (PicmanDisplay *display)
{
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  if (picman_container_get_n_children (display->picman->displays) > 1)
    {
      picman_display_delete (display);
    }
  else
    {
      picman_display_empty (display);
    }
}

gint
picman_display_get_ID (PicmanDisplay *display)
{
  PicmanDisplayPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), -1);

  private = PICMAN_DISPLAY_GET_PRIVATE (display);

  return private->ID;
}

PicmanDisplay *
picman_display_get_by_ID (Picman *picman,
                        gint  ID)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  for (list = picman_get_display_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanDisplay *display = list->data;

      if (picman_display_get_ID (display) == ID)
        return display;
    }

  return NULL;
}

/**
 * picman_display_get_action_name:
 * @display:
 *
 * Returns: The action name for the given display. The action name
 * depends on the display ID. The result must be freed with g_free().
 **/
gchar *
picman_display_get_action_name (PicmanDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), NULL);

  return g_strdup_printf ("windows-display-%04d",
                          picman_display_get_ID (display));
}

Picman *
picman_display_get_picman (PicmanDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), NULL);

  return display->picman;
}

PicmanImage *
picman_display_get_image (PicmanDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), NULL);

  return PICMAN_DISPLAY_GET_PRIVATE (display)->image;
}

void
picman_display_set_image (PicmanDisplay *display,
                        PicmanImage   *image)
{
  PicmanDisplayPrivate *private;
  PicmanImage          *old_image = NULL;
  PicmanDisplayShell   *shell;

  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (image == NULL || PICMAN_IS_IMAGE (image));

  private = PICMAN_DISPLAY_GET_PRIVATE (display);

  shell = picman_display_get_shell (display);

  if (private->image)
    {
      /*  stop any active tool  */
      tool_manager_control_active (display->picman, PICMAN_TOOL_ACTION_HALT,
                                   display);

      picman_display_shell_disconnect (shell);

      picman_display_disconnect (display);

      picman_image_dec_display_count (private->image);

      /*  set private->image before unrefing because there may be code
       *  that listens for image removals and then iterates the
       *  display list to find a valid display.
       */
      old_image = private->image;

#if 0
      g_print ("%s: image->ref_count before unrefing: %d\n",
               G_STRFUNC, G_OBJECT (old_image)->ref_count);
#endif
    }

  private->image = image;

  if (image)
    {
#if 0
      g_print ("%s: image->ref_count before refing: %d\n",
               G_STRFUNC, G_OBJECT (image)->ref_count);
#endif

      g_object_ref (image);

      private->instance = picman_image_get_instance_count (image);
      picman_image_inc_instance_count (image);

      picman_image_inc_display_count (image);

      picman_display_connect (display);

      if (shell)
        picman_display_shell_connect (shell);
    }

  if (old_image)
    g_object_unref (old_image);

  if (shell)
    {
      if (image)
        picman_display_shell_reconnect (shell);
      else
        picman_display_shell_icon_update (shell);
    }

  if (old_image != image)
    g_object_notify (G_OBJECT (display), "image");
}

gint
picman_display_get_instance (PicmanDisplay *display)
{
  PicmanDisplayPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), 0);

  private = PICMAN_DISPLAY_GET_PRIVATE (display);

  return private->instance;
}

PicmanDisplayShell *
picman_display_get_shell (PicmanDisplay *display)
{
  PicmanDisplayPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), NULL);

  private = PICMAN_DISPLAY_GET_PRIVATE (display);

  return PICMAN_DISPLAY_SHELL (private->shell);
}

void
picman_display_empty (PicmanDisplay *display)
{
  PicmanDisplayPrivate *private;

  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  private = PICMAN_DISPLAY_GET_PRIVATE (display);

  g_return_if_fail (PICMAN_IS_IMAGE (private->image));

  picman_display_set_image (display, NULL);

  picman_display_shell_empty (picman_display_get_shell (display));
}

void
picman_display_fill (PicmanDisplay *display,
                   PicmanImage   *image,
                   PicmanUnit     unit,
                   gdouble      scale)
{
  PicmanDisplayPrivate *private;

  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_DISPLAY_GET_PRIVATE (display);

  g_return_if_fail (private->image == NULL);

  picman_display_set_image (display, image);

  picman_display_shell_fill (picman_display_get_shell (display),
                           image, unit, scale);
}

void
picman_display_update_area (PicmanDisplay *display,
                          gboolean     now,
                          gint         x,
                          gint         y,
                          gint         w,
                          gint         h)
{
  PicmanDisplayPrivate *private;

  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (now)
    {
      picman_display_paint_area (display, x, y, w, h);
    }
  else
    {
      PicmanArea *area;
      gint      image_width  = picman_image_get_width  (private->image);
      gint      image_height = picman_image_get_height (private->image);

      area = picman_area_new (CLAMP (x,     0, image_width),
                            CLAMP (y,     0, image_height),
                            CLAMP (x + w, 0, image_width),
                            CLAMP (y + h, 0, image_height));

      private->update_areas = picman_area_list_process (private->update_areas,
                                                      area);
    }
}

void
picman_display_flush (PicmanDisplay *display)
{
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  picman_display_flush_whenever (display, FALSE);
}

void
picman_display_flush_now (PicmanDisplay *display)
{
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  picman_display_flush_whenever (display, TRUE);
}


/*  private functions  */

static void
picman_display_flush_whenever (PicmanDisplay *display,
                             gboolean     now)
{
  PicmanDisplayPrivate *private = PICMAN_DISPLAY_GET_PRIVATE (display);

  if (private->update_areas)
    {
      GSList *list;

      for (list = private->update_areas; list; list = g_slist_next (list))
        {
          PicmanArea *area = list->data;

          if ((area->x1 != area->x2) && (area->y1 != area->y2))
            {
              picman_display_paint_area (display,
                                       area->x1,
                                       area->y1,
                                       (area->x2 - area->x1),
                                       (area->y2 - area->y1));
            }
        }

      picman_area_list_free (private->update_areas);
      private->update_areas = NULL;
    }

  if (now)
    {
      guint64 now = g_get_monotonic_time ();

      if ((now - private->last_flush_now) > FLUSH_NOW_INTERVAL)
        {
          picman_display_shell_flush (picman_display_get_shell (display), now);

          private->last_flush_now = now;
        }
    }
  else
    {
      picman_display_shell_flush (picman_display_get_shell (display), now);
    }
}

static void
picman_display_paint_area (PicmanDisplay *display,
                         gint         x,
                         gint         y,
                         gint         w,
                         gint         h)
{
  PicmanDisplayPrivate *private      = PICMAN_DISPLAY_GET_PRIVATE (display);
  PicmanDisplayShell   *shell        = picman_display_get_shell (display);
  gint                image_width  = picman_image_get_width  (private->image);
  gint                image_height = picman_image_get_height (private->image);
  gint                x1, y1, x2, y2;
  gdouble             x1_f, y1_f, x2_f, y2_f;

  /*  Bounds check  */
  x1 = CLAMP (x,     0, image_width);
  y1 = CLAMP (y,     0, image_height);
  x2 = CLAMP (x + w, 0, image_width);
  y2 = CLAMP (y + h, 0, image_height);

  x = x1;
  y = y1;
  w = (x2 - x1);
  h = (y2 - y1);

  /*  display the area  */
  picman_display_shell_transform_bounds (shell,
                                       x, y, x + w, y + h,
                                       &x1_f, &y1_f, &x2_f, &y2_f);

  /*  make sure to expose a superset of the transformed sub-pixel expose
   *  area, not a subset. bug #126942. --mitch
   *
   *  also accommodate for spill introduced by potential box filtering.
   *  (bug #474509). --simon
   */
  x1 = floor (x1_f - 0.5);
  y1 = floor (y1_f - 0.5);
  x2 = ceil (x2_f + 0.5);
  y2 = ceil (y2_f + 0.5);

  picman_display_shell_expose_area (shell, x1, y1, x2 - x1, y2 - y1);
}
