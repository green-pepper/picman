/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmessagebox.c
 * Copyright (C) 2004 Sven Neumann <sven@picman.org>
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

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "picmanmessagebox.h"

#include "picman-intl.h"


#define PICMAN_MESSAGE_BOX_SPACING  12

enum
{
  PROP_0,
  PROP_STOCK_ID
};


static void   picman_message_box_constructed   (GObject        *object);
static void   picman_message_box_dispose       (GObject        *object);
static void   picman_message_box_finalize      (GObject        *object);
static void   picman_message_box_set_property  (GObject        *object,
                                              guint           property_id,
                                              const GValue   *value,
                                              GParamSpec     *pspec);
static void   picman_message_box_get_property  (GObject        *object,
                                              guint           property_id,
                                              GValue         *value,
                                              GParamSpec     *pspec);

static void   picman_message_box_forall        (GtkContainer   *container,
                                              gboolean        include_internals,
                                              GtkCallback     callback,
                                              gpointer        callback_data);

static void   picman_message_box_size_request  (GtkWidget      *widget,
                                              GtkRequisition *requisition);
static void   picman_message_box_size_allocate (GtkWidget      *widget,
                                              GtkAllocation  *allocation);


G_DEFINE_TYPE (PicmanMessageBox, picman_message_box, GTK_TYPE_BOX)

#define parent_class picman_message_box_parent_class


