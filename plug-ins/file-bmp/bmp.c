/* bmp.c                                          */
/* Version 0.52                                   */
/* This is a File input and output filter for the */
/* Picman. It loads and saves images in windows(TM) */
/* bitmap format.                                 */
/* Some Parts that deal with the interaction with */
/* PICMAN are taken from the GIF plugin by          */
/* Peter Mattis & Spencer Kimball and from the    */
/* PCX plugin by Francisco Bustamante.            */
/*                                                */
/* Alexander.Schulz@stud.uni-karlsruhe.de         */

/* Changes:   28.11.1997 Noninteractive operation */
/*            16.03.1998 Endian-independent!!     */
/*            21.03.1998 Little Bug-fix           */
/*            06.04.1998 Bugfix in Padding        */
/*            11.04.1998 Arch. cleanup (-Wall)    */
/*                       Parses gtkrc             */
/*            14.04.1998 Another Bug in Padding   */
/*            28.04.1998 RLE-Encoding rewritten   */
/*            29.10.1998 Changes by Tor Lillqvist */
/*                       <tml@iki.fi> to support  */
/*                       16 and 32 bit images     */
/*            28.11.1998 Bug in RLE-read-padding  */
/*                       fixed.                   */
/*            19.12.1999 Resolution support added */
/*            06.05.2000 Overhaul for 16&24-bit   */
/*                       plus better OS/2 code    */
/*                       by njl195@zepler.org.uk  */
/*            29.06.2006 Full support for 16/32   */
/*                       bits bitmaps and support */
/*                       for alpha channel        */
/*                       by p.filiciak@zax.pl     */

/*
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
 * ----------------------------------------------------------------------------
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "bmp.h"

#include "libpicman/stdplugins-intl.h"


const gchar *filename    = NULL;
gboolean     interactive = FALSE;
gboolean     lastvals    = FALSE;

struct Bitmap_File_Head_Struct Bitmap_File_Head;
struct Bitmap_Head_Struct      Bitmap_Head;


/* Declare some local functions.
 */
static void   query (void);
static void   run   (const gchar      *name,
                     gint              nparams,
                     const PicmanParam  *param,
                     gint             *nreturn_vals,
                     PicmanParam       **return_vals);

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef load_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_STRING,   "filename",     "The name of the file to load" },
    { PICMAN_PDB_STRING,   "raw-filename", "The name entered" },
  };
  static const PicmanParamDef load_return_vals[] =
  {
    { PICMAN_PDB_IMAGE, "image", "Output image" },
  };

  static const PicmanParamDef save_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",        "Input image" },
    { PICMAN_PDB_DRAWABLE, "drawable",     "Drawable to save" },
    { PICMAN_PDB_STRING,   "filename",     "The name of the file to save the image in" },
    { PICMAN_PDB_STRING,   "raw-filename", "The name entered" },
  };

  picman_install_procedure (LOAD_PROC,
                          "Loads files of Windows BMP file format",
                          "Loads files of Windows BMP file format",
                          "Alexander Schulz",
                          "Alexander Schulz",
                          "1997",
                          N_("Windows BMP image"),
                          NULL,
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (load_args),
                          G_N_ELEMENTS (load_return_vals),
                          load_args, load_return_vals);

  picman_register_file_handler_mime (LOAD_PROC, "image/bmp");
  picman_register_magic_load_handler (LOAD_PROC,
                                    "bmp",
                                    "",
                                    "0,string,BM");

  picman_install_procedure (SAVE_PROC,
                          "Saves files in Windows BMP file format",
                          "Saves files in Windows BMP file format",
                          "Alexander Schulz",
                          "Alexander Schulz",
                          "1997",
                          N_("Windows BMP image"),
                          "INDEXED, GRAY, RGB*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (save_args), 0,
                          save_args, NULL);

  picman_register_file_handler_mime (SAVE_PROC, "image/bmp");
  picman_register_save_handler (SAVE_PROC, "bmp", "");
}

static void
run (const gchar      *name,
     gint             nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[2];
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  gint32             image_ID;
  gint32             drawable_ID;
  PicmanExportReturn   export = PICMAN_EXPORT_CANCEL;
  GError            *error  = NULL;

  INIT_I18N ();
  gegl_init (NULL, NULL);

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

  if (strcmp (name, LOAD_PROC) == 0)
    {
       switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
          interactive = TRUE;
          break;

        case PICMAN_RUN_NONINTERACTIVE:
          /*  Make sure all the arguments are there!  */
          if (nparams != 3)
            status = PICMAN_PDB_CALLING_ERROR;
          break;

        default:
          break;
        }

       if (status == PICMAN_PDB_SUCCESS)
         {
           image_ID = ReadBMP (param[1].data.d_string, &error);

           if (image_ID != -1)
             {
               *nreturn_vals = 2;
               values[1].type         = PICMAN_PDB_IMAGE;
               values[1].data.d_image = image_ID;
             }
           else
             {
               status = PICMAN_PDB_EXECUTION_ERROR;
             }
         }
    }
  else if (strcmp (name, SAVE_PROC) == 0)
    {
      image_ID    = param[1].data.d_int32;
      drawable_ID = param[2].data.d_int32;

      /*  eventually export the image */
      switch (run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
          interactive = TRUE;
          /* fallthrough */

        case PICMAN_RUN_WITH_LAST_VALS:
          if (run_mode == PICMAN_RUN_WITH_LAST_VALS) lastvals = TRUE;
          picman_ui_init (PLUG_IN_BINARY, FALSE);
          export = picman_export_image (&image_ID, &drawable_ID, NULL,
                                      (PICMAN_EXPORT_CAN_HANDLE_RGB |
                                       PICMAN_EXPORT_CAN_HANDLE_ALPHA |
                                       PICMAN_EXPORT_CAN_HANDLE_GRAY |
                                       PICMAN_EXPORT_CAN_HANDLE_INDEXED));
          if (export == PICMAN_EXPORT_CANCEL)
            {
              values[0].data.d_status = PICMAN_PDB_CANCEL;
              return;
            }
          break;

        case PICMAN_RUN_NONINTERACTIVE:
          /*  Make sure all the arguments are there!  */
          if (nparams != 5)
            status = PICMAN_PDB_CALLING_ERROR;
          break;

        default:
          break;
        }

      if (status == PICMAN_PDB_SUCCESS)
        status = WriteBMP (param[3].data.d_string, image_ID, drawable_ID,
                           &error);

      if (export == PICMAN_EXPORT_EXPORT)
        picman_image_delete (image_ID);
    }
  else
    {
      status = PICMAN_PDB_CALLING_ERROR;
    }

  if (status != PICMAN_PDB_SUCCESS && error)
    {
      *nreturn_vals = 2;
      values[1].type          = PICMAN_PDB_STRING;
      values[1].data.d_string = error->message;
    }

  values[0].data.d_status = status;
}
