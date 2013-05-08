/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandockable.c
 * Copyright (C) 2001-2003 Michael Natterer <mitch@picman.org>
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "menus/menus.h"

#include "core/picmancontext.h"

#include "picmandialogfactory.h"
#include "picmandnd.h"
#include "picmandock.h"
#include "picmandockable.h"
#include "picmandockbook.h"
#include "picmandocked.h"
#include "picmandockwindow.h"
#include "picmanhelp-ids.h"
#include "picmanpanedbox.h"
#include "picmansessioninfo-aux.h"
#include "picmansessionmanaged.h"
#include "picmanuimanager.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_LOCKED
};


struct _PicmanDockablePrivate
{
  gchar        *name;
  gchar        *blurb;
  gchar        *stock_id;
  gchar        *help_id;
  PicmanTabStyle  tab_style;
  PicmanTabStyle  actual_tab_style;
  gboolean      locked;

  PicmanDockbook *dockbook;

  PicmanContext  *context;

  guint         blink_timeout_id;
  gint          blink_counter;

  PicmanPanedBox *drag_handler;

  /*  drag icon hotspot  */
  gint          drag_x;
  gint          drag_y;
};


static void       picman_dockable_session_managed_iface_init
                                                  (PicmanSessionManagedInterface
                                                                  *iface);
static void       picman_dockable_dispose           (GObject        *object);
static void       picman_dockable_set_property      (GObject        *object,
                                                   guint           property_id,
                                                   const GValue   *value,
                                                   GParamSpec     *pspec);
static void       picman_dockable_get_property      (GObject        *object,
                                                   guint           property_id,
                                                   GValue         *value,
                                                   GParamSpec     *pspec);

static void       picman_dockable_size_request      (GtkWidget      *widget,
                                                   GtkRequisition *requisition);
static void       picman_dockable_size_allocate     (GtkWidget      *widget,
                                                   GtkAllocation  *allocation);
static void       picman_dockable_drag_leave        (GtkWidget      *widget,
                                                   GdkDragContext *context,
                                                   guint           time);
static gboolean   picman_dockable_drag_motion       (GtkWidget      *widget,
                                                   GdkDragContext *context,
                                                   gint            x,
                                                   gint            y,
                                                   guint           time);
static gboolean   picman_dockable_drag_drop         (GtkWidget      *widget,
                                                   GdkDragContext *context,
                                                   gint            x,
                                                   gint            y,
                                                   guint           time);

static void       picman_dockable_style_set         (GtkWidget      *widget,
                                                   GtkStyle       *prev_style);

static void       picman_dockable_add               (GtkContainer   *container,
                                                   GtkWidget      *widget);
static GType      picman_dockable_child_type        (GtkContainer   *container);
static GList    * picman_dockable_get_aux_info      (PicmanSessionManaged
                                                                  *session_managed);
static void       picman_dockable_set_aux_info      (PicmanSessionManaged
                                                                  *session_managed,
                                                   GList          *aux_info);

static PicmanTabStyle
                  picman_dockable_convert_tab_style (PicmanDockable   *dockable,
                                                   PicmanTabStyle    tab_style);
static gboolean   picman_dockable_blink_timeout     (PicmanDockable   *dockable);


G_DEFINE_TYPE_WITH_CODE (PicmanDockable, picman_dockable, GTK_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_SESSION_MANAGED,
                                                picman_dockable_session_managed_iface_init))

#define parent_class picman_dockable_parent_class

static const GtkTargetEntry dialog_target_table[] = { PICMAN_TARGET_DIALOG };


