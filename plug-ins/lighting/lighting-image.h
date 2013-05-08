#ifndef __LIGHTING_IMAGE_H__
#define __LIGHTING_IMAGE_H__

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

extern PicmanDrawable *input_drawable,*output_drawable;
extern PicmanPixelRgn  source_region, dest_region;

extern PicmanDrawable *bump_drawable;
extern PicmanPixelRgn  bump_region;

extern PicmanDrawable *env_drawable;
extern PicmanPixelRgn  env_region;

extern guchar          *preview_rgb_data;
extern gint             preview_rgb_stride;
extern cairo_surface_t *preview_surface;

extern glong  maxcounter;
extern gint   imgtype,width,height,env_width,env_height,in_channels,out_channels;
extern PicmanRGB background;

extern gint   border_x1,border_y1,border_x2,border_y2;

extern guchar sinemap[256], spheremap[256], logmap[256];

guchar         peek_map        (PicmanPixelRgn *region,
				gint          x,
				gint          y);
PicmanRGB         peek            (gint          x,
				gint          y);
PicmanRGB         peek_env_map    (gint          x,
				gint          y);
void           poke            (gint          x,
				gint          y,
				PicmanRGB       *color);
gint           check_bounds    (gint          x,
				gint          y);
PicmanVector3    int_to_pos      (gint          x,
				gint          y);
PicmanVector3    int_to_posf     (gdouble       x,
				gdouble       y);
void           pos_to_int      (gdouble       x,
				gdouble       y,
				gint         *scr_x,
				gint         *scr_y);
void           pos_to_float    (gdouble       x,
				gdouble       y,
				gdouble      *xf,
				gdouble      *yf);
PicmanRGB         get_image_color (gdouble       u,
				gdouble       v,
				gint         *inside);
gdouble        get_map_value   (PicmanPixelRgn *region,
				gdouble       u,
				gdouble       v,
				gint         *inside);
gint           image_setup     (PicmanDrawable *drawable,
				gint          interactive);

#endif  /* __LIGHTING_IMAGE_H__ */
