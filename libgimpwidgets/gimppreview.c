/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpreview.c
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"

#include "picmanwidgets.h"

#include "picmanpreview.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanpreview
 * @title: PicmanPreview
 * @short_description: A widget providing a #PicmanPreviewArea plus
 *                     framework to update the preview.
 *
 * A widget providing a #PicmanPreviewArea plus framework to update the
 * preview.
 **/


#define DEFAULT_SIZE     200
#define PREVIEW_TIMEOUT  200


enum
{
  INVALIDATED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_UPDATE
};

typedef struct
{
  GtkWidget *controls;
} PicmanPreviewPrivate;

#define PICMAN_PREVIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), PICMAN_TYPE_PREVIEW, PicmanPreviewPrivate))


static void      picman_preview_class_init          (PicmanPreviewClass *klass);
static void      picman_preview_init                (PicmanPreview      *preview);
static void      picman_preview_dispose             (GObject          *object);
static void      picman_preview_get_property        (GObject          *object,
                                                   guint             property_id,
                                                   GValue           *value,
                                                   GParamSpec       *pspec);
static void      picman_preview_set_property        (GObject          *object,
                                                   guint             property_id,
                                                   const GValue     *value,
                                                   GParamSpec       *pspec);

static void      picman_preview_direction_changed   (GtkWidget        *widget,
                                                   GtkTextDirection  prev_dir);
static gboolean  picman_preview_popup_menu          (GtkWidget        *widget);

static void      picman_preview_area_realize        (GtkWidget        *widget,
                                                   PicmanPreview      *preview);
static void      picman_preview_area_unrealize      (GtkWidget        *widget,
                                                   PicmanPreview      *preview);
static void      picman_preview_area_size_allocate  (GtkWidget        *widget,
                                                   GtkAllocation    *allocation,
                                                   PicmanPreview      *preview);
static void      picman_preview_area_set_cursor     (PicmanPreview      *preview);
static gboolean  picman_preview_area_event          (GtkWidget        *area,
                                                   GdkEvent         *event,
                                                   PicmanPreview      *preview);

static void      picman_preview_toggle_callback     (GtkWidget        *toggle,
                                                   PicmanPreview      *preview);

static void      picman_preview_notify_checks       (PicmanPreview      *preview);

static gboolean  picman_preview_invalidate_now      (PicmanPreview      *preview);
static void      picman_preview_real_set_cursor     (PicmanPreview      *preview);
static void      picman_preview_real_transform      (PicmanPreview      *preview,
                                                   gint              src_x,
                                                   gint              src_y,
                                                   gint             *dest_x,
                                                   gint             *dest_y);
static void      picman_preview_real_untransform    (PicmanPreview      *preview,
                                                   gint              src_x,
                                                   gint              src_y,
                                                   gint             *dest_x,
                                                   gint             *dest_y);


static guint preview_signals[LAST_SIGNAL] = { 0 };

static GtkBoxClass *parent_class = NULL;


GType
picman_preview_get_type (void)
{
  static GType preview_type = 0;

  if (! preview_type)
    {
      const GTypeInfo preview_info =
      {
        sizeof (PicmanPreviewClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) picman_preview_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (PicmanPreview),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) picman_preview_init,
      };

      preview_type = g_type_register_static (GTK_TYPE_BOX,
                                             "PicmanPreview",
                                             &preview_info,
                                             G_TYPE_FLAG_ABSTRACT);
    }

  return preview_type;
}

