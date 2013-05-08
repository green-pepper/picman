/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanviewable.c
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman-utils.h"
#include "picmancontext.h"
#include "picmanmarshal.h"
#include "picmantempbuf.h"
#include "picmanviewable.h"

#include "themes/Default/images/picman-core-pixbufs.h"


enum
{
  PROP_0,
  PROP_STOCK_ID,
  PROP_ICON_PIXBUF,
  PROP_FROZEN
};

enum
{
  INVALIDATE_PREVIEW,
  SIZE_CHANGED,
  LAST_SIGNAL
};


typedef struct _PicmanViewablePrivate PicmanViewablePrivate;

struct _PicmanViewablePrivate
{
  gchar        *stock_id;
  GdkPixbuf    *icon_pixbuf;
  gint          freeze_count;
  PicmanViewable *parent;

  PicmanTempBuf  *preview_temp_buf;
  GdkPixbuf    *preview_pixbuf;
};

#define GET_PRIVATE(viewable) G_TYPE_INSTANCE_GET_PRIVATE (viewable, \
                                                           PICMAN_TYPE_VIEWABLE, \
                                                           PicmanViewablePrivate)


static void    picman_viewable_config_iface_init (PicmanConfigInterface *iface);

static void    picman_viewable_finalize               (GObject        *object);
static void    picman_viewable_set_property           (GObject        *object,
                                                     guint           property_id,
                                                     const GValue   *value,
                                                     GParamSpec     *pspec);
static void    picman_viewable_get_property           (GObject        *object,
                                                     guint           property_id,
                                                     GValue         *value,
                                                     GParamSpec     *pspec);

static gint64  picman_viewable_get_memsize             (PicmanObject    *object,
                                                      gint64        *gui_size);

static void    picman_viewable_real_invalidate_preview (PicmanViewable  *viewable);

static GdkPixbuf * picman_viewable_real_get_new_pixbuf (PicmanViewable  *viewable,
                                                      PicmanContext   *context,
                                                      gint           width,
                                                      gint           height);
static void    picman_viewable_real_get_preview_size   (PicmanViewable  *viewable,
                                                      gint           size,
                                                      gboolean       popup,
                                                      gboolean       dot_for_dot,
                                                      gint          *width,
                                                      gint          *height);
static gboolean picman_viewable_real_get_popup_size    (PicmanViewable  *viewable,
                                                      gint           width,
                                                      gint           height,
                                                      gboolean       dot_for_dot,
                                                      gint          *popup_width,
                                                      gint          *popup_height);
static gchar * picman_viewable_real_get_description    (PicmanViewable  *viewable,
                                                      gchar        **tooltip);
static PicmanContainer * picman_viewable_real_get_children (PicmanViewable *viewable);

static gboolean picman_viewable_serialize_property     (PicmanConfig    *config,
                                                      guint          property_id,
                                                      const GValue  *value,
                                                      GParamSpec    *pspec,
                                                      PicmanConfigWriter *writer);
static gboolean picman_viewable_deserialize_property   (PicmanConfig       *config,
                                                      guint             property_id,
                                                      GValue           *value,
                                                      GParamSpec       *pspec,
                                                      GScanner         *scanner,
                                                      GTokenType       *expected);


G_DEFINE_TYPE_WITH_CODE (PicmanViewable, picman_viewable, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_viewable_config_iface_init))

#define parent_class picman_viewable_parent_class

static guint viewable_signals[LAST_SIGNAL] = { 0 };


