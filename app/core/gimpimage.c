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

#include <string.h>
#include <time.h>

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "config/picmancoreconfig.h"

#include "gegl/picman-babl.h"

#include "picman.h"
#include "picman-parasites.h"
#include "picman-utils.h"
#include "picmancontext.h"
#include "picmandrawablestack.h"
#include "picmangrid.h"
#include "picmanerror.h"
#include "picmanguide.h"
#include "picmanidtable.h"
#include "picmanimage.h"
#include "picmanimage-colormap.h"
#include "picmanimage-guides.h"
#include "picmanimage-sample-points.h"
#include "picmanimage-preview.h"
#include "picmanimage-private.h"
#include "picmanimage-quick-mask.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanitemtree.h"
#include "picmanlayer.h"
#include "picmanlayer-floating-sel.h"
#include "picmanlayermask.h"
#include "picmanmarshal.h"
#include "picmanparasitelist.h"
#include "picmanpickable.h"
#include "picmanprojectable.h"
#include "picmanprojection.h"
#include "picmansamplepoint.h"
#include "picmanselection.h"
#include "picmantempbuf.h"
#include "picmantemplate.h"
#include "picmanundostack.h"

#include "file/file-utils.h"

#include "vectors/picmanvectors.h"

#include "picman-intl.h"


#ifdef DEBUG
#define TRC(x) g_printerr x
#else
#define TRC(x)
#endif

/* Data keys for PicmanImage */
#define PICMAN_FILE_EXPORT_URI_KEY        "picman-file-export-uri"
#define PICMAN_FILE_SAVE_A_COPY_URI_KEY   "picman-file-save-a-copy-uri"
#define PICMAN_FILE_IMPORT_SOURCE_URI_KEY "picman-file-import-source-uri"


enum
{
  MODE_CHANGED,
  PRECISION_CHANGED,
  ALPHA_CHANGED,
  FLOATING_SELECTION_CHANGED,
  ACTIVE_LAYER_CHANGED,
  ACTIVE_CHANNEL_CHANGED,
  ACTIVE_VECTORS_CHANGED,
  COMPONENT_VISIBILITY_CHANGED,
  COMPONENT_ACTIVE_CHANGED,
  MASK_CHANGED,
  RESOLUTION_CHANGED,
  SIZE_CHANGED_DETAILED,
  UNIT_CHANGED,
  QUICK_MASK_CHANGED,
  SELECTION_INVALIDATE,
  CLEAN,
  DIRTY,
  SAVED,
  EXPORTED,
  GUIDE_ADDED,
  GUIDE_REMOVED,
  GUIDE_MOVED,
  SAMPLE_POINT_ADDED,
  SAMPLE_POINT_REMOVED,
  SAMPLE_POINT_MOVED,
  PARASITE_ATTACHED,
  PARASITE_DETACHED,
  COLORMAP_CHANGED,
  UNDO_EVENT,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_PICMAN,
  PROP_ID,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_BASE_TYPE,
  PROP_PRECISION
};


/*  local function prototypes  */

static void     picman_color_managed_iface_init    (PicmanColorManagedInterface *iface);
static void     picman_projectable_iface_init      (PicmanProjectableInterface  *iface);

static void     picman_image_constructed           (GObject           *object);
static void     picman_image_set_property          (GObject           *object,
                                                  guint              property_id,
                                                  const GValue      *value,
                                                  GParamSpec        *pspec);
static void     picman_image_get_property          (GObject           *object,
                                                  guint              property_id,
                                                  GValue            *value,
                                                  GParamSpec        *pspec);
static void     picman_image_dispose               (GObject           *object);
static void     picman_image_finalize              (GObject           *object);

static void     picman_image_name_changed          (PicmanObject        *object);
static gint64   picman_image_get_memsize           (PicmanObject        *object,
                                                  gint64            *gui_size);

static gboolean picman_image_get_size              (PicmanViewable      *viewable,
                                                  gint              *width,
                                                  gint              *height);
static void     picman_image_size_changed          (PicmanViewable      *viewable);
static gchar  * picman_image_get_description       (PicmanViewable      *viewable,
                                                  gchar            **tooltip);

static void     picman_image_real_mode_changed     (PicmanImage         *image);
static void     picman_image_real_precision_changed(PicmanImage         *image);
static void     picman_image_real_size_changed_detailed
                                                 (PicmanImage         *image,
                                                  gint               previous_origin_x,
                                                  gint               previous_origin_y,
                                                  gint               previous_width,
                                                  gint               previous_height);
static void     picman_image_real_colormap_changed (PicmanImage         *image,
                                                  gint               color_index);

static const guint8 * picman_image_get_icc_profile (PicmanColorManaged  *managed,
                                                  gsize             *len);

static void        picman_image_projectable_flush  (PicmanProjectable   *projectable,
                                                  gboolean           invalidate_preview);
static GeglNode   * picman_image_get_graph         (PicmanProjectable   *projectable);
static PicmanImage  * picman_image_get_image         (PicmanProjectable   *projectable);
static const Babl * picman_image_get_proj_format   (PicmanProjectable   *projectable);

static void     picman_image_mask_update           (PicmanDrawable      *drawable,
                                                  gint               x,
                                                  gint               y,
                                                  gint               width,
                                                  gint               height,
                                                  PicmanImage         *image);
static void     picman_image_layer_alpha_changed   (PicmanDrawable      *drawable,
                                                  PicmanImage         *image);
static void     picman_image_channel_add           (PicmanContainer     *container,
                                                  PicmanChannel       *channel,
                                                  PicmanImage         *image);
static void     picman_image_channel_remove        (PicmanContainer     *container,
                                                  PicmanChannel       *channel,
                                                  PicmanImage         *image);
static void     picman_image_channel_name_changed  (PicmanChannel       *channel,
                                                  PicmanImage         *image);
static void     picman_image_channel_color_changed (PicmanChannel       *channel,
                                                  PicmanImage         *image);
static void     picman_image_active_layer_notify   (PicmanItemTree      *tree,
                                                  const GParamSpec  *pspec,
                                                  PicmanImage         *image);
static void     picman_image_active_channel_notify (PicmanItemTree      *tree,
                                                  const GParamSpec  *pspec,
                                                  PicmanImage         *image);
static void     picman_image_active_vectors_notify (PicmanItemTree      *tree,
                                                  const GParamSpec  *pspec,
                                                  PicmanImage         *image);


G_DEFINE_TYPE_WITH_CODE (PicmanImage, picman_image, PICMAN_TYPE_VIEWABLE,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_COLOR_MANAGED,
                                                picman_color_managed_iface_init)
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROJECTABLE,
                                                picman_projectable_iface_init))

#define parent_class picman_image_parent_class

static guint picman_image_signals[LAST_SIGNAL] = { 0 };


