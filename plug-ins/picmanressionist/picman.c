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

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include <libpicman/picman.h>

#include "ppmtool.h"
#include "infile.h"
#include "picmanressionist.h"
#include "preview.h"
#include "brush.h"
#include "presets.h"
#include "random.h"
#include "orientmap.h"
#include "size.h"


#include "libpicman/stdplugins-intl.h"

static void query               (void);
static void run                 (const gchar      *name,
                                 gint              nparams,
                                 const PicmanParam  *param,
                                 gint             *nreturn_vals,
                                 PicmanParam       **return_vals);
static void picmanressionist_main (void);


const PicmanPlugInInfo PLUG_IN_INFO = {
        NULL,   /* init_proc */
        NULL,   /* quit_proc */
        query,  /* query_proc */
        run     /* run_proc */
}; /* PLUG_IN_INFO */

static PicmanDrawable *drawable;
static ppm_t         infile =  {0, 0, NULL};
static ppm_t         inalpha = {0, 0, NULL};


void
infile_copy_to_ppm (ppm_t * p)
{
  if (!PPM_IS_INITED (&infile))
    grabarea ();

  ppm_copy (&infile, p);
}

void
infile_copy_alpha_to_ppm (ppm_t * p)
{
  ppm_copy (&inalpha, p);
}

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0) }"    },
    { PICMAN_PDB_IMAGE,    "image",     "Input image"    },
    { PICMAN_PDB_DRAWABLE, "drawable",  "Input drawable" },
    { PICMAN_PDB_STRING,   "preset",    "Preset Name"    },
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Performs various artistic operations"),
                          "Performs various artistic operations on an image",
                          "Vidar Madsen <vidar@prosalg.no>",
                          "Vidar Madsen",
                          PLUG_IN_VERSION,
                          N_("_PICMANressionist..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);
}