static void
picman_viewable_class_init (PicmanViewableClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  viewable_signals[INVALIDATE_PREVIEW] =
    g_signal_new ("invalidate-preview",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanViewableClass, invalidate_preview),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  viewable_signals[SIZE_CHANGED] =
    g_signal_new ("size-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanViewableClass, size_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->finalize         = picman_viewable_finalize;
  object_class->get_property     = picman_viewable_get_property;
  object_class->set_property     = picman_viewable_set_property;

  picman_object_class->get_memsize = picman_viewable_get_memsize;

  klass->default_stock_id        = "picman-question";
  klass->name_changed_signal     = "name-changed";

  klass->invalidate_preview      = picman_viewable_real_invalidate_preview;
  klass->size_changed            = NULL;

  klass->get_size                = NULL;
  klass->get_preview_size        = picman_viewable_real_get_preview_size;
  klass->get_popup_size          = picman_viewable_real_get_popup_size;
  klass->get_preview             = NULL;
  klass->get_new_preview         = NULL;
  klass->get_pixbuf              = NULL;
  klass->get_new_pixbuf          = picman_viewable_real_get_new_pixbuf;
  klass->get_description         = picman_viewable_real_get_description;
  klass->get_children            = picman_viewable_real_get_children;
  klass->set_expanded            = NULL;
  klass->get_expanded            = NULL;

  PICMAN_CONFIG_INSTALL_PROP_STRING (object_class, PROP_STOCK_ID, "stock-id",
                                   NULL, NULL,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_ICON_PIXBUF,
                                   "icon-pixbuf", NULL,
                                   GDK_TYPE_PIXBUF,
                                   G_PARAM_CONSTRUCT |
                                   PICMAN_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, PROP_FROZEN,
                                   g_param_spec_boolean ("frozen",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READABLE));

  g_type_class_add_private (klass, sizeof (PicmanViewablePrivate));
}

static void
picman_viewable_init (PicmanViewable *viewable)
{
}

static void
picman_viewable_config_iface_init (PicmanConfigInterface *iface)
{
  iface->deserialize_property = picman_viewable_deserialize_property;
  iface->serialize_property   = picman_viewable_serialize_property;
}

static void
picman_viewable_finalize (GObject *object)
{
  PicmanViewablePrivate *private = GET_PRIVATE (object);

  if (private->stock_id)
    {
      g_free (private->stock_id);
      private->stock_id = NULL;
    }

  if (private->icon_pixbuf)
    {
      g_object_unref (private->icon_pixbuf);
      private->icon_pixbuf = NULL;
    }

  if (private->preview_temp_buf)
    {
      picman_temp_buf_unref (private->preview_temp_buf);
      private->preview_temp_buf = NULL;
    }

  if (private->preview_pixbuf)
    {
      g_object_unref (private->preview_pixbuf);
      private->preview_pixbuf = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_viewable_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  PicmanViewable        *viewable = PICMAN_VIEWABLE (object);
  PicmanViewablePrivate *private  = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_STOCK_ID:
      picman_viewable_set_stock_id (viewable, g_value_get_string (value));
      break;
    case PROP_ICON_PIXBUF:
      if (private->icon_pixbuf)
        g_object_unref (private->icon_pixbuf);
      private->icon_pixbuf = g_value_dup_object (value);
      picman_viewable_invalidate_preview (viewable);
      break;
    case PROP_FROZEN:
      /* read-only, fall through */

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_viewable_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  PicmanViewable        *viewable = PICMAN_VIEWABLE (object);
  PicmanViewablePrivate *private  = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_STOCK_ID:
      g_value_set_string (value, picman_viewable_get_stock_id (viewable));
      break;
    case PROP_ICON_PIXBUF:
      g_value_set_object (value, private->icon_pixbuf);
      break;
    case PROP_FROZEN:
      g_value_set_boolean (value, picman_viewable_preview_is_frozen (viewable));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_viewable_get_memsize (PicmanObject *object,
                           gint64     *gui_size)
{
  PicmanViewablePrivate *private = GET_PRIVATE (object);

  *gui_size += picman_temp_buf_get_memsize (private->preview_temp_buf);

  if (private->preview_pixbuf)
    {
      *gui_size +=
        (picman_g_object_get_memsize (G_OBJECT (private->preview_pixbuf)) +
         (gsize) gdk_pixbuf_get_height (private->preview_pixbuf) *
         gdk_pixbuf_get_rowstride (private->preview_pixbuf));
    }

  return PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object, gui_size);
}

static void
picman_viewable_real_invalidate_preview (PicmanViewable *viewable)
{
  PicmanViewablePrivate *private = GET_PRIVATE (viewable);

  if (private->preview_temp_buf)
    {
      picman_temp_buf_unref (private->preview_temp_buf);
      private->preview_temp_buf = NULL;
    }

  if (private->preview_pixbuf)
    {
      g_object_unref (private->preview_pixbuf);
      private->preview_pixbuf = NULL;
    }
}

static void
picman_viewable_real_get_preview_size (PicmanViewable *viewable,
                                     gint          size,
                                     gboolean      popup,
                                     gboolean      dot_for_dot,
                                     gint         *width,
                                     gint         *height)
{
  *width  = size;
  *height = size;
}

static gboolean
picman_viewable_real_get_popup_size (PicmanViewable *viewable,
                                   gint          width,
                                   gint          height,
                                   gboolean      dot_for_dot,
                                   gint         *popup_width,
                                   gint         *popup_height)
{
  gint w, h;

  if (picman_viewable_get_size (viewable, &w, &h))
    {
      if (w > width || h > height)
        {
          *popup_width  = w;
          *popup_height = h;

          return TRUE;
        }
    }

  return FALSE;
}

static GdkPixbuf *
picman_viewable_real_get_new_pixbuf (PicmanViewable *viewable,
                                   PicmanContext  *context,
                                   gint          width,
                                   gint          height)
{
  PicmanViewablePrivate *private = GET_PRIVATE (viewable);
  GdkPixbuf           *pixbuf  = NULL;
  PicmanTempBuf         *temp_buf;

  temp_buf = picman_viewable_get_preview (viewable, context, width, height);

  if (temp_buf)
    {
      GeglBuffer *src_buffer;
      GeglBuffer *dest_buffer;

      pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
                               babl_format_has_alpha (picman_temp_buf_get_format (temp_buf)),
                               8,
                               picman_temp_buf_get_width  (temp_buf),
                               picman_temp_buf_get_height (temp_buf));

      src_buffer  = picman_temp_buf_create_buffer (temp_buf);
      dest_buffer = picman_pixbuf_create_buffer (pixbuf);

      gegl_buffer_copy (src_buffer, NULL, dest_buffer, NULL);

      g_object_unref (src_buffer);
      g_object_unref (dest_buffer);
    }
  else if (private->icon_pixbuf)
    {
      pixbuf = gdk_pixbuf_scale_simple (private->icon_pixbuf,
                                        width,
                                        height,
                                        GDK_INTERP_BILINEAR);
    }

  return pixbuf;
}

static gchar *
picman_viewable_real_get_description (PicmanViewable  *viewable,
                                    gchar        **tooltip)
{
  return g_strdup (picman_object_get_name (viewable));
}

static PicmanContainer *
picman_viewable_real_get_children (PicmanViewable *viewable)
{
  return NULL;
}

static gboolean
picman_viewable_serialize_property (PicmanConfig       *config,
                                  guint             property_id,
                                  const GValue     *value,
                                  GParamSpec       *pspec,
                                  PicmanConfigWriter *writer)
{
  PicmanViewablePrivate *private = GET_PRIVATE (config);

  switch (property_id)
    {
    case PROP_STOCK_ID:
      if (private->stock_id)
        {
          picman_config_writer_open (writer, pspec->name);
          picman_config_writer_string (writer, private->stock_id);
          picman_config_writer_close (writer);
        }
      return TRUE;

    case PROP_ICON_PIXBUF:
      {
        GdkPixbuf *icon_pixbuf    = NULL;
        gchar     *pixbuffer      = NULL;
        gchar     *pixbuffer_enc  = NULL;
        gsize      pixbuffer_size = 0;
        GError    *error          = NULL;

        icon_pixbuf = g_value_get_object (value);
        if (icon_pixbuf)
          {
            if (gdk_pixbuf_save_to_buffer (icon_pixbuf,
                                           &pixbuffer,
                                           &pixbuffer_size,
                                           "png", &error, NULL))
              {
                pixbuffer_enc = g_base64_encode ((guchar *)pixbuffer,
                                                 pixbuffer_size);
                picman_config_writer_open (writer, "icon-pixbuf");
                picman_config_writer_string (writer, pixbuffer_enc);
                picman_config_writer_close (writer);

                g_free (pixbuffer_enc);
                g_free (pixbuffer);
              }
          }
      }
      return TRUE;

    default:
      break;
    }

  return FALSE;
}

static gboolean
picman_viewable_deserialize_property (PicmanConfig *config,
                                    guint       property_id,
                                    GValue     *value,
                                    GParamSpec *pspec,
                                    GScanner   *scanner,
                                    GTokenType *expected)
{
  switch (property_id)
    {
    case PROP_ICON_PIXBUF:
      {
        gchar     *encoded_image = NULL;
        GdkPixbuf *icon_pixbuf   = NULL;

        if (! picman_scanner_parse_string (scanner, &encoded_image))
          {
            *expected = G_TOKEN_STRING;
            break;
          }

        if (encoded_image && strlen (encoded_image) > 0)
          {
            gsize   out_len       = 0;
            guchar *decoded_image = g_base64_decode (encoded_image, &out_len);

            if (decoded_image)
              {
                GInputStream *decoded_image_stream = NULL;
                GdkPixbuf    *pixbuf               = NULL;

                decoded_image_stream =
                  g_memory_input_stream_new_from_data (decoded_image,
                                                       out_len, NULL);
                pixbuf = gdk_pixbuf_new_from_stream (decoded_image_stream,
                                                     NULL,
                                                     NULL);
                g_object_unref (decoded_image_stream);

                if (pixbuf)
                  {
                    if (icon_pixbuf)
                      g_object_unref (icon_pixbuf);
                    icon_pixbuf = pixbuf;
                  }

                g_free (decoded_image);
              }
          }

        g_value_take_object (value, icon_pixbuf);
      }
      break;

    default:
      return FALSE;
    }

  return TRUE;
}

/**
 * picman_viewable_invalidate_preview:
 * @viewable: a viewable object
 *
 * Causes any cached preview to be marked as invalid, so that a new
 * preview will be generated at the next attempt to display one.
 **/
void
picman_viewable_invalidate_preview (PicmanViewable *viewable)
{
  PicmanViewablePrivate *private;

  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  private = GET_PRIVATE (viewable);

  if (private->freeze_count == 0)
    g_signal_emit (viewable, viewable_signals[INVALIDATE_PREVIEW], 0);
}

/**
 * picman_viewable_size_changed:
 * @viewable: a viewable object
 *
 * This function sends a signal that is handled at a lower level in the
 * object hierarchy, and provides a mechanism by which objects derived
 * from #PicmanViewable can respond to size changes.
 **/
void
picman_viewable_size_changed (PicmanViewable *viewable)
{
  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  g_signal_emit (viewable, viewable_signals[SIZE_CHANGED], 0);
}

/**
 * picman_viewable_calc_preview_size:
 * @aspect_width:   unscaled width of the preview for an item.
 * @aspect_height:  unscaled height of the preview for an item.
 * @width:          maximum available width for scaled preview.
 * @height:         maximum available height for scaled preview.
 * @dot_for_dot:    if #TRUE, ignore any differences in axis resolution.
 * @xresolution:    resolution in the horizontal direction.
 * @yresolution:    resolution in the vertical direction.
 * @return_width:   place to return the calculated preview width.
 * @return_height:  place to return the calculated preview height.
 * @scaling_up:     returns #TRUE here if the calculated preview size
 *                  is larger than the viewable itself.
 *
 * A utility function, for calculating the dimensions of a preview
 * based on the information specified in the arguments.  The arguments
 * @aspect_width and @aspect_height are the dimensions of the unscaled
 * preview.  The arguments @width and @height represent the maximum
 * width and height that the scaled preview must fit into. The
 * preview is scaled to be as large as possible without exceeding
 * these constraints.
 *
 * If @dot_for_dot is #TRUE, and @xresolution and @yresolution are
 * different, then these results are corrected for the difference in
 * resolution on the two axes, so that the requested aspect ratio
 * applies to the appearance of the display rather than to pixel
 * counts.
 **/
void
picman_viewable_calc_preview_size (gint       aspect_width,
                                 gint       aspect_height,
                                 gint       width,
                                 gint       height,
                                 gboolean   dot_for_dot,
                                 gdouble    xresolution,
                                 gdouble    yresolution,
                                 gint      *return_width,
                                 gint      *return_height,
                                 gboolean  *scaling_up)
{
  gdouble xratio;
  gdouble yratio;

  if (aspect_width > aspect_height)
    {
      xratio = yratio = (gdouble) width / (gdouble) aspect_width;
    }
  else
    {
      xratio = yratio = (gdouble) height / (gdouble) aspect_height;
    }

  if (! dot_for_dot && xresolution != yresolution)
    {
      yratio *= xresolution / yresolution;
    }

  width  = RINT (xratio * (gdouble) aspect_width);
  height = RINT (yratio * (gdouble) aspect_height);

  if (width  < 1) width  = 1;
  if (height < 1) height = 1;

  if (return_width)  *return_width  = width;
  if (return_height) *return_height = height;
  if (scaling_up)    *scaling_up    = (xratio > 1.0) || (yratio > 1.0);
}

gboolean
picman_viewable_get_size (PicmanViewable  *viewable,
                        gint          *width,
                        gint          *height)
{
  PicmanViewableClass *viewable_class;
  gboolean           retval = FALSE;
  gint               w      = 0;
  gint               h      = 0;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), FALSE);

  viewable_class = PICMAN_VIEWABLE_GET_CLASS (viewable);

  if (viewable_class->get_size)
    retval = viewable_class->get_size (viewable, &w, &h);

  if (width)  *width  = w;
  if (height) *height = h;

  return retval;
}

