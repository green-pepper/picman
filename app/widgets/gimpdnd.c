/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#include "core/picman.h"
#include "core/picmanbrush.h"
#include "core/picmanbuffer.h"
#include "core/picmanchannel.h"
#include "core/picmancontainer.h"
#include "core/picmandatafactory.h"
#include "core/picmandrawable.h"
#include "core/picmangradient.h"
#include "core/picmanimage.h"
#include "core/picmanimagefile.h"
#include "core/picmanlayer.h"
#include "core/picmanlayermask.h"
#include "core/picmanpalette.h"
#include "core/picmanpattern.h"
#include "core/picmantemplate.h"
#include "core/picmantoolinfo.h"

#include "text/picmanfont.h"

#include "vectors/picmanvectors.h"

#include "picmandnd.h"
#include "picmandnd-xds.h"
#include "picmanpixbuf.h"
#include "picmanselectiondata.h"
#include "picmanview.h"
#include "picmanviewrendererimage.h"

#include "picman-log.h"
#include "picman-intl.h"


#define DRAG_PREVIEW_SIZE  PICMAN_VIEW_SIZE_LARGE
#define DRAG_ICON_OFFSET   -8


typedef GtkWidget * (* PicmanDndGetIconFunc)  (GtkWidget        *widget,
                                             GCallback         get_data_func,
                                             gpointer          get_data_data);
typedef void        (* PicmanDndDragDataFunc) (GtkWidget        *widget,
                                             GdkDragContext   *context,
                                             GCallback         get_data_func,
                                             gpointer          get_data_data,
                                             GtkSelectionData *selection);
typedef gboolean    (* PicmanDndDropDataFunc) (GtkWidget        *widget,
                                             gint              x,
                                             gint              y,
                                             GCallback         set_data_func,
                                             gpointer          set_data_data,
                                             GtkSelectionData *selection);


typedef struct _PicmanDndDataDef PicmanDndDataDef;

struct _PicmanDndDataDef
{
  GtkTargetEntry       target_entry;

  const gchar         *get_data_func_name;
  const gchar         *get_data_data_name;

  const gchar         *set_data_func_name;
  const gchar         *set_data_data_name;

  PicmanDndGetIconFunc   get_icon_func;
  PicmanDndDragDataFunc  get_data_func;
  PicmanDndDropDataFunc  set_data_func;
};


static GtkWidget * picman_dnd_get_viewable_icon  (GtkWidget        *widget,
                                                GCallback         get_viewable_func,
                                                gpointer          get_viewable_data);
static GtkWidget * picman_dnd_get_component_icon (GtkWidget        *widget,
                                                GCallback         get_comp_func,
                                                gpointer          get_comp_data);
static GtkWidget * picman_dnd_get_color_icon     (GtkWidget        *widget,
                                                GCallback         get_color_func,
                                                gpointer          get_color_data);

