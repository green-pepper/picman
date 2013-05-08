/* borderaverage 0.01 - image processing plug-in for PICMAN.
 *
 * Copyright (C) 1998 Philipp Klaus (webmaster@access.ch)
 *
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


#define PLUG_IN_PROC   "plug-in-borderaverage"
#define PLUG_IN_BINARY "border-average"
#define PLUG_IN_ROLE   "picman-border-average"


/* Declare local functions.
 */
static void      query  (void);
static void      run    (const gchar      *name,
                         gint              nparams,
                         const PicmanParam  *param,
                         gint             *nreturn_vals,
                         PicmanParam       **return_vals);


static void      borderaverage        (PicmanDrawable *drawable,
                                       PicmanRGB      *result);

static gboolean  borderaverage_dialog (gint32        image_ID,
                                       PicmanDrawable *drawable);

static void      add_new_color        (gint          bytes,
                                       const guchar *buffer,
                                       gint         *cube,
                                       gint          bucket_expo);

static void      thickness_callback   (GtkWidget    *widget,
                                       gpointer      data);

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init  */
  NULL,  /* quit  */
  query, /* query */
  run,   /* run   */
};

static gint  borderaverage_thickness       = 3;
static gint  borderaverage_bucket_exponent = 4;

struct borderaverage_data
{
  gint  thickness;
  gint  bucket_exponent;
}

static borderaverage_data =
{
  3,
  4
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",        "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",           "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable",        "Input drawable" },
    { PICMAN_PDB_INT32,    "thickness",       "Border size to take in count" },
    { PICMAN_PDB_INT32,    "bucket-exponent", "Bits for bucket size (default=4: 16 Levels)" },
  };
  static const PicmanParamDef return_vals[] =
  {
    { PICMAN_PDB_COLOR,    "borderaverage",   "The average color of the specified border." },
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Set foreground to the average color of the image border"),
                          "",
                          "Philipp Klaus",
                          "Internet Access AG",
                          "1998",
                          N_("_Border Average..."),
                          "RGB*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args),
                          G_N_ELEMENTS (return_vals),
                          args, return_vals);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Colors/Info");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[3];
  PicmanDrawable      *drawable;
  gint32             image_ID;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  PicmanRGB            result_color;
  PicmanRunMode        run_mode;

  INIT_I18N ();

  run_mode = param[0].data.d_int32;
  image_ID = param[1].data.d_int32;

  /*    Get the specified drawable      */
  drawable = picman_drawable_get (param[2].data.d_drawable);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      picman_get_data (PLUG_IN_PROC, &borderaverage_data);
      borderaverage_thickness       = borderaverage_data.thickness;
      borderaverage_bucket_exponent = borderaverage_data.bucket_exponent;
      if (! borderaverage_dialog (image_ID, drawable))
        status = PICMAN_PDB_EXECUTION_ERROR;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      if (nparams != 5)
        status = PICMAN_PDB_CALLING_ERROR;
      if (status == PICMAN_PDB_SUCCESS)
        {
          borderaverage_thickness       = param[3].data.d_int32;
          borderaverage_bucket_exponent = param[4].data.d_int32;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      picman_get_data (PLUG_IN_PROC, &borderaverage_data);
      borderaverage_thickness       = borderaverage_data.thickness;
      borderaverage_bucket_exponent = borderaverage_data.bucket_exponent;
      break;

    default:
      break;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      /*  Make sure that the drawable is RGB color  */
      if (picman_drawable_is_rgb (drawable->drawable_id))
        {
          picman_progress_init ( _("Border Average"));
          borderaverage (drawable, &result_color);

          if (run_mode != PICMAN_RUN_NONINTERACTIVE)
            {
              picman_context_set_foreground (&result_color);
            }
          if (run_mode == PICMAN_RUN_INTERACTIVE)
            {
              borderaverage_data.thickness       = borderaverage_thickness;
              borderaverage_data.bucket_exponent = borderaverage_bucket_exponent;
              picman_set_data (PLUG_IN_PROC,
                             &borderaverage_data, sizeof (borderaverage_data));
            }
        }
      else
        {
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
    }
  *nreturn_vals = 3;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  values[1].type         = PICMAN_PDB_COLOR;
  values[1].data.d_color = result_color;

  picman_drawable_detach (drawable);
}

typedef struct {
  gint x1, x2, y1, y2;
  gint bucket_expo;
  gint *cube;
} BorderAverageParam_t;

static void
borderaverage_func (gint x,
                    gint y,
                    const guchar *src,
                    gint bpp,
                    gpointer data)
{
  BorderAverageParam_t *param = (BorderAverageParam_t*) data;

  if (x <  param->x1 + borderaverage_thickness ||
      x >= param->x2 - borderaverage_thickness ||
      y <  param->y1 + borderaverage_thickness ||
      y >= param->y2 - borderaverage_thickness)
    {
      add_new_color (bpp, src, param->cube, param->bucket_expo);
    }
}

