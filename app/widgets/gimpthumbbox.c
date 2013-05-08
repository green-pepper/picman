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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanthumb/picmanthumb.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimagefile.h"
#include "core/picmanprogress.h"
#include "core/picmansubprogress.h"

#include "plug-in/picmanpluginmanager.h"

#include "file/file-procedure.h"
#include "file/file-utils.h"

#include "picmanfiledialog.h" /* eek */
#include "picmanthumbbox.h"
#include "picmanview.h"
#include "picmanviewrenderer-frame.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void     picman_thumb_box_progress_iface_init (PicmanProgressInterface *iface);

static void     picman_thumb_box_dispose            (GObject           *object);
static void     picman_thumb_box_finalize           (GObject           *object);

static void     picman_thumb_box_style_set          (GtkWidget         *widget,
                                                   GtkStyle          *prev_style);

static PicmanProgress *
                picman_thumb_box_progress_start     (PicmanProgress      *progress,
                                                   const gchar       *message,
                                                   gboolean           cancelable);
static void     picman_thumb_box_progress_end       (PicmanProgress      *progress);
static gboolean picman_thumb_box_progress_is_active (PicmanProgress      *progress);
static void     picman_thumb_box_progress_set_value (PicmanProgress      *progress,
                                                   gdouble            percentage);
static gdouble  picman_thumb_box_progress_get_value (PicmanProgress      *progress);
static void     picman_thumb_box_progress_pulse     (PicmanProgress      *progress);

static gboolean picman_thumb_box_progress_message   (PicmanProgress      *progress,
                                                   Picman              *picman,
                                                   PicmanMessageSeverity  severity,
                                                   const gchar       *domain,
                                                   const gchar       *message);

static gboolean picman_thumb_box_ebox_button_press  (GtkWidget         *widget,
                                                   GdkEventButton    *bevent,
                                                   PicmanThumbBox      *box);
static void picman_thumb_box_thumbnail_clicked      (GtkWidget         *widget,
                                                   GdkModifierType    state,
                                                   PicmanThumbBox      *box);
static void picman_thumb_box_imagefile_info_changed (PicmanImagefile     *imagefile,
                                                   PicmanThumbBox      *box);
static void picman_thumb_box_thumb_state_notify     (PicmanThumbnail     *thumb,
                                                   GParamSpec        *pspec,
                                                   PicmanThumbBox      *box);
static void picman_thumb_box_create_thumbnails      (PicmanThumbBox      *box,
                                                   gboolean           force);
static void picman_thumb_box_create_thumbnail       (PicmanThumbBox      *box,
                                                   const gchar       *uri,
                                                   PicmanThumbnailSize  size,
                                                   gboolean           force,
                                                   PicmanProgress      *progress);
static gboolean picman_thumb_box_auto_thumbnail     (PicmanThumbBox      *box);


G_DEFINE_TYPE_WITH_CODE (PicmanThumbBox, picman_thumb_box, GTK_TYPE_FRAME,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROGRESS,
                                                picman_thumb_box_progress_iface_init))

#define parent_class picman_thumb_box_parent_class


static void
picman_thumb_box_class_init (PicmanThumbBoxClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose     = picman_thumb_box_dispose;
  object_class->finalize    = picman_thumb_box_finalize;

  widget_class->style_set   = picman_thumb_box_style_set;
}

static void
picman_thumb_box_init (PicmanThumbBox *box)
{
  gtk_frame_set_shadow_type (GTK_FRAME (box), GTK_SHADOW_IN);

  box->idle_id = 0;
}

static void
picman_thumb_box_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start     = picman_thumb_box_progress_start;
  iface->end       = picman_thumb_box_progress_end;
  iface->is_active = picman_thumb_box_progress_is_active;
  iface->set_value = picman_thumb_box_progress_set_value;
  iface->get_value = picman_thumb_box_progress_get_value;
  iface->pulse     = picman_thumb_box_progress_pulse;
  iface->message   = picman_thumb_box_progress_message;
}

static void
picman_thumb_box_dispose (GObject *object)
{
  PicmanThumbBox *box = PICMAN_THUMB_BOX (object);

  if (box->idle_id)
    {
      g_source_remove (box->idle_id);
      box->idle_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);

  box->progress = NULL;
}

