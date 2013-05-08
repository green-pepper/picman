/* PICMAN - The GNU Image Manipulation Program Copyright (C) 1995
 * Spencer Kimball and Peter Mattis
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmandisplayconfig.h"

#include "core/picmanimage.h"
#include "core/picmanprogress.h"

#include "widgets/picmanwidgets-utils.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-scale.h"
#include "picmanimagewindow.h"
#include "picmanscalecombobox.h"
#include "picmanstatusbar.h"

#include "picman-intl.h"


/*  maximal width of the string holding the cursor-coordinates  */
#define CURSOR_LEN        256

/*  the spacing of the hbox                                     */
#define HBOX_SPACING        1

/*  spacing between the icon and the statusbar label            */
#define ICON_SPACING        2

/*  timeout (in milliseconds) for temporary statusbar messages  */
#define MESSAGE_TIMEOUT  8000


typedef struct _PicmanStatusbarMsg PicmanStatusbarMsg;

struct _PicmanStatusbarMsg
{
  guint  context_id;
  gchar *stock_id;
  gchar *text;
};


static void     picman_statusbar_progress_iface_init (PicmanProgressInterface *iface);

static void     picman_statusbar_dispose            (GObject           *object);
static void     picman_statusbar_finalize           (GObject           *object);

static void     picman_statusbar_hbox_size_request  (GtkWidget         *widget,
                                                   GtkRequisition    *requisition,
                                                   PicmanStatusbar     *statusbar);

static PicmanProgress *
                picman_statusbar_progress_start     (PicmanProgress      *progress,
                                                   const gchar       *message,
                                                   gboolean           cancelable);
static void     picman_statusbar_progress_end       (PicmanProgress      *progress);
static gboolean picman_statusbar_progress_is_active (PicmanProgress      *progress);
static void     picman_statusbar_progress_set_text  (PicmanProgress      *progress,
                                                   const gchar       *message);
static void     picman_statusbar_progress_set_value (PicmanProgress      *progress,
                                                   gdouble            percentage);
static gdouble  picman_statusbar_progress_get_value (PicmanProgress      *progress);
static void     picman_statusbar_progress_pulse     (PicmanProgress      *progress);
static gboolean picman_statusbar_progress_message   (PicmanProgress      *progress,
                                                   Picman              *picman,
                                                   PicmanMessageSeverity severity,
                                                   const gchar       *domain,
                                                   const gchar       *message);
static void     picman_statusbar_progress_canceled  (GtkWidget         *button,
                                                   PicmanStatusbar     *statusbar);

static gboolean picman_statusbar_label_expose       (GtkWidget         *widget,
                                                   GdkEventExpose    *event,
                                                   PicmanStatusbar     *statusbar);

static void     picman_statusbar_update             (PicmanStatusbar     *statusbar);
static void     picman_statusbar_unit_changed       (PicmanUnitComboBox  *combo,
                                                   PicmanStatusbar     *statusbar);
static void     picman_statusbar_scale_changed      (PicmanScaleComboBox *combo,
                                                   PicmanStatusbar     *statusbar);
static void     picman_statusbar_scale_activated    (PicmanScaleComboBox *combo,
                                                   PicmanStatusbar     *statusbar);
static void     picman_statusbar_shell_scaled       (PicmanDisplayShell  *shell,
                                                   PicmanStatusbar     *statusbar);
static void     picman_statusbar_shell_status_notify(PicmanDisplayShell  *shell,
                                                   const GParamSpec  *pspec,
                                                   PicmanStatusbar     *statusbar);
static guint    picman_statusbar_get_context_id     (PicmanStatusbar     *statusbar,
                                                   const gchar       *context);
static gboolean picman_statusbar_temp_timeout       (PicmanStatusbar     *statusbar);

static void     picman_statusbar_msg_free           (PicmanStatusbarMsg  *msg);

static gchar *  picman_statusbar_vprintf            (const gchar       *format,
                                                   va_list            args);


G_DEFINE_TYPE_WITH_CODE (PicmanStatusbar, picman_statusbar, GTK_TYPE_STATUSBAR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROGRESS,
                                                picman_statusbar_progress_iface_init))

#define parent_class picman_statusbar_parent_class


static void
picman_statusbar_class_init (PicmanStatusbarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose  = picman_statusbar_dispose;
  object_class->finalize = picman_statusbar_finalize;
}

static void
picman_statusbar_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start     = picman_statusbar_progress_start;
  iface->end       = picman_statusbar_progress_end;
  iface->is_active = picman_statusbar_progress_is_active;
  iface->set_text  = picman_statusbar_progress_set_text;
  iface->set_value = picman_statusbar_progress_set_value;
  iface->get_value = picman_statusbar_progress_get_value;
  iface->pulse     = picman_statusbar_progress_pulse;
  iface->message   = picman_statusbar_progress_message;
}

