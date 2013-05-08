/* edge filter for PICMAN
 *  -Peter Mattis
 *
 * This filter performs edge detection on the input image.
 *  The code for this filter is based on "pgmedge", a program
 *  that is part of the netpbm package.
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

/* pgmedge.c - edge-detect a portable graymap
 *
 * Copyright (C) 1989 by Jef Poskanzer.
 */

/*
 *  Ported to PICMAN Plug-in API 1.0
 *  version 1.07
 *  This version requires PICMAN v0.99.10 or above.
 *
 *  This plug-in performs edge detection. The code is based on edge.c
 *  for PICMAN 0.54 by Peter Mattis.
 *
 *      Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 *      http://ha1.seikyou.ne.jp/home/taka/picman/
 *
 *  Tips: you can enter arbitrary value into entry.
 *      (not bounded between 1.0 and 10.0)
 */

/*  29 July 2003   Dave Neary  <bolsh@picman.org>
 *  Added more edge detection routines, from the thin_line
 *  plug-in by iccii
 */

#include "config.h"

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


/* Some useful macros */

#define PLUG_IN_PROC    "plug-in-edge"
#define PLUG_IN_BINARY  "edge"
#define PLUG_IN_ROLE    "picman-edge"
#define TILE_CACHE_SIZE 48

enum
{
  SOBEL,
  PREWITT,
  GRADIENT,
  ROBERTS,
  DIFFERENTIAL,
  LAPLACE
};

typedef struct
{
  gdouble  amount;
  gint     edgemode;
  gint     wrapmode;
} EdgeVals;

/*
 * Function prototypes.
 */

static void       query               (void);
static void       run                 (const gchar      *name,
                                       gint              nparams,
                                       const PicmanParam  *param,
                                       gint             *nreturn_vals,
                                       PicmanParam       **return_vals);

static void       edge                (PicmanDrawable     *drawable);
static gboolean   edge_dialog         (PicmanDrawable     *drawable);
static void       edge_preview_update (PicmanPreview      *preview);

static gint       edge_detect         (const guchar     *data);
static gint       prewitt             (const guchar     *data);
static gint       gradient            (const guchar     *data);
static gint       roberts             (const guchar     *data);
static gint       differential        (const guchar     *data);
static gint       laplace             (const guchar     *data);
static gint       sobel               (const guchar     *data);


/***** Local vars *****/

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init  */
  NULL,  /* quit  */
  query, /* query */
  run,   /* run   */
};

static EdgeVals evals =
{
  2.0,                           /* amount */
  SOBEL,                         /* Edge detection algorithm */
  PICMAN_PIXEL_FETCHER_EDGE_SMEAR  /* wrapmode */
};

