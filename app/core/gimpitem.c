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

#include <stdlib.h>
#include <string.h>

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "picman.h"
#include "picman-parasites.h"
#include "picmanchannel.h"
#include "picmanidtable.h"
#include "picmanimage.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanitem.h"
#include "picmanitem-linked.h"
#include "picmanitem-preview.h"
#include "picmanitemtree.h"
#include "picmanlist.h"
#include "picmanmarshal.h"
#include "picmanparasitelist.h"
#include "picmanprogress.h"
#include "picmanstrokeoptions.h"

#include "picman-intl.h"


enum
{
  REMOVED,
  VISIBILITY_CHANGED,
  LINKED_CHANGED,
  LOCK_CONTENT_CHANGED,
  LOCK_POSITION_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_IMAGE,
  PROP_ID,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_OFFSET_X,
  PROP_OFFSET_Y,
  PROP_VISIBLE,
  PROP_LINKED,
  PROP_LOCK_CONTENT,
  PROP_LOCK_POSITION
};


typedef struct _PicmanItemPrivate PicmanItemPrivate;

struct _PicmanItemPrivate
{
  gint              ID;                 /*  provides a unique ID     */
  guint32           tattoo;             /*  provides a permanent ID  */

  PicmanImage        *image;              /*  item owner               */

  PicmanParasiteList *parasites;          /*  Plug-in parasite data    */

  gint              width, height;      /*  size in pixels           */
  gint              offset_x, offset_y; /*  pixel offset in image    */

  guint             visible       : 1;  /*  control visibility       */
  guint             linked        : 1;  /*  control linkage          */
  guint             lock_content  : 1;  /*  content editability      */
  guint             lock_position : 1;  /*  content movability       */

  guint             removed : 1;        /*  removed from the image?  */

  GList            *offset_nodes;       /*  offset nodes to manage   */
};

#define GET_PRIVATE(item) G_TYPE_INSTANCE_GET_PRIVATE (item, \
                                                       PICMAN_TYPE_ITEM, \
                                                       PicmanItemPrivate)


/*  local function prototypes  */

static void       picman_item_constructed             (GObject        *object);
static void       picman_item_finalize                (GObject        *object);
static void       picman_item_set_property            (GObject        *object,
                                                     guint           property_id,
                                                     const GValue   *value,
                                                     GParamSpec     *pspec);
static void       picman_item_get_property            (GObject        *object,
                                                     guint           property_id,
                                                     GValue         *value,
                                                     GParamSpec     *pspec);

static gint64     picman_item_get_memsize             (PicmanObject     *object,
                                                     gint64         *gui_size);

static void       picman_item_real_visibility_changed (PicmanItem       *item);

static gboolean   picman_item_real_is_content_locked  (const PicmanItem *item);
static gboolean   picman_item_real_is_position_locked (const PicmanItem *item);
static PicmanItem * picman_item_real_duplicate          (PicmanItem       *item,
                                                     GType           new_type);
static void       picman_item_real_convert            (PicmanItem       *item,
                                                     PicmanImage      *dest_image);
static gboolean   picman_item_real_rename             (PicmanItem       *item,
                                                     const gchar    *new_name,
                                                     const gchar    *undo_desc,
                                                     GError        **error);
static void       picman_item_real_translate          (PicmanItem       *item,
                                                     gint            offset_x,
                                                     gint            offset_y,
                                                     gboolean        push_undo);
static void       picman_item_real_scale              (PicmanItem       *item,
                                                     gint            new_width,
                                                     gint            new_height,
                                                     gint            new_offset_x,
                                                     gint            new_offset_y,
                                                     PicmanInterpolationType interpolation,
                                                     PicmanProgress   *progress);
static void       picman_item_real_resize             (PicmanItem       *item,
                                                     PicmanContext    *context,
                                                     gint            new_width,
                                                     gint            new_height,
                                                     gint            offset_x,
                                                     gint            offset_y);


G_DEFINE_TYPE (PicmanItem, picman_item, PICMAN_TYPE_FILTER)

#define parent_class picman_item_parent_class

static guint picman_item_signals[LAST_SIGNAL] = { 0 };


