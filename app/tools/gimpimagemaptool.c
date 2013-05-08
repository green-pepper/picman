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

/* This file contains a base class for tools that implement on canvas
 * preview for non destructive editing. The processing of the pixels can
 * be done either by a gegl op or by a C function (apply_func).
 *
 * For the core side of this, please see /app/core/picmanimagemap.c.
 */

#include "config.h"

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanimage-pick-color.h"
#include "core/picmanimagemap.h"
#include "core/picmanimagemapconfig.h"
#include "core/picmanlist.h"
#include "core/picmanpickable.h"
#include "core/picmanprogress.h"
#include "core/picmanprojection.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmanoverlaybox.h"
#include "widgets/picmanoverlaydialog.h"
#include "widgets/picmansettingsbox.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmantooldialog.h"

#include "picmancoloroptions.h"
#include "picmanimagemaptool.h"
#include "picmanimagemaptool-settings.h"
#include "picmantoolcontrol.h"
#include "tool_manager.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void      picman_image_map_tool_class_init     (PicmanImageMapToolClass *klass);
static void      picman_image_map_tool_base_init      (PicmanImageMapToolClass *klass);

static void      picman_image_map_tool_init           (PicmanImageMapTool *im_tool);

static void      picman_image_map_tool_constructed    (GObject          *object);
static void      picman_image_map_tool_finalize       (GObject          *object);

static gboolean  picman_image_map_tool_initialize     (PicmanTool         *tool,
                                                     PicmanDisplay      *display,
                                                     GError          **error);
static void      picman_image_map_tool_control        (PicmanTool         *tool,
                                                     PicmanToolAction    action,
                                                     PicmanDisplay      *display);
static gboolean  picman_image_map_tool_key_press      (PicmanTool         *tool,
                                                     GdkEventKey      *kevent,
                                                     PicmanDisplay      *display);
static void      picman_image_map_tool_options_notify (PicmanTool         *tool,
                                                     PicmanToolOptions  *options,
                                                     const GParamSpec *pspec);

static gboolean  picman_image_map_tool_pick_color     (PicmanColorTool    *color_tool,
                                                     gint              x,
                                                     gint              y,
                                                     const Babl      **sample_format,
                                                     PicmanRGB          *color,
                                                     gint             *color_index);
static void      picman_image_map_tool_color_picked   (PicmanColorTool    *color_tool,
                                                     PicmanColorPickState pick_state,
                                                     const Babl       *sample_format,
                                                     const PicmanRGB    *color,
                                                     gint              color_index);

static void      picman_image_map_tool_map            (PicmanImageMapTool *im_tool);
static void      picman_image_map_tool_dialog         (PicmanImageMapTool *im_tool);
static void      picman_image_map_tool_dialog_unmap   (GtkWidget        *dialog,
                                                     PicmanImageMapTool *im_tool);
static void      picman_image_map_tool_reset          (PicmanImageMapTool *im_tool);
static void      picman_image_map_tool_create_map     (PicmanImageMapTool *im_tool);

static void      picman_image_map_tool_flush          (PicmanImageMap     *image_map,
                                                     PicmanImageMapTool *im_tool);
static void      picman_image_map_tool_config_notify  (GObject          *object,
                                                     const GParamSpec *pspec,
                                                     PicmanImageMapTool *im_tool);

static void      picman_image_map_tool_response       (GtkWidget        *widget,
                                                     gint              response_id,
                                                     PicmanImageMapTool *im_tool);

static void      picman_image_map_tool_dialog_hide    (PicmanImageMapTool *im_tool);
static void      picman_image_map_tool_dialog_destroy (PicmanImageMapTool *im_tool);


static PicmanColorToolClass *parent_class = NULL;


GType
picman_image_map_tool_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (PicmanImageMapToolClass),
        (GBaseInitFunc) picman_image_map_tool_base_init,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) picman_image_map_tool_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (PicmanImageMapTool),
        0,              /* n_preallocs */
        (GInstanceInitFunc) picman_image_map_tool_init,
      };

      type = g_type_register_static (PICMAN_TYPE_COLOR_TOOL,
                                     "PicmanImageMapTool",
                                     &info, 0);
    }

  return type;
}