static void
picman_message_box_class_init (PicmanMessageBoxClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  GtkWidgetClass    *widget_class    = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->constructed   = picman_message_box_constructed;
  object_class->dispose       = picman_message_box_dispose;
  object_class->finalize      = picman_message_box_finalize;
  object_class->set_property  = picman_message_box_set_property;
  object_class->get_property  = picman_message_box_get_property;


  widget_class->size_request  = picman_message_box_size_request;
  widget_class->size_allocate = picman_message_box_size_allocate;

  container_class->forall     = picman_message_box_forall;

  g_object_class_install_property (object_class, PROP_STOCK_ID,
                                   g_param_spec_string ("stock-id", NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_message_box_init (PicmanMessageBox *box)
{
  gint i;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (box),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_spacing (GTK_BOX (box), 12);
  gtk_container_set_border_width (GTK_CONTAINER (box), 12);

  /*  Unset the focus chain to keep the labels from being in the focus
   *  chain.  Users of PicmanMessageBox that add focusable widgets should
   *  either unset the focus chain or (better) explicitly set one.
   */
  gtk_container_set_focus_chain (GTK_CONTAINER (box), NULL);

  for (i = 0; i < 2; i++)
    {
      GtkWidget *label = g_object_new (GTK_TYPE_LABEL,
                                       "wrap",       TRUE,
                                       "selectable", TRUE,
                                       "xalign",     0.0,
                                       "yalign",     0.5,
                                       NULL);

      if (i == 0)
        picman_label_set_attributes (GTK_LABEL (label),
                                   PANGO_ATTR_SCALE,  PANGO_SCALE_LARGE,
                                   PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                                   -1);

      gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

      box->label[i] = label;
    }

  box->repeat   = 0;
  box->label[2] = NULL;
}

static void
picman_message_box_constructed (GObject *object)
{
  PicmanMessageBox *box = PICMAN_MESSAGE_BOX (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  if (box->stock_id)
    {
      gtk_widget_push_composite_child ();
      box->image = gtk_image_new_from_stock (box->stock_id,
                                             GTK_ICON_SIZE_DIALOG);
      gtk_widget_pop_composite_child ();

      gtk_misc_set_alignment (GTK_MISC (box->image), 0.0, 0.0);
      gtk_widget_set_parent (box->image, GTK_WIDGET (box));
      gtk_widget_show (box->image);
    }
}

static void
picman_message_box_dispose (GObject *object)
{
  PicmanMessageBox *box = PICMAN_MESSAGE_BOX (object);

  if (box->image)
    {
      gtk_widget_unparent (box->image);
      box->image = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_message_box_finalize (GObject *object)
{
  PicmanMessageBox *box = PICMAN_MESSAGE_BOX (object);

  if (box->stock_id)
    {
      g_free (box->stock_id);
      box->stock_id = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_message_box_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanMessageBox *box = PICMAN_MESSAGE_BOX (object);

  switch (property_id)
    {
    case PROP_STOCK_ID:
      box->stock_id = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_message_box_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanMessageBox *box = PICMAN_MESSAGE_BOX (object);

  switch (property_id)
    {
    case PROP_STOCK_ID:
      g_value_set_string (value, box->stock_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_message_box_size_request (GtkWidget      *widget,
                               GtkRequisition *requisition)
{
  PicmanMessageBox *box = PICMAN_MESSAGE_BOX (widget);

  GTK_WIDGET_CLASS (parent_class)->size_request (widget, requisition);

  if (box->image && gtk_widget_get_visible (box->image))
    {
      GtkRequisition  child_requisition;
      gint            border_width;

      gtk_widget_size_request (box->image, &child_requisition);

      border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));

      requisition->width  += child_requisition.width + PICMAN_MESSAGE_BOX_SPACING;
      requisition->height = MAX (requisition->height,
                                 child_requisition.height +
                                 2 * border_width);
    }
}

static void
picman_message_box_size_allocate (GtkWidget     *widget,
                                GtkAllocation *allocation)
{
  PicmanMessageBox *box       = PICMAN_MESSAGE_BOX (widget);
  GtkContainer   *container = GTK_CONTAINER (widget);
  gint            width     = 0;
  gboolean        rtl;

  rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);

  if (box->image && gtk_widget_get_visible (box->image))
    {
      GtkRequisition  child_requisition;
      GtkAllocation   child_allocation;
      gint            border_width;
      gint            height;

      gtk_widget_size_request (box->image, &child_requisition);

      border_width = gtk_container_get_border_width (container);

      width  = MIN (allocation->width - 2 * border_width,
                    child_requisition.width + PICMAN_MESSAGE_BOX_SPACING);
      width  = MAX (1, width);

      height = allocation->height - 2 * border_width;
      height = MAX (1, height);

      if (rtl)
        child_allocation.x  = (allocation->width -
                               border_width      -
                               child_requisition.width);
      else
        child_allocation.x  = allocation->x + border_width;

      child_allocation.y      = allocation->y + border_width;
      child_allocation.width  = width;
      child_allocation.height = height;

      gtk_widget_size_allocate (box->image, &child_allocation);
    }

  allocation->x     += rtl ? 0 : width;
  allocation->width -= width;

  GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

  allocation->x      -= rtl ? 0 : width;
  allocation->width  += width;

  gtk_widget_set_allocation (widget, allocation);
}

static void
picman_message_box_forall (GtkContainer *container,
                         gboolean      include_internals,
                         GtkCallback   callback,
                         gpointer      callback_data)
{
  if (include_internals)
    {
      PicmanMessageBox *box = PICMAN_MESSAGE_BOX (container);

      if (box->image)
        (* callback) (box->image, callback_data);
    }

  GTK_CONTAINER_CLASS (parent_class)->forall (container, include_internals,
                                              callback, callback_data);
}

static void
picman_message_box_set_label_text (PicmanMessageBox *box,
                                 gint            n,
                                 const gchar    *format,
                                 va_list         args)
{
  GtkWidget *label = box->label[n];

  if (format)
    {
      gchar *text = g_strdup_vprintf (format, args);
      gchar *utf8 = picman_any_to_utf8 (text, -1, "Cannot convert text to utf8.");

      gtk_label_set_text (GTK_LABEL (label), utf8);
      gtk_widget_show (label);

      g_free (utf8);
      g_free (text);
    }
  else
    {
      gtk_widget_hide (label);
      gtk_label_set_text (GTK_LABEL (label), NULL);
    }
}

static void
picman_message_box_set_label_markup (PicmanMessageBox *box,
                                   gint            n,
                                   const gchar    *format,
                                   va_list         args)
{
  GtkWidget *label = box->label[n];

  if (format)
    {
      gchar *text = g_markup_vprintf_escaped (format, args);

      gtk_label_set_markup (GTK_LABEL (label), text);
      gtk_widget_show (label);

      g_free (text);
    }
  else
    {
      gtk_widget_hide (label);
      gtk_label_set_text (GTK_LABEL (label), NULL);
    }
}

/*  public functions  */

GtkWidget *
picman_message_box_new (const gchar *stock_id)
{
  return g_object_new (PICMAN_TYPE_MESSAGE_BOX,
                       "stock-id",  stock_id,
                       NULL);
}

void
picman_message_box_set_primary_text (PicmanMessageBox *box,
                                   const gchar    *format,
                                   ...)
{
  va_list args;

  g_return_if_fail (PICMAN_IS_MESSAGE_BOX (box));

  va_start (args, format);
  picman_message_box_set_label_text (box, 0, format, args);
  va_end (args);
}

void
picman_message_box_set_text (PicmanMessageBox *box,
                           const gchar    *format,
                           ...)
{
  va_list args;

  g_return_if_fail (PICMAN_IS_MESSAGE_BOX (box));

  va_start (args, format);
  picman_message_box_set_label_text (box, 1, format, args);
  va_end (args);
}

void
picman_message_box_set_markup (PicmanMessageBox *box,
                             const gchar    *format,
                             ...)
{
  va_list args;

  g_return_if_fail (PICMAN_IS_MESSAGE_BOX (box));

  va_start (args, format);
  picman_message_box_set_label_markup (box, 1,format, args);
  va_end (args);
}

gint
picman_message_box_repeat (PicmanMessageBox *box)
{
  gchar *message;

  g_return_val_if_fail (PICMAN_IS_MESSAGE_BOX (box), 0);

  box->repeat++;

  message = g_strdup_printf (ngettext ("Message repeated once.",
                                       "Message repeated %d times.",
                                       box->repeat),
                             box->repeat);

  if (box->label[2])
    {
      gtk_label_set_text (GTK_LABEL (box->label[2]), message);
    }
  else
    {
      GtkWidget *label = box->label[2] = gtk_label_new (message);

      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 1.0);
      picman_label_set_attributes (GTK_LABEL (label),
                                 PANGO_ATTR_STYLE, PANGO_STYLE_OBLIQUE,
                                 -1);
      gtk_box_pack_end (GTK_BOX (box), label, FALSE, FALSE, 0);
      gtk_widget_show (label);
    }

  g_free (message);

  return box->repeat;
}