static void
picman_thumb_box_finalize (GObject *object)
{
  PicmanThumbBox *box = PICMAN_THUMB_BOX (object);

  picman_thumb_box_take_uris (box, NULL);

  if (box->imagefile)
    {
      g_object_unref (box->imagefile);
      box->imagefile = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_thumb_box_style_set (GtkWidget *widget,
                          GtkStyle  *prev_style)
{
  PicmanThumbBox *box   = PICMAN_THUMB_BOX (widget);
  GtkStyle     *style = gtk_widget_get_style (widget);
  GtkWidget    *ebox;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_modify_bg (box->preview, GTK_STATE_NORMAL,
                        &style->base[GTK_STATE_NORMAL]);
  gtk_widget_modify_bg (box->preview, GTK_STATE_INSENSITIVE,
                        &style->base[GTK_STATE_NORMAL]);

  ebox = gtk_bin_get_child (GTK_BIN (widget));

  gtk_widget_modify_bg (ebox, GTK_STATE_NORMAL,
                        &style->base[GTK_STATE_NORMAL]);
  gtk_widget_modify_bg (ebox, GTK_STATE_INSENSITIVE,
                        &style->base[GTK_STATE_NORMAL]);
}

static PicmanProgress *
picman_thumb_box_progress_start (PicmanProgress *progress,
                               const gchar  *message,
                               gboolean      cancelable)
{
  PicmanThumbBox *box = PICMAN_THUMB_BOX (progress);

  if (! box->progress)
    return NULL;

  if (! box->progress_active)
    {
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);
      GtkWidget      *toplevel;

      gtk_progress_bar_set_fraction (bar, 0.0);

      box->progress_active = TRUE;

      toplevel = gtk_widget_get_toplevel (GTK_WIDGET (box));

      if (PICMAN_IS_FILE_DIALOG (toplevel))
        gtk_dialog_set_response_sensitive (GTK_DIALOG (toplevel),
                                           GTK_RESPONSE_CANCEL, cancelable);

      return progress;
    }

  return NULL;
}

static void
picman_thumb_box_progress_end (PicmanProgress *progress)
{
  if (picman_thumb_box_progress_is_active (progress))
    {
      PicmanThumbBox   *box = PICMAN_THUMB_BOX (progress);
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_progress_bar_set_fraction (bar, 0.0);

      box->progress_active = FALSE;
    }
}

static gboolean
picman_thumb_box_progress_is_active (PicmanProgress *progress)
{
  PicmanThumbBox *box = PICMAN_THUMB_BOX (progress);

  return (box->progress && box->progress_active);
}

static void
picman_thumb_box_progress_set_value (PicmanProgress *progress,
                                   gdouble       percentage)
{
  if (picman_thumb_box_progress_is_active (progress))
    {
      PicmanThumbBox   *box = PICMAN_THUMB_BOX (progress);
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_progress_bar_set_fraction (bar, percentage);
    }
}

static gdouble
picman_thumb_box_progress_get_value (PicmanProgress *progress)
{
  if (picman_thumb_box_progress_is_active (progress))
    {
      PicmanThumbBox   *box = PICMAN_THUMB_BOX (progress);
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      return gtk_progress_bar_get_fraction (bar);
    }

  return 0.0;
}

static void
picman_thumb_box_progress_pulse (PicmanProgress *progress)
{
  if (picman_thumb_box_progress_is_active (progress))
    {
      PicmanThumbBox   *box = PICMAN_THUMB_BOX (progress);
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_progress_bar_pulse (bar);
    }
}

static gboolean
picman_thumb_box_progress_message (PicmanProgress        *progress,
                                 Picman                *picman,
                                 PicmanMessageSeverity  severity,
                                 const gchar         *domain,
                                 const gchar         *message)
{
  /*  PicmanThumbBox never shows any messages  */

  return TRUE;
}


/*  public functions  */

GtkWidget *
picman_thumb_box_new (PicmanContext *context)
{
  PicmanThumbBox   *box;
  GtkWidget      *vbox;
  GtkWidget      *vbox2;
  GtkWidget      *ebox;
  GtkWidget      *hbox;
  GtkWidget      *button;
  GtkWidget      *label;
  gchar          *str;
  gint            h, v;
  GtkRequisition  info_requisition;
  GtkRequisition  progress_requisition;

  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  box = g_object_new (PICMAN_TYPE_THUMB_BOX, NULL);

  box->context = context;

  ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (box), ebox);
  gtk_widget_show (ebox);

  g_signal_connect (ebox, "button-press-event",
                    G_CALLBACK (picman_thumb_box_ebox_button_press),
                    box);

  str = g_strdup_printf (_("Click to update preview\n"
                           "%s-Click to force update even "
                           "if preview is up-to-date"),
                         picman_get_mod_string (picman_get_toggle_behavior_mask ()));

  picman_help_set_help_data (ebox, str, NULL);

  g_free (str);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (ebox), vbox);
  gtk_widget_show (vbox);

  button = gtk_button_new ();
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  label = gtk_label_new_with_mnemonic (_("Pr_eview"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_widget_show (label);

  g_signal_connect (button, "button-press-event",
                    G_CALLBACK (gtk_true),
                    NULL);
  g_signal_connect (button, "button-release-event",
                    G_CALLBACK (gtk_true),
                    NULL);
  g_signal_connect (button, "enter-notify-event",
                    G_CALLBACK (gtk_true),
                    NULL);
  g_signal_connect (button, "leave-notify-event",
                    G_CALLBACK (gtk_true),
                    NULL);

  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 2);
  gtk_box_pack_start (GTK_BOX (vbox), vbox2, TRUE, TRUE, 0);
  gtk_widget_show (vbox2);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  box->imagefile = picman_imagefile_new (context->picman, NULL);

  g_signal_connect (box->imagefile, "info-changed",
                    G_CALLBACK (picman_thumb_box_imagefile_info_changed),
                    box);

  g_signal_connect (picman_imagefile_get_thumbnail (box->imagefile),
                    "notify::thumb-state",
                    G_CALLBACK (picman_thumb_box_thumb_state_notify),
                    box);

  picman_view_renderer_get_frame_size (&h, &v);

  box->preview = picman_view_new (context,
                                PICMAN_VIEWABLE (box->imagefile),
                                /* add padding for the shadow frame */
                                context->picman->config->thumbnail_size +
                                MAX (h, v),
                                0, FALSE);

  gtk_box_pack_start (GTK_BOX (hbox), box->preview, TRUE, FALSE, 2);
  gtk_widget_show (box->preview);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), box->preview);

  g_signal_connect (box->preview, "clicked",
                    G_CALLBACK (picman_thumb_box_thumbnail_clicked),
                    box);

  box->filename = gtk_label_new (_("No selection"));
  gtk_label_set_ellipsize (GTK_LABEL (box->filename), PANGO_ELLIPSIZE_MIDDLE);
  gtk_label_set_justify (GTK_LABEL (box->filename), GTK_JUSTIFY_CENTER);
  picman_label_set_attributes (GTK_LABEL (box->filename),
                             PANGO_ATTR_STYLE, PANGO_STYLE_OBLIQUE,
                             -1);
  gtk_box_pack_start (GTK_BOX (vbox2), box->filename, FALSE, FALSE, 0);
  gtk_widget_show (box->filename);

  box->info = gtk_label_new (" \n \n \n ");
  gtk_misc_set_alignment (GTK_MISC (box->info), 0.5, 0.0);
  gtk_label_set_justify (GTK_LABEL (box->info), GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (GTK_LABEL (box->info), TRUE);
  picman_label_set_attributes (GTK_LABEL (box->info),
                             PANGO_ATTR_SCALE, PANGO_SCALE_SMALL,
                             -1);
  gtk_box_pack_start (GTK_BOX (vbox2), box->info, FALSE, FALSE, 0);
  gtk_widget_show (box->info);

  box->progress = gtk_progress_bar_new ();
  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (box->progress), "Fog");
  gtk_box_pack_end (GTK_BOX (vbox2), box->progress, FALSE, FALSE, 0);
  gtk_widget_set_no_show_all (box->progress, TRUE);
  /* don't gtk_widget_show (box->progress); */

  /* eek */
  gtk_widget_size_request (box->info,     &info_requisition);
  gtk_widget_size_request (box->progress, &progress_requisition);

  gtk_widget_set_size_request (box->info,
                               -1, info_requisition.height);
  gtk_widget_set_size_request (box->filename,
                               progress_requisition.width, -1);

  gtk_widget_set_size_request (box->progress,
                               -1, progress_requisition.height);
  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (box->progress), "");

  return GTK_WIDGET (box);
}

