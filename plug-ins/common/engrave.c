/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997 Eiichi Takamori
 * Copyright (C) 1996, 1997 Torsten Martinsen
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


/* Some useful macros */

#define PLUG_IN_PROC    "plug-in-engrave"
#define PLUG_IN_BINARY  "engrave"
#define PLUG_IN_ROLE    "picman-engrave"
#define SCALE_WIDTH     125
#define TILE_CACHE_SIZE  16

typedef struct
{
  gint     height;
  gboolean limit;
} EngraveValues;

static void query (void);
static void run   (const gchar      *name,
                   gint              nparams,
                   const PicmanParam  *param,
                   gint             *nreturn_vals,
                   PicmanParam       **return_vals);

static gboolean  engrave_dialog (PicmanDrawable *drawable);

static void      engrave        (PicmanDrawable *drawable,
                                 PicmanPreview  *preview);

#if 0
static void      engrave_large  (PicmanDrawable *drawable,
                                 gint          height,
                                 gboolean      limit,
                                 PicmanPreview  *preview);
#endif

static void      engrave_small  (PicmanDrawable *drawable,
                                 gint          height,
                                 gboolean      limit,
                                 gint          tile_width,
                                 PicmanPreview  *preview);

static void      engrave_sub    (gint          height,
                                 gboolean      limit,
                                 gint          bpp,
                                 gint          num_channels);

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static EngraveValues pvals =
{
  10,    /* height  */
  FALSE  /* limit   */
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",    "Input image (unused)"             },
    { PICMAN_PDB_DRAWABLE, "drawable", "Input drawable"                   },
    { PICMAN_PDB_INT32,    "height",   "Resolution in pixels"             },
    { PICMAN_PDB_INT32,    "limit",    "Limit line width { TRUE, FALSE }" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Simulate an antique engraving"),
                          "Creates a black-and-white 'engraved' version of an image as seen in old illustrations",
                          "Spencer Kimball & Peter Mattis, Eiichi Takamori, Torsten Martinsen",
                          "Spencer Kimball & Peter Mattis, Eiichi Takamori, Torsten Martinsen",
                          "1995,1996,1997",
                          N_("En_grave..."),
                          "RGBA, GRAYA",
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
  picman_tile_cache_ntiles (TILE_CACHE_SIZE);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &pvals);

      /*  First acquire information with a dialog  */
      if (!engrave_dialog (drawable))
        {
          picman_drawable_detach (drawable);
          return;
        }
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 5)
        status = PICMAN_PDB_CALLING_ERROR;
      if (status == PICMAN_PDB_SUCCESS)
        {
          pvals.height = param[3].data.d_int32;
          pvals.limit  = (param[4].data.d_int32) ? TRUE : FALSE;
        }
      if ((status == PICMAN_PDB_SUCCESS) &&
          pvals.height < 0)
        status = PICMAN_PDB_CALLING_ERROR;
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &pvals);
      break;

    default:
      break;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      picman_progress_init (_("Engraving"));

      engrave (drawable, NULL);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      /*  Store data  */
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &pvals, sizeof (EngraveValues));
    }
  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

static gboolean
engrave_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *table;
  GtkWidget *toggle;
  GtkObject *adj;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Engrave"), PLUG_IN_ROLE,
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
                            G_CALLBACK (engrave),
                            drawable);

  table = gtk_table_new (1, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                              _("_Height:"), SCALE_WIDTH, 0,
                              pvals.height, 2.0, 16.0, 1.0, 4.0, 0,
                              TRUE, 0, 0,
                              NULL, NULL);
  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &pvals.height);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  toggle = gtk_check_button_new_with_mnemonic (_("_Limit line width"));
  gtk_box_pack_start (GTK_BOX (main_vbox), toggle, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), pvals.limit);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &pvals.limit);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

/*  Engrave interface functions  */

