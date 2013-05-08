#ifndef __MAPOBJECT_IMAGE_H__
#define __MAPOBJECT_IMAGE_H__

/* Externally visible variables */
/* ============================ */

extern PicmanDrawable *input_drawable, *output_drawable;
extern PicmanPixelRgn  source_region,dest_region;

extern PicmanDrawable *box_drawables[6];
extern PicmanPixelRgn  box_regions[6];

extern PicmanDrawable *cylinder_drawables[2];
extern PicmanPixelRgn  cylinder_regions[2];

extern guchar          *preview_rgb_data;
extern gint             preview_rgb_stride;
extern cairo_surface_t *preview_surface;

extern glong   maxcounter, old_depth, max_depth;
extern gint    imgtype, width,height, in_channels, out_channels, image_id;
extern PicmanRGB  background;
extern gdouble oldtreshold;

extern gint border_x1, border_y1, border_x2, border_y2;

extern PicmanTile *current_in_tile, *current_out_tile;

/* Externally visible functions */
/* ============================ */

extern gint        image_setup              (PicmanDrawable *drawable,
                                             gint          interactive);
extern glong       in_xy_to_index           (gint          x,
                                             gint          y);
extern glong       out_xy_to_index          (gint          x,
                                             gint          y);
extern gint        checkbounds              (gint          x,
                                             gint          y);
extern PicmanRGB      peek                     (gint          x,
                                             gint          y);
extern void        poke                     (gint          x,
                                             gint          y,
                                             PicmanRGB      *color,
                                             gpointer      data);
extern PicmanVector3 int_to_pos               (gint          x,
                                             gint          y);
extern void        pos_to_int               (gdouble       x,
                                             gdouble       y,
                                             gint         *scr_x,
                                             gint         *scr_y);

extern PicmanRGB      get_image_color          (gdouble      u,
                                              gdouble      v,
                                              gint        *inside);
extern PicmanRGB      get_box_image_color      (gint         image,
                                              gdouble      u,
                                              gdouble      v);
extern PicmanRGB      get_cylinder_image_color (gint         image,
                                              gdouble      u,
                                              gdouble      v);

#endif  /* __MAPOBJECT_IMAGE_H__ */
