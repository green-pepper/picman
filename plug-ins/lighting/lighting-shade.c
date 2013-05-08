/*****************/
/* Shading stuff */
/*****************/

#include "config.h"

#include <libpicman/picman.h>

#include "lighting-main.h"
#include "lighting-image.h"
#include "lighting-shade.h"


static PicmanVector3 *triangle_normals[2] = { NULL, NULL };
static PicmanVector3 *vertex_normals[3]   = { NULL, NULL, NULL };
static gdouble     *heights[3] = { NULL, NULL, NULL };
static gdouble      xstep, ystep;
static guchar      *bumprow = NULL;

static gint pre_w = -1;
static gint pre_h = -1;

/*****************/
/* Phong shading */
/*****************/

static PicmanRGB
phong_shade (PicmanVector3 *position,
             PicmanVector3 *viewpoint,
             PicmanVector3 *normal,
             PicmanVector3 *lightposition,
             PicmanRGB      *diff_col,
             PicmanRGB      *light_col,
             LightType    light_type)
{
  PicmanRGB       diffuse_color, specular_color;
  gdouble      nl, rv, dist;
  PicmanVector3  l, v, n, lnormal, h;

  /* Compute ambient intensity */
  /* ========================= */

  n = *normal;

  /* Compute (N*L) term of Phong's equation */
  /* ====================================== */

  if (light_type == POINT_LIGHT)
    picman_vector3_sub (&l, lightposition, position);
  else
    {
      l = *lightposition;
      picman_vector3_normalize (&l);
    }

  dist = picman_vector3_length (&l);

  if (dist != 0.0)
    picman_vector3_mul (&l, 1.0 / dist);

  nl = MAX (0., 2.0 * picman_vector3_inner_product (&n, &l));

  lnormal = l;
  picman_vector3_normalize (&lnormal);

  if (nl >= 0.0)
    {
      /* Compute (R*V)^alpha term of Phong's equation */
      /* ============================================ */

      picman_vector3_sub (&v, viewpoint, position);
      picman_vector3_normalize (&v);

      picman_vector3_add (&h, &lnormal, &v);
      picman_vector3_normalize (&h);

      rv = MAX (0.01, picman_vector3_inner_product (&n, &h));
      rv = pow (rv, mapvals.material.highlight);
      rv *= nl;

      /* Compute diffuse and specular intensity contribution */
      /* =================================================== */

      diffuse_color = *light_col;
      picman_rgb_multiply (&diffuse_color, mapvals.material.diffuse_int);
      diffuse_color.r *= diff_col->r;
      diffuse_color.g *= diff_col->g;
      diffuse_color.b *= diff_col->b;
      picman_rgb_multiply (&diffuse_color, nl);

      specular_color = *light_col;
      if (mapvals.material.metallic)  /* for metals, specular color = diffuse color */
        {
          specular_color.r *= diff_col->r;
          specular_color.g *= diff_col->g;
          specular_color.b *= diff_col->b;
        }
      picman_rgb_multiply (&specular_color, mapvals.material.specular_ref);
      picman_rgb_multiply (&specular_color, rv);

      picman_rgb_add (&diffuse_color, &specular_color);
      picman_rgb_clamp (&diffuse_color);
    }

  picman_rgb_clamp (&diffuse_color);

  return diffuse_color;
}