static void
picmanressionist_get_data (void)
{
  restore_default_values ();
  picman_get_data (PLUG_IN_PROC, &pcvals);
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[2];
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status;
  gboolean           with_specified_preset;
  gchar             *preset_name = NULL;

  status   = PICMAN_PDB_SUCCESS;
  run_mode = param[0].data.d_int32;
  with_specified_preset = FALSE;

  if (nparams > 3)
    {
      preset_name = param[3].data.d_string;
      if (strcmp (preset_name, ""))
        {
          with_specified_preset = TRUE;
        }
    }

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  *nreturn_vals = 1;
  *return_vals  = values;

  INIT_I18N ();

  /* Get the active drawable info */

  drawable = picman_drawable_get (param[2].data.d_drawable);
  img_has_alpha = picman_drawable_has_alpha (drawable->drawable_id);

  random_generator = g_rand_new ();

  /*
   * Check precondition before we open a dialog: Is there a selection
   * that intersects, OR is there no selection (use entire drawable.)
   */
  {
    gint x1, y1, width, height; /* Not used. */

    if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                        &x1, &y1, &width, &height))
      {
        values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;
        *nreturn_vals           = 2;
        values[1].type          = PICMAN_PDB_STRING;
        values[1].data.d_string = _("The selection does not intersect "
                                    "the active layer or mask.");

        picman_drawable_detach (drawable);

        return;
      }
  }


  switch (run_mode)
    {
        /*
         * Note: there's a limitation here. Running this plug-in before the
         * interactive plug-in was run will cause it to crash, because the
         * data is uninitialized.
         * */
    case PICMAN_RUN_INTERACTIVE:
    case PICMAN_RUN_NONINTERACTIVE:
    case PICMAN_RUN_WITH_LAST_VALS:
      picmanressionist_get_data ();
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        {
          if (!create_picmanressionist ())
              return;
        }
      break;
    default:
      status = PICMAN_PDB_EXECUTION_ERROR;
      break;
    }
  if ((status == PICMAN_PDB_SUCCESS) &&
      (picman_drawable_is_rgb (drawable->drawable_id) ||
       picman_drawable_is_gray (drawable->drawable_id)))
    {

      if (with_specified_preset)
        {
          /* If select_preset fails - set to an error */
          if (select_preset (preset_name))
            {
              status = PICMAN_PDB_EXECUTION_ERROR;
            }
        }
      /* It seems that the value of the run variable is stored in
       * the preset. I don't know if it's a bug or a feature, but
       * I just work here and am anxious to get a working version.
       * So I'm setting it to the correct value here.
       *
       * It also seems that defaultpcvals have this erroneous
       * value as well, so it gets set to FALSE as well. Thus it
       * is always set to TRUE upon a non-interactive run.
       *        -- Shlomi Fish
       * */
      if (run_mode == PICMAN_RUN_NONINTERACTIVE)
        {
          pcvals.run = TRUE;
        }

      if (status == PICMAN_PDB_SUCCESS)
        {
          picmanressionist_main ();
          picman_displays_flush ();

          if (run_mode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC,
                           &pcvals,
                           sizeof (picmanressionist_vals_t));
        }
    }
  else if (status == PICMAN_PDB_SUCCESS)
    {
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  /* Resources Cleanup */
  g_rand_free (random_generator);
  free_parsepath_cache ();
  brush_reload (NULL, NULL);
  preview_free_resources ();
  brush_free ();
  preset_free ();
  orientation_map_free_resources ();
  size_map_free_resources ();

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

void
grabarea (void)
{
  PicmanPixelRgn  src_rgn;
  ppm_t        *p;
  gint          x1, y1;
  gint          x, y;
  gint          width, height;
  gint          row, col;
  gint          rowstride;
  gpointer      pr;

  if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                      &x1, &y1, &width, &height))
    return;

  ppm_new (&infile, width, height);
  p = &infile;

  if (picman_drawable_has_alpha (drawable->drawable_id))
    ppm_new (&inalpha, width, height);

  rowstride = p->width * 3;

  picman_pixel_rgn_init (&src_rgn,
                       drawable, x1, y1, width, height, FALSE, FALSE);

  for (pr = picman_pixel_rgns_register (1, &src_rgn);
       pr != NULL;
       pr = picman_pixel_rgns_process (pr))
    {
      const guchar *src = src_rgn.data;

      switch (src_rgn.bpp)
        {
        case 1:
          for (y = 0, row = src_rgn.y - y1; y < src_rgn.h; y++, row++)
            {
              const guchar *s      = src;
              guchar       *tmprow = p->col + row * rowstride;

              for (x = 0, col = src_rgn.x - x1; x < src_rgn.w; x++, col++)
                {
                  gint k = col * 3;

                  tmprow[k + 0] = s[0];
                  tmprow[k + 1] = s[0];
                  tmprow[k + 2] = s[0];

                  s++;
                }

              src += src_rgn.rowstride;
            }
          break;

        case 2:
          for (y = 0, row = src_rgn.y - y1; y < src_rgn.h; y++, row++)
            {
              const guchar *s       = src;
              guchar       *tmprow  = p->col + row * rowstride;
              guchar       *tmparow = inalpha.col + row * rowstride;

              for (x = 0, col = src_rgn.x - x1; x < src_rgn.w; x++, col++)
                {
                  gint k = col * 3;

                  tmprow[k + 0] = s[0];
                  tmprow[k + 1] = s[0];
                  tmprow[k + 2] = s[0];
                  tmparow[k]    = 255 - s[1];

                  s += 2;
                }

              src += src_rgn.rowstride;
            }
          break;

        case 3:
          col = src_rgn.x - x1;

          for (y = 0, row = src_rgn.y - y1; y < src_rgn.h; y++, row++)
            {
              memcpy (p->col + row * rowstride + col * 3, src, src_rgn.w * 3);

              src += src_rgn.rowstride;
            }
          break;

        case 4:
          for (y = 0, row = src_rgn.y - y1; y < src_rgn.h; y++, row++)
            {
              const guchar *s       = src;
              guchar       *tmprow  = p->col + row * rowstride;
              guchar       *tmparow = inalpha.col + row * rowstride;

              for (x = 0, col = src_rgn.x - x1; x < src_rgn.w; x++, col++)
                {
                  gint k = col * 3;

                  tmprow[k + 0] = s[0];
                  tmprow[k + 1] = s[1];
                  tmprow[k + 2] = s[2];
                  tmparow[k]    = 255 - s[3];

                  s += 4;
                }

              src += src_rgn.rowstride;
            }
          break;
        }
    }
}

