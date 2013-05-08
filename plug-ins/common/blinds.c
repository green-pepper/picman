/*
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This is a plug-in for PICMAN.
 *
 * Blinds plug-in. Distort an image as though it was stuck to
 * window blinds and the blinds where opened/closed.
 *
 * Copyright (C) 1997 Andy Thomas  alt@picnic.demon.co.uk
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
 * A fair proprotion of this code was taken from the Whirl plug-in
 * which was copyrighted by Federico Mena Quintero (as below).
 *
 * Whirl plug-in --- distort an image into a whirlpool
 * Copyright (C) 1997 Federico Mena Quintero
 *
 */

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"

/***** Magic numbers *****/

#define PLUG_IN_PROC   "plug-in-blinds"
#define PLUG_IN_BINARY "blinds"
#define PLUG_IN_ROLE   "picman-blinds"

#define SCALE_WIDTH    150

#define MAX_FANS       100

/* Variables set in dialog box */
typedef struct data
{
  gint                 angledsp;
  gint                 numsegs;
  PicmanOrientationType  orientation;
  gboolean bg_trans;
} BlindVals;

/* Array to hold each size of fans. And no there are not each the
 * same size (rounding errors...)
 */

static gint fanwidths[MAX_FANS];

static void      query  (void);
static void      run    (const gchar      *name,
                         gint              nparams,
                         const PicmanParam  *param,
                         gint             *nreturn_vals,
                         PicmanParam       **return_vals);

static gboolean  blinds_dialog         (PicmanDrawable  *drawable);

static void      dialog_update_preview (PicmanDrawable  *drawable,
                                        PicmanPreview   *preview);
static void      apply_blinds          (PicmanDrawable  *drawable);

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,    /* init_proc */
  NULL,    /* quit_proc */
  query,   /* query_proc */
  run,     /* run_proc */
};