static void
picman_item_class_init (PicmanItemClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);

  picman_item_signals[REMOVED] =
    g_signal_new ("removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanItemClass, removed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_item_signals[VISIBILITY_CHANGED] =
    g_signal_new ("visibility-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanItemClass, visibility_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_item_signals[LINKED_CHANGED] =
    g_signal_new ("linked-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanItemClass, linked_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_item_signals[LOCK_CONTENT_CHANGED] =
    g_signal_new ("lock-content-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanItemClass, lock_content_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_item_signals[LOCK_POSITION_CHANGED] =
    g_signal_new ("lock-position-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanItemClass, lock_position_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->constructed        = picman_item_constructed;
  object_class->finalize           = picman_item_finalize;
  object_class->set_property       = picman_item_set_property;
  object_class->get_property       = picman_item_get_property;

  picman_object_class->get_memsize   = picman_item_get_memsize;

  viewable_class->get_preview_size = picman_item_get_preview_size;
  viewable_class->get_popup_size   = picman_item_get_popup_size;

  klass->removed                   = NULL;
  klass->visibility_changed        = picman_item_real_visibility_changed;
  klass->linked_changed            = NULL;
  klass->lock_content_changed      = NULL;
  klass->lock_position_changed     = NULL;

  klass->unset_removed             = NULL;
  klass->is_attached               = NULL;
  klass->is_content_locked         = picman_item_real_is_content_locked;
  klass->is_position_locked        = picman_item_real_is_position_locked;
  klass->get_tree                  = NULL;
  klass->duplicate                 = picman_item_real_duplicate;
  klass->convert                   = picman_item_real_convert;
  klass->rename                    = picman_item_real_rename;
  klass->translate                 = picman_item_real_translate;
  klass->scale                     = picman_item_real_scale;
  klass->resize                    = picman_item_real_resize;
  klass->flip                      = NULL;
  klass->rotate                    = NULL;
  klass->transform                 = NULL;
  klass->stroke                    = NULL;
  klass->to_selection              = NULL;

  klass->default_name              = NULL;
  klass->rename_desc               = NULL;
  klass->translate_desc            = NULL;
  klass->scale_desc                = NULL;
  klass->resize_desc               = NULL;
  klass->flip_desc                 = NULL;
  klass->rotate_desc               = NULL;
  klass->transform_desc            = NULL;

  g_object_class_install_property (object_class, PROP_IMAGE,
                                   g_param_spec_object ("image", NULL, NULL,
                                                        PICMAN_TYPE_IMAGE,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_ID,
                                   g_param_spec_int ("id", NULL, NULL,
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_WIDTH,
                                   g_param_spec_int ("width", NULL, NULL,
                                                     1, PICMAN_MAX_IMAGE_SIZE, 1,
                                                     PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_HEIGHT,
                                   g_param_spec_int ("height", NULL, NULL,
                                                     1, PICMAN_MAX_IMAGE_SIZE, 1,
                                                     PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_OFFSET_X,
                                   g_param_spec_int ("offset-x", NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE, 0,
                                                     PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_OFFSET_Y,
                                   g_param_spec_int ("offset-y", NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE, 0,
                                                     PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_VISIBLE,
                                   g_param_spec_boolean ("visible", NULL, NULL,
                                                         TRUE,
                                                         PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_LINKED,
                                   g_param_spec_boolean ("linked", NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_LOCK_CONTENT,
                                   g_param_spec_boolean ("lock-content",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_LOCK_POSITION,
                                   g_param_spec_boolean ("lock-position",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READABLE));

  g_type_class_add_private (klass, sizeof (PicmanItemPrivate));
}

static void
picman_item_init (PicmanItem *item)
{
  PicmanItemPrivate *private = GET_PRIVATE (item);

  g_object_force_floating (G_OBJECT (item));

  private->ID            = 0;
  private->tattoo        = 0;
  private->image         = NULL;
  private->parasites     = picman_parasite_list_new ();
  private->width         = 0;
  private->height        = 0;
  private->offset_x      = 0;
  private->offset_y      = 0;
  private->visible       = TRUE;
  private->linked        = FALSE;
  private->lock_content  = FALSE;
  private->lock_position = FALSE;
  private->removed       = FALSE;
}

static void
picman_item_constructed (GObject *object)
{
  PicmanItemPrivate *private = GET_PRIVATE (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_IMAGE (private->image));
  g_assert (private->ID != 0);
}

static void
picman_item_finalize (GObject *object)
{
  PicmanItemPrivate *private = GET_PRIVATE (object);

  if (private->offset_nodes)
    {
      g_list_free_full (private->offset_nodes,
                        (GDestroyNotify) g_object_unref);
      private->offset_nodes = NULL;
    }

  if (private->image && private->image->picman)
    {
      picman_id_table_remove (private->image->picman->item_table, private->ID);
      private->image = NULL;
    }

  if (private->parasites)
    {
      g_object_unref (private->parasites);
      private->parasites = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_item_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  PicmanItem *item = PICMAN_ITEM (object);

  switch (property_id)
    {
    case PROP_IMAGE:
      picman_item_set_image (item, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_item_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  PicmanItemPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_IMAGE:
      g_value_set_object (value, private->image);
      break;
    case PROP_ID:
      g_value_set_int (value, private->ID);
      break;
    case PROP_WIDTH:
      g_value_set_int (value, private->width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, private->height);
      break;
    case PROP_OFFSET_X:
      g_value_set_int (value, private->offset_x);
      break;
    case PROP_OFFSET_Y:
      g_value_set_int (value, private->offset_y);
      break;
    case PROP_VISIBLE:
      g_value_set_boolean (value, private->visible);
      break;
    case PROP_LINKED:
      g_value_set_boolean (value, private->linked);
      break;
    case PROP_LOCK_CONTENT:
      g_value_set_boolean (value, private->lock_content);
      break;
    case PROP_LOCK_POSITION:
      g_value_set_boolean (value, private->lock_position);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_item_get_memsize (PicmanObject *object,
                       gint64     *gui_size)
{
  PicmanItemPrivate *private = GET_PRIVATE (object);
  gint64           memsize = 0;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->parasites),
                                      gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_item_real_visibility_changed (PicmanItem *item)
{
  GeglNode *node = picman_filter_peek_node (PICMAN_FILTER (item));

  if (node)
    {
      if (picman_item_get_visible (item))
        {
          /* Leave this up to subclasses */
        }
      else
        {
          GeglNode *input  = gegl_node_get_input_proxy  (node, "input");
          GeglNode *output = gegl_node_get_output_proxy (node, "output");

          gegl_node_connect_to (input,  "output",
                                output, "input");
        }
    }
}

static gboolean
picman_item_real_is_content_locked (const PicmanItem *item)
{
  PicmanItem *parent = picman_item_get_parent (item);

  if (parent && picman_item_is_content_locked (parent))
    return TRUE;

  return GET_PRIVATE (item)->lock_content;
}

static gboolean
picman_item_real_is_position_locked (const PicmanItem *item)
{
  if (picman_item_get_linked (item))
    if (picman_item_linked_is_locked (item))
      return TRUE;

  return GET_PRIVATE (item)->lock_position;
}

static PicmanItem *
picman_item_real_duplicate (PicmanItem *item,
                          GType     new_type)
{
  PicmanItemPrivate *private;
  PicmanItem        *new_item;
  gchar           *new_name;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);

  private = GET_PRIVATE (item);

  g_return_val_if_fail (PICMAN_IS_IMAGE (private->image), NULL);
  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_ITEM), NULL);

  /*  formulate the new name  */
  {
    const gchar *name;
    gchar       *ext;
    gint         number;
    gint         len;

    name = picman_object_get_name (item);

    g_return_val_if_fail (name != NULL, NULL);


    ext = strrchr (name, '#');
    len = strlen (_("copy"));

    if ((strlen (name) >= len &&
         strcmp (&name[strlen (name) - len], _("copy")) == 0) ||
        (ext && (number = atoi (ext + 1)) > 0 &&
         ((int)(log10 (number) + 1)) == strlen (ext + 1)))
      {
        /* don't have redundant "copy"s */
        new_name = g_strdup (name);
      }
    else
      {
        new_name = g_strdup_printf (_("%s copy"), name);
      }
  }

  new_item = picman_item_new (new_type,
                            picman_item_get_image (item), new_name,
                            private->offset_x, private->offset_y,
                            picman_item_get_width  (item),
                            picman_item_get_height (item));

  g_free (new_name);

  g_object_unref (GET_PRIVATE (new_item)->parasites);
  GET_PRIVATE (new_item)->parasites = picman_parasite_list_copy (private->parasites);

  picman_item_set_visible (new_item, picman_item_get_visible (item), FALSE);
  picman_item_set_linked  (new_item, picman_item_get_linked (item),  FALSE);

  if (picman_item_can_lock_content (new_item))
    picman_item_set_lock_content (new_item, picman_item_get_lock_content (item),
                                FALSE);

  if (picman_item_can_lock_position (new_item))
    picman_item_set_lock_position (new_item, picman_item_get_lock_position (item),
                                 FALSE);

  return new_item;
}

static void
picman_item_real_convert (PicmanItem  *item,
                        PicmanImage *dest_image)
{
  picman_item_set_image (item, dest_image);
}

static gboolean
picman_item_real_rename (PicmanItem     *item,
                       const gchar  *new_name,
                       const gchar  *undo_desc,
                       GError      **error)
{
  if (picman_item_is_attached (item))
    picman_item_tree_rename_item (picman_item_get_tree (item), item,
                                new_name, TRUE, undo_desc);
  else
    picman_object_set_name (PICMAN_OBJECT (item), new_name);

  return TRUE;
}

static void
picman_item_real_translate (PicmanItem *item,
                          gint      offset_x,
                          gint      offset_y,
                          gboolean  push_undo)
{
  PicmanItemPrivate *private = GET_PRIVATE (item);

  picman_item_set_offset (item,
                        private->offset_x + offset_x,
                        private->offset_y + offset_y);
}

static void
picman_item_real_scale (PicmanItem              *item,
                      gint                   new_width,
                      gint                   new_height,
                      gint                   new_offset_x,
                      gint                   new_offset_y,
                      PicmanInterpolationType  interpolation,
                      PicmanProgress          *progress)
{
  PicmanItemPrivate *private = GET_PRIVATE (item);

  if (private->width != new_width)
    {
      private->width = new_width;
      g_object_notify (G_OBJECT (item), "width");
    }

  if (private->height != new_height)
    {
      private->height = new_height;
      g_object_notify (G_OBJECT (item), "height");
    }

  picman_item_set_offset (item, new_offset_x, new_offset_y);
}

static void
picman_item_real_resize (PicmanItem    *item,
                       PicmanContext *context,
                       gint         new_width,
                       gint         new_height,
                       gint         offset_x,
                       gint         offset_y)
{
  PicmanItemPrivate *private = GET_PRIVATE (item);

  if (private->width != new_width)
    {
      private->width = new_width;
      g_object_notify (G_OBJECT (item), "width");
    }

  if (private->height != new_height)
    {
      private->height = new_height;
      g_object_notify (G_OBJECT (item), "height");
    }

  picman_item_set_offset (item,
                        private->offset_x - offset_x,
                        private->offset_y - offset_y);
}


/*  public functions  */

/**
 * picman_item_new:
 * @type:     The new item's type.
 * @image:    The new item's #PicmanImage.
 * @name:     The name to assign the item.
 * @offset_x: The X offset to assign the item.
 * @offset_y: The Y offset to assign the item.
 * @width:    The width to assign the item.
 * @height:   The height to assign the item.
 *
 * Return value: The newly created item.
 */
PicmanItem *
picman_item_new (GType        type,
               PicmanImage   *image,
               const gchar *name,
               gint         offset_x,
               gint         offset_y,
               gint         width,
               gint         height)
{
  PicmanItem        *item;
  PicmanItemPrivate *private;

  g_return_val_if_fail (g_type_is_a (type, PICMAN_TYPE_ITEM), NULL);
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (width > 0 && height > 0, NULL);

  item = g_object_new (type,
                       "image", image,
                       NULL);

  private = GET_PRIVATE (item);

  private->width  = width;
  private->height = height;
  picman_item_set_offset (item, offset_x, offset_y);

  if (name && strlen (name))
    picman_object_set_name (PICMAN_OBJECT (item), name);
  else
    picman_object_set_static_name (PICMAN_OBJECT (item),
                                 PICMAN_ITEM_GET_CLASS (item)->default_name);

  return item;
}

/**
 * picman_item_remove:
 * @item: the #PicmanItem to remove.
 *
 * This function sets the 'removed' flag on @item to #TRUE, and emits
 * a 'removed' signal on the item.
 */
void
picman_item_removed (PicmanItem *item)
{
  PicmanContainer *children;

  g_return_if_fail (PICMAN_IS_ITEM (item));

  GET_PRIVATE (item)->removed = TRUE;

  children = picman_viewable_get_children (PICMAN_VIEWABLE (item));

  if (children)
    picman_container_foreach (children, (GFunc) picman_item_removed, NULL);

  g_signal_emit (item, picman_item_signals[REMOVED], 0);
}

/**
 * picman_item_is_removed:
 * @item: the #PicmanItem to check.
 *
 * Returns: %TRUE if the 'removed' flag is set for @item, %FALSE otherwise.
 */
gboolean
picman_item_is_removed (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  return GET_PRIVATE (item)->removed;
}

/**
 * picman_item_unset_removed:
 * @item: a #PicmanItem which was on the undo stack
 *
 * Unsets an item's "removed" state. This function is called when an
 * item was on the undo stack and is added back to its parent
 * container during and undo or redo. It must never be called from
 * anywhere else.
 **/
void
picman_item_unset_removed (PicmanItem *item)
{
  PicmanContainer *children;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_is_removed (item));

  GET_PRIVATE (item)->removed = FALSE;

  children = picman_viewable_get_children (PICMAN_VIEWABLE (item));

  if (children)
    picman_container_foreach (children, (GFunc) picman_item_unset_removed, NULL);

  if (PICMAN_ITEM_GET_CLASS (item)->unset_removed)
    PICMAN_ITEM_GET_CLASS (item)->unset_removed (item);
}

/**
 * picman_item_is_attached:
 * @item: The #PicmanItem to check.
 *
 * Returns: %TRUE if the item is attached to an image, %FALSE otherwise.
 */
gboolean
picman_item_is_attached (const PicmanItem *item)
{
  PicmanItem *parent;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  parent = picman_item_get_parent (item);

  if (parent)
    return picman_item_is_attached (parent);

  return PICMAN_ITEM_GET_CLASS (item)->is_attached (item);
}

PicmanItem *
picman_item_get_parent (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);

  return PICMAN_ITEM (picman_viewable_get_parent (PICMAN_VIEWABLE (item)));
}

PicmanItemTree *
picman_item_get_tree (PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);

  if (PICMAN_ITEM_GET_CLASS (item)->get_tree)
    return PICMAN_ITEM_GET_CLASS (item)->get_tree (item);

  return NULL;
}

PicmanContainer *
picman_item_get_container (PicmanItem *item)
{
  PicmanItem     *parent;
  PicmanItemTree *tree;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);

  parent = picman_item_get_parent (item);

  if (parent)
    return picman_viewable_get_children (PICMAN_VIEWABLE (parent));

  tree = picman_item_get_tree (item);

  if (tree)
    return tree->container;

  return NULL;
}

GList *
picman_item_get_container_iter (PicmanItem *item)
{
  PicmanContainer *container;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);

  container = picman_item_get_container (item);

  if (container)
    return PICMAN_LIST (container)->list;

  return NULL;
}

gint
picman_item_get_index (PicmanItem *item)
{
  PicmanContainer *container;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), -1);

  container = picman_item_get_container (item);

  if (container)
    return picman_container_get_child_index (container, PICMAN_OBJECT (item));

  return -1;
}

GList *
picman_item_get_path (PicmanItem *item)
{
  PicmanContainer *container;
  GList         *path = NULL;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);

  container = picman_item_get_container (item);

  while (container)
    {
      guint32 index = picman_container_get_child_index (container,
                                                      PICMAN_OBJECT (item));

      path = g_list_prepend (path, GUINT_TO_POINTER (index));

      item = picman_item_get_parent (item);

      if (item)
        container = picman_item_get_container (item);
      else
        container = NULL;
    }

  return path;
}

/**
 * picman_item_duplicate:
 * @item:     The #PicmanItem to duplicate.
 * @new_type: The type to make the new item.
 *
 * Returns: the newly created item.
 */
PicmanItem *
picman_item_duplicate (PicmanItem *item,
                     GType     new_type)
{
  PicmanItemPrivate *private;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);

  private = GET_PRIVATE (item);

  g_return_val_if_fail (PICMAN_IS_IMAGE (private->image), NULL);
  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_ITEM), NULL);

  return PICMAN_ITEM_GET_CLASS (item)->duplicate (item, new_type);
}

/**
 * picman_item_convert:
 * @item:       The #PicmanItem to convert.
 * @dest_image: The #PicmanImage in which to place the converted item.
 * @new_type:   The type to convert the item to.
 *
 * Returns: the new item that results from the conversion.
 */
PicmanItem *
picman_item_convert (PicmanItem  *item,
                   PicmanImage *dest_image,
                   GType      new_type)
{
  PicmanItem *new_item;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (PICMAN_IS_IMAGE (GET_PRIVATE (item)->image), NULL);
  g_return_val_if_fail (PICMAN_IS_IMAGE (dest_image), NULL);
  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_ITEM), NULL);

  new_item = picman_item_duplicate (item, new_type);

  if (new_item)
    PICMAN_ITEM_GET_CLASS (new_item)->convert (new_item, dest_image);

  return new_item;
}

/**
 * picman_item_rename:
 * @item:     The #PicmanItem to rename.
 * @new_name: The new name to give the item.
 * @error:    Return location for error message.
 *
 * This function assigns a new name to the item, if the desired name is
 * different from the name it already has, and pushes an entry onto the
 * undo stack for the item's image.  If @new_name is NULL or empty, the
 * default name for the item's class is used.  If the name is changed,
 * the PicmanObject::name-changed signal is emitted for the item.
 *
 * Returns: %TRUE if the @item could be renamed, %FALSE otherwise.
 */
gboolean
picman_item_rename (PicmanItem     *item,
                  const gchar  *new_name,
                  GError      **error)
{
  PicmanItemClass *item_class;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  item_class = PICMAN_ITEM_GET_CLASS (item);

  if (! new_name || ! *new_name)
    new_name = item_class->default_name;

  if (strcmp (new_name, picman_object_get_name (item)))
    return item_class->rename (item, new_name, item_class->rename_desc, error);

  return TRUE;
}

/**
 * picman_item_get_width:
 * @item: The #PicmanItem to check.
 *
 * Returns: The width of the item.
 */
gint
picman_item_get_width (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), -1);

  return GET_PRIVATE (item)->width;
}