static void
picman_image_map_tool_class_init (PicmanImageMapToolClass *klass)
{
  GObjectClass       *object_class     = G_OBJECT_CLASS (klass);
  PicmanToolClass      *tool_class       = PICMAN_TOOL_CLASS (klass);
  PicmanColorToolClass *color_tool_class = PICMAN_COLOR_TOOL_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->constructed  = picman_image_map_tool_constructed;
  object_class->finalize     = picman_image_map_tool_finalize;

  tool_class->initialize     = picman_image_map_tool_initialize;
  tool_class->control        = picman_image_map_tool_control;
  tool_class->key_press      = picman_image_map_tool_key_press;
  tool_class->options_notify = picman_image_map_tool_options_notify;

  color_tool_class->pick     = picman_image_map_tool_pick_color;
  color_tool_class->picked   = picman_image_map_tool_color_picked;

  klass->dialog_desc         = NULL;
  klass->settings_name       = NULL;
  klass->import_dialog_title = NULL;
  klass->export_dialog_title = NULL;

  klass->get_operation       = NULL;
  klass->map                 = NULL;
  klass->dialog              = NULL;
  klass->reset               = NULL;
  klass->get_settings_ui     = picman_image_map_tool_real_get_settings_ui;
  klass->settings_import     = picman_image_map_tool_real_settings_import;
  klass->settings_export     = picman_image_map_tool_real_settings_export;
}

static void
picman_image_map_tool_base_init (PicmanImageMapToolClass *klass)
{
  klass->recent_settings = picman_list_new (PICMAN_TYPE_IMAGE_MAP_CONFIG, TRUE);
  picman_list_set_sort_func (PICMAN_LIST (klass->recent_settings),
                           (GCompareFunc) picman_image_map_config_compare);
}

static void
picman_image_map_tool_init (PicmanImageMapTool *image_map_tool)
{
  PicmanTool *tool = PICMAN_TOOL (image_map_tool);

  picman_tool_control_set_scroll_lock (tool->control, TRUE);
  picman_tool_control_set_preserve    (tool->control, FALSE);
  picman_tool_control_set_dirty_mask  (tool->control,
                                     PICMAN_DIRTY_IMAGE           |
                                     PICMAN_DIRTY_IMAGE_STRUCTURE |
                                     PICMAN_DIRTY_DRAWABLE        |
                                     PICMAN_DIRTY_SELECTION       |
                                     PICMAN_DIRTY_ACTIVE_DRAWABLE);
}