static void
picman_dockable_class_init (PicmanDockableClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  GtkWidgetClass    *widget_class    = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->dispose       = picman_dockable_dispose;
  object_class->set_property  = picman_dockable_set_property;
  object_class->get_property  = picman_dockable_get_property;

  widget_class->size_request  = picman_dockable_size_request;
  widget_class->size_allocate = picman_dockable_size_allocate;
  widget_class->style_set     = picman_dockable_style_set;
  widget_class->drag_leave    = picman_dockable_drag_leave;
  widget_class->drag_motion   = picman_dockable_drag_motion;
  widget_class->drag_drop     = picman_dockable_drag_drop;

  container_class->add        = picman_dockable_add;
  container_class->child_type = picman_dockable_child_type;

  g_object_class_install_property (object_class, PROP_LOCKED,
                                   g_param_spec_boolean ("locked", NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("content-border",
                                                             NULL, NULL,
                                                             0,
                                                             G_MAXINT,
                                                             0,
                                                             PICMAN_PARAM_READABLE));

  g_type_class_add_private (klass, sizeof (PicmanDockablePrivate));
}

static void
picman_dockable_init (PicmanDockable *dockable)
{
  dockable->p = G_TYPE_INSTANCE_GET_PRIVATE (dockable,
                                             PICMAN_TYPE_DOCKABLE,
                                             PicmanDockablePrivate);
  dockable->p->tab_style        = PICMAN_TAB_STYLE_AUTOMATIC;
  dockable->p->actual_tab_style = PICMAN_TAB_STYLE_UNDEFINED;
  dockable->p->drag_x           = PICMAN_DOCKABLE_DRAG_OFFSET;
  dockable->p->drag_y           = PICMAN_DOCKABLE_DRAG_OFFSET;

  gtk_drag_dest_set (GTK_WIDGET (dockable),
                     0,
                     dialog_target_table, G_N_ELEMENTS (dialog_target_table),
                     GDK_ACTION_MOVE);
}

static void
picman_dockable_session_managed_iface_init (PicmanSessionManagedInterface *iface)
{
  iface->get_aux_info = picman_dockable_get_aux_info;
  iface->set_aux_info = picman_dockable_set_aux_info;
}

static void
picman_dockable_dispose (GObject *object)
{
  PicmanDockable *dockable = PICMAN_DOCKABLE (object);

  picman_dockable_blink_cancel (dockable);

  if (dockable->p->context)
    picman_dockable_set_context (dockable, NULL);

  if (dockable->p->blurb)
    {
      if (dockable->p->blurb != dockable->p->name)
        g_free (dockable->p->blurb);

      dockable->p->blurb = NULL;
    }

  if (dockable->p->name)
    {
      g_free (dockable->p->name);
      dockable->p->name = NULL;
    }

  if (dockable->p->stock_id)
    {
      g_free (dockable->p->stock_id);
      dockable->p->stock_id = NULL;
    }

  if (dockable->p->help_id)
    {
      g_free (dockable->p->help_id);
      dockable->p->help_id = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_dockable_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  PicmanDockable *dockable = PICMAN_DOCKABLE (object);

  switch (property_id)
    {
    case PROP_LOCKED:
      picman_dockable_set_locked (dockable, g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_dockable_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  PicmanDockable *dockable = PICMAN_DOCKABLE (object);

  switch (property_id)
    {
    case PROP_LOCKED:
      g_value_set_boolean (value, dockable->p->locked);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_dockable_size_request (GtkWidget      *widget,
                            GtkRequisition *requisition)
{
  GtkContainer   *container = GTK_CONTAINER (widget);
  GtkWidget      *child     = gtk_bin_get_child (GTK_BIN (widget));
  GtkRequisition  child_requisition;
  gint            border_width;

  border_width = gtk_container_get_border_width (container);

  requisition->width  = border_width * 2;
  requisition->height = border_width * 2;

  if (child && gtk_widget_get_visible (child))
    {
      gtk_widget_size_request (child, &child_requisition);

      requisition->width  += child_requisition.width;
      requisition->height += child_requisition.height;
    }
}

static void
picman_dockable_size_allocate (GtkWidget     *widget,
                             GtkAllocation *allocation)
{
  GtkContainer   *container = GTK_CONTAINER (widget);
  GtkWidget      *child     = gtk_bin_get_child (GTK_BIN (widget));

  GtkRequisition  button_requisition = { 0, };
  GtkAllocation   child_allocation;
  gint            border_width;


  gtk_widget_set_allocation (widget, allocation);

  border_width = gtk_container_get_border_width (container);

  if (child && gtk_widget_get_visible (child))
    {
      child_allocation.x      = allocation->x + border_width;
      child_allocation.y      = allocation->y + border_width;
      child_allocation.width  = MAX (allocation->width  -
                                     border_width * 2,
                                     0);
      child_allocation.height = MAX (allocation->height -
                                     border_width * 2 -
                                     button_requisition.height,
                                     0);

      child_allocation.y += button_requisition.height;

      gtk_widget_size_allocate (child, &child_allocation);
    }
}

static void
picman_dockable_drag_leave (GtkWidget      *widget,
                          GdkDragContext *context,
                          guint           time)
{
  picman_highlight_widget (widget, FALSE);
}

static gboolean
picman_dockable_drag_motion (GtkWidget      *widget,
                           GdkDragContext *context,
                           gint            x,
                           gint            y,
                           guint           time)
{
  PicmanDockable *dockable          = PICMAN_DOCKABLE (widget);
  gboolean      other_will_handle = FALSE;

  other_will_handle = picman_paned_box_will_handle_drag (dockable->p->drag_handler,
                                                       widget,
                                                       context,
                                                       x, y,
                                                       time);

  gdk_drag_status (context, other_will_handle ? 0 : GDK_ACTION_MOVE, time);
  picman_highlight_widget (widget, ! other_will_handle);
  return other_will_handle ? FALSE : TRUE;
}

static gboolean
picman_dockable_drag_drop (GtkWidget      *widget,
                         GdkDragContext *context,
                         gint            x,
                         gint            y,
                         guint           time)
{
  PicmanDockable *dockable = PICMAN_DOCKABLE (widget);
  gboolean      handled  = FALSE;

  if (picman_paned_box_will_handle_drag (dockable->p->drag_handler,
                                       widget,
                                       context,
                                       x, y,
                                       time))
    {
      /* Make event fall through to the drag handler */
      handled = FALSE;
    }
  else
    {
      handled =
        picman_dockbook_drop_dockable (PICMAN_DOCKABLE (widget)->p->dockbook,
                                     gtk_drag_get_source_widget (context));
    }

  /* We must call gtk_drag_finish() ourselves */
  if (handled)
    gtk_drag_finish (context, TRUE, TRUE, time);

  return handled;
}

static void
picman_dockable_style_set (GtkWidget *widget,
                         GtkStyle  *prev_style)
{
  gint content_border;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget,
                        "content-border", &content_border,
                        NULL);

  gtk_container_set_border_width (GTK_CONTAINER (widget), content_border);
}


static void
picman_dockable_add (GtkContainer *container,
                   GtkWidget    *widget)
{
  PicmanDockable *dockable;

  g_return_if_fail (gtk_bin_get_child (GTK_BIN (container)) == NULL);

  GTK_CONTAINER_CLASS (parent_class)->add (container, widget);

  /*  not all tab styles are supported by all children  */
  dockable = PICMAN_DOCKABLE (container);
  picman_dockable_set_tab_style (dockable, dockable->p->tab_style);
}

static GType
picman_dockable_child_type (GtkContainer *container)
{
  if (gtk_bin_get_child (GTK_BIN (container)))
    return G_TYPE_NONE;

  return PICMAN_TYPE_DOCKED;
}

static GtkWidget *
picman_dockable_new_tab_widget_internal (PicmanDockable *dockable,
                                       PicmanContext  *context,
                                       PicmanTabStyle  tab_style,
                                       GtkIconSize   size,
                                       gboolean      dnd)
{
  GtkWidget *tab_widget = NULL;
  GtkWidget *label      = NULL;
  GtkWidget *icon       = NULL;

  switch (tab_style)
    {
    case PICMAN_TAB_STYLE_NAME:
    case PICMAN_TAB_STYLE_ICON_NAME:
    case PICMAN_TAB_STYLE_PREVIEW_NAME:
      label = gtk_label_new (dockable->p->name);
      break;

    case PICMAN_TAB_STYLE_BLURB:
    case PICMAN_TAB_STYLE_ICON_BLURB:
    case PICMAN_TAB_STYLE_PREVIEW_BLURB:
      label = gtk_label_new (dockable->p->blurb);
      break;

    default:
      break;
    }

  switch (tab_style)
    {
    case PICMAN_TAB_STYLE_ICON:
    case PICMAN_TAB_STYLE_ICON_NAME:
    case PICMAN_TAB_STYLE_ICON_BLURB:
      icon = picman_dockable_get_icon (dockable, size);
      break;

    case PICMAN_TAB_STYLE_PREVIEW:
    case PICMAN_TAB_STYLE_PREVIEW_NAME:
    case PICMAN_TAB_STYLE_PREVIEW_BLURB:
      {
        GtkWidget *child = gtk_bin_get_child (GTK_BIN (dockable));

        if (child)
          icon = picman_docked_get_preview (PICMAN_DOCKED (child),
                                          context, size);

        if (! icon)
          icon = picman_dockable_get_icon (dockable, size);
      }
      break;

    default:
      break;
    }

  if (label && dnd)
    picman_label_set_attributes (GTK_LABEL (label),
                               PANGO_ATTR_WEIGHT, PANGO_WEIGHT_SEMIBOLD,
                               -1);

  switch (tab_style)
    {
    case PICMAN_TAB_STYLE_ICON:
    case PICMAN_TAB_STYLE_PREVIEW:
      tab_widget = icon;
      break;

    case PICMAN_TAB_STYLE_NAME:
    case PICMAN_TAB_STYLE_BLURB:
      tab_widget = label;
      break;

    case PICMAN_TAB_STYLE_ICON_NAME:
    case PICMAN_TAB_STYLE_ICON_BLURB:
    case PICMAN_TAB_STYLE_PREVIEW_NAME:
    case PICMAN_TAB_STYLE_PREVIEW_BLURB:
      tab_widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, dnd ? 6 : 2);

      gtk_box_pack_start (GTK_BOX (tab_widget), icon, FALSE, FALSE, 0);
      gtk_widget_show (icon);

      gtk_box_pack_start (GTK_BOX (tab_widget), label, FALSE, FALSE, 0);
      gtk_widget_show (label);
      break;

    case PICMAN_TAB_STYLE_UNDEFINED:
    case PICMAN_TAB_STYLE_AUTOMATIC:
      g_warning ("Tab style error, unexpected code path taken, fix!");
      break;
    }

  return tab_widget;
}

/*  public functions  */

GtkWidget *
picman_dockable_new (const gchar *name,
                   const gchar *blurb,
                   const gchar *stock_id,
                   const gchar *help_id)
{
  PicmanDockable *dockable;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (stock_id != NULL, NULL);
  g_return_val_if_fail (help_id != NULL, NULL);

  dockable = g_object_new (PICMAN_TYPE_DOCKABLE, NULL);

  dockable->p->name     = g_strdup (name);
  dockable->p->stock_id = g_strdup (stock_id);
  dockable->p->help_id  = g_strdup (help_id);

  if (blurb)
    dockable->p->blurb  = g_strdup (blurb);
  else
    dockable->p->blurb  = dockable->p->name;

  picman_help_set_help_data (GTK_WIDGET (dockable), NULL, help_id);

  return GTK_WIDGET (dockable);
}

void
picman_dockable_set_dockbook (PicmanDockable *dockable,
                            PicmanDockbook *dockbook)
{
  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));
  g_return_if_fail (dockbook == NULL ||
                    PICMAN_IS_DOCKBOOK (dockbook));

  dockable->p->dockbook = dockbook;
}

PicmanDockbook *
picman_dockable_get_dockbook (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);

  return dockable->p->dockbook;
}

PicmanTabStyle
picman_dockable_get_tab_style (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), -1);

  return dockable->p->tab_style;
}