/**
 * picman_viewable_get_preview_size:
 * @viewable:    the object for which to calculate the preview size.
 * @size:        requested size for preview.
 * @popup:       %TRUE if the preview is intended for a popup window.
 * @dot_for_dot: If #TRUE, ignore any differences in X and Y resolution.
 * @width:       return location for the the calculated width.
 * @height:      return location for the calculated height.
 *
 * Retrieve the size of a viewable's preview.  By default, this
 * simply returns the value of the @size argument for both the @width
 * and @height, but this can be overridden in objects derived from
 * #PicmanViewable.  If either the width or height exceeds
 * #PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, they are silently truncated.
 **/
void
picman_viewable_get_preview_size (PicmanViewable *viewable,
                                gint          size,
                                gboolean      popup,
                                gboolean      dot_for_dot,
                                gint         *width,
                                gint         *height)
{
  gint w, h;

  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));
  g_return_if_fail (size > 0);

  PICMAN_VIEWABLE_GET_CLASS (viewable)->get_preview_size (viewable, size,
                                                        popup, dot_for_dot,
                                                        &w, &h);

  w = MIN (w, PICMAN_VIEWABLE_MAX_PREVIEW_SIZE);
  h = MIN (h, PICMAN_VIEWABLE_MAX_PREVIEW_SIZE);

  if (width)  *width  = w;
  if (height) *height = h;

}