static void
picman_statusbar_init (PicmanStatusbar *statusbar)
{
  GtkWidget     *hbox;
  GtkWidget     *hbox2;
  GtkWidget     *image;
  GtkWidget     *label;
  PicmanUnitStore *store;
  GList         *children;

  statusbar->shell          = NULL;
  statusbar->messages       = NULL;
  statusbar->context_ids    = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                     g_free, NULL);
  statusbar->seq_context_id = 1;

  statusbar->temp_context_id =
    picman_statusbar_get_context_id (statusbar, "picman-statusbar-temp");

  statusbar->cursor_format_str[0]   = '\0';
  statusbar->cursor_format_str_f[0] = '\0';
  statusbar->length_format_str[0]   = '\0';

  statusbar->progress_active      = FALSE;
  statusbar->progress_shown       = FALSE;

  /*  remove the message label from the message area  */
  hbox = gtk_statusbar_get_message_area (GTK_STATUSBAR (statusbar));
  gtk_box_set_spacing (GTK_BOX (hbox), HBOX_SPACING);

  children = gtk_container_get_children (GTK_CONTAINER (hbox));
  statusbar->label = g_object_ref (children->data);
  g_list_free (children);

  gtk_container_remove (GTK_CONTAINER (hbox), statusbar->label);

  g_signal_connect (hbox, "size-request",
                    G_CALLBACK (picman_statusbar_hbox_size_request),
                    statusbar);

  statusbar->cursor_label = gtk_label_new ("8888, 8888");
  gtk_misc_set_alignment (GTK_MISC (statusbar->cursor_label), 0.5, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), statusbar->cursor_label, FALSE, FALSE, 0);
  gtk_widget_show (statusbar->cursor_label);

  store = picman_unit_store_new (2);
  statusbar->unit_combo = picman_unit_combo_box_new_with_model (store);
  g_object_unref (store);

  gtk_widget_set_can_focus (statusbar->unit_combo, FALSE);
  g_object_set (statusbar->unit_combo, "focus-on-click", FALSE, NULL);
  gtk_box_pack_start (GTK_BOX (hbox), statusbar->unit_combo, FALSE, FALSE, 0);
  gtk_widget_show (statusbar->unit_combo);

  g_signal_connect (statusbar->unit_combo, "changed",
                    G_CALLBACK (picman_statusbar_unit_changed),
                    statusbar);

  statusbar->scale_combo = picman_scale_combo_box_new ();
  gtk_widget_set_can_focus (statusbar->scale_combo, FALSE);
  g_object_set (statusbar->scale_combo, "focus-on-click", FALSE, NULL);
  gtk_box_pack_start (GTK_BOX (hbox), statusbar->scale_combo, FALSE, FALSE, 0);
  gtk_widget_show (statusbar->scale_combo);

  g_signal_connect (statusbar->scale_combo, "changed",
                    G_CALLBACK (picman_statusbar_scale_changed),
                    statusbar);

  g_signal_connect (statusbar->scale_combo, "entry-activated",
                    G_CALLBACK (picman_statusbar_scale_activated),
                    statusbar);

  /*  put the label back into the message area  */
  gtk_box_pack_start (GTK_BOX (hbox), statusbar->label, TRUE, TRUE, 1);

  g_object_unref (statusbar->label);

  g_signal_connect_after (statusbar->label, "expose-event",
                          G_CALLBACK (picman_statusbar_label_expose),
                          statusbar);

  statusbar->progressbar = g_object_new (GTK_TYPE_PROGRESS_BAR,
                                         "text-xalign", 0.0,
                                         "text-yalign", 0.5,
                                         "ellipsize",   PANGO_ELLIPSIZE_END,
                                         NULL);
  gtk_box_pack_start (GTK_BOX (hbox), statusbar->progressbar, TRUE, TRUE, 0);
  /*  don't show the progress bar  */

  /*  construct the cancel button's contents manually because we
   *  always want image and label regardless of settings, and we want
   *  a menu size image.
   */
  statusbar->cancel_button = gtk_button_new ();
  gtk_widget_set_can_focus (statusbar->cancel_button, FALSE);
  gtk_button_set_relief (GTK_BUTTON (statusbar->cancel_button),
                         GTK_RELIEF_NONE);
  gtk_widget_set_sensitive (statusbar->cancel_button, FALSE);
  gtk_box_pack_end (GTK_BOX (hbox),
                    statusbar->cancel_button, FALSE, FALSE, 0);
  /*  don't show the cancel button  */

  hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (statusbar->cancel_button), hbox2);
  gtk_widget_show (hbox2);

  image = gtk_image_new_from_stock (GTK_STOCK_CANCEL, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox2), image, FALSE, FALSE, 2);
  gtk_widget_show (image);

  label = gtk_label_new ("Cancel");
  gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 2);
  gtk_widget_show (label);

  g_signal_connect (statusbar->cancel_button, "clicked",
                    G_CALLBACK (picman_statusbar_progress_canceled),
                    statusbar);
}

