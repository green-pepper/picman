/* PICMAN - The GNU Image Manipulation Program
 * Red Eye Plug-in for PICMAN.
 *
 * Copyright (C) 2004  Robert Merkel <robert.merkel@benambra.org>
 * Copyright (C) 2006  Andreas Røsdal <andrearo@stud.ntnu.no>
 *
 * All Rights Reserved.
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
 * The GNU General Public License is also available from
 * http://www.fsf.org/copyleft/gpl.html
 *
 * This plugin is used for removing the red-eye effect
 * that occurs in flash photos.
 *
 * Based on a PICMAN 1.2 Perl plugin by Geoff Kuenning
 *
 * Version History:
 * 0.1 - initial preliminary release, single file, only documentation
 *       in the header comments, 2004-05-26
 * 0.2 - Bugfix of red levels, improved documentation.
 * 0.3 - Add internationalization support, preview widget,
 *       improve red eye algorithm threshold and documentation.
 *
 */

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


/* Declare local functions.
 */
static void     query                 (void);
static void     run                   (const gchar      *name,
                                       gint              nparams,
                                       const PicmanParam  *param,
                                       gint             *nreturn_vals,
                                       PicmanParam       **return_vals);

static void     remove_redeye         (PicmanDrawable     *drawable);
static void     remove_redeye_preview (PicmanDrawable     *drawable,
                                       PicmanPreview      *preview);
static void     redeye_inner_loop     (const guchar     *src,
                                       guchar           *dest,
                                       gint              width,
                                       gint              height,
                                       gint              bpp,
                                       gboolean          has_alpha,
                                       int               rowstride);


#define RED_FACTOR    0.5133333
#define GREEN_FACTOR  1
#define BLUE_FACTOR   0.1933333

#define SCALE_WIDTH   100

#define PLUG_IN_PROC    "plug-in-red-eye-removal"
#define PLUG_IN_BINARY  "red-eye-removal"
#define PLUG_IN_ROLE    "picman-red-eye-removal"


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static gint threshold = 50;

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",     "Input image"                  },
    { PICMAN_PDB_DRAWABLE, "drawable",  "Input drawable"               },
    { PICMAN_PDB_INT32,    "threshold", "Red eye threshold in percent" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Remove the red eye effect caused by camera "
                             "flashes"),
                          "This plug-in removes the red eye effect caused by "
                          "camera flashes by using a percentage based red "
                          "color threshold.  Make a selection containing the "
                          "eyes, and apply the filter while adjusting the "
                          "threshold to accurately remove the red eyes.",
                          "Robert Merkel <robert.merkel@benambra.org>, "
                          "Andreas Røsdal <andrearo@stud.ntnu.no>",
                          "Copyright 2004-2006 Robert Merkel, Andreas Røsdal",
                          "2006",
                          N_("_Red Eye Removal..."),
                          "RGB*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Enhance");
}


/*
 * Create dialog for red eye removal
 */
static gboolean
dialog (gint32        image_id,
        PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *preview;
  GtkWidget *table;
  GtkWidget *main_vbox;
  GtkObject *adj;
  gboolean   run = FALSE;

  picman_ui_init (PLUG_IN_BINARY, TRUE);

  dialog = picman_dialog_new (_("Red Eye Removal"), PLUG_IN_ROLE,
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

  preview = picman_zoom_preview_new (drawable);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);
  table = gtk_table_new (1, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  adj = picman_scale_entry_new (GTK_TABLE (table), 1, 0,
                             _("_Threshold:"),
                             SCALE_WIDTH, 0,
                             threshold,
                             0, 100, 1, 5, 0,
                             TRUE,
                             0, 100,
                             _("Threshold for the red eye color to remove."),
                             NULL);

  if (picman_selection_is_empty (picman_item_get_image (drawable->drawable_id)))
    {
      GtkWidget *hints = picman_hint_box_new (_("Manually selecting the eyes may "
                                              "improve the results."));

      gtk_box_pack_end (GTK_BOX (main_vbox), hints, FALSE, FALSE, 0);
      gtk_widget_show (hints);
    }

  g_signal_connect_swapped (preview, "invalidated",
                            G_CALLBACK (remove_redeye_preview),
                            drawable);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &threshold);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
run (const gchar      *name,
     gint             nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam        **return_vals)
{
  static PicmanParam   values[1];
  PicmanDrawable      *drawable;
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  gint32             image_ID;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);
  image_ID = param[1].data.d_image;

  switch (run_mode)
    {
      case PICMAN_RUN_NONINTERACTIVE:
        if (nparams != 4)
          status = PICMAN_PDB_CALLING_ERROR;
        else
          threshold = param[3].data.d_int32;
        break;

      case PICMAN_RUN_INTERACTIVE:
        picman_get_data (PLUG_IN_PROC, &threshold);

        if (! dialog (image_ID, drawable))
          status = PICMAN_PDB_CANCEL;
        break;

      case PICMAN_RUN_WITH_LAST_VALS:
        picman_get_data (PLUG_IN_PROC, &threshold);
        break;

      default:
        break;
    }

  /*  Make sure that the drawable is RGB color.  */
  if (status == PICMAN_PDB_SUCCESS &&
      picman_drawable_is_rgb (drawable->drawable_id))
    {
      remove_redeye (drawable);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &threshold, sizeof (threshold));
    }
  else
    {
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}