void
picman_thumb_box_take_uri (PicmanThumbBox *box,
                         gchar        *uri)
{
  g_return_if_fail (PICMAN_IS_THUMB_BOX (box));

  if (box->idle_id)
    {
      g_source_remove (box->idle_id);
      box->idle_id = 0;
    }

  picman_object_take_name (PICMAN_OBJECT (box->imagefile), uri);

  if (uri)
    {
      gchar *basename = file_utils_uri_display_basename (uri);

      gtk_label_set_text (GTK_LABEL (box->filename), basename);
      g_free (basename);
    }
  else
    {
      gtk_label_set_text (GTK_LABEL (box->filename), _("No selection"));
    }

  gtk_widget_set_sensitive (GTK_WIDGET (box), uri != NULL);
  picman_imagefile_update (box->imagefile);
}

void
picman_thumb_box_take_uris (PicmanThumbBox *box,
                          GSList       *uris)
{
  g_return_if_fail (PICMAN_IS_THUMB_BOX (box));

  if (box->uris)
    {
      g_slist_free_full (box->uris, (GDestroyNotify) g_free);
      box->uris = NULL;
    }

  box->uris = uris;
}


/*  private functions  */

static gboolean
picman_thumb_box_ebox_button_press (GtkWidget      *widget,
                                  GdkEventButton *bevent,
                                  PicmanThumbBox   *box)
{
  picman_thumb_box_thumbnail_clicked (widget, bevent->state, box);

  return TRUE;
}

