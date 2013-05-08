#ifndef __MAPOBJECT_SHADE_H__
#define __MAPOBJECT_SHADE_H__

typedef PicmanRGB (* get_ray_color_func) (PicmanVector3 *pos);

extern get_ray_color_func get_ray_color;

PicmanRGB   get_ray_color_plane    (PicmanVector3 *pos);
PicmanRGB   get_ray_color_sphere   (PicmanVector3 *pos);
PicmanRGB   get_ray_color_box      (PicmanVector3 *pos);
PicmanRGB   get_ray_color_cylinder (PicmanVector3 *pos);
void     compute_bounding_box   (void);

void     vecmulmat              (PicmanVector3 *u,
                                 PicmanVector3 *v,
                                 gfloat       m[16]);
void     rotatemat              (gfloat       angle,
                                 PicmanVector3 *v,
                                 gfloat       m[16]);
void     transpose_mat          (gfloat       m[16]);
void     matmul                 (gfloat       a[16],
                                 gfloat       b[16],
                                 gfloat       c[16]);
void     ident_mat              (gfloat       m[16]);

#endif  /* __MAPOBJECT_SHADE_H__ */
