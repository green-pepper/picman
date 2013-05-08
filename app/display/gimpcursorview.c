/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancursorview.c
 * Copyright (C) 2005 Michael Natterer <mitch@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-pick-color.h"
#include "core/picmanitem.h"

#include "widgets/picmancolorframe.h"
#include "widgets/picmandocked.h"
#include "widgets/picmanmenufactory.h"
#include "widgets/picmansessioninfo-aux.h"

#include "picmancursorview.h"
#include "picmandisplay.h"
#include "picmandisplayshell.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_SAMPLE_MERGED
};


struct _PicmanCursorViewPriv
{
  PicmanEditor        parent_instance;

  GtkWidget        *coord_hbox;
  GtkWidget        *selection_hbox;
  GtkWidget        *color_hbox;

  GtkWidget        *pixel_x_label;
  GtkWidget        *pixel_y_label;
  GtkWidget        *unit_x_label;
  GtkWidget        *unit_y_label;
  GtkWidget        *selection_x_label;
  GtkWidget        *selection_y_label;
  GtkWidget        *selection_width_label;
  GtkWidget        *selection_height_label;
  GtkWidget        *color_frame_1;
  GtkWidget        *color_frame_2;

  gboolean          sample_merged;

  PicmanContext      *context;
  PicmanDisplayShell *shell;
  PicmanImage        *image;
  PicmanUnit          unit;
};


static void   picman_cursor_view_docked_iface_init     (PicmanDockedInterface *iface);

static void   picman_cursor_view_set_property          (GObject             *object,
                                                      guint                property_id,
                                                      const GValue        *value,
                                                      GParamSpec          *pspec);
static void   picman_cursor_view_get_property          (GObject             *object,
                                                      guint                property_id,
                                                      GValue              *value,
                                                      GParamSpec          *pspec);

static void   picman_cursor_view_style_set             (GtkWidget           *widget,
                                                      GtkStyle            *prev_style);

static void   picman_cursor_view_set_aux_info          (PicmanDocked          *docked,
                                                      GList               *aux_info);
static GList *picman_cursor_view_get_aux_info          (PicmanDocked          *docked);

static void   picman_cursor_view_set_context           (PicmanDocked          *docked,
                                                      PicmanContext         *context);
static void   picman_cursor_view_image_changed         (PicmanCursorView      *view,
                                                      PicmanImage           *image,
                                                      PicmanContext         *context);
static void   picman_cursor_view_mask_changed          (PicmanCursorView      *view,
                                                      PicmanImage           *image);
static void   picman_cursor_view_diplay_changed        (PicmanCursorView      *view,
                                                      PicmanDisplay         *display,
                                                      PicmanContext         *context);
static void   picman_cursor_view_shell_unit_changed    (PicmanCursorView      *view,
                                                      GParamSpec          *pspec,
                                                      PicmanDisplayShell    *shell);
static void   picman_cursor_view_format_as_unit        (PicmanUnit             unit,
                                                      gchar               *output_buf,
                                                      gint                 output_buf_size,
                                                      gdouble              pixel_value,
                                                      gdouble              image_res);
static void   picman_cursor_view_set_label_italic      (GtkWidget           *label,
                                                      gboolean             italic);
static void   picman_cursor_view_update_selection_info (PicmanCursorView      *view,
                                                      PicmanImage           *image,
                                                      PicmanUnit             unit);





G_DEFINE_TYPE_WITH_CODE (PicmanCursorView, picman_cursor_view, PICMAN_TYPE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_cursor_view_docked_iface_init))

#define parent_class picman_cursor_view_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_cursor_view_class_init (PicmanCursorViewClass* klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = picman_cursor_view_get_property;
  object_class->set_property = picman_cursor_view_set_property;

  widget_class->style_set    = picman_cursor_view_style_set;

  g_object_class_install_property (object_class, PROP_SAMPLE_MERGED,
                                   g_param_spec_boolean ("sample-merged",
                                                         NULL, NULL,
                                                         TRUE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (PicmanCursorViewPriv));
}