static void        picman_dnd_get_uri_list_data  (GtkWidget        *widget,
                                                GdkDragContext   *context,
                                                GCallback         get_uri_list_func,
                                                gpointer          get_uri_list_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_uri_list_data  (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_uri_list_func,
                                                gpointer          set_uri_list_data,
                                                GtkSelectionData *selection);

static void        picman_dnd_get_xds_data       (GtkWidget        *widget,
                                                GdkDragContext   *context,
                                                GCallback         get_image_func,
                                                gpointer          get_image_data,
                                                GtkSelectionData *selection);

static void        picman_dnd_get_color_data     (GtkWidget        *widget,
                                                GdkDragContext   *context,
                                                GCallback         get_color_func,
                                                gpointer          get_color_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_color_data     (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_color_func,
                                                gpointer          set_color_data,
                                                GtkSelectionData *selection);

static void        picman_dnd_get_stream_data    (GtkWidget        *widget,
                                                GdkDragContext   *context,
                                                GCallback         get_stream_func,
                                                gpointer          get_stream_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_stream_data    (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_stream_func,
                                                gpointer          set_stream_data,
                                                GtkSelectionData *selection);

static void        picman_dnd_get_pixbuf_data    (GtkWidget        *widget,
                                                GdkDragContext   *context,
                                                GCallback         get_pixbuf_func,
                                                gpointer          get_pixbuf_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_pixbuf_data    (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_pixbuf_func,
                                                gpointer          set_pixbuf_data,
                                                GtkSelectionData *selection);
static void        picman_dnd_get_component_data (GtkWidget        *widget,
                                                GdkDragContext   *context,
                                                GCallback         get_comp_func,
                                                gpointer          get_comp_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_component_data (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_comp_func,
                                                gpointer          set_comp_data,
                                                GtkSelectionData *selection);

static void        picman_dnd_get_image_data     (GtkWidget        *widget,
                                                GdkDragContext   *context,
                                                GCallback         get_image_func,
                                                gpointer          get_image_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_image_data     (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_image_func,
                                                gpointer          set_image_data,
                                                GtkSelectionData *selection);

static void        picman_dnd_get_item_data      (GtkWidget        *widget,
                                                GdkDragContext   *context,
                                                GCallback         get_item_func,
                                                gpointer          get_item_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_item_data      (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_item_func,
                                                gpointer          set_item_data,
                                                GtkSelectionData *selection);

static void        picman_dnd_get_object_data    (GtkWidget        *widget,
                                                GdkDragContext   *context,
                                                GCallback         get_object_func,
                                                gpointer          get_object_data,
                                                GtkSelectionData *selection);

static gboolean    picman_dnd_set_brush_data     (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_brush_func,
                                                gpointer          set_brush_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_pattern_data   (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_pattern_func,
                                                gpointer          set_pattern_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_gradient_data  (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_gradient_func,
                                                gpointer          set_gradient_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_palette_data   (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_palette_func,
                                                gpointer          set_palette_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_font_data      (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_font_func,
                                                gpointer          set_font_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_buffer_data    (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_buffer_func,
                                                gpointer          set_buffer_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_imagefile_data (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_imagefile_func,
                                                gpointer          set_imagefile_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_template_data  (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_template_func,
                                                gpointer          set_template_data,
                                                GtkSelectionData *selection);
static gboolean    picman_dnd_set_tool_info_data (GtkWidget        *widget,
                                                gint              x,
                                                gint              y,
                                                GCallback         set_tool_info_func,
                                                gpointer          set_tool_info_data,
                                                GtkSelectionData *selection);



static const PicmanDndDataDef dnd_data_defs[] =
{
  {
    { NULL, 0, -1 },

    NULL,
    NULL,

    NULL,
    NULL,
    NULL
  },

  {
    PICMAN_TARGET_URI_LIST,

    "picman-dnd-get-uri-list-func",
    "picman-dnd-get-uri-list-data",

    "picman-dnd-set-uri-list-func",
    "picman-dnd-set-uri-list-data",

    NULL,
    picman_dnd_get_uri_list_data,
    picman_dnd_set_uri_list_data
  },

  {
    PICMAN_TARGET_TEXT_PLAIN,

    NULL,
    NULL,

    "picman-dnd-set-uri-list-func",
    "picman-dnd-set-uri-list-data",

    NULL,
    NULL,
    picman_dnd_set_uri_list_data
  },

  {
    PICMAN_TARGET_NETSCAPE_URL,

    NULL,
    NULL,

    "picman-dnd-set-uri-list-func",
    "picman-dnd-set-uri-list-data",

    NULL,
    NULL,
    picman_dnd_set_uri_list_data
  },

  {
    PICMAN_TARGET_XDS,

    "picman-dnd-get-xds-func",
    "picman-dnd-get-xds-data",

    NULL,
    NULL,

    picman_dnd_get_viewable_icon,
    picman_dnd_get_xds_data,
    NULL
  },

  {
    PICMAN_TARGET_COLOR,

    "picman-dnd-get-color-func",
    "picman-dnd-get-color-data",

    "picman-dnd-set-color-func",
    "picman-dnd-set-color-data",

    picman_dnd_get_color_icon,
    picman_dnd_get_color_data,
    picman_dnd_set_color_data
  },

  {
    PICMAN_TARGET_SVG,

    "picman-dnd-get-svg-func",
    "picman-dnd-get-svg-data",

    "picman-dnd-set-svg-func",
    "picman-dnd-set-svg-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_stream_data,
    picman_dnd_set_stream_data
  },

  {
    PICMAN_TARGET_SVG_XML,

    "picman-dnd-get-svg-xml-func",
    "picman-dnd-get-svg-xml-data",

    "picman-dnd-set-svg-xml-func",
    "picman-dnd-set-svg-xml-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_stream_data,
    picman_dnd_set_stream_data
  },

  {
    PICMAN_TARGET_PIXBUF,

    "picman-dnd-get-pixbuf-func",
    "picman-dnd-get-pixbuf-data",

    "picman-dnd-set-pixbuf-func",
    "picman-dnd-set-pixbuf-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_pixbuf_data,
    picman_dnd_set_pixbuf_data
  },

  {
    PICMAN_TARGET_IMAGE,

    "picman-dnd-get-image-func",
    "picman-dnd-get-image-data",

    "picman-dnd-set-image-func",
    "picman-dnd-set-image-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_image_data,
    picman_dnd_set_image_data,
  },

  {
    PICMAN_TARGET_COMPONENT,

    "picman-dnd-get-component-func",
    "picman-dnd-get-component-data",

    "picman-dnd-set-component-func",
    "picman-dnd-set-component-data",

    picman_dnd_get_component_icon,
    picman_dnd_get_component_data,
    picman_dnd_set_component_data,
  },

  {
    PICMAN_TARGET_LAYER,

    "picman-dnd-get-layer-func",
    "picman-dnd-get-layer-data",

    "picman-dnd-set-layer-func",
    "picman-dnd-set-layer-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_item_data,
    picman_dnd_set_item_data,
  },

  {
    PICMAN_TARGET_CHANNEL,

    "picman-dnd-get-channel-func",
    "picman-dnd-get-channel-data",

    "picman-dnd-set-channel-func",
    "picman-dnd-set-channel-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_item_data,
    picman_dnd_set_item_data,
  },

  {
    PICMAN_TARGET_LAYER_MASK,

    "picman-dnd-get-layer-mask-func",
    "picman-dnd-get-layer-mask-data",

    "picman-dnd-set-layer-mask-func",
    "picman-dnd-set-layer-mask-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_item_data,
    picman_dnd_set_item_data,
  },

  {
    PICMAN_TARGET_VECTORS,

    "picman-dnd-get-vectors-func",
    "picman-dnd-get-vectors-data",

    "picman-dnd-set-vectors-func",
    "picman-dnd-set-vectors-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_item_data,
    picman_dnd_set_item_data,
  },

  {
    PICMAN_TARGET_BRUSH,

    "picman-dnd-get-brush-func",
    "picman-dnd-get-brush-data",

    "picman-dnd-set-brush-func",
    "picman-dnd-set-brush-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_object_data,
    picman_dnd_set_brush_data
  },

  {
    PICMAN_TARGET_PATTERN,

    "picman-dnd-get-pattern-func",
    "picman-dnd-get-pattern-data",

    "picman-dnd-set-pattern-func",
    "picman-dnd-set-pattern-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_object_data,
    picman_dnd_set_pattern_data
  },

  {
    PICMAN_TARGET_GRADIENT,

    "picman-dnd-get-gradient-func",
    "picman-dnd-get-gradient-data",

    "picman-dnd-set-gradient-func",
    "picman-dnd-set-gradient-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_object_data,
    picman_dnd_set_gradient_data
  },

  {
    PICMAN_TARGET_PALETTE,

    "picman-dnd-get-palette-func",
    "picman-dnd-get-palette-data",

    "picman-dnd-set-palette-func",
    "picman-dnd-set-palette-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_object_data,
    picman_dnd_set_palette_data
  },

  {
    PICMAN_TARGET_FONT,

    "picman-dnd-get-font-func",
    "picman-dnd-get-font-data",

    "picman-dnd-set-font-func",
    "picman-dnd-set-font-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_object_data,
    picman_dnd_set_font_data
  },

  {
    PICMAN_TARGET_BUFFER,

    "picman-dnd-get-buffer-func",
    "picman-dnd-get-buffer-data",

    "picman-dnd-set-buffer-func",
    "picman-dnd-set-buffer-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_object_data,
    picman_dnd_set_buffer_data
  },

  {
    PICMAN_TARGET_IMAGEFILE,

    "picman-dnd-get-imagefile-func",
    "picman-dnd-get-imagefile-data",

    "picman-dnd-set-imagefile-func",
    "picman-dnd-set-imagefile-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_object_data,
    picman_dnd_set_imagefile_data
  },

  {
    PICMAN_TARGET_TEMPLATE,

    "picman-dnd-get-template-func",
    "picman-dnd-get-template-data",

    "picman-dnd-set-template-func",
    "picman-dnd-set-template-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_object_data,
    picman_dnd_set_template_data
  },

  {
    PICMAN_TARGET_TOOL_INFO,

    "picman-dnd-get-tool-info-func",
    "picman-dnd-get-tool-info-data",

    "picman-dnd-set-tool-info-func",
    "picman-dnd-set-tool-info-data",

    picman_dnd_get_viewable_icon,
    picman_dnd_get_object_data,
    picman_dnd_set_tool_info_data
  },

  {
    PICMAN_TARGET_DIALOG,

    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,
    NULL
  }
};


static Picman *the_dnd_picman = NULL;


void
picman_dnd_init (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (the_dnd_picman == NULL);

  the_dnd_picman = picman;
}


/**********************/
/*  helper functions  */
/**********************/

static void
picman_dnd_target_list_add (GtkTargetList        *list,
                          const GtkTargetEntry *entry)
{
  GdkAtom atom = gdk_atom_intern (entry->target, FALSE);
  guint   info;

  if (! gtk_target_list_find (list, atom, &info) || info != entry->info)
    {
      gtk_target_list_add (list, atom, entry->flags, entry->info);
    }
}


/********************************/
/*  general data dnd functions  */
/********************************/

static void
picman_dnd_data_drag_begin (GtkWidget      *widget,
                          GdkDragContext *context,
                          gpointer        data)
{
  const PicmanDndDataDef *dnd_data;
  PicmanDndType           data_type;
  GCallback             get_data_func = NULL;
  gpointer              get_data_data = NULL;
  GtkWidget            *icon_widget;

  data_type = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                                  "picman-dnd-get-data-type"));

  PICMAN_LOG (DND, "data type %d", data_type);

  if (! data_type)
    return;

  dnd_data = dnd_data_defs + data_type;

  if (dnd_data->get_data_func_name)
    get_data_func = g_object_get_data (G_OBJECT (widget),
                                       dnd_data->get_data_func_name);

  if (dnd_data->get_data_data_name)
    get_data_data = g_object_get_data (G_OBJECT (widget),
                                       dnd_data->get_data_data_name);

  if (! get_data_func)
    return;

  icon_widget = dnd_data->get_icon_func (widget,
                                         get_data_func,
                                         get_data_data);

  if (icon_widget)
    {
      GtkWidget *frame;
      GtkWidget *window;

      window = gtk_window_new (GTK_WINDOW_POPUP);
      gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DND);
      gtk_window_set_screen (GTK_WINDOW (window),
                             gtk_widget_get_screen (widget));

      gtk_widget_realize (window);

      frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
      gtk_container_add (GTK_CONTAINER (window), frame);
      gtk_widget_show (frame);

      gtk_container_add (GTK_CONTAINER (frame), icon_widget);
      gtk_widget_show (icon_widget);

      g_object_set_data_full (G_OBJECT (widget), "picman-dnd-data-widget",
                              window, (GDestroyNotify) gtk_widget_destroy);

      gtk_drag_set_icon_widget (context, window,
                                DRAG_ICON_OFFSET, DRAG_ICON_OFFSET);

      /*  remember for which drag context the widget was made  */
      g_object_set_data (G_OBJECT (window), "picman-gdk-drag-context", context);
    }
}

static void
picman_dnd_data_drag_end (GtkWidget      *widget,
                        GdkDragContext *context)
{
  PicmanDndType  data_type;
  GtkWidget   *icon_widget;

  data_type = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                                  "picman-dnd-get-data-type"));

  PICMAN_LOG (DND, "data type %d", data_type);

  icon_widget = g_object_get_data (G_OBJECT (widget), "picman-dnd-data-widget");

  if (icon_widget)
    {
      /*  destroy the icon_widget only if it was made for this drag
       *  context. See bug #139337.
       */
      if (g_object_get_data (G_OBJECT (icon_widget),
                             "picman-gdk-drag-context") ==
          (gpointer) context)
        {
          g_object_set_data (G_OBJECT (widget), "picman-dnd-data-widget", NULL);
        }
    }
}

static void
picman_dnd_data_drag_handle (GtkWidget        *widget,
                           GdkDragContext   *context,
                           GtkSelectionData *selection_data,
                           guint             info,
                           guint             time,
                           gpointer          data)
{
  GCallback    get_data_func = NULL;
  gpointer     get_data_data = NULL;
  PicmanDndType  data_type;

  PICMAN_LOG (DND, "data type %d", info);

  for (data_type = PICMAN_DND_TYPE_NONE + 1;
       data_type <= PICMAN_DND_TYPE_LAST;
       data_type++)
    {
      const PicmanDndDataDef *dnd_data = dnd_data_defs + data_type;

      if (dnd_data->target_entry.info == info)
        {
          PICMAN_LOG (DND, "target %s", dnd_data->target_entry.target);

          if (dnd_data->get_data_func_name)
            get_data_func = g_object_get_data (G_OBJECT (widget),
                                               dnd_data->get_data_func_name);

          if (dnd_data->get_data_data_name)
            get_data_data = g_object_get_data (G_OBJECT (widget),
                                               dnd_data->get_data_data_name);

          if (! get_data_func)
            return;

          dnd_data->get_data_func (widget,
                                   context,
                                   get_data_func,
                                   get_data_data,
                                   selection_data);

          return;
        }
    }
}

static void
picman_dnd_data_drop_handle (GtkWidget        *widget,
                           GdkDragContext   *context,
                           gint              x,
                           gint              y,
                           GtkSelectionData *selection_data,
                           guint             info,
                           guint             time,
                           gpointer          data)
{
  PicmanDndType data_type;

  PICMAN_LOG (DND, "data type %d", info);

  if (gtk_selection_data_get_length (selection_data) <= 0)
    {
      gtk_drag_finish (context, FALSE, FALSE, time);
      return;
    }

  for (data_type = PICMAN_DND_TYPE_NONE + 1;
       data_type <= PICMAN_DND_TYPE_LAST;
       data_type++)
    {
      const PicmanDndDataDef *dnd_data = dnd_data_defs + data_type;

      if (dnd_data->target_entry.info == info)
        {
          GCallback set_data_func = NULL;
          gpointer  set_data_data = NULL;

          PICMAN_LOG (DND, "target %s", dnd_data->target_entry.target);

          if (dnd_data->set_data_func_name)
            set_data_func = g_object_get_data (G_OBJECT (widget),
                                               dnd_data->set_data_func_name);

          if (dnd_data->set_data_data_name)
            set_data_data = g_object_get_data (G_OBJECT (widget),
                                               dnd_data->set_data_data_name);

          if (set_data_func &&
              dnd_data->set_data_func (widget, x, y,
                                       set_data_func,
                                       set_data_data,
                                       selection_data))
            {
              gtk_drag_finish (context, TRUE, FALSE, time);
              return;
            }

          gtk_drag_finish (context, FALSE, FALSE, time);
          return;
        }
    }
}

static void
picman_dnd_data_source_add (PicmanDndType  data_type,
                          GtkWidget   *widget,
                          GCallback    get_data_func,
                          gpointer     get_data_data)
{
  const PicmanDndDataDef *dnd_data;
  gboolean              drag_connected;

  dnd_data = dnd_data_defs + data_type;

  /*  set a default drag source if not already done  */
  if (! g_object_get_data (G_OBJECT (widget), "gtk-site-data"))
    gtk_drag_source_set (widget, GDK_BUTTON1_MASK | GDK_BUTTON2_MASK,
                         NULL, 0,
                         GDK_ACTION_COPY | GDK_ACTION_MOVE);

  drag_connected =
    GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                        "picman-dnd-drag-connected"));

  if (! drag_connected)
    {
      g_signal_connect (widget, "drag-begin",
                        G_CALLBACK (picman_dnd_data_drag_begin),
                        NULL);
      g_signal_connect (widget, "drag-end",
                        G_CALLBACK (picman_dnd_data_drag_end),
                        NULL);
      g_signal_connect (widget, "drag-data-get",
                        G_CALLBACK (picman_dnd_data_drag_handle),
                        NULL);

      g_object_set_data (G_OBJECT (widget), "picman-dnd-drag-connected",
                         GINT_TO_POINTER (TRUE));
    }

  g_object_set_data (G_OBJECT (widget), dnd_data->get_data_func_name,
                     get_data_func);
  g_object_set_data (G_OBJECT (widget), dnd_data->get_data_data_name,
                     get_data_data);

  /*  remember the first set source type for drag view creation  */
  if (! g_object_get_data (G_OBJECT (widget), "picman-dnd-get-data-type"))
    g_object_set_data (G_OBJECT (widget), "picman-dnd-get-data-type",
                       GINT_TO_POINTER (data_type));

  if (dnd_data->target_entry.target)
    {
      GtkTargetList *target_list;

      target_list = gtk_drag_source_get_target_list (widget);

      if (target_list)
        {
          picman_dnd_target_list_add (target_list, &dnd_data->target_entry);
        }
      else
        {
          target_list = gtk_target_list_new (&dnd_data->target_entry, 1);

          gtk_drag_source_set_target_list (widget, target_list);
          gtk_target_list_unref (target_list);
        }
    }
}

static void
picman_dnd_data_source_remove (PicmanDndType  data_type,
                             GtkWidget   *widget)
{
  const PicmanDndDataDef *dnd_data;
  gboolean              drag_connected;

  drag_connected =
    GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                        "picman-dnd-drag-connected"));

  if (! drag_connected)
    return;

  dnd_data = dnd_data_defs + data_type;

  g_object_set_data (G_OBJECT (widget), dnd_data->get_data_func_name, NULL);
  g_object_set_data (G_OBJECT (widget), dnd_data->get_data_data_name, NULL);

  /*  remove the dnd type remembered for the dnd icon  */
  if (data_type ==
      GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                          "picman-dnd-get-data-type")))
    g_object_set_data (G_OBJECT (widget), "picman-dnd-get-data-type", NULL);

  if (dnd_data->target_entry.target)
    {
      GtkTargetList *target_list;

      target_list = gtk_drag_source_get_target_list (widget);

      if (target_list)
        {
          GdkAtom atom = gdk_atom_intern (dnd_data->target_entry.target, TRUE);

          if (atom != GDK_NONE)
            gtk_target_list_remove (target_list, atom);
        }
    }
}

static void
picman_dnd_data_dest_add (PicmanDndType  data_type,
                        GtkWidget   *widget,
                        gpointer     set_data_func,
                        gpointer     set_data_data)
{
  const PicmanDndDataDef *dnd_data;
  gboolean              drop_connected;

  /*  set a default drag dest if not already done  */
  if (! g_object_get_data (G_OBJECT (widget), "gtk-drag-dest"))
    gtk_drag_dest_set (widget, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);

  drop_connected =
    GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                        "picman-dnd-drop-connected"));

  if (set_data_func && ! drop_connected)
    {
      g_signal_connect (widget, "drag-data-received",
                        G_CALLBACK (picman_dnd_data_drop_handle),
                        NULL);

      g_object_set_data (G_OBJECT (widget), "picman-dnd-drop-connected",
                         GINT_TO_POINTER (TRUE));
    }

  dnd_data = dnd_data_defs + data_type;

  if (set_data_func)
    {
      g_object_set_data (G_OBJECT (widget), dnd_data->set_data_func_name,
                         set_data_func);
      g_object_set_data (G_OBJECT (widget), dnd_data->set_data_data_name,
                         set_data_data);
    }

  if (dnd_data->target_entry.target)
    {
      GtkTargetList *target_list;

      target_list = gtk_drag_dest_get_target_list (widget);

      if (target_list)
        {
          picman_dnd_target_list_add (target_list, &dnd_data->target_entry);
        }
      else
        {
          target_list = gtk_target_list_new (&dnd_data->target_entry, 1);

          gtk_drag_dest_set_target_list (widget, target_list);
          gtk_target_list_unref (target_list);
        }
    }
}

static void
picman_dnd_data_dest_remove (PicmanDndType  data_type,
                           GtkWidget   *widget)
{
  const PicmanDndDataDef *dnd_data;

  dnd_data = dnd_data_defs + data_type;

  g_object_set_data (G_OBJECT (widget), dnd_data->set_data_func_name, NULL);
  g_object_set_data (G_OBJECT (widget), dnd_data->set_data_data_name, NULL);

  if (dnd_data->target_entry.target)
    {
      GtkTargetList *target_list;

      target_list = gtk_drag_dest_get_target_list (widget);

      if (target_list)
        {
          GdkAtom atom = gdk_atom_intern (dnd_data->target_entry.target, TRUE);

          if (atom != GDK_NONE)
            gtk_target_list_remove (target_list, atom);
        }
    }
}


/****************************/
/*  uri list dnd functions  */
/****************************/

static void
picman_dnd_get_uri_list_data (GtkWidget        *widget,
                            GdkDragContext   *context,
                            GCallback         get_uri_list_func,
                            gpointer          get_uri_list_data,
                            GtkSelectionData *selection)
{
  GList *uri_list;

  uri_list = (* (PicmanDndDragUriListFunc) get_uri_list_func) (widget,
                                                             get_uri_list_data);

  if (uri_list)
    {
      picman_selection_data_set_uri_list (selection, uri_list);

      g_list_free_full (uri_list, (GDestroyNotify) g_free);
    }
}

static gboolean
picman_dnd_set_uri_list_data (GtkWidget        *widget,
                            gint              x,
                            gint              y,
                            GCallback         set_uri_list_func,
                            gpointer          set_uri_list_data,
                            GtkSelectionData *selection)
{
  GList *uri_list = picman_selection_data_get_uri_list (selection);

  if (! uri_list)
    return FALSE;

  (* (PicmanDndDropUriListFunc) set_uri_list_func) (widget, x, y, uri_list,
                                                  set_uri_list_data);

  g_list_free_full (uri_list, (GDestroyNotify) g_free);

  return TRUE;
}

void
picman_dnd_uri_list_source_add (GtkWidget              *widget,
                              PicmanDndDragUriListFunc  get_uri_list_func,
                              gpointer                data)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_add (PICMAN_DND_TYPE_URI_LIST, widget,
                            G_CALLBACK (get_uri_list_func),
                            data);
}

