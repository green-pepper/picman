/******************************************************/
/* Apply mapping and shading on the whole input image */
/******************************************************/

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "map-object-main.h"
#include "map-object-image.h"
#include "map-object-shade.h"
#include "map-object-apply.h"

#include "libpicman/stdplugins-intl.h"


/*************/
/* Main loop */
/*************/

gdouble       imat[4][4];
gfloat        rotmat[16];
static gfloat a[16], b[16];

void
init_compute (void)
{
  gint i;

  switch (mapvals.maptype)
    {
      case MAP_SPHERE:

        /* Rotate the equator/northpole axis */
        /* ================================= */

        picman_vector3_set (&mapvals.firstaxis,  0.0, 0.0, -1.0);
        picman_vector3_set (&mapvals.secondaxis, 0.0, 1.0,  0.0);

        picman_vector3_rotate (&mapvals.firstaxis,
                             picman_deg_to_rad (mapvals.alpha),
                             picman_deg_to_rad (mapvals.beta),
                             picman_deg_to_rad (mapvals.gamma));
        picman_vector3_rotate (&mapvals.secondaxis,
                             picman_deg_to_rad (mapvals.alpha),
                             picman_deg_to_rad (mapvals.beta),
                             picman_deg_to_rad (mapvals.gamma));

        /* Compute the 2D bounding box of the sphere spanned by the axis */
        /* ============================================================= */

        compute_bounding_box ();

        get_ray_color = get_ray_color_sphere;

        break;

      case MAP_PLANE:

        /* Rotate the plane axis */
        /* ===================== */

        picman_vector3_set (&mapvals.firstaxis,  1.0, 0.0, 0.0);
        picman_vector3_set (&mapvals.secondaxis, 0.0, 1.0, 0.0);
        picman_vector3_set (&mapvals.normal,     0.0, 0.0, 1.0);

        picman_vector3_rotate (&mapvals.firstaxis,
                             picman_deg_to_rad (mapvals.alpha),
                             picman_deg_to_rad (mapvals.beta),
                             picman_deg_to_rad (mapvals.gamma));
        picman_vector3_rotate (&mapvals.secondaxis,
                             picman_deg_to_rad (mapvals.alpha),
                             picman_deg_to_rad (mapvals.beta),
                             picman_deg_to_rad (mapvals.gamma));

        mapvals.normal = picman_vector3_cross_product (&mapvals.firstaxis,
                                                     &mapvals.secondaxis);

        if (mapvals.normal.z < 0.0)
          picman_vector3_mul (&mapvals.normal, -1.0);

        /* Initialize intersection matrix */
        /* ============================== */

        imat[0][1] = -mapvals.firstaxis.x;
        imat[1][1] = -mapvals.firstaxis.y;
        imat[2][1] = -mapvals.firstaxis.z;

        imat[0][2] = -mapvals.secondaxis.x;
        imat[1][2] = -mapvals.secondaxis.y;
        imat[2][2] = -mapvals.secondaxis.z;

        imat[0][3] = mapvals.position.x - mapvals.viewpoint.x;
        imat[1][3] = mapvals.position.y - mapvals.viewpoint.y;
        imat[2][3] = mapvals.position.z - mapvals.viewpoint.z;

        get_ray_color = get_ray_color_plane;

        break;

      case MAP_BOX:
        get_ray_color = get_ray_color_box;

        picman_vector3_set (&mapvals.firstaxis,  1.0, 0.0, 0.0);
        picman_vector3_set (&mapvals.secondaxis, 0.0, 1.0, 0.0);
        picman_vector3_set (&mapvals.normal,     0.0, 0.0, 1.0);

        ident_mat (rotmat);

        rotatemat (mapvals.alpha, &mapvals.firstaxis, a);

        matmul (a, rotmat, b);

        memcpy (rotmat, b, sizeof (gfloat) * 16);

        rotatemat (mapvals.beta, &mapvals.secondaxis, a);
        matmul (a, rotmat, b);

        memcpy (rotmat, b, sizeof (gfloat) * 16);

        rotatemat (mapvals.gamma, &mapvals.normal, a);
        matmul (a, rotmat, b);

        memcpy (rotmat, b, sizeof (gfloat) * 16);

        /* Set up pixel regions for the box face images */
        /* ============================================ */

        for (i = 0; i < 6; i++)
          {
            box_drawables[i] = picman_drawable_get (mapvals.boxmap_id[i]);

            picman_pixel_rgn_init (&box_regions[i], box_drawables[i],
                                 0, 0,
                                 box_drawables[i]->width,
                                 box_drawables[i]->height,
                                 FALSE, FALSE);
          }

        break;

      case MAP_CYLINDER:
        get_ray_color = get_ray_color_cylinder;

        picman_vector3_set (&mapvals.firstaxis,  1.0, 0.0, 0.0);
        picman_vector3_set (&mapvals.secondaxis, 0.0, 1.0, 0.0);
        picman_vector3_set (&mapvals.normal,     0.0, 0.0, 1.0);

        ident_mat (rotmat);

        rotatemat (mapvals.alpha, &mapvals.firstaxis, a);

        matmul (a, rotmat, b);

        memcpy (rotmat, b, sizeof (gfloat) * 16);

        rotatemat (mapvals.beta, &mapvals.secondaxis, a);
        matmul (a, rotmat, b);

        memcpy (rotmat, b, sizeof (gfloat) * 16);

        rotatemat (mapvals.gamma, &mapvals.normal, a);
        matmul (a, rotmat, b);

        memcpy (rotmat, b, sizeof (gfloat) * 16);

        /* Set up pixel regions for the cylinder cap images */
        /* ================================================ */

        for (i = 0; i < 2; i++)
          {
            cylinder_drawables[i] =
              picman_drawable_get (mapvals.cylindermap_id[i]);

            picman_pixel_rgn_init (&cylinder_regions[i], cylinder_drawables[i],
                                 0, 0,
                                 cylinder_drawables[i]->width,
                                 cylinder_drawables[i]->height,
                                 FALSE, FALSE);
          }

        break;
    }

  max_depth = (gint) mapvals.maxdepth;
}