static void
picman_image_map_tool_constructed (GObject *object)
{
  PicmanImageMapTool *image_map_tool = PICMAN_IMAGE_MAP_TOOL (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_image_map_tool_get_operation (image_map_tool);
}

static void
picman_image_map_tool_finalize (GObject *object)
{
  PicmanImageMapTool *image_map_tool = PICMAN_IMAGE_MAP_TOOL (object);

  if (image_map_tool->operation)
    {
      g_object_unref (image_map_tool->operation);
      image_map_tool->operation = NULL;
    }

  if (image_map_tool->config)
    {
      g_object_unref (image_map_tool->config);
      image_map_tool->config = NULL;
    }

  if (image_map_tool->default_config)
    {
      g_object_unref (image_map_tool->default_config);
      image_map_tool->default_config = NULL;
    }

  if (image_map_tool->undo_desc)
    {
      g_free (image_map_tool->undo_desc);
      image_map_tool->undo_desc = NULL;
    }

  if (image_map_tool->dialog)
    picman_image_map_tool_dialog_destroy (image_map_tool);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

#define RESPONSE_RESET 1

static gboolean
picman_image_map_tool_initialize (PicmanTool     *tool,
                                PicmanDisplay  *display,
                                GError      **error)
{
  PicmanImageMapTool *image_map_tool = PICMAN_IMAGE_MAP_TOOL (tool);
  PicmanToolInfo     *tool_info      = tool->tool_info;
  PicmanImage        *image          = picman_display_get_image (display);
  PicmanDrawable     *drawable       = picman_image_get_active_drawable (image);
  PicmanDisplayShell *display_shell  = picman_display_get_shell (display);

  if (! drawable)
    return FALSE;

  if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Cannot modify the pixels of layer groups."));
      return FALSE;
    }

  if (picman_item_is_content_locked (PICMAN_ITEM (drawable)))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("The active layer's pixels are locked."));
      return FALSE;
    }

  if (image_map_tool->active_picker)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (image_map_tool->active_picker),
                                  FALSE);


  /*  set display so the dialog can be hidden on display destruction  */
  tool->display = display;

  if (image_map_tool->config)
    picman_config_reset (PICMAN_CONFIG (image_map_tool->config));

  if (! image_map_tool->dialog)
    {
      PicmanImageMapToolClass *klass;
      GtkWidget             *dialog;
      GtkWidget             *vbox;
      GtkWidget             *toggle;

      klass = PICMAN_IMAGE_MAP_TOOL_GET_CLASS (image_map_tool);

      /*  disabled for at least PICMAN 2.8  */
      image_map_tool->overlay = FALSE;

      if (image_map_tool->overlay)
        {
          image_map_tool->dialog = dialog =
            picman_overlay_dialog_new (tool_info,
                                     klass->dialog_desc,

                                     PICMAN_STOCK_RESET, RESPONSE_RESET,
                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                     GTK_STOCK_OK,     GTK_RESPONSE_OK,

                                     NULL);

          gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

          picman_overlay_box_add_child (PICMAN_OVERLAY_BOX (display_shell->canvas),
                                      dialog, 1.0, 1.0);
          picman_overlay_box_set_child_angle (PICMAN_OVERLAY_BOX (display_shell->canvas),
                                            dialog, 0.0);

          image_map_tool->main_vbox = vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
          gtk_container_add (GTK_CONTAINER (dialog), vbox);
        }
      else
        {
          image_map_tool->dialog = dialog =
            picman_tool_dialog_new (tool_info,
                                  display_shell,
                                  klass->dialog_desc,

                                  PICMAN_STOCK_RESET, RESPONSE_RESET,
                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                  GTK_STOCK_OK,     GTK_RESPONSE_OK,

                                  NULL);

          gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                                   RESPONSE_RESET,
                                                   GTK_RESPONSE_OK,
                                                   GTK_RESPONSE_CANCEL,
                                                   -1);

          image_map_tool->main_vbox = vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
          gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
          gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                              vbox, TRUE, TRUE, 0);
        }

      g_signal_connect_object (dialog, "response",
                               G_CALLBACK (picman_image_map_tool_response),
                               G_OBJECT (image_map_tool), 0);

      if (image_map_tool->config && klass->settings_name)
        {
          GtkWidget *settings_ui;
          gchar     *settings_filename;
          gchar     *default_folder;

          settings_filename =
            picman_tool_info_build_options_filename (tool_info, ".settings");

          default_folder =
            g_build_filename (picman_directory (), klass->settings_name, NULL);

          settings_ui = klass->get_settings_ui (image_map_tool,
                                                klass->recent_settings,
                                                settings_filename,
                                                klass->import_dialog_title,
                                                klass->export_dialog_title,
                                                tool_info->help_id,
                                                default_folder,
                                                &image_map_tool->settings_box);

          g_free (settings_filename);
          g_free (default_folder);

          gtk_box_pack_start (GTK_BOX (image_map_tool->main_vbox), settings_ui,
                              FALSE, FALSE, 0);
          gtk_widget_show (settings_ui);
        }

      /*  The preview toggle  */
      toggle = picman_prop_check_button_new (G_OBJECT (tool_info->tool_options),
                                           "preview",
                                           _("_Preview"));

      gtk_box_pack_end (GTK_BOX (image_map_tool->main_vbox), toggle,
                        FALSE, FALSE, 0);
      gtk_widget_show (toggle);

      /*  Fill in subclass widgets  */
      picman_image_map_tool_dialog (image_map_tool);

      gtk_widget_show (vbox);
    }

  if (PICMAN_IS_VIEWABLE_DIALOG (image_map_tool->dialog))
    {
      picman_viewable_dialog_set_viewable (PICMAN_VIEWABLE_DIALOG (image_map_tool->dialog),
                                         PICMAN_VIEWABLE (drawable),
                                         PICMAN_CONTEXT (tool_info->tool_options));
      picman_tool_dialog_set_shell (PICMAN_TOOL_DIALOG (image_map_tool->dialog),
                                  display_shell);
    }
  else if (PICMAN_IS_OVERLAY_DIALOG (image_map_tool->dialog))
    {
      if (! gtk_widget_get_parent (image_map_tool->dialog))
        {
          picman_overlay_box_add_child (PICMAN_OVERLAY_BOX (display_shell->canvas),
                                      image_map_tool->dialog, 1.0, 1.0);
          g_object_unref (image_map_tool->dialog);
        }
    }

  gtk_widget_show (image_map_tool->dialog);

  image_map_tool->drawable = drawable;

  picman_image_map_tool_create_map (image_map_tool);
  picman_image_map_tool_preview (image_map_tool);

  return TRUE;
}