void
picman_dnd_uri_list_source_remove (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_remove (PICMAN_DND_TYPE_URI_LIST, widget);
}

void
picman_dnd_uri_list_dest_add (GtkWidget              *widget,
                            PicmanDndDropUriListFunc  set_uri_list_func,
                            gpointer                data)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  /*  Set a default drag dest if not already done. Explicitely set
   *  COPY and MOVE for file drag destinations. Some file managers
   *  such as Konqueror only offer MOVE by default.
   */
  if (! g_object_get_data (G_OBJECT (widget), "gtk-drag-dest"))
    gtk_drag_dest_set (widget,
                       GTK_DEST_DEFAULT_ALL, NULL, 0,
                       GDK_ACTION_COPY | GDK_ACTION_MOVE);

  picman_dnd_data_dest_add (PICMAN_DND_TYPE_URI_LIST, widget,
                          G_CALLBACK (set_uri_list_func),
                          data);
  picman_dnd_data_dest_add (PICMAN_DND_TYPE_TEXT_PLAIN, widget,
                          G_CALLBACK (set_uri_list_func),
                          data);
  picman_dnd_data_dest_add (PICMAN_DND_TYPE_NETSCAPE_URL, widget,
                          G_CALLBACK (set_uri_list_func),
                          data);
}