/**
 * picman_item_get_height:
 * @item: The #PicmanItem to check.
 *
 * Returns: The height of the item.
 */
gint
picman_item_get_height (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), -1);

  return GET_PRIVATE (item)->height;
}

void
picman_item_set_size (PicmanItem *item,
                    gint      width,
                    gint      height)
{
  PicmanItemPrivate *private;

  g_return_if_fail (PICMAN_IS_ITEM (item));

  private = GET_PRIVATE (item);

  if (private->width  != width ||
      private->height != height)
    {
      g_object_freeze_notify (G_OBJECT (item));

      if (private->width != width)
        {
          private->width = width;
          g_object_notify (G_OBJECT (item), "width");
        }

      if (private->height != height)
        {
          private->height = height;
          g_object_notify (G_OBJECT (item), "height");
        }

      g_object_thaw_notify (G_OBJECT (item));

      picman_viewable_size_changed (PICMAN_VIEWABLE (item));
    }
}

/**
 * picman_item_get_offset:
 * @item:     The #PicmanItem to check.
 * @offset_x: Return location for the item's X offset.
 * @offset_y: Return location for the item's Y offset.
 *
 * Reveals the X and Y offsets of the item.
 */
void
picman_item_get_offset (const PicmanItem *item,
                      gint           *offset_x,
                      gint           *offset_y)
{
  PicmanItemPrivate *private;

  g_return_if_fail (PICMAN_IS_ITEM (item));

  private = GET_PRIVATE (item);

  if (offset_x) *offset_x = private->offset_x;
  if (offset_y) *offset_y = private->offset_y;
}

