/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanProfileChooserDialog
 * Copyright (C) 2006 Sven Neumann <sven@picman.org>
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

#include <gtk/gtk.h>

#include "widgets-types.h"

#include "core/picman.h"

#include "plug-in/plug-in-icc-profile.h"

#include "picmanprofilechooserdialog.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PICMAN
};


static void   picman_profile_chooser_dialog_constructed    (GObject                  *object);
static void   picman_profile_chooser_dialog_dispose        (GObject                  *object);
static void   picman_profile_chooser_dialog_finalize       (GObject                  *object);
static void   picman_profile_chooser_dialog_set_property   (GObject                  *object,
                                                          guint                     prop_id,
                                                          const GValue             *value,
                                                          GParamSpec               *pspec);
static void   picman_profile_chooser_dialog_get_property   (GObject                  *object,
                                                          guint                     prop_id,
                                                          GValue                   *value,
                                                          GParamSpec               *pspec);

static void   picman_profile_chooser_dialog_add_shortcut   (PicmanProfileChooserDialog *dialog);
static void   picman_profile_chooser_dialog_update_preview (PicmanProfileChooserDialog *dialog);

static GtkWidget * picman_profile_view_new                 (GtkTextBuffer            *buffer);
static gboolean    picman_profile_view_query               (PicmanProfileChooserDialog *dialog);


G_DEFINE_TYPE (PicmanProfileChooserDialog, picman_profile_chooser_dialog,
               GTK_TYPE_FILE_CHOOSER_DIALOG);

#define parent_class picman_profile_chooser_dialog_parent_class


static void
picman_profile_chooser_dialog_class_init (PicmanProfileChooserDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_profile_chooser_dialog_constructed;
  object_class->dispose      = picman_profile_chooser_dialog_dispose;
  object_class->finalize     = picman_profile_chooser_dialog_finalize;
  object_class->get_property = picman_profile_chooser_dialog_get_property;
  object_class->set_property = picman_profile_chooser_dialog_set_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_profile_chooser_dialog_init (PicmanProfileChooserDialog *dialog)
{
  dialog->idle_id = 0;
  dialog->buffer  = gtk_text_buffer_new (NULL);
}

static void
picman_profile_chooser_dialog_constructed (GObject *object)
{
  PicmanProfileChooserDialog *dialog = PICMAN_PROFILE_CHOOSER_DIALOG (object);
  GtkFileFilter            *filter;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  gtk_window_set_role (GTK_WINDOW (dialog), "picman-profile-chooser-dialog");

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OPEN,   GTK_RESPONSE_ACCEPT,
                          NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_ACCEPT,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

  picman_profile_chooser_dialog_add_shortcut (dialog);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("All files (*.*)"));
  gtk_file_filter_add_pattern (filter, "*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("ICC color profile (*.icc, *.icm)"));
  gtk_file_filter_add_pattern (filter, "*.[Ii][Cc][Cc]");
  gtk_file_filter_add_pattern (filter, "*.[Ii][Cc][Mm]");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), filter);

  gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (dialog),
                                       picman_profile_view_new (dialog->buffer));

  g_signal_connect (dialog, "update-preview",
                    G_CALLBACK (picman_profile_chooser_dialog_update_preview),
                    NULL);
}