void
precompute_init (gint w,
                 gint h)
{
  gint n;
  gint bpp=1;

  xstep = 1.0 / (gdouble) width;
  ystep = 1.0 / (gdouble) height;

  pre_w = w;
  pre_h = h;

  for (n = 0; n < 3; n++)
    {
      if (vertex_normals[n] != NULL)
        g_free (vertex_normals[n]);
      if (heights[n] != NULL)
        g_free (heights[n]);

      heights[n] = g_new (gdouble, w);
      vertex_normals[n] = g_new (PicmanVector3, w);
    }

  for (n = 0; n < 2; n++)
    if (triangle_normals[n] != NULL)
      g_free (triangle_normals[n]);

  if (bumprow != NULL)
    {
      g_free (bumprow);
      bumprow = NULL;
    }
  if (mapvals.bumpmap_id != -1)
    {
      bpp = picman_drawable_bpp(mapvals.bumpmap_id);
    }

  bumprow = g_new (guchar, w * bpp);

  triangle_normals[0] = g_new (PicmanVector3, (w << 1) + 2);
  triangle_normals[1] = g_new (PicmanVector3, (w << 1) + 2);

  for (n = 0; n < (w << 1) + 1; n++)
    {
      picman_vector3_set (&triangle_normals[0][n], 0.0, 0.0, 1.0);
      picman_vector3_set (&triangle_normals[1][n], 0.0, 0.0, 1.0);
    }

  for (n = 0; n < w; n++)
    {
      picman_vector3_set (&vertex_normals[0][n], 0.0, 0.0, 1.0);
      picman_vector3_set (&vertex_normals[1][n], 0.0, 0.0, 1.0);
      picman_vector3_set (&vertex_normals[2][n], 0.0, 0.0, 1.0);
      heights[0][n] = 0.0;
      heights[1][n] = 0.0;
      heights[2][n] = 0.0;
    }
}


/* Interpol linearly height[2] and triangle_normals[1]
 * using the next row
 */
void
interpol_row (gint x1,
              gint x2,
              gint y)
{
  PicmanVector3   p1, p2, p3;
  gint          n, i;
  guchar        *map = NULL;
  gint          bpp = 1;
  guchar* bumprow1 = NULL;
  guchar* bumprow2 = NULL;

  if (mapvals.bumpmap_id != -1)
    {
      bpp = picman_drawable_bpp(mapvals.bumpmap_id);
    }

  bumprow1 = g_new (guchar, pre_w * bpp);
  bumprow2 = g_new (guchar, pre_w * bpp);

  picman_pixel_rgn_get_row (&bump_region, bumprow1, x1, y, x2 - x1);
  picman_pixel_rgn_get_row (&bump_region, bumprow2, x1, y+1, x2 - x1);

  if (mapvals.bumpmaptype > 0)
    {
      switch (mapvals.bumpmaptype)
        {
        case 1:
          map = logmap;
          break;
        case 2:
          map = sinemap;
          break;
        default:
          map = spheremap;
          break;
        }
    }

  for (n = 0; n < (x2 - x1); n++)
    {
      gdouble diff;
      guchar mapval;
      guchar mapval1, mapval2;

      if (bpp>1)
        {
          mapval1 = (guchar)((float)((bumprow1[n * bpp] +bumprow1[n * bpp +1] + bumprow1[n * bpp + 2])/3.0 )) ;
          mapval2 = (guchar)((float)((bumprow2[n * bpp] +bumprow2[n * bpp +1] + bumprow2[n * bpp + 2])/3.0 )) ;
        }
      else
        {
          mapval1 = bumprow1[n * bpp];
          mapval2 = bumprow2[n * bpp];
        }

      diff =  mapval1 - mapval2;
      mapval = (guchar) CLAMP (mapval1 + diff, 0.0, 255.0);

      if (mapvals.bumpmaptype > 0)
        {
          heights[1][n] = (gdouble) mapvals.bumpmax * (gdouble) map[mapval1] / 255.0;
          heights[2][n] = (gdouble) mapvals.bumpmax * (gdouble) map[mapval] / 255.0;
        }
      else
        {
          heights[1][n] = (gdouble) mapvals.bumpmax * (gdouble) mapval1 / 255.0;
          heights[2][n] = (gdouble) mapvals.bumpmax * (gdouble) mapval / 255.0;
        }
    }

  i = 0;
  for (n = 0; n < (x2 - x1 - 1); n++)
    {
      /* heights rows 1 and 2 are inverted */
      p1.x = 0.0;
      p1.y = ystep;
      p1.z = heights[1][n] - heights[2][n];

      p2.x = xstep;
      p2.y = ystep;
      p2.z = heights[1][n+1] - heights[2][n];

      p3.x = xstep;
      p3.y = 0.0;
      p3.z = heights[2][n+1] - heights[2][n];

      triangle_normals[1][i] = picman_vector3_cross_product (&p2, &p1);
      triangle_normals[1][i+1] = picman_vector3_cross_product (&p3, &p2);

      picman_vector3_normalize (&triangle_normals[1][i]);
      picman_vector3_normalize (&triangle_normals[1][i+1]);

      i += 2;
    }

  g_free (bumprow1);
  g_free (bumprow2);
}