void
picman_item_set_offset (PicmanItem *item,
                      gint      offset_x,
                      gint      offset_y)
{
  PicmanItemPrivate *private;
  GList           *list;

  g_return_if_fail (PICMAN_IS_ITEM (item));

  private = GET_PRIVATE (item);

  g_object_freeze_notify (G_OBJECT (item));

  if (private->offset_x != offset_x)
    {
      private->offset_x = offset_x;
      g_object_notify (G_OBJECT (item), "offset-x");
    }

  if (private->offset_y != offset_y)
    {
      private->offset_y = offset_y;
      g_object_notify (G_OBJECT (item), "offset-y");
    }

  for (list = private->offset_nodes; list; list = g_list_next (list))
    {
      GeglNode *node = list->data;

      gegl_node_set (node,
                     "x", (gdouble) private->offset_x,
                     "y", (gdouble) private->offset_y,
                     NULL);
    }

  g_object_thaw_notify (G_OBJECT (item));
}

gint
picman_item_get_offset_x (PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), 0);

  return GET_PRIVATE (item)->offset_x;
}

gint
picman_item_get_offset_y (PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), 0);

  return GET_PRIVATE (item)->offset_y;
}

/**
 * picman_item_translate:
 * @item:      The #PicmanItem to move.
 * @offset_x:  Increment to the X offset of the item.
 * @offset_y:  Increment to the Y offset of the item.
 * @push_undo: If #TRUE, create an entry in the image's undo stack
 *             for this action.
 *
 * Adds the specified increments to the X and Y offsets for the item.
 */
void
picman_item_translate (PicmanItem *item,
                     gint      offset_x,
                     gint      offset_y,
                     gboolean  push_undo)
{
  PicmanItemClass *item_class;
  PicmanImage     *image;

  g_return_if_fail (PICMAN_IS_ITEM (item));

  item_class = PICMAN_ITEM_GET_CLASS (item);
  image = picman_item_get_image (item);

  if (! picman_item_is_attached (item))
    push_undo = FALSE;

  if (push_undo)
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_ITEM_DISPLACE,
                                 item_class->translate_desc);

  item_class->translate (item, offset_x, offset_y, push_undo);

  if (push_undo)
    picman_image_undo_group_end (image);
}

/**
 * picman_item_check_scaling:
 * @item:       Item to check
 * @new_width:  proposed width of item, in pixels
 * @new_height: proposed height of item, in pixels
 *
 * Scales item dimensions, then snaps them to pixel centers
 *
 * Returns: #FALSE if any dimension reduces to zero as a result
 *          of this; otherwise, returns #TRUE.
 **/
gboolean
picman_item_check_scaling (const PicmanItem *item,
                         gint            new_width,
                         gint            new_height)
{
  PicmanImage *image;
  gdouble    img_scale_w;
  gdouble    img_scale_h;
  gint       new_item_width;
  gint       new_item_height;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  image = picman_item_get_image (item);

  img_scale_w     = ((gdouble) new_width /
                     (gdouble) picman_image_get_width (image));
  img_scale_h     = ((gdouble) new_height /
                     (gdouble) picman_image_get_height (image));
  new_item_width  = ROUND (img_scale_w * (gdouble) picman_item_get_width  (item));
  new_item_height = ROUND (img_scale_h * (gdouble) picman_item_get_height (item));

  return (new_item_width > 0 && new_item_height > 0);
}

