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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmanlist.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanundostack.h"

#include "picmancontainertreeview.h"
#include "picmancontainerview.h"
#include "picmandocked.h"
#include "picmanhelp-ids.h"
#include "picmanmenufactory.h"
#include "picmanundoeditor.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_VIEW_SIZE
};


static void   picman_undo_editor_docked_iface_init (PicmanDockedInterface *iface);

static void   picman_undo_editor_constructed    (GObject           *object);
static void   picman_undo_editor_set_property   (GObject           *object,
                                               guint              property_id,
                                               const GValue      *value,
                                               GParamSpec        *pspec);

static void   picman_undo_editor_set_image      (PicmanImageEditor   *editor,
                                               PicmanImage         *image);

static void   picman_undo_editor_set_context    (PicmanDocked        *docked,
                                               PicmanContext       *context);

static void   picman_undo_editor_fill           (PicmanUndoEditor    *editor);
static void   picman_undo_editor_clear          (PicmanUndoEditor    *editor);

static void   picman_undo_editor_undo_event     (PicmanImage         *image,
                                               PicmanUndoEvent      event,
                                               PicmanUndo          *undo,
                                               PicmanUndoEditor    *editor);

static void   picman_undo_editor_select_item    (PicmanContainerView *view,
                                               PicmanUndo          *undo,
                                               gpointer           insert_data,
                                               PicmanUndoEditor    *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanUndoEditor, picman_undo_editor,
                         PICMAN_TYPE_IMAGE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_undo_editor_docked_iface_init))