static void
picman_image_class_init (PicmanImageClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);

  picman_image_signals[MODE_CHANGED] =
    g_signal_new ("mode-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, mode_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[PRECISION_CHANGED] =
    g_signal_new ("precision-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, precision_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[ALPHA_CHANGED] =
    g_signal_new ("alpha-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, alpha_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[FLOATING_SELECTION_CHANGED] =
    g_signal_new ("floating-selection-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, floating_selection_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[ACTIVE_LAYER_CHANGED] =
    g_signal_new ("active-layer-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, active_layer_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[ACTIVE_CHANNEL_CHANGED] =
    g_signal_new ("active-channel-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, active_channel_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[ACTIVE_VECTORS_CHANGED] =
    g_signal_new ("active-vectors-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, active_vectors_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[COMPONENT_VISIBILITY_CHANGED] =
    g_signal_new ("component-visibility-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, component_visibility_changed),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_CHANNEL_TYPE);

  picman_image_signals[COMPONENT_ACTIVE_CHANGED] =
    g_signal_new ("component-active-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, component_active_changed),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_CHANNEL_TYPE);

  picman_image_signals[MASK_CHANGED] =
    g_signal_new ("mask-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, mask_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[RESOLUTION_CHANGED] =
    g_signal_new ("resolution-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, resolution_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[SIZE_CHANGED_DETAILED] =
    g_signal_new ("size-changed-detailed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, size_changed_detailed),
                  NULL, NULL,
                  picman_marshal_VOID__INT_INT_INT_INT,
                  G_TYPE_NONE, 4,
                  G_TYPE_INT,
                  G_TYPE_INT,
                  G_TYPE_INT,
                  G_TYPE_INT);

  picman_image_signals[UNIT_CHANGED] =
    g_signal_new ("unit-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, unit_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[QUICK_MASK_CHANGED] =
    g_signal_new ("quick-mask-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, quick_mask_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[SELECTION_INVALIDATE] =
    g_signal_new ("selection-invalidate",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, selection_invalidate),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_image_signals[CLEAN] =
    g_signal_new ("clean",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, clean),
                  NULL, NULL,
                  picman_marshal_VOID__FLAGS,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_DIRTY_MASK);

  picman_image_signals[DIRTY] =
    g_signal_new ("dirty",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, dirty),
                  NULL, NULL,
                  picman_marshal_VOID__FLAGS,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_DIRTY_MASK);

  picman_image_signals[SAVED] =
    g_signal_new ("saved",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, saved),
                  NULL, NULL,
                  picman_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  picman_image_signals[EXPORTED] =
    g_signal_new ("exported",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, exported),
                  NULL, NULL,
                  picman_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  picman_image_signals[GUIDE_ADDED] =
    g_signal_new ("guide-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, guide_added),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_GUIDE);

  picman_image_signals[GUIDE_REMOVED] =
    g_signal_new ("guide-removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, guide_removed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_GUIDE);

  picman_image_signals[GUIDE_MOVED] =
    g_signal_new ("guide-moved",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, guide_moved),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_GUIDE);

  picman_image_signals[SAMPLE_POINT_ADDED] =
    g_signal_new ("sample-point-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, sample_point_added),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  picman_image_signals[SAMPLE_POINT_REMOVED] =
    g_signal_new ("sample-point-removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, sample_point_removed),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  picman_image_signals[SAMPLE_POINT_MOVED] =
    g_signal_new ("sample-point-moved",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, sample_point_moved),
                  NULL, NULL,
                  picman_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  picman_image_signals[PARASITE_ATTACHED] =
    g_signal_new ("parasite-attached",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, parasite_attached),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  picman_image_signals[PARASITE_DETACHED] =
    g_signal_new ("parasite-detached",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, parasite_detached),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  picman_image_signals[COLORMAP_CHANGED] =
    g_signal_new ("colormap-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, colormap_changed),
                  NULL, NULL,
                  picman_marshal_VOID__INT,
                  G_TYPE_NONE, 1,
                  G_TYPE_INT);

  picman_image_signals[UNDO_EVENT] =
    g_signal_new ("undo-event",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageClass, undo_event),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM_OBJECT,
                  G_TYPE_NONE, 2,
                  PICMAN_TYPE_UNDO_EVENT,
                  PICMAN_TYPE_UNDO);

  object_class->constructed           = picman_image_constructed;
  object_class->set_property          = picman_image_set_property;
  object_class->get_property          = picman_image_get_property;
  object_class->dispose               = picman_image_dispose;
  object_class->finalize              = picman_image_finalize;

  picman_object_class->name_changed     = picman_image_name_changed;
  picman_object_class->get_memsize      = picman_image_get_memsize;

  viewable_class->default_stock_id    = "picman-image";
  viewable_class->get_size            = picman_image_get_size;
  viewable_class->size_changed        = picman_image_size_changed;
  viewable_class->get_preview_size    = picman_image_get_preview_size;
  viewable_class->get_popup_size      = picman_image_get_popup_size;
  viewable_class->get_new_preview     = picman_image_get_new_preview;
  viewable_class->get_description     = picman_image_get_description;

  klass->mode_changed                 = picman_image_real_mode_changed;
  klass->precision_changed            = picman_image_real_precision_changed;
  klass->alpha_changed                = NULL;
  klass->floating_selection_changed   = NULL;
  klass->active_layer_changed         = NULL;
  klass->active_channel_changed       = NULL;
  klass->active_vectors_changed       = NULL;
  klass->component_visibility_changed = NULL;
  klass->component_active_changed     = NULL;
  klass->mask_changed                 = NULL;
  klass->resolution_changed           = NULL;
  klass->size_changed_detailed        = picman_image_real_size_changed_detailed;
  klass->unit_changed                 = NULL;
  klass->quick_mask_changed           = NULL;
  klass->selection_invalidate         = NULL;

  klass->clean                        = NULL;
  klass->dirty                        = NULL;
  klass->saved                        = NULL;
  klass->exported                     = NULL;
  klass->guide_added                  = NULL;
  klass->guide_removed                = NULL;
  klass->guide_moved                  = NULL;
  klass->sample_point_added           = NULL;
  klass->sample_point_removed         = NULL;
  klass->sample_point_moved           = NULL;
  klass->parasite_attached            = NULL;
  klass->parasite_detached            = NULL;
  klass->colormap_changed             = picman_image_real_colormap_changed;
  klass->undo_event                   = NULL;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman", NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_ID,
                                   g_param_spec_int ("id", NULL, NULL,
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_WIDTH,
                                   g_param_spec_int ("width", NULL, NULL,
                                                     1, PICMAN_MAX_IMAGE_SIZE, 1,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_HEIGHT,
                                   g_param_spec_int ("height", NULL, NULL,
                                                     1, PICMAN_MAX_IMAGE_SIZE, 1,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_BASE_TYPE,
                                   g_param_spec_enum ("base-type", NULL, NULL,
                                                      PICMAN_TYPE_IMAGE_BASE_TYPE,
                                                      PICMAN_RGB,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_PRECISION,
                                   g_param_spec_enum ("precision", NULL, NULL,
                                                      PICMAN_TYPE_PRECISION,
                                                      PICMAN_PRECISION_U8,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (PicmanImagePrivate));
}

static void
picman_color_managed_iface_init (PicmanColorManagedInterface *iface)
{
  iface->get_icc_profile = picman_image_get_icc_profile;
}

static void
picman_projectable_iface_init (PicmanProjectableInterface *iface)
{
  iface->flush              = picman_image_projectable_flush;
  iface->get_image          = picman_image_get_image;
  iface->get_format         = picman_image_get_proj_format;
  iface->get_size           = (void (*) (PicmanProjectable*, gint*, gint*)) picman_image_get_size;
  iface->get_graph          = picman_image_get_graph;
  iface->invalidate_preview = (void (*) (PicmanProjectable*)) picman_viewable_invalidate_preview;
}

static void
picman_image_init (PicmanImage *image)
{
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);
  gint              i;

  private->ID                  = 0;

  private->load_proc           = NULL;
  private->save_proc           = NULL;

  private->width               = 0;
  private->height              = 0;
  private->xresolution         = 1.0;
  private->yresolution         = 1.0;
  private->resolution_unit     = PICMAN_UNIT_INCH;
  private->base_type           = PICMAN_RGB;
  private->precision           = PICMAN_PRECISION_U8;

  private->colormap            = NULL;
  private->n_colors            = 0;
  private->palette             = NULL;

  private->dirty               = 1;
  private->dirty_time          = 0;
  private->undo_freeze_count   = 0;

  private->export_dirty        = 1;

  private->instance_count      = 0;
  private->disp_count          = 0;

  private->tattoo_state        = 0;

  private->projection          = picman_projection_new (PICMAN_PROJECTABLE (image));

  private->guides              = NULL;
  private->grid                = NULL;
  private->sample_points       = NULL;

  private->layers              = picman_item_tree_new (image,
                                                     PICMAN_TYPE_DRAWABLE_STACK,
                                                     PICMAN_TYPE_LAYER);
  private->channels            = picman_item_tree_new (image,
                                                     PICMAN_TYPE_DRAWABLE_STACK,
                                                     PICMAN_TYPE_CHANNEL);
  private->vectors             = picman_item_tree_new (image,
                                                     PICMAN_TYPE_ITEM_STACK,
                                                     PICMAN_TYPE_VECTORS);
  private->layer_stack         = NULL;

  g_signal_connect (private->layers, "notify::active-item",
                    G_CALLBACK (picman_image_active_layer_notify),
                    image);
  g_signal_connect (private->channels, "notify::active-item",
                    G_CALLBACK (picman_image_active_channel_notify),
                    image);
  g_signal_connect (private->vectors, "notify::active-item",
                    G_CALLBACK (picman_image_active_vectors_notify),
                    image);

  g_signal_connect_swapped (private->layers->container, "update",
                            G_CALLBACK (picman_image_invalidate),
                            image);

  private->layer_alpha_handler =
    picman_container_add_handler (private->layers->container, "alpha-changed",
                                G_CALLBACK (picman_image_layer_alpha_changed),
                                image);

  g_signal_connect_swapped (private->channels->container, "update",
                            G_CALLBACK (picman_image_invalidate),
                            image);

  private->channel_name_changed_handler =
    picman_container_add_handler (private->channels->container, "name-changed",
                                G_CALLBACK (picman_image_channel_name_changed),
                                image);
  private->channel_color_changed_handler =
    picman_container_add_handler (private->channels->container, "color-changed",
                                G_CALLBACK (picman_image_channel_color_changed),
                                image);

  g_signal_connect (private->channels->container, "add",
                    G_CALLBACK (picman_image_channel_add),
                    image);
  g_signal_connect (private->channels->container, "remove",
                    G_CALLBACK (picman_image_channel_remove),
                    image);

  private->floating_sel        = NULL;
  private->selection_mask      = NULL;

  private->parasites           = picman_parasite_list_new ();

  for (i = 0; i < MAX_CHANNELS; i++)
    {
      private->visible[i] = TRUE;
      private->active[i]  = TRUE;
    }

  private->quick_mask_state    = FALSE;
  private->quick_mask_inverted = FALSE;
  picman_rgba_set (&private->quick_mask_color, 1.0, 0.0, 0.0, 0.5);

  private->undo_stack          = picman_undo_stack_new (image);
  private->redo_stack          = picman_undo_stack_new (image);
  private->group_count         = 0;
  private->pushing_undo_group  = PICMAN_UNDO_GROUP_NONE;

  private->flush_accum.alpha_changed              = FALSE;
  private->flush_accum.mask_changed               = FALSE;
  private->flush_accum.floating_selection_changed = FALSE;
  private->flush_accum.preview_invalidated        = FALSE;
}

static void
picman_image_constructed (GObject *object)
{
  PicmanImage        *image   = PICMAN_IMAGE (object);
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);
  PicmanChannel      *selection;
  PicmanCoreConfig   *config;
  PicmanTemplate     *template;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (image->picman));

  config = image->picman->config;

  private->ID = picman_id_table_insert (image->picman->image_table, image);

  template = config->default_image;

  private->xresolution     = picman_template_get_resolution_x (template);
  private->yresolution     = picman_template_get_resolution_y (template);
  private->resolution_unit = picman_template_get_resolution_unit (template);

  private->grid = picman_config_duplicate (PICMAN_CONFIG (config->default_grid));

  private->quick_mask_color = config->quick_mask_color;

  if (private->base_type == PICMAN_INDEXED)
    picman_image_colormap_init (image);

  selection = picman_selection_new (image,
                                  picman_image_get_width  (image),
                                  picman_image_get_height (image));
  picman_image_take_mask (image, selection);

  g_signal_connect_object (config, "notify::transparency-type",
                           G_CALLBACK (picman_item_stack_invalidate_previews),
                           private->layers->container, G_CONNECT_SWAPPED);
  g_signal_connect_object (config, "notify::transparency-size",
                           G_CALLBACK (picman_item_stack_invalidate_previews),
                           private->layers->container, G_CONNECT_SWAPPED);
  g_signal_connect_object (config, "notify::layer-previews",
                           G_CALLBACK (picman_viewable_size_changed),
                           image, G_CONNECT_SWAPPED);

  picman_container_add (image->picman->images, PICMAN_OBJECT (image));
}

static void
picman_image_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  PicmanImage        *image   = PICMAN_IMAGE (object);
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);

  switch (property_id)
    {
    case PROP_PICMAN:
      image->picman = g_value_get_object (value);
      break;
    case PROP_ID:
      g_assert_not_reached ();
      break;
    case PROP_WIDTH:
      private->width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      private->height = g_value_get_int (value);
      break;
    case PROP_BASE_TYPE:
      private->base_type = g_value_get_enum (value);
      break;
    case PROP_PRECISION:
      private->precision = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_image_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  PicmanImage        *image   = PICMAN_IMAGE (object);
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, image->picman);
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
    case PROP_BASE_TYPE:
      g_value_set_enum (value, private->base_type);
      break;
    case PROP_PRECISION:
      g_value_set_enum (value, private->precision);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_image_dispose (GObject *object)
{
  PicmanImage        *image   = PICMAN_IMAGE (object);
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (private->colormap)
    picman_image_colormap_dispose (image);

  picman_image_undo_free (image);

  g_signal_handlers_disconnect_by_func (private->layers->container,
                                        picman_image_invalidate,
                                        image);

  picman_container_remove_handler (private->layers->container,
                                 private->layer_alpha_handler);

  g_signal_handlers_disconnect_by_func (private->channels->container,
                                        picman_image_invalidate,
                                        image);

  picman_container_remove_handler (private->channels->container,
                                 private->channel_name_changed_handler);
  picman_container_remove_handler (private->channels->container,
                                 private->channel_color_changed_handler);

  g_signal_handlers_disconnect_by_func (private->channels->container,
                                        picman_image_channel_add,
                                        image);
  g_signal_handlers_disconnect_by_func (private->channels->container,
                                        picman_image_channel_remove,
                                        image);

  picman_container_foreach (private->layers->container,
                          (GFunc) picman_item_removed, NULL);
  picman_container_foreach (private->channels->container,
                          (GFunc) picman_item_removed, NULL);
  picman_container_foreach (private->vectors->container,
                          (GFunc) picman_item_removed, NULL);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_image_finalize (GObject *object)
{
  PicmanImage        *image   = PICMAN_IMAGE (object);
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (private->projection)
    {
      g_object_unref (private->projection);
      private->projection = NULL;
    }

  if (private->graph)
    {
      g_object_unref (private->graph);
      private->graph = NULL;
      private->visible_mask = NULL;
    }

  if (private->colormap)
    picman_image_colormap_free (image);

  if (private->layers)
    {
      g_object_unref (private->layers);
      private->layers = NULL;
    }
  if (private->channels)
    {
      g_object_unref (private->channels);
      private->channels = NULL;
    }
  if (private->vectors)
    {
      g_object_unref (private->vectors);
      private->vectors = NULL;
    }
  if (private->layer_stack)
    {
      g_slist_free (private->layer_stack);
      private->layer_stack = NULL;
    }

  if (private->selection_mask)
    {
      g_object_unref (private->selection_mask);
      private->selection_mask = NULL;
    }

  if (private->parasites)
    {
      g_object_unref (private->parasites);
      private->parasites = NULL;
    }

  if (private->guides)
    {
      g_list_free_full (private->guides, (GDestroyNotify) g_object_unref);
      private->guides = NULL;
    }

  if (private->grid)
    {
      g_object_unref (private->grid);
      private->grid = NULL;
    }

  if (private->sample_points)
    {
      g_list_free_full (private->sample_points,
                        (GDestroyNotify) picman_sample_point_unref);
      private->sample_points = NULL;
    }

  if (private->undo_stack)
    {
      g_object_unref (private->undo_stack);
      private->undo_stack = NULL;
    }
  if (private->redo_stack)
    {
      g_object_unref (private->redo_stack);
      private->redo_stack = NULL;
    }

  if (image->picman && image->picman->image_table)
    {
      picman_id_table_remove (image->picman->image_table, private->ID);
      image->picman = NULL;
    }

  if (private->display_name)
    {
      g_free (private->display_name);
      private->display_name = NULL;
    }

  if (private->display_path)
    {
      g_free (private->display_path);
      private->display_path = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_image_name_changed (PicmanObject *object)
{
  PicmanImage        *image   = PICMAN_IMAGE (object);
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);
  const gchar      *name;

  if (PICMAN_OBJECT_CLASS (parent_class)->name_changed)
    PICMAN_OBJECT_CLASS (parent_class)->name_changed (object);

  if (private->display_name)
    {
      g_free (private->display_name);
      private->display_name = NULL;
    }

  if (private->display_path)
    {
      g_free (private->display_path);
      private->display_path = NULL;
    }

  /* We never want the empty string as a name, so change empty strings
   * to NULL strings (without emitting the "name-changed" signal
   * again)
   */
  name = picman_object_get_name (object);
  if (name && strlen (name) == 0)
    picman_object_name_free (object);
}

static gint64
picman_image_get_memsize (PicmanObject *object,
                        gint64     *gui_size)
{
  PicmanImage        *image   = PICMAN_IMAGE (object);
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);
  gint64            memsize = 0;

  if (picman_image_get_colormap (image))
    memsize += PICMAN_IMAGE_COLORMAP_SIZE;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->palette),
                                      gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->projection),
                                      gui_size);

  memsize += picman_g_list_get_memsize (picman_image_get_guides (image),
                                      sizeof (PicmanGuide));

  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->grid), gui_size);

  memsize += picman_g_list_get_memsize (picman_image_get_sample_points (image),
                                      sizeof (PicmanSamplePoint));

  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->layers),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->channels),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->vectors),
                                      gui_size);

  memsize += picman_g_slist_get_memsize (private->layer_stack, 0);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->selection_mask),
                                      gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->parasites),
                                      gui_size);

  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->undo_stack),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->redo_stack),
                                      gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_image_get_size (PicmanViewable *viewable,
                     gint         *width,
                     gint         *height)
{
  PicmanImage *image = PICMAN_IMAGE (viewable);

  *width  = picman_image_get_width  (image);
  *height = picman_image_get_height (image);

  return TRUE;
}