void
picman_dnd_uri_list_dest_remove (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_dest_remove (PICMAN_DND_TYPE_URI_LIST, widget);
  picman_dnd_data_dest_remove (PICMAN_DND_TYPE_TEXT_PLAIN, widget);
  picman_dnd_data_dest_remove (PICMAN_DND_TYPE_NETSCAPE_URL, widget);
}


/******************************/
/* Direct Save Protocol (XDS) */
/******************************/

static void
picman_dnd_get_xds_data (GtkWidget        *widget,
                       GdkDragContext   *context,
                       GCallback         get_image_func,
                       gpointer          get_image_data,
                       GtkSelectionData *selection)
{
  PicmanImage   *image;
  PicmanContext *picman_context;

  image = (PicmanImage *)
    (* (PicmanDndDragViewableFunc) get_image_func) (widget, &picman_context,
                                                  get_image_data);

  if (image)
    picman_dnd_xds_save_image (context, image, selection);
}

static void
picman_dnd_xds_drag_begin (GtkWidget      *widget,
                         GdkDragContext *context)
{
  const PicmanDndDataDef *dnd_data = dnd_data_defs + PICMAN_DND_TYPE_XDS;
  GCallback             get_data_func;
  gpointer              get_data_data;

  get_data_func = g_object_get_data (G_OBJECT (widget),
                                     dnd_data->get_data_func_name);
  get_data_data = g_object_get_data (G_OBJECT (widget),
                                     dnd_data->get_data_data_name);

  if (get_data_func)
    {
      PicmanImage   *image;
      PicmanContext *picman_context;

      image = (PicmanImage *)
        (* (PicmanDndDragViewableFunc) get_data_func) (widget, &picman_context,
                                                     get_data_data);

      picman_dnd_xds_source_set (context, image);
    }
}