/**
 * picman_viewable_get_popup_size:
 * @viewable:     the object for which to calculate the popup size.
 * @width:        the width of the preview from which the popup will be shown.
 * @height:       the height of the preview from which the popup will be shown.
 * @dot_for_dot:  If #TRUE, ignore any differences in X and Y resolution.
 * @popup_width:  return location for the calculated popup width.
 * @popup_height: return location for the calculated popup height.
 *
 * Calculate the size of a viewable's preview, for use in making a
 * popup. The arguments @width and @height specify the size of the
 * preview from which the popup will be shown.
 *
 * Returns: Whether the viewable wants a popup to be shown. Usually
 *          %TRUE if the passed preview size is smaller than the viewable
 *          size, and %FALSE if the viewable completely fits into the
 *          original preview.
 **/
gboolean
picman_viewable_get_popup_size (PicmanViewable *viewable,
                              gint          width,
                              gint          height,
                              gboolean      dot_for_dot,
                              gint         *popup_width,
                              gint         *popup_height)
{
  gint w, h;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), FALSE);

  if (PICMAN_VIEWABLE_GET_CLASS (viewable)->get_popup_size (viewable,
                                                          width, height,
                                                          dot_for_dot,
                                                          &w, &h))
    {
      if (w < 1) w = 1;
      if (h < 1) h = 1;

      /*  limit the popup to 2 * PICMAN_VIEWABLE_MAX_POPUP_SIZE
       *  on each axis.
       */
      if ((w > (2 * PICMAN_VIEWABLE_MAX_POPUP_SIZE)) ||
          (h > (2 * PICMAN_VIEWABLE_MAX_POPUP_SIZE)))
        {
          picman_viewable_calc_preview_size (w, h,
                                           2 * PICMAN_VIEWABLE_MAX_POPUP_SIZE,
                                           2 * PICMAN_VIEWABLE_MAX_POPUP_SIZE,
                                           dot_for_dot, 1.0, 1.0,
                                           &w, &h, NULL);
        }

      /*  limit the number of pixels to
       *  PICMAN_VIEWABLE_MAX_POPUP_SIZE ^ 2
       */
      if ((w * h) > SQR (PICMAN_VIEWABLE_MAX_POPUP_SIZE))
        {
          gdouble factor;

          factor = sqrt (((gdouble) (w * h) /
                          (gdouble) SQR (PICMAN_VIEWABLE_MAX_POPUP_SIZE)));

          w = RINT ((gdouble) w / factor);
          h = RINT ((gdouble) h / factor);
        }

      if (w < 1) w = 1;
      if (h < 1) h = 1;

      if (popup_width)  *popup_width  = w;
      if (popup_height) *popup_height = h;

      return TRUE;
    }

  return FALSE;
}