/*
 * Red Eye Removal Alorithm, based on using a threshold to detect
 * red pixels. Having a user-made selection around the eyes will
 * prevent incorrect pixels from being selected.
 */
static void
remove_redeye (PicmanDrawable *drawable)
{
  PicmanPixelRgn  src_rgn;
  PicmanPixelRgn  dest_rgn;
  gint          progress, max_progress;
  gboolean      has_alpha;
  gint          x, y;
  gint          width, height;
  gint          i;
  gpointer      pr;

  if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                      &x, &y, &width, &height))
    return;

  picman_progress_init (_("Removing red eye"));

  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);

  progress = 0;
  max_progress = width * height;

  picman_pixel_rgn_init (&src_rgn, drawable,
                       x, y, width, height, FALSE, FALSE);
  picman_pixel_rgn_init (&dest_rgn, drawable,
                       x, y, width, height, TRUE, TRUE);

  for (pr = picman_pixel_rgns_register (2, &src_rgn, &dest_rgn), i = 0;
       pr != NULL;
       pr = picman_pixel_rgns_process (pr), i++)
    {
      redeye_inner_loop (src_rgn.data, dest_rgn.data, src_rgn.w, src_rgn.h,
                         src_rgn.bpp, has_alpha, src_rgn.rowstride);

      progress += src_rgn.w * src_rgn.h;

      if (i % 16 == 0)
        picman_progress_update ((gdouble) progress / (gdouble) max_progress);
    }

  picman_progress_update (1.0);
  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x, y, width, height);
}

static void
remove_redeye_preview (PicmanDrawable *drawable,
                       PicmanPreview  *preview)
{
  guchar   *src;
  guchar   *dest;
  gboolean  has_alpha;
  gint      width, height;
  gint      bpp;
  gint      rowstride;

  src  = picman_zoom_preview_get_source (PICMAN_ZOOM_PREVIEW (preview),
                                       &width, &height, &bpp);
  dest = g_new (guchar, height * width * bpp);

  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  rowstride = bpp * width;

  redeye_inner_loop (src, dest, width, height, bpp, has_alpha, rowstride);

  picman_preview_draw_buffer (preview, dest, rowstride);
  g_free (src);
  g_free (dest);
}

static void
redeye_inner_loop (const guchar *src,
                   guchar       *dest,
                   gint          width,
                   gint          height,
                   gint          bpp,
                   gboolean      has_alpha,
                   gint          rowstride)
{
  const gint red   = 0;
  const gint green = 1;
  const gint blue  = 2;
  const gint alpha = 3;
  gint       x, y;

  for (y = 0; y < height; y++)
    {
      const guchar *s = src;
      guchar       *d = dest;

      for (x = 0; x < width; x++)
        {
          gint adjusted_red       = s[red] * RED_FACTOR;
          gint adjusted_green     = s[green] * GREEN_FACTOR;
          gint adjusted_blue      = s[blue] * BLUE_FACTOR;
          gint adjusted_threshold = (threshold - 50) * 2;

          if (adjusted_red >= adjusted_green - adjusted_threshold &&
              adjusted_red >= adjusted_blue - adjusted_threshold)
            {
              d[red] = CLAMP (((gdouble) (adjusted_green + adjusted_blue)
                        / (2.0  * RED_FACTOR)), 0, 255);
            }
          else
            {
              d[red] = s[red];
            }

          d[green] = s[green];
          d[blue] = s[blue];

          if (has_alpha)
            d[alpha] = s[alpha];

          s += bpp;
          d += bpp;
        }

      src += rowstride;
      dest += rowstride;
    }
}