static void
picman_image_map_tool_control (PicmanTool       *tool,
                             PicmanToolAction  action,
                             PicmanDisplay    *display)
{
  PicmanImageMapTool *image_map_tool = PICMAN_IMAGE_MAP_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      picman_image_map_tool_dialog_hide (image_map_tool);

      if (image_map_tool->image_map)
        {
          picman_tool_control_push_preserve (tool->control, TRUE);

          picman_image_map_abort (image_map_tool->image_map);
          g_object_unref (image_map_tool->image_map);
          image_map_tool->image_map = NULL;

          picman_tool_control_pop_preserve (tool->control);
        }

      tool->drawable = NULL;
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static gboolean
picman_image_map_tool_key_press (PicmanTool    *tool,
                               GdkEventKey *kevent,
                               PicmanDisplay *display)
{
  PicmanImageMapTool *image_map_tool = PICMAN_IMAGE_MAP_TOOL (tool);

  if (image_map_tool->dialog && display == tool->display)
    {
      switch (kevent->keyval)
        {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
        case GDK_KEY_ISO_Enter:
          picman_image_map_tool_response (image_map_tool->dialog,
                                        GTK_RESPONSE_OK,
                                        image_map_tool);
          return TRUE;

        case GDK_KEY_BackSpace:
          picman_image_map_tool_response (image_map_tool->dialog,
                                        RESPONSE_RESET,
                                        image_map_tool);
          return TRUE;

        case GDK_KEY_Escape:
          picman_image_map_tool_response (image_map_tool->dialog,
                                        GTK_RESPONSE_CANCEL,
                                        image_map_tool);
          return TRUE;
        }
    }

  return FALSE;
}

static void
picman_image_map_tool_options_notify (PicmanTool         *tool,
                                    PicmanToolOptions  *options,
                                    const GParamSpec *pspec)
{
  PicmanImageMapTool *image_map_tool = PICMAN_IMAGE_MAP_TOOL (tool);

  if (! strcmp (pspec->name, "preview") &&
      image_map_tool->image_map)
    {
      PicmanImageMapOptions *im_options = PICMAN_IMAGE_MAP_OPTIONS (options);

      if (im_options->preview)
        {
          picman_tool_control_push_preserve (tool->control, TRUE);

          picman_image_map_tool_map (image_map_tool);

          picman_tool_control_pop_preserve (tool->control);
        }
      else
        {
          picman_tool_control_push_preserve (tool->control, TRUE);

          picman_image_map_abort (image_map_tool->image_map);

          picman_tool_control_pop_preserve (tool->control);
        }
    }
}

static gboolean
picman_image_map_tool_pick_color (PicmanColorTool  *color_tool,
                                gint            x,
                                gint            y,
                                const Babl    **sample_format,
                                PicmanRGB        *color,
                                gint           *color_index)
{
  PicmanImageMapTool *tool = PICMAN_IMAGE_MAP_TOOL (color_tool);
  gint              off_x, off_y;

  picman_item_get_offset (PICMAN_ITEM (tool->drawable), &off_x, &off_y);

  *sample_format = picman_drawable_get_format (tool->drawable);

  return picman_pickable_pick_color (PICMAN_PICKABLE (tool->image_map),
                                   x - off_x,
                                   y - off_y,
                                   color_tool->options->sample_average,
                                   color_tool->options->average_radius,
                                   color, color_index);
}

static void
picman_image_map_tool_color_picked (PicmanColorTool      *color_tool,
                                  PicmanColorPickState  pick_state,
                                  const Babl         *sample_format,
                                  const PicmanRGB      *color,
                                  gint                color_index)
{
  PicmanImageMapTool *tool = PICMAN_IMAGE_MAP_TOOL (color_tool);
  gpointer          identifier;

  identifier = g_object_get_data (G_OBJECT (tool->active_picker),
                                  "picker-identifier");

  PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool)->color_picked (tool,
                                                      identifier,
                                                      sample_format,
                                                      color);
}