static void
picman_image_size_changed (PicmanViewable *viewable)
{
  PicmanImage *image = PICMAN_IMAGE (viewable);
  GList     *all_items;
  GList     *list;

  if (PICMAN_VIEWABLE_CLASS (parent_class)->size_changed)
    PICMAN_VIEWABLE_CLASS (parent_class)->size_changed (viewable);

  all_items = picman_image_get_layer_list (image);
  for (list = all_items; list; list = g_list_next (list))
    {
      PicmanLayerMask *mask = picman_layer_get_mask (PICMAN_LAYER (list->data));

      picman_viewable_size_changed (PICMAN_VIEWABLE (list->data));

      if (mask)
        picman_viewable_size_changed (PICMAN_VIEWABLE (mask));
    }
  g_list_free (all_items);

  all_items = picman_image_get_channel_list (image);
  g_list_free_full (all_items, (GDestroyNotify) picman_viewable_size_changed);

  all_items = picman_image_get_vectors_list (image);
  g_list_free_full (all_items, (GDestroyNotify) picman_viewable_size_changed);

  picman_viewable_size_changed (PICMAN_VIEWABLE (picman_image_get_mask (image)));

  picman_projectable_structure_changed (PICMAN_PROJECTABLE (image));
}

static gchar *
picman_image_get_description (PicmanViewable  *viewable,
                            gchar        **tooltip)
{
  PicmanImage *image = PICMAN_IMAGE (viewable);

  if (tooltip)
    *tooltip = g_strdup (picman_image_get_display_path (image));

  return g_strdup_printf ("%s-%d",
			  picman_image_get_display_name (image),
			  picman_image_get_ID (image));
}

static void
picman_image_real_mode_changed (PicmanImage *image)
{
  picman_projectable_structure_changed (PICMAN_PROJECTABLE (image));
}

static void
picman_image_real_precision_changed (PicmanImage *image)
{
  picman_projectable_structure_changed (PICMAN_PROJECTABLE (image));
}

static void
picman_image_real_size_changed_detailed (PicmanImage *image,
                                       gint       previous_origin_x,
                                       gint       previous_origin_y,
                                       gint       previous_width,
                                       gint       previous_height)
{
  /* Whenever PicmanImage::size-changed-detailed is emitted, so is
   * PicmanViewable::size-changed. Clients choose what signal to listen
   * to depending on how much info they need.
   */
  picman_viewable_size_changed (PICMAN_VIEWABLE (image));
}

static void
picman_image_real_colormap_changed (PicmanImage *image,
                                  gint       color_index)
{
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (private->colormap)
    {
      babl_palette_set_palette (private->babl_palette_rgb,
                                picman_babl_format (PICMAN_RGB,
                                                  private->precision, FALSE),
                                private->colormap,
                                private->n_colors);
      babl_palette_set_palette (private->babl_palette_rgba,
                                picman_babl_format (PICMAN_RGB,
                                                  private->precision, FALSE),
                                private->colormap,
                                private->n_colors);
    }

  if (picman_image_get_base_type (image) == PICMAN_INDEXED)
    {
      /* A colormap alteration affects the whole image */
      picman_image_invalidate (image,
                             0, 0,
                             picman_image_get_width  (image),
                             picman_image_get_height (image));

      picman_item_stack_invalidate_previews (PICMAN_ITEM_STACK (private->layers->container));
    }
}

static const guint8 *
picman_image_get_icc_profile (PicmanColorManaged *managed,
                            gsize            *len)
{
  const PicmanParasite *parasite;

  parasite = picman_image_parasite_find (PICMAN_IMAGE (managed), "icc-profile");

  if (parasite)
    {
      gsize data_size = picman_parasite_data_size (parasite);

      if (data_size > 0)
        {
          *len = data_size;

          return picman_parasite_data (parasite);
        }
    }

  return NULL;
}

static void
picman_image_projectable_flush (PicmanProjectable *projectable,
                              gboolean         invalidate_preview)
{
  PicmanImage        *image   = PICMAN_IMAGE (projectable);
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (private->flush_accum.alpha_changed)
    {
      picman_image_alpha_changed (image);
      private->flush_accum.alpha_changed = FALSE;
    }

  if (private->flush_accum.mask_changed)
    {
      picman_image_mask_changed (image);
      private->flush_accum.mask_changed = FALSE;
    }

  if (private->flush_accum.floating_selection_changed)
    {
      picman_image_floating_selection_changed (image);
      private->flush_accum.floating_selection_changed = FALSE;
    }

  if (private->flush_accum.preview_invalidated)
    {
      /*  don't invalidate the preview here, the projection does this when
       *  it is completely constructed.
       */
      private->flush_accum.preview_invalidated = FALSE;
    }
}

static PicmanImage *
picman_image_get_image (PicmanProjectable *projectable)
{
  return PICMAN_IMAGE (projectable);
}

static const Babl *
picman_image_get_proj_format (PicmanProjectable *projectable)
{
  PicmanImage        *image   = PICMAN_IMAGE (projectable);
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);

  switch (private->base_type)
    {
    case PICMAN_RGB:
    case PICMAN_INDEXED:
      return picman_image_get_format (image, PICMAN_RGB,
                                    picman_image_get_precision (image), TRUE);

    case PICMAN_GRAY:
      return picman_image_get_format (image, PICMAN_GRAY,
                                    picman_image_get_precision (image), TRUE);
    }

  g_assert_not_reached ();

  return NULL;
}

static GeglNode *
picman_image_get_graph (PicmanProjectable *projectable)
{
  PicmanImage         *image   = PICMAN_IMAGE (projectable);
  PicmanImagePrivate  *private = PICMAN_IMAGE_GET_PRIVATE (image);
  GeglNode          *layers_node;
  GeglNode          *channels_node;
  GeglNode          *output;
  PicmanComponentMask  mask;

  if (private->graph)
    return private->graph;

  private->graph = gegl_node_new ();

  layers_node =
    picman_filter_stack_get_graph (PICMAN_FILTER_STACK (private->layers->container));

  gegl_node_add_child (private->graph, layers_node);

  channels_node =
    picman_filter_stack_get_graph (PICMAN_FILTER_STACK (private->channels->container));

  gegl_node_add_child (private->graph, channels_node);

  gegl_node_connect_to (layers_node,   "output",
                        channels_node, "input");

  mask = ~picman_image_get_visible_mask (image) & PICMAN_COMPONENT_ALL;

  private->visible_mask =
    gegl_node_new_child (private->graph,
                         "operation", "picman:mask-components",
                         "mask",      mask,
                         NULL);

  gegl_node_connect_to (channels_node,         "output",
                        private->visible_mask, "input");

  output = gegl_node_get_output_proxy (private->graph, "output");

  gegl_node_connect_to (private->visible_mask, "output",
                        output,                "input");

  return private->graph;
}

static void
picman_image_mask_update (PicmanDrawable *drawable,
                        gint          x,
                        gint          y,
                        gint          width,
                        gint          height,
                        PicmanImage    *image)
{
  PICMAN_IMAGE_GET_PRIVATE (image)->flush_accum.mask_changed = TRUE;
}

static void
picman_image_layer_alpha_changed (PicmanDrawable *drawable,
                                PicmanImage    *image)
{
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (picman_container_get_n_children (private->layers->container) == 1)
    private->flush_accum.alpha_changed = TRUE;
}

static void
picman_image_channel_add (PicmanContainer *container,
                        PicmanChannel   *channel,
                        PicmanImage     *image)
{
  if (! strcmp (PICMAN_IMAGE_QUICK_MASK_NAME,
                picman_object_get_name (channel)))
    {
      picman_image_set_quick_mask_state (image, TRUE);
    }
}

static void
picman_image_channel_remove (PicmanContainer *container,
                           PicmanChannel   *channel,
                           PicmanImage     *image)
{
  if (! strcmp (PICMAN_IMAGE_QUICK_MASK_NAME,
                picman_object_get_name (channel)))
    {
      picman_image_set_quick_mask_state (image, FALSE);
    }
}

static void
picman_image_channel_name_changed (PicmanChannel *channel,
                                 PicmanImage   *image)
{
  if (! strcmp (PICMAN_IMAGE_QUICK_MASK_NAME,
                picman_object_get_name (channel)))
    {
      picman_image_set_quick_mask_state (image, TRUE);
    }
  else if (picman_image_get_quick_mask_state (image) &&
           ! picman_image_get_quick_mask (image))
    {
      picman_image_set_quick_mask_state (image, FALSE);
    }
}

static void
picman_image_channel_color_changed (PicmanChannel *channel,
                                  PicmanImage   *image)
{
  if (! strcmp (PICMAN_IMAGE_QUICK_MASK_NAME,
                picman_object_get_name (channel)))
    {
      PICMAN_IMAGE_GET_PRIVATE (image)->quick_mask_color = channel->color;
    }
}

static void
picman_image_active_layer_notify (PicmanItemTree     *tree,
                                const GParamSpec *pspec,
                                PicmanImage        *image)
{
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);
  PicmanLayer        *layer   = picman_image_get_active_layer (image);

  if (layer)
    {
      /*  Configure the layer stack to reflect this change  */
      private->layer_stack = g_slist_remove (private->layer_stack, layer);
      private->layer_stack = g_slist_prepend (private->layer_stack, layer);
    }

  g_signal_emit (image, picman_image_signals[ACTIVE_LAYER_CHANGED], 0);

  if (layer && picman_image_get_active_channel (image))
    picman_image_set_active_channel (image, NULL);
}

static void
picman_image_active_channel_notify (PicmanItemTree     *tree,
                                  const GParamSpec *pspec,
                                  PicmanImage        *image)
{
  PicmanChannel *channel = picman_image_get_active_channel (image);

  g_signal_emit (image, picman_image_signals[ACTIVE_CHANNEL_CHANGED], 0);

  if (channel && picman_image_get_active_layer (image))
    picman_image_set_active_layer (image, NULL);
}

static void
picman_image_active_vectors_notify (PicmanItemTree     *tree,
                                  const GParamSpec *pspec,
                                  PicmanImage        *image)
{
  g_signal_emit (image, picman_image_signals[ACTIVE_VECTORS_CHANGED], 0);
}


