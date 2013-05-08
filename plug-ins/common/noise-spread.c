/* Spread --- image filter plug-in for PICMAN
 * Copyright (C) 1997 Brian Degenhardt and Federico Mena Quintero
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

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC    "plug-in-spread"
#define PLUG_IN_BINARY  "noise-spread"
#define PLUG_IN_ROLE    "picman-noise-spread"
#define TILE_CACHE_SIZE 16

typedef struct
{
  gdouble  spread_amount_x;
  gdouble  spread_amount_y;
} SpreadValues;

/* Declare local functions.
 */
static void      query                 (void);
static void      run                   (const gchar      *name,
                                        gint              nparams,
                                        const PicmanParam  *param,
                                        gint             *nreturn_vals,
                                        PicmanParam       **return_vals);

static void      spread                (PicmanDrawable     *drawable);

static void      spread_preview_update (PicmanPreview      *preview,
                                        GtkWidget        *size);
static gboolean  spread_dialog         (gint32            image_ID,
                                        PicmanDrawable     *drawable);

/***** Local vars *****/

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static SpreadValues spvals =
{
  5,   /*  horizontal spread amount  */
  5    /*  vertical spread amount    */
};

/***** Functions *****/

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",        "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",           "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable",        "Input drawable" },
    { PICMAN_PDB_FLOAT,    "spread-amount-x", "Horizontal spread amount (0 <= spread_amount_x <= 200)" },
    { PICMAN_PDB_FLOAT,    "spread-amount-y", "Vertical spread amount (0 <= spread_amount_y <= 200)"   }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Move pixels around randomly"),
                          "Spreads the pixels of the specified drawable.  "
                          "Pixels are randomly moved to another location whose "
                          "distance varies from the original by the horizontal "
                          "and vertical spread amounts ",
                          "Spencer Kimball and Peter Mattis, ported by Brian "
                          "Degenhardt and Federico Mena Quintero",
                          "Federico Mena Quintero and Brian Degenhardt",
                          "1997",
                          N_("Sp_read..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Noise");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  gint32             image_ID;
  PicmanDrawable      *drawable;
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  /*  Get the specified image and drawable  */
  image_ID = param[1].data.d_image;
  drawable = picman_drawable_get (param[2].data.d_drawable);

  /*  set the tile cache size  */
  picman_tile_cache_ntiles (TILE_CACHE_SIZE);

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &spvals);

      /*  First acquire information with a dialog  */
      if (! spread_dialog (image_ID, drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 5)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      else
        {
          spvals.spread_amount_x= param[3].data.d_float;
          spvals.spread_amount_y = param[4].data.d_float;
        }

      if ((status == PICMAN_PDB_SUCCESS) &&
          (spvals.spread_amount_x < 0 || spvals.spread_amount_x > 200) &&
          (spvals.spread_amount_y < 0 || spvals.spread_amount_y > 200))
        status = PICMAN_PDB_CALLING_ERROR;
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &spvals);
      break;

    default:
      break;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      /*  Make sure that the drawable is gray or RGB color  */
      if (picman_drawable_is_rgb (drawable->drawable_id) ||
          picman_drawable_is_gray (drawable->drawable_id))
        {
          picman_progress_init (_("Spreading"));

          /*  run the spread effect  */
          spread (drawable);

          if (run_mode != PICMAN_RUN_NONINTERACTIVE)
            picman_displays_flush ();

          /*  Store data  */
          if (run_mode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC, &spvals, sizeof (SpreadValues));
        }
      else
        {
          /* picman_message ("spread: cannot operate on indexed color images"); */
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
    }

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

typedef struct
{
  PicmanPixelFetcher *pft;
  GRand            *gr;
  gint              x_amount;
  gint              y_amount;
  gint              width;
  gint              height;
} SpreadParam_t;

/* Spread the image.  This is done by going through every pixel
   in the source image and swapping it with some other random
   pixel.  The random pixel is located within an ellipse that is
   as high as the spread_amount_y parameter and as wide as the
   spread_amount_x parameter.  This is done by randomly selecting
   an angle and then multiplying the sine of the angle to a random
   number whose range is between -spread_amount_x/2 and spread_amount_x/2.
   The y coordinate is found by multiplying the cosine of the angle
   to the random value generated from spread_amount_y.  The reason
   that the spread is done this way is to make the end product more
   random looking.  To see a result of this, compare spreading a
   square with picman 0.54 to spreading a square with this filter.
   The corners are less sharp with this algorithm.
*/

