/*
 * smooth palette - derive smooth palette from image
 * Copyright (C) 1997  Scott Draves <spot@cs.cmu.edu>
 *
 * PICMAN - The GNU Image Manipulation Program
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

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC   "plug-in-smooth-palette"
#define PLUG_IN_BINARY "smooth-palette"
#define PLUG_IN_ROLE   "picman-smooth-palette"


/* Declare local functions. */
static void      query          (void);
static void      run            (const gchar      *name,
                                 gint              nparams,
                                 const PicmanParam  *param,
                                 gint             *nreturn_vals,
                                 PicmanParam       **return_vals);

static gboolean  dialog         (PicmanDrawable     *drawable);

static gint32    smooth_palette (PicmanDrawable     *drawable,
                                 gint32           *layer_id);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};


MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",   "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",      "Input image (unused)"         },
    { PICMAN_PDB_DRAWABLE, "drawable",   "Input drawable"               },
    { PICMAN_PDB_INT32,    "width",      "Width"                        },
    { PICMAN_PDB_INT32,    "height",     "Height"                       },
    { PICMAN_PDB_INT32,    "ntries",     "Search Depth"                 },
    { PICMAN_PDB_INT32,    "show-image", "Show Image?"                  }
  };

  static const PicmanParamDef return_vals[] =
  {
    { PICMAN_PDB_IMAGE, "new-image", "Output image" },
    { PICMAN_PDB_LAYER, "new-layer", "Output layer" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Derive a smooth color palette from the image"),
                          "help!",
                          "Scott Draves",
                          "Scott Draves",
                          "1997",
                          N_("Smoo_th Palette..."),
                          "RGB*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), G_N_ELEMENTS (return_vals),
                          args, return_vals);
}

static struct
{
  gint width;
  gint height;
  gint ntries;
  gint try_size;
  gint show_image;
} config =
{
  256,
  64,
  50,
  10000,
  1
};

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[3];
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  PicmanDrawable      *drawable;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 3;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;
  values[1].type          = PICMAN_PDB_IMAGE;
  values[2].type          = PICMAN_PDB_LAYER;

  drawable = picman_drawable_get (param[2].data.d_drawable);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      picman_get_data (PLUG_IN_PROC, &config);
      if (! dialog (drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      if (nparams != 7)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      else
        {
          config.width      = param[3].data.d_int32;
          config.height     = param[4].data.d_int32;
          config.ntries     = param[5].data.d_int32;
          config.show_image = param[6].data.d_int32 ? TRUE : FALSE;
        }

      if (status == PICMAN_PDB_SUCCESS &&
          ((config.width <= 0) || (config.height <= 0) || config.ntries <= 0))
        status = PICMAN_PDB_CALLING_ERROR;

      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &config);
      break;

    default:
      break;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      if (picman_drawable_is_rgb (drawable->drawable_id))
        {
          picman_progress_init (_("Deriving smooth palette"));

          picman_tile_cache_ntiles (2 * (drawable->width + 1) /
                                  picman_tile_width ());

          values[1].data.d_image = smooth_palette (drawable,
                                                   &values[2].data.d_layer);

          if (run_mode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC, &config, sizeof (config));

          if (config.show_image)
            picman_display_new (values[1].data.d_image);
        }
      else
        {
          status = PICMAN_PDB_EXECUTION_ERROR;
        }

      picman_drawable_detach (drawable);
    }

  values[0].data.d_status = status;
}

static long
pix_diff (guchar *pal,
          guint    bpp,
          gint    i,
          gint    j)
{
  glong r = 0;
  guint k;

  for (k = 0; k < bpp; k++)
    {
      gint p1 = pal[j * bpp + k];
      gint p2 = pal[i * bpp + k];
      r += (p1 - p2) * (p1 - p2);
    }

  return r;
}

static void
pix_swap (guchar *pal,
          guint   bpp,
          gint    i,
          gint    j)
{
  guint k;

  for (k = 0; k < bpp; k++)
    {
      guchar t = pal[j * bpp + k];
      pal[j * bpp + k] = pal[i * bpp + k];
      pal[i * bpp + k] = t;
    }
}