void
picman_item_scale (PicmanItem              *item,
                 gint                   new_width,
                 gint                   new_height,
                 gint                   new_offset_x,
                 gint                   new_offset_y,
                 PicmanInterpolationType  interpolation,
                 PicmanProgress          *progress)
{
  PicmanItemClass *item_class;
  PicmanImage     *image;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  if (new_width < 1 || new_height < 1)
    return;

  item_class = PICMAN_ITEM_GET_CLASS (item);
  image = picman_item_get_image (item);

  if (picman_item_is_attached (item))
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_ITEM_SCALE,
                                 item_class->scale_desc);

  g_object_freeze_notify (G_OBJECT (item));

  item_class->scale (item, new_width, new_height, new_offset_x, new_offset_y,
                     interpolation, progress);

  g_object_thaw_notify (G_OBJECT (item));

  if (picman_item_is_attached (item))
    picman_image_undo_group_end (image);
}

/**
 * picman_item_scale_by_factors:
 * @item:     Item to be transformed by explicit width and height factors.
 * @w_factor: scale factor to apply to width and horizontal offset
 * @h_factor: scale factor to apply to height and vertical offset
 * @interpolation:
 * @progress:
 *
 * Scales item dimensions and offsets by uniform width and
 * height factors.
 *
 * Use picman_item_scale_by_factors() in circumstances when the same
 * width and height scaling factors are to be uniformly applied to a
 * set of items. In this context, the item's dimensions and offsets
 * from the sides of the containing image all change by these
 * predetermined factors. By fiat, the fixed point of the transform is
 * the upper left hand corner of the image. Returns #FALSE if a
 * requested scale factor is zero or if a scaling zero's out a item
 * dimension; returns #TRUE otherwise.
 *
 * Use picman_item_scale() in circumstances where new item width
 * and height dimensions are predetermined instead.
 *
 * Side effects: Undo set created for item. Old item imagery
 *               scaled & painted to new item tiles.
 *
 * Returns: #TRUE, if the scaled item has positive dimensions
 *          #FALSE if the scaled item has at least one zero dimension
 **/
gboolean
picman_item_scale_by_factors (PicmanItem              *item,
                            gdouble                w_factor,
                            gdouble                h_factor,
                            PicmanInterpolationType  interpolation,
                            PicmanProgress          *progress)
{
  PicmanItemPrivate *private;
  gint             new_width, new_height;
  gint             new_offset_x, new_offset_y;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), FALSE);

  private = GET_PRIVATE (item);

  if (w_factor == 0.0 || h_factor == 0.0)
    {
      g_warning ("%s: requested width or height scale equals zero", G_STRFUNC);
      return FALSE;
    }

  new_offset_x = SIGNED_ROUND (w_factor * (gdouble) private->offset_x);
  new_offset_y = SIGNED_ROUND (h_factor * (gdouble) private->offset_y);
  new_width    = ROUND (w_factor * (gdouble) picman_item_get_width  (item));
  new_height   = ROUND (h_factor * (gdouble) picman_item_get_height (item));

  if (new_width != 0 && new_height != 0)
    {
      picman_item_scale (item,
                       new_width, new_height,
                       new_offset_x, new_offset_y,
                       interpolation, progress);
      return TRUE;
    }

  return FALSE;
}

/**
 * picman_item_scale_by_origin:
 * @item:         The item to be transformed by width & height scale factors
 * @new_width:    The width that item will acquire
 * @new_height:   The height that the item will acquire
 * @interpolation:
 * @progress:
 * @local_origin: sets fixed point of the scaling transform. See below.
 *
 * Sets item dimensions to new_width and
 * new_height. Derives vertical and horizontal scaling
 * transforms from new width and height. If local_origin is
 * #TRUE, the fixed point of the scaling transform coincides
 * with the item's center point.  Otherwise, the fixed
 * point is taken to be [-PicmanItem::offset_x, -PicmanItem::->offset_y].
 *
 * Since this function derives scale factors from new and
 * current item dimensions, these factors will vary from
 * item to item because of aliasing artifacts; factor
 * variations among items can be quite large where item
 * dimensions approach pixel dimensions. Use
 * picman_item_scale_by_factors() where constant scales are to
 * be uniformly applied to a number of items.
 *
 * Side effects: undo set created for item.
 *               Old item imagery scaled
 *               & painted to new item tiles
 **/
void
picman_item_scale_by_origin (PicmanItem              *item,
                           gint                   new_width,
                           gint                   new_height,
                           PicmanInterpolationType  interpolation,
                           PicmanProgress          *progress,
                           gboolean               local_origin)
{
  PicmanItemPrivate *private;
  gint             new_offset_x, new_offset_y;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  private = GET_PRIVATE (item);

  if (new_width == 0 || new_height == 0)
    {
      g_warning ("%s: requested width or height equals zero", G_STRFUNC);
      return;
    }

  if (local_origin)
    {
      new_offset_x = (private->offset_x +
                      ((picman_item_get_width  (item) - new_width)  / 2.0));
      new_offset_y = (private->offset_y +
                      ((picman_item_get_height (item) - new_height) / 2.0));
    }
  else
    {
      new_offset_x = (gint) (((gdouble) new_width *
                              (gdouble) private->offset_x /
                              (gdouble) picman_item_get_width (item)));

      new_offset_y = (gint) (((gdouble) new_height *
                              (gdouble) private->offset_y /
                              (gdouble) picman_item_get_height (item)));
    }

  picman_item_scale (item,
                   new_width, new_height,
                   new_offset_x, new_offset_y,
                   interpolation, progress);
}

void
picman_item_resize (PicmanItem    *item,
                  PicmanContext *context,
                  gint         new_width,
                  gint         new_height,
                  gint         offset_x,
                  gint         offset_y)
{
  PicmanItemClass *item_class;
  PicmanImage     *image;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  if (new_width < 1 || new_height < 1)
    return;

  item_class = PICMAN_ITEM_GET_CLASS (item);
  image = picman_item_get_image (item);

  if (picman_item_is_attached (item))
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_ITEM_RESIZE,
                                 item_class->resize_desc);

  g_object_freeze_notify (G_OBJECT (item));

  item_class->resize (item, context, new_width, new_height, offset_x, offset_y);

  g_object_thaw_notify (G_OBJECT (item));

  if (picman_item_is_attached (item))
    picman_image_undo_group_end (image);
}

void
picman_item_flip (PicmanItem            *item,
                PicmanContext         *context,
                PicmanOrientationType  flip_type,
                gdouble              axis,
                gboolean             clip_result)
{
  PicmanItemClass *item_class;
  PicmanImage     *image;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_is_attached (item));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  item_class = PICMAN_ITEM_GET_CLASS (item);
  image = picman_item_get_image (item);

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TRANSFORM,
                               item_class->flip_desc);

  g_object_freeze_notify (G_OBJECT (item));

  item_class->flip (item, context, flip_type, axis, clip_result);

  g_object_thaw_notify (G_OBJECT (item));

  picman_image_undo_group_end (image);
}

