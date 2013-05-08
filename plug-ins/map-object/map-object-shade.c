/*****************/
/* Shading stuff */
/*****************/

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "map-object-apply.h"
#include "map-object-main.h"
#include "map-object-image.h"
#include "map-object-shade.h"


static gdouble     bx1, by1, bx2, by2;
get_ray_color_func get_ray_color;

typedef struct
{
  gdouble     u, v;
  gdouble     t;
  PicmanVector3 s;
  PicmanVector3 n;
  gint        face;
} FaceIntersectInfo;

/*****************/
/* Phong shading */
/*****************/

static PicmanRGB
phong_shade (PicmanVector3 *pos,
             PicmanVector3 *viewpoint,
             PicmanVector3 *normal,
             PicmanRGB     *diff_col,
             PicmanRGB     *spec_col,
             LightType    type)
{
  PicmanRGB       ambientcolor, diffusecolor, specularcolor;
  gdouble      NL, RV, dist;
  PicmanVector3  L, NN, V, N;
  PicmanVector3  *light;

  light = mapvals.lightsource.type == DIRECTIONAL_LIGHT
    ? &mapvals.lightsource.direction
    : &mapvals.lightsource.position,

  /* Compute ambient intensity */
  /* ========================= */

  N = *normal;
  ambientcolor = *diff_col;
  picman_rgb_multiply (&ambientcolor, mapvals.material.ambient_int);

  /* Compute (N*L) term of Phong's equation */
  /* ====================================== */

  if (type == POINT_LIGHT)
    picman_vector3_sub (&L, light, pos);
  else
    L = *light;

  dist = picman_vector3_length (&L);

  if (dist != 0.0)
    picman_vector3_mul (&L, 1.0 / dist);

  NL = 2.0 * picman_vector3_inner_product (&N, &L);

  if (NL >= 0.0)
    {
      /* Compute (R*V)^alpha term of Phong's equation */
      /* ============================================ */

      picman_vector3_sub (&V, viewpoint, pos);
      picman_vector3_normalize (&V);

      picman_vector3_mul (&N, NL);
      picman_vector3_sub (&NN, &N, &L);
      RV = picman_vector3_inner_product (&NN, &V);
      RV = 0.0 < RV ? pow (RV, mapvals.material.highlight) : 0.0;

      /* Compute diffuse and specular intensity contribution */
      /* =================================================== */

      diffusecolor = *diff_col;
      picman_rgb_multiply (&diffusecolor, mapvals.material.diffuse_ref);
      picman_rgb_multiply (&diffusecolor, NL);

      specularcolor = *spec_col;
      picman_rgb_multiply (&specularcolor, mapvals.material.specular_ref);
      picman_rgb_multiply (&specularcolor, RV);

      picman_rgb_add (&diffusecolor, &specularcolor);
      picman_rgb_multiply (&diffusecolor, mapvals.material.diffuse_int);
      picman_rgb_clamp (&diffusecolor);

      picman_rgb_add (&ambientcolor, &diffusecolor);
    }

  return ambientcolor;
}

static gint
plane_intersect (PicmanVector3 *dir,
                 PicmanVector3 *viewp,
                 PicmanVector3 *ipos,
                 gdouble     *u,
                 gdouble     *v)
{
  static gdouble det, det1, det2, det3, t;

  imat[0][0] = dir->x;
  imat[1][0] = dir->y;
  imat[2][0] = dir->z;

  /* Compute determinant of the first 3x3 sub matrix (denominator) */
  /* ============================================================= */

  det = (imat[0][0] * imat[1][1] * imat[2][2] +
         imat[0][1] * imat[1][2] * imat[2][0] +
         imat[0][2] * imat[1][0] * imat[2][1] -
         imat[0][2] * imat[1][1] * imat[2][0] -
         imat[0][0] * imat[1][2] * imat[2][1] -
         imat[2][2] * imat[0][1] * imat[1][0]);

  /* If the determinant is non-zero, a intersection point exists */
  /* =========================================================== */

  if (det != 0.0)
    {
      /* Now, lets compute the numerator determinants (wow ;) */
      /* ==================================================== */

      det1 = (imat[0][3] * imat[1][1] * imat[2][2] +
              imat[0][1] * imat[1][2] * imat[2][3] +
              imat[0][2] * imat[1][3] * imat[2][1] -
              imat[0][2] * imat[1][1] * imat[2][3] -
              imat[1][2] * imat[2][1] * imat[0][3] -
              imat[2][2] * imat[0][1] * imat[1][3]);

      det2 = (imat[0][0] * imat[1][3] * imat[2][2] +
              imat[0][3] * imat[1][2] * imat[2][0] +
              imat[0][2] * imat[1][0] * imat[2][3] -
              imat[0][2] * imat[1][3] * imat[2][0] -
              imat[1][2] * imat[2][3] * imat[0][0] -
              imat[2][2] * imat[0][3] * imat[1][0]);

      det3 = (imat[0][0] * imat[1][1] * imat[2][3] +
              imat[0][1] * imat[1][3] * imat[2][0] +
              imat[0][3] * imat[1][0] * imat[2][1] -
              imat[0][3] * imat[1][1] * imat[2][0] -
              imat[1][3] * imat[2][1] * imat[0][0] -
              imat[2][3] * imat[0][1] * imat[1][0]);

      /* Now we have the simultaneous solutions. Lets compute the unknowns */
      /* (skip u&v if t is <0, this means the intersection is behind us)  */
      /* ================================================================ */

      t = det1 / det;

      if (t > 0.0)
        {
          *u = 1.0 + ((det2 / det) - 0.5);
          *v = 1.0 + ((det3 / det) - 0.5);

                ipos->x = viewp->x + t * dir->x;
          ipos->y = viewp->y + t * dir->y;
          ipos->z = viewp->z + t * dir->z;

          return TRUE;
        }
    }

  return FALSE;
}

