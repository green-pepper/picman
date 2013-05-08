/*
 * This is a plugin for PICMAN.
 *
 * Copyright (C) 1996 Stephen Norris
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

/*
 * This plug-in produces plasma fractal images. The algorithm is losely
 * based on a description of the fractint algorithm, but completely
 * re-implemented because the fractint code was too ugly to read :)
 *
 * Please send any patches or suggestions to me: srn@flibble.cs.su.oz.au.
 */

/*
 * TODO:
 *      - The progress bar sucks.
 *      - It writes some pixels more than once.
 */

/* Version 1.01 */

/*
 * Ported to PICMAN Plug-in API 1.0
 *    by Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 *
 * $Id$
 *
 * A few functions names and their order are changed :)
 * Plasma implementation almost hasn't been changed.
 *
 * Feel free to correct my WRONG English, or to modify Plug-in Path,
 * and so on. ;-)
 *
 * Version 1.02
 *
 * May 2000
 * tim copperfield [timecop@japan.co.jp]
 * Added dynamic preview mode.
 *
 */

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


/* Some useful macros */

#define PLUG_IN_PROC     "plug-in-plasma"
#define PLUG_IN_BINARY   "plasma"
#define PLUG_IN_ROLE     "picman-plasma"
#define SCALE_WIDTH      48
#define TILE_CACHE_SIZE  32


typedef struct
{
  guint32   seed;
  gdouble   turbulence;
  gboolean  random_seed;
} PlasmaValues;


/*
 * Function prototypes.
 */

static void       query (void);
static void       run   (const gchar      *name,
                         gint              nparams,
                         const PicmanParam  *param,
                         gint             *nreturn_vals,
                         PicmanParam       **return_vals);

static gboolean   plasma_dialog          (PicmanDrawable  *drawable);
static void plasma_seed_changed_callback (PicmanDrawable  *drawable,
                                          gpointer       data);

static void     plasma       (PicmanDrawable *drawable,
                              gboolean      preview_mode);
static void     random_rgb   (GRand        *gr,
                              guchar       *pixel);
static void     add_random   (GRand        *gr,
                              guchar       *pixel,
                              gint          amount);
static PicmanPixelFetcher *init_plasma  (PicmanDrawable *drawable,
                                       gboolean      preview_mode,
                                       GRand        *gr);
static void     end_plasma   (PicmanDrawable     *drawable,
                              PicmanPixelFetcher *pft,
                              GRand            *gr);
static void     get_pixel    (PicmanPixelFetcher *pft,
                              gint              x,
                              gint              y,
                              guchar           *pixel);
static void     put_pixel    (PicmanPixelFetcher *pft,
                              gint              x,
                              gint              y,
                              guchar           *pixel);
static gboolean do_plasma    (PicmanPixelFetcher *pft,
                              gint              x1,
                              gint              y1,
                              gint              x2,
                              gint              y2,
                              gint              depth,
                              gint              scale_depth,
                              GRand            *gr);


/***** Local vars *****/

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static PlasmaValues pvals =
{
  0,     /* seed            */
  1.0,   /* turbulence      */
  FALSE  /* Use random seed */
};

/*
 * Some globals to save passing too many paramaters that don't change.
 */
static GtkWidget *preview;
static guchar    *preview_buffer;
static gint       preview_width, preview_height;

static gint       ix1, iy1, ix2, iy2;     /* Selected image size. */
static gint       bpp, alpha;
static gboolean   has_alpha;
static glong      max_progress, progress;

/***** Functions *****/

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[]=
  {
    { PICMAN_PDB_INT32,    "run-mode",   "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",      "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable",   "Input drawable"       },
    { PICMAN_PDB_INT32,    "seed",       "Random seed"          },
    { PICMAN_PDB_FLOAT,    "turbulence", "Turbulence of plasma" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Create a random plasma texture"),
                          "More help",
                          "Stephen Norris & (ported to 1.0 by) Eiichi Takamori",
                          "Stephen Norris",
                          "May 2000",
                          N_("_Plasma..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Render/Clouds");
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
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &pvals);

      /*  First acquire information with a dialog  */
      if (! plasma_dialog (drawable))
        {
          picman_drawable_detach (drawable);
          return;
        }
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 5)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      else
        {
          pvals.seed       = (guint32) param[3].data.d_int32;
          pvals.turbulence = (gdouble) param[4].data.d_float;

          if (pvals.turbulence <= 0)
            status = PICMAN_PDB_CALLING_ERROR;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &pvals);

      if (pvals.random_seed)
        pvals.seed = g_random_int ();
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
          picman_progress_init (_("Plasma"));
          picman_tile_cache_ntiles (TILE_CACHE_SIZE);

          plasma (drawable, FALSE);

          if (run_mode != PICMAN_RUN_NONINTERACTIVE)
            picman_displays_flush ();

          /*  Store data  */
          if (run_mode == PICMAN_RUN_INTERACTIVE ||
              (run_mode == PICMAN_RUN_WITH_LAST_VALS))
            picman_set_data (PLUG_IN_PROC, &pvals, sizeof (PlasmaValues));
        }
      else
        {
          /* picman_message ("plasma: cannot operate on indexed color images"); */
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
    }

  values[0].data.d_status = status;
  picman_drawable_detach (drawable);
}

