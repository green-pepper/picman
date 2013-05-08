/* PICMAN - The GNU Image Manipulation Program
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

#include "config.h"

#include <gegl.h>

#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "vectors/picmanvectors.h"

#include "picmanimage.h"
#include "picmanimage-arrange.h"
#include "picmanimage-guides.h"
#include "picmanimage-undo.h"
#include "picmanitem.h"
#include "picmanchannel.h"
#include "picmandrawable.h"
#include "picmanguide.h"

#include "picman-intl.h"

static GList * sort_by_offset  (GList             *list);
static void    compute_offsets (GList             *list,
                                PicmanAlignmentType  alignment);
static void    compute_offset  (GObject           *object,
                                PicmanAlignmentType  alignment);
static gint    offset_compare  (gconstpointer      a,
                                gconstpointer      b);


/**
 * picman_image_arrange_objects:
 * @image:                The #PicmanImage to which the objects belong.
 * @list:                 A #GList of objects to be aligned.
 * @alignment:            The point on each target object to bring into alignment.
 * @reference:            The #GObject to align the targets with, or #NULL.
 * @reference_alignment:  The point on the reference object to align the target item with..
 * @offset:               How much to shift the target from perfect alignment..
 *
 * This function shifts the positions of a set of target objects, which can be
 * "items" or guides, to bring them into a specified type of alignment with a
 * reference object, which can be an item, guide, or image.  If the requested
 * alignment does not make sense (i.e., trying to align a vertical guide vertically),
 * nothing happens and no error message is generated.
 *
 * The objects in the list are sorted into increasing order before
 * being arranged, where the order is defined by the type of alignment
 * being requested.  If the @reference argument is #NULL, then the first
 * object in the sorted list is used as reference.
 *
 * When there are multiple target objects, they are arranged so that the spacing
 * between consecutive ones is given by the argument @offset.
 */
void
picman_image_arrange_objects (PicmanImage         *image,
                            GList             *list,
                            PicmanAlignmentType  alignment,
                            GObject           *reference,
                            PicmanAlignmentType  reference_alignment,
                            gint               offset)
{
  gboolean do_x               = FALSE;
  gboolean do_y               = FALSE;
  gint     z0                 = 0;
  GList   *object_list;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (G_IS_OBJECT (reference) || reference == NULL);

  /* get offsets used for sorting */
  switch (alignment)
    {
      /* order vertically for horizontal alignment */
    case PICMAN_ALIGN_LEFT:
    case PICMAN_ALIGN_HCENTER:
    case PICMAN_ALIGN_RIGHT:
      do_x = TRUE;
      compute_offsets (list, PICMAN_ALIGN_TOP);
      break;
      /* order horizontally for horizontal arrangement */
    case PICMAN_ARRANGE_LEFT:
    case PICMAN_ARRANGE_HCENTER:
    case PICMAN_ARRANGE_RIGHT:
      do_x = TRUE;
      compute_offsets (list, alignment);
      break;
      /* order horizontally for vertical alignment */
    case PICMAN_ALIGN_TOP:
    case PICMAN_ALIGN_VCENTER:
    case PICMAN_ALIGN_BOTTOM:
      do_y = TRUE;
      compute_offsets (list, PICMAN_ALIGN_LEFT);
      break;
      /* order vertically for vertical arrangement */
    case PICMAN_ARRANGE_TOP:
    case PICMAN_ARRANGE_VCENTER:
    case PICMAN_ARRANGE_BOTTOM:
      do_y = TRUE;
      compute_offsets (list, alignment);
      break;
    }

  object_list = sort_by_offset (list);

  /* now get offsets used for aligning */
  compute_offsets (list, alignment);

  if (reference == NULL)
    {
      reference = G_OBJECT (object_list->data);
      object_list = g_list_next (object_list);
    }
  else
    compute_offset (reference, reference_alignment);

  z0 = GPOINTER_TO_INT (g_object_get_data (reference, "align-offset"));

  if (object_list)
    {
      GList *l;
      gint   n;

      /* FIXME: undo group type is wrong */
      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_ITEM_DISPLACE,
                                   C_("undo-type", "Arrange Objects"));

      for (l = object_list, n = 1; l; l = g_list_next (l), n++)
        {
          GObject *target          = G_OBJECT (l->data);
          gint     xtranslate      = 0;
          gint     ytranslate      = 0;
          gint     z1;

          z1 = GPOINTER_TO_INT (g_object_get_data (target,
                                                    "align-offset"));

          if (do_x)
            xtranslate = z0 - z1 + n * offset;

          if (do_y)
            ytranslate = z0 - z1 + n * offset;

          /* now actually align the target object */
          if (PICMAN_IS_ITEM (target))
            {
              picman_item_translate (PICMAN_ITEM (target),
                                   xtranslate, ytranslate, TRUE);
            }
          else if (PICMAN_IS_GUIDE (target))
            {
              PicmanGuide *guide = PICMAN_GUIDE (target);

              switch (picman_guide_get_orientation (guide))
                {
                case PICMAN_ORIENTATION_VERTICAL:
                  picman_image_move_guide (image, guide, z1 + xtranslate, TRUE);
                  break;

                case PICMAN_ORIENTATION_HORIZONTAL:
                  picman_image_move_guide (image, guide, z1 + ytranslate, TRUE);
                  break;

                default:
                  break;
                }
            }
        }

      picman_image_undo_group_end (image);
    }

  g_list_free (object_list);
}


