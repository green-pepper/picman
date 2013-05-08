/*
 *  Goat exercise plug-in by Øyvind Kolås, pippin@picman.org
 */

/*
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

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC "plug-in-goat-exercise"


/* Declare local functions.
 */
static void   query       (void);
static void   run         (const gchar      *name,
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
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",    "Input image (unused)"         },
    { PICMAN_PDB_DRAWABLE, "drawable", "Input drawable"               }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Exercise a goat"),
                          "takes a goat for a walk",
                          "Øyvind KOlås <pippinp@picman.org>",
                          "Øyvind KOlås <pippinp@picman.org>",
                          "21march 2012",
                          N_("Goat-exercise"),
                          "RGB*, INDEXED*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  gint32             drawable_id;
  gint               x, y, width, height;

  INIT_I18N();
  gegl_init (NULL, NULL);

  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  drawable_id = param[2].data.d_drawable;

  if (picman_drawable_mask_intersect (drawable_id, &x, &y, &width, &height))
    {
      GeglBuffer *buffer;
      GeglBuffer *shadow_buffer;

      buffer        = picman_drawable_get_buffer (drawable_id);
      shadow_buffer = picman_drawable_get_shadow_buffer (drawable_id);

      gegl_render_op (buffer, shadow_buffer, "gegl:invert", NULL);

      g_object_unref (shadow_buffer); /* flushes the shadow tiles */
      g_object_unref (buffer);

      picman_drawable_merge_shadow (drawable_id, TRUE);
      picman_drawable_update (drawable_id, x, y, width, height);
      picman_displays_flush ();
    }

  values[0].data.d_status = status;
  gegl_exit ();
}