static void
picman_cursor_view_init (PicmanCursorView *view)
{
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *toggle;
  gint       content_spacing;

  view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view,
                                            PICMAN_TYPE_CURSOR_VIEW,
                                            PicmanCursorViewPriv);

  view->priv->sample_merged = TRUE;
  view->priv->context       = NULL;
  view->priv->shell         = NULL;
  view->priv->image         = NULL;
  view->priv->unit          = PICMAN_UNIT_PIXEL;

  gtk_widget_style_get (GTK_WIDGET (view),
                        "content-spacing", &content_spacing,
                        NULL);


  /* cursor information */

  view->priv->coord_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL,
                                        content_spacing);
  gtk_box_set_homogeneous (GTK_BOX (view->priv->coord_hbox), TRUE);
  gtk_box_pack_start (GTK_BOX (view), view->priv->coord_hbox,
                      FALSE, FALSE, 0);
  gtk_widget_show (view->priv->coord_hbox);

  view->priv->selection_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL,
                                            content_spacing);
  gtk_box_set_homogeneous (GTK_BOX (view->priv->selection_hbox), TRUE);
  gtk_box_pack_start (GTK_BOX (view), view->priv->selection_hbox,
                      FALSE, FALSE, 0);
  gtk_widget_show (view->priv->selection_hbox);


  /* Pixels */

  frame = picman_frame_new (_("Pixels"));
  gtk_box_pack_start (GTK_BOX (view->priv->coord_hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  view->priv->pixel_x_label = gtk_label_new (_("n/a"));
  gtk_misc_set_alignment (GTK_MISC (view->priv->pixel_x_label), 1.0, 0.5);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("X"), 0.5, 0.5,
                             view->priv->pixel_x_label, 1, FALSE);

  view->priv->pixel_y_label = gtk_label_new (_("n/a"));
  gtk_misc_set_alignment (GTK_MISC (view->priv->pixel_y_label), 1.0, 0.5);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("Y"), 0.5, 0.5,
                             view->priv->pixel_y_label, 1, FALSE);


  /* Units */

  frame = picman_frame_new (_("Units"));
  gtk_box_pack_start (GTK_BOX (view->priv->coord_hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  view->priv->unit_x_label = gtk_label_new (_("n/a"));
  gtk_misc_set_alignment (GTK_MISC (view->priv->unit_x_label), 1.0, 0.5);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("X"), 0.5, 0.5,
                             view->priv->unit_x_label, 1, FALSE);

  view->priv->unit_y_label = gtk_label_new (_("n/a"));
  gtk_misc_set_alignment (GTK_MISC (view->priv->unit_y_label), 1.0, 0.5);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("Y"), 0.5, 0.5,
                             view->priv->unit_y_label, 1, FALSE);


  /* Selection Bounding Box */

  frame = picman_frame_new (_("Selection Bounding Box"));
  gtk_box_pack_start (GTK_BOX (view->priv->selection_hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  view->priv->selection_x_label = gtk_label_new (_("n/a"));
  gtk_misc_set_alignment (GTK_MISC (view->priv->selection_x_label), 1.0, 0.5);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("X"), 0.5, 0.5,
                             view->priv->selection_x_label, 1, FALSE);

  view->priv->selection_y_label = gtk_label_new (_("n/a"));
  gtk_misc_set_alignment (GTK_MISC (view->priv->selection_y_label), 1.0, 0.5);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("Y"), 0.5, 0.5,
                             view->priv->selection_y_label, 1, FALSE);

  frame = picman_frame_new ("");
  gtk_box_pack_start (GTK_BOX (view->priv->selection_hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  view->priv->selection_width_label = gtk_label_new (_("n/a"));
  gtk_misc_set_alignment (GTK_MISC (view->priv->selection_width_label), 1.0, 0.5);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             /* Width */
                             _("W"), 0.5, 0.5,
                             view->priv->selection_width_label, 1, FALSE);

  view->priv->selection_height_label = gtk_label_new (_("n/a"));
  gtk_misc_set_alignment (GTK_MISC (view->priv->selection_height_label), 1.0, 0.5);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             /* Height */
                             _("H"), 0.5, 0.5,
                             view->priv->selection_height_label, 1, FALSE);


  /* color information */

  view->priv->color_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL,
                                        content_spacing);
  gtk_box_set_homogeneous (GTK_BOX (view->priv->color_hbox), TRUE);
  gtk_box_pack_start (GTK_BOX (view), view->priv->color_hbox, FALSE, FALSE, 0);
  gtk_widget_show (view->priv->color_hbox);

  view->priv->color_frame_1 = picman_color_frame_new ();
  picman_color_frame_set_mode (PICMAN_COLOR_FRAME (view->priv->color_frame_1),
                             PICMAN_COLOR_FRAME_MODE_PIXEL);
  gtk_box_pack_start (GTK_BOX (view->priv->color_hbox), view->priv->color_frame_1,
                      TRUE, TRUE, 0);
  gtk_widget_show (view->priv->color_frame_1);

  view->priv->color_frame_2 = picman_color_frame_new ();
  picman_color_frame_set_mode (PICMAN_COLOR_FRAME (view->priv->color_frame_2),
                             PICMAN_COLOR_FRAME_MODE_RGB);
  gtk_box_pack_start (GTK_BOX (view->priv->color_hbox), view->priv->color_frame_2,
                      TRUE, TRUE, 0);
  gtk_widget_show (view->priv->color_frame_2);

  /* sample merged toggle */

  toggle = picman_prop_check_button_new (G_OBJECT (view), "sample-merged",
                                       _("_Sample Merged"));
  gtk_box_pack_start (GTK_BOX (view), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);
}