static gboolean
plasma_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *label;
  GtkWidget *table;
  GtkWidget *seed;
  GtkObject *adj;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, TRUE);

  dialog = picman_dialog_new (_("Plasma"), PLUG_IN_ROLE,
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
                            G_CALLBACK (plasma_seed_changed_callback),
                            drawable);

  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  seed = picman_random_seed_new (&pvals.seed, &pvals.random_seed);
  label = picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                                     _("Random _seed:"), 0.0, 0.5,
                                     seed, 2, TRUE);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                 PICMAN_RANDOM_SEED_SPINBUTTON (seed));

  g_signal_connect_swapped (PICMAN_RANDOM_SEED_SPINBUTTON_ADJ (seed),
                            "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                              _("T_urbulence:"), SCALE_WIDTH, 0,
                              pvals.turbulence,
                              0.1, 7.0, 0.1, 1.0, 1,
                              TRUE, 0, 0,
                              NULL, NULL);
  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &pvals.turbulence);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
plasma_seed_changed_callback (PicmanDrawable *drawable,
                              gpointer      data)
{
  plasma (drawable, TRUE);
}

/*
 * The setup function.
 */

static void
plasma (PicmanDrawable *drawable,
        gboolean      preview_mode)
{
  PicmanPixelFetcher *pft;
  gint              depth;
  GRand            *gr;

  gr = g_rand_new ();

  pft = init_plasma (drawable, preview_mode, gr);

  if (!preview_mode && !pft)
    return;

  if (ix1 != ix2 && iy1 != iy2)
    {
      /*
       * This first time only puts in the seed pixels - one in each
       * corner, and one in the center of each edge, plus one in the
       * center of the image.
       */

      do_plasma (pft, ix1, iy1, ix2 - 1, iy2 - 1, -1, 0, gr);

      /*
       * Now we recurse through the images, going further each time.
       */
      depth = 1;
      while (!do_plasma (pft, ix1, iy1, ix2 - 1, iy2 - 1, depth, 0, gr))
        {
          depth++;
        }
      if (pft)
        picman_progress_update (1.0);
    }

  end_plasma (drawable, pft, gr);
}

static PicmanPixelFetcher*
init_plasma (PicmanDrawable *drawable,
             gboolean      preview_mode,
             GRand        *gr)
{
  PicmanPixelFetcher *pft;

  g_rand_set_seed (gr, pvals.seed);

  if (preview_mode)
    {
      ix1 = iy1 = 0;
      picman_preview_get_size (PICMAN_PREVIEW (preview),
                             &preview_width, &preview_height);
      ix2 = preview_width;
      iy2 = preview_height;
      preview_buffer = g_new (guchar, ix2 * iy2 * drawable->bpp);

      pft = NULL;
    }
  else if (picman_drawable_mask_intersect (drawable->drawable_id,
                                         &ix1, &iy1, &ix2, &iy2))
    {
      ix2 += ix1;
      iy2 += iy1;
      pft = picman_pixel_fetcher_new (drawable, TRUE);
    }
  else
    {
      return NULL;
    }

  bpp       = drawable->bpp;
  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  alpha     = (has_alpha) ? bpp - 1 : bpp;

  progress     = 0;
  max_progress = (ix2 - ix1) * (iy2 - iy1);

  return pft;
}

static void
end_plasma (PicmanDrawable     *drawable,
            PicmanPixelFetcher *pft,
            GRand            *gr)
{
  if (pft)
    {
      picman_pixel_fetcher_destroy (pft);

      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id,
                            ix1, iy1, ix2 - ix1, iy2 - iy1);
    }
  else
    {
      picman_preview_draw_buffer (PICMAN_PREVIEW (preview),
                                preview_buffer, preview_width * bpp);
      g_free (preview_buffer);
    }

  g_rand_free (gr);
}

static void
get_pixel (PicmanPixelFetcher *pft,
           gint              x,
           gint              y,
           guchar           *pixel)
{
  if (pft)
    {
      picman_pixel_fetcher_get_pixel (pft, x, y, pixel);
    }
  else
    {
      memcpy (pixel, preview_buffer + (y * preview_width + x) * bpp, bpp);
    }
}

static void
put_pixel (PicmanPixelFetcher *pft,
           gint              x,
           gint              y,
           guchar           *pixel)
{
  if (pft)
    {
      picman_pixel_fetcher_put_pixel (pft, x, y, pixel);
      progress++;
    }
  else
    {
      memcpy (preview_buffer + (y * preview_width + x) * bpp, pixel, bpp);
    }
}