/**
 * picman_viewable_get_preview:
 * @viewable: The viewable object to get a preview for.
 * @context:  The context to render the preview for.
 * @width:    desired width for the preview
 * @height:   desired height for the preview
 *
 * Gets a preview for a viewable object, by running through a variety
 * of methods until it finds one that works.  First, if an
 * implementation exists of a "get_preview" method, it is tried, and
 * the result is returned if it is not #NULL.  Second, the function
 * checks to see whether there is a cached preview with the correct
 * dimensions; if so, it is returned.  If neither of these works, then
 * the function looks for an implementation of the "get_new_preview"
 * method, and executes it, caching the result.  If everything fails,
 * #NULL is returned.
 *
 * Returns: A #PicmanTempBuf containg the preview image, or #NULL if
 *          none can be found or created.
 **/
PicmanTempBuf *
picman_viewable_get_preview (PicmanViewable *viewable,
                           PicmanContext  *context,
                           gint          width,
                           gint          height)
{
  PicmanViewablePrivate *private;
  PicmanViewableClass   *viewable_class;
  PicmanTempBuf         *temp_buf = NULL;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (width  > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);

  private = GET_PRIVATE (viewable);

  if (G_UNLIKELY (context == NULL))
    g_warning ("%s: context is NULL", G_STRFUNC);

  viewable_class = PICMAN_VIEWABLE_GET_CLASS (viewable);

  if (viewable_class->get_preview)
    temp_buf = viewable_class->get_preview (viewable, context, width, height);

  if (temp_buf)
    return temp_buf;

  if (private->preview_temp_buf)
    {
      if (picman_temp_buf_get_width  (private->preview_temp_buf) == width &&
          picman_temp_buf_get_height (private->preview_temp_buf) == height)
        {
          return private->preview_temp_buf;
        }

      picman_temp_buf_unref (private->preview_temp_buf);
      private->preview_temp_buf = NULL;
    }

  if (viewable_class->get_new_preview)
    temp_buf = viewable_class->get_new_preview (viewable, context,
                                                width, height);

  private->preview_temp_buf = temp_buf;

  return temp_buf;
}