void
picman_item_rotate (PicmanItem         *item,
                  PicmanContext      *context,
                  PicmanRotationType  rotate_type,
                  gdouble           center_x,
                  gdouble           center_y,
                  gboolean          clip_result)
{
  PicmanItemClass *item_class;
  PicmanImage     *image;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_is_attached (item));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  item_class = PICMAN_ITEM_GET_CLASS (item);
  image = picman_item_get_image (item);

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TRANSFORM,
                               item_class->rotate_desc);

  g_object_freeze_notify (G_OBJECT (item));

  item_class->rotate (item, context, rotate_type, center_x, center_y,
                      clip_result);

  g_object_thaw_notify (G_OBJECT (item));

  picman_image_undo_group_end (image);
}

void
picman_item_transform (PicmanItem               *item,
                     PicmanContext            *context,
                     const PicmanMatrix3      *matrix,
                     PicmanTransformDirection  direction,
                     PicmanInterpolationType   interpolation,
                     gint                    recursion_level,
                     PicmanTransformResize     clip_result,
                     PicmanProgress           *progress)
{
  PicmanItemClass *item_class;
  PicmanImage     *image;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_is_attached (item));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (matrix != NULL);
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  item_class = PICMAN_ITEM_GET_CLASS (item);
  image = picman_item_get_image (item);

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TRANSFORM,
                               item_class->transform_desc);

  g_object_freeze_notify (G_OBJECT (item));

  item_class->transform (item, context, matrix, direction, interpolation,
                         recursion_level, clip_result, progress);

  g_object_thaw_notify (G_OBJECT (item));

  picman_image_undo_group_end (image);
}

gboolean
picman_item_stroke (PicmanItem          *item,
                  PicmanDrawable      *drawable,
                  PicmanContext       *context,
                  PicmanStrokeOptions *stroke_options,
                  gboolean           use_default_values,
                  gboolean           push_undo,
                  PicmanProgress      *progress,
                  GError           **error)
{
  PicmanItemClass *item_class;
  gboolean       retval = FALSE;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (picman_item_is_attached (item), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (stroke_options), FALSE);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  item_class = PICMAN_ITEM_GET_CLASS (item);

  if (item_class->stroke)
    {
      PicmanImage *image = picman_item_get_image (item);

      picman_stroke_options_prepare (stroke_options, context, use_default_values);

      if (push_undo)
        picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_PAINT,
                                     item_class->stroke_desc);

      retval = item_class->stroke (item, drawable, stroke_options, push_undo,
                                   progress, error);

      if (push_undo)
        picman_image_undo_group_end (image);

      picman_stroke_options_finish (stroke_options);
    }

  return retval;
}

void
picman_item_to_selection (PicmanItem       *item,
                        PicmanChannelOps  op,
                        gboolean        antialias,
                        gboolean        feather,
                        gdouble         feather_radius_x,
                        gdouble         feather_radius_y)
{
  PicmanItemClass *item_class;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_is_attached (item));

  item_class = PICMAN_ITEM_GET_CLASS (item);

  if (item_class->to_selection)
    item_class->to_selection (item, op, antialias,
                              feather, feather_radius_x, feather_radius_y);
}

void
picman_item_add_offset_node (PicmanItem *item,
                           GeglNode *node)
{
  PicmanItemPrivate *private;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (GEGL_IS_NODE (node));

  private = GET_PRIVATE (item);

  g_return_if_fail (g_list_find (private->offset_nodes, node) == NULL);

  gegl_node_set (node,
                 "x", (gdouble) private->offset_x,
                 "y", (gdouble) private->offset_y,
                 NULL);

  private->offset_nodes = g_list_append (private->offset_nodes,
                                         g_object_ref (node));
}

void
picman_item_remove_offset_node (PicmanItem *item,
                              GeglNode *node)
{
  PicmanItemPrivate *private;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (GEGL_IS_NODE (node));

  private = GET_PRIVATE (item);

  g_return_if_fail (g_list_find (private->offset_nodes, node) != NULL);

  private->offset_nodes = g_list_remove (private->offset_nodes, node);
  g_object_unref (node);
}

gint
picman_item_get_ID (PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), -1);

  return GET_PRIVATE (item)->ID;
}

PicmanItem *
picman_item_get_by_ID (Picman *picman,
                     gint  item_id)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (picman->item_table == NULL)
    return NULL;

  return (PicmanItem *) picman_id_table_lookup (picman->item_table, item_id);
}

PicmanTattoo
picman_item_get_tattoo (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), 0);

  return GET_PRIVATE (item)->tattoo;
}

void
picman_item_set_tattoo (PicmanItem   *item,
                      PicmanTattoo  tattoo)
{
  g_return_if_fail (PICMAN_IS_ITEM (item));

  GET_PRIVATE (item)->tattoo = tattoo;
}

PicmanImage *
picman_item_get_image (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);

  return GET_PRIVATE (item)->image;
}

void
picman_item_set_image (PicmanItem  *item,
                     PicmanImage *image)
{
  PicmanItemPrivate *private;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (! picman_item_is_attached (item));
  g_return_if_fail (! picman_item_is_removed (item));
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = GET_PRIVATE (item);

  if (image == private->image)
    return;

  g_object_freeze_notify (G_OBJECT (item));

  if (private->ID == 0)
    {
      private->ID = picman_id_table_insert (image->picman->item_table, item);

      g_object_notify (G_OBJECT (item), "id");
    }

  if (private->tattoo == 0 || private->image != image)
    {
      private->tattoo = picman_image_get_new_tattoo (image);
    }

  private->image = image;
  g_object_notify (G_OBJECT (item), "image");

  g_object_thaw_notify (G_OBJECT (item));
}

/**
 * picman_item_replace_item:
 * @item: a newly allocated #PicmanItem
 * @replace: the #PicmanItem to be replaced by @item
 *
 * This function shouly only be called right after @item has been
 * newly allocated.
 *
 * Replaces @replace by @item, as far as possible within the #PicmanItem
 * class. The new @item takes over @replace's ID, tattoo, offset, size
 * etc. and all these properties are set to %NULL on @replace.
 *
 * This function *only* exists to allow subclasses to do evil hacks
 * like in XCF text layer loading. Don't ever use this function if you
 * are not sure.
 *
 * After this function returns, @replace has become completely
 * unusable, should only be used to steal everything it has (like its
 * drawable properties if it's a drawable), and then be destroyed.
 **/