/*****************************************************************************
 * These routines computes the color of the surface
 * of the plane at a given point
 *****************************************************************************/

PicmanRGB
get_ray_color_plane (PicmanVector3 *pos)
{
  PicmanRGB color = background;

  static gint         inside = FALSE;
  static PicmanVector3  ray, spos;
  static gdouble      vx, vy;

  /* Construct a line from our VP to the point */
  /* ========================================= */

  picman_vector3_sub (&ray, pos, &mapvals.viewpoint);
  picman_vector3_normalize (&ray);

  /* Check for intersection. This is a quasi ray-tracer. */
  /* =================================================== */

  if (plane_intersect (&ray, &mapvals.viewpoint, &spos, &vx, &vy) == TRUE)
    {
      color = get_image_color (vx, vy, &inside);

      if (color.a != 0.0 && inside == TRUE &&
          mapvals.lightsource.type != NO_LIGHT)
        {
          /* Compute shading at this point */
          /* ============================= */

          color = phong_shade (&spos,
                               &mapvals.viewpoint,
                               &mapvals.normal,
                               &color,
                               &mapvals.lightsource.color,
                               mapvals.lightsource.type);

          picman_rgb_clamp (&color);
        }
    }

  if (mapvals.transparent_background == FALSE && color.a < 1.0)
    {
      picman_rgb_composite (&color, &background,
                          PICMAN_RGB_COMPOSITE_BEHIND);
    }

  return color;
}

/***********************************************************************/
/* Given the NorthPole, Equator and a third vector (normal) compute    */
/* the conversion from spherical oordinates to image space coordinates */
/***********************************************************************/

static void
sphere_to_image (PicmanVector3 *normal,
                 gdouble     *u,
                 gdouble     *v)
{
  static gdouble      alpha, fac;
  static PicmanVector3  cross_prod;

  alpha = acos (-picman_vector3_inner_product (&mapvals.secondaxis, normal));

  *v = alpha / G_PI;

  if (*v == 0.0 || *v == 1.0)
    {
      *u = 0.0;
    }
  else
    {
      fac = (picman_vector3_inner_product (&mapvals.firstaxis, normal) /
             sin (alpha));

      /* Make sure that we map to -1.0..1.0 (take care of rounding errors) */
      /* ================================================================= */

      fac = CLAMP (fac, -1.0, 1.0);

      *u = acos (fac) / (2.0 * G_PI);

      cross_prod = picman_vector3_cross_product (&mapvals.secondaxis,
                                               &mapvals.firstaxis);

      if (picman_vector3_inner_product (&cross_prod, normal) < 0.0)
        *u = 1.0 - *u;
    }
}

/***************************************************/
/* Compute intersection point with sphere (if any) */
/***************************************************/

static gint
sphere_intersect (PicmanVector3 *dir,
                  PicmanVector3 *viewp,
                  PicmanVector3 *spos1,
                  PicmanVector3 *spos2)
{
  static gdouble      alpha, beta, tau, s1, s2, tmp;
  static PicmanVector3  t;

  picman_vector3_sub (&t, &mapvals.position, viewp);

  alpha = picman_vector3_inner_product (dir, &t);
  beta  = picman_vector3_inner_product (&t, &t);

  tau = alpha * alpha - beta + mapvals.radius * mapvals.radius;

  if (tau >= 0.0)
    {
      tau = sqrt (tau);
      s1 = alpha + tau;
      s2 = alpha - tau;

      if (s2 < s1)
        {
          tmp = s1;
          s1 = s2;
          s2 = tmp;
        }

      spos1->x = viewp->x + s1 * dir->x;
      spos1->y = viewp->y + s1 * dir->y;
      spos1->z = viewp->z + s1 * dir->z;
      spos2->x = viewp->x + s2 * dir->x;
      spos2->y = viewp->y + s2 * dir->y;
      spos2->z = viewp->z + s2 * dir->z;

      return TRUE;
    }

  return FALSE;
}