static void
picman_dnd_xds_drag_end (GtkWidget      *widget,
                       GdkDragContext *context)
{
  picman_dnd_xds_source_set (context, NULL);
}

void
picman_dnd_xds_source_add (GtkWidget               *widget,
                         PicmanDndDragViewableFunc  get_image_func,
                         gpointer                 data)
{
  gulong handler;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_add (PICMAN_DND_TYPE_XDS, widget,
                            G_CALLBACK (get_image_func),
                            data);

  handler = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (widget),
                                                 "picman-dnd-xds-drag-begin"));

  if (! handler)
    {
      handler = g_signal_connect (widget, "drag-begin",
                                  G_CALLBACK (picman_dnd_xds_drag_begin),
                                  NULL);
      g_object_set_data (G_OBJECT (widget), "picman-dnd-xds-drag-begin",
                         GUINT_TO_POINTER (handler));
    }

  handler = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (widget),
                                                 "picman-dnd-xds-drag-end"));

  if (! handler)
    {
      handler = g_signal_connect (widget, "drag-end",
                                  G_CALLBACK (picman_dnd_xds_drag_end),
                                  NULL);
      g_object_set_data (G_OBJECT (widget), "picman-dnd-xds-drag-end",
                         GUINT_TO_POINTER (handler));
    }
}

void
picman_dnd_xds_source_remove (GtkWidget *widget)
{
  gulong handler;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  handler = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (widget),
                                                 "picman-dnd-xds-drag-begin"));
  if (handler)
    {
      g_signal_handler_disconnect (widget, handler);
      g_object_set_data (G_OBJECT (widget), "picman-dnd-xds-drag-begin", NULL);
    }

  handler = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (widget),
                                                 "picman-dnd-xds-drag-end"));
  if (handler)
    {
      g_signal_handler_disconnect (widget, handler);
      g_object_set_data (G_OBJECT (widget), "picman-dnd-xds-drag-end", NULL);
    }

  picman_dnd_data_source_remove (PICMAN_DND_TYPE_XDS, widget);
}


/*************************/
/*  color dnd functions  */
/*************************/

static GtkWidget *
picman_dnd_get_color_icon (GtkWidget *widget,
                         GCallback  get_color_func,
                         gpointer   get_color_data)
{
  GtkWidget *color_area;
  PicmanRGB    color;

  (* (PicmanDndDragColorFunc) get_color_func) (widget, &color, get_color_data);

  color_area = picman_color_area_new (&color, PICMAN_COLOR_AREA_SMALL_CHECKS, 0);
  gtk_widget_set_size_request (color_area,
                               DRAG_PREVIEW_SIZE, DRAG_PREVIEW_SIZE);

  return color_area;
}

static void
picman_dnd_get_color_data (GtkWidget        *widget,
                         GdkDragContext   *context,
                         GCallback         get_color_func,
                         gpointer          get_color_data,
                         GtkSelectionData *selection)
{
  PicmanRGB color;

  (* (PicmanDndDragColorFunc) get_color_func) (widget, &color, get_color_data);

  picman_selection_data_set_color (selection, &color);
}

static gboolean
picman_dnd_set_color_data (GtkWidget        *widget,
                         gint              x,
                         gint              y,
                         GCallback         set_color_func,
                         gpointer          set_color_data,
                         GtkSelectionData *selection)
{
  PicmanRGB color;

  if (! picman_selection_data_get_color (selection, &color))
    return FALSE;

  (* (PicmanDndDropColorFunc) set_color_func) (widget, x, y, &color,
                                             set_color_data);

  return TRUE;
}

void
picman_dnd_color_source_add (GtkWidget            *widget,
                           PicmanDndDragColorFunc  get_color_func,
                           gpointer              data)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_add (PICMAN_DND_TYPE_COLOR, widget,
                            G_CALLBACK (get_color_func),
                            data);
}

void
picman_dnd_color_source_remove (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_remove (PICMAN_DND_TYPE_COLOR, widget);
}

void
picman_dnd_color_dest_add (GtkWidget            *widget,
                         PicmanDndDropColorFunc  set_color_func,
                         gpointer              data)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_dest_add (PICMAN_DND_TYPE_COLOR, widget,
                          G_CALLBACK (set_color_func),
                          data);
}

void
picman_dnd_color_dest_remove (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_dest_remove (PICMAN_DND_TYPE_COLOR, widget);
}


/**************************/
/*  stream dnd functions  */
/**************************/

static void
picman_dnd_get_stream_data (GtkWidget        *widget,
                          GdkDragContext   *context,
                          GCallback         get_stream_func,
                          gpointer          get_stream_data,
                          GtkSelectionData *selection)
{
  guchar *stream;
  gsize   stream_length;

  stream = (* (PicmanDndDragStreamFunc) get_stream_func) (widget, &stream_length,
                                                        get_stream_data);

  if (stream)
    {
      picman_selection_data_set_stream (selection, stream, stream_length);
      g_free (stream);
    }
}

static gboolean
picman_dnd_set_stream_data (GtkWidget        *widget,
                          gint              x,
                          gint              y,
                          GCallback         set_stream_func,
                          gpointer          set_stream_data,
                          GtkSelectionData *selection)
{
  const guchar *stream;
  gsize         stream_length;

  stream = picman_selection_data_get_stream (selection, &stream_length);

  if (! stream)
    return FALSE;

  (* (PicmanDndDropStreamFunc) set_stream_func) (widget, x, y,
                                               stream, stream_length,
                                               set_stream_data);

  return TRUE;
}

void
picman_dnd_svg_source_add (GtkWidget             *widget,
                         PicmanDndDragStreamFunc  get_svg_func,
                         gpointer               data)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_add (PICMAN_DND_TYPE_SVG, widget,
                            G_CALLBACK (get_svg_func),
                            data);
  picman_dnd_data_source_add (PICMAN_DND_TYPE_SVG_XML, widget,
                            G_CALLBACK (get_svg_func),
                            data);
}

void
picman_dnd_svg_source_remove (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_remove (PICMAN_DND_TYPE_SVG, widget);
  picman_dnd_data_source_remove (PICMAN_DND_TYPE_SVG_XML, widget);
}

void
picman_dnd_svg_dest_add (GtkWidget             *widget,
                       PicmanDndDropStreamFunc  set_svg_func,
                       gpointer               data)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_dest_add (PICMAN_DND_TYPE_SVG, widget,
                          G_CALLBACK (set_svg_func),
                          data);
  picman_dnd_data_dest_add (PICMAN_DND_TYPE_SVG_XML, widget,
                          G_CALLBACK (set_svg_func),
                          data);
}

