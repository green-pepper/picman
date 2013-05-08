/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Destripe filter
 *
 * Copyright 1997 Marc Lehmann, heavily modified from a filter by
 * Michael Sweet.
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
 *
 */

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


/*
 * Constants...
 */

#define PLUG_IN_PROC    "plug-in-destripe"
#define PLUG_IN_BINARY  "destripe"
#define PLUG_IN_ROLE    "picman-destripe"
#define PLUG_IN_VERSION "0.2"
#define SCALE_WIDTH     140
#define MAX_AVG         100


/*
 * Local functions...
 */

static void      query (void);
static void      run   (const gchar      *name,
                        gint              nparams,
                        const PicmanParam  *param,
                        gint             *nreturn_vals,
                        PicmanParam       **return_vals);

static void      destripe        (PicmanDrawable *drawable,
                                  PicmanPreview  *preview);

static gboolean  destripe_dialog (PicmanDrawable *drawable);

/*
 * Globals...
 */

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run    /* run_proc   */
};

typedef struct
{
  gboolean histogram;
  gint     avg_width;
  gboolean preview;
} DestripeValues;

static DestripeValues vals =
{
  FALSE, /* histogram     */
  36,    /* average width */
  TRUE   /* preview */
};


MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }"          },
    { PICMAN_PDB_IMAGE,    "image",     "Input image"                           },
    { PICMAN_PDB_DRAWABLE, "drawable",  "Input drawable"                        },
    { PICMAN_PDB_INT32,    "avg-width", "Averaging filter width (default = 36)" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Remove vertical stripe artifacts from the image"),
                          "This plug-in tries to remove vertical stripes from "
                          "an image.",
                          "Marc Lehmann <pcg@goof.com>",
                          "Marc Lehmann <pcg@goof.com>",
                          PLUG_IN_VERSION,
                          N_("Des_tripe..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Enhance");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  PicmanRunMode        run_mode;   /* Current run mode */
  PicmanPDBStatusType  status;     /* Return status */
  static PicmanParam   values[1];  /* Return values */
  PicmanDrawable      *drawable;

  INIT_I18N ();

  /*
   * Initialize parameter data...
   */

  status   = PICMAN_PDB_SUCCESS;
  run_mode = param[0].data.d_int32;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  *nreturn_vals = 1;
  *return_vals  = values;

  /*
   * Get drawable information...
   */

  drawable = picman_drawable_get (param[2].data.d_drawable);

  /*
   * See how we will run
   */

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*
       * Possibly retrieve data...
       */
      picman_get_data (PLUG_IN_PROC, &vals);

      /*
       * Get information from the dialog...
       */
      if (!destripe_dialog (drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*
       * Make sure all the arguments are present...
       */
      if (nparams != 4)
        status = PICMAN_PDB_CALLING_ERROR;
      else
        vals.avg_width = param[3].data.d_int32;
      break;

    case PICMAN_RUN_WITH_LAST_VALS :
      /*
       * Possibly retrieve data...
       */
      picman_get_data (PLUG_IN_PROC, &vals);
      break;

    default :
      status = PICMAN_PDB_CALLING_ERROR;
      break;
    };

  /*
   * Destripe the image...
   */

  if (status == PICMAN_PDB_SUCCESS)
    {
      if ((picman_drawable_is_rgb (drawable->drawable_id) ||
           picman_drawable_is_gray (drawable->drawable_id)))
        {
          /*
           * Set the tile cache size...
           */
          picman_tile_cache_ntiles ((drawable->width + picman_tile_width () - 1) /
                                  picman_tile_width ());

          /*
           * Run!
           */
          destripe (drawable, NULL);

          /*
           * If run mode is interactive, flush displays...
           */
          if (run_mode != PICMAN_RUN_NONINTERACTIVE)
            picman_displays_flush ();

          /*
           * Store data...
           */
          if (run_mode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC, &vals, sizeof (vals));
        }
      else
        status = PICMAN_PDB_EXECUTION_ERROR;
    };

  /*
   * Reset the current run status...
   */
  values[0].data.d_status = status;

  /*
   * Detach from the drawable...
   */
  picman_drawable_detach (drawable);
}

static void
destripe (PicmanDrawable *drawable,
          PicmanPreview  *preview)
{
  PicmanPixelRgn  src_rgn;        /* source image region */
  PicmanPixelRgn  dst_rgn;        /* destination image region */
  guchar       *src_rows;       /* image data */
  gdouble       progress, progress_inc;
  gint          x1, x2, y1;
  gint          width, height;
  gint          bpp;
  glong        *hist, *corr;        /* "histogram" data */
  gint          tile_width = picman_tile_width ();
  gint          i, x, y, ox, cols;

  /* initialize */

  progress = 0.0;
  progress_inc = 0.0;

  /*
   * Let the user know what we're doing...
   */
  bpp = picman_drawable_bpp (drawable->drawable_id);
  if (preview)
    {
      picman_preview_get_position (preview, &x1, &y1);
      picman_preview_get_size (preview, &width, &height);
    }
  else
    {
      picman_progress_init (_("Destriping"));
      if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                          &x1, &y1, &width, &height))
        {
          return;
        }
      progress = 0;
      progress_inc = 0.5 * tile_width / width;
    }

  x2 = x1 + width;

  /*
   * Setup for filter...
   */

  picman_pixel_rgn_init (&src_rgn, drawable,
                       x1, y1, width, height, FALSE, FALSE);
  picman_pixel_rgn_init (&dst_rgn, drawable,
                       x1, y1, width, height, (preview == NULL), TRUE);

  hist = g_new (long, width * bpp);
  corr = g_new (long, width * bpp);
  src_rows = g_new (guchar, tile_width * height * bpp);

  memset (hist, 0, width * bpp * sizeof (long));

  /*
   * collect "histogram" data.
   */

  for (ox = x1; ox < x2; ox += tile_width)
    {
      guchar *rows = src_rows;

      cols = x2 - ox;
      if (cols > tile_width)
        cols = tile_width;

      picman_pixel_rgn_get_rect (&src_rgn, rows, ox, y1, cols, height);

      for (y = 0; y < height; y++)
        {
          long   *h       = hist + (ox - x1) * bpp;
          guchar *row_end = rows + cols * bpp;

          while (rows < row_end)
            *h++ += *rows++;
        }

      if (!preview)
        picman_progress_update (progress += progress_inc);
    }

  /*
   * average out histogram
   */

  {
    gint extend = (vals.avg_width / 2) * bpp;

    for (i = 0; i < MIN (3, bpp); i++)
      {
        long *h   = hist - extend + i;
        long *c   = corr - extend + i;
        long  sum = 0;
        gint  cnt = 0;

        for (x = -extend; x < width * bpp; x += bpp)
          {
            if (x + extend < width * bpp)
              {
                sum += h[ extend]; cnt++;
              }
            if (x - extend >= 0)
              {
                sum -= h[-extend]; cnt--;
              }
            if (x >= 0)
              {
                if (*h)
                  *c = ((sum / cnt - *h) << 10) / *h;
                else
                  *c = G_MAXINT;
              }

            h += bpp;
            c += bpp;
          }
      }
  }

  /*
   * remove stripes.
   */

  for (ox = x1; ox < x2; ox += tile_width)
    {
      guchar *rows = src_rows;

      cols = x2 - ox;
      if (cols > tile_width)
        cols = tile_width;

      picman_pixel_rgn_get_rect (&src_rgn, rows, ox, y1, cols, height);

      if (!preview)
        picman_progress_update (progress += progress_inc);

      for (y = 0; y < height; y++)
        {
          long   *c = corr + (ox - x1) * bpp;
          guchar *row_end = rows + cols * bpp;

          if (vals.histogram)
            while (rows < row_end)
              {
                *rows = MIN (255, MAX (0, 128 + (*rows * *c >> 10)));
                c++; rows++;
              }
          else
            while (rows < row_end)
              {
                *rows = MIN (255, MAX (0, *rows + (*rows * *c >> 10) ));
                c++; rows++;
              }
        }

      picman_pixel_rgn_set_rect (&dst_rgn, src_rows,
                               ox, y1, cols, height);
      if (!preview)
        picman_progress_update (progress += progress_inc);
    }

  g_free (src_rows);

  /*
   * Update the screen...
   */

  if (preview)
    {
      picman_drawable_preview_draw_region (PICMAN_DRAWABLE_PREVIEW (preview),
                                         &dst_rgn);
    }
  else
    {
      picman_progress_update (1.0);
      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id,
                            x1, y1, width, height);
    }
  g_free (hist);
  g_free (corr);
}

static gboolean
destripe_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *table;
  GtkWidget *button;
  GtkObject *adj;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, TRUE);

  dialog = picman_dialog_new (_("Destripe"), PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func, PLUG_IN_PROC,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  picman_window_set_transient (GTK_WINDOW (dialog));

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  preview = picman_drawable_preview_new (drawable, NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  g_signal_connect_swapped (preview, "invalidated",
                            G_CALLBACK (destripe),
                            drawable);

  table = gtk_table_new (1, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                              _("_Width:"), SCALE_WIDTH, 0,
                              vals.avg_width, 2, MAX_AVG, 1, 10, 0,
                              TRUE, 0, 0,
                              NULL, NULL);
  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &vals.avg_width);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  button = gtk_check_button_new_with_mnemonic (_("Create _histogram"));
  gtk_box_pack_start (GTK_BOX (main_vbox), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), vals.histogram);
  gtk_widget_show (button);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &vals.histogram);
  g_signal_connect_swapped (button, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}