/*****************************************************************************
 * These routines computes the color of the surface
 * of the sphere at a given point
 *****************************************************************************/

PicmanRGB
get_ray_color_sphere (PicmanVector3 *pos)
{
  PicmanRGB color = background;

  static PicmanRGB      color2;
  static gint         inside = FALSE;
  static PicmanVector3  normal, ray, spos1, spos2;
  static gdouble      vx, vy;

  /* Check if ray is within the bounding box */
  /* ======================================= */

  if (pos->x<bx1 || pos->x>bx2 || pos->y<by1 || pos->y>by2)
    return color;

  /* Construct a line from our VP to the point */
  /* ========================================= */

  picman_vector3_sub (&ray, pos, &mapvals.viewpoint);
  picman_vector3_normalize (&ray);

  /* Check for intersection. This is a quasi ray-tracer. */
  /* =================================================== */

  if (sphere_intersect (&ray, &mapvals.viewpoint, &spos1, &spos2) == TRUE)
    {
      /* Compute spherical to rectangular mapping */
      /* ======================================== */

      picman_vector3_sub (&normal, &spos1, &mapvals.position);
      picman_vector3_normalize (&normal);
      sphere_to_image (&normal, &vx, &vy);
      color = get_image_color (vx, vy, &inside);

      /* Check for total transparency... */
      /* =============================== */

      if (color.a < 1.0)
        {
          /* Hey, we can see  through here!      */
          /* Lets see what's on the other side.. */
          /* =================================== */

          color = phong_shade (&spos1,
                               &mapvals.viewpoint,
                               &normal,
                               &color,
                               &mapvals.lightsource.color,
                               mapvals.lightsource.type);

          picman_rgb_clamp (&color);

          picman_vector3_sub (&normal, &spos2, &mapvals.position);
          picman_vector3_normalize (&normal);
          sphere_to_image (&normal, &vx, &vy);
          color2 = get_image_color (vx, vy, &inside);

          /* Make the normal point inwards */
          /* ============================= */

          picman_vector3_mul (&normal, -1.0);

          color2 = phong_shade (&spos2,
                                &mapvals.viewpoint,
                                &normal,
                                &color2,
                                &mapvals.lightsource.color,
                                mapvals.lightsource.type);

          picman_rgb_clamp (&color2);

          /* Compute a mix of the first and second colors */
          /* ============================================ */

          picman_rgb_composite (&color, &color2, PICMAN_RGB_COMPOSITE_NORMAL);
          picman_rgb_clamp (&color);
        }
      else if (color.a != 0.0 &&
               inside == TRUE &&
               mapvals.lightsource.type != NO_LIGHT)
        {
          /* Compute shading at this point */
          /* ============================= */

          color = phong_shade (&spos1,
                               &mapvals.viewpoint,
                               &normal,
                               &color,
                               &mapvals.lightsource.color,
                               mapvals.lightsource.type);

          picman_rgb_clamp (&color);
        }
    }

  if (mapvals.transparent_background == FALSE && color.a < 1.0)
    {
      picman_rgb_composite (&color, &background,
                          PICMAN_RGB_COMPOSITE_BEHIND);
    }

  return color;
}

/***************************************************/
/* Transform the corners of the bounding box to 2D */
/***************************************************/

void
compute_bounding_box (void)
{
  PicmanVector3  p1, p2;
  gdouble      t;
  PicmanVector3  dir;

  p1 = mapvals.position;
  p1.x -= (mapvals.radius + 0.01);
  p1.y -= (mapvals.radius + 0.01);

  p2 = mapvals.position;
  p2.x += (mapvals.radius + 0.01);
  p2.y += (mapvals.radius + 0.01);

  picman_vector3_sub (&dir, &p1, &mapvals.viewpoint);
  picman_vector3_normalize (&dir);

  if (dir.z != 0.0)
    {
      t = (-1.0 * mapvals.viewpoint.z) / dir.z;
      p1.x = (mapvals.viewpoint.x + t * dir.x);
      p1.y = (mapvals.viewpoint.y + t * dir.y);
    }

  picman_vector3_sub (&dir, &p2, &mapvals.viewpoint);
  picman_vector3_normalize (&dir);

  if (dir.z != 0.0)
    {
      t = (-1.0 * mapvals.viewpoint.z) / dir.z;
      p2.x = (mapvals.viewpoint.x + t * dir.x);
      p2.y = (mapvals.viewpoint.y + t * dir.y);
    }

  bx1 = p1.x;
  by1 = p1.y;
  bx2 = p2.x;
  by2 = p2.y;
}