static void
picman_statusbar_dispose (GObject *object)
{
  PicmanStatusbar *statusbar = PICMAN_STATUSBAR (object);

  if (statusbar->temp_timeout_id)
    {
      g_source_remove (statusbar->temp_timeout_id);
      statusbar->temp_timeout_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_statusbar_finalize (GObject *object)
{
  PicmanStatusbar *statusbar = PICMAN_STATUSBAR (object);

  if (statusbar->icon)
    {
      g_object_unref (statusbar->icon);
      statusbar->icon = NULL;
    }

  g_slist_free_full (statusbar->messages,
                     (GDestroyNotify) picman_statusbar_msg_free);
  statusbar->messages = NULL;

  if (statusbar->context_ids)
    {
      g_hash_table_destroy (statusbar->context_ids);
      statusbar->context_ids = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_statusbar_hbox_size_request (GtkWidget      *widget,
                                  GtkRequisition *requisition,
                                  PicmanStatusbar  *statusbar)
{
  GtkRequisition child_requisition;
  gint           width = 0;

  /*  also consider the children which can be invisible  */

  gtk_widget_size_request (statusbar->cursor_label, &child_requisition);
  width += child_requisition.width;
  requisition->height = MAX (requisition->height,
                             child_requisition.height);

  gtk_widget_size_request (statusbar->unit_combo, &child_requisition);
  width += child_requisition.width;
  requisition->height = MAX (requisition->height,
                             child_requisition.height);

  gtk_widget_size_request (statusbar->scale_combo, &child_requisition);
  width += child_requisition.width;
  requisition->height = MAX (requisition->height,
                             child_requisition.height);

  gtk_widget_size_request (statusbar->progressbar, &child_requisition);
  requisition->height = MAX (requisition->height,
                             child_requisition.height);

  gtk_widget_size_request (statusbar->label, &child_requisition);
  requisition->height = MAX (requisition->height,
                             child_requisition.height);

  gtk_widget_size_request (statusbar->cancel_button, &child_requisition);
  requisition->height = MAX (requisition->height,
                             child_requisition.height);

  requisition->width = MAX (requisition->width, width + 32);
}

static PicmanProgress *
picman_statusbar_progress_start (PicmanProgress *progress,
                               const gchar  *message,
                               gboolean      cancelable)
{
  PicmanStatusbar *statusbar = PICMAN_STATUSBAR (progress);

  if (! statusbar->progress_active)
    {
      GtkWidget *bar = statusbar->progressbar;

      statusbar->progress_active = TRUE;
      statusbar->progress_value  = 0.0;

      picman_statusbar_push (statusbar, "progress", NULL, "%s", message);
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (bar), 0.0);
      gtk_widget_set_sensitive (statusbar->cancel_button, cancelable);

      if (cancelable)
        {
          if (message)
            {
              gchar *tooltip = g_strdup_printf (_("Cancel <i>%s</i>"), message);

              picman_help_set_help_data_with_markup (statusbar->cancel_button,
                                                   tooltip, NULL);
              g_free (tooltip);
            }

          gtk_widget_show (statusbar->cancel_button);
        }

      gtk_widget_show (statusbar->progressbar);
      gtk_widget_hide (statusbar->label);

      /*  This call is needed so that the progress bar is drawn in the
       *  correct place. Probably due a bug in GTK+.
       */
      gtk_container_resize_children (GTK_CONTAINER (statusbar));

      if (! gtk_widget_get_visible (GTK_WIDGET (statusbar)))
        {
          gtk_widget_show (GTK_WIDGET (statusbar));
          statusbar->progress_shown = TRUE;
        }

      picman_widget_flush_expose (bar);

      picman_statusbar_override_window_title (statusbar);

      return progress;
    }

  return NULL;
}

static void
picman_statusbar_progress_end (PicmanProgress *progress)
{
  PicmanStatusbar *statusbar = PICMAN_STATUSBAR (progress);

  if (statusbar->progress_active)
    {
      GtkWidget *bar = statusbar->progressbar;

      if (statusbar->progress_shown)
        {
          gtk_widget_hide (GTK_WIDGET (statusbar));
          statusbar->progress_shown = FALSE;
        }

      statusbar->progress_active = FALSE;
      statusbar->progress_value  = 0.0;

      gtk_widget_hide (bar);
      gtk_widget_show (statusbar->label);

      picman_statusbar_pop (statusbar, "progress");

      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (bar), 0.0);
      gtk_widget_set_sensitive (statusbar->cancel_button, FALSE);
      gtk_widget_hide (statusbar->cancel_button);

      picman_statusbar_restore_window_title (statusbar);
    }
}

static gboolean
picman_statusbar_progress_is_active (PicmanProgress *progress)
{
  PicmanStatusbar *statusbar = PICMAN_STATUSBAR (progress);

  return statusbar->progress_active;
}

static void
picman_statusbar_progress_set_text (PicmanProgress *progress,
                                  const gchar  *message)
{
  PicmanStatusbar *statusbar = PICMAN_STATUSBAR (progress);

  if (statusbar->progress_active)
    {
      GtkWidget *bar = statusbar->progressbar;

      picman_statusbar_replace (statusbar, "progress", NULL, "%s", message);

      picman_widget_flush_expose (bar);

      picman_statusbar_override_window_title (statusbar);
    }
}

static void
picman_statusbar_progress_set_value (PicmanProgress *progress,
                                   gdouble       percentage)
{
  PicmanStatusbar *statusbar = PICMAN_STATUSBAR (progress);

  if (statusbar->progress_active)
    {
      GtkWidget     *bar = statusbar->progressbar;
      GtkAllocation  allocation;

      gtk_widget_get_allocation (bar, &allocation);

      statusbar->progress_value = percentage;

      /* only update the progress bar if this causes a visible change */
      if (fabs (allocation.width *
                (percentage -
                 gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (bar)))) > 1.0)
        {
          gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (bar), percentage);

          picman_widget_flush_expose (bar);
        }
    }
}

static gdouble
picman_statusbar_progress_get_value (PicmanProgress *progress)
{
  PicmanStatusbar *statusbar = PICMAN_STATUSBAR (progress);

  if (statusbar->progress_active)
    return statusbar->progress_value;

  return 0.0;
}

static void
picman_statusbar_progress_pulse (PicmanProgress *progress)
{
  PicmanStatusbar *statusbar = PICMAN_STATUSBAR (progress);

  if (statusbar->progress_active)
    {
      GtkWidget *bar = statusbar->progressbar;

      gtk_progress_bar_pulse (GTK_PROGRESS_BAR (bar));

      picman_widget_flush_expose (bar);
    }
}