static void
picman_image_map_tool_map (PicmanImageMapTool *tool)
{
  if (PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool)->map)
    PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool)->map (tool);

  picman_image_map_apply (tool->image_map);
}

static void
picman_image_map_tool_dialog (PicmanImageMapTool *tool)
{
  PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool)->dialog (tool);

  g_signal_connect (tool->dialog, "unmap",
                    G_CALLBACK (picman_image_map_tool_dialog_unmap),
                    tool);
}

static void
picman_image_map_tool_dialog_unmap (GtkWidget        *dialog,
                                  PicmanImageMapTool *tool)
{
  if (tool->active_picker)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tool->active_picker),
                                  FALSE);
}

static void
picman_image_map_tool_reset (PicmanImageMapTool *tool)
{
  if (PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool)->reset)
    {
      PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool)->reset (tool);
    }
  else if (tool->config)
    {
      if (tool->default_config)
        {
          picman_config_copy (PICMAN_CONFIG (tool->default_config),
                            PICMAN_CONFIG (tool->config),
                            0);
        }
      else
        {
          picman_config_reset (PICMAN_CONFIG (tool->config));
        }
    }
}

static void
picman_image_map_tool_create_map (PicmanImageMapTool *tool)
{
  PicmanToolInfo *tool_info;

  g_return_if_fail (PICMAN_IS_IMAGE_MAP_TOOL (tool));

  if (tool->image_map)
    {
      picman_image_map_abort (tool->image_map);
      g_object_unref (tool->image_map);
    }

  g_assert (tool->operation);

  tool_info = PICMAN_TOOL (tool)->tool_info;

  tool->image_map = picman_image_map_new (tool->drawable,
                                        tool->undo_desc,
                                        tool->operation,
                                        picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool_info)));

  g_signal_connect (tool->image_map, "flush",
                    G_CALLBACK (picman_image_map_tool_flush),
                    tool);
}

static void
picman_image_map_tool_flush (PicmanImageMap     *image_map,
                           PicmanImageMapTool *image_map_tool)
{
  PicmanTool  *tool  = PICMAN_TOOL (image_map_tool);
  PicmanImage *image = picman_display_get_image (tool->display);

  picman_projection_flush (picman_image_get_projection (image));
}

static void
picman_image_map_tool_config_notify (GObject          *object,
                                   const GParamSpec *pspec,
                                   PicmanImageMapTool *image_map_tool)
{
  picman_image_map_tool_preview (image_map_tool);
}

