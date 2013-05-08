/* Deinterlace 1.00 - image processing plug-in for PICMAN
 *
 * Copyright (C) 1997 Andrew Kieschnick (andrewk@mail.utexas.edu)
 *
 * Original deinterlace for PICMAN 0.54 API by Federico Mena Quintero
 *
 * Copyright (C) 1996 Federico Mena Quintero
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

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC   "plug-in-deinterlace"
#define PLUG_IN_BINARY "deinterlace"
#define PLUG_IN_ROLE   "picman-deinterlace"


enum
{
  ODD_FIELDS,
  EVEN_FIELDS
};

typedef struct
{
  gint     evenness;
} DeinterlaceValues;


/* Declare local functions.
 */
static void      query  (void);
static void      run    (const gchar      *name,
                         gint              nparams,
                         const PicmanParam  *param,
                         gint             *nreturn_vals,
                         PicmanParam       **return_vals);


static void      deinterlace        (PicmanDrawable *drawable,
                                     PicmanPreview  *preview);

static gboolean  deinterlace_dialog (PicmanDrawable *drawable);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static DeinterlaceValues devals =
{
  EVEN_FIELDS   /* evenness */
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,     "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,     "image",    "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE,  "drawable", "Input drawable"       },
    { PICMAN_PDB_INT32,     "evenodd",  "Which lines to keep { KEEP-ODD (0), KEEP-EVEN (1) }" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Fix images where every other row is missing"),
                          "Deinterlace is useful for processing images from "
                          "video capture cards. When only the odd or even "
                          "fields get captured, deinterlace can be used to "
                          "interpolate between the existing fields to correct "
                          "this.",
                          "Andrew Kieschnick",
                          "Andrew Kieschnick",
                          "1997",
                          N_("_Deinterlace..."),
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
  static PicmanParam   values[1];
  PicmanDrawable      *drawable;
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      picman_get_data (PLUG_IN_PROC, &devals);
      if (! deinterlace_dialog (drawable))
        status = PICMAN_PDB_EXECUTION_ERROR;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      if (nparams != 4)
        status = PICMAN_PDB_CALLING_ERROR;
      if (status == PICMAN_PDB_SUCCESS)
        devals.evenness = param[3].data.d_int32;
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      picman_get_data (PLUG_IN_PROC, &devals);
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
          picman_progress_init (_("Deinterlace"));
          picman_tile_cache_ntiles (2 * (drawable->width /
                                       picman_tile_width () + 1));
          deinterlace (drawable, NULL);

          if (run_mode != PICMAN_RUN_NONINTERACTIVE)
            picman_displays_flush ();
          if (run_mode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC, &devals, sizeof (DeinterlaceValues));
        }
      else
        {
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
    }
  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

static void
deinterlace (PicmanDrawable *drawable,
             PicmanPreview  *preview)
{
  PicmanPixelRgn  srcPR, destPR;
  gboolean      has_alpha;
  guchar       *dest;
  guchar       *dest_buffer = NULL;
  guchar       *upper;
  guchar       *lower;
  gint          row, col;
  gint          x, y;
  gint          width, height;
  gint          bytes;

  bytes = drawable->bpp;

  if (preview)
    {
      picman_preview_get_position (preview, &x, &y);
      picman_preview_get_size (preview, &width, &height);

      dest_buffer = g_new (guchar, width * height * bytes);
      dest = dest_buffer;
    }
  else
    {
      if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                          &x, &y, &width, &height))
        return;

      dest = g_new (guchar, width * bytes);

      picman_pixel_rgn_init (&destPR, drawable, x, y, width, height, TRUE, TRUE);
    }

  picman_pixel_rgn_init (&srcPR, drawable,
                       x, MAX (y - 1, 0),
                       width, MIN (height + 1, drawable->height),
                       FALSE, FALSE);

  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);

  /*  allocate row buffers  */
  upper = g_new (guchar, width * bytes);
  lower = g_new (guchar, width * bytes);

  /*  loop through the rows, performing our magic  */
  for (row = y; row < y + height; row++)
    {
      picman_pixel_rgn_get_row (&srcPR, dest, x, row, width);

      if (row % 2 != devals.evenness)
        {
          if (row > 0)
            picman_pixel_rgn_get_row (&srcPR, upper, x, row - 1, width);
          else
            picman_pixel_rgn_get_row (&srcPR, upper, x, devals.evenness, width);

          if (row + 1 < drawable->height)
            picman_pixel_rgn_get_row (&srcPR, lower, x, row + 1, width);
          else
            picman_pixel_rgn_get_row (&srcPR, lower, x, row - 1 + devals.evenness,
                                    width);

          if (has_alpha)
            {
              const guchar *upix = upper;
              const guchar *lpix = lower;
              guchar       *dpix = dest;

              for (col = 0; col < width; col++)
                {
                  guint ualpha = upix[bytes - 1];
                  guint lalpha = lpix[bytes - 1];
                  guint alpha  = ualpha + lalpha;

                  if ((dpix[bytes - 1] = (alpha >> 1)))
                    {
                      gint b;

                      for (b = 0; b < bytes - 1; b++)
                        dpix[b] = (upix[b] * ualpha + lpix[b] * lalpha) / alpha;
                    }

                  upix += bytes;
                  lpix += bytes;
                  dpix += bytes;
                }
            }
          else
            {
              for (col = 0; col < width * bytes; col++)
                dest[col] = ((guint) upper[col] + (guint) lower[col]) / 2;
            }
        }

      if (preview)
        {
          dest += width * bytes;
        }
      else
        {
          picman_pixel_rgn_set_row (&destPR, dest, x, row, width);

          if ((row % 20) == 0)
            picman_progress_update ((double) row / (double) (height));
        }
    }

  if (preview)
    {
      picman_preview_draw_buffer (preview, dest_buffer, width * bytes);
      dest = dest_buffer;
    }
  else
    {
      picman_progress_update (1.0);
      /*  update the deinterlaced region  */
      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id, x, y, width, height);
    }

  g_free (lower);
  g_free (upper);
  g_free (dest);
}

static gboolean
deinterlace_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *frame;
  GtkWidget *odd;
  GtkWidget *even;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Deinterlace"), PLUG_IN_ROLE,
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
                            G_CALLBACK (deinterlace),
                            drawable);

  frame = picman_int_radio_group_new (FALSE, NULL,
                                    G_CALLBACK (picman_radio_button_update),
                                    &devals.evenness, devals.evenness,

                                    _("Keep o_dd fields"),  ODD_FIELDS,  &odd,
                                    _("Keep _even fields"), EVEN_FIELDS, &even,

                                    NULL);

  g_signal_connect_swapped (odd, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);
  g_signal_connect_swapped (even, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}