/*  public functions  */

PicmanImage *
picman_image_new (Picman              *picman,
                gint               width,
                gint               height,
                PicmanImageBaseType  base_type,
                PicmanPrecision      precision)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (base_type != PICMAN_INDEXED ||
                        precision == PICMAN_PRECISION_U8, NULL);

  return g_object_new (PICMAN_TYPE_IMAGE,
                       "picman",      picman,
                       "width",     width,
                       "height",    height,
                       "base-type", base_type,
                       "precision", precision,
                       NULL);
}

PicmanImageBaseType
picman_image_get_base_type (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), -1);

  return PICMAN_IMAGE_GET_PRIVATE (image)->base_type;
}

PicmanPrecision
picman_image_get_precision (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), -1);

  return PICMAN_IMAGE_GET_PRIVATE (image)->precision;
}

const Babl *
picman_image_get_format (const PicmanImage   *image,
                       PicmanImageBaseType  base_type,
                       PicmanPrecision      precision,
                       gboolean           with_alpha)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  switch (base_type)
    {
    case PICMAN_RGB:
    case PICMAN_GRAY:
      return picman_babl_format (base_type, precision, with_alpha);

    case PICMAN_INDEXED:
      if (precision == PICMAN_PRECISION_U8)
        {
          if (with_alpha)
            return picman_image_colormap_get_rgba_format (image);
          else
            return picman_image_colormap_get_rgb_format (image);
        }
    }

  g_return_val_if_reached (NULL);
}

const Babl *
picman_image_get_layer_format (const PicmanImage *image,
                             gboolean         with_alpha)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_image_get_format (image,
                                picman_image_get_base_type (image),
                                picman_image_get_precision (image),
                                with_alpha);
}

const Babl *
picman_image_get_channel_format (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_image_get_format (image, PICMAN_GRAY,
                                picman_image_get_precision (image),
                                FALSE);
}

const Babl *
picman_image_get_mask_format (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_babl_mask_format (picman_image_get_precision (image));
}

gint
picman_image_get_ID (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), -1);

  return PICMAN_IMAGE_GET_PRIVATE (image)->ID;
}

PicmanImage *
picman_image_get_by_ID (Picman *picman,
                      gint  image_id)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (picman->image_table == NULL)
    return NULL;

  return (PicmanImage *) picman_id_table_lookup (picman->image_table, image_id);
}

void
picman_image_set_uri (PicmanImage   *image,
                    const gchar *uri)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  picman_object_set_name (PICMAN_OBJECT (image), uri);
}

static void
picman_image_take_uri (PicmanImage *image,
                     gchar     *uri)
{
  picman_object_take_name (PICMAN_OBJECT (image), uri);
}

/**
 * picman_image_get_untitled_string:
 *
 * Returns: The (translated) "Untitled" string for newly created
 * images.
 **/
const gchar *
picman_image_get_string_untitled (void)
{
  return _("Untitled");
}

/**
 * picman_image_get_uri_or_untitled:
 * @image: A #PicmanImage.
 *
 * Get the URI of the XCF image, or "Untitled" if there is no URI.
 *
 * Returns: The URI, or "Untitled".
 **/
const gchar *
picman_image_get_uri_or_untitled (const PicmanImage *image)
{
  const gchar *uri;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  uri = picman_image_get_uri (image);

  return uri ? uri : picman_image_get_string_untitled ();
}

/**
 * picman_image_get_uri:
 * @image: A #PicmanImage.
 *
 * Get the URI of the XCF image, or NULL if there is no URI.
 *
 * Returns: The URI, or NULL.
 **/
const gchar *
picman_image_get_uri (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_object_get_name (image);
}

void
picman_image_set_filename (PicmanImage   *image,
                         const gchar *filename)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  if (filename && strlen (filename))
    {
      picman_image_take_uri (image,
                           file_utils_filename_to_uri (image->picman,
                                                       filename,
                                                       NULL));
    }
  else
    {
      picman_image_set_uri (image, NULL);
    }
}

/**
 * picman_image_get_imported_uri:
 * @image: A #PicmanImage.
 *
 * Returns: The URI of the imported image, or NULL if the image has
 * been saved as XCF after it was imported.
 **/
const gchar *
picman_image_get_imported_uri (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return g_object_get_data (G_OBJECT (image),
                            PICMAN_FILE_IMPORT_SOURCE_URI_KEY);
}

/**
 * picman_image_get_exported_uri:
 * @image: A #PicmanImage.
 *
 * Returns: The URI of the image last exported from this XCF file, or
 * NULL if the image has never been exported.
 **/
const gchar *
picman_image_get_exported_uri (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return g_object_get_data (G_OBJECT (image),
                            PICMAN_FILE_EXPORT_URI_KEY);
}

/**
 * picman_image_get_save_a_copy_uri:
 * @image: A #PicmanImage.
 *
 * Returns: The URI of the last copy that was saved of this XCF file.
 **/
const gchar *
picman_image_get_save_a_copy_uri (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return g_object_get_data (G_OBJECT (image),
                            PICMAN_FILE_SAVE_A_COPY_URI_KEY);
}

/**
 * picman_image_get_any_uri:
 * @image: A #PicmanImage.
 *
 * Returns: The XCF file URI, the imported file URI, or the exported
 * file URI, in that order of precedence.
 **/
const gchar *
picman_image_get_any_uri (const PicmanImage *image)
{
  const gchar *uri;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  uri = picman_image_get_uri (image);
  if (! uri)
    {
      uri = picman_image_get_imported_uri (image);
      if (! uri)
        {
          uri = picman_image_get_exported_uri (image);
        }
    }

  return uri;
}

/**
 * picman_image_set_imported_uri:
 * @image: A #PicmanImage.
 * @uri:
 *
 * Sets the URI this file was imported from.
 **/
void
picman_image_set_imported_uri (PicmanImage   *image,
                             const gchar *uri)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  if (picman_image_get_imported_uri (image) == uri)
    return;

  g_object_set_data_full (G_OBJECT (image), PICMAN_FILE_IMPORT_SOURCE_URI_KEY,
                          g_strdup (uri), (GDestroyNotify) g_free);

  picman_object_name_changed (PICMAN_OBJECT (image));
}

/**
 * picman_image_set_exported_uri:
 * @image: A #PicmanImage.
 * @uri:
 *
 * Sets the URI this file was last exported to. Note that saving as
 * XCF is not "exporting".
 **/
void
picman_image_set_exported_uri (PicmanImage   *image,
                             const gchar *uri)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  if (picman_image_get_exported_uri (image) == uri)
    return;

  g_object_set_data_full (G_OBJECT (image),
                          PICMAN_FILE_EXPORT_URI_KEY,
                          g_strdup (uri), (GDestroyNotify) g_free);

  picman_object_name_changed (PICMAN_OBJECT (image));
}

/**
 * picman_image_set_save_a_copy_uri:
 * @image: A #PicmanImage.
 * @uri:
 *
 * Set the URI to the last copy this XCF file was saved to through the
 * "save a copy" action.
 **/
void
picman_image_set_save_a_copy_uri (PicmanImage   *image,
                                const gchar *uri)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  if (picman_image_get_save_a_copy_uri (image) == uri)
    return;

  g_object_set_data_full (G_OBJECT (image),
                          PICMAN_FILE_SAVE_A_COPY_URI_KEY,
                          g_strdup (uri), (GDestroyNotify) g_free);
}

gchar *
picman_image_get_filename (const PicmanImage *image)
{
  const gchar *uri;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  uri = picman_image_get_uri (image);

  if (! uri)
    return NULL;

  return g_filename_from_uri (uri, NULL, NULL);
}

static gchar *
picman_image_format_display_uri (PicmanImage *image,
                               gboolean   basename)
{
  const gchar *uri_format    = NULL;
  const gchar *export_status = NULL;
  const gchar *uri;
  const gchar *source;
  const gchar *dest;
  gboolean     is_imported;
  gboolean     is_exported;
  gchar       *display_uri   = NULL;
  gchar       *format_string;
  gchar       *tmp;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  uri    = picman_image_get_uri (image);
  source = picman_image_get_imported_uri (image);
  dest   = picman_image_get_exported_uri (image);

  is_imported = (source != NULL);
  is_exported = (dest   != NULL);

  if (uri)
    {
      display_uri = g_strdup (uri);
      uri_format  = "%s";
    }
  else
    {
      if (is_imported)
        display_uri = g_strdup (source);

      /* Calculate filename suffix */
      if (! picman_image_is_export_dirty (image))
        {
          if (is_exported)
            {
              display_uri   = g_strdup (dest);
              export_status = _(" (exported)");
            }
          else if (is_imported)
            {
              export_status = _(" (overwritten)");
            }
          else
            {
              g_warning ("Unexpected code path, Save+export implementation is buggy!");
            }
        }
      else if (is_imported)
        {
          export_status = _(" (imported)");
        }

      if (display_uri)
        {
          gchar *tmp = file_utils_uri_with_new_ext (display_uri, NULL);
          g_free (display_uri);
          display_uri = tmp;
        }

      uri_format = "[%s]";
    }

  if (! display_uri)
    {
      display_uri = g_strdup (picman_image_get_string_untitled ());
    }
  else if (basename)
    {
      tmp = file_utils_uri_display_basename (display_uri);
      g_free (display_uri);
      display_uri = tmp;
    }
  else
    {
      tmp = file_utils_uri_display_name (display_uri);
      g_free (display_uri);
      display_uri = tmp;
    }

  format_string = g_strconcat (uri_format, export_status, NULL);

  tmp = g_strdup_printf (format_string, display_uri);
  g_free (display_uri);
  display_uri = tmp;

  g_free (format_string);

  return display_uri;
}

const gchar *
picman_image_get_display_name (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (! private->display_name)
    private->display_name = picman_image_format_display_uri (image, TRUE);

  return private->display_name;
}

const gchar *
picman_image_get_display_path (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (! private->display_path)
    private->display_path = picman_image_format_display_uri (image, FALSE);

  return private->display_path;
}

void
picman_image_set_load_proc (PicmanImage           *image,
                          PicmanPlugInProcedure *proc)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  PICMAN_IMAGE_GET_PRIVATE (image)->load_proc = proc;
}

PicmanPlugInProcedure *
picman_image_get_load_proc (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->load_proc;
}

void
picman_image_set_save_proc (PicmanImage           *image,
                          PicmanPlugInProcedure *proc)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  PICMAN_IMAGE_GET_PRIVATE (image)->save_proc = proc;
}

PicmanPlugInProcedure *
picman_image_get_save_proc (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->save_proc;
}

void
picman_image_set_resolution (PicmanImage *image,
                           gdouble    xresolution,
                           gdouble    yresolution)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  /* don't allow to set the resolution out of bounds */
  if (xresolution < PICMAN_MIN_RESOLUTION || xresolution > PICMAN_MAX_RESOLUTION ||
      yresolution < PICMAN_MIN_RESOLUTION || yresolution > PICMAN_MAX_RESOLUTION)
    return;

  if ((ABS (private->xresolution - xresolution) >= 1e-5) ||
      (ABS (private->yresolution - yresolution) >= 1e-5))
    {
      picman_image_undo_push_image_resolution (image,
                                             C_("undo-type", "Change Image Resolution"));

      private->xresolution = xresolution;
      private->yresolution = yresolution;

      picman_image_resolution_changed (image);
      picman_image_size_changed_detailed (image,
                                        0,
                                        0,
                                        picman_image_get_width (image),
                                        picman_image_get_height (image));
    }
}

void
picman_image_get_resolution (const PicmanImage *image,
                           gdouble         *xresolution,
                           gdouble         *yresolution)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (xresolution != NULL && yresolution != NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  *xresolution = private->xresolution;
  *yresolution = private->yresolution;
}

void
picman_image_resolution_changed (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[RESOLUTION_CHANGED], 0);
}