static gint32
smooth_palette (PicmanDrawable *drawable,
                gint32       *layer_id)
{
  gint32        new_image_id;
  PicmanDrawable *new_layer;
  gint          psize, i, j;
  guchar       *pal;
  guint         bpp = drawable->bpp;
  gint          sel_x1, sel_x2, sel_y1, sel_y2;
  gint          width, height;
  PicmanPixelRgn  pr;
  GRand        *gr;

  gr = g_rand_new ();

  new_image_id = picman_image_new (config.width, config.height, PICMAN_RGB);
  picman_image_undo_disable (new_image_id);
  *layer_id = picman_layer_new (new_image_id, _("Background"),
                              config.width, config.height,
                              picman_drawable_type (drawable->drawable_id),
                              100, PICMAN_NORMAL_MODE);
  picman_image_insert_layer (new_image_id, *layer_id, -1, 0);
  new_layer = picman_drawable_get (*layer_id);

  psize = config.width;

  pal = g_new (guchar, psize * bpp);

  picman_drawable_mask_bounds (drawable->drawable_id,
                             &sel_x1, &sel_y1, &sel_x2, &sel_y2);
  width = sel_x2 - sel_x1;
  height = sel_y2 - sel_y1;

  picman_pixel_rgn_init (&pr, drawable, sel_x1, sel_y1, width, height,
                       FALSE, FALSE);

  /* get initial palette */
  for (i = 0; i < psize; i++)
    {
      gint x = sel_x1 + g_rand_int_range (gr, 0, width);
      gint y = sel_y1 + g_rand_int_range (gr, 0, height);

      picman_pixel_rgn_get_pixel (&pr, pal + bpp * i, x, y);
    }

  /* reorder */
  if (1)
    {
      guchar  *pal_best;
      guchar  *original;
      gdouble  len_best = 0;
      gint     try;

      pal_best = g_memdup (pal, bpp * psize);
      original = g_memdup (pal, bpp * psize);

      for (try = 0; try < config.ntries; try++)
        {
          gdouble len;

          if (!(try%5))
            picman_progress_update (try / (double) config.ntries);
          memcpy (pal, original, bpp * psize);

          /* scramble */
          for (i = 1; i < psize; i++)
            pix_swap (pal, bpp, i, g_rand_int_range (gr, 0, psize));

          /* measure */
          len = 0.0;
          for (i = 1; i < psize; i++)
            len += pix_diff (pal, bpp, i, i-1);

          /* improve */
          for (i = 0; i < config.try_size; i++)
            {
              gint  i0 = 1 + g_rand_int_range (gr, 0, psize-2);
              gint  i1 = 1 + g_rand_int_range (gr, 0, psize-2);
              glong as_is, swapd;

              if (1 == (i0 - i1))
                {
                  as_is = (pix_diff (pal, bpp, i1 - 1, i1) +
                           pix_diff (pal, bpp, i0, i0 + 1));
                  swapd = (pix_diff (pal, bpp, i1 - 1, i0) +
                           pix_diff (pal, bpp, i1, i0 + 1));
                }
              else if (1 == (i1 - i0))
                {
                  as_is = (pix_diff (pal, bpp, i0 - 1, i0) +
                           pix_diff (pal, bpp, i1, i1 + 1));
                  swapd = (pix_diff (pal, bpp, i0 - 1, i1) +
                           pix_diff (pal, bpp, i0, i1 + 1));
                }
              else
                {
                  as_is = (pix_diff (pal, bpp, i0, i0 + 1) +
                           pix_diff (pal, bpp, i0, i0 - 1) +
                           pix_diff (pal, bpp, i1, i1 + 1) +
                           pix_diff (pal, bpp, i1, i1 - 1));
                  swapd = (pix_diff (pal, bpp, i1, i0 + 1) +
                           pix_diff (pal, bpp, i1, i0 - 1) +
                           pix_diff (pal, bpp, i0, i1 + 1) +
                           pix_diff (pal, bpp, i0, i1 - 1));
                }
              if (swapd < as_is)
                {
                  pix_swap (pal, bpp, i0, i1);
                  len += swapd - as_is;
                }
            }
          /* best? */
          if (0 == try || len < len_best)
            {
              memcpy (pal_best, pal, bpp * psize);
              len_best = len;
            }
        }
      picman_progress_update (1.0);
      memcpy (pal, pal_best, bpp * psize);
      g_free (pal_best);
      g_free (original);
      /* clean */
      for (i = 1; i < 4 * psize; i++)
        {
          glong as_is, swapd;
          gint i0 = 1 + g_rand_int_range (gr, 0, psize - 2);
          gint i1 = i0 + 1;

          as_is = (pix_diff (pal, bpp, i0 - 1, i0) +
                   pix_diff (pal, bpp, i1, i1 + 1));
          swapd = (pix_diff (pal, bpp, i0 - 1, i1) +
                   pix_diff (pal, bpp, i0, i1 + 1));
          if (swapd < as_is)
            {
              pix_swap (pal, bpp, i0, i1);
              len_best += swapd - as_is;
            }
        }
    }

  /* store smooth palette */
  picman_pixel_rgn_init (&pr, new_layer, 0, 0,
                       config.width, config.height,
                       TRUE, FALSE);
  for (j = 0; j < config.height; j++)
    for (i = 0; i < config.width; i++)
      picman_pixel_rgn_set_pixel (&pr, pal + bpp * i, i, j);
  g_free (pal);

  g_rand_free (gr);
  picman_drawable_flush (new_layer);
  picman_drawable_update(new_layer->drawable_id, 0, 0,
                       config.width, config.height);
  picman_image_undo_enable (new_image_id);

  return new_image_id;
}

