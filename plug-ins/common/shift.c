/* Shift --- image filter plug-in for PICMAN
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
 * Please direct all comments, questions, bug reports  etc to Brian Degenhardt
 * bdegenha@ucsd.edu
 *
 * You can contact Federico Mena Quintero at quartic@polloux.fciencias.unam.mx
 * You can contact the original PICMAN authors at picman@xcf.berkeley.edu
 */

#include "config.h"

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


/* Some useful macros */

#define PLUG_IN_PROC      "plug-in-shift"
#define PLUG_IN_BINARY    "shift"
#define PLUG_IN_ROLE      "picman-shift"
#define SPIN_BUTTON_WIDTH  8
#define TILE_CACHE_SIZE   16
#define HORIZONTAL         0
#define VERTICAL           1

typedef struct
{
  gint  shift_amount;
  gint  orientation;
} ShiftValues;


/* Declare local functions.
 */
static void      query  (void);
static void      run    (const gchar      *name,
                         gint              nparams,
                         const PicmanParam  *param,
                         gint             *nreturn_vals,
                         PicmanParam       **return_vals);

static void      shift                 (PicmanDrawable *drawable,
                                        PicmanPreview  *preview);

static gboolean  shift_dialog          (gint32        image_ID,
                                        PicmanDrawable *drawable);
static void      shift_amount_callback (GtkWidget    *widget,
                                        gpointer      data);


/***** Local vars *****/

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static ShiftValues shvals =
{
  5,          /* shift amount */
  HORIZONTAL  /* orientation  */
};


/***** Functions *****/

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }"     },
    { PICMAN_PDB_IMAGE,    "image",        "Input image (unused)"             },
    { PICMAN_PDB_DRAWABLE, "drawable",     "Input drawable"                   },
    { PICMAN_PDB_INT32,    "shift-amount", "shift amount (0 <= shift_amount_x <= 200)" },
    { PICMAN_PDB_INT32,    "orientation",  "vertical, horizontal orientation" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Shift each row of pixels by a random amount"),
                          "Shifts the pixels of the specified drawable. "
                          "Each row will be displaced a random value of pixels.",
                          "Spencer Kimball and Peter Mattis, ported by Brian "
                          "Degenhardt and Federico Mena Quintero",
                          "Brian Degenhardt",
                          "1997",
                          N_("_Shift..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Distorts");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  PicmanDrawable      *drawable;
  gint32             image_ID;
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;

  run_mode = param[0].data.d_int32;
  image_ID = param[1].data.d_int32;

  INIT_I18N ();

  /*  Get the specified drawable  */
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
      picman_get_data (PLUG_IN_PROC, &shvals);

      /*  First acquire information with a dialog  */
      if (! shift_dialog (image_ID, drawable))
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
          shvals.shift_amount = param[3].data.d_int32;
          shvals.orientation = (param[4].data.d_int32) ? HORIZONTAL : VERTICAL;

          if (shvals.shift_amount < 0 || shvals.shift_amount > 200)
            status = PICMAN_PDB_CALLING_ERROR;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &shvals);
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
          picman_progress_init (_("Shifting"));

          /*  run the shift effect  */
          shift (drawable, NULL);

          if (run_mode != PICMAN_RUN_NONINTERACTIVE)
            picman_displays_flush ();

          /*  Store data  */
          if (run_mode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC, &shvals, sizeof (ShiftValues));
        }
      else
        {
          /* picman_message ("shift: cannot operate on indexed color images"); */
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
    }

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