void
picman_image_set_unit (PicmanImage *image,
                     PicmanUnit   unit)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (unit > PICMAN_UNIT_PIXEL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (private->resolution_unit != unit)
    {
      picman_image_undo_push_image_resolution (image,
                                             C_("undo-type", "Change Image Unit"));

      private->resolution_unit = unit;
      picman_image_unit_changed (image);
    }
}

PicmanUnit
picman_image_get_unit (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), PICMAN_UNIT_INCH);

  return PICMAN_IMAGE_GET_PRIVATE (image)->resolution_unit;
}

void
picman_image_unit_changed (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[UNIT_CHANGED], 0);
}

gint
picman_image_get_width (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  return PICMAN_IMAGE_GET_PRIVATE (image)->width;
}

gint
picman_image_get_height (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  return PICMAN_IMAGE_GET_PRIVATE (image)->height;
}

gboolean
picman_image_has_alpha (const PicmanImage *image)
{
  PicmanImagePrivate *private;
  PicmanLayer        *layer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), TRUE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  layer = PICMAN_LAYER (picman_container_get_first_child (private->layers->container));

  return ((picman_image_get_n_layers (image) > 1) ||
          (layer && picman_drawable_has_alpha (PICMAN_DRAWABLE (layer))));
}

gboolean
picman_image_is_empty (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), TRUE);

  return picman_container_is_empty (PICMAN_IMAGE_GET_PRIVATE (image)->layers->container);
}

void
picman_image_set_floating_selection (PicmanImage *image,
                                   PicmanLayer *floating_sel)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (floating_sel == NULL || PICMAN_IS_LAYER (floating_sel));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (private->floating_sel != floating_sel)
    {
      private->floating_sel = floating_sel;

      private->flush_accum.floating_selection_changed = TRUE;
    }
}

PicmanLayer *
picman_image_get_floating_selection (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->floating_sel;
}

void
picman_image_floating_selection_changed (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[FLOATING_SELECTION_CHANGED], 0);
}

PicmanChannel *
picman_image_get_mask (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->selection_mask;
}

void
picman_image_mask_changed (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[MASK_CHANGED], 0);
}

void
picman_image_take_mask (PicmanImage   *image,
                      PicmanChannel *mask)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_SELECTION (mask));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (private->selection_mask)
    g_object_unref (private->selection_mask);

  private->selection_mask = g_object_ref_sink (mask);

  g_signal_connect (private->selection_mask, "update",
                    G_CALLBACK (picman_image_mask_update),
                    image);
}


/*  image components  */

const Babl *
picman_image_get_component_format (const PicmanImage *image,
                                 PicmanChannelType  channel)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  switch (channel)
    {
    case PICMAN_RED_CHANNEL:
      return picman_babl_component_format (PICMAN_RGB,
                                         picman_image_get_precision (image),
                                         RED);

    case PICMAN_GREEN_CHANNEL:
      return picman_babl_component_format (PICMAN_RGB,
                                         picman_image_get_precision (image),
                                         GREEN);

    case PICMAN_BLUE_CHANNEL:
      return picman_babl_component_format (PICMAN_RGB,
                                         picman_image_get_precision (image),
                                         BLUE);

    case PICMAN_ALPHA_CHANNEL:
      return picman_babl_component_format (PICMAN_RGB,
                                         picman_image_get_precision (image),
                                         ALPHA);

    case PICMAN_GRAY_CHANNEL:
      return picman_babl_component_format (PICMAN_GRAY,
                                         picman_image_get_precision (image),
                                         GRAY);

    case PICMAN_INDEXED_CHANNEL:
      return babl_format ("Y u8"); /* will extract grayscale, the best
                                    * we can do here */
    }

  return NULL;
}

gint
picman_image_get_component_index (const PicmanImage *image,
                                PicmanChannelType  channel)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), -1);

  switch (channel)
    {
    case PICMAN_RED_CHANNEL:     return RED;
    case PICMAN_GREEN_CHANNEL:   return GREEN;
    case PICMAN_BLUE_CHANNEL:    return BLUE;
    case PICMAN_GRAY_CHANNEL:    return GRAY;
    case PICMAN_INDEXED_CHANNEL: return INDEXED;
    case PICMAN_ALPHA_CHANNEL:
      switch (picman_image_get_base_type (image))
        {
        case PICMAN_RGB:     return ALPHA;
        case PICMAN_GRAY:    return ALPHA_G;
        case PICMAN_INDEXED: return ALPHA_I;
        }
    }

  return -1;
}

void
picman_image_set_component_active (PicmanImage       *image,
                                 PicmanChannelType  channel,
                                 gboolean         active)
{
  PicmanImagePrivate *private;
  gint              index = -1;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  index = picman_image_get_component_index (image, channel);

  if (index != -1 && active != private->active[index])
    {
      private->active[index] = active ? TRUE : FALSE;

      /*  If there is an active channel and we mess with the components,
       *  the active channel gets unset...
       */
      picman_image_unset_active_channel (image);

      g_signal_emit (image,
                     picman_image_signals[COMPONENT_ACTIVE_CHANGED], 0,
                     channel);
    }
}

gboolean
picman_image_get_component_active (const PicmanImage *image,
                                 PicmanChannelType  channel)
{
  gint index = -1;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  index = picman_image_get_component_index (image, channel);

  if (index != -1)
    return PICMAN_IMAGE_GET_PRIVATE (image)->active[index];

  return FALSE;
}

void
picman_image_get_active_array (const PicmanImage *image,
                             gboolean        *components)
{
  PicmanImagePrivate *private;
  gint              i;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (components != NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  for (i = 0; i < MAX_CHANNELS; i++)
    components[i] = private->active[i];
}

PicmanComponentMask
picman_image_get_active_mask (const PicmanImage *image)
{
  PicmanImagePrivate  *private;
  PicmanComponentMask  mask = 0;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  switch (picman_image_get_base_type (image))
    {
    case PICMAN_RGB:
      mask |= (private->active[RED])   ? PICMAN_COMPONENT_RED   : 0;
      mask |= (private->active[GREEN]) ? PICMAN_COMPONENT_GREEN : 0;
      mask |= (private->active[BLUE])  ? PICMAN_COMPONENT_BLUE  : 0;
      mask |= (private->active[ALPHA]) ? PICMAN_COMPONENT_ALPHA : 0;
      break;

    case PICMAN_GRAY:
    case PICMAN_INDEXED:
      mask |= (private->active[GRAY])    ? PICMAN_COMPONENT_RED   : 0;
      mask |= (private->active[GRAY])    ? PICMAN_COMPONENT_GREEN : 0;
      mask |= (private->active[GRAY])    ? PICMAN_COMPONENT_BLUE  : 0;
      mask |= (private->active[ALPHA_G]) ? PICMAN_COMPONENT_ALPHA : 0;
      break;
    }

  return mask;
}

void
picman_image_set_component_visible (PicmanImage       *image,
                                  PicmanChannelType  channel,
                                  gboolean         visible)
{
  PicmanImagePrivate *private;
  gint              index = -1;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  index = picman_image_get_component_index (image, channel);

  if (index != -1 && visible != private->visible[index])
    {
      private->visible[index] = visible ? TRUE : FALSE;

      if (private->visible_mask)
        {
          PicmanComponentMask mask;

          mask = ~picman_image_get_visible_mask (image) & PICMAN_COMPONENT_ALL;

          gegl_node_set (private->visible_mask,
                         "mask", mask,
                         NULL);
        }

      g_signal_emit (image,
                     picman_image_signals[COMPONENT_VISIBILITY_CHANGED], 0,
                     channel);

      picman_image_invalidate (image,
                             0, 0,
                             picman_image_get_width  (image),
                             picman_image_get_height (image));
    }
}

gboolean
picman_image_get_component_visible (const PicmanImage *image,
                                  PicmanChannelType  channel)
{
  gint index = -1;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  index = picman_image_get_component_index (image, channel);

  if (index != -1)
    return PICMAN_IMAGE_GET_PRIVATE (image)->visible[index];

  return FALSE;
}

void
picman_image_get_visible_array (const PicmanImage *image,
                              gboolean        *components)
{
  PicmanImagePrivate *private;
  gint              i;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (components != NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  for (i = 0; i < MAX_CHANNELS; i++)
    components[i] = private->visible[i];
}

PicmanComponentMask
picman_image_get_visible_mask (const PicmanImage *image)
{
  PicmanImagePrivate  *private;
  PicmanComponentMask  mask = 0;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  switch (picman_image_get_base_type (image))
    {
    case PICMAN_RGB:
      mask |= (private->visible[RED])   ? PICMAN_COMPONENT_RED   : 0;
      mask |= (private->visible[GREEN]) ? PICMAN_COMPONENT_GREEN : 0;
      mask |= (private->visible[BLUE])  ? PICMAN_COMPONENT_BLUE  : 0;
      mask |= (private->visible[ALPHA]) ? PICMAN_COMPONENT_ALPHA : 0;
      break;

    case PICMAN_GRAY:
    case PICMAN_INDEXED:
      mask |= (private->visible[GRAY])  ? PICMAN_COMPONENT_RED   : 0;
      mask |= (private->visible[GRAY])  ? PICMAN_COMPONENT_GREEN : 0;
      mask |= (private->visible[GRAY])  ? PICMAN_COMPONENT_BLUE  : 0;
      mask |= (private->visible[ALPHA]) ? PICMAN_COMPONENT_ALPHA : 0;
      break;
    }

  return mask;
}


/*  emitting image signals  */

void
picman_image_mode_changed (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[MODE_CHANGED], 0);
}

void
picman_image_precision_changed (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[PRECISION_CHANGED], 0);
}

void
picman_image_alpha_changed (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[ALPHA_CHANGED], 0);
}

void
picman_image_invalidate (PicmanImage *image,
                       gint       x,
                       gint       y,
                       gint       width,
                       gint       height)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  picman_projectable_invalidate (PICMAN_PROJECTABLE (image),
                               x, y, width, height);

  PICMAN_IMAGE_GET_PRIVATE (image)->flush_accum.preview_invalidated = TRUE;
}

void
picman_image_guide_added (PicmanImage *image,
                        PicmanGuide *guide)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_GUIDE (guide));

  g_signal_emit (image, picman_image_signals[GUIDE_ADDED], 0,
                 guide);
}

void
picman_image_guide_removed (PicmanImage *image,
                          PicmanGuide *guide)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_GUIDE (guide));

  g_signal_emit (image, picman_image_signals[GUIDE_REMOVED], 0,
                 guide);
}

void
picman_image_guide_moved (PicmanImage *image,
                        PicmanGuide *guide)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_GUIDE (guide));

  g_signal_emit (image, picman_image_signals[GUIDE_MOVED], 0,
                 guide);
}

void
picman_image_sample_point_added (PicmanImage       *image,
                               PicmanSamplePoint *sample_point)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (sample_point != NULL);

  g_signal_emit (image, picman_image_signals[SAMPLE_POINT_ADDED], 0,
                 sample_point);
}

void
picman_image_sample_point_removed (PicmanImage       *image,
                                 PicmanSamplePoint *sample_point)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (sample_point != NULL);

  g_signal_emit (image, picman_image_signals[SAMPLE_POINT_REMOVED], 0,
                 sample_point);
}

void
picman_image_sample_point_moved (PicmanImage       *image,
                               PicmanSamplePoint *sample_point)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (sample_point != NULL);

  g_signal_emit (image, picman_image_signals[SAMPLE_POINT_MOVED], 0,
                 sample_point);
}

/**
 * picman_image_size_changed_detailed:
 * @image:
 * @previous_origin_x:
 * @previous_origin_y:
 *
 * Emits the size-changed-detailed signal that is typically used to adjust the
 * position of the image in the display shell on various operations,
 * e.g. crop.
 *
 * This function makes sure that PicmanViewable::size-changed is also emitted.
 **/
void
picman_image_size_changed_detailed (PicmanImage *image,
                                  gint       previous_origin_x,
                                  gint       previous_origin_y,
                                  gint       previous_width,
                                  gint       previous_height)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[SIZE_CHANGED_DETAILED], 0,
                 previous_origin_x,
                 previous_origin_y,
                 previous_width,
                 previous_height);
}