static GList *
sort_by_offset (GList *list)
{
  return g_list_sort (g_list_copy (list),
                      offset_compare);

}

static gint
offset_compare (gconstpointer a,
                gconstpointer b)
{
  gint offset1 = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (a),
                                                     "align-offset"));
  gint offset2 = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (b),
                                                     "align-offset"));

  return offset1 - offset2;
}

/*
 * this function computes the position of the alignment point
 * for each object in the list, and attaches it to the
 * object as object data.
 */
static void
compute_offsets (GList             *list,
                 PicmanAlignmentType  alignment)
{
  GList *l;

  for (l = list; l; l = g_list_next (l))
    compute_offset (G_OBJECT (l->data), alignment);
}

static void
compute_offset (GObject *object,
                PicmanAlignmentType  alignment)
{
  gint object_offset_x = 0;
  gint object_offset_y = 0;
  gint object_height   = 0;
  gint object_width    = 0;
  gint offset          = 0;

  if (PICMAN_IS_IMAGE (object))
    {
      PicmanImage *image = PICMAN_IMAGE (object);

      object_offset_x = 0;
      object_offset_y = 0;
      object_height   = picman_image_get_height (image);
      object_width    = picman_image_get_width (image);
    }
  else if (PICMAN_IS_CHANNEL (object))
    {
      /* for channels, we use the bounds of the visible area, not
         the layer bounds.  This includes the selection channel */

      PicmanChannel *channel = PICMAN_CHANNEL (object);

      if (picman_channel_is_empty (channel))
        {
          /* fall back on using the offsets instead */
          PicmanItem *item = PICMAN_ITEM (object);

          picman_item_get_offset (item, &object_offset_x, &object_offset_y);
          object_width  = picman_item_get_width  (item);
          object_height = picman_item_get_height (item);
        }
      else
        {
          gint x1, x2, y1, y2;

          picman_channel_bounds (channel, &x1, &y1, &x2, &y2);
          object_offset_x = x1;
          object_offset_y = y1;
          object_width    = x2 - x1;
          object_height   = y2 - y1;
        }
    }
  else if (PICMAN_IS_ITEM (object))
    {
      PicmanItem *item = PICMAN_ITEM (object);

      if (PICMAN_IS_VECTORS (object))
        {
          gdouble x1_f, y1_f, x2_f, y2_f;

          picman_vectors_bounds (PICMAN_VECTORS (item),
                               &x1_f, &y1_f,
                               &x2_f, &y2_f);

          object_offset_x = ROUND (x1_f);
          object_offset_y = ROUND (y1_f);
          object_height   = ROUND (y2_f - y1_f);
          object_width    = ROUND (x2_f - x1_f);
        }
      else
        {
          picman_item_get_offset (item, &object_offset_x, &object_offset_y);
          object_width  = picman_item_get_width  (item);
          object_height = picman_item_get_height (item);
        }
    }
  else if (PICMAN_IS_GUIDE (object))
    {
      PicmanGuide *guide = PICMAN_GUIDE (object);

      switch (picman_guide_get_orientation (guide))
        {
        case PICMAN_ORIENTATION_VERTICAL:
          object_offset_x = picman_guide_get_position (guide);
          object_width = 0;
          break;

        case PICMAN_ORIENTATION_HORIZONTAL:
          object_offset_y = picman_guide_get_position (guide);
          object_height = 0;
          break;

        default:
          break;
        }
    }
  else
    {
      g_printerr ("Alignment object is not an image, item or guide.\n");
    }

  switch (alignment)
    {
    case PICMAN_ALIGN_LEFT:
    case PICMAN_ARRANGE_LEFT:
      offset = object_offset_x;
      break;
    case PICMAN_ALIGN_HCENTER:
    case PICMAN_ARRANGE_HCENTER:
      offset = object_offset_x + object_width/2;
      break;
    case PICMAN_ALIGN_RIGHT:
    case PICMAN_ARRANGE_RIGHT:
      offset = object_offset_x + object_width;
      break;
    case PICMAN_ALIGN_TOP:
    case PICMAN_ARRANGE_TOP:
      offset = object_offset_y;
      break;
    case PICMAN_ALIGN_VCENTER:
    case PICMAN_ARRANGE_VCENTER:
      offset = object_offset_y + object_height/2;
      break;
    case PICMAN_ALIGN_BOTTOM:
    case PICMAN_ARRANGE_BOTTOM:
      offset = object_offset_y + object_height;
      break;
    default:
      g_assert_not_reached ();
    }

  g_object_set_data (object, "align-offset",
                     GINT_TO_POINTER (offset));
}