static void
picman_thumb_box_thumbnail_clicked (GtkWidget       *widget,
                                  GdkModifierType  state,
                                  PicmanThumbBox    *box)
{
  picman_thumb_box_create_thumbnails (box,
                                    (state & picman_get_toggle_behavior_mask ()) ?
                                    TRUE : FALSE);
}

static void
picman_thumb_box_imagefile_info_changed (PicmanImagefile *imagefile,
                                       PicmanThumbBox  *box)
{
  gtk_label_set_text (GTK_LABEL (box->info),
                      picman_imagefile_get_desc_string (imagefile));
}

static void
picman_thumb_box_thumb_state_notify (PicmanThumbnail *thumb,
                                   GParamSpec    *pspec,
                                   PicmanThumbBox  *box)
{
  if (box->idle_id)
    return;

  if (thumb->image_state == PICMAN_THUMB_STATE_REMOTE)
    return;

  switch (thumb->thumb_state)
    {
    case PICMAN_THUMB_STATE_NOT_FOUND:
    case PICMAN_THUMB_STATE_OLD:
      box->idle_id =
        g_idle_add_full (G_PRIORITY_LOW,
                         (GSourceFunc) picman_thumb_box_auto_thumbnail,
                         box, NULL);
      break;

    default:
      break;
    }
}

static void
picman_thumb_box_create_thumbnails (PicmanThumbBox *box,
                                  gboolean      force)
{
  Picman           *picman     = box->context->picman;
  PicmanProgress   *progress = PICMAN_PROGRESS (box);
  PicmanFileDialog *dialog   = NULL;
  GtkWidget      *toplevel;
  GSList         *list;
  gint            n_uris;
  gint            i;

  if (picman->config->thumbnail_size == PICMAN_THUMBNAIL_SIZE_NONE)
    return;

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (box));

  if (PICMAN_IS_FILE_DIALOG (toplevel))
    dialog = PICMAN_FILE_DIALOG (toplevel);

  picman_set_busy (picman);

  if (dialog)
    picman_file_dialog_set_sensitive (dialog, FALSE);
  else
    gtk_widget_set_sensitive (toplevel, FALSE);

  if (box->uris)
    {
      gtk_widget_hide (box->info);
      gtk_widget_show (box->progress);
    }

  n_uris = g_slist_length (box->uris);

  if (n_uris > 1)
    {
      gchar *str;

      picman_progress_start (PICMAN_PROGRESS (box), "", TRUE);

      progress = picman_sub_progress_new (PICMAN_PROGRESS (box));

      picman_sub_progress_set_step (PICMAN_SUB_PROGRESS (progress), 0, n_uris);

      for (list = box->uris->next, i = 1;
           list;
           list = g_slist_next (list), i++)
        {
          str = g_strdup_printf (_("Thumbnail %d of %d"), i, n_uris);
          gtk_progress_bar_set_text (GTK_PROGRESS_BAR (box->progress), str);
          g_free (str);

          picman_progress_set_value (progress, 0.0);

          while (gtk_events_pending ())
            gtk_main_iteration ();

          picman_thumb_box_create_thumbnail (box,
                                           list->data,
                                           picman->config->thumbnail_size,
                                           force,
                                           progress);

          if (dialog && dialog->canceled)
            goto canceled;

          picman_sub_progress_set_step (PICMAN_SUB_PROGRESS (progress), i, n_uris);
        }

      str = g_strdup_printf (_("Thumbnail %d of %d"), n_uris, n_uris);
      gtk_progress_bar_set_text (GTK_PROGRESS_BAR (box->progress), str);
      g_free (str);

      picman_progress_set_value (progress, 0.0);

      while (gtk_events_pending ())
        gtk_main_iteration ();
    }

  if (box->uris)
    {
      picman_thumb_box_create_thumbnail (box,
                                       box->uris->data,
                                       picman->config->thumbnail_size,
                                       force,
                                       progress);

      picman_progress_set_value (progress, 1.0);
    }

 canceled:

  if (n_uris > 1)
    {
      g_object_unref (progress);

      picman_progress_end (PICMAN_PROGRESS (box));
      gtk_progress_bar_set_text (GTK_PROGRESS_BAR (box->progress), "");
    }

  if (box->uris)
    {
      gtk_widget_hide (box->progress);
      gtk_widget_show (box->info);
    }

  if (dialog)
    picman_file_dialog_set_sensitive (dialog, TRUE);
  else
    gtk_widget_set_sensitive (toplevel, TRUE);

  picman_unset_busy (picman);
}