/**
 * picman_dockable_get_actual_tab_style:
 * @dockable:
 *
 * Get actual tab style, i.e. never "automatic". This state should
 * actually be hold on a per-dockbook basis, but at this point that
 * feels like over-engineering...
 **/
PicmanTabStyle
picman_dockable_get_actual_tab_style (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), -1);

  return dockable->p->actual_tab_style;
}

const gchar *
picman_dockable_get_name (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);

  return dockable->p->name;
}

const gchar *
picman_dockable_get_blurb (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);

  return dockable->p->blurb;
}

const gchar *
picman_dockable_get_help_id (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);

  return dockable->p->help_id;
}

const gchar *
picman_dockable_get_stock_id (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);

  return dockable->p->stock_id;
}

GtkWidget *
picman_dockable_get_icon (PicmanDockable *dockable,
                        GtkIconSize   size)
{
  GdkScreen    *screen = gtk_widget_get_screen (GTK_WIDGET (dockable));
  GtkIconTheme *theme  = gtk_icon_theme_get_for_screen (screen);

  if (gtk_icon_theme_has_icon (theme, dockable->p->stock_id))
    {
      return gtk_image_new_from_icon_name (dockable->p->stock_id, size);
    }

  return  gtk_image_new_from_stock (dockable->p->stock_id, size);
}

