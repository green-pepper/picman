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

#ifndef __PICMAN_DISPLAY_SHELL_H__
#define __PICMAN_DISPLAY_SHELL_H__


/* Apply to a float the same rounding mode used in the renderer */
#define  PROJ_ROUND(coord)   ((gint) RINT (coord))
#define  PROJ_ROUND64(coord) ((gint64) RINT (coord))

/* scale values */
#define  SCALEX(s,x)      PROJ_ROUND ((x) * (s)->scale_x)
#define  SCALEY(s,y)      PROJ_ROUND ((y) * (s)->scale_y)

/* unscale values */
#define  UNSCALEX(s,x)    ((gint) ((x) / (s)->scale_x))
#define  UNSCALEY(s,y)    ((gint) ((y) / (s)->scale_y))
/* (and float-returning versions) */
#define  FUNSCALEX(s,x)   ((x) / (s)->scale_x)
#define  FUNSCALEY(s,y)   ((y) / (s)->scale_y)


#define PICMAN_TYPE_DISPLAY_SHELL            (picman_display_shell_get_type ())
#define PICMAN_DISPLAY_SHELL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DISPLAY_SHELL, PicmanDisplayShell))
#define PICMAN_DISPLAY_SHELL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DISPLAY_SHELL, PicmanDisplayShellClass))
#define PICMAN_IS_DISPLAY_SHELL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DISPLAY_SHELL))
#define PICMAN_IS_DISPLAY_SHELL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DISPLAY_SHELL))
#define PICMAN_DISPLAY_SHELL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DISPLAY_SHELL, PicmanDisplayShellClass))


typedef struct _PicmanDisplayShellClass  PicmanDisplayShellClass;

struct _PicmanDisplayShell
{
  GtkBox             parent_instance;

  PicmanDisplay       *display;

  PicmanUIManager     *popup_manager;

  PicmanDisplayOptions *options;
  PicmanDisplayOptions *fullscreen_options;
  PicmanDisplayOptions *no_image_options;

  gboolean           snap_to_guides;   /*  should the guides be snapped to?   */
  gboolean           snap_to_grid;     /*  should the grid be snapped to?     */
  gboolean           snap_to_canvas;   /*  should the canvas be snapped to?   */
  gboolean           snap_to_vectors;  /*  should the active path be snapped  */

  PicmanUnit           unit;

  gint               offset_x;         /*  offset of display image            */
  gint               offset_y;

  gdouble            scale_x;          /*  horizontal scale factor            */
  gdouble            scale_y;          /*  vertical scale factor              */

  gdouble            rotate_angle;
  cairo_matrix_t    *rotate_transform;
  cairo_matrix_t    *rotate_untransform;

  gdouble            monitor_xres;
  gdouble            monitor_yres;
  gboolean           dot_for_dot;      /*  ignore monitor resolution          */

  PicmanZoomModel     *zoom;

  gdouble            last_scale;       /*  scale used when reverting zoom     */
  guint              last_scale_time;  /*  time when last_scale was set       */
  gint               last_offset_x;    /*  offsets used when reverting zoom   */
  gint               last_offset_y;

  gdouble            other_scale;      /*  scale factor entered in Zoom->Other*/

  gint               disp_width;       /*  width of drawing area              */
  gint               disp_height;      /*  height of drawing area             */

  gboolean           proximity;        /*  is a device in proximity           */

  Selection         *selection;        /*  Selection (marching ants)          */

  GList             *children;

  GtkWidget         *canvas;           /*  PicmanCanvas widget                  */

  GtkAdjustment     *hsbdata;          /*  adjustments                        */
  GtkAdjustment     *vsbdata;
  GtkWidget         *hsb;              /*  scroll bars                        */
  GtkWidget         *vsb;

  GtkWidget         *hrule;            /*  rulers                             */
  GtkWidget         *vrule;

  GtkWidget         *origin;           /*  NW: origin                         */
  GtkWidget         *quick_mask_button;/*  SW: quick mask button              */
  GtkWidget         *zoom_button;      /*  NE: zoom toggle button             */
  GtkWidget         *nav_ebox;         /*  SE: navigation event box           */

  GtkWidget         *statusbar;        /*  statusbar                          */

  PicmanDisplayXfer   *xfer;             /*  managers image buffer transfers    */
  cairo_surface_t   *mask_surface;     /*  buffer for rendering the mask      */
  cairo_pattern_t   *checkerboard;     /*  checkerboard pattern               */

  PicmanCanvasItem    *canvas_item;      /*  items drawn on the canvas          */
  PicmanCanvasItem    *unrotated_item;   /*  unrotated items for e.g. cursor    */
  PicmanCanvasItem    *passe_partout;    /*  item for the highlight             */
  PicmanCanvasItem    *preview_items;    /*  item for previews                  */
  PicmanCanvasItem    *vectors;          /*  item proxy of vectors              */
  PicmanCanvasItem    *grid;             /*  item proxy of the grid             */
  PicmanCanvasItem    *guides;           /*  item proxies of guides             */
  PicmanCanvasItem    *sample_points;    /*  item proxies of sample points      */
  PicmanCanvasItem    *layer_boundary;   /*  item for the layer boundary        */
  PicmanCanvasItem    *tool_items;       /*  tools items, below the cursor      */
  PicmanCanvasItem    *cursor;           /*  item for the software cursor       */

  guint              title_idle_id;    /*  title update idle ID               */
  gchar             *title;            /*  current title                      */
  gchar             *status;           /*  current default statusbar content  */

  gint               icon_size;        /*  size of the icon pixmap            */
  guint              icon_idle_id;     /*  ID of the idle-function            */
  GdkPixbuf         *icon;             /*  icon                               */