/**
 * picman_viewable_get_new_preview:
 * @viewable: The viewable object to get a preview for.
 * @width:    desired width for the preview
 * @height:   desired height for the preview
 *
 * Gets a new preview for a viewable object.  Similar to
 * picman_viewable_get_preview(), except that it tries things in a
 * different order, first looking for a "get_new_preview" method, and
 * then if that fails for a "get_preview" method.  This function does
 * not look for a cached preview.
 *
 * Returns: A #PicmanTempBuf containg the preview image, or #NULL if
 *          none can be found or created.
 **/
PicmanTempBuf *
picman_viewable_get_new_preview (PicmanViewable *viewable,
                               PicmanContext  *context,
                               gint          width,
                               gint          height)
{
  PicmanViewableClass *viewable_class;
  PicmanTempBuf       *temp_buf = NULL;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (width  > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);

  if (G_UNLIKELY (context == NULL))
    g_warning ("%s: context is NULL", G_STRFUNC);

  viewable_class = PICMAN_VIEWABLE_GET_CLASS (viewable);

  if (viewable_class->get_new_preview)
    temp_buf = viewable_class->get_new_preview (viewable, context,
                                                width, height);

  if (temp_buf)
    return temp_buf;

  if (viewable_class->get_preview)
    temp_buf = viewable_class->get_preview (viewable, context,
                                            width, height);

  if (temp_buf)
    return picman_temp_buf_copy (temp_buf);

  return NULL;
}

/**
 * picman_viewable_get_dummy_preview:
 * @viewable: viewable object for which to get a dummy preview.
 * @width:    width of the preview.
 * @height:   height of the preview.
 * @bpp:      bytes per pixel for the preview, must be 3 or 4.
 *
 * Creates a dummy preview the fits into the specified dimensions,
 * containing a default "question" symbol.  This function is used to
 * generate a preview in situations where layer previews have been
 * disabled in the current Picman configuration.
 *
 * Returns: a #PicmanTempBuf containing the preview image.
 **/
PicmanTempBuf *
picman_viewable_get_dummy_preview (PicmanViewable *viewable,
                                 gint          width,
                                 gint          height,
                                 const Babl   *format)
{
  GdkPixbuf   *pixbuf;
  PicmanTempBuf *buf;
  GeglBuffer  *src_buffer;
  GeglBuffer  *dest_buffer;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);
  g_return_val_if_fail (width  > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);
  g_return_val_if_fail (format != NULL, NULL);

  pixbuf = picman_viewable_get_dummy_pixbuf (viewable, width, height,
                                           babl_format_has_alpha (format));

  buf = picman_temp_buf_new (width, height, format);

  src_buffer  = picman_pixbuf_create_buffer (pixbuf);
  dest_buffer = picman_temp_buf_create_buffer (buf);

  gegl_buffer_copy (src_buffer, NULL, dest_buffer, NULL);

  g_object_unref (src_buffer);
  g_object_unref (dest_buffer);

  g_object_unref (pixbuf);

  return buf;
}

/**
 * picman_viewable_get_pixbuf:
 * @viewable: The viewable object to get a pixbuf preview for.
 * @context:  The context to render the preview for.
 * @width:    desired width for the preview
 * @height:   desired height for the preview
 *
 * Gets a preview for a viewable object, by running through a variety
 * of methods until it finds one that works.  First, if an
 * implementation exists of a "get_pixbuf" method, it is tried, and
 * the result is returned if it is not #NULL.  Second, the function
 * checks to see whether there is a cached preview with the correct
 * dimensions; if so, it is returned.  If neither of these works, then
 * the function looks for an implementation of the "get_new_pixbuf"
 * method, and executes it, caching the result.  If everything fails,
 * #NULL is returned.
 *
 * Returns: A #GdkPixbuf containing the preview pixbuf, or #NULL if none can
 *          be found or created.
 **/