/* These two were taken from the Mesa source. Mesa is written   */
/* and is (C) by Brian Paul. vecmulmat() performs a post-mul by */
/* a 4x4 matrix to a 1x4(3) vector. rotmat() creates a matrix   */
/* that by post-mul will rotate a 1x4(3) vector the given angle */
/* about the given axis.                                        */
/* ============================================================ */

void
vecmulmat (PicmanVector3 *u,
           PicmanVector3 *v,
           gfloat       m[16])
{
  gfloat v0=v->x, v1=v->y, v2=v->z;
#define M(row,col)  m[col*4+row]
  u->x = v0 * M(0,0) + v1 * M(1,0) + v2 * M(2,0) + M(3,0);
  u->y = v0 * M(0,1) + v1 * M(1,1) + v2 * M(2,1) + M(3,1);
  u->z = v0 * M(0,2) + v1 * M(1,2) + v2 * M(2,2) + M(3,2);
#undef M
}

void
rotatemat (gfloat       angle,
           PicmanVector3 *v,
           gfloat       m[16])
{
  /* This function contributed by Erich Boleyn (erich@uruk.org) */
  gfloat mag, s, c;
  gfloat xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
  gfloat IdentityMat[16];
  gint   cnt;

  s = sin (angle * (G_PI / 180.0));
  c = cos (angle * (G_PI / 180.0));

  mag = sqrt (v->x*v->x + v->y*v->y + v->z*v->z);

  if (mag == 0.0)
    {
      /* generate an identity matrix and return */

      for (cnt = 0; cnt < 16; cnt++)
        IdentityMat[cnt] = 0.0;

      IdentityMat[0]  = 1.0;
      IdentityMat[5]  = 1.0;
      IdentityMat[10] = 1.0;
      IdentityMat[15] = 1.0;

      memcpy (m, IdentityMat, sizeof (gfloat) * 16);
      return;
   }

  v->x /= mag;
  v->y /= mag;
  v->z /= mag;

#define M(row,col)  m[col*4+row]

  xx = v->x * v->x;
  yy = v->y * v->y;
  zz = v->z * v->z;
  xy = v->x * v->y;
  yz = v->y * v->z;
  zx = v->z * v->x;
  xs = v->x * s;
  ys = v->y * s;
  zs = v->z * s;
  one_c = 1.0F - c;

  M(0,0) = (one_c * xx) + c;
  M(0,1) = (one_c * xy) - zs;
  M(0,2) = (one_c * zx) + ys;
  M(0,3) = 0.0F;

  M(1,0) = (one_c * xy) + zs;
  M(1,1) = (one_c * yy) + c;
  M(1,2) = (one_c * yz) - xs;
  M(1,3) = 0.0F;

  M(2,0) = (one_c * zx) - ys;
  M(2,1) = (one_c * yz) + xs;
  M(2,2) = (one_c * zz) + c;
  M(2,3) = 0.0F;

  M(3,0) = 0.0F;
  M(3,1) = 0.0F;
  M(3,2) = 0.0F;
  M(3,3) = 1.0F;

#undef M
}

/* Transpose the matrix m. If m is orthogonal (like a rotation matrix), */
/* this is equal to the inverse of the matrix.                          */
/* ==================================================================== */

void
transpose_mat (gfloat m[16])
{
  gint    i, j;
  gfloat  t;

  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < i; j++)
        {
          t = m[j*4+i];
          m[j*4+i] = m[i*4+j];
          m[i*4+j] = t;
        }
    }
}

/* Compute the matrix product c=a*b */
/* ================================ */

void
matmul (gfloat a[16],
        gfloat b[16],
        gfloat c[16])
{
  gint   i, j, k;
  gfloat value;

#define A(row,col)  a[col*4+row]
#define B(row,col)  b[col*4+row]
#define C(row,col)  c[col*4+row]

  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < 4; j++)
        {
          value = 0.0;

          for (k = 0; k < 4; k++)
            value += A(i,k) * B(k,j);

          C(i,j) = value;
        }
    }

#undef A
#undef B
#undef C
}

void
ident_mat (gfloat m[16])
{
  gint  i, j;

#define M(row,col)  m[col*4+row]

  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < 4; j++)
        {
          if (i == j)
            M(i,j) = 1.0;
          else
            M(i,j) = 0.0;
        }
    }