static gboolean
picman_statusbar_progress_message (PicmanProgress        *progress,
                                 Picman                *picman,
                                 PicmanMessageSeverity  severity,
                                 const gchar         *domain,
                                 const gchar         *message)
{
  PicmanStatusbar *statusbar  = PICMAN_STATUSBAR (progress);
  PangoLayout   *layout;
  const gchar   *stock_id;
  gboolean       handle_msg = FALSE;

  /*  don't accept a message if we are already displaying a more severe one  */
  if (statusbar->temp_timeout_id && statusbar->temp_severity > severity)
    return FALSE;

  /*  we can only handle short one-liners  */
  layout = gtk_widget_create_pango_layout (statusbar->label, message);

  stock_id = picman_get_message_stock_id (severity);

  if (pango_layout_get_line_count (layout) == 1)
    {
      GtkAllocation label_allocation;
      gint          width;

      gtk_widget_get_allocation (statusbar->label, &label_allocation);

      pango_layout_get_pixel_size (layout, &width, NULL);

      if (width < label_allocation.width)
        {
          if (stock_id)
            {
              GdkPixbuf *pixbuf;

              pixbuf = gtk_widget_render_icon (statusbar->label, stock_id,
                                               GTK_ICON_SIZE_MENU, NULL);

              width += ICON_SPACING + gdk_pixbuf_get_width (pixbuf);

              g_object_unref (pixbuf);

              handle_msg = (width < label_allocation.width);
            }
          else
            {
              handle_msg = TRUE;
            }
        }
    }

  g_object_unref (layout);

  if (handle_msg)
    picman_statusbar_push_temp (statusbar, severity, stock_id, "%s", message);

  return handle_msg;
}

static void
picman_statusbar_progress_canceled (GtkWidget     *button,
                                  PicmanStatusbar *statusbar)
{
  if (statusbar->progress_active)
    picman_progress_cancel (PICMAN_PROGRESS (statusbar));
}

static void
picman_statusbar_set_text (PicmanStatusbar *statusbar,
                         const gchar   *stock_id,
                         const gchar   *text)
{
  if (statusbar->progress_active)
    {
      gtk_progress_bar_set_text (GTK_PROGRESS_BAR (statusbar->progressbar),
                                 text);
    }
  else
    {
      if (statusbar->icon)
        g_object_unref (statusbar->icon);

      if (stock_id)
        statusbar->icon = gtk_widget_render_icon (statusbar->label,
                                                  stock_id,
                                                  GTK_ICON_SIZE_MENU, NULL);
      else
        statusbar->icon = NULL;

      if (statusbar->icon)
        {
          PangoAttrList  *attrs;
          PangoAttribute *attr;
          PangoRectangle  rect;
          gchar          *tmp;

          tmp = g_strconcat (" ", text, NULL);
          gtk_label_set_text (GTK_LABEL (statusbar->label), tmp);
          g_free (tmp);

          rect.x      = 0;
          rect.y      = 0;
          rect.width  = PANGO_SCALE * (gdk_pixbuf_get_width (statusbar->icon) +
                                       ICON_SPACING);
          rect.height = 0;

          attrs = pango_attr_list_new ();

          attr = pango_attr_shape_new (&rect, &rect);
          attr->start_index = 0;
          attr->end_index   = 1;
          pango_attr_list_insert (attrs, attr);

          gtk_label_set_attributes (GTK_LABEL (statusbar->label), attrs);
          pango_attr_list_unref (attrs);
        }
      else
        {
          gtk_label_set_text (GTK_LABEL (statusbar->label), text);
          gtk_label_set_attributes (GTK_LABEL (statusbar->label), NULL);
        }
    }
}

static void
picman_statusbar_update (PicmanStatusbar *statusbar)
{
  PicmanStatusbarMsg *msg = NULL;

  if (statusbar->messages)
    {
      msg = statusbar->messages->data;

      /*  only allow progress messages while the progress is active  */
      if (statusbar->progress_active)
        {
          guint context_id = picman_statusbar_get_context_id (statusbar,
                                                            "progress");

          if (context_id != msg->context_id)
            return;
        }
    }

  if (msg && msg->text)
    {
      picman_statusbar_set_text (statusbar, msg->stock_id, msg->text);
    }
  else
    {
      picman_statusbar_set_text (statusbar, NULL, "");
    }
}


/*  public functions  */

GtkWidget *
picman_statusbar_new (void)
{
  return g_object_new (PICMAN_TYPE_STATUSBAR, NULL);
}

void
picman_statusbar_set_shell (PicmanStatusbar    *statusbar,
                          PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell == statusbar->shell)
    return;

  if (statusbar->shell)
    {
      g_signal_handlers_disconnect_by_func (statusbar->shell,
                                            picman_statusbar_shell_scaled,
                                            statusbar);

      g_signal_handlers_disconnect_by_func (statusbar->shell,
                                            picman_statusbar_shell_status_notify,
                                            statusbar);
    }

  statusbar->shell = shell;

  g_signal_connect_object (statusbar->shell, "scaled",
                           G_CALLBACK (picman_statusbar_shell_scaled),
                           statusbar, 0);

  g_signal_connect_object (statusbar->shell, "notify::status",
                           G_CALLBACK (picman_statusbar_shell_status_notify),
                           statusbar, 0);
}

gboolean
picman_statusbar_get_visible (PicmanStatusbar *statusbar)
{
  g_return_val_if_fail (PICMAN_IS_STATUSBAR (statusbar), FALSE);

  if (statusbar->progress_shown)
    return FALSE;

  return gtk_widget_get_visible (GTK_WIDGET (statusbar));
}

