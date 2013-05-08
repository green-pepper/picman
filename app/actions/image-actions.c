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

#include "actions-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanitemstack.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "image-actions.h"
#include "image-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry image_actions[] =
{
  { "image-menubar", NULL,
    NC_("image-action", "Image Menu"), NULL, NULL, NULL,
    PICMAN_HELP_IMAGE_WINDOW },

  { "image-popup", NULL,
    NC_("image-action", "Image Menu"), NULL, NULL, NULL,
    PICMAN_HELP_IMAGE_WINDOW },

  { "image-menu",             NULL, NC_("image-action", "_Image")      },
  { "image-mode-menu",        NULL, NC_("image-action", "_Mode")       },
  { "image-precision-menu",   NULL, NC_("image-action", "_Precision")  },
  { "image-transform-menu",   NULL, NC_("image-action", "_Transform")  },
  { "image-guides-menu",      NULL, NC_("image-action", "_Guides")     },

  { "colors-menu",            NULL, NC_("image-action", "_Colors")     },
  { "colors-info-menu",       NULL, NC_("image-action", "I_nfo")       },
  { "colors-auto-menu",       NULL, NC_("image-action", "_Auto")       },
  { "colors-map-menu",        NULL, NC_("image-action", "_Map")        },
  { "colors-components-menu", NULL, NC_("image-action", "C_omponents") },
  { "colors-desaturate-menu", NULL, NC_("image-action", "D_esaturate") },

  { "image-new", GTK_STOCK_NEW,
    NC_("image-action", "_New..."), "<primary>N",
    NC_("image-action", "Create a new image"),
    G_CALLBACK (image_new_cmd_callback),
    PICMAN_HELP_FILE_NEW },

  { "image-resize", PICMAN_STOCK_RESIZE,
    NC_("image-action", "Can_vas Size..."), NULL,
    NC_("image-action", "Adjust the image dimensions"),
    G_CALLBACK (image_resize_cmd_callback),
    PICMAN_HELP_IMAGE_RESIZE },

  { "image-resize-to-layers", NULL,
    NC_("image-action", "Fit Canvas to L_ayers"), NULL,
    NC_("image-action", "Resize the image to enclose all layers"),
    G_CALLBACK (image_resize_to_layers_cmd_callback),
    PICMAN_HELP_IMAGE_RESIZE_TO_LAYERS },

  { "image-resize-to-selection", NULL,
    NC_("image-action", "F_it Canvas to Selection"), NULL,
    NC_("image-action", "Resize the image to the extents of the selection"),
    G_CALLBACK (image_resize_to_selection_cmd_callback),
    PICMAN_HELP_IMAGE_RESIZE_TO_SELECTION },

  { "image-print-size", PICMAN_STOCK_PRINT_RESOLUTION,
    NC_("image-action", "_Print Size..."), NULL,
    NC_("image-action", "Adjust the print resolution"),
    G_CALLBACK (image_print_size_cmd_callback),
    PICMAN_HELP_IMAGE_PRINT_SIZE },

  { "image-scale", PICMAN_STOCK_SCALE,
    NC_("image-action", "_Scale Image..."), NULL,
    NC_("image-action", "Change the size of the image content"),
    G_CALLBACK (image_scale_cmd_callback),
    PICMAN_HELP_IMAGE_SCALE },

  { "image-crop-to-selection", PICMAN_STOCK_TOOL_CROP,
    NC_("image-action", "_Crop to Selection"), NULL,
    NC_("image-action", "Crop the image to the extents of the selection"),
    G_CALLBACK (image_crop_to_selection_cmd_callback),
    PICMAN_HELP_IMAGE_CROP },

  { "image-crop-to-content", PICMAN_STOCK_TOOL_CROP,
    NC_("image-action", "Crop to C_ontent"), NULL,
    NC_("image-action", "Crop the image to the extents of its content (remove empty borders from the image)"),
    G_CALLBACK (image_crop_to_content_cmd_callback),
    PICMAN_HELP_IMAGE_CROP },

  { "image-duplicate", PICMAN_STOCK_DUPLICATE,
    NC_("image-action", "_Duplicate"), "<primary>D",
    NC_("image-action", "Create a duplicate of this image"),
    G_CALLBACK (image_duplicate_cmd_callback),
    PICMAN_HELP_IMAGE_DUPLICATE },

  { "image-merge-layers", NULL,
    NC_("image-action", "Merge Visible _Layers..."), "<primary>M",
    NC_("image-action", "Merge all visible layers into one layer"),
    G_CALLBACK (image_merge_layers_cmd_callback),
    PICMAN_HELP_IMAGE_MERGE_LAYERS },

  { "image-flatten", NULL,
    NC_("image-action", "_Flatten Image"), NULL,
    NC_("image-action", "Merge all layers into one and remove transparency"),
    G_CALLBACK (image_flatten_image_cmd_callback),
    PICMAN_HELP_IMAGE_FLATTEN },

  { "image-configure-grid", PICMAN_STOCK_GRID,
    NC_("image-action", "Configure G_rid..."), NULL,
    NC_("image-action", "Configure the grid for this image"),
    G_CALLBACK (image_configure_grid_cmd_callback),
    PICMAN_HELP_IMAGE_GRID },

  { "image-properties", GTK_STOCK_INFO,
    NC_("image-action", "Image Pr_operties"), "<alt>Return",
    NC_("image-action", "Display information about this image"),
    G_CALLBACK (image_properties_cmd_callback),
    PICMAN_HELP_IMAGE_PROPERTIES }
};