/***** Functions *****/

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",    "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable", "Input drawable" },
    { PICMAN_PDB_FLOAT,    "amount",   "Edge detection amount" },
    { PICMAN_PDB_INT32,    "wrapmode", "Edge detection behavior { WRAP (1), SMEAR (2), BLACK (3) }" },
    { PICMAN_PDB_INT32,    "edgemode", "Edge detection algorithm { SOBEL (0), PREWITT (1), GRADIENT (2), ROBERTS (3), DIFFERENTIAL (4), LAPLACE (5) }" }
  };

  const gchar *help_string =
    "Perform edge detection on the contents of the specified drawable."
    "AMOUNT is an arbitrary constant, WRAPMODE is like displace plug-in "
    "(useful for tilable image). EDGEMODE sets the kind of matrix "
    "transform applied to the pixels, SOBEL was the method used in older "
    "versions.";

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Several simple methods for detecting edges"),
                          help_string,
                          "Peter Mattis & (ported to 1.0 by) Eiichi Takamori",
                          "Peter Mattis",
                          "1996",
                          N_("_Edge..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Edge-Detect");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  PicmanDrawable      *drawable;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &evals);

      /*  First acquire information with a dialog  */
      if (! edge_dialog (drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 5 && nparams != 6)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      if (status == PICMAN_PDB_SUCCESS)
        {
          evals.amount   = param[3].data.d_float;
          evals.wrapmode = param[4].data.d_int32;
          evals.edgemode = nparams > 5 ? param[5].data.d_int32 : SOBEL;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &evals);
      break;

    default:
      break;
    }

  /* make sure the drawable exist and is not indexed */
  if (picman_drawable_is_rgb (drawable->drawable_id) ||
      picman_drawable_is_gray (drawable->drawable_id))
    {
      picman_progress_init (_("Edge detection"));

      /*  set the tile cache size  */
      picman_tile_cache_ntiles (TILE_CACHE_SIZE);

      /*  run the edge effect  */
      edge (drawable);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      /*  Store data  */
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &evals, sizeof (EdgeVals));
    }
  else
    {
      /* picman_message ("edge: cannot operate on indexed color images"); */
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

/********************************************************/
/*              Edge Detection main                     */
/********************************************************/

static void
edge (PicmanDrawable *drawable)
{
  PicmanPixelRgn      src_rgn, dest_rgn;
  gpointer          pr;
  PicmanPixelFetcher *pft;
  guchar           *srcrow, *src;
  guchar           *destrow, *dest;
  guchar            pix00[4], pix01[4], pix02[4];
  guchar            pix10[4], pix11[4], pix12[4];
  guchar            pix20[4], pix21[4], pix22[4];
  gint              alpha;
  gboolean          has_alpha;
  gint              chan;
  gint              x, y;
  gint              x1, y1, x2, y2;
  gint              cur_progress;
  gint              max_progress;
  gdouble           per_progress;

  if (evals.amount < 1.0)
    evals.amount = 1.0;

  pft = picman_pixel_fetcher_new (drawable, FALSE);
  picman_pixel_fetcher_set_edge_mode (pft, evals.wrapmode);

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);

  alpha     = picman_drawable_bpp (drawable->drawable_id);
  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  if (has_alpha)
    alpha--;

  cur_progress = 0;
  per_progress = 0.0;
  max_progress = (x2 - x1) * (y2 - y1) / 100;

  picman_pixel_rgn_init (&src_rgn, drawable, x1, y1, x2-x1, y2-y1, FALSE, FALSE);
  picman_pixel_rgn_init (&dest_rgn, drawable, x1, y1, x2-x1, y2-y1, TRUE, TRUE);

  for (pr = picman_pixel_rgns_register (2, &src_rgn, &dest_rgn);
       pr != NULL;
       pr = picman_pixel_rgns_process (pr))
    {
      srcrow  = src_rgn.data;
      destrow = dest_rgn.data;

      for (y = dest_rgn.y;
           y < (dest_rgn.y + dest_rgn.h);
           y++, srcrow += src_rgn.rowstride, destrow += dest_rgn.rowstride)
        {
          src  = srcrow;
          dest = destrow;

          for (x = dest_rgn.x;
               x < (dest_rgn.x + dest_rgn.w);
               x++,  src += src_rgn.bpp, dest += dest_rgn.bpp)
            {
              if (dest_rgn.x < x &&  x < dest_rgn.x + dest_rgn.w - 2 &&
                  dest_rgn.y < y &&  y < dest_rgn.y + dest_rgn.h - 2)
                {
                  /*
                   * 3x3 kernel is inside of the tile -- do fast
                   * version
                   */
                  for (chan = 0; chan < alpha; chan++)
                    {
                      /* get the 3x3 kernel into a guchar array,
                       * and send it to edge_detect */
                      guchar kernel[9];
                      gint   i,j;

#define PIX(X,Y)  src[ (Y-1)*(int)src_rgn.rowstride + (X-1)*(int)src_rgn.bpp + chan ]
                      /* make convolution */
                      for(i = 0; i < 3; i++)
                        for(j = 0; j < 3; j++)
                          kernel[3*i + j] = PIX(i,j);

#undef  PIX

                      dest[chan] = edge_detect (kernel);
                    }
                }
              else
                {
                  /*
                   * The kernel is not inside of the tile -- do slow
                   * version
                   */

                  picman_pixel_fetcher_get_pixel (pft, x-1, y-1, pix00);
                  picman_pixel_fetcher_get_pixel (pft, x  , y-1, pix10);
                  picman_pixel_fetcher_get_pixel (pft, x+1, y-1, pix20);
                  picman_pixel_fetcher_get_pixel (pft, x-1, y  , pix01);
                  picman_pixel_fetcher_get_pixel (pft, x  , y  , pix11);
                  picman_pixel_fetcher_get_pixel (pft, x+1, y  , pix21);
                  picman_pixel_fetcher_get_pixel (pft, x-1, y+1, pix02);
                  picman_pixel_fetcher_get_pixel (pft, x  , y+1, pix12);
                  picman_pixel_fetcher_get_pixel (pft, x+1, y+1, pix22);

                  for (chan = 0; chan < alpha; chan++)
                    {
                      guchar kernel[9];

                      kernel[0] = pix00[chan];
                      kernel[1] = pix01[chan];
                      kernel[2] = pix02[chan];
                      kernel[3] = pix10[chan];
                      kernel[4] = pix11[chan];
                      kernel[5] = pix12[chan];
                      kernel[6] = pix20[chan];
                      kernel[7] = pix21[chan];
                      kernel[8] = pix22[chan];

                      dest[chan] = edge_detect (kernel);
                    }
                }
              if (has_alpha)
                dest[alpha] = src[alpha];
            }
        }
      cur_progress += dest_rgn.w * dest_rgn.h;

      if (cur_progress > max_progress)
        {
          cur_progress = cur_progress - max_progress;
          per_progress = per_progress + 0.01;
          picman_progress_update (per_progress);
        }
    }

  picman_progress_update (1.0);

  picman_pixel_fetcher_destroy (pft);

  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x1, y1, (x2 - x1), (y2 - y1));
}