static void
picman_image_map_tool_response (GtkWidget        *widget,
                              gint              response_id,
                              PicmanImageMapTool *image_map_tool)
{
  PicmanTool *tool = PICMAN_TOOL (image_map_tool);

  switch (response_id)
    {
    case RESPONSE_RESET:
      picman_image_map_tool_reset (image_map_tool);
      picman_image_map_tool_preview (image_map_tool);
      break;

    case GTK_RESPONSE_OK:
      picman_image_map_tool_dialog_hide (image_map_tool);

      if (image_map_tool->image_map)
        {
          PicmanImageMapOptions *options = PICMAN_IMAGE_MAP_TOOL_GET_OPTIONS (tool);

          picman_tool_control_push_preserve (tool->control, TRUE);

          if (! options->preview)
            picman_image_map_tool_map (image_map_tool);

          picman_image_map_commit (image_map_tool->image_map,
                                 PICMAN_PROGRESS (tool));
          g_object_unref (image_map_tool->image_map);
          image_map_tool->image_map = NULL;

          picman_tool_control_pop_preserve (tool->control);

          picman_image_flush (picman_display_get_image (tool->display));

          if (image_map_tool->config && image_map_tool->settings_box)
            picman_settings_box_add_current (PICMAN_SETTINGS_BOX (image_map_tool->settings_box),
                                           PICMAN_GUI_CONFIG (tool->tool_info->picman->config)->image_map_tool_max_recent);
        }

      tool->display  = NULL;
      tool->drawable = NULL;
      break;

    default:
      picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, tool->display);
      break;
    }
}

static void
picman_image_map_tool_dialog_hide (PicmanImageMapTool *image_map_tool)
{
  GtkWidget *dialog = image_map_tool->dialog;

  if (GTK_IS_DIALOG (dialog))
    {
      picman_dialog_factory_hide_dialog (dialog);
    }
  else if (PICMAN_IS_OVERLAY_DIALOG (dialog))
    {
      if (gtk_widget_get_parent (dialog))
        {
          g_object_ref (dialog);
          gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (dialog)),
                                dialog);
          gtk_widget_hide (dialog);
        }
    }
}

static void
picman_image_map_tool_dialog_destroy (PicmanImageMapTool *image_map_tool)
{
  if (image_map_tool->label_group)
    {
      g_object_unref (image_map_tool->label_group);
      image_map_tool->label_group = NULL;
    }

  if (GTK_IS_DIALOG (image_map_tool->dialog) ||
      gtk_widget_get_parent (image_map_tool->dialog))
    gtk_widget_destroy (image_map_tool->dialog);
  else
    g_object_unref (image_map_tool->dialog);

  image_map_tool->dialog       = NULL;
  image_map_tool->main_vbox    = NULL;
  image_map_tool->settings_box = NULL;
}

void
picman_image_map_tool_get_operation (PicmanImageMapTool *image_map_tool)
{
  PicmanImageMapToolClass *klass;

  g_return_if_fail (PICMAN_IS_IMAGE_MAP_TOOL (image_map_tool));

  klass = PICMAN_IMAGE_MAP_TOOL_GET_CLASS (image_map_tool);

  if (image_map_tool->image_map)
    {
      picman_image_map_abort (image_map_tool->image_map);
      g_object_unref (image_map_tool->image_map);
      image_map_tool->image_map = NULL;
    }

  if (image_map_tool->operation)
    {
      g_object_unref (image_map_tool->operation);
      image_map_tool->operation = NULL;
    }

  if (image_map_tool->config)
    {
      g_signal_handlers_disconnect_by_func (image_map_tool->config,
                                            picman_image_map_tool_config_notify,
                                            image_map_tool);

      g_object_unref (image_map_tool->config);
      image_map_tool->config = NULL;
    }

  if (image_map_tool->undo_desc)
    {
      g_free (image_map_tool->undo_desc);
      image_map_tool->undo_desc = NULL;
    }

  image_map_tool->operation = klass->get_operation (image_map_tool,
                                                    &image_map_tool->config,
                                                    &image_map_tool->undo_desc);

  if (! image_map_tool->undo_desc)
    image_map_tool->undo_desc =
      g_strdup (PICMAN_TOOL (image_map_tool)->tool_info->blurb);

  if (image_map_tool->config)
    g_signal_connect_object (image_map_tool->config, "notify",
                             G_CALLBACK (picman_image_map_tool_config_notify),
                             G_OBJECT (image_map_tool), 0);

  if (PICMAN_TOOL (image_map_tool)->drawable)
    picman_image_map_tool_create_map (image_map_tool);
}

