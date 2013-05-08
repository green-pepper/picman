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

#include "widgets-types.h"

#include "core/picmancontext.h"
#include "core/picmanimage.h"

#include "picmandocked.h"
#include "picmanimageeditor.h"
#include "picmanuimanager.h"


static void   picman_image_editor_docked_iface_init (PicmanDockedInterface *iface);

static void   picman_image_editor_set_context    (PicmanDocked       *docked,
                                                PicmanContext      *context);

static void   picman_image_editor_dispose        (GObject          *object);

static void   picman_image_editor_real_set_image (PicmanImageEditor  *editor,
                                                PicmanImage        *image);
static void   picman_image_editor_image_flush    (PicmanImage        *image,
                                                gboolean          invalidate_preview,
                                                PicmanImageEditor  *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanImageEditor, picman_image_editor, PICMAN_TYPE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_image_editor_docked_iface_init))

#define parent_class picman_image_editor_parent_class


static void
picman_image_editor_class_init (PicmanImageEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = picman_image_editor_dispose;

  klass->set_image      = picman_image_editor_real_set_image;
}

static void
picman_image_editor_init (PicmanImageEditor *editor)
{
  editor->image = NULL;

  gtk_widget_set_sensitive (GTK_WIDGET (editor), FALSE);
}

static void
picman_image_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  iface->set_context = picman_image_editor_set_context;
}

static void
picman_image_editor_set_context (PicmanDocked  *docked,
                               PicmanContext *context)
{
  PicmanImageEditor *editor = PICMAN_IMAGE_EDITOR (docked);
  PicmanImage       *image  = NULL;

  if (editor->context)
    {
      g_signal_handlers_disconnect_by_func (editor->context,
                                            picman_image_editor_set_image,
                                            editor);

      g_object_unref (editor->context);
    }

  editor->context = context;

  if (context)
    {
      g_object_ref (editor->context);

      g_signal_connect_swapped (context, "image-changed",
                                G_CALLBACK (picman_image_editor_set_image),
                                editor);

      image = picman_context_get_image (context);
    }

  picman_image_editor_set_image (editor, image);
}

static void
picman_image_editor_dispose (GObject *object)
{
  PicmanImageEditor *editor = PICMAN_IMAGE_EDITOR (object);

  if (editor->image)
    picman_image_editor_set_image (editor, NULL);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_image_editor_real_set_image (PicmanImageEditor *editor,
                                  PicmanImage       *image)
{
  if (editor->image)
    g_signal_handlers_disconnect_by_func (editor->image,
                                          picman_image_editor_image_flush,
                                          editor);

  editor->image = image;

  if (editor->image)
    g_signal_connect (editor->image, "flush",
                      G_CALLBACK (picman_image_editor_image_flush),
                      editor);

  gtk_widget_set_sensitive (GTK_WIDGET (editor), image != NULL);
}


/*  public functions  */

void
picman_image_editor_set_image (PicmanImageEditor *editor,
                             PicmanImage       *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE_EDITOR (editor));
  g_return_if_fail (image == NULL || PICMAN_IS_IMAGE (image));

  if (image != editor->image)
    {
      PICMAN_IMAGE_EDITOR_GET_CLASS (editor)->set_image (editor, image);

      if (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)))
        picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                                picman_editor_get_popup_data (PICMAN_EDITOR (editor)));
    }
}

PicmanImage *
picman_image_editor_get_image (PicmanImageEditor *editor)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE_EDITOR (editor), NULL);

  return editor->image;
}


/*  private functions  */

static void
picman_image_editor_image_flush (PicmanImage       *image,
                               gboolean         invalidate_preview,
                               PicmanImageEditor *editor)
{
  if (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)))
    picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                            picman_editor_get_popup_data (PICMAN_EDITOR (editor)));
}