#undef M
}

static gboolean
intersect_rect (gdouble            u,
                gdouble            v,
                gdouble            w,
                PicmanVector3        viewp,
                PicmanVector3        dir,
                FaceIntersectInfo *face_info)
{
  gboolean result = FALSE;
  gdouble  u2, v2;

  if (dir.z!=0.0)
    {
      u2 = u / 2.0;
      v2 = v / 2.0;

      face_info->t = (w-viewp.z) / dir.z;
      face_info->s.x = viewp.x + face_info->t * dir.x;
      face_info->s.y = viewp.y + face_info->t * dir.y;
      face_info->s.z = w;

      if (face_info->s.x >= -u2 && face_info->s.x <= u2 &&
          face_info->s.y >= -v2 && face_info->s.y <= v2)
        {
          face_info->u = (face_info->s.x + u2) / u;
          face_info->v = (face_info->s.y + v2) / v;
          result = TRUE;
        }
    }

  return result;
}

static gboolean
intersect_box (PicmanVector3        scale,
               PicmanVector3        viewp,
               PicmanVector3        dir,
               FaceIntersectInfo *face_intersect)
{
  PicmanVector3        v, d, tmp, axis[3];
  FaceIntersectInfo  face_tmp;
  gboolean           result = FALSE;
  gfloat             m[16];
  gint               i = 0;

  picman_vector3_set (&axis[0], 1.0, 0.0, 0.0);
  picman_vector3_set (&axis[1], 0.0, 1.0, 0.0);
  picman_vector3_set (&axis[2], 0.0, 0.0, 1.0);

  /* Front side */
  /* ========== */

  if (intersect_rect (scale.x, scale.y, scale.z / 2.0,
                      viewp, dir, &face_intersect[i]) == TRUE)
    {
      face_intersect[i].face = 0;
      picman_vector3_set (&face_intersect[i++].n, 0.0, 0.0, 1.0);
      result = TRUE;
    }

  /* Back side */
  /* ========= */

  if (intersect_rect (scale.x, scale.y, -scale.z / 2.0,
                      viewp, dir, &face_intersect[i]) == TRUE)
    {
      face_intersect[i].face = 1;
      face_intersect[i].u = 1.0 - face_intersect[i].u;
      picman_vector3_set (&face_intersect[i++].n, 0.0, 0.0, -1.0);
      result = TRUE;
    }

  /* Check if we've found the two possible intersection points */
  /* ========================================================= */

  if (i < 2)
    {
      /* Top: Rotate viewpoint and direction into rectangle's local coordinate system */
      /* ============================================================================ */

      rotatemat (90, &axis[0], m);
      vecmulmat (&v, &viewp, m);
      vecmulmat (&d, &dir, m);

      if (intersect_rect (scale.x, scale.z, scale.y / 2.0,
                          v, d, &face_intersect[i]) == TRUE)
        {
          face_intersect[i].face = 2;

          transpose_mat (m);
          vecmulmat(&tmp, &face_intersect[i].s, m);
          face_intersect[i].s = tmp;

          picman_vector3_set (&face_intersect[i++].n, 0.0, -1.0, 0.0);
          result = TRUE;
        }
    }

  /* Check if we've found the two possible intersection points */
  /* ========================================================= */

  if (i < 2)
    {
      /* Bottom: Rotate viewpoint and direction into rectangle's local coordinate system */
      /* =============================================================================== */

      rotatemat (90, &axis[0], m);
      vecmulmat (&v, &viewp, m);
      vecmulmat (&d, &dir, m);

      if (intersect_rect (scale.x, scale.z, -scale.y / 2.0,
                          v, d, &face_intersect[i]) == TRUE)
        {
          face_intersect[i].face = 3;

          transpose_mat (m);

          vecmulmat (&tmp, &face_intersect[i].s, m);
          face_intersect[i].s = tmp;

          face_intersect[i].v = 1.0 - face_intersect[i].v;

          picman_vector3_set (&face_intersect[i++].n, 0.0, 1.0, 0.0);

          result = TRUE;
        }
    }

  /* Check if we've found the two possible intersection points */
  /* ========================================================= */

  if (i < 2)
    {
      /* Left side: Rotate viewpoint and direction into rectangle's local coordinate system */
      /* ================================================================================== */

      rotatemat (90, &axis[1], m);
      vecmulmat (&v, &viewp, m);
      vecmulmat (&d, &dir, m);

      if (intersect_rect (scale.z, scale.y, scale.x / 2.0,
                          v, d, &face_intersect[i]) == TRUE)
        {
          face_intersect[i].face = 4;

          transpose_mat (m);
          vecmulmat (&tmp, &face_intersect[i].s, m);
          face_intersect[i].s = tmp;

          picman_vector3_set (&face_intersect[i++].n, 1.0, 0.0, 0.0);
          result = TRUE;
        }
    }

  /* Check if we've found the two possible intersection points */
  /* ========================================================= */

  if (i < 2)
    {
      /* Right side: Rotate viewpoint and direction into rectangle's local coordinate system */
      /* =================================================================================== */

      rotatemat (90, &axis[1], m);
      vecmulmat (&v, &viewp, m);
      vecmulmat (&d, &dir, m);

      if (intersect_rect (scale.z, scale.y, -scale.x / 2.0,
                          v, d, &face_intersect[i]) == TRUE)
        {
          face_intersect[i].face = 5;

          transpose_mat (m);
          vecmulmat (&tmp, &face_intersect[i].s, m);

          face_intersect[i].u = 1.0 - face_intersect[i].u;

          picman_vector3_set (&face_intersect[i++].n, -1.0, 0.0, 0.0);
          result = TRUE;
        }
    }

  /* Sort intersection points */
  /* ======================== */

  if (face_intersect[0].t > face_intersect[1].t)
    {
      face_tmp = face_intersect[0];
      face_intersect[0] = face_intersect[1];
      face_intersect[1] = face_tmp;
    }

  return result;
}