/********************************************/
/* Compute triangle and then vertex normals */
/********************************************/


void
precompute_normals (gint x1,
                    gint x2,
                    gint y)
{
  PicmanVector3 *tmpv, p1, p2, p3, normal;
  gdouble     *tmpd;
  gint         n, i, nv;
  guchar      *map = NULL;
  gint bpp = 1;
  guchar mapval;


  /* First, compute the heights */
  /* ========================== */

  tmpv                = triangle_normals[0];
  triangle_normals[0] = triangle_normals[1];
  triangle_normals[1] = tmpv;

  tmpv              = vertex_normals[0];
  vertex_normals[0] = vertex_normals[1];
  vertex_normals[1] = vertex_normals[2];
  vertex_normals[2] = tmpv;

  tmpd       = heights[0];
  heights[0] = heights[1];
  heights[1] = heights[2];
  heights[2] = tmpd;

  if (mapvals.bumpmap_id != -1)
    {
      bpp = picman_drawable_bpp(mapvals.bumpmap_id);
    }

  picman_pixel_rgn_get_row (&bump_region, bumprow, x1, y, x2 - x1);

  if (mapvals.bumpmaptype > 0)
    {
      switch (mapvals.bumpmaptype)
        {
          case 1:
            map = logmap;
            break;
          case 2:
            map = sinemap;
            break;
          default:
            map = spheremap;
            break;
        }

      for (n = 0; n < (x2 - x1); n++)
        {
          if (bpp>1)
            {
              mapval = (guchar)((float)((bumprow[n * bpp] +bumprow[n * bpp +1] + bumprow[n * bpp + 2])/3.0 )) ;
            }
          else
            {
              mapval = bumprow[n * bpp];
            }

          heights[2][n] = (gdouble) mapvals.bumpmax * (gdouble) map[mapval] / 255.0;
        }
    }
  else
    {
      for (n = 0; n < (x2 - x1); n++)
        {
          if (bpp>1)
            {
              mapval = (guchar)((float)((bumprow[n * bpp] +bumprow[n * bpp +1] + bumprow[n * bpp + 2])/3.0 )) ;
            }
          else
            {
              mapval = bumprow[n * bpp];
            }
          heights[2][n] = (gdouble) mapvals.bumpmax * (gdouble) mapval / 255.0;
        }
    }

  /* Compute triangle normals */
  /* ======================== */

  i = 0;
  for (n = 0; n < (x2 - x1 - 1); n++)
    {
      p1.x = 0.0;
      p1.y = ystep;
      p1.z = heights[2][n] - heights[1][n];

      p2.x = xstep;
      p2.y = ystep;
      p2.z = heights[2][n+1] - heights[1][n];

      p3.x = xstep;
      p3.y = 0.0;
      p3.z = heights[1][n+1] - heights[1][n];

      triangle_normals[1][i] = picman_vector3_cross_product (&p2, &p1);
      triangle_normals[1][i+1] = picman_vector3_cross_product (&p3, &p2);

      picman_vector3_normalize (&triangle_normals[1][i]);
      picman_vector3_normalize (&triangle_normals[1][i+1]);

      i += 2;
    }

  /* Compute vertex normals */
  /* ====================== */

  i = 0;
  picman_vector3_set (&normal, 0.0, 0.0, 0.0);

  for (n = 0; n < (x2 - x1 - 1); n++)
    {
      nv = 0;

      if (n > 0)
        {
          if (y > 0)
            {
              picman_vector3_add (&normal, &normal, &triangle_normals[0][i-1]);
              picman_vector3_add (&normal, &normal, &triangle_normals[0][i-2]);
              nv += 2;
            }

          if (y < pre_h)
            {
              picman_vector3_add (&normal, &normal, &triangle_normals[1][i-1]);
              nv++;
            }
        }

      if (n <pre_w)
        {
          if (y > 0)
            {
              picman_vector3_add (&normal, &normal, &triangle_normals[0][i]);
              picman_vector3_add (&normal, &normal, &triangle_normals[0][i+1]);
              nv += 2;
            }

          if (y < pre_h)
            {
              picman_vector3_add (&normal, &normal, &triangle_normals[1][i]);
              picman_vector3_add (&normal, &normal, &triangle_normals[1][i+1]);
              nv += 2;
            }
        }

      picman_vector3_mul (&normal, 1.0 / (gdouble) nv);
      picman_vector3_normalize (&normal);
      vertex_normals[1][n] = normal;

      i += 2;
    }
}