#define parent_class picman_undo_editor_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_undo_editor_class_init (PicmanUndoEditorClass *klass)
{
  GObjectClass         *object_class       = G_OBJECT_CLASS (klass);
  PicmanImageEditorClass *image_editor_class = PICMAN_IMAGE_EDITOR_CLASS (klass);

  object_class->constructed     = picman_undo_editor_constructed;
  object_class->set_property    = picman_undo_editor_set_property;

  image_editor_class->set_image = picman_undo_editor_set_image;

  g_object_class_install_property (object_class, PROP_VIEW_SIZE,
                                   g_param_spec_enum ("view-size",
                                                      NULL, NULL,
                                                      PICMAN_TYPE_VIEW_SIZE,
                                                      PICMAN_VIEW_SIZE_LARGE,
                                                      PICMAN_PARAM_WRITABLE |
                                                      G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_undo_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  parent_docked_iface = g_type_interface_peek_parent (iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  iface->set_context = picman_undo_editor_set_context;
}

static void
picman_undo_editor_init (PicmanUndoEditor *undo_editor)
{
}

static void
picman_undo_editor_constructed (GObject *object)
{
  PicmanUndoEditor *undo_editor = PICMAN_UNDO_EDITOR (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  undo_editor->view = picman_container_tree_view_new (NULL, NULL,
                                                    undo_editor->view_size,
                                                    1);

  gtk_box_pack_start (GTK_BOX (undo_editor), undo_editor->view, TRUE, TRUE, 0);
  gtk_widget_show (undo_editor->view);

  g_signal_connect (undo_editor->view, "select-item",
                    G_CALLBACK (picman_undo_editor_select_item),
                    undo_editor);

  undo_editor->undo_button =
    picman_editor_add_action_button (PICMAN_EDITOR (undo_editor), "edit",
                                   "edit-undo", NULL);

  undo_editor->redo_button =
    picman_editor_add_action_button (PICMAN_EDITOR (undo_editor), "edit",
                                   "edit-redo", NULL);

  undo_editor->clear_button =
    picman_editor_add_action_button (PICMAN_EDITOR (undo_editor), "edit",
                                   "edit-undo-clear", NULL);
}

static void
picman_undo_editor_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanUndoEditor *undo_editor = PICMAN_UNDO_EDITOR (object);

  switch (property_id)
    {
    case PROP_VIEW_SIZE:
      undo_editor->view_size = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_undo_editor_set_image (PicmanImageEditor *image_editor,
                            PicmanImage       *image)
{
  PicmanUndoEditor *editor = PICMAN_UNDO_EDITOR (image_editor);

  if (image_editor->image)
    {
      picman_undo_editor_clear (editor);

      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_undo_editor_undo_event,
                                            editor);
    }

  PICMAN_IMAGE_EDITOR_CLASS (parent_class)->set_image (image_editor, image);

  if (image_editor->image)
    {
      if (picman_image_undo_is_enabled (image_editor->image))
        picman_undo_editor_fill (editor);

      g_signal_connect (image_editor->image, "undo-event",
                        G_CALLBACK (picman_undo_editor_undo_event),
                        editor);
    }
}

static void
picman_undo_editor_set_context (PicmanDocked  *docked,
                              PicmanContext *context)
{
  PicmanUndoEditor *editor = PICMAN_UNDO_EDITOR (docked);

  if (editor->context)
    g_object_unref (editor->context);

  editor->context = context;

  if (editor->context)
    g_object_ref (editor->context);

  /* This calls picman_undo_editor_set_image(), so make sure that it
   * isn't called before editor->context has been initialized.
   */
  parent_docked_iface->set_context (docked, context);

  picman_container_view_set_context (PICMAN_CONTAINER_VIEW (editor->view),
                                   context);
}


/*  public functions  */

GtkWidget *
picman_undo_editor_new (PicmanCoreConfig  *config,
                      PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_CORE_CONFIG (config), NULL);
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  return g_object_new (PICMAN_TYPE_UNDO_EDITOR,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<Undo>",
                       "ui-path",         "/undo-popup",
                       "view-size",       config->undo_preview_size,
                       NULL);
}


/*  private functions  */

static void
picman_undo_editor_fill (PicmanUndoEditor *editor)
{
  PicmanImage     *image      = PICMAN_IMAGE_EDITOR (editor)->image;
  PicmanUndoStack *undo_stack = picman_image_get_undo_stack (image);
  PicmanUndoStack *redo_stack = picman_image_get_redo_stack (image);
  PicmanUndo      *top_undo_item;
  GList         *list;

  /*  create a container as model for the undo history list  */
  editor->container = picman_list_new (PICMAN_TYPE_UNDO, FALSE);
  editor->base_item = g_object_new (PICMAN_TYPE_UNDO,
                                    "image", image,
                                    "name",  _("[ Base Image ]"),
                                    NULL);

  /*  the list prepends its items, so first add the redo items...  */
  for (list = PICMAN_LIST (redo_stack->undos)->list;
       list;
       list = g_list_next (list))
    {
      picman_container_add (editor->container, PICMAN_OBJECT (list->data));
    }

  /*  ...reverse the list so the redo items are in ascending order...  */
  picman_list_reverse (PICMAN_LIST (editor->container));

  /*  ...then add the undo items in descending order...  */
  for (list = PICMAN_LIST (undo_stack->undos)->list;
       list;
       list = g_list_next (list))
    {
      /*  Don't add the topmost item if it is an open undo group,
       *  it will be added upon closing of the group.
       */
      if (list->prev || ! PICMAN_IS_UNDO_STACK (list->data) ||
          picman_image_get_undo_group_count (image) == 0)
        {
          picman_container_add (editor->container, PICMAN_OBJECT (list->data));
        }
    }

  /*  ...finally, the first item is the special "base_item" which stands
   *  for the image with no more undos available to pop
   */
  picman_container_add (editor->container, PICMAN_OBJECT (editor->base_item));

  /*  display the container  */
  picman_container_view_set_container (PICMAN_CONTAINER_VIEW (editor->view),
                                     editor->container);

  top_undo_item = picman_undo_stack_peek (undo_stack);

  g_signal_handlers_block_by_func (editor->view,
                                   picman_undo_editor_select_item,
                                   editor);

  /*  select the current state of the image  */
  if (top_undo_item)
    {
      picman_container_view_select_item (PICMAN_CONTAINER_VIEW (editor->view),
                                       PICMAN_VIEWABLE (top_undo_item));
      picman_undo_create_preview (top_undo_item, editor->context, FALSE);
    }
  else
    {
      picman_container_view_select_item (PICMAN_CONTAINER_VIEW (editor->view),
                                       PICMAN_VIEWABLE (editor->base_item));
      picman_undo_create_preview (editor->base_item, editor->context, TRUE);
    }

  g_signal_handlers_unblock_by_func (editor->view,
                                     picman_undo_editor_select_item,
                                     editor);
}

static void
picman_undo_editor_clear (PicmanUndoEditor *editor)
{
  if (editor->container)
    {
      picman_container_view_set_container (PICMAN_CONTAINER_VIEW (editor->view),
                                         NULL);
      g_object_unref (editor->container);
      editor->container = NULL;
    }

  if (editor->base_item)
    {
      g_object_unref (editor->base_item);
      editor->base_item = NULL;
    }
}

static void
picman_undo_editor_undo_event (PicmanImage      *image,
                             PicmanUndoEvent   event,
                             PicmanUndo       *undo,
                             PicmanUndoEditor *editor)
{
  PicmanUndoStack *undo_stack    = picman_image_get_undo_stack (image);
  PicmanUndo      *top_undo_item = picman_undo_stack_peek (undo_stack);

  switch (event)
    {
    case PICMAN_UNDO_EVENT_UNDO_PUSHED:
      g_signal_handlers_block_by_func (editor->view,
                                       picman_undo_editor_select_item,
                                       editor);

      picman_container_insert (editor->container, PICMAN_OBJECT (undo), -1);
      picman_container_view_select_item (PICMAN_CONTAINER_VIEW (editor->view),
                                       PICMAN_VIEWABLE (undo));
      picman_undo_create_preview (undo, editor->context, FALSE);

      g_signal_handlers_unblock_by_func (editor->view,
                                         picman_undo_editor_select_item,
                                         editor);
      break;

    case PICMAN_UNDO_EVENT_UNDO_EXPIRED:
    case PICMAN_UNDO_EVENT_REDO_EXPIRED:
      picman_container_remove (editor->container, PICMAN_OBJECT (undo));
      break;

    case PICMAN_UNDO_EVENT_UNDO:
    case PICMAN_UNDO_EVENT_REDO:
      g_signal_handlers_block_by_func (editor->view,
                                       picman_undo_editor_select_item,
                                       editor);

      if (top_undo_item)
        {
          picman_container_view_select_item (PICMAN_CONTAINER_VIEW (editor->view),
                                           PICMAN_VIEWABLE (top_undo_item));
          picman_undo_create_preview (top_undo_item, editor->context, FALSE);
        }
      else
        {
          picman_container_view_select_item (PICMAN_CONTAINER_VIEW (editor->view),
                                           PICMAN_VIEWABLE (editor->base_item));
          picman_undo_create_preview (editor->base_item, editor->context, TRUE);
        }

      g_signal_handlers_unblock_by_func (editor->view,
                                         picman_undo_editor_select_item,
                                         editor);
      break;

    case PICMAN_UNDO_EVENT_UNDO_FREE:
      if (picman_image_undo_is_enabled (image))
        picman_undo_editor_clear (editor);
      break;

    case PICMAN_UNDO_EVENT_UNDO_FREEZE:
      picman_undo_editor_clear (editor);
      break;

    case PICMAN_UNDO_EVENT_UNDO_THAW:
      picman_undo_editor_fill (editor);
      break;
    }
}

static void
picman_undo_editor_select_item (PicmanContainerView *view,
                              PicmanUndo          *undo,
                              gpointer           insert_data,
                              PicmanUndoEditor    *editor)
{
  PicmanImage     *image      = PICMAN_IMAGE_EDITOR (editor)->image;
  PicmanUndoStack *undo_stack = picman_image_get_undo_stack (image);
  PicmanUndoStack *redo_stack = picman_image_get_redo_stack (image);
  PicmanUndo      *top_undo_item;

  if (! undo)
    return;

  top_undo_item = picman_undo_stack_peek (undo_stack);

  if (undo == editor->base_item)
    {
      /*  the base_item was selected, pop all available undo items
       */
      while (top_undo_item != NULL)
        {
          if (! picman_image_undo (image))
            break;

          top_undo_item = picman_undo_stack_peek (undo_stack);
        }
    }
  else if (picman_container_have (undo_stack->undos, PICMAN_OBJECT (undo)))
    {
      /*  the selected item is on the undo stack, pop undos until it
       *  is on top of the undo stack
       */
      while (top_undo_item != undo)
        {
          if(! picman_image_undo (image))
            break;

          top_undo_item = picman_undo_stack_peek (undo_stack);
        }
    }
  else if (picman_container_have (redo_stack->undos, PICMAN_OBJECT (undo)))
    {
      /*  the selected item is on the redo stack, pop redos until it
       *  is on top of the undo stack
       */
      while (top_undo_item != undo)
        {
          if (! picman_image_redo (image))
            break;

          top_undo_item = picman_undo_stack_peek (undo_stack);
        }
    }

  picman_image_flush (image);
}
