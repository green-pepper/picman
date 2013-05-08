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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmanchannel-select.h"
#include "core/picmancontainer.h"
#include "core/picmanimage.h"
#include "core/picmanimage-pick-color.h"
#include "core/picmanselection.h"
#include "core/picmantoolinfo.h"

/* FIXME: #include "tools/tools-types.h" */
#include "tools/tools-types.h"
#include "tools/picmanregionselectoptions.h"

#include "picmanselectioneditor.h"
#include "picmandnd.h"
#include "picmandocked.h"
#include "picmanmenufactory.h"
#include "picmanview.h"
#include "picmanviewrenderer.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


static void  picman_selection_editor_docked_iface_init (PicmanDockedInterface *iface);

static void   picman_selection_editor_constructed    (GObject             *object);

static void   picman_selection_editor_set_image      (PicmanImageEditor     *editor,
                                                    PicmanImage           *image);

static void   picman_selection_editor_set_context    (PicmanDocked          *docked,
                                                    PicmanContext         *context);

static gboolean picman_selection_view_button_press   (GtkWidget           *widget,
                                                    GdkEventButton      *bevent,
                                                    PicmanSelectionEditor *editor);
static void   picman_selection_editor_drop_color     (GtkWidget           *widget,
                                                    gint                 x,
                                                    gint                 y,
                                                    const PicmanRGB       *color,
                                                    gpointer             data);

static void   picman_selection_editor_mask_changed   (PicmanImage           *image,
                                                    PicmanSelectionEditor *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanSelectionEditor, picman_selection_editor,
                         PICMAN_TYPE_IMAGE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_selection_editor_docked_iface_init))

#define parent_class picman_selection_editor_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_selection_editor_class_init (PicmanSelectionEditorClass *klass)
{
  GObjectClass         *object_class       = G_OBJECT_CLASS (klass);
  PicmanImageEditorClass *image_editor_class = PICMAN_IMAGE_EDITOR_CLASS (klass);

  object_class->constructed     = picman_selection_editor_constructed;

  image_editor_class->set_image = picman_selection_editor_set_image;
}

static void
picman_selection_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  parent_docked_iface = g_type_interface_peek_parent (iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  iface->set_context = picman_selection_editor_set_context;
}

static void
picman_selection_editor_init (PicmanSelectionEditor *editor)
{
  GtkWidget *frame;

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (editor), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  editor->view = picman_view_new_by_types (NULL,
                                         PICMAN_TYPE_VIEW,
                                         PICMAN_TYPE_SELECTION,
                                         PICMAN_VIEW_SIZE_HUGE,
                                         0, TRUE);
  picman_view_renderer_set_background (PICMAN_VIEW (editor->view)->renderer,
                                     PICMAN_STOCK_TEXTURE);
  gtk_widget_set_size_request (editor->view,
                               PICMAN_VIEW_SIZE_HUGE, PICMAN_VIEW_SIZE_HUGE);
  picman_view_set_expand (PICMAN_VIEW (editor->view), TRUE);
  gtk_container_add (GTK_CONTAINER (frame), editor->view);
  gtk_widget_show (editor->view);

  g_signal_connect (editor->view, "button-press-event",
                    G_CALLBACK (picman_selection_view_button_press),
                    editor);

  picman_dnd_color_dest_add (editor->view,
                           picman_selection_editor_drop_color,
                           editor);

  gtk_widget_set_sensitive (GTK_WIDGET (editor), FALSE);
}

static void
picman_selection_editor_constructed (GObject *object)
{
  PicmanSelectionEditor *editor = PICMAN_SELECTION_EDITOR (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  editor->all_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor), "select",
                                   "select-all", NULL);

  editor->none_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor), "select",
                                   "select-none", NULL);

  editor->invert_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor), "select",
                                   "select-invert", NULL);

  editor->save_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor), "select",
                                   "select-save", NULL);

  editor->path_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor), "vectors",
                                   "vectors-selection-to-vectors",
                                   "vectors-selection-to-vectors-advanced",
                                   GDK_SHIFT_MASK,
                                   NULL);

  editor->stroke_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor), "select",
                                   "select-stroke",
                                   "select-stroke-last-values",
                                   GDK_SHIFT_MASK,
                                   NULL);
}