static const PicmanRadioActionEntry image_convert_base_type_actions[] =
{
  { "image-convert-rgb", PICMAN_STOCK_CONVERT_RGB,
    NC_("image-convert-action", "_RGB"), NULL,
    NC_("image-convert-action", "Convert the image to the RGB colorspace"),
    PICMAN_RGB, PICMAN_HELP_IMAGE_CONVERT_RGB },

  { "image-convert-grayscale", PICMAN_STOCK_CONVERT_GRAYSCALE,
    NC_("image-convert-action", "_Grayscale"), NULL,
    NC_("image-convert-action", "Convert the image to grayscale"),
    PICMAN_GRAY, PICMAN_HELP_IMAGE_CONVERT_GRAYSCALE },

  { "image-convert-indexed", PICMAN_STOCK_CONVERT_INDEXED,
    NC_("image-convert-action", "_Indexed..."), NULL,
    NC_("image-convert-action", "Convert the image to indexed colors"),
    PICMAN_INDEXED, PICMAN_HELP_IMAGE_CONVERT_INDEXED }
};

static const PicmanRadioActionEntry image_convert_precision_actions[] =
{
  { "image-convert-u8", NULL,
    NC_("image-convert-action", "8 bit integer"), NULL,
    NC_("image-convert-action", "Convert the image to 8 bit integer"),
    PICMAN_PRECISION_U8, PICMAN_HELP_IMAGE_CONVERT_U8 },

  { "image-convert-u16", NULL,
    NC_("image-convert-action", "16 bit integer"), NULL,
    NC_("image-convert-action", "Convert the image to 16 bit integer"),
    PICMAN_PRECISION_U16, PICMAN_HELP_IMAGE_CONVERT_U16 },

  { "image-convert-u32", NULL,
    NC_("image-convert-action", "32 bit integer"), NULL,
    NC_("image-convert-action", "Convert the image to 32 bit integer"),
    PICMAN_PRECISION_U32, PICMAN_HELP_IMAGE_CONVERT_U32 },

  { "image-convert-half", NULL,
    NC_("image-convert-action", "16 bit floating point"), NULL,
    NC_("image-convert-action", "Convert the image to 16 bit floating point"),
    PICMAN_PRECISION_HALF, PICMAN_HELP_IMAGE_CONVERT_HALF },

  { "image-convert-float", NULL,
    NC_("image-convert-action", "32 bit floating point"), NULL,
    NC_("image-convert-action", "Convert the image to 32 bit floating point"),
    PICMAN_PRECISION_FLOAT, PICMAN_HELP_IMAGE_CONVERT_FLOAT }
};

static const PicmanEnumActionEntry image_flip_actions[] =
{
  { "image-flip-horizontal", PICMAN_STOCK_FLIP_HORIZONTAL,
    NC_("image-action", "Flip _Horizontally"), NULL,
    NC_("image-action", "Flip image horizontally"),
    PICMAN_ORIENTATION_HORIZONTAL, FALSE,
    PICMAN_HELP_IMAGE_FLIP_HORIZONTAL },

  { "image-flip-vertical", PICMAN_STOCK_FLIP_VERTICAL,
    NC_("image-action", "Flip _Vertically"), NULL,
    NC_("image-action", "Flip image vertically"),
    PICMAN_ORIENTATION_VERTICAL, FALSE,
    PICMAN_HELP_IMAGE_FLIP_VERTICAL }
};

static const PicmanEnumActionEntry image_rotate_actions[] =
{
  { "image-rotate-90", PICMAN_STOCK_ROTATE_90,
    NC_("image-action", "Rotate 90° _clockwise"), NULL,
    NC_("image-action", "Rotate the image 90 degrees to the right"),
    PICMAN_ROTATE_90, FALSE,
    PICMAN_HELP_IMAGE_ROTATE_90 },

  { "image-rotate-180", PICMAN_STOCK_ROTATE_180,
    NC_("image-action", "Rotate _180°"), NULL,
    NC_("image-action", "Turn the image upside-down"),
    PICMAN_ROTATE_180, FALSE,
    PICMAN_HELP_IMAGE_ROTATE_180 },

  { "image-rotate-270", PICMAN_STOCK_ROTATE_270,
    NC_("image-action", "Rotate 90° counter-clock_wise"), NULL,
    NC_("image-action", "Rotate the image 90 degrees to the left"),
    PICMAN_ROTATE_270, FALSE,
    PICMAN_HELP_IMAGE_ROTATE_270 }
};


