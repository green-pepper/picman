/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainereditor.c
 * Copyright (C) 2001-2011 Michael Natterer <mitch@picman.org>
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

#include "core/picmancontext.h"
#include "core/picmanlist.h"
#include "core/picmanviewable.h"

#include "picmancontainereditor.h"
#include "picmancontainergridview.h"
#include "picmancontainericonview.h"
#include "picmancontainertreeview.h"
#include "picmancontainerview.h"
#include "picmandocked.h"
#include "picmanmenufactory.h"
#include "picmanviewrenderer.h"
#include "picmanuimanager.h"


enum
{
  PROP_0,
  PROP_VIEW_TYPE,
  PROP_CONTAINER,
  PROP_CONTEXT,
  PROP_VIEW_SIZE,
  PROP_VIEW_BORDER_WIDTH,
  PROP_MENU_FACTORY,
  PROP_MENU_IDENTIFIER,
  PROP_UI_PATH
};


struct _PicmanContainerEditorPrivate
{
  PicmanViewType     view_type;
  PicmanContainer   *container;
  PicmanContext     *context;
  gint             view_size;
  gint             view_border_width;
  PicmanMenuFactory *menu_factory;
  gchar           *menu_identifier;
  gchar           *ui_path;
};


static void  picman_container_editor_docked_iface_init (PicmanDockedInterface *iface);

static void   picman_container_editor_constructed      (GObject             *object);
static void   picman_container_editor_dispose          (GObject             *object);
static void   picman_container_editor_set_property     (GObject             *object,
                                                      guint                property_id,
                                                      const GValue        *value,
                                                      GParamSpec          *pspec);
static void   picman_container_editor_get_property     (GObject             *object,
                                                      guint                property_id,
                                                      GValue              *value,
                                                      GParamSpec          *pspec);

static gboolean picman_container_editor_select_item    (GtkWidget           *widget,
                                                      PicmanViewable        *viewable,
                                                      gpointer             insert_data,
                                                      PicmanContainerEditor *editor);
static void   picman_container_editor_activate_item    (GtkWidget           *widget,
                                                      PicmanViewable        *viewable,
                                                      gpointer             insert_data,
                                                      PicmanContainerEditor *editor);
static void   picman_container_editor_context_item     (GtkWidget           *widget,
                                                      PicmanViewable        *viewable,
                                                      gpointer             insert_data,
                                                      PicmanContainerEditor *editor);
static void   picman_container_editor_real_context_item(PicmanContainerEditor *editor,
                                                      PicmanViewable        *viewable);

static GtkWidget * picman_container_editor_get_preview (PicmanDocked       *docked,
                                                      PicmanContext      *context,
                                                      GtkIconSize       size);
static void        picman_container_editor_set_context (PicmanDocked       *docked,
                                                      PicmanContext      *context);
static PicmanUIManager * picman_container_editor_get_menu(PicmanDocked       *docked,
                                                      const gchar     **ui_path,
                                                      gpointer         *popup_data);

static gboolean  picman_container_editor_has_button_bar      (PicmanDocked *docked);
static void      picman_container_editor_set_show_button_bar (PicmanDocked *docked,
                                                            gboolean    show);
static gboolean  picman_container_editor_get_show_button_bar (PicmanDocked *docked);


G_DEFINE_TYPE_WITH_CODE (PicmanContainerEditor, picman_container_editor,
                         GTK_TYPE_BOX,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_container_editor_docked_iface_init))

#define parent_class picman_container_editor_parent_class