void
picman_image_colormap_changed (PicmanImage *image,
                             gint       color_index)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (color_index >= -1 &&
                    color_index < PICMAN_IMAGE_GET_PRIVATE (image)->n_colors);

  g_signal_emit (image, picman_image_signals[COLORMAP_CHANGED], 0,
                 color_index);
}

void
picman_image_selection_invalidate (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[SELECTION_INVALIDATE], 0);
}

void
picman_image_quick_mask_changed (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_emit (image, picman_image_signals[QUICK_MASK_CHANGED], 0);
}

void
picman_image_undo_event (PicmanImage     *image,
                       PicmanUndoEvent  event,
                       PicmanUndo      *undo)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (((event == PICMAN_UNDO_EVENT_UNDO_FREE   ||
                      event == PICMAN_UNDO_EVENT_UNDO_FREEZE ||
                      event == PICMAN_UNDO_EVENT_UNDO_THAW) && undo == NULL) ||
                    PICMAN_IS_UNDO (undo));

  g_signal_emit (image, picman_image_signals[UNDO_EVENT], 0, event, undo);
}


/*  dirty counters  */

/* NOTE about the image->dirty counter:
 *   If 0, then the image is clean (ie, copy on disk is the same as the one
 *      in memory).
 *   If positive, then that's the number of dirtying operations done
 *       on the image since the last save.
 *   If negative, then user has hit undo and gone back in time prior
 *       to the saved copy.  Hitting redo will eventually come back to
 *       the saved copy.
 *
 *   The image is dirty (ie, needs saving) if counter is non-zero.
 *
 *   If the counter is around 100000, this is due to undo-ing back
 *   before a saved version, then changing the image (thus destroying
 *   the redo stack).  Once this has happened, it's impossible to get
 *   the image back to the state on disk, since the redo info has been
 *   freed.  See picmanimage-undo.c for the gory details.
 */

/*
 * NEVER CALL picman_image_dirty() directly!
 *
 * If your code has just dirtied the image, push an undo instead.
 * Failing that, push the trivial undo which tells the user the
 * command is not undoable: undo_push_cantundo() (But really, it would
 * be best to push a proper undo).  If you just dirty the image
 * without pushing an undo then the dirty count is increased, but
 * popping that many undo actions won't lead to a clean image.
 */

gint
picman_image_dirty (PicmanImage     *image,
                  PicmanDirtyMask  dirty_mask)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  private->dirty++;
  private->export_dirty++;

  if (! private->dirty_time)
    private->dirty_time = time (NULL);

  g_signal_emit (image, picman_image_signals[DIRTY], 0, dirty_mask);

  TRC (("dirty %d -> %d\n", private->dirty - 1, private->dirty));

  return private->dirty;
}

gint
picman_image_clean (PicmanImage     *image,
                  PicmanDirtyMask  dirty_mask)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  private->dirty--;
  private->export_dirty--;

  g_signal_emit (image, picman_image_signals[CLEAN], 0, dirty_mask);

  TRC (("clean %d -> %d\n", private->dirty + 1, private->dirty));

  return private->dirty;
}

void
picman_image_clean_all (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  private->dirty      = 0;
  private->dirty_time = 0;

  g_signal_emit (image, picman_image_signals[CLEAN], 0, PICMAN_DIRTY_ALL);
}

void
picman_image_export_clean_all (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  private->export_dirty = 0;

  g_signal_emit (image, picman_image_signals[CLEAN], 0, PICMAN_DIRTY_ALL);
}

/**
 * picman_image_is_dirty:
 * @image:
 *
 * Returns: True if the image is dirty, false otherwise.
 **/
gint
picman_image_is_dirty (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  return PICMAN_IMAGE_GET_PRIVATE (image)->dirty != 0;
}

/**
 * picman_image_is_export_dirty:
 * @image:
 *
 * Returns: True if the image export is dirty, false otherwise.
 **/
gboolean
picman_image_is_export_dirty (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  return PICMAN_IMAGE_GET_PRIVATE (image)->export_dirty != 0;
}

gint
picman_image_get_dirty_time (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  return PICMAN_IMAGE_GET_PRIVATE (image)->dirty_time;
}

/**
 * picman_image_saved:
 * @image:
 * @uri:
 *
 * Emits the "saved" signal, indicating that @image was saved to the
 * location specified by @uri.
 */
void
picman_image_saved (PicmanImage   *image,
                  const gchar *uri)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (uri != NULL);

  g_signal_emit (image, picman_image_signals[SAVED], 0, uri);
}

/**
 * picman_image_exported:
 * @image:
 * @uri:
 *
 * Emits the "exported" signal, indicating that @image was exported to the
 * location specified by @uri.
 */
void
picman_image_exported (PicmanImage   *image,
                     const gchar *uri)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (uri != NULL);

  g_signal_emit (image, picman_image_signals[EXPORTED], 0, uri);
}


/*  flush this image's displays  */

void
picman_image_flush (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  picman_projectable_flush (PICMAN_PROJECTABLE (image),
                          PICMAN_IMAGE_GET_PRIVATE (image)->flush_accum.preview_invalidated);
}


/*  display / instance counters  */

gint
picman_image_get_display_count (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  return PICMAN_IMAGE_GET_PRIVATE (image)->disp_count;
}

void
picman_image_inc_display_count (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  PICMAN_IMAGE_GET_PRIVATE (image)->disp_count++;
}

void
picman_image_dec_display_count (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  PICMAN_IMAGE_GET_PRIVATE (image)->disp_count--;
}

gint
picman_image_get_instance_count (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  return PICMAN_IMAGE_GET_PRIVATE (image)->instance_count;
}

void
picman_image_inc_instance_count (PicmanImage *image)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  PICMAN_IMAGE_GET_PRIVATE (image)->instance_count++;
}


/*  parasites  */

const PicmanParasite *
picman_image_parasite_find (const PicmanImage *image,
                          const gchar     *name)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_parasite_list_find (PICMAN_IMAGE_GET_PRIVATE (image)->parasites,
                                  name);
}

static void
list_func (gchar          *key,
           PicmanParasite   *p,
           gchar        ***cur)
{
  *(*cur)++ = (gchar *) g_strdup (key);
}

gchar **
picman_image_parasite_list (const PicmanImage *image,
                          gint            *count)
{
  PicmanImagePrivate  *private;
  gchar            **list;
  gchar            **cur;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  *count = picman_parasite_list_length (private->parasites);
  cur = list = g_new (gchar *, *count);

  picman_parasite_list_foreach (private->parasites, (GHFunc) list_func, &cur);

  return list;
}

void
picman_image_parasite_attach (PicmanImage          *image,
                            const PicmanParasite *parasite)
{
  PicmanParasite  copy;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (parasite != NULL);

  /*  make a temporary copy of the PicmanParasite struct because
   *  picman_parasite_shift_parent() changes it
   */
  copy = *parasite;

  /*  only set the dirty bit manually if we can be saved and the new
   *  parasite differs from the current one and we aren't undoable
   */
  if (picman_parasite_is_undoable (&copy))
    picman_image_undo_push_image_parasite (image,
                                         C_("undo-type", "Attach Parasite to Image"),
                                         &copy);

  /*  We used to push an cantundo on te stack here. This made the undo stack
   *  unusable (NULL on the stack) and prevented people from undoing after a
   *  save (since most save plug-ins attach an undoable comment parasite).
   *  Now we simply attach the parasite without pushing an undo. That way
   *  it's undoable but does not block the undo system.   --Sven
   */
  picman_parasite_list_add (PICMAN_IMAGE_GET_PRIVATE (image)->parasites, &copy);

  if (picman_parasite_has_flag (&copy, PICMAN_PARASITE_ATTACH_PARENT))
    {
      picman_parasite_shift_parent (&copy);
      picman_parasite_attach (image->picman, &copy);
    }

  g_signal_emit (image, picman_image_signals[PARASITE_ATTACHED], 0,
                 parasite->name);

  if (strcmp (parasite->name, "icc-profile") == 0)
    picman_color_managed_profile_changed (PICMAN_COLOR_MANAGED (image));
}

void
picman_image_parasite_detach (PicmanImage   *image,
                            const gchar *name)
{
  PicmanImagePrivate   *private;
  const PicmanParasite *parasite;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (name != NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (! (parasite = picman_parasite_list_find (private->parasites, name)))
    return;

  if (picman_parasite_is_undoable (parasite))
    picman_image_undo_push_image_parasite_remove (image,
                                                C_("undo-type", "Remove Parasite from Image"),
                                                name);

  picman_parasite_list_remove (private->parasites, name);

  g_signal_emit (image, picman_image_signals[PARASITE_DETACHED], 0,
                 name);

  if (strcmp (name, "icc-profile") == 0)
    picman_color_managed_profile_changed (PICMAN_COLOR_MANAGED (image));
}


/*  tattoos  */

PicmanTattoo
picman_image_get_new_tattoo (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  private->tattoo_state++;

  if (G_UNLIKELY (private->tattoo_state == 0))
    g_warning ("%s: Tattoo state corrupted (integer overflow).", G_STRFUNC);

  return private->tattoo_state;
}

PicmanTattoo
picman_image_get_tattoo_state (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  return PICMAN_IMAGE_GET_PRIVATE (image)->tattoo_state;
}

gboolean
picman_image_set_tattoo_state (PicmanImage  *image,
                             PicmanTattoo  val)
{
  GList      *all_items;
  GList      *list;
  gboolean    retval = TRUE;
  PicmanTattoo  maxval = 0;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  /* Check that the layer tattoos don't overlap with channel or vector ones */
  all_items = picman_image_get_layer_list (image);

  for (list = all_items; list; list = g_list_next (list))
    {
      PicmanTattoo ltattoo;

      ltattoo = picman_item_get_tattoo (PICMAN_ITEM (list->data));
      if (ltattoo > maxval)
        maxval = ltattoo;

      if (picman_image_get_channel_by_tattoo (image, ltattoo))
        retval = FALSE; /* Oopps duplicated tattoo in channel */

      if (picman_image_get_vectors_by_tattoo (image, ltattoo))
        retval = FALSE; /* Oopps duplicated tattoo in vectors */
    }

  g_list_free (all_items);

  /* Now check that the channel and vectors tattoos don't overlap */
  all_items = picman_image_get_channel_list (image);

  for (list = all_items; list; list = g_list_next (list))
    {
      PicmanTattoo ctattoo;

      ctattoo = picman_item_get_tattoo (PICMAN_ITEM (list->data));
      if (ctattoo > maxval)
        maxval = ctattoo;

      if (picman_image_get_vectors_by_tattoo (image, ctattoo))
        retval = FALSE; /* Oopps duplicated tattoo in vectors */
    }

  g_list_free (all_items);

  /* Find the max tattoo value in the vectors */
  all_items = picman_image_get_vectors_list (image);

  for (list = all_items; list; list = g_list_next (list))
    {
      PicmanTattoo vtattoo;

      vtattoo = picman_item_get_tattoo (PICMAN_ITEM (list->data));
      if (vtattoo > maxval)
        maxval = vtattoo;
    }

  g_list_free (all_items);

  if (val < maxval)
    retval = FALSE;

  /* Must check if the state is valid */
  if (retval == TRUE)
    PICMAN_IMAGE_GET_PRIVATE (image)->tattoo_state = val;

  return retval;
}


/*  projection  */

PicmanProjection *
picman_image_get_projection (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->projection;
}


/*  layers / channels / vectors  */

PicmanItemTree *
picman_image_get_layer_tree (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->layers;
}

PicmanItemTree *
picman_image_get_channel_tree (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->channels;
}

PicmanItemTree *
picman_image_get_vectors_tree (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->vectors;
}

PicmanContainer *
picman_image_get_layers (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->layers->container;
}

PicmanContainer *
picman_image_get_channels (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->channels->container;
}

PicmanContainer *
picman_image_get_vectors (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->vectors->container;
}

gint
picman_image_get_n_layers (const PicmanImage *image)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  stack = PICMAN_ITEM_STACK (picman_image_get_layers (image));

  return picman_item_stack_get_n_items (stack);
}