GdkPixbuf *
picman_viewable_get_pixbuf (PicmanViewable *viewable,
                          PicmanContext  *context,
                          gint          width,
                          gint          height)
{
  PicmanViewablePrivate *private;
  PicmanViewableClass   *viewable_class;
  GdkPixbuf           *pixbuf = NULL;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (width  > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);

  private = GET_PRIVATE (viewable);

  if (G_UNLIKELY (context == NULL))
    g_warning ("%s: context is NULL", G_STRFUNC);

  viewable_class = PICMAN_VIEWABLE_GET_CLASS (viewable);

  if (viewable_class->get_pixbuf)
    pixbuf = viewable_class->get_pixbuf (viewable, context, width, height);

  if (pixbuf)
    return pixbuf;

  if (private->preview_pixbuf)
    {
      if (gdk_pixbuf_get_width  (private->preview_pixbuf) == width &&
          gdk_pixbuf_get_height (private->preview_pixbuf) == height)
        {
          return private->preview_pixbuf;
        }

      g_object_unref (private->preview_pixbuf);
      private->preview_pixbuf = NULL;
    }

  if (viewable_class->get_new_pixbuf)
    pixbuf = viewable_class->get_new_pixbuf (viewable, context, width, height);

  private->preview_pixbuf = pixbuf;

  return pixbuf;
}

/**
 * picman_viewable_get_new_pixbuf:
 * @viewable: The viewable object to get a new pixbuf preview for.
 * @context:  The context to render the preview for.
 * @width:    desired width for the pixbuf
 * @height:   desired height for the pixbuf
 *
 * Gets a new preview for a viewable object.  Similar to
 * picman_viewable_get_pixbuf(), except that it tries things in a
 * different order, first looking for a "get_new_pixbuf" method, and
 * then if that fails for a "get_pixbuf" method.  This function does
 * not look for a cached pixbuf.
 *
 * Returns: A #GdkPixbuf containing the preview, or #NULL if none can
 *          be created.
 **/
GdkPixbuf *
picman_viewable_get_new_pixbuf (PicmanViewable *viewable,
                              PicmanContext  *context,
                              gint          width,
                              gint          height)
{
  PicmanViewableClass *viewable_class;
  GdkPixbuf         *pixbuf = NULL;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (width  > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);

  if (G_UNLIKELY (context == NULL))
    g_warning ("%s: context is NULL", G_STRFUNC);

  viewable_class = PICMAN_VIEWABLE_GET_CLASS (viewable);

  if (viewable_class->get_new_pixbuf)
    pixbuf = viewable_class->get_new_pixbuf (viewable, context, width, height);

  if (pixbuf)
    return pixbuf;

  if (viewable_class->get_pixbuf)
    pixbuf = viewable_class->get_pixbuf (viewable, context, width, height);

  if (pixbuf)
    return gdk_pixbuf_copy (pixbuf);

  return NULL;
}

/**
 * picman_viewable_get_dummy_pixbuf:
 * @viewable: the viewable object for which to create a dummy representation.
 * @width:    maximum permitted width for the pixbuf.
 * @height:   maximum permitted height for the pixbuf.
 * @bpp:      bytes per pixel for the pixbuf, must equal 3 or 4.
 *
 * Creates a pixbuf containing a default "question" symbol, sized to
 * fit into the specified dimensions.  The depth of the pixbuf must be
 * 3 or 4 because #GdkPixbuf does not support grayscale.  This
 * function is used to generate a preview in situations where
 * previewing has been disabled in the current Picman configuration.
 * [Note: this function is currently unused except internally to
 * #PicmanViewable -- consider making it static?]
 *
 * Returns: the created #GdkPixbuf.
 **/
GdkPixbuf *
picman_viewable_get_dummy_pixbuf (PicmanViewable  *viewable,
                                gint           width,
                                gint           height,
                                gboolean       with_alpha)
{
  GdkPixbuf *icon;
  GdkPixbuf *pixbuf;
  gdouble    ratio;
  gint       w, h;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);
  g_return_val_if_fail (width  > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);

  icon = gdk_pixbuf_new_from_inline (-1, stock_question_64, FALSE, NULL);

  g_return_val_if_fail (icon != NULL, NULL);

  w = gdk_pixbuf_get_width (icon);
  h = gdk_pixbuf_get_height (icon);

  ratio = (gdouble) MIN (width, height) / (gdouble) MAX (w, h);
  ratio = MIN (ratio, 1.0);

  w = RINT (ratio * (gdouble) w);
  h = RINT (ratio * (gdouble) h);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, with_alpha, 8, width, height);
  gdk_pixbuf_fill (pixbuf, 0xffffffff);

  if (w && h)
    gdk_pixbuf_composite (icon, pixbuf,
                          (width - w) / 2, (height - h) / 2, w, h,
                          (width - w) / 2, (height - h) / 2, ratio, ratio,
                          GDK_INTERP_BILINEAR, 0xFF);

  g_object_unref (icon);

  return pixbuf;
}

/**
 * picman_viewable_get_description:
 * @viewable: viewable object for which to retrieve a description.
 * @tooltip:  return loaction for an optional tooltip string.
 *
 * Retrieves a string containing a description of the viewable object,
 * By default, it simply returns the name of the object, but this can
 * be overridden by object types that inherit from #PicmanViewable.
 *
 * Returns: a copy of the description string.  This should be freed
 *          when it is no longer needed.
 **/
