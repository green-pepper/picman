/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandataeditor.c
 * Copyright (C) 2002-2004 Michael Natterer <mitch@picman.org>
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
#include <gdk/gdkkeysyms.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmandata.h"
#include "core/picmandatafactory.h"

#include "picmandataeditor.h"
#include "picmandocked.h"
#include "picmanmenufactory.h"
#include "picmansessioninfo-aux.h"
#include "picmanuimanager.h"

#include "picman-intl.h"


#define DEFAULT_MINIMAL_HEIGHT 96


enum
{
  PROP_0,
  PROP_DATA_FACTORY,
  PROP_CONTEXT,
  PROP_DATA
};


static void       picman_data_editor_docked_iface_init (PicmanDockedInterface *iface);

static void       picman_data_editor_constructed       (GObject        *object);
static void       picman_data_editor_dispose           (GObject        *object);
static void       picman_data_editor_set_property      (GObject        *object,
                                                      guint           property_id,
                                                      const GValue   *value,
                                                      GParamSpec     *pspec);
static void       picman_data_editor_get_property      (GObject        *object,
                                                      guint           property_id,
                                                      GValue         *value,
                                                      GParamSpec     *pspec);

static void       picman_data_editor_style_set         (GtkWidget      *widget,
                                                      GtkStyle       *prev_style);

static void       picman_data_editor_set_context       (PicmanDocked     *docked,
                                                      PicmanContext    *context);
static void       picman_data_editor_set_aux_info      (PicmanDocked     *docked,
                                                      GList          *aux_info);
static GList    * picman_data_editor_get_aux_info      (PicmanDocked     *docked);
static gchar    * picman_data_editor_get_title         (PicmanDocked     *docked);

static void       picman_data_editor_real_set_data     (PicmanDataEditor *editor,
                                                      PicmanData       *data);

static void       picman_data_editor_data_changed      (PicmanContext    *context,
                                                      PicmanData       *data,
                                                      PicmanDataEditor *editor);
static gboolean   picman_data_editor_name_key_press    (GtkWidget      *widget,
                                                      GdkEventKey    *kevent,
                                                      PicmanDataEditor *editor);
static void       picman_data_editor_name_activate     (GtkWidget      *widget,
                                                      PicmanDataEditor *editor);
static gboolean   picman_data_editor_name_focus_out    (GtkWidget      *widget,
                                                      GdkEvent       *event,
                                                      PicmanDataEditor *editor);

static void       picman_data_editor_data_name_changed (PicmanObject     *object,
                                                      PicmanDataEditor *editor);

static void       picman_data_editor_save_clicked      (GtkWidget      *widget,
                                                      PicmanDataEditor *editor);
static void       picman_data_editor_revert_clicked    (GtkWidget      *widget,
                                                      PicmanDataEditor *editor);
static void       picman_data_editor_save_dirty        (PicmanDataEditor *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanDataEditor, picman_data_editor, PICMAN_TYPE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_data_editor_docked_iface_init))

#define parent_class picman_data_editor_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_data_editor_class_init (PicmanDataEditorClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed  = picman_data_editor_constructed;
  object_class->dispose      = picman_data_editor_dispose;
  object_class->set_property = picman_data_editor_set_property;
  object_class->get_property = picman_data_editor_get_property;

  widget_class->style_set    = picman_data_editor_style_set;

  klass->set_data            = picman_data_editor_real_set_data;

  g_object_class_install_property (object_class, PROP_DATA_FACTORY,
                                   g_param_spec_object ("data-factory",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DATA_FACTORY,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_DATA,
                                   g_param_spec_object ("data",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DATA,
                                                        PICMAN_PARAM_READWRITE));

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("minimal-height",
                                                             NULL, NULL,
                                                             32,
                                                             G_MAXINT,
                                                             DEFAULT_MINIMAL_HEIGHT,
                                                             PICMAN_PARAM_READABLE));
}

static void
picman_data_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  parent_docked_iface = g_type_interface_peek_parent (iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  iface->set_context  = picman_data_editor_set_context;
  iface->set_aux_info = picman_data_editor_set_aux_info;
  iface->get_aux_info = picman_data_editor_get_aux_info;
  iface->get_title    = picman_data_editor_get_title;
}