/***********************************************************************/
/* Compute the reflected ray given the normalized normal and ins. vec. */
/***********************************************************************/

static PicmanVector3
compute_reflected_ray (PicmanVector3 *normal,
                       PicmanVector3 *view)
{
  PicmanVector3 ref;
  gdouble     nl;

  nl = 2.0 * picman_vector3_inner_product (normal, view);

  ref = *normal;

  picman_vector3_mul (&ref, nl);
  picman_vector3_sub (&ref, &ref, view);

  return ref;
}

/************************************************************************/
/* Given the NorthPole, Equator and a third vector (normal) compute     */
/* the conversion from spherical coordinates to image space coordinates */
/************************************************************************/

static void
sphere_to_image (PicmanVector3 *normal,
                 gdouble     *u,
                 gdouble     *v)
{
  static gdouble     alpha, fac;
  static PicmanVector3 cross_prod;
  static PicmanVector3 firstaxis  = { 1.0, 0.0, 0.0 };
  static PicmanVector3 secondaxis = { 0.0, 1.0, 0.0 };

  alpha = acos (-picman_vector3_inner_product (&secondaxis, normal));

  *v = alpha / G_PI;

  if (*v==0.0 || *v==1.0)
    {
      *u = 0.0;
    }
  else
    {
      fac = picman_vector3_inner_product (&firstaxis, normal) / sin (alpha);

      /* Make sure that we map to -1.0..1.0 (take care of rounding errors) */
      /* ================================================================= */

      if (fac>1.0)
        fac = 1.0;
      else if (fac<-1.0)
        fac = -1.0;

      *u = acos (fac) / (2.0 * G_PI);

      cross_prod = picman_vector3_cross_product (&secondaxis, &firstaxis);

      if (picman_vector3_inner_product (&cross_prod, normal) < 0.0)
        *u = 1.0 - *u;
    }
}

/*********************************************************************/
/* These routines computes the color of the surface at a given point */
/*********************************************************************/

PicmanRGB
get_ray_color (PicmanVector3 *position)
{
  PicmanRGB       color;
  PicmanRGB       color_int;
  PicmanRGB       color_sum;
  PicmanRGB       light_color;
  gint          x, f;
  gdouble       xf, yf;
  PicmanVector3   normal, *p;
  gint          k;

  pos_to_float (position->x, position->y, &xf, &yf);

  x = RINT (xf);

  if (mapvals.transparent_background && heights[1][x] == 0)
    {
      picman_rgb_set_alpha (&color_sum, 0.0);
    }
  else
    {
      color = get_image_color (xf, yf, &f);

      color_sum = color;
      picman_rgb_multiply (&color_sum, mapvals.material.ambient_int);

      for (k = 0; k < NUM_LIGHTS; k++)
        {
          if (!mapvals.lightsource[k].active
              || mapvals.lightsource[k].type == NO_LIGHT)
            continue;
          else if (mapvals.lightsource[k].type == POINT_LIGHT)
            p = &mapvals.lightsource[k].position;
          else
            p = &mapvals.lightsource[k].direction;

          color_int = mapvals.lightsource[k].color;
          picman_rgb_multiply (&color_int, mapvals.lightsource[k].intensity);

          if (mapvals.bump_mapped == FALSE || mapvals.bumpmap_id == -1)
            {
              light_color = phong_shade (position,
                                         &mapvals.viewpoint,
                                         &mapvals.planenormal,
                                         p,
                                         &color,
                                         &color_int,
                                         mapvals.lightsource[k].type);
            }
          else
            {
              normal = vertex_normals[1][(gint) RINT (xf)];

              light_color = phong_shade (position,
                                         &mapvals.viewpoint,
                                         &normal,
                                         p,
                                         &color,
                                         &color_int,
                                         mapvals.lightsource[k].type);
            }

          picman_rgb_add (&color_sum, &light_color);
        }
    }

  picman_rgb_clamp (&color_sum);
  return color_sum;
}

