/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrenderer.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_VIEW_RENDERER_H__
#define __PICMAN_VIEW_RENDERER_H__


#define PICMAN_VIEW_MAX_BORDER_WIDTH 16


#define PICMAN_TYPE_VIEW_RENDERER            (picman_view_renderer_get_type ())
#define PICMAN_VIEW_RENDERER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VIEW_RENDERER, PicmanViewRenderer))
#define PICMAN_VIEW_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VIEW_RENDERER, PicmanViewRendererClass))
#define PICMAN_IS_VIEW_RENDERER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_VIEW_RENDERER))
#define PICMAN_IS_VIEW_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VIEW_RENDERER))
#define PICMAN_VIEW_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VIEW_RENDERER, PicmanViewRendererClass))


typedef struct _PicmanViewRendererClass  PicmanViewRendererClass;

struct _PicmanViewRenderer
{
  GObject             parent_instance;

  PicmanContext        *context;

  GType               viewable_type;
  PicmanViewable       *viewable;

  gint                width;
  gint                height;
  gint                border_width;
  guint               dot_for_dot : 1;
  guint               is_popup    : 1;

  PicmanViewBorderType  border_type;
  PicmanRGB             border_color;

  /*< protected >*/
  cairo_surface_t    *surface;

  /*< private >*/
  cairo_pattern_t    *pattern;
  GdkPixbuf          *pixbuf;
  gchar              *bg_stock_id;

  gint                size;
  gboolean            needs_render;
  guint               idle_id;
};

struct _PicmanViewRendererClass
{
  GObjectClass   parent_class;

  GdkPixbuf     *frame;
  gint           frame_left;
  gint           frame_right;
  gint           frame_bottom;
  gint           frame_top;

  /*  signals  */
  void (* update)      (PicmanViewRenderer *renderer);

  /*  virtual functions  */
  void (* set_context) (PicmanViewRenderer *renderer,
                        PicmanContext      *context);
  void (* invalidate)  (PicmanViewRenderer *renderer);
  void (* draw)        (PicmanViewRenderer *renderer,
                        GtkWidget        *widget,
                        cairo_t          *cr,
                        gint              available_width,
                        gint              available_height);
  void (* render)      (PicmanViewRenderer *renderer,
                        GtkWidget        *widget);
};


GType              picman_view_renderer_get_type (void) G_GNUC_CONST;

PicmanViewRenderer * picman_view_renderer_new      (PicmanContext *context,
                                                GType        viewable_type,
                                                gint         size,
                                                gint         border_width,
                                                gboolean     is_popup);
PicmanViewRenderer * picman_view_renderer_new_full (PicmanContext *context,
                                                GType        viewable_type,
                                                gint         width,
                                                gint         height,
                                                gint         border_width,
                                                gboolean     is_popup);

void   picman_view_renderer_set_context      (PicmanViewRenderer   *renderer,
                                            PicmanContext        *context);
void   picman_view_renderer_set_viewable     (PicmanViewRenderer   *renderer,
                                            PicmanViewable       *viewable);
void   picman_view_renderer_set_size         (PicmanViewRenderer   *renderer,
                                            gint                size,
                                            gint                border_width);
void   picman_view_renderer_set_size_full    (PicmanViewRenderer   *renderer,
                                            gint                width,
                                            gint                height,
                                            gint                border_width);
void   picman_view_renderer_set_dot_for_dot  (PicmanViewRenderer   *renderer,
                                            gboolean            dot_for_dot);
void   picman_view_renderer_set_border_type  (PicmanViewRenderer   *renderer,
                                            PicmanViewBorderType  border_type);
void   picman_view_renderer_set_border_color (PicmanViewRenderer   *renderer,
                                            const PicmanRGB      *border_color);
void   picman_view_renderer_set_background   (PicmanViewRenderer   *renderer,
                                            const gchar        *stock_id);

void   picman_view_renderer_invalidate       (PicmanViewRenderer   *renderer);
void   picman_view_renderer_update           (PicmanViewRenderer   *renderer);
void   picman_view_renderer_update_idle      (PicmanViewRenderer   *renderer);
void   picman_view_renderer_remove_idle      (PicmanViewRenderer   *renderer);

void   picman_view_renderer_draw             (PicmanViewRenderer   *renderer,
                                            GtkWidget          *widget,
                                            cairo_t            *cr,
                                            gint                available_width,
                                            gint                available_height);

/*  protected  */

void   picman_view_renderer_render_temp_buf_simple (PicmanViewRenderer *renderer,
                                                  PicmanTempBuf      *temp_buf);
void   picman_view_renderer_render_temp_buf        (PicmanViewRenderer *renderer,
                                                  PicmanTempBuf      *temp_buf,
                                                  gint              temp_buf_x,
                                                  gint              temp_buf_y,
                                                  gint              channel,
                                                  PicmanViewBG        inside_bg,
                                                  PicmanViewBG        outside_bg);
void   picman_view_renderer_render_pixbuf          (PicmanViewRenderer *renderer,
                                                  GdkPixbuf        *pixbuf);
void   picman_view_renderer_render_stock           (PicmanViewRenderer *renderer,
                                                  GtkWidget        *widget,
                                                  const gchar      *stock_id);



#endif /* __PICMAN_VIEW_RENDERER_H__ */