static gboolean
dialog (PicmanDrawable *drawable)
{
  GtkWidget *dlg;
  GtkWidget *spinbutton;
  GtkObject *adj;
  GtkWidget *sizeentry;
  guint32    image_id;
  PicmanUnit   unit;
  gdouble    xres, yres;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dlg = picman_dialog_new (_("Smooth Palette"), PLUG_IN_ROLE,
                         NULL, 0,
                         picman_standard_help_func, PLUG_IN_PROC,

                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                         GTK_STOCK_OK,     GTK_RESPONSE_OK,

                         NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dlg),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  picman_window_set_transient (GTK_WINDOW (dlg));

  image_id = picman_item_get_image (drawable->drawable_id);
  unit = picman_image_get_unit (image_id);
  picman_image_get_resolution (image_id, &xres, &yres);

  sizeentry = picman_coordinates_new (unit, "%a", TRUE, FALSE, 6,
                                    PICMAN_SIZE_ENTRY_UPDATE_SIZE,
                                    FALSE, FALSE,

                                    _("_Width:"),
                                    config.width, xres,
                                    2, PICMAN_MAX_IMAGE_SIZE,
                                    2, PICMAN_MAX_IMAGE_SIZE,

                                    _("_Height:"),
                                    config.height, yres,
                                    1, PICMAN_MAX_IMAGE_SIZE,
                                    1, PICMAN_MAX_IMAGE_SIZE);
  gtk_container_set_border_width (GTK_CONTAINER (sizeentry), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
                      sizeentry, FALSE, FALSE, 0);
  gtk_widget_show (sizeentry);

  spinbutton = picman_spin_button_new (&adj, config.ntries,
                                     1, 1024, 1, 10, 0, 1, 0);
  picman_table_attach_aligned (GTK_TABLE (sizeentry), 0, 2,
                             _("_Search depth:"), 0.0, 0.5,
                             spinbutton, 1, FALSE);
  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &config.ntries);

  gtk_widget_show (dlg);

  run = (picman_dialog_run (PICMAN_DIALOG (dlg)) == GTK_RESPONSE_OK);

  if (run)
    {
      config.width  = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (sizeentry),
                                                  0);
      config.height = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (sizeentry),
                                                  1);
    }

  gtk_widget_destroy (dlg);

  return run;
}