static void
shift (PicmanDrawable *drawable,
       PicmanPreview  *preview)
{
  PicmanPixelRgn      dest_rgn;
  gpointer          pr;
  PicmanPixelFetcher *pft;
  gint              width, height;
  gint              bytes;
  guchar           *destline;
  guchar           *dest;
  gint              x1, y1, x2, y2;
  gint              x, y;
  gint              progress, max_progress;
  gint              i, n = 0;
  gint             *offsets;
  GRand            *gr;

  if (preview)
    {
      picman_preview_get_position (preview, &x1, &y1);
      picman_preview_get_size (preview, &width, &height);
    }
  else
    {
      picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
      width  = x2 - x1;
      height = y2 - y1;
    }

  bytes  = drawable->bpp;

  progress     = 0;
  max_progress = width * height;

  /* Shift the image.  It's a pretty simple algorithm.  If horizontal
     is selected, then every row is shifted a random number of pixels
     in the range of -shift_amount/2 to shift_amount/2.  The effect is
     just reproduced with columns if vertical is selected.
   */

  n = (shvals.orientation == HORIZONTAL) ? height : width;

  offsets = g_new (gint, n);
  gr = g_rand_new ();

  for (i = 0; i < n; i++)
    offsets[i] = g_rand_int_range (gr,
                                   - (shvals.shift_amount + 1) / 2.0,
                                   + (shvals.shift_amount + 1) / 2.0);

  g_rand_free (gr);

  pft = picman_pixel_fetcher_new (drawable, FALSE);
  picman_pixel_fetcher_set_edge_mode (pft, PICMAN_PIXEL_FETCHER_EDGE_WRAP);

  picman_pixel_rgn_init (&dest_rgn, drawable,
                       x1, y1, width, height, (preview == NULL), TRUE);

  for (pr = picman_pixel_rgns_register (1, &dest_rgn);
       pr != NULL;
       pr = picman_pixel_rgns_process (pr))
    {
      destline = dest_rgn.data;

      switch (shvals.orientation)
        {
        case HORIZONTAL:
          for (y = dest_rgn.y; y < dest_rgn.y + dest_rgn.h; y++)
            {
              dest = destline;

              for (x = dest_rgn.x; x < dest_rgn.x + dest_rgn.w; x++)
                {
                  picman_pixel_fetcher_get_pixel (pft,
                                                x + offsets[y - y1], y, dest);
                  dest += bytes;
                }

              destline += dest_rgn.rowstride;
            }
          break;

        case VERTICAL:
          for (x = dest_rgn.x; x < dest_rgn.x + dest_rgn.w; x++)
            {
              dest = destline;

              for (y = dest_rgn.y; y < dest_rgn.y + dest_rgn.h; y++)
                {
                  picman_pixel_fetcher_get_pixel (pft,
                                                x, y + offsets[x - x1], dest);
                  dest += dest_rgn.rowstride;
                }

              destline += bytes;
            }
          break;
        }

      if (preview)
        {
          picman_drawable_preview_draw_region (PICMAN_DRAWABLE_PREVIEW (preview),
                                             &dest_rgn);
        }
      else
        {
          progress += dest_rgn.w * dest_rgn.h;
          picman_progress_update ((double) progress / (double) max_progress);
        }
    }

  picman_pixel_fetcher_destroy (pft);
  g_free (offsets);

  if (! preview)
    {
      picman_progress_update (1.0);
      /*  update the region  */
      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id, x1, y1, width, height);
    }
}

static gboolean
shift_dialog (gint32        image_ID,
              PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *frame;
  GtkWidget *size_entry;
  GtkWidget *vertical;
  GtkWidget *horizontal;
  PicmanUnit   unit;
  gdouble    xres;
  gdouble    yres;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Shift"), PLUG_IN_ROLE,
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
                            G_CALLBACK (shift),
                            drawable);

  frame = picman_int_radio_group_new (FALSE, NULL,
                                    G_CALLBACK (picman_radio_button_update),
                                    &shvals.orientation, shvals.orientation,

                                    _("Shift _horizontally"),
                                    HORIZONTAL, &horizontal,

                                    _("Shift _vertically"),
                                    VERTICAL,   &vertical,

                                    NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  g_signal_connect_swapped (horizontal, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);
  g_signal_connect_swapped (vertical, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  /*  Get the image resolution and unit  */
  picman_image_get_resolution (image_ID, &xres, &yres);
  unit = picman_image_get_unit (image_ID);

  size_entry = picman_size_entry_new (1, unit, "%a", TRUE, FALSE, FALSE,
                                    SPIN_BUTTON_WIDTH,
                                    PICMAN_SIZE_ENTRY_UPDATE_SIZE);

  picman_size_entry_set_unit (PICMAN_SIZE_ENTRY (size_entry), PICMAN_UNIT_PIXEL);
  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (size_entry), 0, xres, TRUE);
  picman_size_entry_set_refval_boundaries (PICMAN_SIZE_ENTRY (size_entry), 0,
                                         1.0, 200.0);
  gtk_table_set_col_spacing (GTK_TABLE (size_entry), 0, 4);
  gtk_table_set_col_spacing (GTK_TABLE (size_entry), 2, 12);
  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (size_entry), 0,
                              (gdouble) shvals.shift_amount);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (size_entry),
                                _("Shift _amount:"), 1, 0, 0.0);

  g_signal_connect (size_entry, "value-changed",
                    G_CALLBACK (shift_amount_callback),
                    preview);
  gtk_box_pack_start (GTK_BOX (main_vbox), size_entry, FALSE, FALSE, 0);
  gtk_widget_show (size_entry);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
shift_amount_callback (GtkWidget *widget,
                       gpointer   data)
{
  PicmanPreview *preview = PICMAN_PREVIEW (data);

  shvals.shift_amount = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (widget),
                                                    0);
  picman_preview_invalidate (preview);
}