static void
picman_selection_editor_set_image (PicmanImageEditor *image_editor,
                                 PicmanImage       *image)
{
  PicmanSelectionEditor *editor = PICMAN_SELECTION_EDITOR (image_editor);

  if (image_editor->image)
    {
      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_selection_editor_mask_changed,
                                            editor);
    }

  PICMAN_IMAGE_EDITOR_CLASS (parent_class)->set_image (image_editor, image);

  if (image)
    {
      g_signal_connect (image, "mask-changed",
                        G_CALLBACK (picman_selection_editor_mask_changed),
                        editor);

      picman_view_set_viewable (PICMAN_VIEW (editor->view),
                              PICMAN_VIEWABLE (picman_image_get_mask (image)));
    }
  else
    {
      picman_view_set_viewable (PICMAN_VIEW (editor->view), NULL);
    }
}

static void
picman_selection_editor_set_context (PicmanDocked  *docked,
                                   PicmanContext *context)
{
  PicmanSelectionEditor *editor = PICMAN_SELECTION_EDITOR (docked);

  parent_docked_iface->set_context (docked, context);

  picman_view_renderer_set_context (PICMAN_VIEW (editor->view)->renderer,
                                  context);
}


/*  public functions  */

GtkWidget *
picman_selection_editor_new (PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  return g_object_new (PICMAN_TYPE_SELECTION_EDITOR,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<Selection>",
                       "ui-path",         "/selection-popup",
                       NULL);
}

static gboolean
picman_selection_view_button_press (GtkWidget           *widget,
                                  GdkEventButton      *bevent,
                                  PicmanSelectionEditor *editor)
{
  PicmanImageEditor         *image_editor = PICMAN_IMAGE_EDITOR (editor);
  PicmanViewRenderer        *renderer;
  PicmanToolInfo            *tool_info;
  PicmanSelectionOptions    *sel_options;
  PicmanRegionSelectOptions *options;
  PicmanDrawable            *drawable;
  PicmanChannelOps           operation;
  gint                     x, y;
  PicmanRGB                  color;

  if (! image_editor->image)
    return TRUE;

  renderer = PICMAN_VIEW (editor->view)->renderer;

  tool_info = picman_get_tool_info (image_editor->image->picman,
                                  "picman-by-color-select-tool");

  if (! tool_info)
    return TRUE;

  sel_options = PICMAN_SELECTION_OPTIONS (tool_info->tool_options);
  options     = PICMAN_REGION_SELECT_OPTIONS (tool_info->tool_options);

  drawable = picman_image_get_active_drawable (image_editor->image);

  if (! drawable)
    return TRUE;

  operation = picman_modifiers_to_channel_op (bevent->state);

  x = picman_image_get_width  (image_editor->image) * bevent->x / renderer->width;
  y = picman_image_get_height (image_editor->image) * bevent->y / renderer->height;

  if (picman_image_pick_color (image_editor->image, drawable, x, y,
                             options->sample_merged,
                             FALSE, 0.0,
                             NULL,
                             &color, NULL))
    {
      picman_channel_select_by_color (picman_image_get_mask (image_editor->image),
                                    drawable,
                                    options->sample_merged,
                                    &color,
                                    options->threshold,
                                    options->select_transparent,
                                    options->select_criterion,
                                    operation,
                                    sel_options->antialias,
                                    sel_options->feather,
                                    sel_options->feather_radius,
                                    sel_options->feather_radius);
      picman_image_flush (image_editor->image);
    }

  return TRUE;
}

static void
picman_selection_editor_drop_color (GtkWidget     *widget,
                                  gint           x,
                                  gint           y,
                                  const PicmanRGB *color,
                                  gpointer       data)
{
  PicmanImageEditor         *editor = PICMAN_IMAGE_EDITOR (data);
  PicmanToolInfo            *tool_info;
  PicmanSelectionOptions    *sel_options;
  PicmanRegionSelectOptions *options;
  PicmanDrawable            *drawable;

  if (! editor->image)
    return;

  tool_info = picman_get_tool_info (editor->image->picman,
                                  "picman-by-color-select-tool");
  if (! tool_info)
    return;

  sel_options = PICMAN_SELECTION_OPTIONS (tool_info->tool_options);
  options     = PICMAN_REGION_SELECT_OPTIONS (tool_info->tool_options);

  drawable = picman_image_get_active_drawable (editor->image);

  if (! drawable)
    return;

  picman_channel_select_by_color (picman_image_get_mask (editor->image),
                                drawable,
                                options->sample_merged,
                                color,
                                options->threshold,
                                options->select_transparent,
                                options->select_criterion,
                                sel_options->operation,
                                sel_options->antialias,
                                sel_options->feather,
                                sel_options->feather_radius,
                                sel_options->feather_radius);
  picman_image_flush (editor->image);
}

static void
picman_selection_editor_mask_changed (PicmanImage           *image,
                                    PicmanSelectionEditor *editor)
{
  picman_view_renderer_invalidate (PICMAN_VIEW (editor->view)->renderer);
}