static void
engrave (PicmanDrawable *drawable,
         PicmanPreview  *preview)
{
  gint     tile_width;
  gint     height;
  gboolean limit;

  tile_width = picman_tile_width();
  height = pvals.height;
  limit = pvals.limit;
  /* [DindinX] this test is always false since
   * tile_width == 64 and height <= 16 */
#if 0
  if (height >= tile_width)
    engrave_large (drawable, height, limit, preview);
  else
#endif
    engrave_small (drawable, height, limit, tile_width, preview);
}

#if 0
static void
engrave_large (PicmanDrawable *drawable,
               gint          height,
               gboolean      limit,
               PicmanPreview  *preview)
{
  PicmanPixelRgn  src_rgn, dest_rgn;
  guchar       *src_row, *dest_row;
  guchar       *src, *dest;
  gulong       *average;
  gint          row, col, b, bpp;
  gint          x, y, y_step, inten, v;
  gulong        count;
  gint          x1, y1, x2, y2;
  gint          progress, max_progress;
  gpointer      pr;

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);

  bpp = (picman_drawable_is_rgb (drawable->drawable_id)) ? 3 : 1;
  average = g_new (gulong, bpp);

  /* Initialize progress */
  progress = 0;
  max_progress = 2 * (x2 - x1) * (y2 - y1);

  for (y = y1; y < y2; y += height - (y % height))
    {
      for (x = x1; x < x2; ++x)
        {
          y_step = height - (y % height);
          y_step = MIN (y_step, x2 - x);

          picman_pixel_rgn_init (&src_rgn, drawable, x, y, 1, y_step,
                               FALSE, FALSE);
          for (b = 0; b < bpp; b++)
            average[b] = 0;
          count = 0;

          for (pr = picman_pixel_rgns_register (1, &src_rgn);
               pr != NULL;
               pr = picman_pixel_rgns_process(pr))
            {
              src_row = src_rgn.data;
              for (row = 0; row < src_rgn.h; row++)
                {
                  src = src_row;
                  for (col = 0; col < src_rgn.w; col++)
                    {
                      for (b = 0; b < bpp; b++)
                        average[b] += src[b];
                      src += src_rgn.bpp;
                      count += 1;
                    }
                  src_row += src_rgn.rowstride;
                }
              /* Update progress */
              progress += src_rgn.w * src_rgn.h;
              picman_progress_update ((double) progress / (double) max_progress);
            }

          if (count > 0)
            for (b = 0; b < bpp; b++)
              average[b] = (guchar) (average[b] / count);

          if (bpp < 3)
            inten = average[0] / 254.0 * height;
          else
            inten = PICMAN_RGB_LUMINANCE (average[0],
                                        average[1],
                                        average[2]) / 254.0 * height;

          picman_pixel_rgn_init (&dest_rgn,
                               drawable, x, y, 1, y_step, TRUE, TRUE);
          for (pr = picman_pixel_rgns_register (1, &dest_rgn);
               pr != NULL;
               pr = picman_pixel_rgns_process(pr))
            {
              dest_row = dest_rgn.data;
              for (row = 0; row < dest_rgn.h; row++)
                {
                  dest = dest_row;
                  v = inten > row ? 255 : 0;
                  if (limit)
                    {
                      if (row == 0)
                        v = 255;
                      else if (row == height-1)
                        v = 0;
                    }
                  for (b = 0; b < bpp; b++)
                    dest[b] = v;
                  dest_row += dest_rgn.rowstride;
                }
              /* Update progress */
              progress += dest_rgn.w * dest_rgn.h;
              picman_progress_update((double) progress / (double) max_progress);
            }
        }
    }

  g_free (average);

  /*  update the engraved region  */
  picman_drawable_flush( drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x1, y1, x2 - x1, y2 - y1);
}
#endif

typedef struct
{
  gint    x, y, h;
  gint    width;
  guchar *data;
} PixelArea;

static PixelArea area;