void
image_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "image-action",
                                 image_actions,
                                 G_N_ELEMENTS (image_actions));

  picman_action_group_add_radio_actions (group, "image-convert-action",
                                       image_convert_base_type_actions,
                                       G_N_ELEMENTS (image_convert_base_type_actions),
                                       NULL, 0,
                                       G_CALLBACK (image_convert_base_type_cmd_callback));

  picman_action_group_add_radio_actions (group, "image-convert-action",
                                       image_convert_precision_actions,
                                       G_N_ELEMENTS (image_convert_precision_actions),
                                       NULL, 0,
                                       G_CALLBACK (image_convert_precision_cmd_callback));

  picman_action_group_add_enum_actions (group, "image-action",
                                      image_flip_actions,
                                      G_N_ELEMENTS (image_flip_actions),
                                      G_CALLBACK (image_flip_cmd_callback));

  picman_action_group_add_enum_actions (group, "image-action",
                                      image_rotate_actions,
                                      G_N_ELEMENTS (image_rotate_actions),
                                      G_CALLBACK (image_rotate_cmd_callback));

#define SET_ALWAYS_SHOW_IMAGE(action,show) \
        picman_action_group_set_action_always_show_image (group, action, show)

  SET_ALWAYS_SHOW_IMAGE ("image-rotate-90",  TRUE);
  SET_ALWAYS_SHOW_IMAGE ("image-rotate-180", TRUE);
  SET_ALWAYS_SHOW_IMAGE ("image-rotate-270", TRUE);

#undef SET_ALWAYS_SHOW_IMAGE
}

void
image_actions_update (PicmanActionGroup *group,
                      gpointer         data)
{
  PicmanImage *image      = action_data_get_image (data);
  gboolean   is_indexed = FALSE;
  gboolean   is_u8      = FALSE;
  gboolean   aux        = FALSE;
  gboolean   lp         = FALSE;
  gboolean   sel        = FALSE;
  gboolean   groups     = FALSE;

  if (image)
    {
      PicmanContainer *layers;
      const gchar   *action = NULL;

      switch (picman_image_get_base_type (image))
        {
        case PICMAN_RGB:     action = "image-convert-rgb";       break;
        case PICMAN_GRAY:    action = "image-convert-grayscale"; break;
        case PICMAN_INDEXED: action = "image-convert-indexed";   break;
        }

      picman_action_group_set_action_active (group, action, TRUE);

      switch (picman_image_get_precision (image))
        {
        case PICMAN_PRECISION_U8:    action = "image-convert-u8";    break;
        case PICMAN_PRECISION_U16:   action = "image-convert-u16";   break;
        case PICMAN_PRECISION_U32:   action = "image-convert-u32";   break;
        case PICMAN_PRECISION_HALF:  action = "image-convert-half";  break;
        case PICMAN_PRECISION_FLOAT: action = "image-convert-float"; break;
        }

      picman_action_group_set_action_active (group, action, TRUE);

      is_indexed = (picman_image_get_base_type (image) == PICMAN_INDEXED);
      is_u8      = (picman_image_get_precision (image) == PICMAN_PRECISION_U8);
      aux        = (picman_image_get_active_channel (image) != NULL);
      lp         = ! picman_image_is_empty (image);
      sel        = ! picman_channel_is_empty (picman_image_get_mask (image));

      layers = picman_image_get_layers (image);

      groups = ! picman_item_stack_is_flat (PICMAN_ITEM_STACK (layers));
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_SENSITIVE ("image-convert-rgb",       image);
  SET_SENSITIVE ("image-convert-grayscale", image);
  SET_SENSITIVE ("image-convert-indexed",   image && !groups && is_u8);

  SET_SENSITIVE ("image-convert-u8",    image);
  SET_SENSITIVE ("image-convert-u16",   image && !is_indexed);
  SET_SENSITIVE ("image-convert-u32",   image && !is_indexed);
  SET_SENSITIVE ("image-convert-half",  image && !is_indexed);
  SET_SENSITIVE ("image-convert-float", image && !is_indexed);

  SET_SENSITIVE ("image-flip-horizontal", image);
  SET_SENSITIVE ("image-flip-vertical",   image);
  SET_SENSITIVE ("image-rotate-90",       image);
  SET_SENSITIVE ("image-rotate-180",      image);
  SET_SENSITIVE ("image-rotate-270",      image);

  SET_SENSITIVE ("image-resize",              image);
  SET_SENSITIVE ("image-resize-to-layers",    image);
  SET_SENSITIVE ("image-resize-to-selection", image && sel);
  SET_SENSITIVE ("image-print-size",          image);
  SET_SENSITIVE ("image-scale",               image);
  SET_SENSITIVE ("image-crop-to-selection",   image && sel);
  SET_SENSITIVE ("image-crop-to-content",     image);
  SET_SENSITIVE ("image-duplicate",           image);
  SET_SENSITIVE ("image-merge-layers",        image && !aux && lp);
  SET_SENSITIVE ("image-flatten",             image && !aux && lp);
  SET_SENSITIVE ("image-configure-grid",      image);
  SET_SENSITIVE ("image-properties",          image);

#undef SET_SENSITIVE
}