void
picman_item_replace_item (PicmanItem *item,
                        PicmanItem *replace)
{
  PicmanItemPrivate *private;
  gint             offset_x;
  gint             offset_y;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (! picman_item_is_attached (item));
  g_return_if_fail (! picman_item_is_removed (item));
  g_return_if_fail (PICMAN_IS_ITEM (replace));

  private = GET_PRIVATE (item);

  picman_object_set_name (PICMAN_OBJECT (item), picman_object_get_name (replace));

  if (private->ID)
    picman_id_table_remove (picman_item_get_image (item)->picman->item_table,
                          picman_item_get_ID (item));

  private->ID = picman_item_get_ID (replace);
  picman_id_table_replace (picman_item_get_image (item)->picman->item_table,
                         picman_item_get_ID (item),
                         item);

  /* Set image before tatoo so that the explicitly set tatoo overrides
   * the one implicitly set when setting the image
   */
  picman_item_set_image (item, picman_item_get_image (replace));
  GET_PRIVATE (replace)->image  = NULL;

  picman_item_set_tattoo (item, picman_item_get_tattoo (replace));
  picman_item_set_tattoo (replace, 0);

  g_object_unref (private->parasites);
  private->parasites = GET_PRIVATE (replace)->parasites;
  GET_PRIVATE (replace)->parasites = NULL;

  picman_item_get_offset (replace, &offset_x, &offset_y);
  picman_item_set_offset (item, offset_x, offset_y);

  picman_item_set_size (item,
                      picman_item_get_width  (replace),
                      picman_item_get_height (replace));

  picman_item_set_visible       (item, picman_item_get_visible (replace), FALSE);
  picman_item_set_linked        (item, picman_item_get_linked (replace), FALSE);
  picman_item_set_lock_content  (item, picman_item_get_lock_content (replace), FALSE);
  picman_item_set_lock_position (item, picman_item_get_lock_position (replace), FALSE);
}

/**
 * picman_item_set_parasites:
 * @item: a #PicmanItem
 * @parasites: a #PicmanParasiteList
 *
 * Set an @item's #PicmanParasiteList. It's usually never needed to
 * fiddle with an item's parasite list directly. This function exists
 * for special purposes only, like when creating items from unusual
 * sources.
 **/
void
picman_item_set_parasites (PicmanItem         *item,
                         PicmanParasiteList *parasites)
{
  PicmanItemPrivate *private;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (PICMAN_IS_PARASITE_LIST (parasites));

  private = GET_PRIVATE (item);

  if (parasites != private->parasites)
    {
      g_object_unref (private->parasites);
      private->parasites = g_object_ref (parasites);
    }
}

/**
 * picman_item_get_parasites:
 * @item: a #PicmanItem
 *
 * Get an @item's #PicmanParasiteList. It's usually never needed to
 * fiddle with an item's parasite list directly. This function exists
 * for special purposes only, like when saving an item to XCF.
 *
 * Return value: The @item's #PicmanParasiteList.
 **/
PicmanParasiteList *
picman_item_get_parasites (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);

  return GET_PRIVATE (item)->parasites;
}

void
picman_item_parasite_attach (PicmanItem           *item,
                           const PicmanParasite *parasite,
                           gboolean            push_undo)
{
  PicmanItemPrivate *private;
  PicmanParasite     copy;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (parasite != NULL);

  private = GET_PRIVATE (item);

  /*  make a temporary copy of the PicmanParasite struct because
   *  picman_parasite_shift_parent() changes it
   */
  copy = *parasite;

  if (! picman_item_is_attached (item))
    push_undo = FALSE;

  if (push_undo)
    {
      /*  only set the dirty bit manually if we can be saved and the new
       *  parasite differs from the current one and we aren't undoable
       */
      if (picman_parasite_is_undoable (&copy))
        {
          /* do a group in case we have attach_parent set */
          picman_image_undo_group_start (private->image,
                                       PICMAN_UNDO_GROUP_PARASITE_ATTACH,
                                       C_("undo-type", "Attach Parasite"));

          picman_image_undo_push_item_parasite (private->image, NULL, item, &copy);
        }
      else if (picman_parasite_is_persistent (&copy) &&
               ! picman_parasite_compare (&copy,
                                        picman_item_parasite_find
                                        (item, picman_parasite_name (&copy))))
        {
          picman_image_undo_push_cantundo (private->image,
                                         C_("undo-type", "Attach Parasite to Item"));
        }
    }

  picman_parasite_list_add (private->parasites, &copy);

  if (picman_parasite_has_flag (&copy, PICMAN_PARASITE_ATTACH_PARENT))
    {
      picman_parasite_shift_parent (&copy);
      picman_image_parasite_attach (private->image, &copy);
    }
  else if (picman_parasite_has_flag (&copy, PICMAN_PARASITE_ATTACH_GRANDPARENT))
    {
      picman_parasite_shift_parent (&copy);
      picman_parasite_shift_parent (&copy);
      picman_parasite_attach (private->image->picman, &copy);
    }

  if (picman_item_is_attached (item) &&
      picman_parasite_is_undoable (&copy))
    {
      picman_image_undo_group_end (private->image);
    }
}

void
picman_item_parasite_detach (PicmanItem    *item,
                           const gchar *name,
                           gboolean     push_undo)
{
  PicmanItemPrivate    *private;
  const PicmanParasite *parasite;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (name != NULL);

  private = GET_PRIVATE (item);

  parasite = picman_parasite_list_find (private->parasites, name);

  if (! parasite)
    return;

  if (! picman_item_is_attached (item))
    push_undo = FALSE;

  if (push_undo)
    {
      if (picman_parasite_is_undoable (parasite))
        {
          picman_image_undo_push_item_parasite_remove (private->image,
                                                     C_("undo-type", "Remove Parasite from Item"),
                                                     item,
                                                     picman_parasite_name (parasite));
        }
      else if (picman_parasite_is_persistent (parasite))
        {
          picman_image_undo_push_cantundo (private->image,
                                         C_("undo-type", "Remove Parasite from Item"));
        }
    }

  picman_parasite_list_remove (private->parasites, name);
}

const PicmanParasite *
picman_item_parasite_find (const PicmanItem *item,
                         const gchar    *name)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);

  return picman_parasite_list_find (GET_PRIVATE (item)->parasites, name);
}

static void
picman_item_parasite_list_foreach_func (gchar          *name,
                                      PicmanParasite   *parasite,
                                      gchar        ***cur)
{
  *(*cur)++ = (gchar *) g_strdup (name);
}

gchar **
picman_item_parasite_list (const PicmanItem *item,
                         gint           *count)
{
  PicmanItemPrivate  *private;
  gchar           **list;
  gchar           **cur;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (count != NULL, NULL);

  private = GET_PRIVATE (item);

  *count = picman_parasite_list_length (private->parasites);

  cur = list = g_new (gchar *, *count);

  picman_parasite_list_foreach (private->parasites,
                              (GHFunc) picman_item_parasite_list_foreach_func,
                              &cur);

  return list;
}

void
picman_item_set_visible (PicmanItem *item,
                       gboolean  visible,
                       gboolean  push_undo)
{
  g_return_if_fail (PICMAN_IS_ITEM (item));

  visible = visible ? TRUE : FALSE;

  if (picman_item_get_visible (item) != visible)
    {
      if (push_undo && picman_item_is_attached (item))
        {
          PicmanImage *image = picman_item_get_image (item);

          if (image)
            picman_image_undo_push_item_visibility (image, NULL, item);
        }

      GET_PRIVATE (item)->visible = visible;

      g_signal_emit (item, picman_item_signals[VISIBILITY_CHANGED], 0);

      g_object_notify (G_OBJECT (item), "visible");
    }
}

gboolean
picman_item_get_visible (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  return GET_PRIVATE (item)->visible;
}

gboolean
picman_item_is_visible (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  if (picman_item_get_visible (item))
    {
      PicmanItem *parent = picman_item_get_parent (item);

      if (parent)
        return picman_item_is_visible (parent);

      return TRUE;
    }

  return FALSE;
}