static void
picman_thumb_box_create_thumbnail (PicmanThumbBox      *box,
                                 const gchar       *uri,
                                 PicmanThumbnailSize  size,
                                 gboolean           force,
                                 PicmanProgress      *progress)
{
  gchar         *filename = file_utils_filename_from_uri (uri);
  PicmanThumbnail *thumb;
  gchar         *basename;

  if (filename)
    {
      gboolean regular = g_file_test (filename, G_FILE_TEST_IS_REGULAR);

      g_free (filename);

      if (! regular)
        return;
    }

  thumb = picman_imagefile_get_thumbnail (box->imagefile);

  basename = file_utils_uri_display_basename (uri);
  gtk_label_set_text (GTK_LABEL (box->filename), basename);
  g_free (basename);

  picman_object_set_name (PICMAN_OBJECT (box->imagefile), uri);

  if (force ||
      (picman_thumbnail_peek_thumb (thumb, size) < PICMAN_THUMB_STATE_FAILED &&
       ! picman_thumbnail_has_failed (thumb)))
    {
      picman_imagefile_create_thumbnail (box->imagefile, box->context,
                                       progress,
                                       size,
                                       !force);
    }
}

static gboolean
picman_thumb_box_auto_thumbnail (PicmanThumbBox *box)
{
  Picman          *picman  = box->context->picman;
  PicmanThumbnail *thumb = picman_imagefile_get_thumbnail (box->imagefile);
  const gchar   *uri   = picman_object_get_name (box->imagefile);

  box->idle_id = 0;

  if (thumb->image_state == PICMAN_THUMB_STATE_NOT_FOUND)
    return FALSE;

  switch (thumb->thumb_state)
    {
    case PICMAN_THUMB_STATE_NOT_FOUND:
    case PICMAN_THUMB_STATE_OLD:
      if (thumb->image_filesize < picman->config->thumbnail_filesize_limit &&
          ! picman_thumbnail_has_failed (thumb)                            &&
          file_procedure_find_by_extension (picman->plug_in_manager->load_procs,
                                            uri))
        {
          if (thumb->image_filesize > 0)
            {
              gchar *size;
              gchar *text;

              size = g_format_size (thumb->image_filesize);
              text = g_strdup_printf ("%s\n%s",
                                      size, _("Creating preview..."));

              gtk_label_set_text (GTK_LABEL (box->info), text);

              g_free (text);
              g_free (size);
            }
          else
            {
              gtk_label_set_text (GTK_LABEL (box->info),
                                  _("Creating preview..."));
            }

          picman_imagefile_create_thumbnail_weak (box->imagefile, box->context,
                                                PICMAN_PROGRESS (box),
                                                picman->config->thumbnail_size,
                                                TRUE);
        }
      break;

    default:
      break;
    }

  return FALSE;
}