static void
picman_data_editor_init (PicmanDataEditor *editor)
{
  editor->data_factory  = NULL;
  editor->context       = NULL;
  editor->data          = NULL;
  editor->data_editable = FALSE;

  editor->name_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (editor), editor->name_entry, FALSE, FALSE, 0);
  gtk_widget_show (editor->name_entry);

  gtk_editable_set_editable (GTK_EDITABLE (editor->name_entry), FALSE);

  g_signal_connect (editor->name_entry, "key-press-event",
                    G_CALLBACK (picman_data_editor_name_key_press),
                    editor);
  g_signal_connect (editor->name_entry, "activate",
                    G_CALLBACK (picman_data_editor_name_activate),
                    editor);
  g_signal_connect (editor->name_entry, "focus-out-event",
                    G_CALLBACK (picman_data_editor_name_focus_out),
                    editor);
}

static void
picman_data_editor_constructed (GObject *object)
{
  PicmanDataEditor *editor = PICMAN_DATA_EDITOR (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_DATA_FACTORY (editor->data_factory));
  g_assert (PICMAN_IS_CONTEXT (editor->context));

  picman_data_editor_set_edit_active (editor, TRUE);

  editor->save_button =
    picman_editor_add_button (PICMAN_EDITOR (editor),
                            GTK_STOCK_SAVE,
                            _("Save"), NULL,
                            G_CALLBACK (picman_data_editor_save_clicked),
                            NULL,
                            editor);

  editor->revert_button =
    picman_editor_add_button (PICMAN_EDITOR (editor),
                            GTK_STOCK_REVERT_TO_SAVED,
                            _("Revert"), NULL,
                            G_CALLBACK (picman_data_editor_revert_clicked),
                            NULL,
                            editor);
  /* Hide because revert buttons are not yet implemented */
  gtk_widget_hide (editor->revert_button);
}