void
picman_dnd_svg_dest_remove (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_dest_remove (PICMAN_DND_TYPE_SVG, widget);
  picman_dnd_data_dest_remove (PICMAN_DND_TYPE_SVG_XML, widget);
}


/**************************/
/*  pixbuf dnd functions  */
/**************************/

static void
picman_dnd_get_pixbuf_data (GtkWidget        *widget,
                          GdkDragContext   *context,
                          GCallback         get_pixbuf_func,
                          gpointer          get_pixbuf_data,
                          GtkSelectionData *selection)
{
  GdkPixbuf *pixbuf;

  pixbuf = (* (PicmanDndDragPixbufFunc) get_pixbuf_func) (widget,
                                                        get_pixbuf_data);

  if (pixbuf)
    {
      picman_set_busy (the_dnd_picman);

      gtk_selection_data_set_pixbuf (selection, pixbuf);
      g_object_unref (pixbuf);

      picman_unset_busy (the_dnd_picman);
    }
}

static gboolean
picman_dnd_set_pixbuf_data (GtkWidget        *widget,
                          gint              x,
                          gint              y,
                          GCallback         set_pixbuf_func,
                          gpointer          set_pixbuf_data,
                          GtkSelectionData *selection)
{
  GdkPixbuf *pixbuf;

  picman_set_busy (the_dnd_picman);

  pixbuf = gtk_selection_data_get_pixbuf (selection);

  picman_unset_busy (the_dnd_picman);

  if (! pixbuf)
    return FALSE;

  (* (PicmanDndDropPixbufFunc) set_pixbuf_func) (widget, x, y,
                                               pixbuf,
                                               set_pixbuf_data);

  g_object_unref (pixbuf);

  return TRUE;
}

void
picman_dnd_pixbuf_source_add (GtkWidget             *widget,
                            PicmanDndDragPixbufFunc  get_pixbuf_func,
                            gpointer               data)
{
  GtkTargetList *target_list;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_add (PICMAN_DND_TYPE_PIXBUF, widget,
                            G_CALLBACK (get_pixbuf_func),
                            data);

  target_list = gtk_drag_source_get_target_list (widget);

  if (target_list)
    gtk_target_list_ref (target_list);
  else
    target_list = gtk_target_list_new (NULL, 0);

  picman_pixbuf_targets_add (target_list, PICMAN_DND_TYPE_PIXBUF, TRUE);

  gtk_drag_source_set_target_list (widget, target_list);
  gtk_target_list_unref (target_list);
}

void
picman_dnd_pixbuf_source_remove (GtkWidget *widget)
{
  GtkTargetList *target_list;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_remove (PICMAN_DND_TYPE_PIXBUF, widget);

  target_list = gtk_drag_source_get_target_list (widget);

  if (target_list)
    picman_pixbuf_targets_remove (target_list);
}

void
picman_dnd_pixbuf_dest_add (GtkWidget              *widget,
                          PicmanDndDropPixbufFunc   set_pixbuf_func,
                          gpointer                data)
{
  GtkTargetList *target_list;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_dest_add (PICMAN_DND_TYPE_PIXBUF, widget,
                          G_CALLBACK (set_pixbuf_func),
                          data);

  target_list = gtk_drag_dest_get_target_list (widget);

  if (target_list)
    gtk_target_list_ref (target_list);
  else
    target_list = gtk_target_list_new (NULL, 0);

  picman_pixbuf_targets_add (target_list, PICMAN_DND_TYPE_PIXBUF, FALSE);

  gtk_drag_dest_set_target_list (widget, target_list);
  gtk_target_list_unref (target_list);
}

void
picman_dnd_pixbuf_dest_remove (GtkWidget *widget)
{
  GtkTargetList *target_list;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_dest_remove (PICMAN_DND_TYPE_PIXBUF, widget);

  target_list = gtk_drag_dest_get_target_list (widget);

  if (target_list)
    picman_pixbuf_targets_remove (target_list);
}


/*****************************/
/*  component dnd functions  */
/*****************************/

static GtkWidget *
picman_dnd_get_component_icon (GtkWidget *widget,
                             GCallback  get_comp_func,
                             gpointer   get_comp_data)
{
  GtkWidget       *view;
  PicmanImage       *image;
  PicmanContext     *context;
  PicmanChannelType  channel;

  image = (* (PicmanDndDragComponentFunc) get_comp_func) (widget, &context,
                                                        &channel,
                                                        get_comp_data);

  if (! image)
    return NULL;

  view = picman_view_new (context, PICMAN_VIEWABLE (image),
                        DRAG_PREVIEW_SIZE, 0, TRUE);

  PICMAN_VIEW_RENDERER_IMAGE (PICMAN_VIEW (view)->renderer)->channel = channel;

  return view;
}

static void
picman_dnd_get_component_data (GtkWidget        *widget,
                             GdkDragContext   *context,
                             GCallback         get_comp_func,
                             gpointer          get_comp_data,
                             GtkSelectionData *selection)
{
  PicmanImage       *image;
  PicmanContext     *picman_context;
  PicmanChannelType  channel = 0;

  image = (* (PicmanDndDragComponentFunc) get_comp_func) (widget, &picman_context,
                                                        &channel,
                                                        get_comp_data);

  if (image)
    picman_selection_data_set_component (selection, image, channel);
}

static gboolean
picman_dnd_set_component_data (GtkWidget        *widget,
                             gint              x,
                             gint              y,
                             GCallback         set_comp_func,
                             gpointer          set_comp_data,
                             GtkSelectionData *selection)
{
  PicmanImage       *image;
  PicmanChannelType  channel = 0;

  image = picman_selection_data_get_component (selection, the_dnd_picman,
                                             &channel);

  if (! image)
    return FALSE;

  (* (PicmanDndDropComponentFunc) set_comp_func) (widget, x, y,
                                                image, channel,
                                                set_comp_data);

  return TRUE;
}

void
picman_dnd_component_source_add (GtkWidget                *widget,
                               PicmanDndDragComponentFunc  get_comp_func,
                               gpointer                  data)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_add (PICMAN_DND_TYPE_COMPONENT, widget,
                            G_CALLBACK (get_comp_func),
                            data);
}

void
picman_dnd_component_source_remove (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_source_remove (PICMAN_DND_TYPE_COMPONENT, widget);
}

void
picman_dnd_component_dest_add (GtkWidget                 *widget,
                             PicmanDndDropComponentFunc   set_comp_func,
                             gpointer                   data)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_dest_add (PICMAN_DND_TYPE_COMPONENT, widget,
                          G_CALLBACK (set_comp_func),
                          data);
}

void
picman_dnd_component_dest_remove (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  picman_dnd_data_dest_remove (PICMAN_DND_TYPE_COMPONENT, widget);
}


/*******************************************/
/*  PicmanViewable (by GType) dnd functions  */
/*******************************************/