static void
render (gdouble   x,
        gdouble   y,
        PicmanRGB  *col,
        gpointer  data)
{
  PicmanVector3 pos;

  pos.x = x / (gdouble) width;
  pos.y = y / (gdouble) height;
  pos.z = 0.0;

  *col = get_ray_color (&pos);
}

static void
show_progress (gint     min,
               gint     max,
               gint     curr,
               gpointer data)
{
  picman_progress_update ((gdouble) curr / (gdouble) max);
}

/**************************************************/
/* Performs map-to-sphere on the whole input image */
/* and updates or creates a new PICMAN image.       */
/**************************************************/

void
compute_image (void)
{
  gint         xcount, ycount;
  PicmanRGB      color;
  glong        progress_counter = 0;
  PicmanVector3  p;
  gint32       new_image_id = -1;
  gint32       new_layer_id = -1;
  gboolean     insert_layer = FALSE;

  init_compute ();

  if (mapvals.create_new_image)
    {
      new_image_id = picman_image_new (width, height, PICMAN_RGB);
    }
  else
    {
      new_image_id = image_id;
    }

  picman_image_undo_group_start (new_image_id);

  if (mapvals.create_new_image ||
      mapvals.create_new_layer ||
      (mapvals.transparent_background &&
       output_drawable->bpp != 4))
    {
      gchar *layername[] = {_("Map to plane"), _("Map to sphere"), _("Map to box"),
                            _("Map to cylinder"), _("Background")};

      new_layer_id = picman_layer_new (new_image_id, layername[mapvals.create_new_image ? 4 :
                                                             mapvals.maptype],
                                     width, height,
                                     mapvals.transparent_background ? PICMAN_RGBA_IMAGE
                                                                    : PICMAN_RGB_IMAGE,
                                     100.0,
                                     PICMAN_NORMAL_MODE);

      insert_layer = TRUE;
      output_drawable = picman_drawable_get (new_layer_id);
    }

  picman_pixel_rgn_init (&dest_region, output_drawable,
                       0, 0, width, height, TRUE, TRUE);

  switch (mapvals.maptype)
    {
      case MAP_PLANE:
        picman_progress_init (_("Map to plane"));
        break;
      case MAP_SPHERE:
        picman_progress_init (_("Map to sphere"));
        break;
      case MAP_BOX:
        picman_progress_init (_("Map to box"));
        break;
      case MAP_CYLINDER:
        picman_progress_init (_("Map to cylinder"));
        break;
    }

  if (mapvals.antialiasing == FALSE)
    {
      for (ycount = 0; ycount < height; ycount++)
        {
          for (xcount = 0; xcount < width; xcount++)
            {
              p = int_to_pos (xcount, ycount);
              color = (* get_ray_color) (&p);
              poke (xcount, ycount, &color, NULL);

              if ((progress_counter++ % width) == 0)
                picman_progress_update ((gdouble) progress_counter /
                                      (gdouble) maxcounter);
            }
        }
    }
  else
    {
      picman_adaptive_supersample_area (0, 0,
                                      width - 1, height - 1,
                                      max_depth,
                                      mapvals.pixeltreshold,
                                      render,
                                      NULL,
                                      poke,
                                      NULL,
                                      show_progress,
                                      NULL);
    }
  picman_progress_update (1.0);

  /* Update the region */
  /* ================= */

  picman_drawable_flush (output_drawable);
  if (insert_layer)
    picman_image_insert_layer (new_image_id, new_layer_id, -1, 0);
  picman_drawable_merge_shadow (output_drawable->drawable_id, TRUE);
  picman_drawable_update (output_drawable->drawable_id, 0, 0, width, height);

  if (new_image_id != image_id)
    {
      picman_display_new (new_image_id);
      picman_displays_flush ();
      picman_drawable_detach (output_drawable);
    }

  picman_image_undo_group_end (new_image_id);
}