void
picman_statusbar_set_visible (PicmanStatusbar *statusbar,
                            gboolean       visible)
{
  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));

  if (statusbar->progress_shown)
    {
      if (visible)
        {
          statusbar->progress_shown = FALSE;
          return;
        }
    }

  gtk_widget_set_visible (GTK_WIDGET (statusbar), visible);
}

void
picman_statusbar_empty (PicmanStatusbar *statusbar)
{
  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));

  gtk_widget_hide (statusbar->cursor_label);
  gtk_widget_hide (statusbar->unit_combo);
  gtk_widget_hide (statusbar->scale_combo);
}

void
picman_statusbar_fill (PicmanStatusbar *statusbar)
{
  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));

  gtk_widget_show (statusbar->cursor_label);
  gtk_widget_show (statusbar->unit_combo);
  gtk_widget_show (statusbar->scale_combo);
}

void
picman_statusbar_override_window_title (PicmanStatusbar *statusbar)
{
  GtkWidget *toplevel;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (statusbar));

  if (picman_image_window_is_iconified (PICMAN_IMAGE_WINDOW (toplevel)))
    {
      const gchar *message = picman_statusbar_peek (statusbar, "progress");

      if (message)
        gtk_window_set_title (GTK_WINDOW (toplevel), message);
    }
}

void
picman_statusbar_restore_window_title (PicmanStatusbar *statusbar)
{
  GtkWidget *toplevel;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (statusbar));

  if (picman_image_window_is_iconified (PICMAN_IMAGE_WINDOW (toplevel)))
    {
      g_object_notify (G_OBJECT (statusbar->shell), "title");
    }
}

void
picman_statusbar_push (PicmanStatusbar *statusbar,
                     const gchar   *context,
                     const gchar   *stock_id,
                     const gchar   *format,
                     ...)
{
  va_list args;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));
  g_return_if_fail (context != NULL);
  g_return_if_fail (format != NULL);

  va_start (args, format);
  picman_statusbar_push_valist (statusbar, context, stock_id, format, args);
  va_end (args);
}

void
picman_statusbar_push_valist (PicmanStatusbar *statusbar,
                            const gchar   *context,
                            const gchar   *stock_id,
                            const gchar   *format,
                            va_list        args)
{
  PicmanStatusbarMsg *msg;
  guint             context_id;
  GSList           *list;
  gchar            *message;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));
  g_return_if_fail (context != NULL);
  g_return_if_fail (format != NULL);

  message = picman_statusbar_vprintf (format, args);

  context_id = picman_statusbar_get_context_id (statusbar, context);

  if (statusbar->messages)
    {
      msg = statusbar->messages->data;

      if (msg->context_id == context_id && strcmp (msg->text, message) == 0)
        {
          g_free (message);
          return;
        }
    }

  for (list = statusbar->messages; list; list = g_slist_next (list))
    {
      msg = list->data;

      if (msg->context_id == context_id)
        {
          statusbar->messages = g_slist_remove (statusbar->messages, msg);
          picman_statusbar_msg_free (msg);

          break;
        }
    }

  msg = g_slice_new (PicmanStatusbarMsg);

  msg->context_id = context_id;
  msg->stock_id   = g_strdup (stock_id);
  msg->text       = message;

  if (statusbar->temp_timeout_id)
    statusbar->messages = g_slist_insert (statusbar->messages, msg, 1);
  else
    statusbar->messages = g_slist_prepend (statusbar->messages, msg);

  picman_statusbar_update (statusbar);
}

void
picman_statusbar_push_coords (PicmanStatusbar       *statusbar,
                            const gchar         *context,
                            const gchar         *stock_id,
                            PicmanCursorPrecision  precision,
                            const gchar         *title,
                            gdouble              x,
                            const gchar         *separator,
                            gdouble              y,
                            const gchar         *help)
{
  PicmanDisplayShell *shell;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));
  g_return_if_fail (title != NULL);
  g_return_if_fail (separator != NULL);

  if (help == NULL)
    help = "";

  shell = statusbar->shell;

  switch (precision)
    {
    case PICMAN_CURSOR_PRECISION_PIXEL_CENTER:
      x = (gint) x;
      y = (gint) y;
      break;

    case PICMAN_CURSOR_PRECISION_PIXEL_BORDER:
      x = RINT (x);
      y = RINT (y);
      break;

    case PICMAN_CURSOR_PRECISION_SUBPIXEL:
      break;
    }

  if (shell->unit == PICMAN_UNIT_PIXEL)
    {
      if (precision == PICMAN_CURSOR_PRECISION_SUBPIXEL)
        {
          picman_statusbar_push (statusbar, context,
                               stock_id,
                               statusbar->cursor_format_str_f,
                               title,
                               x,
                               separator,
                               y,
                               help);
        }
      else
        {
          picman_statusbar_push (statusbar, context,
                               stock_id,
                               statusbar->cursor_format_str,
                               title,
                               (gint) RINT (x),
                               separator,
                               (gint) RINT (y),
                               help);
        }
    }
  else /* show real world units */
    {
      gdouble xres;
      gdouble yres;

      picman_image_get_resolution (picman_display_get_image (shell->display),
                                 &xres, &yres);

      picman_statusbar_push (statusbar, context,
                           stock_id,
                           statusbar->cursor_format_str,
                           title,
                           picman_pixels_to_units (x, shell->unit, xres),
                           separator,
                           picman_pixels_to_units (y, shell->unit, yres),
                           help);
    }
}