gboolean
picman_dockable_get_locked (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), FALSE);

  return dockable->p->locked;
}

void
picman_dockable_set_drag_pos (PicmanDockable *dockable,
                            gint          drag_x,
                            gint          drag_y)
{
  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));

  dockable->p->drag_x = drag_x;
  dockable->p->drag_y = drag_y;
}

void
picman_dockable_get_drag_pos (PicmanDockable *dockable,
                            gint         *drag_x,
                            gint         *drag_y)
{
  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));

  if (drag_x != NULL)
    *drag_x = dockable->p->drag_x;
  if (drag_y != NULL)
    *drag_y = dockable->p->drag_y;
}

PicmanPanedBox *
picman_dockable_get_drag_handler (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);

  return dockable->p->drag_handler;
}

void
picman_dockable_set_locked (PicmanDockable *dockable,
                          gboolean      lock)
{
  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));

  if (dockable->p->locked != lock)
    {
      dockable->p->locked = lock ? TRUE : FALSE;

      g_object_notify (G_OBJECT (dockable), "locked");
    }
}

gboolean
picman_dockable_is_locked (PicmanDockable *dockable)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), FALSE);

  return dockable->p->locked;
}


void
picman_dockable_set_tab_style (PicmanDockable *dockable,
                             PicmanTabStyle  tab_style)
{
  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));

  dockable->p->tab_style = picman_dockable_convert_tab_style (dockable, tab_style);

  if (tab_style == PICMAN_TAB_STYLE_AUTOMATIC)
    picman_dockable_set_actual_tab_style (dockable, PICMAN_TAB_STYLE_UNDEFINED);
  else
    picman_dockable_set_actual_tab_style (dockable, tab_style);

  if (dockable->p->dockbook)
    picman_dockbook_update_auto_tab_style (dockable->p->dockbook);
}