gint
picman_image_get_n_channels (const PicmanImage *image)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  stack = PICMAN_ITEM_STACK (picman_image_get_channels (image));

  return picman_item_stack_get_n_items (stack);
}

gint
picman_image_get_n_vectors (const PicmanImage *image)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  stack = PICMAN_ITEM_STACK (picman_image_get_vectors (image));

  return picman_item_stack_get_n_items (stack);
}

GList *
picman_image_get_layer_iter (const PicmanImage *image)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  stack = PICMAN_ITEM_STACK (picman_image_get_layers (image));

  return picman_item_stack_get_item_iter (stack);
}

GList *
picman_image_get_channel_iter (const PicmanImage *image)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  stack = PICMAN_ITEM_STACK (picman_image_get_channels (image));

  return picman_item_stack_get_item_iter (stack);
}

GList *
picman_image_get_vectors_iter (const PicmanImage *image)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  stack = PICMAN_ITEM_STACK (picman_image_get_vectors (image));

  return picman_item_stack_get_item_iter (stack);
}

GList *
picman_image_get_layer_list (const PicmanImage *image)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  stack = PICMAN_ITEM_STACK (picman_image_get_layers (image));

  return picman_item_stack_get_item_list (stack);
}

GList *
picman_image_get_channel_list (const PicmanImage *image)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  stack = PICMAN_ITEM_STACK (picman_image_get_channels (image));

  return picman_item_stack_get_item_list (stack);
}

GList *
picman_image_get_vectors_list (const PicmanImage *image)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  stack = PICMAN_ITEM_STACK (picman_image_get_vectors (image));

  return picman_item_stack_get_item_list (stack);
}


/*  active drawable, layer, channel, vectors  */

PicmanDrawable *
picman_image_get_active_drawable (const PicmanImage *image)
{
  PicmanImagePrivate *private;
  PicmanItem         *active_channel;
  PicmanItem         *active_layer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  active_channel = picman_item_tree_get_active_item (private->channels);
  active_layer   = picman_item_tree_get_active_item (private->layers);

  /*  If there is an active channel (a saved selection, etc.),
   *  we ignore the active layer
   */
  if (active_channel)
    {
      return PICMAN_DRAWABLE (active_channel);
    }
  else if (active_layer)
    {
      PicmanLayer     *layer = PICMAN_LAYER (active_layer);
      PicmanLayerMask *mask  = picman_layer_get_mask (layer);

      if (mask && picman_layer_get_edit_mask (layer))
        return PICMAN_DRAWABLE (mask);
      else
        return PICMAN_DRAWABLE (layer);
    }

  return NULL;
}

PicmanLayer *
picman_image_get_active_layer (const PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  return PICMAN_LAYER (picman_item_tree_get_active_item (private->layers));
}

PicmanChannel *
picman_image_get_active_channel (const PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  return PICMAN_CHANNEL (picman_item_tree_get_active_item (private->channels));
}

PicmanVectors *
picman_image_get_active_vectors (const PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  return PICMAN_VECTORS (picman_item_tree_get_active_item (private->vectors));
}

PicmanLayer *
picman_image_set_active_layer (PicmanImage *image,
                             PicmanLayer *layer)
{
  PicmanImagePrivate *private;
  PicmanLayer        *floating_sel;
  PicmanLayer        *active_layer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (layer == NULL || PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (layer == NULL ||
                        (picman_item_is_attached (PICMAN_ITEM (layer)) &&
                         picman_item_get_image (PICMAN_ITEM (layer)) == image),
                        NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  floating_sel = picman_image_get_floating_selection (image);

  /*  Make sure the floating_sel always is the active layer  */
  if (floating_sel && layer != floating_sel)
    return floating_sel;

  active_layer = picman_image_get_active_layer (image);

  if (layer != active_layer)
    {
      /*  Don't cache selection info for the previous active layer  */
      if (active_layer)
        picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (active_layer));

      picman_item_tree_set_active_item (private->layers, PICMAN_ITEM (layer));
    }

  return picman_image_get_active_layer (image);
}

PicmanChannel *
picman_image_set_active_channel (PicmanImage   *image,
                               PicmanChannel *channel)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (channel == NULL || PICMAN_IS_CHANNEL (channel), NULL);
  g_return_val_if_fail (channel == NULL ||
                        (picman_item_is_attached (PICMAN_ITEM (channel)) &&
                         picman_item_get_image (PICMAN_ITEM (channel)) == image),
                        NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  /*  Not if there is a floating selection  */
  if (channel && picman_image_get_floating_selection (image))
    return NULL;

  if (channel != picman_image_get_active_channel (image))
    {
      picman_item_tree_set_active_item (private->channels, PICMAN_ITEM (channel));
    }

  return picman_image_get_active_channel (image);
}

PicmanChannel *
picman_image_unset_active_channel (PicmanImage *image)
{
  PicmanImagePrivate *private;
  PicmanChannel      *channel;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  channel = picman_image_get_active_channel (image);

  if (channel)
    {
      picman_image_set_active_channel (image, NULL);

      if (private->layer_stack)
        picman_image_set_active_layer (image, private->layer_stack->data);
    }

  return channel;
}

PicmanVectors *
picman_image_set_active_vectors (PicmanImage   *image,
                               PicmanVectors *vectors)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (vectors == NULL || PICMAN_IS_VECTORS (vectors), NULL);
  g_return_val_if_fail (vectors == NULL ||
                        (picman_item_is_attached (PICMAN_ITEM (vectors)) &&
                         picman_item_get_image (PICMAN_ITEM (vectors)) == image),
                        NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (vectors != picman_image_get_active_vectors (image))
    {
      picman_item_tree_set_active_item (private->vectors, PICMAN_ITEM (vectors));
    }

  return picman_image_get_active_vectors (image);
}


/*  layer, channel, vectors by tattoo  */

PicmanLayer *
picman_image_get_layer_by_tattoo (const PicmanImage *image,
                                PicmanTattoo       tattoo)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  stack = PICMAN_ITEM_STACK (picman_image_get_layers (image));

  return PICMAN_LAYER (picman_item_stack_get_item_by_tattoo (stack, tattoo));
}

PicmanChannel *
picman_image_get_channel_by_tattoo (const PicmanImage *image,
                                  PicmanTattoo       tattoo)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  stack = PICMAN_ITEM_STACK (picman_image_get_channels (image));

  return PICMAN_CHANNEL (picman_item_stack_get_item_by_tattoo (stack, tattoo));
}

PicmanVectors *
picman_image_get_vectors_by_tattoo (const PicmanImage *image,
                                  PicmanTattoo       tattoo)
{
  PicmanItemStack *stack;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  stack = PICMAN_ITEM_STACK (picman_image_get_vectors (image));

  return PICMAN_VECTORS (picman_item_stack_get_item_by_tattoo (stack, tattoo));
}


/*  layer, channel, vectors by name  */

PicmanLayer *
picman_image_get_layer_by_name (const PicmanImage *image,
                              const gchar     *name)
{
  PicmanItemTree *tree;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  tree = picman_image_get_layer_tree (image);

  return PICMAN_LAYER (picman_item_tree_get_item_by_name (tree, name));
}

PicmanChannel *
picman_image_get_channel_by_name (const PicmanImage *image,
                                const gchar     *name)
{
  PicmanItemTree *tree;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  tree = picman_image_get_channel_tree (image);

  return PICMAN_CHANNEL (picman_item_tree_get_item_by_name (tree, name));
}

PicmanVectors *
picman_image_get_vectors_by_name (const PicmanImage *image,
                                const gchar     *name)
{
  PicmanItemTree *tree;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  tree = picman_image_get_vectors_tree (image);

  return PICMAN_VECTORS (picman_item_tree_get_item_by_name (tree, name));
}


/*  items  */

gboolean
picman_image_reorder_item (PicmanImage   *image,
                         PicmanItem    *item,
                         PicmanItem    *new_parent,
                         gint         new_index,
                         gboolean     push_undo,
                         const gchar *undo_desc)
{
  PicmanItemTree *tree;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (picman_item_get_image (item) == image, FALSE);

  tree = picman_item_get_tree (item);

  g_return_val_if_fail (tree != NULL, FALSE);

  if (push_undo && ! undo_desc)
    undo_desc = PICMAN_ITEM_GET_CLASS (item)->reorder_desc;

  /*  item and new_parent are type-checked in PicmanItemTree
   */
  return picman_item_tree_reorder_item (tree, item,
                                      new_parent, new_index,
                                      push_undo, undo_desc);
}

gboolean
picman_image_raise_item (PicmanImage *image,
                       PicmanItem  *item,
                       GError    **error)
{
  gint index;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  index = picman_item_get_index (item);

  g_return_val_if_fail (index != -1, FALSE);

  if (index == 0)
    {
      g_set_error_literal (error,  PICMAN_ERROR, PICMAN_FAILED,
			   PICMAN_ITEM_GET_CLASS (item)->raise_failed);
      return FALSE;
    }

  return picman_image_reorder_item (image, item,
                                  picman_item_get_parent (item), index - 1,
                                  TRUE, PICMAN_ITEM_GET_CLASS (item)->raise_desc);
}

gboolean
picman_image_raise_item_to_top (PicmanImage *image,
                              PicmanItem  *item)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  return picman_image_reorder_item (image, item,
                                  picman_item_get_parent (item), 0,
                                  TRUE, PICMAN_ITEM_GET_CLASS (item)->raise_to_top_desc);
}

gboolean
picman_image_lower_item (PicmanImage *image,
                       PicmanItem  *item,
                       GError    **error)
{
  PicmanContainer *container;
  gint           index;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  container = picman_item_get_container (item);

  g_return_val_if_fail (container != NULL, FALSE);

  index = picman_item_get_index (item);

  if (index == picman_container_get_n_children (container) - 1)
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   PICMAN_ITEM_GET_CLASS (item)->lower_failed);
      return FALSE;
    }

  return picman_image_reorder_item (image, item,
                                  picman_item_get_parent (item), index + 1,
                                  TRUE, PICMAN_ITEM_GET_CLASS (item)->lower_desc);
}

gboolean
picman_image_lower_item_to_bottom (PicmanImage *image,
                                  PicmanItem *item)
{
  PicmanContainer *container;
  gint           length;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);

  container = picman_item_get_container (item);

  g_return_val_if_fail (container != NULL, FALSE);

  length = picman_container_get_n_children (container);

  return picman_image_reorder_item (image, item,
                                  picman_item_get_parent (item), length - 1,
                                  TRUE, PICMAN_ITEM_GET_CLASS (item)->lower_to_bottom_desc);
}


/*  layers  */

gboolean
picman_image_add_layer (PicmanImage *image,
                      PicmanLayer *layer,
                      PicmanLayer *parent,
                      gint       position,
                      gboolean   push_undo)
{
  PicmanImagePrivate *private;
  gboolean          old_has_alpha;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  /*  item and parent are type-checked in PicmanItemTree
   */
  if (! picman_item_tree_get_insert_pos (private->layers,
                                       (PicmanItem *) layer,
                                       (PicmanItem **) &parent,
                                       &position))
    return FALSE;

  /*  If there is a floating selection (and this isn't it!),
   *  make sure the insert position is greater than 0
   */
  if (parent == NULL && position == 0 &&
      picman_image_get_floating_selection (image))
    position = 1;

  old_has_alpha = picman_image_has_alpha (image);

  if (push_undo)
    picman_image_undo_push_layer_add (image, C_("undo-type", "Add Layer"),
                                    layer,
                                    picman_image_get_active_layer (image));

  picman_item_tree_add_item (private->layers, PICMAN_ITEM (layer),
                           PICMAN_ITEM (parent), position);

  picman_image_set_active_layer (image, layer);

  /*  If the layer is a floating selection, attach it to the drawable  */
  if (picman_layer_is_floating_sel (layer))
    picman_drawable_attach_floating_sel (picman_layer_get_floating_sel_drawable (layer),
                                       layer);

  if (old_has_alpha != picman_image_has_alpha (image))
    private->flush_accum.alpha_changed = TRUE;

  return TRUE;
}

