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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "operations/picmandesaturateconfig.h"

#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "picmanimagemapoptions.h"
#include "picmandesaturatetool.h"

#include "picman-intl.h"


static gboolean   picman_desaturate_tool_initialize    (PicmanTool           *tool,
                                                      PicmanDisplay        *display,
                                                      GError            **error);

static GeglNode * picman_desaturate_tool_get_operation (PicmanImageMapTool   *im_tool,
                                                      GObject           **config,
                                                      gchar             **undo_desc);
static void       picman_desaturate_tool_dialog        (PicmanImageMapTool   *im_tool);

static void       picman_desaturate_tool_config_notify (GObject            *object,
                                                      GParamSpec         *pspec,
                                                      PicmanDesaturateTool *desaturate_tool);
static void       picman_desaturate_tool_mode_changed  (GtkWidget          *button,
                                                      PicmanDesaturateTool *desaturate_tool);


G_DEFINE_TYPE (PicmanDesaturateTool, picman_desaturate_tool,
               PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_desaturate_tool_parent_class


void
picman_desaturate_tool_register (PicmanToolRegisterCallback  callback,
                               gpointer                  data)
{
  (* callback) (PICMAN_TYPE_DESATURATE_TOOL,
                PICMAN_TYPE_IMAGE_MAP_OPTIONS, NULL,
                0,
                "picman-desaturate-tool",
                _("Desaturate"),
                _("Desaturate Tool: Turn colors into shades of gray"),
                N_("_Desaturate..."), NULL,
                NULL, PICMAN_HELP_TOOL_DESATURATE,
                PICMAN_STOCK_TOOL_DESATURATE,
                data);
}

static void
picman_desaturate_tool_class_init (PicmanDesaturateToolClass *klass)
{
  PicmanToolClass         *tool_class    = PICMAN_TOOL_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  tool_class->initialize       = picman_desaturate_tool_initialize;

  im_tool_class->dialog_desc   = _("Desaturate (Remove Colors)");

  im_tool_class->get_operation = picman_desaturate_tool_get_operation;
  im_tool_class->dialog        = picman_desaturate_tool_dialog;
}

static void
picman_desaturate_tool_init (PicmanDesaturateTool *desaturate_tool)
{
}

static gboolean
picman_desaturate_tool_initialize (PicmanTool     *tool,
                                PicmanDisplay  *display,
                                GError      **error)
{
  PicmanDesaturateTool *desaturate_tool = PICMAN_DESATURATE_TOOL (tool);
  PicmanImage          *image           = picman_display_get_image (display);
  PicmanDrawable       *drawable        = picman_image_get_active_drawable (image);

  if (! drawable)
    return FALSE;

  if (! picman_drawable_is_rgb (drawable))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Desaturate only operates on RGB layers."));
      return FALSE;
    }

  if (! PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error))
    {
      return FALSE;
    }

  picman_int_radio_group_set_active (GTK_RADIO_BUTTON (desaturate_tool->button),
                                   desaturate_tool->config->mode);

  return TRUE;
}

static GeglNode *
picman_desaturate_tool_get_operation (PicmanImageMapTool  *image_map_tool,
                                    GObject          **config,
                                    gchar            **undo_desc)
{
  PicmanDesaturateTool *desaturate_tool = PICMAN_DESATURATE_TOOL (image_map_tool);

  desaturate_tool->config = g_object_new (PICMAN_TYPE_DESATURATE_CONFIG, NULL);

  g_signal_connect_object (desaturate_tool->config, "notify",
                           G_CALLBACK (picman_desaturate_tool_config_notify),
                           G_OBJECT (desaturate_tool), 0);

  *config = G_OBJECT (desaturate_tool->config);

  return gegl_node_new_child (NULL,
                              "operation", "picman:desaturate",
                              "config",    desaturate_tool->config,
                              NULL);
}


/***********************/
/*  Desaturate dialog  */
/***********************/

static void
picman_desaturate_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanDesaturateTool *desaturate_tool = PICMAN_DESATURATE_TOOL (image_map_tool);
  GtkWidget          *main_vbox;
  GtkWidget          *frame;

  main_vbox = picman_image_map_tool_dialog_get_vbox (image_map_tool);

  /*  The table containing sliders  */
  frame = picman_enum_radio_frame_new (PICMAN_TYPE_DESATURATE_MODE,
                                     gtk_label_new (_("Choose shade of gray based on:")),
                                     G_CALLBACK (picman_desaturate_tool_mode_changed),
                                     desaturate_tool,
                                     &desaturate_tool->button);

  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);
}

static void
picman_desaturate_tool_config_notify (GObject            *object,
                                    GParamSpec         *pspec,
                                    PicmanDesaturateTool *desaturate_tool)
{
  PicmanDesaturateConfig *config = PICMAN_DESATURATE_CONFIG (object);

  if (! desaturate_tool->button)
    return;

  picman_int_radio_group_set_active (GTK_RADIO_BUTTON (desaturate_tool->button),
                                   config->mode);
}

static void
picman_desaturate_tool_mode_changed (GtkWidget          *button,
                                   PicmanDesaturateTool *desaturate_tool)
{
  PicmanDesaturateConfig *config = desaturate_tool->config;
  PicmanDesaturateMode    mode;

  mode = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button),
                                             "picman-item-data"));

  if (config->mode != mode)
    {
      g_object_set (config,
                    "mode", mode,
                    NULL);
    }
}