static void
spread_func (gint      x,
             gint      y,
             guchar   *dest,
             gint      bpp,
             gpointer  data)
{
  SpreadParam_t *param = (SpreadParam_t*) data;
  gdouble        angle;
  gint           xdist, ydist;
  gint           xi, yi;

  /* get random angle, x distance, and y distance */
  xdist = (param->x_amount > 0
           ? g_rand_int_range (param->gr, -param->x_amount, param->x_amount)
           : 0);
  ydist = (param->y_amount > 0
           ? g_rand_int_range (param->gr, -param->y_amount, param->y_amount)
           : 0);
  angle = g_rand_double_range (param->gr, -G_PI, G_PI);

  xi = x + floor (sin (angle) * xdist);
  yi = y + floor (cos (angle) * ydist);

  /* Only displace the pixel if it's within the bounds of the image. */
  if (xi >= 0 && xi < param->width && yi >= 0 && yi < param->height)
    {
      picman_pixel_fetcher_get_pixel (param->pft, xi, yi, dest);
    }
  else /* Else just copy it */
    {
      picman_pixel_fetcher_get_pixel (param->pft, x, y, dest);
    }
}

static void
spread (PicmanDrawable *drawable)
{
  PicmanRgnIterator *iter;
  SpreadParam_t    param;

  param.pft      = picman_pixel_fetcher_new (drawable, FALSE);
  param.gr       = g_rand_new ();
  param.x_amount = (spvals.spread_amount_x + 1) / 2;
  param.y_amount = (spvals.spread_amount_y + 1) / 2;
  param.width    = drawable->width;
  param.height   = drawable->height;

  picman_pixel_fetcher_set_edge_mode(param.pft, PICMAN_PIXEL_FETCHER_EDGE_BLACK);
  iter = picman_rgn_iterator_new (drawable, 0);
  picman_rgn_iterator_dest (iter, spread_func, &param);
  picman_rgn_iterator_free (iter);

  g_rand_free (param.gr);
}

static void
spread_preview_update (PicmanPreview *preview,
                       GtkWidget   *size)
{
  PicmanDrawable   *drawable;
  SpreadParam_t   param;
  gint            x, y, bpp;
  guchar         *buffer, *dest;
  gint            x_off, y_off;
  gint            width, height;

  drawable =
    picman_drawable_preview_get_drawable (PICMAN_DRAWABLE_PREVIEW (preview));

  param.pft      = picman_pixel_fetcher_new (drawable, FALSE);
  param.gr       = g_rand_new ();
  param.x_amount = (picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (size),
                                                0) + 1) / 2;
  param.y_amount = (picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (size),
                                                1) + 1) / 2;
  param.width    = drawable->width;
  param.height   = drawable->height;
  picman_pixel_fetcher_set_edge_mode(param.pft, PICMAN_PIXEL_FETCHER_EDGE_BLACK);

  picman_preview_get_size (preview, &width, &height);

  bpp = drawable->bpp;
  dest = buffer = g_new (guchar, width * height * bpp);

  picman_preview_get_position (preview, &x_off, &y_off);

  for (y = 0 ; y < height ; y++)
    for (x = 0 ; x < width ; x++)
      {
        spread_func (x + x_off, y + y_off, dest, bpp, &param);
        dest += bpp;
      }

  picman_preview_draw_buffer (preview, buffer, width * bpp);

  g_free (buffer);
  g_rand_free (param.gr);
}

static gboolean
spread_dialog (gint32        image_ID,
               PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *frame;
  GtkWidget *size;
  PicmanUnit   unit;
  gdouble    xres;
  gdouble    yres;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Spread"), PLUG_IN_ROLE,
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

  frame = picman_frame_new (_("Spread Amount"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  Get the image resolution and unit  */
  picman_image_get_resolution (image_ID, &xres, &yres);
  unit = picman_image_get_unit (image_ID);

  /* sizeentries */
  size = picman_coordinates_new (unit, "%a", TRUE, FALSE, -1,
                               PICMAN_SIZE_ENTRY_UPDATE_SIZE,

                               spvals.spread_amount_x == spvals.spread_amount_y,
                               FALSE,

                               _("_Horizontal:"), spvals.spread_amount_x, xres,
                               0, MAX (drawable->width, drawable->height),
                               0, 0,

                               _("_Vertical:"), spvals.spread_amount_y, yres,
                               0, MAX (drawable->width, drawable->height),
                               0, 0);
  gtk_container_add (GTK_CONTAINER (frame), size);
  gtk_widget_show (size);

  g_signal_connect (preview, "invalidated",
                    G_CALLBACK (spread_preview_update),
                    size);
  g_signal_connect_swapped (size, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (dialog);

  spread_preview_update (PICMAN_PREVIEW (preview), size);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  if (run)
    {
      spvals.spread_amount_x =
        picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (size), 0);
      spvals.spread_amount_y =
        picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (size), 1);
    }

  gtk_widget_destroy (dialog);

  return run;
}