void
picman_image_map_tool_preview (PicmanImageMapTool *image_map_tool)
{
  PicmanTool            *tool;
  PicmanImageMapOptions *options;

  g_return_if_fail (PICMAN_IS_IMAGE_MAP_TOOL (image_map_tool));

  tool    = PICMAN_TOOL (image_map_tool);
  options = PICMAN_IMAGE_MAP_TOOL_GET_OPTIONS (tool);

  if (image_map_tool->image_map && options->preview)
    {
      picman_tool_control_push_preserve (tool->control, TRUE);

      picman_image_map_tool_map (image_map_tool);

      picman_tool_control_pop_preserve (tool->control);
    }
}

void
picman_image_map_tool_edit_as (PicmanImageMapTool *im_tool,
                             const gchar      *new_tool_id,
                             PicmanConfig       *config)
{
  PicmanDisplay  *display;
  PicmanContext  *user_context;
  PicmanToolInfo *tool_info;
  PicmanTool     *new_tool;

  g_return_if_fail (PICMAN_IS_IMAGE_MAP_TOOL (im_tool));
  g_return_if_fail (new_tool_id);
  g_return_if_fail (PICMAN_IS_CONFIG (config));

  display = PICMAN_TOOL (im_tool)->display;

  user_context = picman_get_user_context (display->picman);

  tool_info = (PicmanToolInfo *)
    picman_container_get_child_by_name (display->picman->tool_info_list,
                                      new_tool_id);

  picman_context_set_tool (user_context, tool_info);
  tool_manager_initialize_active (display->picman, display);

  new_tool = tool_manager_get_active (display->picman);

  PICMAN_IMAGE_MAP_TOOL (new_tool)->default_config = g_object_ref (config);

  picman_image_map_tool_reset (PICMAN_IMAGE_MAP_TOOL (new_tool));
}

GtkWidget *
picman_image_map_tool_dialog_get_vbox (PicmanImageMapTool *tool)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE_MAP_TOOL (tool), NULL);

  return tool->main_vbox;
}

GtkSizeGroup *
picman_image_map_tool_dialog_get_label_group (PicmanImageMapTool *tool)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE_MAP_TOOL (tool), NULL);

  if (! tool->label_group)
    tool->label_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  return tool->label_group;
}

static void
picman_image_map_tool_color_picker_toggled (GtkWidget        *widget,
                                          PicmanImageMapTool *tool)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
      if (tool->active_picker == widget)
        return;

      if (tool->active_picker)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tool->active_picker),
                                      FALSE);

      tool->active_picker = widget;

      picman_color_tool_enable (PICMAN_COLOR_TOOL (tool),
                              PICMAN_COLOR_TOOL_GET_OPTIONS (tool));
    }
  else if (tool->active_picker == widget)
    {
      tool->active_picker = NULL;
      picman_color_tool_disable (PICMAN_COLOR_TOOL (tool));
    }
}

GtkWidget *
picman_image_map_tool_add_color_picker (PicmanImageMapTool *tool,
                                      gpointer          identifier,
                                      const gchar      *stock_id,
                                      const gchar      *help_id)
{
  GtkWidget *button;
  GtkWidget *image;

  g_return_val_if_fail (PICMAN_IS_IMAGE_MAP_TOOL (tool), NULL);
  g_return_val_if_fail (stock_id != NULL, NULL);

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "draw-indicator", FALSE,
                         NULL);

  image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
  gtk_misc_set_padding (GTK_MISC (image), 2, 2);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_widget_show (image);

  if (help_id)
    picman_help_set_help_data (button, help_id, NULL);

  g_object_set_data (G_OBJECT (button), "picker-identifier", identifier);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_image_map_tool_color_picker_toggled),
                    tool);

  return button;
}