static GtkWidget *
picman_dnd_get_viewable_icon (GtkWidget *widget,
                            GCallback  get_viewable_func,
                            gpointer   get_viewable_data)
{
  PicmanViewable *viewable;
  PicmanContext  *context;
  GtkWidget    *view;
  gchar        *desc;

  viewable = (* (PicmanDndDragViewableFunc) get_viewable_func) (widget, &context,
                                                              get_viewable_data);

  if (! viewable)
    return NULL;

  view = picman_view_new (context, viewable,
                        DRAG_PREVIEW_SIZE, 0, TRUE);

  desc = picman_viewable_get_description (viewable, NULL);

  if (desc)
    {
      GtkWidget *hbox;
      GtkWidget *label;

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
      gtk_box_pack_start (GTK_BOX (hbox), view, FALSE, FALSE, 0);
      gtk_widget_show (view);

      label = g_object_new (GTK_TYPE_LABEL,
                            "label",           desc,
                            "xpad",            3,
                            "xalign",          0.0,
                            "yalign",          0.5,
                            "max-width-chars", 30,
                            "ellipsize",       PANGO_ELLIPSIZE_END,
                            NULL);

      g_free (desc);

      gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
      gtk_widget_show (label);

      return hbox;
    }

  return view;
}

static PicmanDndType
picman_dnd_data_type_get_by_g_type (GType type)
{
  PicmanDndType dnd_type = PICMAN_DND_TYPE_NONE;

  if (g_type_is_a (type, PICMAN_TYPE_IMAGE))
    {
      dnd_type = PICMAN_DND_TYPE_IMAGE;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_LAYER))
    {
      dnd_type = PICMAN_DND_TYPE_LAYER;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_LAYER_MASK))
    {
      dnd_type = PICMAN_DND_TYPE_LAYER_MASK;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_CHANNEL))
    {
      dnd_type = PICMAN_DND_TYPE_CHANNEL;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_VECTORS))
    {
      dnd_type = PICMAN_DND_TYPE_VECTORS;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_BRUSH))
    {
      dnd_type = PICMAN_DND_TYPE_BRUSH;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_PATTERN))
    {
      dnd_type = PICMAN_DND_TYPE_PATTERN;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_GRADIENT))
    {
      dnd_type = PICMAN_DND_TYPE_GRADIENT;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_PALETTE))
    {
      dnd_type = PICMAN_DND_TYPE_PALETTE;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_FONT))
    {
      dnd_type = PICMAN_DND_TYPE_FONT;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_BUFFER))
    {
      dnd_type = PICMAN_DND_TYPE_BUFFER;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_IMAGEFILE))
    {
      dnd_type = PICMAN_DND_TYPE_IMAGEFILE;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_TEMPLATE))
    {
      dnd_type = PICMAN_DND_TYPE_TEMPLATE;
    }
  else if (g_type_is_a (type, PICMAN_TYPE_TOOL_INFO))
    {
      dnd_type = PICMAN_DND_TYPE_TOOL_INFO;
    }

  return dnd_type;
}

gboolean
picman_dnd_drag_source_set_by_type (GtkWidget       *widget,
                                  GdkModifierType  start_button_mask,
                                  GType            type,
                                  GdkDragAction    actions)
{
  PicmanDndType dnd_type;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  dnd_type = picman_dnd_data_type_get_by_g_type (type);

  if (dnd_type == PICMAN_DND_TYPE_NONE)
    return FALSE;

  gtk_drag_source_set (widget, start_button_mask,
                       &dnd_data_defs[dnd_type].target_entry, 1,
                       actions);

  return TRUE;
}

gboolean
picman_dnd_drag_dest_set_by_type (GtkWidget       *widget,
                                GtkDestDefaults  flags,
                                GType            type,
                                GdkDragAction    actions)
{
  PicmanDndType dnd_type;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  dnd_type = picman_dnd_data_type_get_by_g_type (type);

  if (dnd_type == PICMAN_DND_TYPE_NONE)
    return FALSE;

  gtk_drag_dest_set (widget, flags,
                     &dnd_data_defs[dnd_type].target_entry, 1,
                     actions);

  return TRUE;
}

gboolean
picman_dnd_viewable_source_add (GtkWidget               *widget,
                              GType                    type,
                              PicmanDndDragViewableFunc  get_viewable_func,
                              gpointer                 data)
{
  PicmanDndType dnd_type;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
  g_return_val_if_fail (get_viewable_func != NULL, FALSE);

  dnd_type = picman_dnd_data_type_get_by_g_type (type);

  if (dnd_type == PICMAN_DND_TYPE_NONE)
    return FALSE;

  picman_dnd_data_source_add (dnd_type, widget,
                            G_CALLBACK (get_viewable_func),
                            data);

  return TRUE;
}

gboolean
picman_dnd_viewable_source_remove (GtkWidget *widget,
                                 GType      type)
{
  PicmanDndType dnd_type;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  dnd_type = picman_dnd_data_type_get_by_g_type (type);

  if (dnd_type == PICMAN_DND_TYPE_NONE)
    return FALSE;

  picman_dnd_data_source_remove (dnd_type, widget);

  return TRUE;
}

gboolean
picman_dnd_viewable_dest_add (GtkWidget               *widget,
                            GType                    type,
                            PicmanDndDropViewableFunc  set_viewable_func,
                            gpointer                 data)
{
  PicmanDndType dnd_type;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  dnd_type = picman_dnd_data_type_get_by_g_type (type);

  if (dnd_type == PICMAN_DND_TYPE_NONE)
    return FALSE;

  picman_dnd_data_dest_add (dnd_type, widget,
                          G_CALLBACK (set_viewable_func),
                          data);

  return TRUE;
}

gboolean
picman_dnd_viewable_dest_remove (GtkWidget *widget,
                               GType      type)
{
  PicmanDndType dnd_type;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  dnd_type = picman_dnd_data_type_get_by_g_type (type);

  if (dnd_type == PICMAN_DND_TYPE_NONE)
    return FALSE;

  picman_dnd_data_dest_remove (dnd_type, widget);

  return TRUE;
}

PicmanViewable *
picman_dnd_get_drag_data (GtkWidget *widget)
{
  const PicmanDndDataDef    *dnd_data;
  PicmanDndType              data_type;
  PicmanDndDragViewableFunc  get_data_func = NULL;
  gpointer                 get_data_data = NULL;
  PicmanContext             *context;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  data_type = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                                  "picman-dnd-get-data-type"));

  if (! data_type)
    return NULL;

  dnd_data = dnd_data_defs + data_type;

  if (dnd_data->get_data_func_name)
    get_data_func = g_object_get_data (G_OBJECT (widget),
                                       dnd_data->get_data_func_name);

  if (dnd_data->get_data_data_name)
    get_data_data = g_object_get_data (G_OBJECT (widget),
                                       dnd_data->get_data_data_name);

  if (! get_data_func)
    return NULL;

  return (PicmanViewable *) (* get_data_func) (widget, &context, get_data_data);

}


/*****************************/
/*  PicmanImage dnd functions  */
/*****************************/

static void
picman_dnd_get_image_data (GtkWidget        *widget,
                         GdkDragContext   *context,
                         GCallback         get_image_func,
                         gpointer          get_image_data,
                         GtkSelectionData *selection)
{
  PicmanImage   *image;
  PicmanContext *picman_context;

  image = (PicmanImage *)
    (* (PicmanDndDragViewableFunc) get_image_func) (widget, &picman_context,
                                                  get_image_data);

  if (image)
    picman_selection_data_set_image (selection, image);
}