static void
engrave_small (PicmanDrawable *drawable,
               gint          line_height,
               gboolean      limit,
               gint          tile_width,
               PicmanPreview  *preview)
{
  PicmanPixelRgn src_rgn, dest_rgn;
  gint         bpp, num_channels;
  gint         x1, y1, x2, y2;
  gint         width, height;
  gint         progress, max_progress;

  /*
    For speed efficiency, operates on PixelAreas, whose each width and
    height are less than tile size.

    If both ends of area cannot be divided by line_height ( as
    x1%line_height != 0 etc.), operates on the remainder pixels.
  */

  if (preview)
    {
      picman_preview_get_position (preview, &x1, &y1);
      picman_preview_get_size (preview, &width, &height);

      x2 = x1 + width;
      y2 = y1 + height;
    }
  else
    {
      picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);

      width  = x2 - x1;
      height = y2 - y1;
    }
  picman_pixel_rgn_init (&src_rgn, drawable,
                       x1, y1, width, height, FALSE, FALSE);
  picman_pixel_rgn_init (&dest_rgn, drawable,
                       x1, y1, width, height, (preview == NULL), TRUE);

  /* Initialize progress */
  progress = 0;
  max_progress = width * height;

  bpp = drawable->bpp;
  num_channels = (picman_drawable_is_rgb (drawable->drawable_id)) ? 3 : 1;

  area.width = (tile_width / line_height) * line_height;
  area.data = g_new(guchar, (glong) bpp * area.width * area.width);

  for (area.y = y1; area.y < y2;
       area.y += area.width - (area.y % area.width))
    {
      area.h = area.width - (area.y % area.width);
      area.h = MIN(area.h, y2 - area.y);
      for (area.x = x1; area.x < x2; ++area.x)
        {
          picman_pixel_rgn_get_rect (&src_rgn, area.data, area.x, area.y, 1,
                                   area.h);

          engrave_sub (line_height, limit, bpp, num_channels);

          picman_pixel_rgn_set_rect (&dest_rgn, area.data,
                                   area.x, area.y, 1, area.h);
        }
      if (!preview)
        {
          /* Update progress */
          progress += area.h * width;
          picman_progress_update ((double) progress / (double) max_progress);
        }
    }

  g_free(area.data);

  /*  update the engraved region  */
  if (preview)
    {
      picman_drawable_preview_draw_region (PICMAN_DRAWABLE_PREVIEW (preview),
                                         &dest_rgn);
    }
  else
    {
      picman_progress_update (1.0);
      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id, x1, y1, x2 - x1, y2 - y1);
    }
}

static void
engrave_sub (gint height,
             gint limit,
             gint bpp,
             gint num_channels)
{
  glong average[3];             /* num_channels <= 3 */
  gint y, h, inten, v;
  guchar *buf_row, *buf;
  gint row;
  gint rowstride;
  gint count;
  gint i;

  g_return_if_fail ((num_channels == 1) || (num_channels == 3));

  /*
    Since there's so many nested FOR's,
    put a few of them here...
  */

  rowstride = bpp;

  for (y = area.y; y < area.y + area.h; y += height - (y % height))
    {
      h = height - (y % height);
      h = MIN(h, area.y + area.h - y);

      for (i = 0; i < num_channels; i++)
        average[i] = 0;
      count = 0;

      /* Read */
      buf_row = area.data + (y - area.y) * rowstride;

      for (row = 0; row < h; row++)
        {
          buf = buf_row;
          for (i = 0; i < num_channels; i++)
            average[i] += buf[i];
          count++;
          buf_row += rowstride;
        }

      /* Average */
      if (count > 0)
        for (i = 0; i < num_channels; i++)
          average[i] /= count;

      if (num_channels == 1)
        inten = average[0] / 254.0 * height;
      else
        inten = PICMAN_RGB_LUMINANCE (average[0],
                                    average[1],
                                    average[2]) / 254.0 * height;

      /* Write */
      buf_row = area.data + (y - area.y) * rowstride;

      for (row = 0; row < h; row++)
        {
          buf = buf_row;
          v = inten > row ? 255 : 0;
          if (limit)
            {
              if (row == 0)
                v = 255;
              else if (row == height-1)
                v = 0;
            }
          for (i = 0; i < num_channels; i++)
            buf[i] = v;
          buf_row += rowstride;
        }
    }
}