void
picman_item_set_linked (PicmanItem *item,
                      gboolean  linked,
                      gboolean  push_undo)
{
  g_return_if_fail (PICMAN_IS_ITEM (item));

  linked = linked ? TRUE : FALSE;

  if (picman_item_get_linked (item) != linked)
    {
      if (push_undo && picman_item_is_attached (item))
        {
          PicmanImage *image = picman_item_get_image (item);

          if (image)
            picman_image_undo_push_item_linked (image, NULL, item);
        }

      GET_PRIVATE (item)->linked = linked;

      g_signal_emit (item, picman_item_signals[LINKED_CHANGED], 0);

      g_object_notify (G_OBJECT (item), "linked");
    }
}

gboolean
picman_item_get_linked (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  return GET_PRIVATE (item)->linked;
}

void
picman_item_set_lock_content (PicmanItem *item,
                            gboolean  lock_content,
                            gboolean  push_undo)
{
  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_can_lock_content (item));

  lock_content = lock_content ? TRUE : FALSE;

  if (picman_item_get_lock_content (item) != lock_content)
    {
      if (push_undo && picman_item_is_attached (item))
        {
          /* Right now I don't think this should be pushed. */
#if 0
          PicmanImage *image = picman_item_get_image (item);

          picman_image_undo_push_item_lock_content (image, NULL, item);
#endif
        }

      GET_PRIVATE (item)->lock_content = lock_content;

      g_signal_emit (item, picman_item_signals[LOCK_CONTENT_CHANGED], 0);

      g_object_notify (G_OBJECT (item), "lock-content");
    }
}

gboolean
picman_item_get_lock_content (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  return GET_PRIVATE (item)->lock_content;
}

gboolean
picman_item_can_lock_content (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  return TRUE;
}

gboolean
picman_item_is_content_locked (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  return PICMAN_ITEM_GET_CLASS (item)->is_content_locked (item);
}

void
picman_item_set_lock_position (PicmanItem *item,
                             gboolean  lock_position,
                             gboolean  push_undo)
{
  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_can_lock_position (item));

  lock_position = lock_position ? TRUE : FALSE;

  if (picman_item_get_lock_position (item) != lock_position)
    {
      if (push_undo && picman_item_is_attached (item))
        {
          PicmanImage *image = picman_item_get_image (item);

          picman_image_undo_push_item_lock_position (image, NULL, item);
        }

      GET_PRIVATE (item)->lock_position = lock_position;

      g_signal_emit (item, picman_item_signals[LOCK_POSITION_CHANGED], 0);

      g_object_notify (G_OBJECT (item), "lock-position");
    }
}

gboolean
picman_item_get_lock_position (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  return GET_PRIVATE (item)->lock_position;
}

gboolean
picman_item_can_lock_position (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  if (picman_viewable_get_children (PICMAN_VIEWABLE (item)))
    return FALSE;

  return TRUE;
}

gboolean
picman_item_is_position_locked (const PicmanItem *item)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  return PICMAN_ITEM_GET_CLASS (item)->is_position_locked (item);
}

gboolean
picman_item_mask_bounds (PicmanItem *item,
                       gint     *x1,
                       gint     *y1,
                       gint     *x2,
                       gint     *y2)
{
  PicmanImage   *image;
  PicmanChannel *selection;
  gint         tmp_x1, tmp_y1;
  gint         tmp_x2, tmp_y2;
  gboolean     retval;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (picman_item_is_attached (item), FALSE);

  image     = picman_item_get_image (item);
  selection = picman_image_get_mask (image);

  if (PICMAN_ITEM (selection) != item &&
      picman_channel_bounds (selection, &tmp_x1, &tmp_y1, &tmp_x2, &tmp_y2))
    {
      gint off_x, off_y;

      picman_item_get_offset (item, &off_x, &off_y);

      tmp_x1 = CLAMP (tmp_x1 - off_x, 0, picman_item_get_width  (item));
      tmp_y1 = CLAMP (tmp_y1 - off_y, 0, picman_item_get_height (item));
      tmp_x2 = CLAMP (tmp_x2 - off_x, 0, picman_item_get_width  (item));
      tmp_y2 = CLAMP (tmp_y2 - off_y, 0, picman_item_get_height (item));

      retval = TRUE;
    }
  else
    {
      tmp_x1 = 0;
      tmp_y1 = 0;
      tmp_x2 = picman_item_get_width  (item);
      tmp_y2 = picman_item_get_height (item);

      retval = FALSE;
    }

  if (x1) *x1 = tmp_x1;
  if (y1) *y1 = tmp_y1;
  if (x2) *x2 = tmp_x2;
  if (y2) *y2 = tmp_y2;

  return retval;
}

gboolean
picman_item_mask_intersect (PicmanItem *item,
                          gint     *x,
                          gint     *y,
                          gint     *width,
                          gint     *height)
{
  PicmanImage   *image;
  PicmanChannel *selection;
  gint         tmp_x, tmp_y;
  gint         tmp_width, tmp_height;
  gboolean     retval;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (picman_item_is_attached (item), FALSE);

  image     = picman_item_get_image (item);
  selection = picman_image_get_mask (image);

  if (PICMAN_ITEM (selection) != item &&
      picman_channel_bounds (selection, &tmp_x, &tmp_y, &tmp_width, &tmp_height))
    {
      gint off_x, off_y;

      picman_item_get_offset (item, &off_x, &off_y);

      tmp_width  -= tmp_x;
      tmp_height -= tmp_y;

      retval = picman_rectangle_intersect (tmp_x - off_x, tmp_y - off_y,
                                         tmp_width, tmp_height,
                                         0, 0,
                                         picman_item_get_width  (item),
                                         picman_item_get_height (item),
                                         &tmp_x, &tmp_y,
                                         &tmp_width, &tmp_height);
    }
  else
    {
      tmp_x      = 0;
      tmp_y      = 0;
      tmp_width  = picman_item_get_width  (item);
      tmp_height = picman_item_get_height (item);

      retval = TRUE;
    }

  if (x)      *x      = tmp_x;
  if (y)      *y      = tmp_y;
  if (width)  *width  = tmp_width;
  if (height) *height = tmp_height;

  return retval;
}

gboolean
picman_item_is_in_set (PicmanItem    *item,
                     PicmanItemSet  set)
{
  PicmanItemPrivate *private;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  private = GET_PRIVATE (item);

  switch (set)
    {
    case PICMAN_ITEM_SET_NONE:
      return FALSE;

    case PICMAN_ITEM_SET_ALL:
      return TRUE;

    case PICMAN_ITEM_SET_IMAGE_SIZED:
      return (picman_item_get_width  (item) == picman_image_get_width  (private->image) &&
              picman_item_get_height (item) == picman_image_get_height (private->image));

    case PICMAN_ITEM_SET_VISIBLE:
      return picman_item_get_visible (item);

    case PICMAN_ITEM_SET_LINKED:
      return picman_item_get_linked (item);
    }

  return FALSE;
}
