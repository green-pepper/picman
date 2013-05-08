/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanImageCommentEditor
 * Copyright (C) 2007  Sven Neumann <sven@picman.org>
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
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmanimage.h"
#include "core/picmantemplate.h"

#include "picmanimagecommenteditor.h"

#include "picman-intl.h"

#define PICMAN_IMAGE_COMMENT_PARASITE "picman-comment"


static void  picman_image_comment_editor_update              (PicmanImageParasiteView  *view);

static void  picman_image_comment_editor_buffer_changed      (GtkTextBuffer          *buffer,
                                                            PicmanImageCommentEditor *editor);
static void  picman_image_comment_editor_use_default_comment (GtkWidget              *button,
                                                            PicmanImageCommentEditor *editor);


G_DEFINE_TYPE (PicmanImageCommentEditor,
               picman_image_comment_editor, PICMAN_TYPE_IMAGE_PARASITE_VIEW)

static void
picman_image_comment_editor_class_init (PicmanImageCommentEditorClass *klass)
{
  PicmanImageParasiteViewClass *view_class;

  view_class = PICMAN_IMAGE_PARASITE_VIEW_CLASS (klass);

  view_class->update = picman_image_comment_editor_update;
}

static void
picman_image_comment_editor_init (PicmanImageCommentEditor *editor)
{
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  GtkWidget *text_view;
  GtkWidget *button;

  /* Init */
  editor->recoursing = FALSE;

  /* Vbox */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (editor), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  /* Scrolled winow */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 2);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
  gtk_widget_show (scrolled_window);

  /* Text view */
  text_view = gtk_text_view_new ();

  gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), TRUE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);

  gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (text_view), 6);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (text_view), 6);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (text_view), 6);

  gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);
  gtk_widget_show (text_view);

  /* Button */
  button = gtk_button_new_with_label (_("Use default comment"));
  picman_help_set_help_data (GTK_WIDGET (button),
                           _("Replace the current image comment with the "
                             "default comment set in "
                             "Edit→Preferences→Default Image."),
                           NULL);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, TRUE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (picman_image_comment_editor_use_default_comment),
                    editor);

  /* Buffer */
  editor->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  g_signal_connect (editor->buffer, "changed",
                    G_CALLBACK (picman_image_comment_editor_buffer_changed),
                    editor);
}


/*  public functions  */

GtkWidget *
picman_image_comment_editor_new (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return g_object_new (PICMAN_TYPE_IMAGE_COMMENT_EDITOR,
                       "image",    image,
                       "parasite", PICMAN_IMAGE_COMMENT_PARASITE,
                       NULL);
}


/*  private functions  */

static void
picman_image_comment_editor_update (PicmanImageParasiteView *view)
{
  PicmanImageCommentEditor *editor = PICMAN_IMAGE_COMMENT_EDITOR (view);
  const PicmanParasite     *parasite;

  if (editor->recoursing)
    return;

  g_signal_handlers_block_by_func (editor->buffer,
                                   picman_image_comment_editor_buffer_changed,
                                   editor);

  parasite = picman_image_parasite_view_get_parasite (view);

  if (parasite)
    {
      gchar *text = g_strndup (picman_parasite_data (parasite),
                               picman_parasite_data_size (parasite));

      if (! g_utf8_validate (text, -1, NULL))
        {
          gchar *tmp = picman_any_to_utf8 (text, -1, NULL);

          g_free (text);
          text = tmp;
        }

      gtk_text_buffer_set_text (editor->buffer, text, -1);
      g_free (text);
    }
  else
    {
      gtk_text_buffer_set_text (editor->buffer, "", 0);
    }

  g_signal_handlers_unblock_by_func (editor->buffer,
                                     picman_image_comment_editor_buffer_changed,
                                     editor);
}

static void
picman_image_comment_editor_buffer_changed (GtkTextBuffer          *buffer,
                                          PicmanImageCommentEditor *editor)
{
  PicmanImage   *image;
  gchar       *text;
  gint         len;
  GtkTextIter  start;
  GtkTextIter  end;

  image =
    picman_image_parasite_view_get_image (PICMAN_IMAGE_PARASITE_VIEW (editor));

  gtk_text_buffer_get_bounds (buffer, &start, &end);

  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

  len = text ? strlen (text) : 0;

  editor->recoursing = TRUE;

  if (len > 0)
    {
      PicmanParasite *parasite;

      parasite = picman_parasite_new (PICMAN_IMAGE_COMMENT_PARASITE,
                                    PICMAN_PARASITE_PERSISTENT,
                                    len + 1, text);

      picman_image_parasite_attach (image, parasite);
      picman_parasite_free (parasite);
    }
  else
    {
      picman_image_parasite_detach (image, PICMAN_IMAGE_COMMENT_PARASITE);
    }

  editor->recoursing = FALSE;

  g_free (text);
}

static void
picman_image_comment_editor_use_default_comment (GtkWidget              *button,
                                               PicmanImageCommentEditor *editor)
{
  PicmanImage   *image;
  const gchar *comment = NULL;

  image = picman_image_parasite_view_get_image (PICMAN_IMAGE_PARASITE_VIEW (editor));

  if (image)
    {
      PicmanTemplate *template = image->picman->config->default_image;

      comment = picman_template_get_comment (template);
    }

  if (comment)
    gtk_text_buffer_set_text (editor->buffer, comment, -1);
  else
    gtk_text_buffer_set_text (editor->buffer, "", -1);
}