static void
picman_preview_class_init (PicmanPreviewClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  preview_signals[INVALIDATED] =
    g_signal_new ("invalidated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPreviewClass, invalidated),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->dispose           = picman_preview_dispose;
  object_class->get_property      = picman_preview_get_property;
  object_class->set_property      = picman_preview_set_property;

  widget_class->direction_changed = picman_preview_direction_changed;
  widget_class->popup_menu        = picman_preview_popup_menu;

  klass->draw                     = NULL;
  klass->draw_thumb               = NULL;
  klass->draw_buffer              = NULL;
  klass->set_cursor               = picman_preview_real_set_cursor;
  klass->transform                = picman_preview_real_transform;
  klass->untransform              = picman_preview_real_untransform;

  g_type_class_add_private (object_class, sizeof (PicmanPreviewPrivate));

  g_object_class_install_property (object_class,
                                   PROP_UPDATE,
                                   g_param_spec_boolean ("update",
                                                         NULL, NULL,
                                                         TRUE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("size",
                                                             NULL, NULL,
                                                             1, 1024,
                                                             DEFAULT_SIZE,
                                                             PICMAN_PARAM_READABLE));
}

static void
picman_preview_init (PicmanPreview *preview)
{
  PicmanPreviewPrivate *priv = PICMAN_PREVIEW_GET_PRIVATE (preview);
  GtkWidget          *frame;
  gdouble             xalign = 0.0;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (preview),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_homogeneous (GTK_BOX (preview), FALSE);
  gtk_box_set_spacing (GTK_BOX (preview), 6);

  if (gtk_widget_get_direction (GTK_WIDGET (preview)) == GTK_TEXT_DIR_RTL)
    xalign = 1.0;

  preview->frame = gtk_aspect_frame_new (NULL, xalign, 0.0, 1.0, TRUE);
  gtk_frame_set_shadow_type (GTK_FRAME (preview->frame), GTK_SHADOW_NONE);
  gtk_box_pack_start (GTK_BOX (preview), preview->frame, TRUE, TRUE, 0);
  gtk_widget_show (preview->frame);

  preview->table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_row_spacing (GTK_TABLE (preview->table), 1, 3);
  gtk_container_add (GTK_CONTAINER (preview->frame), preview->table);
  gtk_widget_show (preview->table);

  preview->timeout_id = 0;

  preview->xmin   = preview->ymin = 0;
  preview->xmax   = preview->ymax = 1;
  preview->width  = preview->xmax - preview->xmin;
  preview->height = preview->ymax - preview->ymin;

  preview->xoff   = 0;
  preview->yoff   = 0;

  preview->default_cursor = NULL;

  /*  preview area  */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_table_attach (GTK_TABLE (preview->table), frame, 0, 1, 0, 1,
                    GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
  gtk_widget_show (frame);

  preview->area = picman_preview_area_new ();
  gtk_container_add (GTK_CONTAINER (frame), preview->area);
  gtk_widget_show (preview->area);

  g_signal_connect_swapped (preview->area, "notify::check-size",
                            G_CALLBACK (picman_preview_notify_checks),
                            preview);
  g_signal_connect_swapped (preview->area, "notify::check-type",
                            G_CALLBACK (picman_preview_notify_checks),
                            preview);

  gtk_widget_add_events (preview->area,
                         GDK_BUTTON_PRESS_MASK        |
                         GDK_BUTTON_RELEASE_MASK      |
                         GDK_POINTER_MOTION_HINT_MASK |
                         GDK_BUTTON_MOTION_MASK);

  g_signal_connect (preview->area, "event",
                    G_CALLBACK (picman_preview_area_event),
                    preview);

  g_signal_connect (preview->area, "realize",
                    G_CALLBACK (picman_preview_area_realize),
                    preview);
  g_signal_connect (preview->area, "unrealize",
                    G_CALLBACK (picman_preview_area_unrealize),
                    preview);

  g_signal_connect_data (preview->area, "realize",
                         G_CALLBACK (picman_preview_area_set_cursor),
                         preview, NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);

  g_signal_connect (preview->area, "size-allocate",
                    G_CALLBACK (picman_preview_area_size_allocate),
                    preview);

  g_signal_connect_data (preview->area, "size-allocate",
                         G_CALLBACK (picman_preview_area_set_cursor),
                         preview, NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);

  priv->controls = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_table_attach (GTK_TABLE (preview->table), priv->controls, 0, 2, 2, 3,
                    GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  gtk_widget_show (priv->controls);

  /*  toggle button to (de)activate the instant preview  */
  preview->toggle = gtk_check_button_new_with_mnemonic (_("_Preview"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (preview->toggle),
                                preview->update_preview);
  gtk_box_pack_start (GTK_BOX (priv->controls), preview->toggle, TRUE, TRUE, 0);
  gtk_widget_show (preview->toggle);

  g_signal_connect (preview->toggle, "toggled",
                    G_CALLBACK (picman_preview_toggle_callback),
                    preview);
}

static void
picman_preview_dispose (GObject *object)
{
  PicmanPreview *preview = PICMAN_PREVIEW (object);

  if (preview->timeout_id)
    {
      g_source_remove (preview->timeout_id);
      preview->timeout_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_preview_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  PicmanPreview *preview = PICMAN_PREVIEW (object);

  switch (property_id)
    {
    case PROP_UPDATE:
      g_value_set_boolean (value, preview->update_preview);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_preview_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  PicmanPreview *preview = PICMAN_PREVIEW (object);

  switch (property_id)
    {
    case PROP_UPDATE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (preview->toggle),
                                    g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_preview_direction_changed (GtkWidget        *widget,
                                GtkTextDirection  prev_dir)
{
  PicmanPreview *preview = PICMAN_PREVIEW (widget);
  gdouble      xalign  = 0.0;

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    xalign = 1.0;

  gtk_aspect_frame_set (GTK_ASPECT_FRAME (preview->frame),
                        xalign, 0.0, 1.0, TRUE);
}

static gboolean
picman_preview_popup_menu (GtkWidget *widget)
{
  PicmanPreview *preview = PICMAN_PREVIEW (widget);

  picman_preview_area_menu_popup (PICMAN_PREVIEW_AREA (preview->area), NULL);

  return TRUE;
}

static void
picman_preview_area_realize (GtkWidget   *widget,
                           PicmanPreview *preview)
{
  GdkDisplay *display = gtk_widget_get_display (widget);

  g_return_if_fail (preview->cursor_busy == NULL);

  preview->cursor_busy = gdk_cursor_new_for_display (display, GDK_WATCH);

}

static void
picman_preview_area_unrealize (GtkWidget   *widget,
                             PicmanPreview *preview)
{
  if (preview->cursor_busy)
    {
      gdk_cursor_unref (preview->cursor_busy);
      preview->cursor_busy = NULL;
    }
}

static void
picman_preview_area_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation,
                                 PicmanPreview   *preview)
{
  gint width  = preview->xmax - preview->xmin;
  gint height = preview->ymax - preview->ymin;

  preview->width  = MIN (width,  allocation->width);
  preview->height = MIN (height, allocation->height);

  picman_preview_draw (preview);
  picman_preview_invalidate (preview);
}

static void
picman_preview_area_set_cursor (PicmanPreview *preview)
{
  PICMAN_PREVIEW_GET_CLASS (preview)->set_cursor (preview);
}

static gboolean
picman_preview_area_event (GtkWidget   *area,
                         GdkEvent    *event,
                         PicmanPreview *preview)
{
  GdkEventButton *button_event = (GdkEventButton *) event;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      switch (button_event->button)
        {
        case 3:
          picman_preview_area_menu_popup (PICMAN_PREVIEW_AREA (area), button_event);
          return TRUE;
        }
      break;

    default:
      break;
    }

  return FALSE;
}

static void
picman_preview_toggle_callback (GtkWidget   *toggle,
                              PicmanPreview *preview)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle)))
    {
      preview->update_preview = TRUE;

      g_object_notify (G_OBJECT (preview), "update");

      if (preview->timeout_id)
        g_source_remove (preview->timeout_id);

      picman_preview_invalidate_now (preview);
    }
  else
    {
      preview->update_preview = FALSE;

      g_object_notify (G_OBJECT (preview), "update");

      picman_preview_draw (preview);
    }
}

static void
picman_preview_notify_checks (PicmanPreview *preview)
{
  picman_preview_draw (preview);
  picman_preview_invalidate (preview);
}

static gboolean
picman_preview_invalidate_now (PicmanPreview *preview)
{
  GtkWidget        *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (preview));
  PicmanPreviewClass *class    = PICMAN_PREVIEW_GET_CLASS (preview);

  picman_preview_draw (preview);

  preview->timeout_id = 0;

  if (toplevel && gtk_widget_get_realized (toplevel))
    {
      gdk_window_set_cursor (gtk_widget_get_window (toplevel),
                             preview->cursor_busy);
      gdk_window_set_cursor (gtk_widget_get_window (preview->area),
                             preview->cursor_busy);

      gdk_flush ();

      g_signal_emit (preview, preview_signals[INVALIDATED], 0);

      class->set_cursor (preview);
      gdk_window_set_cursor (gtk_widget_get_window (toplevel), NULL);
    }
  else
    {
      g_signal_emit (preview, preview_signals[INVALIDATED], 0);
    }

  return FALSE;
}

static void
picman_preview_real_set_cursor (PicmanPreview *preview)
{
  if (gtk_widget_get_realized (preview->area))
    gdk_window_set_cursor (gtk_widget_get_window (preview->area),
                           preview->default_cursor);
}

static void
picman_preview_real_transform (PicmanPreview *preview,
                             gint         src_x,
                             gint         src_y,
                             gint        *dest_x,
                             gint        *dest_y)
{
  *dest_x = src_x - preview->xoff - preview->xmin;
  *dest_y = src_y - preview->yoff - preview->ymin;
}

static void
picman_preview_real_untransform (PicmanPreview *preview,
                               gint         src_x,
                               gint         src_y,
                               gint        *dest_x,
                               gint        *dest_y)
{
  *dest_x = src_x + preview->xoff + preview->xmin;
  *dest_y = src_y + preview->yoff + preview->ymin;
}

/**
 * picman_preview_set_update:
 * @preview: a #PicmanPreview widget
 * @update: %TRUE if the preview should invalidate itself when being
 *          scrolled or when picman_preview_invalidate() is being called
 *
 * Sets the state of the "Preview" check button.
 *
 * Since: PICMAN 2.2
 **/
void
picman_preview_set_update (PicmanPreview *preview,
                         gboolean     update)
{
  g_return_if_fail (PICMAN_IS_PREVIEW (preview));

  g_object_set (preview,
                "update", update,
                NULL);
}

/**
 * picman_preview_get_update:
 * @preview: a #PicmanPreview widget
 *
 * Return value: the state of the "Preview" check button.
 *
 * Since: PICMAN 2.2
 **/
gboolean
picman_preview_get_update (PicmanPreview *preview)
{
  g_return_val_if_fail (PICMAN_IS_PREVIEW (preview), FALSE);

  return preview->update_preview;
}

/**
 * picman_preview_set_bounds:
 * @preview: a #PicmanPreview widget
 * @xmin:    the minimum X value
 * @ymin:    the minimum Y value
 * @xmax:    the maximum X value
 * @ymax:    the maximum Y value
 *
 * Sets the lower and upper limits for the previewed area. The
 * difference between the upper and lower value is used to set the
 * maximum size of the #PicmanPreviewArea used in the @preview.
 *
 * Since: PICMAN 2.2
 **/
void
picman_preview_set_bounds (PicmanPreview *preview,
                         gint         xmin,
                         gint         ymin,
                         gint         xmax,
                         gint         ymax)
{
  g_return_if_fail (PICMAN_IS_PREVIEW (preview));
  g_return_if_fail (xmax > xmin);
  g_return_if_fail (ymax > ymin);

  preview->xmin = xmin;
  preview->ymin = ymin;
  preview->xmax = xmax;
  preview->ymax = ymax;

  picman_preview_area_set_max_size (PICMAN_PREVIEW_AREA (preview->area),
                                  xmax - xmin,
                                  ymax - ymin);
}

/**
 * picman_preview_get_size:
 * @preview: a #PicmanPreview widget
 * @width:   return location for the preview area width
 * @height:  return location for the preview area height
 *
 * Since: PICMAN 2.2
 **/
void
picman_preview_get_size (PicmanPreview *preview,
                       gint        *width,
                       gint        *height)
{
  g_return_if_fail (PICMAN_IS_PREVIEW (preview));

  if (width)
    *width = preview->width;

  if (height)
    *height = preview->height;
}

/**
 * picman_preview_get_position:
 * @preview: a #PicmanPreview widget
 * @x:       return location for the horizontal offset
 * @y:       return location for the vertical offset
 *
 * Since: PICMAN 2.2
 **/
void
picman_preview_get_position (PicmanPreview *preview,
                           gint        *x,
                           gint        *y)
{
  g_return_if_fail (PICMAN_IS_PREVIEW (preview));

  if (x)
    *x = preview->xoff + preview->xmin;

  if (y)
    *y = preview->yoff + preview->ymin;
}

/**
 * picman_preview_transform:
 * @preview: a #PicmanPreview widget
 * @src_x:   horizontal position on the previewed image
 * @src_y:   vertical position on the previewed image
 * @dest_x:  returns the transformed horizontal position
 * @dest_y:  returns the transformed vertical position
 *
 * Transforms from image to widget coordinates.
 *
 * Since: PICMAN 2.4
 **/
void
picman_preview_transform (PicmanPreview *preview,
                        gint         src_x,
                        gint         src_y,
                        gint        *dest_x,
                        gint        *dest_y)
{
  g_return_if_fail (PICMAN_IS_PREVIEW (preview));
  g_return_if_fail (dest_x != NULL && dest_y != NULL);

  PICMAN_PREVIEW_GET_CLASS (preview)->transform (preview,
                                               src_x, src_y, dest_x, dest_y);
}

/**
 * picman_preview_untransform:
 * @preview: a #PicmanPreview widget
 * @src_x:   horizontal position relative to the preview area's origin
 * @src_y:   vertical position relative to  preview area's origin
 * @dest_x:  returns the untransformed horizontal position
 * @dest_y:  returns the untransformed vertical position
 *
 * Transforms from widget to image coordinates.
 *
 * Since: PICMAN 2.4
 **/
void
picman_preview_untransform (PicmanPreview *preview,
                          gint         src_x,
                          gint         src_y,
                          gint        *dest_x,
                          gint        *dest_y)
{
  g_return_if_fail (PICMAN_IS_PREVIEW (preview));
  g_return_if_fail (dest_x != NULL && dest_y != NULL);

  PICMAN_PREVIEW_GET_CLASS (preview)->untransform (preview,
                                                 src_x, src_y, dest_x, dest_y);
}

/**
 * picman_preview_get_area:
 * @preview: a #PicmanPreview widget
 *
 * In most cases, you shouldn't need to access the #PicmanPreviewArea
 * that is being used in the @preview. Sometimes however, you need to.
 * For example if you want to receive mouse events from the area. In
 * such cases, use picman_preview_get_area().
 *
 * Return value: a pointer to the #PicmanPreviewArea used in the @preview.
 *
 * Since: PICMAN 2.4
 **/
GtkWidget *
picman_preview_get_area (PicmanPreview  *preview)
{
  g_return_val_if_fail (PICMAN_IS_PREVIEW (preview), NULL);

  return preview->area;
}

/**
 * picman_preview_draw:
 * @preview: a #PicmanPreview widget
 *
 * Calls the PicmanPreview::draw method. PicmanPreview itself doesn't
 * implement a default draw method so the behaviour is determined by
 * the derived class implementing this method.
 *
 * #PicmanDrawablePreview implements picman_preview_draw() by drawing the
 * original, unmodified drawable to the @preview.
 *
 * Since: PICMAN 2.2
 **/
void
picman_preview_draw (PicmanPreview *preview)
{
  PicmanPreviewClass *class = PICMAN_PREVIEW_GET_CLASS (preview);

  if (class->draw)
    class->draw (preview);
}

/**
 * picman_preview_draw_buffer:
 * @preview:   a #PicmanPreview widget
 * @buffer:    a pixel buffer the size of the preview
 * @rowstride: the @buffer's rowstride
 *
 * Calls the PicmanPreview::draw_buffer method. PicmanPreview itself
 * doesn't implement this method so the behaviour is determined by the
 * derived class implementing this method.
 *
 * Since: PICMAN 2.2
 **/
void
picman_preview_draw_buffer (PicmanPreview  *preview,
                          const guchar *buffer,
                          gint          rowstride)
{
  PicmanPreviewClass *class = PICMAN_PREVIEW_GET_CLASS (preview);

  if (class->draw_buffer)
    class->draw_buffer (preview, buffer, rowstride);
}

/**
 * picman_preview_invalidate:
 * @preview: a #PicmanPreview widget
 *
 * This function starts or renews a short low-priority timeout. When
 * the timeout expires, the PicmanPreview::invalidated signal is emitted
 * which will usually cause the @preview to be updated.
 *
 * This function does nothing unless the "Preview" button is checked.
 *
 * During the emission of the signal a busy cursor is set on the
 * toplevel window containing the @preview and on the preview area
 * itself.
 *
 * Since: PICMAN 2.2
 **/
void
picman_preview_invalidate (PicmanPreview *preview)
{
  g_return_if_fail (PICMAN_IS_PREVIEW (preview));

  if (preview->update_preview)
    {
      if (preview->timeout_id)
        g_source_remove (preview->timeout_id);

      preview->timeout_id =
        g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, PREVIEW_TIMEOUT,
                            (GSourceFunc) picman_preview_invalidate_now,
                            preview, NULL);
    }
}

/**
 * picman_preview_set_default_cursor:
 * @preview: a #PicmanPreview widget
 * @cursor:  a #GdkCursor or %NULL
 *
 * Sets the default mouse cursor for the preview.  Note that this will
 * be overriden by a %GDK_FLEUR if the preview has scrollbars, or by a
 * %GDK_WATCH when the preview is invalidated.
 *
 * Since: PICMAN 2.2
 **/
void
picman_preview_set_default_cursor (PicmanPreview *preview,
                                 GdkCursor   *cursor)
{
  g_return_if_fail (PICMAN_IS_PREVIEW (preview));

  if (preview->default_cursor)
    gdk_cursor_unref (preview->default_cursor);

  if (cursor)
    gdk_cursor_ref (cursor);

  preview->default_cursor = cursor;
}

/**
 * picman_preview_get_controls:
 * @preview: a #PicmanPreview widget
 *
 * Gives access to the #GtkHBox at the bottom of the preview that
 * contains the update toggle. Derived widgets can use this function
 * if they need to add controls to this area.
 *
 * Return value: the #GtkHBox at the bottom of the preview.
 *
 * Since: PICMAN 2.4
 **/
GtkWidget *
picman_preview_get_controls (PicmanPreview *preview)
{
  g_return_val_if_fail (PICMAN_IS_PREVIEW (preview), NULL);

  return PICMAN_PREVIEW_GET_PRIVATE (preview)->controls;
}