static void
picman_profile_chooser_dialog_dispose (GObject *object)
{
  PicmanProfileChooserDialog *dialog = PICMAN_PROFILE_CHOOSER_DIALOG (object);

  if (dialog->idle_id)
    {
      g_source_remove (dialog->idle_id);
      dialog->idle_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_profile_chooser_dialog_finalize (GObject *object)
{
  PicmanProfileChooserDialog *dialog = PICMAN_PROFILE_CHOOSER_DIALOG (object);

  if (dialog->buffer)
    {
      g_object_unref (dialog->buffer);
      dialog->buffer = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_profile_chooser_dialog_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  PicmanProfileChooserDialog *dialog = PICMAN_PROFILE_CHOOSER_DIALOG (object);

  switch (prop_id)
    {
    case PROP_PICMAN:
      dialog->picman = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_profile_chooser_dialog_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  PicmanProfileChooserDialog *dialog = PICMAN_PROFILE_CHOOSER_DIALOG (object);

  switch (prop_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, dialog->picman);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

GtkWidget *
picman_profile_chooser_dialog_new (Picman        *picman,
                                 const gchar *title)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_object_new (PICMAN_TYPE_PROFILE_CHOOSER_DIALOG,
                       "picman",  picman,
                       "title", title,
                       NULL);
}

gchar *
picman_profile_chooser_dialog_get_desc (PicmanProfileChooserDialog *dialog,
                                      const gchar              *filename)
{
  g_return_val_if_fail (PICMAN_IS_PROFILE_CHOOSER_DIALOG (dialog), NULL);

  if (filename && dialog->filename && strcmp (filename, dialog->filename) == 0)
    return g_strdup (dialog->desc);

  return NULL;
}

/* Add shortcut for default ICC profile location */
static void
picman_profile_chooser_dialog_add_shortcut (PicmanProfileChooserDialog *dialog)
{
#ifdef G_OS_WIN32
  {
    const gchar *prefix = g_getenv ("SystemRoot");
    gchar       *folder;

    if (! prefix)
      prefix = "c:\\windows";

    folder = g_strconcat (prefix, "\\system32\\spool\\drivers\\color", NULL);

    if (g_file_test (folder, G_FILE_TEST_IS_DIR))
      gtk_file_chooser_add_shortcut_folder (GTK_FILE_CHOOSER (dialog),
                                            folder, NULL);

    g_free (folder);
  }
#else
  {
    const gchar folder[] = "/usr/share/color/icc";

    if (g_file_test (folder, G_FILE_TEST_IS_DIR))
      gtk_file_chooser_add_shortcut_folder (GTK_FILE_CHOOSER (dialog),
                                            folder, NULL);
  }
#endif
}

static void
picman_profile_chooser_dialog_update_preview (PicmanProfileChooserDialog *dialog)
{
  gtk_text_buffer_set_text (dialog->buffer, "", 0);

  g_free (dialog->filename);
  dialog->filename = NULL;

  g_free (dialog->desc);
  dialog->desc = NULL;

  if (dialog->idle_id)
    g_source_remove (dialog->idle_id);

  dialog->idle_id = g_idle_add ((GSourceFunc) picman_profile_view_query,
                                dialog);
}

static GtkWidget *
picman_profile_view_new (GtkTextBuffer *buffer)
{
  GtkWidget *frame;
  GtkWidget *scrolled_window;
  GtkWidget *text_view;

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scrolled_window);
  gtk_widget_show (scrolled_window);

  text_view = gtk_text_view_new_with_buffer (buffer);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);

  gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (text_view), 2);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (text_view), 2);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (text_view), 2);

  gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);
  gtk_widget_show (text_view);

  gtk_widget_set_size_request (scrolled_window, 200, -1);

  return frame;
}

static gboolean
picman_profile_view_query (PicmanProfileChooserDialog *dialog)
{
  gchar *filename;

  filename = gtk_file_chooser_get_preview_filename (GTK_FILE_CHOOSER (dialog));

  if (filename)
    {
      gchar *name = NULL;
      gchar *desc = NULL;
      gchar *info = NULL;

      if (plug_in_icc_profile_file_info (dialog->picman,
                                         picman_get_user_context (dialog->picman),
                                         NULL,
                                         filename,
                                         &name, &desc, &info,
                                         NULL))
        {
          gsize info_len = info ? strlen (info) : 0;
          gsize name_len = strlen (filename);

          /*  lcms tends to adds the filename at the end of the info string.
           *  Since this is redundant information here, we remove it.
           */
          if (info_len > name_len &&
              strcmp (info + info_len - name_len, filename) == 0)
            {
              info_len -= name_len;
            }

          gtk_text_buffer_set_text (dialog->buffer, info ? info : "", info_len);

          if (desc)
            {
              dialog->desc = desc;
              desc = NULL;
            }
          else if (name)
            {
              dialog->desc = name;
              name = NULL;
            }

          dialog->filename = filename;
          filename = NULL;

          g_free (name);
          g_free (desc);
          g_free (info);
        }

      g_free (filename);
    }

  return FALSE;
}
