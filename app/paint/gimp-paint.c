/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis and others
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

#include <gegl.h>

#include "paint-types.h"

#include "core/picman.h"
#include "core/picmanlist.h"
#include "core/picmanpaintinfo.h"

#include "picman-paint.h"
#include "picmanairbrush.h"
#include "picmanclone.h"
#include "picmanconvolve.h"
#include "picmandodgeburn.h"
#include "picmaneraser.h"
#include "picmanheal.h"
#include "picmanink.h"
#include "picmanpaintoptions.h"
#include "picmanpaintbrush.h"
#include "picmanpencil.h"
#include "picmanperspectiveclone.h"
#include "picmansmudge.h"


/*  local function prototypes  */

static void   picman_paint_register (Picman        *picman,
                                   GType        paint_type,
                                   GType        paint_options_type,
                                   const gchar *identifier,
                                   const gchar *blurb,
                                   const gchar *stock_id);


/*  public functions  */

void
picman_paint_init (Picman *picman)
{
  PicmanPaintRegisterFunc register_funcs[] =
  {
    picman_dodge_burn_register,
    picman_smudge_register,
    picman_convolve_register,
    picman_perspective_clone_register,
    picman_heal_register,
    picman_clone_register,
    picman_ink_register,
    picman_airbrush_register,
    picman_eraser_register,
    picman_paintbrush_register,
    picman_pencil_register
  };

  gint i;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman->paint_info_list = picman_list_new (PICMAN_TYPE_PAINT_INFO, FALSE);
  picman_object_set_static_name (PICMAN_OBJECT (picman->paint_info_list),
                               "paint infos");

  picman_container_freeze (picman->paint_info_list);

  for (i = 0; i < G_N_ELEMENTS (register_funcs); i++)
    {
      register_funcs[i] (picman, picman_paint_register);
    }

  picman_container_thaw (picman->paint_info_list);
}

void
picman_paint_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman_paint_info_set_standard (picman, NULL);

  if (picman->paint_info_list)
    {
      picman_container_foreach (picman->paint_info_list,
                              (GFunc) g_object_run_dispose, NULL);
      g_object_unref (picman->paint_info_list);
      picman->paint_info_list = NULL;
    }
}


/*  private functions  */

static void
picman_paint_register (Picman        *picman,
                     GType        paint_type,
                     GType        paint_options_type,
                     const gchar *identifier,
                     const gchar *blurb,
                     const gchar *stock_id)
{
  PicmanPaintInfo *paint_info;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (g_type_is_a (paint_type, PICMAN_TYPE_PAINT_CORE));
  g_return_if_fail (g_type_is_a (paint_options_type, PICMAN_TYPE_PAINT_OPTIONS));
  g_return_if_fail (identifier != NULL);
  g_return_if_fail (blurb != NULL);

  paint_info = picman_paint_info_new (picman,
                                    paint_type,
                                    paint_options_type,
                                    identifier,
                                    blurb,
                                    stock_id);

  picman_container_add (picman->paint_info_list, PICMAN_OBJECT (paint_info));
  g_object_unref (paint_info);

  if (paint_type == PICMAN_TYPE_PAINTBRUSH)
    picman_paint_info_set_standard (picman, paint_info);
}