void
picman_image_remove_layer (PicmanImage *image,
                         PicmanLayer *layer,
                         gboolean   push_undo,
                         PicmanLayer *new_active)
{
  PicmanImagePrivate *private;
  PicmanLayer        *active_layer;
  gboolean          old_has_alpha;
  gboolean          undo_group = FALSE;
  const gchar      *undo_desc;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)));
  g_return_if_fail (picman_item_get_image (PICMAN_ITEM (layer)) == image);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (picman_drawable_get_floating_sel (PICMAN_DRAWABLE (layer)))
    {
      if (! push_undo)
        {
          g_warning ("%s() was called from an undo function while the layer "
                     "had a floating selection. Please report this at "
                     "http://www.picman.org/bugs/", G_STRFUNC);
          return;
        }

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_ITEM_REMOVE,
                                   C_("undo-type", "Remove Layer"));
      undo_group = TRUE;

      picman_image_remove_layer (image,
                               picman_drawable_get_floating_sel (PICMAN_DRAWABLE (layer)),
                               TRUE, NULL);
    }

  active_layer = picman_image_get_active_layer (image);

  old_has_alpha = picman_image_has_alpha (image);

  if (picman_layer_is_floating_sel (layer))
    {
      undo_desc = C_("undo-type", "Remove Floating Selection");

      picman_drawable_detach_floating_sel (picman_layer_get_floating_sel_drawable (layer));
    }
  else
    {
      undo_desc = C_("undo-type", "Remove Layer");
    }

  if (push_undo)
    picman_image_undo_push_layer_remove (image, undo_desc, layer,
                                       picman_layer_get_parent (layer),
                                       picman_item_get_index (PICMAN_ITEM (layer)),
                                       active_layer);

  g_object_ref (layer);

  /*  Make sure we're not caching any old selection info  */
  if (layer == active_layer)
    picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (layer));

  private->layer_stack = g_slist_remove (private->layer_stack, layer);

  /*  Also remove all children of a group layer from the layer_stack  */
  if (picman_viewable_get_children (PICMAN_VIEWABLE (layer)))
    {
      PicmanContainer *stack = picman_viewable_get_children (PICMAN_VIEWABLE (layer));
      GList         *children;
      GList         *list;

      children = picman_item_stack_get_item_list (PICMAN_ITEM_STACK (stack));

      for (list = children; list; list = g_list_next (list))
        {
          private->layer_stack = g_slist_remove (private->layer_stack,
                                                 list->data);
        }

      g_list_free (children);
    }

  if (! new_active && private->layer_stack)
    new_active = private->layer_stack->data;

  new_active =
    PICMAN_LAYER (picman_item_tree_remove_item (private->layers,
                                            PICMAN_ITEM (layer),
                                            PICMAN_ITEM (new_active)));

  if (picman_layer_is_floating_sel (layer))
    {
      /*  If this was the floating selection, activate the underlying drawable
       */
      floating_sel_activate_drawable (layer);
    }
  else if (active_layer &&
           (layer == active_layer ||
            picman_viewable_is_ancestor (PICMAN_VIEWABLE (layer),
                                       PICMAN_VIEWABLE (active_layer))))
    {
      picman_image_set_active_layer (image, new_active);
    }

  g_object_unref (layer);

  if (old_has_alpha != picman_image_has_alpha (image))
    private->flush_accum.alpha_changed = TRUE;

  if (undo_group)
    picman_image_undo_group_end (image);
}

void
picman_image_add_layers (PicmanImage   *image,
                       GList       *layers,
                       PicmanLayer   *parent,
                       gint         position,
                       gint         x,
                       gint         y,
                       gint         width,
                       gint         height,
                       const gchar *undo_desc)
{
  PicmanImagePrivate *private;
  GList            *list;
  gint              layers_x      = G_MAXINT;
  gint              layers_y      = G_MAXINT;
  gint              layers_width  = 0;
  gint              layers_height = 0;
  gint              offset_x;
  gint              offset_y;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (layers != NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  /*  item and parent are type-checked in PicmanItemTree
   */
  if (! picman_item_tree_get_insert_pos (private->layers,
                                       (PicmanItem *) layers->data,
                                       (PicmanItem **) &parent,
                                       &position))
    return;

  for (list = layers; list; list = g_list_next (list))
    {
      PicmanItem *item = PICMAN_ITEM (list->data);
      gint      off_x, off_y;

      picman_item_get_offset (item, &off_x, &off_y);

      layers_x = MIN (layers_x, off_x);
      layers_y = MIN (layers_y, off_y);

      layers_width  = MAX (layers_width,
                           off_x + picman_item_get_width (item)  - layers_x);
      layers_height = MAX (layers_height,
                           off_y + picman_item_get_height (item) - layers_y);
    }

  offset_x = x + (width  - layers_width)  / 2 - layers_x;
  offset_y = y + (height - layers_height) / 2 - layers_y;

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_LAYER_ADD, undo_desc);

  for (list = layers; list; list = g_list_next (list))
    {
      PicmanItem *new_item = PICMAN_ITEM (list->data);

      picman_item_translate (new_item, offset_x, offset_y, FALSE);

      picman_image_add_layer (image, PICMAN_LAYER (new_item),
                            parent, position, TRUE);
      position++;
    }

  if (layers)
    picman_image_set_active_layer (image, layers->data);

  picman_image_undo_group_end (image);
}


/*  channels  */

gboolean
picman_image_add_channel (PicmanImage   *image,
                        PicmanChannel *channel,
                        PicmanChannel *parent,
                        gint         position,
                        gboolean     push_undo)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  /*  item and parent are type-checked in PicmanItemTree
   */
  if (! picman_item_tree_get_insert_pos (private->channels,
                                       (PicmanItem *) channel,
                                       (PicmanItem **) &parent,
                                       &position))
    return FALSE;

  if (push_undo)
    picman_image_undo_push_channel_add (image, C_("undo-type", "Add Channel"),
                                      channel,
                                      picman_image_get_active_channel (image));

  picman_item_tree_add_item (private->channels, PICMAN_ITEM (channel),
                           PICMAN_ITEM (parent), position);

  picman_image_set_active_channel (image, channel);

  return TRUE;
}

void
picman_image_remove_channel (PicmanImage   *image,
                           PicmanChannel *channel,
                           gboolean     push_undo,
                           PicmanChannel *new_active)
{
  PicmanImagePrivate *private;
  PicmanChannel      *active_channel;
  gboolean          undo_group = FALSE;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));
  g_return_if_fail (picman_item_get_image (PICMAN_ITEM (channel)) == image);

  if (picman_drawable_get_floating_sel (PICMAN_DRAWABLE (channel)))
    {
      if (! push_undo)
        {
          g_warning ("%s() was called from an undo function while the channel "
                     "had a floating selection. Please report this at "
                     "http://www.picman.org/bugs/", G_STRFUNC);
          return;
        }

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_ITEM_REMOVE,
                                   C_("undo-type", "Remove Channel"));
      undo_group = TRUE;

      picman_image_remove_layer (image,
                               picman_drawable_get_floating_sel (PICMAN_DRAWABLE (channel)),
                               TRUE, NULL);
    }

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  active_channel = picman_image_get_active_channel (image);

  if (push_undo)
    picman_image_undo_push_channel_remove (image, C_("undo-type", "Remove Channel"), channel,
                                         picman_channel_get_parent (channel),
                                         picman_item_get_index (PICMAN_ITEM (channel)),
                                         active_channel);

  g_object_ref (channel);

  new_active =
    PICMAN_CHANNEL (picman_item_tree_remove_item (private->channels,
                                              PICMAN_ITEM (channel),
                                              PICMAN_ITEM (new_active)));

  if (active_channel &&
      (channel == active_channel ||
       picman_viewable_is_ancestor (PICMAN_VIEWABLE (channel),
                                  PICMAN_VIEWABLE (active_channel))))
    {
      if (new_active)
        picman_image_set_active_channel (image, new_active);
      else
        picman_image_unset_active_channel (image);
    }

  g_object_unref (channel);

  if (undo_group)
    picman_image_undo_group_end (image);
}


/*  vectors  */

gboolean
picman_image_add_vectors (PicmanImage   *image,
                        PicmanVectors *vectors,
                        PicmanVectors *parent,
                        gint         position,
                        gboolean     push_undo)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  /*  item and parent are type-checked in PicmanItemTree
   */
  if (! picman_item_tree_get_insert_pos (private->vectors,
                                       (PicmanItem *) vectors,
                                       (PicmanItem **) &parent,
                                       &position))
    return FALSE;

  if (push_undo)
    picman_image_undo_push_vectors_add (image, C_("undo-type", "Add Path"),
                                      vectors,
                                      picman_image_get_active_vectors (image));

  picman_item_tree_add_item (private->vectors, PICMAN_ITEM (vectors),
                           PICMAN_ITEM (parent), position);

  picman_image_set_active_vectors (image, vectors);

  return TRUE;
}

void
picman_image_remove_vectors (PicmanImage   *image,
                           PicmanVectors *vectors,
                           gboolean     push_undo,
                           PicmanVectors *new_active)
{
  PicmanImagePrivate *private;
  PicmanVectors      *active_vectors;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_VECTORS (vectors));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (vectors)));
  g_return_if_fail (picman_item_get_image (PICMAN_ITEM (vectors)) == image);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  active_vectors = picman_image_get_active_vectors (image);

  if (push_undo)
    picman_image_undo_push_vectors_remove (image, C_("undo-type", "Remove Path"), vectors,
                                         picman_vectors_get_parent (vectors),
                                         picman_item_get_index (PICMAN_ITEM (vectors)),
                                         active_vectors);

  g_object_ref (vectors);

  new_active =
    PICMAN_VECTORS (picman_item_tree_remove_item (private->vectors,
                                              PICMAN_ITEM (vectors),
                                              PICMAN_ITEM (new_active)));

  if (active_vectors &&
      (vectors == active_vectors ||
       picman_viewable_is_ancestor (PICMAN_VIEWABLE (vectors),
                                  PICMAN_VIEWABLE (active_vectors))))
    {
      picman_image_set_active_vectors (image, new_active);
    }

  g_object_unref (vectors);
}

gboolean
picman_image_coords_in_active_pickable (PicmanImage        *image,
                                      const PicmanCoords *coords,
                                      gboolean          sample_merged,
                                      gboolean          selected_only)
{
  gint     x, y;
  gboolean in_pickable = FALSE;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  x = floor (coords->x);
  y = floor (coords->y);

  if (sample_merged)
    {
      if (x >= 0 && x < picman_image_get_width  (image) &&
          y >= 0 && y < picman_image_get_height (image))
        in_pickable = TRUE;
    }
  else
    {
      PicmanDrawable *drawable = picman_image_get_active_drawable (image);

      if (drawable)
        {
          PicmanItem *item = PICMAN_ITEM (drawable);
          gint      off_x, off_y;
          gint      d_x, d_y;

          picman_item_get_offset (item, &off_x, &off_y);

          d_x = x - off_x;
          d_y = y - off_y;

          if (d_x >= 0 && d_x < picman_item_get_width  (item) &&
              d_y >= 0 && d_y < picman_item_get_height (item))
            in_pickable = TRUE;
        }
    }

  if (in_pickable && selected_only)
    {
      PicmanChannel *selection = picman_image_get_mask (image);

      if (! picman_channel_is_empty (selection) &&
          ! picman_pickable_get_opacity_at (PICMAN_PICKABLE (selection),
                                          x, y))
        {
          in_pickable = FALSE;
        }
    }

  return in_pickable;
}

void
picman_image_invalidate_previews (PicmanImage *image)
{
  PicmanItemStack *layers;
  PicmanItemStack *channels;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  layers   = PICMAN_ITEM_STACK (picman_image_get_layers (image));
  channels = PICMAN_ITEM_STACK (picman_image_get_channels (image));

  picman_item_stack_invalidate_previews (layers);
  picman_item_stack_invalidate_previews (channels);
}