PicmanRGB
get_ray_color_box (PicmanVector3 *pos)
{
  PicmanVector3        lvp, ldir, vp, p, dir, ns, nn;
  PicmanRGB             color, color2;
  gfloat             m[16];
  gint               i;
  FaceIntersectInfo  face_intersect[2];

  color = background;
  vp = mapvals.viewpoint;
  p = *pos;

  /* Translate viewpoint so that the box has its origin */
  /* at its lower left corner.                          */
  /* ================================================== */

  vp.x = vp.x - mapvals.position.x;
  vp.y = vp.y - mapvals.position.y;
  vp.z = vp.z - mapvals.position.z;

  p.x = p.x - mapvals.position.x;
  p.y = p.y - mapvals.position.y;
  p.z = p.z - mapvals.position.z;

  /* Compute direction */
  /* ================= */

  picman_vector3_sub (&dir, &p, &vp);
  picman_vector3_normalize (&dir);

  /* Compute inverse of rotation matrix and apply it to   */
  /* the viewpoint and direction. This transforms the     */
  /* observer into the local coordinate system of the box */
  /* ==================================================== */

  memcpy (m, rotmat, sizeof (gfloat) * 16);

  transpose_mat (m);

  vecmulmat (&lvp, &vp, m);
  vecmulmat (&ldir, &dir, m);

  /* Ok. Now the observer is in the space where the box is located */
  /* with its lower left corner at the origin and its axis aligned */
  /* to the cartesian basis. Check if the transformed ray hits it. */
  /* ============================================================= */

  face_intersect[0].t = 1000000.0;
  face_intersect[1].t = 1000000.0;

  if (intersect_box (mapvals.scale, lvp, ldir, face_intersect) == TRUE)
    {
      /* We've hit the box. Transform the hit points and */
      /* normals back into the world coordinate system   */
      /* =============================================== */

      for (i = 0; i < 2; i++)
        {
          vecmulmat (&ns, &face_intersect[i].s, rotmat);
          vecmulmat (&nn, &face_intersect[i].n, rotmat);

          ns.x = ns.x + mapvals.position.x;
          ns.y = ns.y + mapvals.position.y;
          ns.z = ns.z + mapvals.position.z;

          face_intersect[i].s = ns;
          face_intersect[i].n = nn;
        }

      color = get_box_image_color (face_intersect[0].face,
                                   face_intersect[0].u,
                                   face_intersect[0].v);

      /* Check for total transparency... */
      /* =============================== */

      if (color.a < 1.0)
        {
          /* Hey, we can see  through here!      */
          /* Lets see what's on the other side.. */
          /* =================================== */

          color = phong_shade (&face_intersect[0].s,
                               &mapvals.viewpoint,
                               &face_intersect[0].n,
                               &color,
                               &mapvals.lightsource.color,
                               mapvals.lightsource.type);

          picman_rgb_clamp (&color);

          color2 = get_box_image_color (face_intersect[1].face,
                                        face_intersect[1].u,
                                        face_intersect[1].v);

          /* Make the normal point inwards */
          /* ============================= */

          picman_vector3_mul (&face_intersect[1].n, -1.0);

          color2 = phong_shade (&face_intersect[1].s,
                                &mapvals.viewpoint,
                                &face_intersect[1].n,
                                &color2,
                                &mapvals.lightsource.color,
                                mapvals.lightsource.type);

          picman_rgb_clamp (&color2);

          if (mapvals.transparent_background == FALSE && color2.a < 1.0)
            {
              picman_rgb_composite (&color2, &background,
                                  PICMAN_RGB_COMPOSITE_BEHIND);
            }

          /* Compute a mix of the first and second colors */
          /* ============================================ */

          picman_rgb_composite (&color, &color2, PICMAN_RGB_COMPOSITE_NORMAL);
          picman_rgb_clamp (&color);
        }
      else if (color.a != 0.0 && mapvals.lightsource.type != NO_LIGHT)
        {
          color = phong_shade (&face_intersect[0].s,
                               &mapvals.viewpoint,
                               &face_intersect[0].n,
                               &color,
                               &mapvals.lightsource.color,
                               mapvals.lightsource.type);

          picman_rgb_clamp (&color);
        }
    }
  else
    {
      if (mapvals.transparent_background == TRUE)
        picman_rgb_set_alpha (&color, 0.0);
    }

  return color;
}