static void
picman_container_editor_class_init (PicmanContainerEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed   = picman_container_editor_constructed;
  object_class->dispose       = picman_container_editor_dispose;
  object_class->set_property  = picman_container_editor_set_property;
  object_class->get_property  = picman_container_editor_get_property;

  klass->select_item     = NULL;
  klass->activate_item   = NULL;
  klass->context_item    = picman_container_editor_real_context_item;

  g_object_class_install_property (object_class, PROP_VIEW_TYPE,
                                   g_param_spec_enum ("view-type",
                                                      NULL, NULL,
                                                      PICMAN_TYPE_VIEW_TYPE,
                                                      PICMAN_VIEW_TYPE_LIST,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CONTAINER,
                                   g_param_spec_object ("container",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTAINER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_VIEW_SIZE,
                                   g_param_spec_int ("view-size",
                                                     NULL, NULL,
                                                     1, PICMAN_VIEWABLE_MAX_PREVIEW_SIZE,
                                                     PICMAN_VIEW_SIZE_MEDIUM,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_VIEW_BORDER_WIDTH,
                                   g_param_spec_int ("view-border-width",
                                                     NULL, NULL,
                                                     0,
                                                     PICMAN_VIEW_MAX_BORDER_WIDTH,
                                                     1,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_MENU_FACTORY,
                                   g_param_spec_object ("menu-factory",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_MENU_FACTORY,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_MENU_IDENTIFIER,
                                   g_param_spec_string ("menu-identifier",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_UI_PATH,
                                   g_param_spec_string ("ui-path",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (PicmanContainerEditorPrivate));
}

static void
picman_container_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  iface->get_preview         = picman_container_editor_get_preview;
  iface->set_context         = picman_container_editor_set_context;
  iface->get_menu            = picman_container_editor_get_menu;
  iface->has_button_bar      = picman_container_editor_has_button_bar;
  iface->set_show_button_bar = picman_container_editor_set_show_button_bar;
  iface->get_show_button_bar = picman_container_editor_get_show_button_bar;
}

static void
picman_container_editor_init (PicmanContainerEditor *editor)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (editor),
                                  GTK_ORIENTATION_VERTICAL);

  editor->priv = G_TYPE_INSTANCE_GET_PRIVATE (editor,
                                              PICMAN_TYPE_CONTAINER_EDITOR,
                                              PicmanContainerEditorPrivate);
}

static void
picman_container_editor_constructed (GObject *object)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_CONTAINER (editor->priv->container));
  g_assert (PICMAN_IS_CONTEXT (editor->priv->context));

  switch (editor->priv->view_type)
    {
    case PICMAN_VIEW_TYPE_GRID:
#if 0
      editor->view =
        PICMAN_CONTAINER_VIEW (picman_container_icon_view_new (editor->priv->container,
                                                           editor->priv->context,
                                                           editor->priv->view_size,
                                                           editor->priv->view_border_width));
#else
      editor->view =
        PICMAN_CONTAINER_VIEW (picman_container_grid_view_new (editor->priv->container,
                                                           editor->priv->context,
                                                           editor->priv->view_size,
                                                           editor->priv->view_border_width));
#endif
      break;

    case PICMAN_VIEW_TYPE_LIST:
      editor->view =
        PICMAN_CONTAINER_VIEW (picman_container_tree_view_new (editor->priv->container,
                                                           editor->priv->context,
                                                           editor->priv->view_size,
                                                           editor->priv->view_border_width));
      break;

    default:
      g_assert_not_reached ();
    }

  if (PICMAN_IS_LIST (editor->priv->container))
    picman_container_view_set_reorderable (PICMAN_CONTAINER_VIEW (editor->view),
                                         ! PICMAN_LIST (editor->priv->container)->sort_func);

  if (editor->priv->menu_factory    &&
      editor->priv->menu_identifier &&
      editor->priv->ui_path)
    {
      picman_editor_create_menu (PICMAN_EDITOR (editor->view),
                               editor->priv->menu_factory,
                               editor->priv->menu_identifier,
                               editor->priv->ui_path,
                               editor);
    }

  gtk_box_pack_start (GTK_BOX (editor), GTK_WIDGET (editor->view),
                      TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (editor->view));

  g_signal_connect_object (editor->view, "select-item",
                           G_CALLBACK (picman_container_editor_select_item),
                           editor, 0);
  g_signal_connect_object (editor->view, "activate-item",
                           G_CALLBACK (picman_container_editor_activate_item),
                           editor, 0);
  g_signal_connect_object (editor->view, "context-item",
                           G_CALLBACK (picman_container_editor_context_item),
                           editor, 0);

  {
    PicmanObject *object = picman_context_get_by_type (editor->priv->context,
                                                   picman_container_get_children_type (editor->priv->container));

    picman_container_editor_select_item (GTK_WIDGET (editor->view),
                                       (PicmanViewable *) object, NULL,
                                       editor);
  }
}

static void
picman_container_editor_dispose (GObject *object)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (object);

  if (editor->priv->container)
    {
      g_object_unref (editor->priv->container);
      editor->priv->container = NULL;
    }

  if (editor->priv->context)
    {
      g_object_unref (editor->priv->context);
      editor->priv->context = NULL;
    }

  if (editor->priv->menu_factory)
    {
      g_object_unref (editor->priv->menu_factory);
      editor->priv->menu_factory = NULL;
    }

  if (editor->priv->menu_identifier)
    {
      g_free (editor->priv->menu_identifier);
      editor->priv->menu_identifier = NULL;
    }

  if (editor->priv->ui_path)
    {
      g_free (editor->priv->ui_path);
      editor->priv->ui_path = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_container_editor_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (object);

  switch (property_id)
    {
    case PROP_VIEW_TYPE:
      editor->priv->view_type = g_value_get_enum (value);
      break;

    case PROP_CONTAINER:
      editor->priv->container = g_value_dup_object (value);
      break;

    case PROP_CONTEXT:
      editor->priv->context = g_value_dup_object (value);
      break;

    case PROP_VIEW_SIZE:
      editor->priv->view_size = g_value_get_int (value);
      break;

    case PROP_VIEW_BORDER_WIDTH:
      editor->priv->view_border_width = g_value_get_int (value);
      break;

    case PROP_MENU_FACTORY:
      editor->priv->menu_factory = g_value_dup_object (value);
      break;

    case PROP_MENU_IDENTIFIER:
      editor->priv->menu_identifier = g_value_dup_string (value);
      break;

    case PROP_UI_PATH:
      editor->priv->ui_path = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_container_editor_get_property (GObject    *object,
                                    guint       property_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (object);

  switch (property_id)
    {
    case PROP_VIEW_TYPE:
      g_value_set_enum (value, editor->priv->view_type);
      break;

    case PROP_CONTAINER:
      g_value_set_object (value, editor->priv->container);
      break;

    case PROP_CONTEXT:
      g_value_set_object (value, editor->priv->context);
      break;

    case PROP_VIEW_SIZE:
      g_value_set_int (value, editor->priv->view_size);
      break;

    case PROP_VIEW_BORDER_WIDTH:
      g_value_set_int (value, editor->priv->view_border_width);
      break;

    case PROP_MENU_FACTORY:
      g_value_set_object (value, editor->priv->menu_factory);
      break;

    case PROP_MENU_IDENTIFIER:
      g_value_set_string (value, editor->priv->menu_identifier);
      break;

    case PROP_UI_PATH:
      g_value_set_string (value, editor->priv->ui_path);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkSelectionMode
picman_container_editor_get_selection_mode (PicmanContainerEditor *editor)
{
  return picman_container_view_get_selection_mode (PICMAN_CONTAINER_VIEW (editor->view));
}

void
picman_container_editor_set_selection_mode (PicmanContainerEditor *editor,
                                          GtkSelectionMode     mode)
{
  picman_container_view_set_selection_mode (PICMAN_CONTAINER_VIEW (editor->view),
                                          mode);
}

/*  private functions  */

static gboolean
picman_container_editor_select_item (GtkWidget           *widget,
                                   PicmanViewable        *viewable,
                                   gpointer             insert_data,
                                   PicmanContainerEditor *editor)
{
  PicmanContainerEditorClass *klass = PICMAN_CONTAINER_EDITOR_GET_CLASS (editor);

  if (klass->select_item)
    klass->select_item (editor, viewable);

  if (picman_editor_get_ui_manager (PICMAN_EDITOR (editor->view)))
    picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor->view)),
                            picman_editor_get_popup_data (PICMAN_EDITOR (editor->view)));

  return TRUE;
}

static void
picman_container_editor_activate_item (GtkWidget           *widget,
                                     PicmanViewable        *viewable,
                                     gpointer             insert_data,
                                     PicmanContainerEditor *editor)
{
  PicmanContainerEditorClass *klass = PICMAN_CONTAINER_EDITOR_GET_CLASS (editor);

  if (klass->activate_item)
    klass->activate_item (editor, viewable);
}

static void
picman_container_editor_context_item (GtkWidget           *widget,
                                    PicmanViewable        *viewable,
                                    gpointer             insert_data,
                                    PicmanContainerEditor *editor)
{
  PicmanContainerEditorClass *klass = PICMAN_CONTAINER_EDITOR_GET_CLASS (editor);

  if (klass->context_item)
    klass->context_item (editor, viewable);
}

static void
picman_container_editor_real_context_item (PicmanContainerEditor *editor,
                                         PicmanViewable        *viewable)
{
  PicmanContainer *container = picman_container_view_get_container (editor->view);

  if (viewable && picman_container_have (container, PICMAN_OBJECT (viewable)))
    {
      picman_editor_popup_menu (PICMAN_EDITOR (editor->view), NULL, NULL);
    }
}

static GtkWidget *
picman_container_editor_get_preview (PicmanDocked   *docked,
                                   PicmanContext  *context,
                                   GtkIconSize   size)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (docked);

  return picman_docked_get_preview (PICMAN_DOCKED (editor->view),
                                  context, size);
}

static void
picman_container_editor_set_context (PicmanDocked  *docked,
                                   PicmanContext *context)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (docked);

  picman_docked_set_context (PICMAN_DOCKED (editor->view), context);
}

static PicmanUIManager *
picman_container_editor_get_menu (PicmanDocked   *docked,
                                const gchar **ui_path,
                                gpointer     *popup_data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (docked);

  return picman_docked_get_menu (PICMAN_DOCKED (editor->view), ui_path, popup_data);
}

static gboolean
picman_container_editor_has_button_bar (PicmanDocked *docked)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (docked);

  return picman_docked_has_button_bar (PICMAN_DOCKED (editor->view));
}

static void
picman_container_editor_set_show_button_bar (PicmanDocked *docked,
                                           gboolean    show)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (docked);

  picman_docked_set_show_button_bar (PICMAN_DOCKED (editor->view), show);
}

static gboolean
picman_container_editor_get_show_button_bar (PicmanDocked *docked)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (docked);

  return picman_docked_get_show_button_bar (PICMAN_DOCKED (editor->view));
}