static void
picman_data_editor_dispose (GObject *object)
{
  PicmanDataEditor *editor = PICMAN_DATA_EDITOR (object);

  if (editor->data)
    {
      /* Save dirty data before we clear out */
      picman_data_editor_save_dirty (editor);
      picman_data_editor_set_data (editor, NULL);
    }

  if (editor->context)
    picman_docked_set_context (PICMAN_DOCKED (editor), NULL);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_data_editor_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanDataEditor *editor = PICMAN_DATA_EDITOR (object);

  switch (property_id)
    {
    case PROP_DATA_FACTORY:
      editor->data_factory = g_value_get_object (value);
      break;
    case PROP_CONTEXT:
      picman_docked_set_context (PICMAN_DOCKED (object),
                               g_value_get_object (value));
      break;
    case PROP_DATA:
      picman_data_editor_set_data (editor, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_data_editor_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanDataEditor *editor = PICMAN_DATA_EDITOR (object);

  switch (property_id)
    {
    case PROP_DATA_FACTORY:
      g_value_set_object (value, editor->data_factory);
      break;
    case PROP_CONTEXT:
      g_value_set_object (value, editor->context);
      break;
    case PROP_DATA:
      g_value_set_object (value, editor->data);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_data_editor_style_set (GtkWidget *widget,
                            GtkStyle  *prev_style)
{
  PicmanDataEditor *editor = PICMAN_DATA_EDITOR (widget);
  gint            minimal_height;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget,
                        "minimal-height", &minimal_height,
                        NULL);

  if (editor->view)
    gtk_widget_set_size_request (editor->view, -1, minimal_height);
}

static void
picman_data_editor_set_context (PicmanDocked  *docked,
                              PicmanContext *context)
{
  PicmanDataEditor *editor = PICMAN_DATA_EDITOR (docked);

  if (context == editor->context)
    return;

  if (parent_docked_iface->set_context)
    parent_docked_iface->set_context (docked, context);

  if (editor->context)
    {
      g_signal_handlers_disconnect_by_func (editor->context,
                                            picman_data_editor_data_changed,
                                            editor);

      g_object_unref (editor->context);
    }

  editor->context = context;

  if (editor->context)
    {
      GType     data_type;
      PicmanData *data;

      g_object_ref (editor->context);

      data_type = picman_data_factory_get_data_type (editor->data_factory);
      data = PICMAN_DATA (picman_context_get_by_type (editor->context, data_type));

      g_signal_connect (editor->context,
                        picman_context_type_to_signal_name (data_type),
                        G_CALLBACK (picman_data_editor_data_changed),
                        editor);

      picman_data_editor_data_changed (editor->context, data, editor);
    }
}

#define AUX_INFO_EDIT_ACTIVE  "edit-active"
#define AUX_INFO_CURRENT_DATA "current-data"

static void
picman_data_editor_set_aux_info (PicmanDocked *docked,
                               GList      *aux_info)
{
  PicmanDataEditor *editor = PICMAN_DATA_EDITOR (docked);
  GList          *list;

  parent_docked_iface->set_aux_info (docked, aux_info);

  for (list = aux_info; list; list = g_list_next (list))
    {
      PicmanSessionInfoAux *aux = list->data;

      if (! strcmp (aux->name, AUX_INFO_EDIT_ACTIVE))
        {
          gboolean edit_active;

          edit_active = ! g_ascii_strcasecmp (aux->value, "true");

          picman_data_editor_set_edit_active (editor, edit_active);
        }
      else if (! strcmp (aux->name, AUX_INFO_CURRENT_DATA))
        {
          if (! editor->edit_active)
            {
              PicmanData *data;

              data = (PicmanData *)
                picman_container_get_child_by_name (picman_data_factory_get_container (editor->data_factory),
                                                  aux->value);

              if (data)
                picman_data_editor_set_data (editor, data);
            }
        }
    }
}

static GList *
picman_data_editor_get_aux_info (PicmanDocked *docked)
{
  PicmanDataEditor     *editor = PICMAN_DATA_EDITOR (docked);
  GList              *aux_info;
  PicmanSessionInfoAux *aux;

  aux_info = parent_docked_iface->get_aux_info (docked);

  aux = picman_session_info_aux_new (AUX_INFO_EDIT_ACTIVE,
                                   editor->edit_active ? "true" : "false");
  aux_info = g_list_append (aux_info, aux);

  if (editor->data)
    {
      const gchar *value;

      value = picman_object_get_name (editor->data);

      aux = picman_session_info_aux_new (AUX_INFO_CURRENT_DATA, value);
      aux_info = g_list_append (aux_info, aux);
    }

  return aux_info;
}

static gchar *
picman_data_editor_get_title (PicmanDocked *docked)
{
  PicmanDataEditor      *editor       = PICMAN_DATA_EDITOR (docked);
  PicmanDataEditorClass *editor_class = PICMAN_DATA_EDITOR_GET_CLASS (editor);

  if (editor->data_editable)
    return g_strdup (editor_class->title);
  else
    return g_strdup_printf (_("%s (read only)"), editor_class->title);
}

static void
picman_data_editor_real_set_data (PicmanDataEditor *editor,
                                PicmanData       *data)
{
  gboolean editable;

  if (editor->data)
    {
      g_signal_handlers_disconnect_by_func (editor->data,
                                            picman_data_editor_data_name_changed,
                                            editor);

      g_object_unref (editor->data);
    }

  editor->data = data;

  if (editor->data)
    {
      g_object_ref (editor->data);

      g_signal_connect (editor->data, "name-changed",
                        G_CALLBACK (picman_data_editor_data_name_changed),
                        editor);

      gtk_entry_set_text (GTK_ENTRY (editor->name_entry),
                          picman_object_get_name (editor->data));
    }
  else
    {
      gtk_entry_set_text (GTK_ENTRY (editor->name_entry), "");
    }

  editable = (editor->data && picman_data_is_writable (editor->data));

  if (editor->data_editable != editable)
    {
      editor->data_editable = editable;

      gtk_editable_set_editable (GTK_EDITABLE (editor->name_entry), editable);
      picman_docked_title_changed (PICMAN_DOCKED (editor));
    }
}

void
picman_data_editor_set_data (PicmanDataEditor *editor,
                           PicmanData       *data)
{
  g_return_if_fail (PICMAN_IS_DATA_EDITOR (editor));
  g_return_if_fail (data == NULL || PICMAN_IS_DATA (data));
  g_return_if_fail (data == NULL ||
                    g_type_is_a (G_TYPE_FROM_INSTANCE (data),
                                 picman_data_factory_get_data_type (editor->data_factory)));

  if (editor->data != data)
    {
      PICMAN_DATA_EDITOR_GET_CLASS (editor)->set_data (editor, data);

      g_object_notify (G_OBJECT (editor), "data");

      if (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)))
        picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                                picman_editor_get_popup_data (PICMAN_EDITOR (editor)));
    }
}