/* ***********************   Edge Detection   ******************** */


/*
 * Edge detect switcher function
 */

static gint
edge_detect (const guchar *data)
{
  gint ret;

  switch (evals.edgemode)
    {
    case SOBEL:
      ret = sobel (data);
      break;
    case PREWITT:
      ret = prewitt (data);
      break;
    case GRADIENT:
      ret = gradient (data);
      break;
    case ROBERTS:
      ret = roberts (data);
      break;
    case DIFFERENTIAL:
      ret = differential (data);
      break;
    case LAPLACE:
      ret = laplace (data);
      break;
    default:
      ret = -1;
      break;
    }

  return CLAMP0255 (ret);
}


/*
 * Sobel Edge detector
 */
static gint
sobel (const guchar *data)
{
  const gint v_kernel[9] = { -1,  0,  1,
                             -2,  0,  2,
                             -1,  0,  1 };
  const gint h_kernel[9] = { -1, -2, -1,
                              0,  0,  0,
                              1,  2,  1 };

  gint i;
  gint v_grad, h_grad;

  for (i = 0, v_grad = 0, h_grad = 0; i < 9; i++)
    {
      v_grad += v_kernel[i] * data[i];
      h_grad += h_kernel[i] * data[i];
    }

  return sqrt (v_grad * v_grad * evals.amount +
               h_grad * h_grad * evals.amount);
}

/*
 * Edge detector via template matting
 *   -- Prewitt Compass
 */
static gint
prewitt (const guchar *data)
{
  gint k, max;
  gint m[8];

  m[0] =   data [0] +   data [1] + data [2]
         + data [3] - 2*data [4] + data [5]
         - data [6] -   data [7] - data [8];
  m[1] =   data [0] +   data [1] + data [2]
         + data [3] - 2*data [4] - data [5]
         + data [6] -   data [7] - data [8];
  m[2] =   data [0] +   data [1] - data [2]
         + data [3] - 2*data [4] - data [5]
         + data [6] +   data [7] - data [8];
  m[3] =   data [0] -   data [1] - data [2]
         + data [3] - 2*data [4] - data [5]
         + data [6] +   data [7] + data [8];
  m[4] = - data [0] -   data [1] - data [2]
         + data [3] - 2*data [4] + data [5]
         + data [6] +   data [7] + data [8];
  m[5] = - data [0] -   data [1] + data [2]
         - data [3] - 2*data [4] + data [5]
         + data [6] +   data [7] + data [8];
  m[6] = - data [0] +   data [1] + data [2]
         - data [3] - 2*data [4] + data [5]
         - data [6] +   data [7] + data [8];
  m[7] =   data [0] +   data [1] + data [2]
         - data [3] - 2*data [4] + data [5]
         - data [6] -   data [7] + data [8];

  for (k = 0, max = 0; k < 8; k++)
    if (max < m[k])
      max = m[k];

  return evals.amount * max;
}