PicmanRGB
get_ray_color_ref (PicmanVector3 *position)
{
  PicmanRGB      color_sum;
  PicmanRGB      color_int;
  PicmanRGB      light_color;
  PicmanRGB      color, env_color;
  gint         x, f;
  gdouble      xf, yf;
  PicmanVector3  normal, *p, v, r;
  gint         k;
  gdouble      tmpval;

  pos_to_float (position->x, position->y, &xf, &yf);

  x = RINT (xf);

  if (mapvals.bump_mapped == FALSE || mapvals.bumpmap_id == -1)
    normal = mapvals.planenormal;
  else
    normal = vertex_normals[1][(gint) RINT (xf)];
  picman_vector3_normalize (&normal);

  if (mapvals.transparent_background && heights[1][x] == 0)
    {
      picman_rgb_set_alpha (&color_sum, 0.0);
    }
  else
    {
      color = get_image_color (xf, yf, &f);
      color_sum = color;
      picman_rgb_multiply (&color_sum, mapvals.material.ambient_int);

      for (k = 0; k < NUM_LIGHTS; k++)
        {
          p = &mapvals.lightsource[k].direction;

          if (!mapvals.lightsource[k].active
              || mapvals.lightsource[k].type == NO_LIGHT)
            continue;
          else if (mapvals.lightsource[k].type == POINT_LIGHT)
            p = &mapvals.lightsource[k].position;

          color_int = mapvals.lightsource[k].color;
          picman_rgb_multiply (&color_int, mapvals.lightsource[k].intensity);

          light_color = phong_shade (position,
                                     &mapvals.viewpoint,
                                     &normal,
                                     p,
                                     &color,
                                     &color_int,
                                     mapvals.lightsource[0].type);
        }

      picman_vector3_sub (&v, &mapvals.viewpoint, position);
      picman_vector3_normalize (&v);

      r = compute_reflected_ray (&normal, &v);

      /* Get color in the direction of r */
      /* =============================== */

      sphere_to_image (&r, &xf, &yf);
      env_color = peek_env_map (RINT (env_width * xf),
                                RINT (env_height * yf));

      tmpval = mapvals.material.diffuse_int;
      mapvals.material.diffuse_int = 0.;

      light_color = phong_shade (position,
                                 &mapvals.viewpoint,
                                 &normal,
                                 &r,
                                 &color,
                                 &env_color,
                                 DIRECTIONAL_LIGHT);

      mapvals.material.diffuse_int = tmpval;

      picman_rgb_add (&color_sum, &light_color);
    }

  picman_rgb_clamp (&color_sum);
  return color_sum;
}

