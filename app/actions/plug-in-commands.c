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
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmandrawable.h"
#include "core/picmanimage.h"
#include "core/picmanitem.h"
#include "core/picmanparamspecs.h"
#include "core/picmanprogress.h"

#include "plug-in/picmanpluginmanager.h"
#include "plug-in/picmanpluginmanager-data.h"
#include "plug-in/picmanpluginmanager-history.h"

#include "pdb/picmanprocedure.h"

#include "widgets/picmanbufferview.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmandatafactoryview.h"
#include "widgets/picmanfontview.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanimageeditor.h"
#include "widgets/picmanitemtreeview.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"

#include "display/picmandisplay.h"

#include "actions.h"
#include "plug-in-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void  plug_in_procedure_execute     (PicmanPlugInProcedure *procedure,
                                            Picman                *picman,
                                            PicmanDisplay         *display,
                                            PicmanValueArray      *args,
                                            gint                 n_args);

static gint  plug_in_collect_data_args     (GtkAction       *action,
                                            PicmanObject      *object,
                                            GParamSpec     **pspecs,
                                            PicmanValueArray  *args,
                                            gint             n_args);
static gint  plug_in_collect_image_args    (GtkAction       *action,
                                            PicmanImage       *image,
                                            GParamSpec     **pspecs,
                                            PicmanValueArray  *args,
                                            gint             n_args);
static gint  plug_in_collect_item_args     (GtkAction       *action,
                                            PicmanImage       *image,
                                            PicmanItem        *item,
                                            GParamSpec     **pspecs,
                                            PicmanValueArray  *args,
                                            gint             n_args);
static gint  plug_in_collect_display_args  (GtkAction       *action,
                                            PicmanDisplay     *display,
                                            GParamSpec     **pspecs,
                                            PicmanValueArray  *args,
                                            gint             n_args);
static void  plug_in_reset_all_response    (GtkWidget       *dialog,
                                            gint             response_id,
                                            Picman            *picman);


/*  public functions  */

void
plug_in_run_cmd_callback (GtkAction           *action,
                          PicmanPlugInProcedure *proc,
                          gpointer             data)
{
  PicmanProcedure  *procedure = PICMAN_PROCEDURE (proc);
  Picman           *picman;
  PicmanValueArray *args;
  gint            n_args    = 0;
  PicmanDisplay    *display   = NULL;
  return_if_no_picman (picman, data);

  args = picman_procedure_get_arguments (procedure);

  /* initialize the first argument  */
  g_value_set_int (picman_value_array_index (args, n_args),
                   PICMAN_RUN_INTERACTIVE);
  n_args++;

  switch (procedure->proc_type)
    {
    case PICMAN_EXTENSION:
      break;

    case PICMAN_PLUGIN:
    case PICMAN_TEMPORARY:
      if (PICMAN_IS_DATA_FACTORY_VIEW (data) ||
          PICMAN_IS_FONT_VIEW (data)         ||
          PICMAN_IS_BUFFER_VIEW (data))
        {
          PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
          PicmanContainer       *container;
          PicmanContext         *context;
          PicmanObject          *object;

          container = picman_container_view_get_container (editor->view);
          context   = picman_container_view_get_context (editor->view);

          object = picman_context_get_by_type (context,
                                             picman_container_get_children_type (container));

          n_args = plug_in_collect_data_args (action, object,
                                              procedure->args,
                                              args, n_args);
        }
      else if (PICMAN_IS_IMAGE_EDITOR (data))
        {
          PicmanImageEditor *editor = PICMAN_IMAGE_EDITOR (data);
          PicmanImage       *image;

          image = picman_image_editor_get_image (editor);

          n_args = plug_in_collect_image_args (action, image,
                                               procedure->args,
                                               args, n_args);
        }
      else if (PICMAN_IS_ITEM_TREE_VIEW (data))
        {
          PicmanItemTreeView *view = PICMAN_ITEM_TREE_VIEW (data);
          PicmanImage        *image;
          PicmanItem         *item;

          image = picman_item_tree_view_get_image (view);

          if (image)
            item = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);
          else
            item = NULL;

          n_args = plug_in_collect_item_args (action, image, item,
                                              procedure->args,
                                              args, n_args);
        }
      else
        {
          display = action_data_get_display (data);

          n_args = plug_in_collect_display_args (action,
                                                 display,
                                                 procedure->args,
                                                 args, n_args);
        }
      break;

    case PICMAN_INTERNAL:
      g_warning ("Unhandled procedure type.");
      n_args = -1;
      break;
    }

  if (n_args >= 1)
    plug_in_procedure_execute (proc, picman, display, args, n_args);

  picman_value_array_unref (args);
}