void
picman_statusbar_push_length (PicmanStatusbar       *statusbar,
                            const gchar         *context,
                            const gchar         *stock_id,
                            const gchar         *title,
                            PicmanOrientationType  axis,
                            gdouble              value,
                            const gchar         *help)
{
  PicmanDisplayShell *shell;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));
  g_return_if_fail (title != NULL);

  if (help == NULL)
    help = "";

  shell = statusbar->shell;

  if (shell->unit == PICMAN_UNIT_PIXEL)
    {
      picman_statusbar_push (statusbar, context,
                           stock_id,
                           statusbar->length_format_str,
                           title,
                           (gint) RINT (value),
                           help);
    }
  else /* show real world units */
    {
      gdouble xres;
      gdouble yres;
      gdouble resolution;

      picman_image_get_resolution (picman_display_get_image (shell->display),
                                 &xres, &yres);

      switch (axis)
        {
        case PICMAN_ORIENTATION_HORIZONTAL:
          resolution = xres;
          break;

        case PICMAN_ORIENTATION_VERTICAL:
          resolution = yres;
          break;

        default:
          g_return_if_reached ();
          break;
        }

      picman_statusbar_push (statusbar, context,
                           stock_id,
                           statusbar->length_format_str,
                           title,
                           picman_pixels_to_units (value, shell->unit, resolution),
                           help);
    }
}

void
picman_statusbar_replace (PicmanStatusbar *statusbar,
                        const gchar   *context,
                        const gchar   *stock_id,
                        const gchar   *format,
                        ...)
{
  va_list args;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));
  g_return_if_fail (context != NULL);
  g_return_if_fail (format != NULL);

  va_start (args, format);
  picman_statusbar_replace_valist (statusbar, context, stock_id, format, args);
  va_end (args);
}

void
picman_statusbar_replace_valist (PicmanStatusbar *statusbar,
                               const gchar   *context,
                               const gchar   *stock_id,
                               const gchar   *format,
                               va_list        args)
{
  PicmanStatusbarMsg *msg;
  GSList           *list;
  guint             context_id;
  gchar            *message;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));
  g_return_if_fail (context != NULL);
  g_return_if_fail (format != NULL);

  message = picman_statusbar_vprintf (format, args);

  context_id = picman_statusbar_get_context_id (statusbar, context);

  for (list = statusbar->messages; list; list = g_slist_next (list))
    {
      msg = list->data;

      if (msg->context_id == context_id)
        {
          if (strcmp (msg->text, message) == 0)
            {
              g_free (message);
              return;
            }

          g_free (msg->stock_id);
          msg->stock_id = g_strdup (stock_id);

          g_free (msg->text);
          msg->text = message;

          if (list == statusbar->messages)
            picman_statusbar_update (statusbar);

          return;
        }
    }

  msg = g_slice_new (PicmanStatusbarMsg);

  msg->context_id = context_id;
  msg->stock_id   = g_strdup (stock_id);
  msg->text       = message;

  if (statusbar->temp_timeout_id)
    statusbar->messages = g_slist_insert (statusbar->messages, msg, 1);
  else
    statusbar->messages = g_slist_prepend (statusbar->messages, msg);

  picman_statusbar_update (statusbar);
}

const gchar *
picman_statusbar_peek (PicmanStatusbar *statusbar,
                     const gchar   *context)
{
  GSList *list;
  guint   context_id;

  g_return_val_if_fail (PICMAN_IS_STATUSBAR (statusbar), NULL);
  g_return_val_if_fail (context != NULL, NULL);

  context_id = picman_statusbar_get_context_id (statusbar, context);

  for (list = statusbar->messages; list; list = list->next)
    {
      PicmanStatusbarMsg *msg = list->data;

      if (msg->context_id == context_id)
        {
          return msg->text;
        }
    }

  return NULL;
}

void
picman_statusbar_pop (PicmanStatusbar *statusbar,
                    const gchar   *context)
{
  GSList *list;
  guint   context_id;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));
  g_return_if_fail (context != NULL);

  context_id = picman_statusbar_get_context_id (statusbar, context);

  for (list = statusbar->messages; list; list = list->next)
    {
      PicmanStatusbarMsg *msg = list->data;

      if (msg->context_id == context_id)
        {
          statusbar->messages = g_slist_remove (statusbar->messages, msg);
          picman_statusbar_msg_free (msg);

          break;
        }
    }

  picman_statusbar_update (statusbar);
}

void
picman_statusbar_push_temp (PicmanStatusbar       *statusbar,
                          PicmanMessageSeverity  severity,
                          const gchar         *stock_id,
                          const gchar         *format,
                          ...)
{
  va_list args;

  va_start (args, format);
  picman_statusbar_push_temp_valist (statusbar, severity, stock_id, format, args);
  va_end (args);
}

void
picman_statusbar_push_temp_valist (PicmanStatusbar       *statusbar,
                                 PicmanMessageSeverity  severity,
                                 const gchar         *stock_id,
                                 const gchar         *format,
                                 va_list              args)
{
  PicmanStatusbarMsg *msg = NULL;
  gchar            *message;

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));
  g_return_if_fail (severity <= PICMAN_MESSAGE_WARNING);
  g_return_if_fail (format != NULL);

  /*  don't accept a message if we are already displaying a more severe one  */
  if (statusbar->temp_timeout_id && statusbar->temp_severity > severity)
    return;

  message = picman_statusbar_vprintf (format, args);

  if (statusbar->temp_timeout_id)
    g_source_remove (statusbar->temp_timeout_id);

  statusbar->temp_timeout_id =
    g_timeout_add (MESSAGE_TIMEOUT,
                   (GSourceFunc) picman_statusbar_temp_timeout, statusbar);

  statusbar->temp_severity = severity;

  if (statusbar->messages)
    {
      msg = statusbar->messages->data;

      if (msg->context_id == statusbar->temp_context_id)
        {
          if (strcmp (msg->text, message) == 0)
            {
              g_free (message);
              return;
            }

          g_free (msg->stock_id);
          msg->stock_id = g_strdup (stock_id);

          g_free (msg->text);
          msg->text = message;

          picman_statusbar_update (statusbar);
          return;
        }
    }

  msg = g_slice_new (PicmanStatusbarMsg);

  msg->context_id = statusbar->temp_context_id;
  msg->stock_id   = g_strdup (stock_id);
  msg->text       = message;

  statusbar->messages = g_slist_prepend (statusbar->messages, msg);

  picman_statusbar_update (statusbar);
}

