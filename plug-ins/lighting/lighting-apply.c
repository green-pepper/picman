/******************************************************/
/* Apply mapping and shading on the whole input image */
/******************************************************/

#include "config.h"

#include <sys/types.h>

#include <libpicman/picman.h>

#include "lighting-main.h"
#include "lighting-image.h"
#include "lighting-shade.h"
#include "lighting-apply.h"

#include "libpicman/stdplugins-intl.h"


/*************/
/* Main loop */
/*************/

void
compute_image (void)
{
  gint         xcount, ycount;
  PicmanRGB       color;
  glong        progress_counter = 0;
  PicmanVector3  p;
  gint32       new_image_id = -1;
  gint32       new_layer_id = -1;
  gint32       index;
  guchar      *row = NULL;
  guchar       obpp;
  gboolean     has_alpha;
  get_ray_func ray_func;



  if (mapvals.create_new_image == TRUE ||
      (mapvals.transparent_background == TRUE &&
       ! picman_drawable_has_alpha (input_drawable->drawable_id)))
    {
      /* Create a new image */
      /* ================== */

      new_image_id = picman_image_new (width, height, PICMAN_RGB);

      if (mapvals.transparent_background == TRUE)
        {
          /* Add a layer with an alpha channel */
          /* ================================= */

          new_layer_id = picman_layer_new (new_image_id, "Background",
					 width, height,
					 PICMAN_RGBA_IMAGE,
					 100.0,
					 PICMAN_NORMAL_MODE);
        }
      else
        {
          /* Create a "normal" layer */
          /* ======================= */

          new_layer_id = picman_layer_new (new_image_id, "Background",
					 width, height,
					 PICMAN_RGB_IMAGE,
					 100.0,
					 PICMAN_NORMAL_MODE);
        }

      picman_image_insert_layer (new_image_id, new_layer_id, -1, 0);
      output_drawable = picman_drawable_get (new_layer_id);
    }

  if (mapvals.bump_mapped == TRUE && mapvals.bumpmap_id != -1)
    {
      picman_pixel_rgn_init (&bump_region, picman_drawable_get (mapvals.bumpmap_id),
			   0, 0, width, height, FALSE, FALSE);
    }

  precompute_init (width, height);

  if (!mapvals.env_mapped || mapvals.envmap_id == -1)
    {
      ray_func = get_ray_color;
    }
  else
    {
      env_width = picman_drawable_width (mapvals.envmap_id);
      env_height = picman_drawable_height (mapvals.envmap_id);
      picman_pixel_rgn_init (&env_region, picman_drawable_get (mapvals.envmap_id),
			   0, 0, env_width, env_height, FALSE, FALSE);
      ray_func = get_ray_color_ref;
    }

  picman_pixel_rgn_init (&dest_region, output_drawable,
		       0, 0, width, height, TRUE, TRUE);

  obpp = picman_drawable_bpp (output_drawable->drawable_id);
  has_alpha = picman_drawable_has_alpha (output_drawable->drawable_id);

  row = g_new (guchar, obpp * width);

  picman_progress_init (_("Lighting Effects"));

  /* Init the first row */
  if (mapvals.bump_mapped == TRUE && mapvals.bumpmap_id != -1 && height >= 2)
    interpol_row (0, width, 0);

  for (ycount = 0; ycount < height; ycount++)
    {
      if (mapvals.bump_mapped == TRUE && mapvals.bumpmap_id != -1)
	precompute_normals (0, width, ycount);

      index = 0;

      for (xcount = 0; xcount < width; xcount++)
	{
	  p = int_to_pos (xcount, ycount);
	  color = (* ray_func) (&p);

	  row[index++] = (guchar) (color.r * 255.0);
	  row[index++] = (guchar) (color.g * 255.0);
	  row[index++] = (guchar) (color.b * 255.0);

	  if (has_alpha)
	    row[index++] = (guchar) (color.a * 255.0);

	  if ((progress_counter++ % width) == 0)
	    picman_progress_update ((gdouble) progress_counter /
				  (gdouble) maxcounter);
	}

      picman_pixel_rgn_set_row (&dest_region, row, 0, ycount, width);
    }

  picman_progress_update (1.0);

  g_free (row);

  /* Update image */
  /* ============ */

  picman_drawable_flush (output_drawable);
  picman_drawable_merge_shadow (output_drawable->drawable_id, TRUE);
  picman_drawable_update (output_drawable->drawable_id, 0, 0, width, height);

  if (new_image_id!=-1)
    {
      picman_display_new (new_image_id);
      picman_displays_flush ();
      picman_drawable_detach (output_drawable);
    }

}