static void
picman_cursor_view_docked_iface_init (PicmanDockedInterface *iface)
{
  parent_docked_iface = g_type_interface_peek_parent (iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  iface->set_aux_info = picman_cursor_view_set_aux_info;
  iface->get_aux_info = picman_cursor_view_get_aux_info;
  iface->set_context  = picman_cursor_view_set_context;
}

static void
picman_cursor_view_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanCursorView *view = PICMAN_CURSOR_VIEW (object);

  switch (property_id)
    {
    case PROP_SAMPLE_MERGED:
      view->priv->sample_merged = g_value_get_boolean (value);
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_cursor_view_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanCursorView *view = PICMAN_CURSOR_VIEW (object);

  switch (property_id)
    {
    case PROP_SAMPLE_MERGED:
      g_value_set_boolean (value, view->priv->sample_merged);
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

#define AUX_INFO_FRAME_1_MODE "frame-1-mode"
#define AUX_INFO_FRAME_2_MODE "frame-2-mode"

static void
picman_cursor_view_set_aux_info (PicmanDocked *docked,
                               GList      *aux_info)
{
  PicmanCursorView *view = PICMAN_CURSOR_VIEW (docked);
  GList          *list;

  parent_docked_iface->set_aux_info (docked, aux_info);

  for (list = aux_info; list; list = g_list_next (list))
    {
      PicmanSessionInfoAux *aux   = list->data;
      GtkWidget          *frame = NULL;

      if (! strcmp (aux->name, AUX_INFO_FRAME_1_MODE))
        frame = view->priv->color_frame_1;
      else if (! strcmp (aux->name, AUX_INFO_FRAME_2_MODE))
        frame = view->priv->color_frame_2;

      if (frame)
        {
          GEnumClass *enum_class;
          GEnumValue *enum_value;

          enum_class = g_type_class_peek (PICMAN_TYPE_COLOR_FRAME_MODE);
          enum_value = g_enum_get_value_by_nick (enum_class, aux->value);

          if (enum_value)
            picman_color_frame_set_mode (PICMAN_COLOR_FRAME (frame),
                                       enum_value->value);
        }
    }
}

static GList *
picman_cursor_view_get_aux_info (PicmanDocked *docked)
{
  PicmanCursorView     *view = PICMAN_CURSOR_VIEW (docked);
  GList              *aux_info;
  const gchar        *nick;
  PicmanSessionInfoAux *aux;

  aux_info = parent_docked_iface->get_aux_info (docked);

  if (picman_enum_get_value (PICMAN_TYPE_COLOR_FRAME_MODE,
                           PICMAN_COLOR_FRAME (view->priv->color_frame_1)->frame_mode,
                           NULL, &nick, NULL, NULL))
    {
      aux = picman_session_info_aux_new (AUX_INFO_FRAME_1_MODE, nick);
      aux_info = g_list_append (aux_info, aux);
    }

  if (picman_enum_get_value (PICMAN_TYPE_COLOR_FRAME_MODE,
                           PICMAN_COLOR_FRAME (view->priv->color_frame_2)->frame_mode,
                           NULL, &nick, NULL, NULL))
    {
      aux = picman_session_info_aux_new (AUX_INFO_FRAME_2_MODE, nick);
      aux_info = g_list_append (aux_info, aux);
    }

  return aux_info;
}

static void
picman_cursor_view_format_as_unit (PicmanUnit  unit,
                                 gchar    *output_buf,
                                 gint      output_buf_size,
                                 gdouble   pixel_value,
                                 gdouble   image_res)
{
  gchar         format_buf[32];
  gdouble       value;
  gint          unit_digits = 0;
  const gchar  *unit_str = "";

  value = picman_pixels_to_units (pixel_value, unit, image_res);

  if (unit != PICMAN_UNIT_PIXEL)
    {
      unit_digits = picman_unit_get_digits (unit);
      unit_str    = picman_unit_get_abbreviation (unit);
    }

  g_snprintf (format_buf, sizeof (format_buf),
              "%%.%df %s", unit_digits, unit_str);

  g_snprintf (output_buf, output_buf_size, format_buf, value);
}

static void
picman_cursor_view_set_label_italic (GtkWidget *label,
                                   gboolean   italic)
{
  PangoAttrType attribute = italic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL;

  picman_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_STYLE, attribute,
                             -1);
}

static void
picman_cursor_view_style_set (GtkWidget *widget,
                            GtkStyle  *prev_style)
{
  PicmanCursorView *view = PICMAN_CURSOR_VIEW (widget);
  gint            content_spacing;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (GTK_WIDGET (view),
                        "content-spacing", &content_spacing,
                        NULL);

  gtk_box_set_spacing (GTK_BOX (view->priv->coord_hbox),     content_spacing);
  gtk_box_set_spacing (GTK_BOX (view->priv->selection_hbox), content_spacing);
  gtk_box_set_spacing (GTK_BOX (view->priv->color_hbox),     content_spacing);
}

static void
picman_cursor_view_set_context (PicmanDocked  *docked,
                              PicmanContext *context)
{
  PicmanCursorView *view    = PICMAN_CURSOR_VIEW (docked);
  PicmanDisplay    *display = NULL;
  PicmanImage      *image   = NULL;

  if (view->priv->context)
    {
      g_signal_handlers_disconnect_by_func (view->priv->context,
                                            picman_cursor_view_diplay_changed,
                                            view);
      g_signal_handlers_disconnect_by_func (view->priv->context,
                                            picman_cursor_view_image_changed,
                                            view);
    }

  view->priv->context = context;

  if (view->priv->context)
    {
      g_signal_connect_swapped (view->priv->context, "display-changed",
                                G_CALLBACK (picman_cursor_view_diplay_changed),
                                view);

      g_signal_connect_swapped (view->priv->context, "image-changed",
                                G_CALLBACK (picman_cursor_view_image_changed),
                                view);

      display = picman_context_get_display (context);
      image   = picman_context_get_image (context);
    }

  picman_cursor_view_diplay_changed (view,
                                   display,
                                   view->priv->context);
  picman_cursor_view_image_changed (view,
                                  image,
                                  view->priv->context);
}

static void
picman_cursor_view_image_changed (PicmanCursorView *view,
                                PicmanImage      *image,
                                PicmanContext    *context)
{
  g_return_if_fail (PICMAN_IS_CURSOR_VIEW (view));

  if (image == view->priv->image)
    return;

  if (view->priv->image)
    {
      g_signal_handlers_disconnect_by_func (view->priv->image,
                                            picman_cursor_view_mask_changed,
                                            view);
    }

  view->priv->image = image;

  if (view->priv->image)
    {
      g_signal_connect_swapped (view->priv->image, "mask-changed",
                                G_CALLBACK (picman_cursor_view_mask_changed),
                                view);
    }

  picman_cursor_view_mask_changed (view, view->priv->image);
}

static void
picman_cursor_view_mask_changed (PicmanCursorView *view,
                               PicmanImage      *image)
{
  picman_cursor_view_update_selection_info (view,
                                          view->priv->image,
                                          view->priv->unit);
}

static void
picman_cursor_view_diplay_changed (PicmanCursorView *view,
                                 PicmanDisplay    *display,
                                 PicmanContext    *context)
{
  PicmanDisplayShell *shell = NULL;

  if (display)
    shell = picman_display_get_shell (display);

  if (view->priv->shell)
    {
      g_signal_handlers_disconnect_by_func (view->priv->shell,
                                            picman_cursor_view_shell_unit_changed,
                                            view);
    }

  view->priv->shell = shell;

  if (view->priv->shell)
    {
      g_signal_connect_swapped (view->priv->shell, "notify::unit",
                                G_CALLBACK (picman_cursor_view_shell_unit_changed),
                                view);
    }

  picman_cursor_view_shell_unit_changed (view,
                                       NULL,
                                       view->priv->shell);
}

static void
picman_cursor_view_shell_unit_changed (PicmanCursorView   *view,
                                     GParamSpec       *pspec,
                                     PicmanDisplayShell *shell)
{
  PicmanUnit new_unit = PICMAN_UNIT_PIXEL;

  if (shell)
    {
      new_unit = picman_display_shell_get_unit (shell);
    }

  if (view->priv->unit != new_unit)
    {
      picman_cursor_view_update_selection_info (view, view->priv->image, new_unit);
      view->priv->unit = new_unit;
    }
}

static void picman_cursor_view_update_selection_info (PicmanCursorView *view,
                                                    PicmanImage      *image,
                                                    PicmanUnit        unit)
{
  gboolean bounds_exist = FALSE;
  gint     x1, y1, x2, y2;

  if (image)
    {
      bounds_exist = picman_channel_bounds (picman_image_get_mask (image), &x1, &y1, &x2, &y2);
    }

  if (bounds_exist)
    {
      gint    width, height;
      gdouble xres, yres;
      gchar   buf[32];

      width  = x2 - x1;
      height = y2 - y1;

      picman_image_get_resolution (image, &xres, &yres);

      picman_cursor_view_format_as_unit (unit, buf, sizeof (buf), x1, xres);
      gtk_label_set_text (GTK_LABEL (view->priv->selection_x_label), buf);

      picman_cursor_view_format_as_unit (unit, buf, sizeof (buf), y1, yres);
      gtk_label_set_text (GTK_LABEL (view->priv->selection_y_label), buf);

      picman_cursor_view_format_as_unit (unit, buf, sizeof (buf), width, xres);
      gtk_label_set_text (GTK_LABEL (view->priv->selection_width_label), buf);

      picman_cursor_view_format_as_unit (unit, buf, sizeof (buf), height, yres);
      gtk_label_set_text (GTK_LABEL (view->priv->selection_height_label), buf);
    }
  else
    {
      gtk_label_set_text (GTK_LABEL (view->priv->selection_x_label),      _("n/a"));
      gtk_label_set_text (GTK_LABEL (view->priv->selection_y_label),      _("n/a"));
      gtk_label_set_text (GTK_LABEL (view->priv->selection_width_label),  _("n/a"));
      gtk_label_set_text (GTK_LABEL (view->priv->selection_height_label), _("n/a"));
    }
}


/*  public functions  */

GtkWidget *
picman_cursor_view_new (PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  return g_object_new (PICMAN_TYPE_CURSOR_VIEW,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<CursorInfo>",
                       "ui-path",         "/cursor-info-popup",
                       NULL);
}

void
picman_cursor_view_set_sample_merged (PicmanCursorView *view,
                                    gboolean        sample_merged)
{
  g_return_if_fail (PICMAN_IS_CURSOR_VIEW (view));

  sample_merged = sample_merged ? TRUE : FALSE;

  if (view->priv->sample_merged != sample_merged)
    {
      view->priv->sample_merged = sample_merged;

      g_object_notify (G_OBJECT (view), "sample-merged");
    }
}

gboolean
picman_cursor_view_get_sample_merged (PicmanCursorView *view)
{
  g_return_val_if_fail (PICMAN_IS_CURSOR_VIEW (view), FALSE);

  return view->priv->sample_merged;
}

void
picman_cursor_view_update_cursor (PicmanCursorView   *view,
                                PicmanImage        *image,
                                PicmanUnit          shell_unit,
                                gdouble           x,
                                gdouble           y)
{
  PicmanUnit      unit = shell_unit;
  gboolean      in_image;
  gchar         buf[32];
  const Babl   *sample_format;
  PicmanRGB       color;
  gint          color_index;
  gdouble       xres;
  gdouble       yres;

  g_return_if_fail (PICMAN_IS_CURSOR_VIEW (view));
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  if (unit == PICMAN_UNIT_PIXEL)
    unit = picman_image_get_unit (image);

  picman_image_get_resolution (image, &xres, &yres);

  in_image = (x >= 0.0 && x < picman_image_get_width  (image) &&
              y >= 0.0 && y < picman_image_get_height (image));

  g_snprintf (buf, sizeof (buf), "%d", (gint) floor (x));
  gtk_label_set_text (GTK_LABEL (view->priv->pixel_x_label), buf);
  picman_cursor_view_set_label_italic (view->priv->pixel_x_label, ! in_image);

  g_snprintf (buf, sizeof (buf), "%d", (gint) floor (y));
  gtk_label_set_text (GTK_LABEL (view->priv->pixel_y_label), buf);
  picman_cursor_view_set_label_italic (view->priv->pixel_y_label, ! in_image);

  picman_cursor_view_format_as_unit (unit, buf, sizeof (buf), x, xres);
  gtk_label_set_text (GTK_LABEL (view->priv->unit_x_label), buf);
  picman_cursor_view_set_label_italic (view->priv->unit_x_label, ! in_image);

  picman_cursor_view_format_as_unit (unit, buf, sizeof (buf), y, yres);
  gtk_label_set_text (GTK_LABEL (view->priv->unit_y_label), buf);
  picman_cursor_view_set_label_italic (view->priv->unit_y_label, ! in_image);

  if (picman_image_pick_color (image, NULL,
                             (gint) floor (x),
                             (gint) floor (y),
                             view->priv->sample_merged,
                             FALSE, 0.0,
                             &sample_format, &color, &color_index))
    {
      picman_color_frame_set_color (PICMAN_COLOR_FRAME (view->priv->color_frame_1),
                                  sample_format, &color, color_index);
      picman_color_frame_set_color (PICMAN_COLOR_FRAME (view->priv->color_frame_2),
                                  sample_format, &color, color_index);
    }
  else
    {
      picman_color_frame_set_invalid (PICMAN_COLOR_FRAME (view->priv->color_frame_1));
      picman_color_frame_set_invalid (PICMAN_COLOR_FRAME (view->priv->color_frame_2));
    }

  /* Show the selection info from the image under the cursor if any */
  picman_cursor_view_update_selection_info (view, image, shell_unit);
}

void
picman_cursor_view_clear_cursor (PicmanCursorView *view)
{
  g_return_if_fail (PICMAN_IS_CURSOR_VIEW (view));

  gtk_label_set_text (GTK_LABEL (view->priv->pixel_x_label), _("n/a"));
  gtk_label_set_text (GTK_LABEL (view->priv->pixel_y_label), _("n/a"));
  gtk_label_set_text (GTK_LABEL (view->priv->unit_x_label),  _("n/a"));
  gtk_label_set_text (GTK_LABEL (view->priv->unit_y_label),  _("n/a"));

  picman_color_frame_set_invalid (PICMAN_COLOR_FRAME (view->priv->color_frame_1));
  picman_color_frame_set_invalid (PICMAN_COLOR_FRAME (view->priv->color_frame_2));

  /* Start showing selection info from the active image again */
  picman_cursor_view_update_selection_info (view,
                                          view->priv->image,
                                          view->priv->unit);
}