/*
 * Gradient Edge detector
 */
static gint
gradient (const guchar *data)
{
  const gint v_kernel[9] = { 0,  0,  0,
                             0,  4, -4,
                             0,  0,  0 };
  const gint h_kernel[9] = { 0,  0,  0,
                             0, -4,  0,
                             0,  4,  0 };

  gint i;
  gint v_grad, h_grad;

  for (i = 0, v_grad = 0, h_grad = 0; i < 9; i++)
    {
      v_grad += v_kernel[i] * data[i];
      h_grad += h_kernel[i] * data[i];
    }

  return  sqrt (v_grad * v_grad * evals.amount +
                h_grad * h_grad * evals.amount);
}

/*
 * Roberts Edge detector
 */
static gint
roberts (const guchar *data)
{
  const gint v_kernel[9] = { 0,  0,  0,
                             0,  4,  0,
                             0,  0, -4 };
  const gint h_kernel[9] = { 0,  0,  0,
                             0,  0,  4,
                             0, -4,  0 };
  gint i;
  gint v_grad, h_grad;

  for (i = 0, v_grad = 0, h_grad = 0; i < 9; i++)
    {
      v_grad += v_kernel[i] * data[i];
      h_grad += h_kernel[i] * data[i];
    }

  return sqrt (v_grad * v_grad * evals.amount +
               h_grad * h_grad * evals.amount);
}

/*
 * Differential Edge detector
 */
static gint
differential (const guchar *data)
{
  const gint v_kernel[9] = { 0,  0,  0,
                             0,  2, -2,
                             0,  2, -2 };
  const gint h_kernel[9] = { 0,  0,  0,
                             0, -2, -2,
                             0,  2,  2 };
  gint i;
  gint v_grad, h_grad;

  for (i = 0, v_grad = 0, h_grad = 0; i < 9; i++)
    {
      v_grad += v_kernel[i] * data[i];
      h_grad += h_kernel[i] * data[i];
    }

  return sqrt (v_grad * v_grad * evals.amount +
               h_grad * h_grad * evals.amount);
}

/*
 * Laplace Edge detector
 */
static gint
laplace (const guchar *data)
{
  const gint kernel[9] = { 1,  1,  1,
                           1, -8,  1,
                           1,  1,  1 };
  gint i;
  gint grad;

  for (i = 0, grad = 0; i < 9; i++)
    grad += kernel[i] * data[i];

  return grad * evals.amount;
}


/*******************************************************/
/*                    Dialog                           */
/*******************************************************/

static gboolean
edge_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *hbox;
  GtkWidget *table;
  GtkWidget *combo;
  GtkWidget *toggle;
  GtkObject *scale_data;
  GSList    *group = NULL;
  gboolean   run;

  gboolean use_wrap  = (evals.wrapmode == PICMAN_PIXEL_FETCHER_EDGE_WRAP);
  gboolean use_smear = (evals.wrapmode == PICMAN_PIXEL_FETCHER_EDGE_SMEAR);
  gboolean use_black = (evals.wrapmode == PICMAN_PIXEL_FETCHER_EDGE_BLACK);

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Edge Detection"), PLUG_IN_ROLE,
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

  g_signal_connect (preview, "invalidated",
                    G_CALLBACK (edge_preview_update),
                    NULL);

  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  combo = picman_int_combo_box_new (_("Sobel"),           SOBEL,
                                  _("Prewitt compass"), PREWITT,
                                  _("Gradient"),        GRADIENT,
                                  _("Roberts"),         ROBERTS,
                                  _("Differential"),    DIFFERENTIAL,
                                  _("Laplace"),         LAPLACE,
                                  NULL);

  picman_int_combo_box_connect (PICMAN_INT_COMBO_BOX (combo),
                              evals.edgemode,
                              G_CALLBACK (picman_int_combo_box_get_active),
                              &evals.edgemode);

  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("_Algorithm:"), 0.0, 0.5,
                             combo, 2, FALSE);
  g_signal_connect_swapped (combo, "changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  /*  Label, scale, entry for evals.amount  */
  scale_data = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                                     _("A_mount:"), 100, 0,
                                     evals.amount, 1.0, 10.0, 0.1, 1.0, 1,
                                     FALSE, 1.0, G_MAXFLOAT,
                                     NULL, NULL);

  g_signal_connect (scale_data, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &evals.amount);
  g_signal_connect_swapped (scale_data, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  /*  Radio buttons WRAP, SMEAR, BLACK  */

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_table_attach (GTK_TABLE (table), hbox, 0, 3, 2, 3,
                    GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (hbox);

  toggle = gtk_radio_button_new_with_mnemonic (group, _("_Wrap"));
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (hbox), toggle, TRUE, TRUE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), use_wrap);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &use_wrap);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  toggle = gtk_radio_button_new_with_mnemonic (group, _("_Smear"));
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (hbox), toggle, TRUE, TRUE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), use_smear);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &use_smear);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  toggle = gtk_radio_button_new_with_mnemonic (group, _("_Black"));
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (hbox), toggle, TRUE, TRUE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), use_black);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &use_black);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  if (use_wrap)
    evals.wrapmode = PICMAN_PIXEL_FETCHER_EDGE_WRAP;
  else if (use_smear)
    evals.wrapmode = PICMAN_PIXEL_FETCHER_EDGE_SMEAR;
  else if (use_black)
    evals.wrapmode = PICMAN_PIXEL_FETCHER_EDGE_BLACK;

  return run;
}

