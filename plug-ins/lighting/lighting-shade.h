#ifndef __LIGHTING_SHADE_H__
#define __LIGHTING_SHADE_H__

typedef PicmanRGB (* get_ray_func) (PicmanVector3 *vector);

PicmanRGB get_ray_color                 (PicmanVector3 *position);
PicmanRGB get_ray_color_no_bilinear     (PicmanVector3 *position);
PicmanRGB get_ray_color_ref             (PicmanVector3 *position);
PicmanRGB get_ray_color_no_bilinear_ref (PicmanVector3 *position);

void    precompute_init               (gint         w,
				       gint         h);
void    precompute_normals            (gint         x1,
				       gint         x2,
				       gint         y);
void    interpol_row                  (gint         x1,
                                       gint         x2,
                                       gint         y);

#endif  /* __LIGHTING_SHADE_H__ */