static void
average_pixel (guchar *dest,
               const guchar *src1,
               const guchar *src2,
               gint bpp)
{
  for (; bpp; bpp--)
    {
      *dest++ = (*src1++ + *src2++) / 2;
    }
}

static void
random_rgb (GRand  *gr,
            guchar *pixel)
{
  gint i;

  for (i = 0; i < alpha; i++)
    pixel[i] = g_rand_int_range (gr, 0, 256);

  if (has_alpha)
    pixel[alpha] = 255;
}

static void
add_random (GRand  *gr,
            guchar *pixel,
            gint    amount)
{
  amount /= 2;

  if (amount > 0)
    {
      gint i, tmp;

      for (i = 0; i < alpha; i++)
        {
          tmp = pixel[i] + g_rand_int_range (gr, - amount, amount);

          pixel[i] = CLAMP0255 (tmp);
        }
    }
}

static gboolean
do_plasma (PicmanPixelFetcher *pft,
           gint              x1,
           gint              y1,
           gint              x2,
           gint              y2,
           gint              depth,
           gint              scale_depth,
           GRand            *gr)
{
  guchar  tl[4], ml[4], bl[4], mt[4], mm[4], mb[4], tr[4], mr[4], br[4];
  guchar  tmp[4];
  gint    xm, ym;

  static gint count = 0;

  xm = (x1 + x2) / 2;
  ym = (y1 + y2) / 2;

  /* Initial pass through - no averaging. */

  if (depth == -1)
    {
      random_rgb (gr, tl);
      put_pixel (pft, x1, y1, tl);
      random_rgb (gr, tr);
      put_pixel (pft, x2, y1, tr);
      random_rgb (gr, bl);
      put_pixel (pft, x1, y2, bl);
      random_rgb (gr, br);
      put_pixel (pft, x2, y2, br);
      random_rgb (gr, mm);
      put_pixel (pft, xm, ym, mm);
      random_rgb (gr, ml);
      put_pixel (pft, x1, ym, ml);
      random_rgb (gr, mr);
      put_pixel (pft, x2, ym, mr);
      random_rgb (gr, mt);
      put_pixel (pft, xm, y1, mt);
      random_rgb (gr, mb);
      put_pixel (pft, xm, y2, mb);

      return FALSE;
    }

  /*
   * Some later pass, at the bottom of this pass,
   * with averaging at this depth.
   */
  if (depth == 0)
    {
      gint ran;

      if (x1 == x2 && y1 == y2)
        {
          return FALSE;
        }

      get_pixel (pft, x1, y1, tl);
      get_pixel (pft, x1, y2, bl);
      get_pixel (pft, x2, y1, tr);
      get_pixel (pft, x2, y2, br);

      ran = (gint) ((256.0 / (2.0 * scale_depth)) * pvals.turbulence);

      if (xm != x1 || xm != x2)
        {
          /* Left. */
          average_pixel (ml, tl, bl, bpp);
          add_random (gr, ml, ran);
          put_pixel (pft, x1, ym, ml);

          if (x1 != x2)
            {
              /* Right. */
              average_pixel (mr, tr, br, bpp);
              add_random (gr, mr, ran);
              put_pixel (pft, x2, ym, mr);
            }
        }

      if (ym != y1 || ym != y2)
        {
          if (x1 != xm || ym != y2)
            {
              /* Bottom. */
              average_pixel (mb, bl, br, bpp);
              add_random (gr, mb, ran);
              put_pixel (pft, xm, y2, mb);
            }

          if (y1 != y2)
            {
              /* Top. */
              average_pixel (mt, tl, tr, bpp);
              add_random (gr, mt, ran);
              put_pixel (pft, xm, y1, mt);
            }
        }

      if (y1 != y2 || x1 != x2)
        {
          /* Middle pixel. */
          average_pixel (mm, tl, br, bpp);
          average_pixel (tmp, bl, tr, bpp);
          average_pixel (mm, mm, tmp, bpp);

          add_random (gr, mm, ran);
          put_pixel (pft, xm, ym, mm);
        }

      count++;

      if (!(count % 2000) && pft)
        {
          picman_progress_update ((gdouble) progress / (gdouble) max_progress);
        }

      return x2 - x1 < 3 && y2 - y1 < 3;
    }

  if (x1 < x2 || y1 < y2)
    {
      /* Top left. */
      do_plasma (pft, x1, y1, xm, ym, depth - 1, scale_depth + 1, gr);
      /* Bottom left. */
      do_plasma (pft, x1, ym, xm ,y2, depth - 1, scale_depth + 1, gr);
      /* Top right. */
      do_plasma (pft, xm, y1, x2 , ym, depth - 1, scale_depth + 1, gr);
      /* Bottom right. */
      return do_plasma (pft, xm, ym, x2, y2, depth - 1, scale_depth + 1, gr);
    }
  else
    {
      return TRUE;
    }
}