static void
edge_preview_update (PicmanPreview *preview)
{
  /* drawable */
  PicmanDrawable *drawable;
  glong         bytes;
  gint          alpha;
  gboolean      has_alpha;

  /* preview */
  guchar       *src           = NULL; /* Buffer to hold source image */
  guchar       *render_buffer = NULL; /* Buffer to hold rendered image */
  guchar       *dest;
  gint          width;                /* Width of preview widget */
  gint          height;               /* Height of preview widget */
  gint          x1;                   /* Upper-left X of preview */
  gint          y1;                   /* Upper-left Y of preview */
  PicmanPixelRgn  srcPR;                /* Pixel regions */

  /* algorithm */
  gint x, y;

  /* Get drawable info */
  drawable =
    picman_drawable_preview_get_drawable (PICMAN_DRAWABLE_PREVIEW (preview));
  bytes  = picman_drawable_bpp (drawable->drawable_id);
  alpha  = bytes;
  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  if (has_alpha)
    alpha--;

  /*
   * Setup for filter...
   */
  picman_preview_get_position (preview, &x1, &y1);
  picman_preview_get_size (preview, &width, &height);

  /* initialize pixel regions */
  picman_pixel_rgn_init (&srcPR, drawable,
                       x1, y1, width, height, FALSE, FALSE);
  src = g_new (guchar, width * height * bytes);
  render_buffer = g_new (guchar, width * height * bytes);

  /* render image */
  picman_pixel_rgn_get_rect(&srcPR, src, x1, y1, width, height);
  dest = render_buffer;

  /* render algorithm */
  for (y = 0 ; y < height ; y++)
    for (x = 0 ; x < width ; x++)
      {
        gint chan;
        for (chan = 0; chan < alpha; chan++)
          {
            guchar kernel[9];

#define SRC(X,Y) src[bytes * (CLAMP((X), 0, width-1) + \
                              width * CLAMP((Y), 0, height-1)) + chan]

            kernel[0] = SRC (x - 1, y - 1);
            kernel[1] = SRC (x - 1, y    );
            kernel[2] = SRC (x - 1, y + 1);
            kernel[3] = SRC (x    , y - 1);
            kernel[4] = SRC (x    , y    );
            kernel[5] = SRC (x    , y + 1);
            kernel[6] = SRC (x + 1, y - 1);
            kernel[7] = SRC (x + 1, y    );
            kernel[8] = SRC (x + 1, y + 1);

#undef SRC
            dest[chan] = edge_detect (kernel);
          }
        if (has_alpha)
          dest[alpha] = src[bytes * (x + width * y) + alpha];
        dest += bytes;
      }
  /*
   * Draw the preview image on the screen...
   */
  picman_preview_draw_buffer (preview, render_buffer, width * bytes);

  g_free (render_buffer);
  g_free (src);
}