/**
 * picman_dockable_set_actual_tab_style:
 * @dockable:
 * @tab_style:
 *
 * Sets actual tab style, meant for those that decides what
 * "automatic" tab style means.
 *
 * Returns: %TRUE if changed, %FALSE otherwise.
 **/
gboolean
picman_dockable_set_actual_tab_style (PicmanDockable *dockable,
                                    PicmanTabStyle  tab_style)
{
  PicmanTabStyle new_tab_style = picman_dockable_convert_tab_style (dockable, tab_style);
  PicmanTabStyle old_tab_style = dockable->p->actual_tab_style;
  
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), FALSE);
  g_return_val_if_fail (tab_style != PICMAN_TAB_STYLE_AUTOMATIC, FALSE);

  dockable->p->actual_tab_style = new_tab_style;

  return new_tab_style != old_tab_style;
}

GtkWidget *
picman_dockable_create_tab_widget (PicmanDockable *dockable,
                                 PicmanContext  *context,
                                 PicmanTabStyle  tab_style,
                                 GtkIconSize   size)
{
  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return picman_dockable_new_tab_widget_internal (dockable, context,
                                                tab_style, size, FALSE);
}

GtkWidget *
picman_dockable_create_drag_widget (PicmanDockable *dockable)
{
  GtkWidget *frame;
  GtkWidget *widget;

  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  widget = picman_dockable_new_tab_widget_internal (dockable,
                                                  dockable->p->context,
                                                  PICMAN_TAB_STYLE_ICON_BLURB,
                                                  GTK_ICON_SIZE_DND,
                                                  TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (widget), 6);
  gtk_container_add (GTK_CONTAINER (frame), widget);
  gtk_widget_show (widget);

  return frame;
}

void
picman_dockable_set_context (PicmanDockable *dockable,
                           PicmanContext  *context)
{
  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  if (context != dockable->p->context)
    {
      GtkWidget *child = gtk_bin_get_child (GTK_BIN (dockable));

      if (child)
        picman_docked_set_context (PICMAN_DOCKED (child), context);

      dockable->p->context = context;
    }
}

PicmanUIManager *
picman_dockable_get_menu (PicmanDockable  *dockable,
                        const gchar  **ui_path,
                        gpointer      *popup_data)
{
  GtkWidget *child;

  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);
  g_return_val_if_fail (ui_path != NULL, NULL);
  g_return_val_if_fail (popup_data != NULL, NULL);

  child = gtk_bin_get_child (GTK_BIN (dockable));

  if (child)
    return picman_docked_get_menu (PICMAN_DOCKED (child), ui_path, popup_data);

  return NULL;
}

/**
 * picman_dockable_set_drag_handler:
 * @dockable:
 * @handler:
 *
 * Set a drag handler that will be asked if it will handle drag events
 * before the dockable handles the event itself.
 **/
void
picman_dockable_set_drag_handler (PicmanDockable *dockable,
                                PicmanPanedBox *handler)
{
  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));

  dockable->p->drag_handler = handler;
}