PicmanData *
picman_data_editor_get_data (PicmanDataEditor *editor)
{
  g_return_val_if_fail (PICMAN_IS_DATA_EDITOR (editor), NULL);

  return editor->data;
}

void
picman_data_editor_set_edit_active (PicmanDataEditor *editor,
                                  gboolean        edit_active)
{
  g_return_if_fail (PICMAN_IS_DATA_EDITOR (editor));

  if (editor->edit_active != edit_active)
    {
      editor->edit_active = edit_active;

      if (editor->edit_active && editor->context)
        {
          GType     data_type;
          PicmanData *data;

          data_type = picman_data_factory_get_data_type (editor->data_factory);
          data = PICMAN_DATA (picman_context_get_by_type (editor->context,
                                                      data_type));

          picman_data_editor_set_data (editor, data);
        }
    }
}

gboolean
picman_data_editor_get_edit_active (PicmanDataEditor *editor)
{
  g_return_val_if_fail (PICMAN_IS_DATA_EDITOR (editor), FALSE);

  return editor->edit_active;
}


/*  private functions  */

static void
picman_data_editor_data_changed (PicmanContext    *context,
                               PicmanData       *data,
                               PicmanDataEditor *editor)
{
  if (editor->edit_active)
    picman_data_editor_set_data (editor, data);
}

static gboolean
picman_data_editor_name_key_press (GtkWidget      *widget,
                                 GdkEventKey    *kevent,
                                 PicmanDataEditor *editor)
{
  if (kevent->keyval == GDK_KEY_Escape)
    {
      gtk_entry_set_text (GTK_ENTRY (editor->name_entry),
                          picman_object_get_name (editor->data));
      return TRUE;
    }

  return FALSE;
}

static void
picman_data_editor_name_activate (GtkWidget      *widget,
                                PicmanDataEditor *editor)
{
  if (editor->data)
    {
      gchar *new_name;

      new_name = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));
      new_name = g_strstrip (new_name);

      if (strlen (new_name))
        {
          picman_object_take_name (PICMAN_OBJECT (editor->data), new_name);
        }
      else
        {
          gtk_entry_set_text (GTK_ENTRY (widget),
                              picman_object_get_name (editor->data));
          g_free (new_name);
        }
    }
}

static gboolean
picman_data_editor_name_focus_out (GtkWidget      *widget,
                                 GdkEvent       *event,
                                 PicmanDataEditor *editor)
{
  picman_data_editor_name_activate (widget, editor);

  return FALSE;
}

static void
picman_data_editor_data_name_changed (PicmanObject     *object,
                                    PicmanDataEditor *editor)
{
  gtk_entry_set_text (GTK_ENTRY (editor->name_entry),
                      picman_object_get_name (object));
}

static void
picman_data_editor_save_clicked (GtkWidget      *widget,
                               PicmanDataEditor *editor)
{
  picman_data_editor_save_dirty (editor);
}

static void
picman_data_editor_revert_clicked (GtkWidget      *widget,
                                 PicmanDataEditor *editor)
{
  g_print ("TODO: implement revert\n");
}

static void
picman_data_editor_save_dirty (PicmanDataEditor *editor)
{
  PicmanData *data = editor->data;

  if (data                      &&
      picman_data_is_dirty (data) &&
      picman_data_is_writable (data))
    {
      GError *error = NULL;

      if (! picman_data_factory_data_save_single (editor->data_factory, data,
                                                &error))
        {
          picman_message_literal (picman_data_factory_get_picman (editor->data_factory),
                                G_OBJECT (editor),
				PICMAN_MESSAGE_ERROR,
				error->message);
          g_clear_error (&error);
        }
    }
}