static gboolean
picman_dnd_set_image_data (GtkWidget        *widget,
                         gint              x,
                         gint              y,
                         GCallback         set_image_func,
                         gpointer          set_image_data,
                         GtkSelectionData *selection)
{
  PicmanImage *image = picman_selection_data_get_image (selection, the_dnd_picman);

  if (! image)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_image_func) (widget, x, y,
                                                PICMAN_VIEWABLE (image),
                                                set_image_data);

  return TRUE;
}


/****************************/
/*  PicmanItem dnd functions  */
/****************************/

static void
picman_dnd_get_item_data (GtkWidget        *widget,
                        GdkDragContext   *context,
                        GCallback         get_item_func,
                        gpointer          get_item_data,
                        GtkSelectionData *selection)
{
  PicmanItem    *item;
  PicmanContext *picman_context;

  item = (PicmanItem *)
    (* (PicmanDndDragViewableFunc) get_item_func) (widget, &picman_context,
                                                 get_item_data);

  if (item)
    picman_selection_data_set_item (selection, item);
}

static gboolean
picman_dnd_set_item_data (GtkWidget        *widget,
                        gint              x,
                        gint              y,
                        GCallback         set_item_func,
                        gpointer          set_item_data,
                        GtkSelectionData *selection)
{
  PicmanItem *item = picman_selection_data_get_item (selection, the_dnd_picman);

  if (! item)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_item_func) (widget, x, y,
                                               PICMAN_VIEWABLE (item),
                                               set_item_data);

  return TRUE;
}


/******************************/
/*  PicmanObject dnd functions  */
/******************************/

static void
picman_dnd_get_object_data (GtkWidget        *widget,
                          GdkDragContext   *context,
                          GCallback         get_object_func,
                          gpointer          get_object_data,
                          GtkSelectionData *selection)
{
  PicmanObject  *object;
  PicmanContext *picman_context;

  object = (PicmanObject *)
    (* (PicmanDndDragViewableFunc) get_object_func) (widget, &picman_context,
                                                   get_object_data);

  if (PICMAN_IS_OBJECT (object))
    picman_selection_data_set_object (selection, object);
}


/*****************************/
/*  PicmanBrush dnd functions  */
/*****************************/

static gboolean
picman_dnd_set_brush_data (GtkWidget        *widget,
                         gint              x,
                         gint              y,
                         GCallback         set_brush_func,
                         gpointer          set_brush_data,
                         GtkSelectionData *selection)
{
  PicmanBrush *brush = picman_selection_data_get_brush (selection, the_dnd_picman);

  if (! brush)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_brush_func) (widget, x, y,
                                                PICMAN_VIEWABLE (brush),
                                                set_brush_data);

  return TRUE;
}


/*******************************/
/*  PicmanPattern dnd functions  */
/*******************************/

static gboolean
picman_dnd_set_pattern_data (GtkWidget        *widget,
                           gint              x,
                           gint              y,
                           GCallback         set_pattern_func,
                           gpointer          set_pattern_data,
                           GtkSelectionData *selection)
{
  PicmanPattern *pattern = picman_selection_data_get_pattern (selection,
                                                          the_dnd_picman);

  if (! pattern)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_pattern_func) (widget, x, y,
                                                  PICMAN_VIEWABLE (pattern),
                                                  set_pattern_data);

  return TRUE;
}


/********************************/
/*  PicmanGradient dnd functions  */
/********************************/

static gboolean
picman_dnd_set_gradient_data (GtkWidget        *widget,
                            gint              x,
                            gint              y,
                            GCallback         set_gradient_func,
                            gpointer          set_gradient_data,
                            GtkSelectionData *selection)
{
  PicmanGradient *gradient = picman_selection_data_get_gradient (selection,
                                                             the_dnd_picman);

  if (! gradient)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_gradient_func) (widget, x, y,
                                                   PICMAN_VIEWABLE (gradient),
                                                   set_gradient_data);

  return TRUE;
}


/*******************************/
/*  PicmanPalette dnd functions  */
/*******************************/

static gboolean
picman_dnd_set_palette_data (GtkWidget        *widget,
                           gint              x,
                           gint              y,
                           GCallback         set_palette_func,
                           gpointer          set_palette_data,
                           GtkSelectionData *selection)
{
  PicmanPalette *palette = picman_selection_data_get_palette (selection,
                                                          the_dnd_picman);

  if (! palette)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_palette_func) (widget, x, y,
                                                  PICMAN_VIEWABLE (palette),
                                                  set_palette_data);

  return TRUE;
}


/****************************/
/*  PicmanFont dnd functions  */
/****************************/

static gboolean
picman_dnd_set_font_data (GtkWidget        *widget,
                        gint              x,
                        gint              y,
                        GCallback         set_font_func,
                        gpointer          set_font_data,
                        GtkSelectionData *selection)
{
  PicmanFont *font = picman_selection_data_get_font (selection, the_dnd_picman);

  if (! font)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_font_func) (widget, x, y,
                                               PICMAN_VIEWABLE (font),
                                               set_font_data);

  return TRUE;
}


/******************************/
/*  PicmanBuffer dnd functions  */
/******************************/

static gboolean
picman_dnd_set_buffer_data (GtkWidget        *widget,
                          gint              x,
                          gint              y,
                          GCallback         set_buffer_func,
                          gpointer          set_buffer_data,
                          GtkSelectionData *selection)
{
  PicmanBuffer *buffer = picman_selection_data_get_buffer (selection, the_dnd_picman);

  if (! buffer)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_buffer_func) (widget, x, y,
                                                 PICMAN_VIEWABLE (buffer),
                                                 set_buffer_data);

  return TRUE;
}


/*********************************/
/*  PicmanImagefile dnd functions  */
/*********************************/

static gboolean
picman_dnd_set_imagefile_data (GtkWidget        *widget,
                             gint              x,
                             gint              y,
                             GCallback         set_imagefile_func,
                             gpointer          set_imagefile_data,
                             GtkSelectionData *selection)
{
  PicmanImagefile *imagefile = picman_selection_data_get_imagefile (selection,
                                                                the_dnd_picman);

  if (! imagefile)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_imagefile_func) (widget, x, y,
                                                    PICMAN_VIEWABLE (imagefile),
                                                    set_imagefile_data);

  return TRUE;
}


/********************************/
/*  PicmanTemplate dnd functions  */
/********************************/

static gboolean
picman_dnd_set_template_data (GtkWidget        *widget,
                            gint              x,
                            gint              y,
                            GCallback         set_template_func,
                            gpointer          set_template_data,
                            GtkSelectionData *selection)
{
  PicmanTemplate *template = picman_selection_data_get_template (selection,
                                                             the_dnd_picman);

  if (! template)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_template_func) (widget, x, y,
                                                   PICMAN_VIEWABLE (template),
                                                   set_template_data);

  return TRUE;
}


/********************************/
/*  PicmanToolInfo dnd functions  */
/********************************/

static gboolean
picman_dnd_set_tool_info_data (GtkWidget        *widget,
                             gint              x,
                             gint              y,
                             GCallback         set_tool_info_func,
                             gpointer          set_tool_info_data,
                             GtkSelectionData *selection)
{
  PicmanToolInfo *tool_info = picman_selection_data_get_tool_info (selection,
                                                               the_dnd_picman);

  if (! tool_info)
    return FALSE;

  (* (PicmanDndDropViewableFunc) set_tool_info_func) (widget, x, y,
                                                    PICMAN_VIEWABLE (tool_info),
                                                    set_tool_info_data);

  return TRUE;
}