  guint              fill_idle_id;     /*  display_shell_fill() idle ID       */

  PicmanHandedness     cursor_handedness;/*  Handedness for cursor display      */
  PicmanCursorType     current_cursor;   /*  Currently installed main cursor    */
  PicmanToolCursorType tool_cursor;      /*  Current Tool cursor                */
  PicmanCursorModifier cursor_modifier;  /*  Cursor modifier (plus, minus, ...) */

  PicmanCursorType     override_cursor;  /*  Overriding cursor                  */
  gboolean           using_override_cursor;
  gboolean           draw_cursor;      /* should we draw software cursor ?    */

  GtkWidget         *close_dialog;     /*  close dialog                       */
  GtkWidget         *scale_dialog;     /*  scale (zoom) dialog                */
  GtkWidget         *rotate_dialog;    /*  rotate dialog                      */
  GtkWidget         *nav_popup;        /*  navigation popup                   */
  GtkWidget         *grid_dialog;      /*  grid configuration dialog          */

  PicmanColorDisplayStack *filter_stack;   /* color display conversion stuff    */
  guint                  filter_idle_id;
  GtkWidget             *filters_dialog; /* color display filter dialog       */

  gint               paused_count;

  PicmanTreeHandler   *vectors_freeze_handler;
  PicmanTreeHandler   *vectors_thaw_handler;
  PicmanTreeHandler   *vectors_visible_handler;

  gboolean           zoom_on_resize;

  gboolean           size_allocate_from_configure_event;

  /*  the state of picman_display_shell_tool_events()  */
  gboolean           pointer_grabbed;
  guint32            pointer_grab_time;

  gboolean           keyboard_grabbed;
  guint32            keyboard_grab_time;

  gboolean           space_pressed;
  gboolean           space_release_pending;
  const gchar       *space_shaded_tool;

  gboolean           scrolling;
  gint               scroll_last_x;
  gint               scroll_last_y;
  gboolean           rotating;
  gdouble            rotate_drag_angle;
  gpointer           scroll_info;

  PicmanDrawable      *mask;
  PicmanRGB            mask_color;

  PicmanMotionBuffer  *motion_buffer;

  GQueue            *zoom_focus_pointer_queue;
};

struct _PicmanDisplayShellClass
{
  GtkBoxClass  parent_class;

  void (* scaled)    (PicmanDisplayShell *shell);
  void (* scrolled)  (PicmanDisplayShell *shell);
  void (* rotated)   (PicmanDisplayShell *shell);
  void (* reconnect) (PicmanDisplayShell *shell);
};


GType             picman_display_shell_get_type      (void) G_GNUC_CONST;

GtkWidget       * picman_display_shell_new           (PicmanDisplay        *display,
                                                    PicmanUnit            unit,
                                                    gdouble             scale,
                                                    PicmanUIManager      *popup_manager);

void              picman_display_shell_add_overlay   (PicmanDisplayShell   *shell,
                                                    GtkWidget          *child,
                                                    gdouble             image_x,
                                                    gdouble             image_y,
                                                    PicmanHandleAnchor    anchor,
                                                    gint                spacing_x,
                                                    gint                spacing_y);
void              picman_display_shell_move_overlay  (PicmanDisplayShell   *shell,
                                                    GtkWidget          *child,
                                                    gdouble             image_x,
                                                    gdouble             image_y,
                                                    PicmanHandleAnchor    anchor,
                                                    gint                spacing_x,
                                                    gint                spacing_y);

PicmanImageWindow * picman_display_shell_get_window    (PicmanDisplayShell   *shell);
PicmanStatusbar   * picman_display_shell_get_statusbar (PicmanDisplayShell   *shell);

void              picman_display_shell_present       (PicmanDisplayShell   *shell);

void              picman_display_shell_reconnect     (PicmanDisplayShell   *shell);

void              picman_display_shell_empty         (PicmanDisplayShell   *shell);
void              picman_display_shell_fill          (PicmanDisplayShell   *shell,
                                                    PicmanImage          *image,
                                                    PicmanUnit            unit,
                                                    gdouble             scale);

void              picman_display_shell_scale_changed (PicmanDisplayShell   *shell);

void              picman_display_shell_scaled        (PicmanDisplayShell   *shell);
void              picman_display_shell_scrolled      (PicmanDisplayShell   *shell);
void              picman_display_shell_rotated       (PicmanDisplayShell   *shell);

void              picman_display_shell_set_unit      (PicmanDisplayShell   *shell,
                                                    PicmanUnit            unit);
PicmanUnit          picman_display_shell_get_unit      (PicmanDisplayShell   *shell);

gboolean          picman_display_shell_snap_coords   (PicmanDisplayShell   *shell,
                                                    PicmanCoords         *coords,
                                                    gint                snap_offset_x,
                                                    gint                snap_offset_y,
                                                    gint                snap_width,
                                                    gint                snap_height);

gboolean          picman_display_shell_mask_bounds   (PicmanDisplayShell   *shell,
                                                    gint               *x1,
                                                    gint               *y1,
                                                    gint               *x2,
                                                    gint               *y2);

void              picman_display_shell_flush         (PicmanDisplayShell   *shell,
                                                    gboolean            now);

void              picman_display_shell_pause         (PicmanDisplayShell   *shell);
void              picman_display_shell_resume        (PicmanDisplayShell   *shell);

void              picman_display_shell_set_highlight (PicmanDisplayShell   *shell,
                                                    const GdkRectangle *highlight);
void              picman_display_shell_set_mask      (PicmanDisplayShell   *shell,
                                                    PicmanDrawable       *mask,
                                                    const PicmanRGB      *color);


#endif /* __PICMAN_DISPLAY_SHELL_H__ */