static void
borderaverage (PicmanDrawable *drawable,
               PicmanRGB      *result)
{
  gint    x1, x2, y1, y2;
  gint    bytes;
  gint    max;
  guchar  r, g, b;
  guchar *buffer;
  gint    bucket_num, bucket_expo, bucket_rexpo;
  gint   *cube;
  gint    i, j, k; /* index variables */
  PicmanRgnIterator *iter;

  BorderAverageParam_t     param;

  /* allocate and clear the cube before */
  bucket_expo = borderaverage_bucket_exponent;
  bucket_rexpo = 8 - bucket_expo;
  param.cube = cube = g_new (gint, 1 << (bucket_rexpo * 3));
  bucket_num = 1 << bucket_rexpo;

  for (i = 0; i < bucket_num; i++)
    {
      for (j = 0; j < bucket_num; j++)
        {
          for (k = 0; k < bucket_num; k++)
            {
              cube[(i << (bucket_rexpo << 1)) + (j << bucket_rexpo) + k] = 0;
            }
        }
    }

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
  param.x1 = x1;
  param.y1 = y1;
  param.x2 = x2;
  param.y2 = y2;
  param.bucket_expo = bucket_expo;

  /*  Get the size of the input image. (This will/must be the same
   *  as the size of the output image.
   */
  bytes = drawable->bpp;

  picman_tile_cache_ntiles (2 * ((x2 - x1) / picman_tile_width () + 1));

  /*  allocate row buffer  */
  buffer = g_new (guchar, (x2 - x1) * bytes);

  iter = picman_rgn_iterator_new (drawable, 0);
  picman_rgn_iterator_src (iter, borderaverage_func, &param);
  picman_rgn_iterator_free (iter);

  max = 0; r = 0; g = 0; b = 0;

  /* get max of cube */
  for (i = 0; i < bucket_num; i++)
    {
      for (j = 0; j < bucket_num; j++)
        {
          for (k = 0; k < bucket_num; k++)
            {
              if (cube[(i << (bucket_rexpo << 1)) +
                      (j << bucket_rexpo) + k] > max)
                {
                  max = cube[(i << (bucket_rexpo << 1)) +
                            (j << bucket_rexpo) + k];
                  r = (i<<bucket_expo) + (1<<(bucket_expo - 1));
                  g = (j<<bucket_expo) + (1<<(bucket_expo - 1));
                  b = (k<<bucket_expo) + (1<<(bucket_expo - 1));
                }
            }
        }
    }

  /* return the color */
  picman_rgb_set_uchar (result, r, g, b);

  g_free (buffer);
  g_free (cube);
}

static void
add_new_color (gint          bytes,
               const guchar *buffer,
               gint         *cube,
               gint          bucket_expo)
{
  guchar r, g, b;
  gint   bucket_rexpo;

  bucket_rexpo = 8 - bucket_expo;
  r = buffer[0] >> bucket_expo;
  g = (bytes > 1) ? buffer[1] >> bucket_expo : 0;
  b = (bytes > 2) ? buffer[2] >> bucket_expo : 0;
  cube[(r << (bucket_rexpo << 1)) + (g << bucket_rexpo) + b]++;
}

static gboolean
borderaverage_dialog (gint32        image_ID,
                      PicmanDrawable *drawable)
{
  GtkWidget    *dialog;
  GtkWidget    *frame;
  GtkWidget    *main_vbox;
  GtkWidget    *hbox;
  GtkWidget    *label;
  GtkWidget    *size_entry;
  PicmanUnit      unit;
  GtkWidget    *combo;
  GtkSizeGroup *group;
  gboolean      run;
  gdouble       xres, yres;

  const gchar *labels[] =
    { "1", "2", "4", "8", "16", "32", "64", "128", "256" };

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Borderaverage"), PLUG_IN_ROLE,
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

  frame = picman_frame_new (_("Border Size"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("_Thickness:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  gtk_size_group_add_widget (group, label);
  g_object_unref (group);

  /*  Get the image resolution and unit  */
  picman_image_get_resolution (image_ID, &xres, &yres);
  unit = picman_image_get_unit (image_ID);

  size_entry = picman_size_entry_new (1, unit, "%a", TRUE, TRUE, FALSE, 4,
                                    PICMAN_SIZE_ENTRY_UPDATE_SIZE);
  gtk_box_pack_start (GTK_BOX (hbox), size_entry, FALSE, FALSE, 0);

  picman_size_entry_set_unit (PICMAN_SIZE_ENTRY (size_entry), PICMAN_UNIT_PIXEL);
  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (size_entry), 0, xres, TRUE);

  /*  set the size (in pixels) that will be treated as 0% and 100%  */
  picman_size_entry_set_size (PICMAN_SIZE_ENTRY (size_entry), 0, 0.0,
                            drawable->width);

  picman_size_entry_set_refval_boundaries (PICMAN_SIZE_ENTRY (size_entry), 0,
                                         1.0, 256.0);
  gtk_table_set_col_spacing (GTK_TABLE (size_entry), 0, 4);
  gtk_table_set_col_spacing (GTK_TABLE (size_entry), 2, 12);
  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (size_entry), 0,
                              (gdouble) borderaverage_thickness);
  g_signal_connect (size_entry, "value-changed",
                    G_CALLBACK (thickness_callback),
                    NULL);
  gtk_widget_show (size_entry);

  frame = picman_frame_new (_("Number of Colors"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("_Bucket size:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  gtk_size_group_add_widget (group, label);

  combo = picman_int_combo_box_new_array (G_N_ELEMENTS (labels), labels);
  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (combo),
                                 borderaverage_bucket_exponent);

  g_signal_connect (combo, "changed",
                    G_CALLBACK (picman_int_combo_box_get_active),
                    &borderaverage_bucket_exponent);

  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
  gtk_widget_show (combo);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
thickness_callback (GtkWidget *widget,
                    gpointer   data)
{
  borderaverage_thickness =
    picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (widget), 0);
}