void
picman_statusbar_pop_temp (PicmanStatusbar *statusbar)
{
  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));

  if (statusbar->temp_timeout_id)
    {
      g_source_remove (statusbar->temp_timeout_id);
      statusbar->temp_timeout_id = 0;
    }

  if (statusbar->messages)
    {
      PicmanStatusbarMsg *msg = statusbar->messages->data;

      if (msg->context_id == statusbar->temp_context_id)
        {
          statusbar->messages = g_slist_remove (statusbar->messages, msg);
          picman_statusbar_msg_free (msg);

          picman_statusbar_update (statusbar);
        }
    }
}

void
picman_statusbar_update_cursor (PicmanStatusbar       *statusbar,
                              PicmanCursorPrecision  precision,
                              gdouble              x,
                              gdouble              y)
{
  PicmanDisplayShell *shell;
  PicmanImage        *image;
  gchar             buffer[CURSOR_LEN];

  g_return_if_fail (PICMAN_IS_STATUSBAR (statusbar));

  shell = statusbar->shell;
  image = picman_display_get_image (shell->display);

  if (! image                            ||
      x <  0                             ||
      y <  0                             ||
      x >= picman_image_get_width  (image) ||
      y >= picman_image_get_height (image))
    {
      gtk_widget_set_sensitive (statusbar->cursor_label, FALSE);
    }
  else
    {
      gtk_widget_set_sensitive (statusbar->cursor_label, TRUE);
    }

  switch (precision)
    {
    case PICMAN_CURSOR_PRECISION_PIXEL_CENTER:
      x = (gint) x;
      y = (gint) y;
      break;

    case PICMAN_CURSOR_PRECISION_PIXEL_BORDER:
      x = RINT (x);
      y = RINT (y);
      break;

    case PICMAN_CURSOR_PRECISION_SUBPIXEL:
      break;
    }

  if (shell->unit == PICMAN_UNIT_PIXEL)
    {
      if (precision == PICMAN_CURSOR_PRECISION_SUBPIXEL)
        {
          g_snprintf (buffer, sizeof (buffer),
                      statusbar->cursor_format_str_f,
                      "", x, ", ", y, "");
        }
      else
        {
          g_snprintf (buffer, sizeof (buffer),
                      statusbar->cursor_format_str,
                      "", (gint) RINT (x), ", ", (gint) RINT (y), "");
        }
    }
  else /* show real world units */
    {
      GtkTreeModel  *model;
      PicmanUnitStore *store;

      model = gtk_combo_box_get_model (GTK_COMBO_BOX (statusbar->unit_combo));
      store = PICMAN_UNIT_STORE (model);

      picman_unit_store_set_pixel_values (store, x, y);
      picman_unit_store_get_values (store, shell->unit, &x, &y);

      g_snprintf (buffer, sizeof (buffer),
                  statusbar->cursor_format_str,
                  "", x, ", ", y, "");
    }

  gtk_label_set_text (GTK_LABEL (statusbar->cursor_label), buffer);
}

void
picman_statusbar_clear_cursor (PicmanStatusbar *statusbar)
{
  gtk_label_set_text (GTK_LABEL (statusbar->cursor_label), "");
  gtk_widget_set_sensitive (statusbar->cursor_label, TRUE);
}


/*  private functions  */

static gboolean
picman_statusbar_label_expose (GtkWidget      *widget,
                             GdkEventExpose *event,
                             PicmanStatusbar  *statusbar)
{
  if (statusbar->icon)
    {
      cairo_t        *cr;
      PangoRectangle  rect;
      gint            x, y;

      cr = gdk_cairo_create (event->window);

      gdk_cairo_region (cr, event->region);
      cairo_clip (cr);

      gtk_label_get_layout_offsets (GTK_LABEL (widget), &x, &y);

      pango_layout_index_to_pos (gtk_label_get_layout (GTK_LABEL (widget)), 0,
                                 &rect);

      /*  the rectangle width is negative when rendering right-to-left  */
      x += PANGO_PIXELS (rect.x) + (rect.width < 0 ?
                                    PANGO_PIXELS (rect.width) : 0);
      y += PANGO_PIXELS (rect.y);

      gdk_cairo_set_source_pixbuf (cr, statusbar->icon, x, y);
      cairo_paint (cr);

      cairo_destroy (cr);
    }

  return FALSE;
}