void
plug_in_repeat_cmd_callback (GtkAction *action,
                             gint       value,
                             gpointer   data)
{
  PicmanPlugInProcedure *procedure;
  Picman                *picman;
  PicmanDisplay         *display;
  PicmanRunMode          run_mode;
  return_if_no_picman (picman, data);
  return_if_no_display (display, data);

  run_mode = (PicmanRunMode) value;

  procedure = picman_plug_in_manager_history_nth (picman->plug_in_manager, 0);

  if (procedure)
    {
      PicmanValueArray *args;
      gint            n_args;

      args = picman_procedure_get_arguments (PICMAN_PROCEDURE (procedure));

      g_value_set_int (picman_value_array_index (args, 0), run_mode);

      n_args = plug_in_collect_display_args (action, display,
                                             PICMAN_PROCEDURE (procedure)->args,
                                             args, 1);

      plug_in_procedure_execute (procedure, picman, display, args, n_args);

      picman_value_array_unref (args);
    }
}

void
plug_in_history_cmd_callback (GtkAction           *action,
                              PicmanPlugInProcedure *procedure,
                              gpointer             data)
{
  Picman           *picman;
  PicmanDisplay    *display;
  PicmanValueArray *args;
  gint            n_args;
  return_if_no_picman (picman, data);
  return_if_no_display (display, data);

  args = picman_procedure_get_arguments (PICMAN_PROCEDURE (procedure));

  g_value_set_int (picman_value_array_index (args, 0), PICMAN_RUN_INTERACTIVE);

  n_args = plug_in_collect_display_args (action, display,
                                         PICMAN_PROCEDURE (procedure)->args,
                                         args, 1);

  plug_in_procedure_execute (procedure, picman, display, args, n_args);

  picman_value_array_unref (args);
}

void
plug_in_reset_all_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  Picman      *picman;
  GtkWidget *dialog;
  return_if_no_picman (picman, data);

  dialog = picman_message_dialog_new (_("Reset all Filters"), PICMAN_STOCK_QUESTION,
                                    NULL, 0,
                                    picman_standard_help_func, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    PICMAN_STOCK_RESET, GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (plug_in_reset_all_response),
                    picman);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("Do you really want to reset all "
                                       "filters to default values?"));

  gtk_widget_show (dialog);
}


/*  private functions  */

static void
plug_in_procedure_execute (PicmanPlugInProcedure *procedure,
                           Picman                *picman,
                           PicmanDisplay         *display,
                           PicmanValueArray      *args,
                           gint                 n_args)
{
  GError *error = NULL;

  picman_value_array_truncate (args, n_args);

  /* run the plug-in procedure */
  picman_procedure_execute_async (PICMAN_PROCEDURE (procedure), picman,
                                picman_get_user_context (picman),
                                PICMAN_PROGRESS (display), args,
                                PICMAN_OBJECT (display), &error);

  if (error)
    {
      picman_message_literal (picman,
			    G_OBJECT (display), PICMAN_MESSAGE_ERROR,
			    error->message);
      g_error_free (error);
    }
  else
    {
      /* remember only image plug-ins */
      if (PICMAN_PROCEDURE (procedure)->num_args  >= 2  &&
          PICMAN_IS_PARAM_SPEC_IMAGE_ID (PICMAN_PROCEDURE (procedure)->args[1]))
        {
          picman_plug_in_manager_history_add (picman->plug_in_manager, procedure);
        }
    }
}