/* Values when first invoked */
static BlindVals bvals =
{
  30,
  3,
  PICMAN_ORIENTATION_HORIZONTAL,
  FALSE
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",       "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",          "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable",       "Input drawable" },
    { PICMAN_PDB_INT32,    "angle-dsp",      "Angle of Displacement" },
    { PICMAN_PDB_INT32,    "num-segments",   "Number of segments in blinds" },
    { PICMAN_PDB_INT32,    "orientation",    "The orientation { ORIENTATION-HORIZONTAL (0), ORIENTATION-VERTICAL (1) }" },
    { PICMAN_PDB_INT32,    "bg-transparent", "Background transparent { FALSE, TRUE }" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Simulate an image painted on window blinds"),
                          "More here later",
                          "Andy Thomas",
                          "Andy Thomas",
                          "1997",
                          N_("_Blinds..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam values[1];
  PicmanDrawable *drawable;
  PicmanRunMode run_mode;
  PicmanPDBStatusType status = PICMAN_PDB_SUCCESS;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  drawable = picman_drawable_get (param[2].data.d_drawable);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      picman_get_data (PLUG_IN_PROC, &bvals);
      if (! blinds_dialog (drawable))
        {
          picman_drawable_detach (drawable);
          return;
        }
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      if (nparams != 7)
        status = PICMAN_PDB_CALLING_ERROR;
      if (status == PICMAN_PDB_SUCCESS)
        {
          bvals.angledsp    = param[3].data.d_int32;
          bvals.numsegs     = param[4].data.d_int32;
          bvals.orientation = param[5].data.d_int32;
          bvals.bg_trans    = param[6].data.d_int32;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      picman_get_data (PLUG_IN_PROC, &bvals);
      break;

    default:
      break;
    }

  if (picman_drawable_is_rgb (drawable->drawable_id) ||
      picman_drawable_is_gray (drawable->drawable_id))
    {
      picman_progress_init (_("Adding blinds"));

      apply_blinds (drawable);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &bvals, sizeof (BlindVals));
    }
  else
    {
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}


static gboolean
blinds_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *table;
  GtkObject *size_data;
  GtkWidget *toggle;
  GtkWidget *horizontal;
  GtkWidget *vertical;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, TRUE);

  dialog = picman_dialog_new (_("Blinds"), PLUG_IN_ROLE,
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

  preview = picman_aspect_preview_new (drawable, NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  g_signal_connect_swapped (preview, "invalidated",
                            G_CALLBACK (dialog_update_preview),
                            drawable);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  frame =
    picman_int_radio_group_new (TRUE, _("Orientation"),
                              G_CALLBACK (picman_radio_button_update),
                              &bvals.orientation, bvals.orientation,

                              _("_Horizontal"), PICMAN_ORIENTATION_HORIZONTAL,
                              &horizontal,

                              _("_Vertical"),   PICMAN_ORIENTATION_VERTICAL,
                              &vertical,

                              NULL);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  g_signal_connect_swapped (horizontal, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);
  g_signal_connect_swapped (vertical, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  frame = picman_frame_new (_("Background"));
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  toggle = gtk_check_button_new_with_mnemonic (_("_Transparent"));
  gtk_container_add (GTK_CONTAINER (frame), toggle);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &bvals.bg_trans);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), bvals.bg_trans);

  if (!picman_drawable_has_alpha (drawable->drawable_id))
    {
      gtk_widget_set_sensitive (toggle, FALSE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), FALSE);
    }

  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  size_data = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                                    _("_Displacement:"), SCALE_WIDTH, 0,
                                    bvals.angledsp, 1, 90, 1, 15, 0,
                                    TRUE, 0, 0,
                                    NULL, NULL);
  g_signal_connect (size_data, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &bvals.angledsp);
  g_signal_connect_swapped (size_data, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  size_data = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                                    _("_Number of segments:"), SCALE_WIDTH, 0,
                                    bvals.numsegs, 1, MAX_FANS, 1, 2, 0,
                                    TRUE, 0, 0,
                                    NULL, NULL);
  g_signal_connect (size_data, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &bvals.numsegs);
  g_signal_connect_swapped (size_data, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
blindsapply (guchar *srow,
             guchar *drow,
             gint    width,
             gint    bpp,
             guchar *bg)
{
  guchar  *src;
  guchar  *dst;
  gint     i,j,k;
  gdouble  ang;
  gint     available;

  /* Make the row 'shrink' around points along its length */
  /* The bvals.numsegs determins how many segments to slip it in to */
  /* The angle is the conceptual 'rotation' of each of these segments */

  /* Note the row is considered to be made up of a two dim array actual
   * pixel locations and the RGB colour at these locations.
   */

  /* In the process copy the src row to the destination row */

  /* Fill in with background color ? */
  for (i = 0 ; i < width ; i++)
    {
      dst = &drow[i*bpp];

      for (j = 0 ; j < bpp; j++)
        {
          dst[j] = bg[j];
        }
    }

  /* Apply it */

  available = width;
  for (i = 0; i < bvals.numsegs; i++)
    {
      /* Width of segs are variable */
      fanwidths[i] = available / (bvals.numsegs - i);
      available -= fanwidths[i];
    }

  /* do center points  first - just for fun...*/
  available = 0;
  for (k = 1; k <= bvals.numsegs; k++)
    {
      int point;

      point = available + fanwidths[k - 1] / 2;

      available += fanwidths[k - 1];

      src = &srow[point * bpp];
      dst = &drow[point * bpp];

      /* Copy pixels across */
      for (j = 0 ; j < bpp; j++)
        {
          dst[j] = src[j];
        }
    }

  /* Disp for each point */
  ang = (bvals.angledsp * 2 * G_PI) / 360; /* Angle in rads */
  ang = (1 - fabs (cos (ang)));

  available = 0;
  for (k = 0 ; k < bvals.numsegs; k++)
    {
      int dx; /* Amount to move by */
      int fw;

      for (i = 0 ; i < (fanwidths[k]/2) ; i++)
        {
          /* Copy pixels across of left half of fan */
          fw = fanwidths[k] / 2;
          dx = (int) (ang * ((double) (fw - (double)(i % fw))));

          src = &srow[(available + i) * bpp];
          dst = &drow[(available + i + dx) * bpp];

          for (j = 0; j < bpp; j++)
            {
              dst[j] = src[j];
            }

          /* Right side */
          j = i + 1;
          src = &srow[(available + fanwidths[k] - j
                       - (fanwidths[k] % 2)) * bpp];
          dst = &drow[(available + fanwidths[k] - j
                       - (fanwidths[k] % 2) - dx) * bpp];

          for (j = 0; j < bpp; j++)
            {
              dst[j] = src[j];
            }
        }

      available += fanwidths[k];
    }
}

static void
dialog_update_preview (PicmanDrawable *drawable,
                       PicmanPreview  *preview)
{
  gint     y;
  guchar  *p, *buffer, *cache;
  PicmanRGB  background;
  guchar   bg[4];
  gint     width, height, bpp;

  picman_preview_get_size (preview, &width, &height);
  bpp = picman_drawable_bpp (drawable->drawable_id);
  cache = picman_drawable_get_thumbnail_data (drawable->drawable_id,
                                            &width, &height, &bpp);
  p = cache;

  picman_context_get_background (&background);

  if (bvals.bg_trans)
    picman_rgb_set_alpha (&background, 0.0);

  picman_drawable_get_color_uchar (drawable->drawable_id, &background, bg);

  buffer = g_new (guchar, width * height * bpp);

  if (bvals.orientation == PICMAN_ORIENTATION_VERTICAL)
    {
      for (y = 0; y < height; y++)
        {
          blindsapply (p,
                       buffer + y * width * bpp,
                       width,
                       bpp, bg);
          p += width * bpp;
        }
    }
  else
    {
      /* Horizontal blinds */
      /* Apply the blinds algo to a single column -
       * this act as a transfomation matrix for the
       * rows. Make row 0 invalid so we can find it again!
       */
      gint i;
      guchar *sr = g_new (guchar, height * 4);
      guchar *dr = g_new0 (guchar, height * 4);
      guchar dummybg[4] = {0, 0, 0, 0};

      /* Fill in with background color ? */
      for (i = 0 ; i < width ; i++)
        {
          gint j;
          gint bd = bpp;
          guchar *dst;
          dst = &buffer[i * bd];

          for (j = 0 ; j < bd; j++)
            {
              dst[j] = bg[j];
            }
        }

      for ( y = 0 ; y < height; y++)
        {
          sr[y] = y+1;
        }

      /* Bit of a fiddle since blindsapply really works on an image
       * row not a set of bytes. - preview can't be > 255
       * or must make dr sr int rows.
       */
      blindsapply (sr, dr, height, 1, dummybg);

      for (y = 0; y < height; y++)
        {
          if (dr[y] == 0)
            {
              /* Draw background line */
              p = buffer;
            }
          else
            {
              /* Draw line from src */
              p = cache +
                (width * bpp * (dr[y] - 1));
            }
          memcpy (buffer + y * width * bpp,
                  p,
                  width * bpp);
        }
      g_free (sr);
      g_free (dr);
    }

  picman_preview_draw_buffer (preview, buffer, width * bpp);

  g_free (buffer);
  g_free (cache);
}

/* STEP tells us how many rows/columns to gulp down in one go... */
/* Note all the "4" literals around here are to do with the depths
 * of the images. Makes it easier to deal with for my small brain.
 */

#define STEP 40

static void
apply_blinds (PicmanDrawable *drawable)
{
  PicmanPixelRgn  des_rgn;
  PicmanPixelRgn  src_rgn;
  guchar       *src_rows, *des_rows;
  gint          x, y;
  PicmanRGB       background;
  guchar        bg[4];
  gint          sel_x1, sel_y1;
  gint          sel_width, sel_height;

  picman_context_get_background (&background);

  if (bvals.bg_trans)
    picman_rgb_set_alpha (&background, 0.0);

  picman_drawable_get_color_uchar (drawable->drawable_id, &background, bg);

  if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                      &sel_x1, &sel_y1,
                                      &sel_width, &sel_height))
    return;

  picman_pixel_rgn_init (&src_rgn, drawable,
                       sel_x1, sel_y1, sel_width, sel_height, FALSE, FALSE);
  picman_pixel_rgn_init (&des_rgn, drawable,
                       sel_x1, sel_y1, sel_width, sel_height, TRUE, TRUE);

  src_rows = g_new (guchar, MAX (sel_width, sel_height) * 4 * STEP);
  des_rows = g_new (guchar, MAX (sel_width, sel_height) * 4 * STEP);

  if (bvals.orientation == PICMAN_ORIENTATION_VERTICAL)
    {
      for (y = 0; y < sel_height; y += STEP)
        {
          gint rr;
          gint step;

          if((y + STEP) > sel_height)
            step = sel_height - y;
          else
            step = STEP;

          picman_pixel_rgn_get_rect (&src_rgn,
                                   src_rows,
                                   sel_x1,
                                   sel_y1 + y,
                                   sel_width,
                                   step);

          /* OK I could make this better */
          for (rr = 0; rr < STEP; rr++)
            blindsapply (src_rows + (sel_width * rr * src_rgn.bpp),
                         des_rows + (sel_width * rr * src_rgn.bpp),
                         sel_width, src_rgn.bpp, bg);

          picman_pixel_rgn_set_rect (&des_rgn,
                                   des_rows,
                                   sel_x1,
                                   sel_y1 + y,
                                   sel_width,
                                   step);

          picman_progress_update ((double) y / (double) sel_height);
        }
    }
  else
    {
      /* Horizontal blinds */
      /* Apply the blinds algo to a single column -
       * this act as a transfomation matrix for the
       * rows. Make row 0 invalid so we can find it again!
       */
      gint    i;
      gint   *sr  = g_new (gint, sel_height * 4);
      gint   *dr  = g_new (gint, sel_height * 4);
      guchar *dst = g_new (guchar, STEP * 4);
      guchar  dummybg[4];

      memset (dummybg, 0, 4);
      memset (dr, 0, sel_height * 4); /* all dr rows are background rows */
      for (y = 0; y < sel_height; y++)
        {
          sr[y] = y+1;
        }

      /* Hmmm. does this work portably? */
      /* This "swaps the intergers around that are held in in the
       * sr & dr arrays.
       */
      blindsapply ((guchar *) sr, (guchar *) dr,
                   sel_height, sizeof (gint), dummybg);

      /* Fill in with background color ? */
      for (i = 0 ; i < STEP ; i++)
        {
          int     j;
          guchar *bgdst;
          bgdst = &dst[i * src_rgn.bpp];

          for (j = 0 ; j < src_rgn.bpp; j++)
            {
              bgdst[j] = bg[j];
            }
        }

      for (x = 0; x < sel_width; x += STEP)
        {
          int     rr;
          int     step;
          guchar *p;

          if((x + STEP) > sel_width)
            step = sel_width - x;
          else
            step = STEP;

          picman_pixel_rgn_get_rect (&src_rgn,
                                   src_rows,
                                   sel_x1 + x,
                                   sel_y1,
                                   step,
                                   sel_height);

          /* OK I could make this better */
          for (rr = 0; rr < sel_height; rr++)
            {
              if(dr[rr] == 0)
                {
                  /* Draw background line */
                  p = dst;
                }
              else
                {
                  /* Draw line from src */
                  p = src_rows + (step * src_rgn.bpp * (dr[rr] - 1));
                }
              memcpy (des_rows + (rr * step * src_rgn.bpp), p,
                      step * src_rgn.bpp);
            }

          picman_pixel_rgn_set_rect (&des_rgn,
                                   des_rows,
                                   sel_x1 + x,
                                   sel_y1,
                                   step,
                                   sel_height);

          picman_progress_update ((double) x / (double) sel_width);
        }

      g_free (dst);
      g_free (sr);
      g_free (dr);
    }

  g_free (src_rows);
  g_free (des_rows);

  picman_progress_update (1.0);
  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id,
                        sel_x1, sel_y1, sel_width, sel_height);
}