static gboolean
intersect_circle (PicmanVector3        vp,
                  PicmanVector3        dir,
                  gdouble            w,
                  FaceIntersectInfo *face_info)
{
  gboolean result = FALSE;
  gdouble  r, d;

#define sqr(a) (a*a)

  if (dir.y != 0.0)
    {
      face_info->t = (w-vp.y)/dir.y;
      face_info->s.x = vp.x + face_info->t*dir.x;
      face_info->s.y = w;
      face_info->s.z = vp.z + face_info->t*dir.z;

      r = sqrt (sqr (face_info->s.x) + sqr (face_info->s.z));

      if (r <= mapvals.cylinder_radius)
        {
          d = 2.0 * mapvals.cylinder_radius;
          face_info->u = (face_info->s.x + mapvals.cylinder_radius) / d;
          face_info->v = (face_info->s.z + mapvals.cylinder_radius) / d;
          result = TRUE;
        }
    }

#undef sqr

  return result;
}

static gboolean
intersect_cylinder (PicmanVector3        vp,
                    PicmanVector3        dir,
                    FaceIntersectInfo *face_intersect)
{
  gdouble  a, b, c, d, e, f, tmp, l;
  gboolean result = FALSE;
  gint     i;

#define sqr(a) (a*a)

  a = sqr (dir.x) + sqr (dir.z);
  b = 2.0 * (vp.x * dir.x + vp.z * dir.z);
  c = sqr (vp.x) + sqr (vp.z) - sqr (mapvals.cylinder_radius);

  d = sqr (b) - 4.0 * a * c;

  if (d >= 0.0)
    {
      e = sqrt (d);
      f = 2.0 * a;

      if (f != 0.0)
        {
          result = TRUE;

          face_intersect[0].t = (-b+e)/f;
          face_intersect[1].t = (-b-e)/f;

          if (face_intersect[0].t>face_intersect[1].t)
            {
              tmp = face_intersect[0].t;
              face_intersect[0].t = face_intersect[1].t;
              face_intersect[1].t = tmp;
            }

          for (i = 0; i < 2; i++)
            {
              face_intersect[i].s.x = vp.x + face_intersect[i].t * dir.x;
              face_intersect[i].s.y = vp.y + face_intersect[i].t * dir.y;
              face_intersect[i].s.z = vp.z + face_intersect[i].t * dir.z;

              face_intersect[i].n = face_intersect[i].s;
              face_intersect[i].n.y = 0.0;
              picman_vector3_normalize(&face_intersect[i].n);

              l = mapvals.cylinder_length/2.0;

              face_intersect[i].u = (atan2(face_intersect[i].s.x,face_intersect[i].s.z)+G_PI)/(2.0*G_PI);
              face_intersect[i].v = (face_intersect[i].s.y+l)/mapvals.cylinder_length;

              /* Mark hitpoint as on the cylinder hull */
              /* ===================================== */

              face_intersect[i].face = 0;

              /* Check if we're completely off the cylinder axis */
              /* =============================================== */

              if (face_intersect[i].s.y>l || face_intersect[i].s.y<-l)
                {
                  /* Check if we've hit a cap */
                  /* ======================== */

                  if (face_intersect[i].s.y>l)
                    {
                      if (intersect_circle(vp,dir,l,&face_intersect[i])==FALSE)
                        result = FALSE;
                      else
                        {
                          face_intersect[i].face = 2;
                          face_intersect[i].v = 1 - face_intersect[i].v;
                          picman_vector3_set(&face_intersect[i].n, 0.0, 1.0, 0.0);
                        }
                    }
                  else
                    {
                      if (intersect_circle(vp,dir,-l,&face_intersect[i])==FALSE)
                        result = FALSE;
                      else
                        {
                          face_intersect[i].face = 1;
                          picman_vector3_set(&face_intersect[i].n, 0.0, -1.0, 0.0);
                        }
                    }
                }
            }
        }
    }

#undef sqr

  return result;
}