void
picman_dockable_detach (PicmanDockable *dockable)
{
  PicmanDockWindow *src_dock_window = NULL;
  PicmanDock       *src_dock        = NULL;
  GtkWidget      *dock            = NULL;
  PicmanDockWindow *dock_window     = NULL;
  GtkWidget      *dockbook        = NULL;

  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));
  g_return_if_fail (PICMAN_IS_DOCKBOOK (dockable->p->dockbook));

  src_dock = picman_dockbook_get_dock (dockable->p->dockbook);
  src_dock_window = picman_dock_window_from_dock (src_dock);

  dock = picman_dock_with_window_new (picman_dialog_factory_get_singleton (),
                                    gtk_widget_get_screen (GTK_WIDGET (dockable)),
                                    FALSE /*toolbox*/);
  dock_window = picman_dock_window_from_dock (PICMAN_DOCK (dock));
  gtk_window_set_position (GTK_WINDOW (dock_window), GTK_WIN_POS_MOUSE);
  if (src_dock_window)
    picman_dock_window_setup (dock_window, src_dock_window);

  dockbook = picman_dockbook_new (global_menu_factory);

  picman_dock_add_book (PICMAN_DOCK (dock), PICMAN_DOCKBOOK (dockbook), 0);

  g_object_ref (dockable);

  picman_dockbook_remove (dockable->p->dockbook, dockable);
  picman_dockbook_add (PICMAN_DOCKBOOK (dockbook), dockable, 0);

  g_object_unref (dockable);

  gtk_widget_show (GTK_WIDGET (dock_window));
  gtk_widget_show (dock);
}

void
picman_dockable_blink (PicmanDockable *dockable)
{
  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));

  if (dockable->p->blink_timeout_id)
    g_source_remove (dockable->p->blink_timeout_id);

  dockable->p->blink_timeout_id =
    g_timeout_add (150, (GSourceFunc) picman_dockable_blink_timeout, dockable);

  picman_highlight_widget (GTK_WIDGET (dockable), TRUE);
}

void
picman_dockable_blink_cancel (PicmanDockable *dockable)
{
  g_return_if_fail (PICMAN_IS_DOCKABLE (dockable));

  if (dockable->p->blink_timeout_id)
    {
      g_source_remove (dockable->p->blink_timeout_id);

      dockable->p->blink_timeout_id = 0;
      dockable->p->blink_counter    = 0;

      picman_highlight_widget (GTK_WIDGET (dockable), FALSE);
    }
}


/*  private functions  */

static GList *
picman_dockable_get_aux_info (PicmanSessionManaged *session_managed)
{
  PicmanDockable *dockable;
  GtkWidget    *child;

  g_return_val_if_fail (PICMAN_IS_DOCKABLE (session_managed), NULL);

  dockable = PICMAN_DOCKABLE (session_managed);

  child = gtk_bin_get_child (GTK_BIN (dockable));

  if (child)
    return picman_docked_get_aux_info (PICMAN_DOCKED (child));

  return NULL;
}

static void
picman_dockable_set_aux_info (PicmanSessionManaged *session_managed,
                            GList              *aux_info)
{
  PicmanDockable *dockable;
  GtkWidget    *child;

  g_return_if_fail (PICMAN_IS_DOCKABLE (session_managed));

  dockable = PICMAN_DOCKABLE (session_managed);

  child = gtk_bin_get_child (GTK_BIN (dockable));

  if (child)
    picman_docked_set_aux_info (PICMAN_DOCKED (child), aux_info);
}

static PicmanTabStyle
picman_dockable_convert_tab_style (PicmanDockable   *dockable,
                                 PicmanTabStyle    tab_style)
{
  GtkWidget *child = gtk_bin_get_child (GTK_BIN (dockable));

  if (child && ! PICMAN_DOCKED_GET_INTERFACE (child)->get_preview)
    tab_style = picman_preview_tab_style_to_icon (tab_style);

  return tab_style;
}

static gboolean
picman_dockable_blink_timeout (PicmanDockable *dockable)
{
  picman_highlight_widget (GTK_WIDGET (dockable),
                         dockable->p->blink_counter % 2 == 1);
  dockable->p->blink_counter++;
  
  if (dockable->p->blink_counter == 3)
    {
      dockable->p->blink_timeout_id = 0;
      dockable->p->blink_counter    = 0;

      return FALSE;
    }

  return TRUE;
}