static gint
plug_in_collect_data_args (GtkAction       *action,
                           PicmanObject      *object,
                           GParamSpec     **pspecs,
                           PicmanValueArray  *args,
                           gint             n_args)
{
  if (picman_value_array_length (args) > n_args &&
      PICMAN_IS_PARAM_SPEC_STRING (pspecs[n_args]))
    {
      if (object)
        {
          g_value_set_string (picman_value_array_index (args, n_args),
                              picman_object_get_name (object));
          n_args++;
        }
      else
        {
          g_warning ("Uh-oh, no active data object for the plug-in!");
          return -1;
        }
    }

  return n_args;
}

static gint
plug_in_collect_image_args (GtkAction       *action,
                            PicmanImage       *image,
                            GParamSpec     **pspecs,
                            PicmanValueArray  *args,
                            gint             n_args)
{
  if (picman_value_array_length (args) > n_args &&
      PICMAN_IS_PARAM_SPEC_IMAGE_ID (pspecs[n_args]))
    {
      if (image)
        {
          picman_value_set_image (picman_value_array_index (args, n_args), image);
          n_args++;
        }
      else
        {
          g_warning ("Uh-oh, no active image for the plug-in!");
          return -1;
        }
    }

  return n_args;
}

static gint
plug_in_collect_item_args (GtkAction       *action,
                           PicmanImage       *image,
                           PicmanItem        *item,
                           GParamSpec     **pspecs,
                           PicmanValueArray  *args,
                           gint             n_args)
{
  if (picman_value_array_length (args) > n_args &&
      PICMAN_IS_PARAM_SPEC_IMAGE_ID (pspecs[n_args]))
    {
      if (image)
        {
          picman_value_set_image (picman_value_array_index (args, n_args), image);
          n_args++;

          if (picman_value_array_length (args) > n_args &&
              PICMAN_IS_PARAM_SPEC_ITEM_ID (pspecs[n_args]))
            {
              if (item &&
                  g_type_is_a (G_TYPE_FROM_INSTANCE (item),
                               PICMAN_PARAM_SPEC_ITEM_ID (pspecs[n_args])->item_type))
                {
                  picman_value_set_item (picman_value_array_index (args, n_args),
                                       item);
                  n_args++;
                }
              else
                {
                  g_warning ("Uh-oh, no active item for the plug-in!");
                  return -1;
                }
            }
        }
    }

  return n_args;
}

static gint
plug_in_collect_display_args (GtkAction       *action,
                              PicmanDisplay     *display,
                              GParamSpec     **pspecs,
                              PicmanValueArray  *args,
                              gint             n_args)
{
  if (picman_value_array_length (args) > n_args &&
      PICMAN_IS_PARAM_SPEC_DISPLAY_ID (pspecs[n_args]))
    {
      if (display)
        {
          picman_value_set_display (picman_value_array_index (args, n_args),
                                  PICMAN_OBJECT (display));
          n_args++;
        }
      else
        {
          g_warning ("Uh-oh, no active display for the plug-in!");
          return -1;
        }
    }

  if (picman_value_array_length (args) > n_args &&
      PICMAN_IS_PARAM_SPEC_IMAGE_ID (pspecs[n_args]))
    {
      PicmanImage *image = display ? picman_display_get_image (display) : NULL;

      if (image)
        {
          picman_value_set_image (picman_value_array_index (args, n_args),
                                image);
          n_args++;

          if (picman_value_array_length (args) > n_args &&
              PICMAN_IS_PARAM_SPEC_DRAWABLE_ID (pspecs[n_args]))
            {
              PicmanDrawable *drawable = picman_image_get_active_drawable (image);

              if (drawable)
                {
                  picman_value_set_drawable (picman_value_array_index (args, n_args),
                                           drawable);
                  n_args++;
                }
              else
                {
                  g_warning ("Uh-oh, no active drawable for the plug-in!");
                  return -1;
                }
            }
        }
    }

  return n_args;
}

static void
plug_in_reset_all_response (GtkWidget *dialog,
                            gint       response_id,
                            Picman      *picman)
{
  gtk_widget_destroy (dialog);

  if (response_id == GTK_RESPONSE_OK)
    picman_plug_in_manager_data_free (picman->plug_in_manager);
}