static PicmanRGB
get_cylinder_color (gint    face,
                    gdouble u,
                    gdouble v)
{
  PicmanRGB  color;
  gint    inside;

  if (face == 0)
    color = get_image_color (u, v, &inside);
  else
    color = get_cylinder_image_color (face - 1, u, v);

  return color;
}

PicmanRGB
get_ray_color_cylinder (PicmanVector3 *pos)
{
  PicmanVector3       lvp, ldir, vp, p, dir, ns, nn;
  PicmanRGB            color, color2;
  gfloat            m[16];
  gint              i;
  FaceIntersectInfo face_intersect[2];

  color = background;
  vp = mapvals.viewpoint;
  p = *pos;

  vp.x = vp.x - mapvals.position.x;
  vp.y = vp.y - mapvals.position.y;
  vp.z = vp.z - mapvals.position.z;

  p.x = p.x - mapvals.position.x;
  p.y = p.y - mapvals.position.y;
  p.z = p.z - mapvals.position.z;

  /* Compute direction */
  /* ================= */

  picman_vector3_sub (&dir, &p, &vp);
  picman_vector3_normalize (&dir);

  /* Compute inverse of rotation matrix and apply it to   */
  /* the viewpoint and direction. This transforms the     */
  /* observer into the local coordinate system of the box */
  /* ==================================================== */

  memcpy (m, rotmat, sizeof (gfloat) * 16);

  transpose_mat (m);

  vecmulmat (&lvp, &vp, m);
  vecmulmat (&ldir, &dir, m);

  if (intersect_cylinder (lvp, ldir, face_intersect) == TRUE)
    {
      /* We've hit the cylinder. Transform the hit points and */
      /* normals back into the world coordinate system        */
      /* ==================================================== */

      for (i = 0; i < 2; i++)
        {
          vecmulmat (&ns, &face_intersect[i].s, rotmat);
          vecmulmat (&nn, &face_intersect[i].n, rotmat);

          ns.x = ns.x + mapvals.position.x;
          ns.y = ns.y + mapvals.position.y;
          ns.z = ns.z + mapvals.position.z;

          face_intersect[i].s = ns;
          face_intersect[i].n = nn;
        }

      color = get_cylinder_color (face_intersect[0].face,
                                  face_intersect[0].u,
                                  face_intersect[0].v);

      /* Check for transparency... */
      /* ========================= */

      if (color.a < 1.0)
        {
          /* Hey, we can see  through here!      */
          /* Lets see what's on the other side.. */
          /* =================================== */

          color = phong_shade (&face_intersect[0].s,
                               &mapvals.viewpoint,
                               &face_intersect[0].n,
                               &color,
                               &mapvals.lightsource.color,
                               mapvals.lightsource.type);

          picman_rgb_clamp (&color);

          color2 = get_cylinder_color (face_intersect[1].face,
                                       face_intersect[1].u,
                                       face_intersect[1].v);

          /* Make the normal point inwards */
          /* ============================= */

          picman_vector3_mul (&face_intersect[1].n, -1.0);

          color2 = phong_shade (&face_intersect[1].s,
                                &mapvals.viewpoint,
                                &face_intersect[1].n,
                                &color2,
                                &mapvals.lightsource.color,
                                mapvals.lightsource.type);

          picman_rgb_clamp (&color2);

          if (mapvals.transparent_background == FALSE && color2.a < 1.0)
            {
              picman_rgb_composite (&color2, &background,
                                  PICMAN_RGB_COMPOSITE_BEHIND);
            }

          /* Compute a mix of the first and second colors */
          /* ============================================ */

          picman_rgb_composite (&color, &color2, PICMAN_RGB_COMPOSITE_NORMAL);
          picman_rgb_clamp (&color);
        }
      else if (color.a != 0.0 && mapvals.lightsource.type != NO_LIGHT)
        {
          color = phong_shade (&face_intersect[0].s,
                               &mapvals.viewpoint,
                               &face_intersect[0].n,
                               &color,
                               &mapvals.lightsource.color,
                               mapvals.lightsource.type);

          picman_rgb_clamp (&color);
        }
    }
  else
    {
      if (mapvals.transparent_background == TRUE)
        picman_rgb_set_alpha (&color, 0.0);
    }

  return color;
}