gchar *
picman_viewable_get_description (PicmanViewable  *viewable,
                               gchar        **tooltip)
{
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);

  if (tooltip)
    *tooltip = NULL;

  return PICMAN_VIEWABLE_GET_CLASS (viewable)->get_description (viewable,
                                                              tooltip);
}

/**
 * picman_viewable_get_stock_id:
 * @viewable: viewable object for which to retrieve a stock ID.
 *
 * Gets the current value of the object's stock ID, for use in
 * constructing an iconic representation of the object.
 *
 * Returns: a pointer to the string containing the stock ID.  The
 *          contents must not be altered or freed.
 **/
const gchar *
picman_viewable_get_stock_id (PicmanViewable *viewable)
{
  PicmanViewablePrivate *private;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);

  private = GET_PRIVATE (viewable);

  if (private->stock_id)
    return (const gchar *) private->stock_id;

  return PICMAN_VIEWABLE_GET_CLASS (viewable)->default_stock_id;
}

/**
 * picman_viewable_set_stock_id:
 * @viewable: viewable object to assign the specified stock ID.
 * @stock_id: string containing a stock identifier.
 *
 * Seta the object's stock ID, for use in constructing iconic smbols
 * of the object.  The contents of @stock_id are copied, so you can
 * free it when you are done with it.
 **/
void
picman_viewable_set_stock_id (PicmanViewable *viewable,
                            const gchar  *stock_id)
{
  PicmanViewablePrivate *private;
  PicmanViewableClass   *viewable_class;

  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  private = GET_PRIVATE (viewable);

  g_free (private->stock_id);
  private->stock_id = NULL;

  viewable_class = PICMAN_VIEWABLE_GET_CLASS (viewable);

  if (stock_id)
    {
      if (viewable_class->default_stock_id == NULL ||
          strcmp (stock_id, viewable_class->default_stock_id))
        private->stock_id = g_strdup (stock_id);
    }

  picman_viewable_invalidate_preview (viewable);

  g_object_notify (G_OBJECT (viewable), "stock-id");
}

void
picman_viewable_preview_freeze (PicmanViewable *viewable)
{
  PicmanViewablePrivate *private;

  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  private = GET_PRIVATE (viewable);

  private->freeze_count++;

  if (private->freeze_count == 1)
    g_object_notify (G_OBJECT (viewable), "frozen");
}

void
picman_viewable_preview_thaw (PicmanViewable *viewable)
{
  PicmanViewablePrivate *private;

  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  private = GET_PRIVATE (viewable);

  g_return_if_fail (private->freeze_count > 0);

  private->freeze_count--;

  if (private->freeze_count == 0)
    {
      picman_viewable_invalidate_preview (viewable);
      g_object_notify (G_OBJECT (viewable), "frozen");
    }
}

gboolean
picman_viewable_preview_is_frozen (PicmanViewable *viewable)
{
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), FALSE);

  return GET_PRIVATE (viewable)->freeze_count != 0;
}

PicmanViewable *
picman_viewable_get_parent (PicmanViewable *viewable)
{
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);

  return GET_PRIVATE (viewable)->parent;
}

void
picman_viewable_set_parent (PicmanViewable *viewable,
                          PicmanViewable *parent)
{
  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));
  g_return_if_fail (parent == NULL || PICMAN_IS_VIEWABLE (parent));

  GET_PRIVATE (viewable)->parent = parent;
}

PicmanContainer *
picman_viewable_get_children (PicmanViewable *viewable)
{
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);

  return PICMAN_VIEWABLE_GET_CLASS (viewable)->get_children (viewable);
}

gboolean
picman_viewable_get_expanded (PicmanViewable *viewable)
{
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), FALSE);

  if (PICMAN_VIEWABLE_GET_CLASS (viewable)->get_expanded)
    return PICMAN_VIEWABLE_GET_CLASS (viewable)->get_expanded (viewable);

  return FALSE;
}

void
picman_viewable_set_expanded (PicmanViewable *viewable,
                            gboolean       expanded)
{
  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  if (PICMAN_VIEWABLE_GET_CLASS (viewable)->set_expanded)
    PICMAN_VIEWABLE_GET_CLASS (viewable)->set_expanded (viewable, expanded);
}

gboolean
picman_viewable_is_ancestor (PicmanViewable *ancestor,
                           PicmanViewable *descendant)
{
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (ancestor), FALSE);
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (descendant), FALSE);

  while (descendant)
    {
      PicmanViewable *parent = picman_viewable_get_parent (descendant);

      if (parent == ancestor)
        return TRUE;

      descendant = parent;
    }

  return FALSE;
}