PicmanRGB
get_ray_color_no_bilinear (PicmanVector3 *position)
{
  PicmanRGB       color;
  PicmanRGB       color_int;
  PicmanRGB       color_sum;
  PicmanRGB       light_color;
  gint          x;
  gdouble       xf, yf;
  PicmanVector3   normal, *p;
  gint          k;


  pos_to_float (position->x, position->y, &xf, &yf);

  x = RINT (xf);

  if (mapvals.transparent_background && heights[1][x] == 0)
    {
      picman_rgb_set_alpha (&color_sum, 0.0);
    }
  else
    {
      color = peek (x, RINT (yf));

      color_sum = color;
      picman_rgb_multiply (&color_sum, mapvals.material.ambient_int);

      for (k = 0; k < NUM_LIGHTS; k++)
        {
          p = &mapvals.lightsource[k].direction;

          if (!mapvals.lightsource[k].active
              || mapvals.lightsource[k].type == NO_LIGHT)
            continue;
          else if (mapvals.lightsource[k].type == POINT_LIGHT)
            p = &mapvals.lightsource[k].position;

          color_int = mapvals.lightsource[k].color;
          picman_rgb_multiply (&color_int, mapvals.lightsource[k].intensity);

          if (mapvals.bump_mapped == FALSE || mapvals.bumpmap_id == -1)
            {
              light_color = phong_shade (position,
                                         &mapvals.viewpoint,
                                         &mapvals.planenormal,
                                         p,
                                         &color,
                                         &color_int,
                                         mapvals.lightsource[k].type);
            }
          else
            {
              normal = vertex_normals[1][x];

              light_color = phong_shade (position,
                                         &mapvals.viewpoint,
                                         &normal,
                                         p,
                                         &color,
                                         &color_int,
                                         mapvals.lightsource[k].type);
            }

          picman_rgb_add (&color_sum, &light_color);
        }
    }

  picman_rgb_clamp (&color_sum);
  return color_sum;
}

PicmanRGB
get_ray_color_no_bilinear_ref (PicmanVector3 *position)
{
  PicmanRGB      color_sum;
  PicmanRGB      color_int;
  PicmanRGB      light_color;
  PicmanRGB      color, env_color;
  gint         x;
  gdouble      xf, yf;
  PicmanVector3  normal, *p, v, r;
  gint         k;
  gdouble      tmpval;

  pos_to_float (position->x, position->y, &xf, &yf);

  x = RINT (xf);

  if (mapvals.bump_mapped == FALSE || mapvals.bumpmap_id == -1)
    normal = mapvals.planenormal;
  else
    normal = vertex_normals[1][(gint) RINT (xf)];
  picman_vector3_normalize (&normal);

  if (mapvals.transparent_background && heights[1][x] == 0)
    {
      picman_rgb_set_alpha (&color_sum, 0.0);
    }
  else
    {
      color = peek (RINT (xf), RINT (yf));
      color_sum = color;
      picman_rgb_multiply (&color_sum, mapvals.material.ambient_int);

      for (k = 0; k < NUM_LIGHTS; k++)
        {
          p = &mapvals.lightsource[k].direction;

          if (!mapvals.lightsource[k].active
              || mapvals.lightsource[k].type == NO_LIGHT)
            continue;
          else if (mapvals.lightsource[k].type == POINT_LIGHT)
            p = &mapvals.lightsource[k].position;

          color_int = mapvals.lightsource[k].color;
          picman_rgb_multiply (&color_int, mapvals.lightsource[k].intensity);

              light_color = phong_shade (position,
                                         &mapvals.viewpoint,
                                         &normal,
                                         p,
                                         &color,
                                         &color_int,
                                         mapvals.lightsource[0].type);
        }

      picman_vector3_sub (&v, &mapvals.viewpoint, position);
      picman_vector3_normalize (&v);

      r = compute_reflected_ray (&normal, &v);

      /* Get color in the direction of r */
      /* =============================== */

      sphere_to_image (&r, &xf, &yf);
      env_color = peek_env_map (RINT (env_width * xf),
                                RINT (env_height * yf));

      tmpval = mapvals.material.diffuse_int;
      mapvals.material.diffuse_int = 0.;

      light_color = phong_shade (position,
                                 &mapvals.viewpoint,
                                 &normal,
                                 &r,
                                 &color,
                                 &env_color,
                                 DIRECTIONAL_LIGHT);

      mapvals.material.diffuse_int = tmpval;

      picman_rgb_add (&color_sum, &light_color);
    }

 picman_rgb_clamp (&color_sum);
 return color_sum;
}