static void
picman_statusbar_shell_scaled (PicmanDisplayShell *shell,
                             PicmanStatusbar    *statusbar)
{
  static PangoLayout *layout = NULL;

  PicmanImage    *image = picman_display_get_image (shell->display);
  GtkTreeModel *model;
  const gchar  *text;
  gint          image_width;
  gint          image_height;
  gdouble       image_xres;
  gdouble       image_yres;
  gint          width;

  if (image)
    {
      image_width  = picman_image_get_width  (image);
      image_height = picman_image_get_height (image);
      picman_image_get_resolution (image, &image_xres, &image_yres);
    }
  else
    {
      image_width  = shell->disp_width;
      image_height = shell->disp_height;
      image_xres   = shell->display->config->monitor_xres;
      image_yres   = shell->display->config->monitor_yres;
    }

  g_signal_handlers_block_by_func (statusbar->scale_combo,
                                   picman_statusbar_scale_changed, statusbar);
  picman_scale_combo_box_set_scale (PICMAN_SCALE_COMBO_BOX (statusbar->scale_combo),
                                  picman_zoom_model_get_factor (shell->zoom));
  g_signal_handlers_unblock_by_func (statusbar->scale_combo,
                                     picman_statusbar_scale_changed, statusbar);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (statusbar->unit_combo));
  picman_unit_store_set_resolutions (PICMAN_UNIT_STORE (model),
                                   image_xres, image_yres);

  g_signal_handlers_block_by_func (statusbar->unit_combo,
                                   picman_statusbar_unit_changed, statusbar);
  picman_unit_combo_box_set_active (PICMAN_UNIT_COMBO_BOX (statusbar->unit_combo),
                                  shell->unit);
  g_signal_handlers_unblock_by_func (statusbar->unit_combo,
                                     picman_statusbar_unit_changed, statusbar);

  if (shell->unit == PICMAN_UNIT_PIXEL)
    {
      g_snprintf (statusbar->cursor_format_str,
                  sizeof (statusbar->cursor_format_str),
                  "%%s%%d%%s%%d%%s");
      g_snprintf (statusbar->cursor_format_str_f,
                  sizeof (statusbar->cursor_format_str_f),
                  "%%s%%.1f%%s%%.1f%%s");
      g_snprintf (statusbar->length_format_str,
                  sizeof (statusbar->length_format_str),
                  "%%s%%d%%s");
    }
  else /* show real world units */
    {
      g_snprintf (statusbar->cursor_format_str,
                  sizeof (statusbar->cursor_format_str),
                  "%%s%%.%df%%s%%.%df%%s",
                  picman_unit_get_digits (shell->unit),
                  picman_unit_get_digits (shell->unit));
      strcpy (statusbar->cursor_format_str_f, statusbar->cursor_format_str);
      g_snprintf (statusbar->length_format_str,
                  sizeof (statusbar->length_format_str),
                  "%%s%%.%df%%s",
                  picman_unit_get_digits (shell->unit));
    }

  picman_statusbar_update_cursor (statusbar, PICMAN_CURSOR_PRECISION_SUBPIXEL,
                                -image_width, -image_height);

  text = gtk_label_get_text (GTK_LABEL (statusbar->cursor_label));

  /* one static layout for all displays should be fine */
  if (! layout)
    layout = gtk_widget_create_pango_layout (statusbar->cursor_label, NULL);

  pango_layout_set_text (layout, text, -1);
  pango_layout_get_pixel_size (layout, &width, NULL);

  gtk_widget_set_size_request (statusbar->cursor_label, width, -1);

  picman_statusbar_clear_cursor (statusbar);
}

static void
picman_statusbar_shell_status_notify (PicmanDisplayShell *shell,
                                    const GParamSpec *pspec,
                                    PicmanStatusbar    *statusbar)
{
  picman_statusbar_replace (statusbar, "title",
                          NULL, "%s", shell->status);
}

static void
picman_statusbar_unit_changed (PicmanUnitComboBox *combo,
                             PicmanStatusbar    *statusbar)
{
  picman_display_shell_set_unit (statusbar->shell,
                               picman_unit_combo_box_get_active (combo));
}

static void
picman_statusbar_scale_changed (PicmanScaleComboBox *combo,
                              PicmanStatusbar     *statusbar)
{
  picman_display_shell_scale (statusbar->shell,
                            PICMAN_ZOOM_TO,
                            picman_scale_combo_box_get_scale (combo),
                            PICMAN_ZOOM_FOCUS_BEST_GUESS);
}

static void
picman_statusbar_scale_activated (PicmanScaleComboBox *combo,
                                PicmanStatusbar     *statusbar)
{
  gtk_widget_grab_focus (statusbar->shell->canvas);
}

static guint
picman_statusbar_get_context_id (PicmanStatusbar *statusbar,
                               const gchar   *context)
{
  guint id = GPOINTER_TO_UINT (g_hash_table_lookup (statusbar->context_ids,
                                                    context));

  if (! id)
    {
      id = statusbar->seq_context_id++;

      g_hash_table_insert (statusbar->context_ids,
                           g_strdup (context), GUINT_TO_POINTER (id));
    }

  return id;
}

static gboolean
picman_statusbar_temp_timeout (PicmanStatusbar *statusbar)
{
  picman_statusbar_pop_temp (statusbar);

  statusbar->temp_timeout_id = 0;

  return FALSE;
}

static void
picman_statusbar_msg_free (PicmanStatusbarMsg *msg)
{
  g_free (msg->stock_id);
  g_free (msg->text);

  g_slice_free (PicmanStatusbarMsg, msg);
}

static gchar *
picman_statusbar_vprintf (const gchar *format,
                        va_list      args)
{
  gchar *message;
  gchar *newline;

  message = g_strdup_vprintf (format, args);

  /*  guard us from multi-line strings  */
  newline = strchr (message, '\r');
  if (newline)
    *newline = '\0';

  newline = strchr (message, '\n');
  if (newline)
    *newline = '\0';

  return message;
}