static void
picmanressionist_main (void)
{
  PicmanPixelRgn  dest_rgn;
  ppm_t        *p;
  gint          x1, y1;
  gint          x, y;
  gint          width, height;
  gint          row, col;
  gint          rowstride;
  gint          count;
  glong         done;
  glong         total;
  gpointer      pr;

  if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                      &x1, &y1, &width, &height))
    return;

  total = width * height;

  picman_progress_init (_("Painting"));

  if (! PPM_IS_INITED (&infile))
    grabarea ();

  repaint (&infile, (img_has_alpha) ? &inalpha : NULL);

  p = &infile;

  rowstride = p->width * 3;

  picman_pixel_rgn_init (&dest_rgn,
                       drawable, x1, y1, width, height, TRUE, TRUE);

  for (pr = picman_pixel_rgns_register (1, &dest_rgn), count = 0, done = 0;
       pr != NULL;
       pr = picman_pixel_rgns_process (pr), count++)
    {
      guchar *dest = dest_rgn.data;

      switch (dest_rgn.bpp)
        {
        case 1:
          for (y = 0, row = dest_rgn.y - y1; y < dest_rgn.h; y++, row++)
            {
              guchar       *d       = dest;
              const guchar *tmprow  = p->col + row * rowstride;

              for (x = 0, col = dest_rgn.x - x1; x < dest_rgn.w; x++, col++)
                {
                  gint k = col * 3;

                  *d++ = PICMAN_RGB_LUMINANCE (tmprow[k + 0],
                                             tmprow[k + 1],
                                             tmprow[k + 2]);
                }

              dest += dest_rgn.rowstride;
            }
          break;

        case 2:
          for (y = 0, row = dest_rgn.y - y1; y < dest_rgn.h; y++, row++)
            {
              guchar       *d       = dest;
              const guchar *tmprow  = p->col + row * rowstride;
              const guchar *tmparow = inalpha.col + row * rowstride;

              for (x = 0, col = dest_rgn.x - x1; x < dest_rgn.w; x++, col++)
                {
                  gint k     = col * 3;
                  gint value = PICMAN_RGB_LUMINANCE (tmprow[k + 0],
                                                   tmprow[k + 1],
                                                   tmprow[k + 2]);

                  d[0] = value;
                  d[1] = 255 - tmparow[k];

                  d += 2;
                }

              dest += dest_rgn.rowstride;
            }
          break;

        case 3:
          col = dest_rgn.x - x1;

          for (y = 0, row = dest_rgn.y - y1; y < dest_rgn.h; y++, row++)
            {
              memcpy (dest, p->col + row * rowstride + col * 3, dest_rgn.w * 3);

              dest += dest_rgn.rowstride;
            }
          break;

        case 4:
          for (y = 0, row = dest_rgn.y - y1; y < dest_rgn.h; y++, row++)
            {
              guchar       *d       = dest;
              const guchar *tmprow  = p->col + row * rowstride;
              const guchar *tmparow = inalpha.col + row * rowstride;

              for (x = 0, col = dest_rgn.x - x1; x < dest_rgn.w; x++, col++)
                {
                  gint k = col * 3;

                  d[0] = tmprow[k + 0];
                  d[1] = tmprow[k + 1];
                  d[2] = tmprow[k + 2];
                  d[3] = 255 - tmparow[k];

                  d += 4;
                }

              dest += dest_rgn.rowstride;
            }
          break;
        }

      done += dest_rgn.w * dest_rgn.h;

      if (count % 16 == 0)
        picman_progress_update (0.8 + 0.2 * done / total);
    }

  picman_progress_update (1.0);
  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x1, y1, width, height);
}
