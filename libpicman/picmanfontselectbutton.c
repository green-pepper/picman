/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanfontselectbutton.c
 * Copyright (C) 2003  Sven Neumann  <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "picman.h"

#include "picmanuitypes.h"
#include "picmanfontselectbutton.h"
#include "picmanuimarshal.h"

#include "libpicman-intl.h"


/**
 * SECTION: picmanfontselectbutton
 * @title: PicmanFontSelectButton
 * @short_description: A button which pops up a font selection dialog.
 *
 * A button which pops up a font selection dialog.
 **/


#define PICMAN_FONT_SELECT_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), PICMAN_TYPE_FONT_SELECT_BUTTON, PicmanFontSelectButtonPrivate))

typedef struct _PicmanFontSelectButtonPrivate PicmanFontSelectButtonPrivate;

struct _PicmanFontSelectButtonPrivate
{
  gchar       *title;

  gchar       *font_name;      /* local copy */

  GtkWidget   *inside;
  GtkWidget   *label;
};

enum
{
  FONT_SET,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_TITLE,
  PROP_FONT_NAME
};


/*  local function prototypes  */

static void   picman_font_select_button_finalize     (GObject      *object);

static void   picman_font_select_button_set_property (GObject      *object,
                                                    guint         property_id,
                                                    const GValue *value,
                                                    GParamSpec   *pspec);
static void   picman_font_select_button_get_property (GObject      *object,
                                                    guint         property_id,
                                                    GValue       *value,
                                                    GParamSpec   *pspec);

static void   picman_font_select_button_clicked  (PicmanFontSelectButton *button);

static void   picman_font_select_button_callback (const gchar *font_name,
                                                gboolean     dialog_closing,
                                                gpointer     user_data);

static void   picman_font_select_drag_data_received (PicmanFontSelectButton *button,
                                                   GdkDragContext       *context,
                                                   gint                  x,
                                                   gint                  y,
                                                   GtkSelectionData     *selection,
                                                   guint                 info,
                                                   guint                 time);

static GtkWidget * picman_font_select_button_create_inside (PicmanFontSelectButton *button);


static const GtkTargetEntry target = { "application/x-picman-font-name", 0 };

static guint font_button_signals[LAST_SIGNAL] = { 0 };


G_DEFINE_TYPE (PicmanFontSelectButton, picman_font_select_button,
               PICMAN_TYPE_SELECT_BUTTON)


static void
picman_font_select_button_class_init (PicmanFontSelectButtonClass *klass)
{
  GObjectClass          *object_class        = G_OBJECT_CLASS (klass);
  PicmanSelectButtonClass *select_button_class = PICMAN_SELECT_BUTTON_CLASS (klass);

  object_class->finalize     = picman_font_select_button_finalize;
  object_class->set_property = picman_font_select_button_set_property;
  object_class->get_property = picman_font_select_button_get_property;

  select_button_class->select_destroy = picman_font_select_destroy;

  klass->font_set = NULL;

  /**
   * PicmanFontSelectButton:title:
   *
   * The title to be used for the font selection popup dialog.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_TITLE,
                                   g_param_spec_string ("title",
                                                        "Title",
                                                        "The title to be used for the font selection popup dialog",
                                                        _("Font Selection"),
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  /**
   * PicmanFontSelectButton:font-name:
   *
   * The name of the currently selected font.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_FONT_NAME,
                                   g_param_spec_string ("font-name",
                                                        "Font name",
                                                        "The name of the currently selected font",
                                                        _("Sans"),
                                                        PICMAN_PARAM_READWRITE));

  /**
   * PicmanFontSelectButton::font-set:
   * @widget: the object which received the signal.
   * @font_name: the name of the currently selected font.
   * @dialog_closing: whether the dialog was closed or not.
   *
   * The ::font-set signal is emitted when the user selects a font.
   *
   * Since: PICMAN 2.4
   */
  font_button_signals[FONT_SET] =
    g_signal_new ("font-set",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanFontSelectButtonClass, font_set),
                  NULL, NULL,
                  _picmanui_marshal_VOID__STRING_BOOLEAN,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_BOOLEAN);

  g_type_class_add_private (object_class,
                            sizeof (PicmanFontSelectButtonPrivate));
}

static void
picman_font_select_button_init (PicmanFontSelectButton *button)
{
  PicmanFontSelectButtonPrivate *priv;

  priv = PICMAN_FONT_SELECT_BUTTON_GET_PRIVATE (button);

  priv->font_name = NULL;

  priv->inside = picman_font_select_button_create_inside (button);
  gtk_container_add (GTK_CONTAINER (button), priv->inside);
}

/**
 * picman_font_select_button_new:
 * @title:     Title of the dialog to use or %NULL to use the default title.
 * @font_name: Initial font name.
 *
 * Creates a new #GtkWidget that completely controls the selection of
 * a font.  This widget is suitable for placement in a table in a
 * plug-in dialog.
 *
 * Returns: A #GtkWidget that you can use in your UI.
 *
 * Since: PICMAN 2.4
 */
GtkWidget *
picman_font_select_button_new (const gchar *title,
                             const gchar *font_name)
{
  GtkWidget *button;

  if (title)
    button = g_object_new (PICMAN_TYPE_FONT_SELECT_BUTTON,
                           "title",     title,
                           "font-name", font_name,
                           NULL);
  else
    button = g_object_new (PICMAN_TYPE_FONT_SELECT_BUTTON,
                           "font-name", font_name,
                           NULL);

  return button;
}

/**
 * picman_font_select_button_get_font:
 * @button: A #PicmanFontSelectButton
 *
 * Retrieves the name of currently selected font.
 *
 * Returns: an internal copy of the font name which must not be freed.
 *
 * Since: PICMAN 2.4
 */
const gchar *
picman_font_select_button_get_font (PicmanFontSelectButton *button)
{
  PicmanFontSelectButtonPrivate *priv;

  g_return_val_if_fail (PICMAN_IS_FONT_SELECT_BUTTON (button), NULL);

  priv = PICMAN_FONT_SELECT_BUTTON_GET_PRIVATE (button);
  return priv->font_name;
}

/**
 * picman_font_select_button_set_font:
 * @button: A #PicmanFontSelectButton
 * @font_name: Font name to set; %NULL means no change.
 *
 * Sets the current font for the font select button.
 *
 * Since: PICMAN 2.4
 */
void
picman_font_select_button_set_font (PicmanFontSelectButton *button,
                                  const gchar          *font_name)
{
  PicmanSelectButton *select_button;

  g_return_if_fail (PICMAN_IS_FONT_SELECT_BUTTON (button));

  select_button = PICMAN_SELECT_BUTTON (button);

  if (select_button->temp_callback)
    picman_fonts_set_popup (select_button->temp_callback, font_name);
  else
    picman_font_select_button_callback (font_name, FALSE, button);
}


/*  private functions  */

static void
picman_font_select_button_finalize (GObject *object)
{
  PicmanFontSelectButtonPrivate *priv;

  priv = PICMAN_FONT_SELECT_BUTTON_GET_PRIVATE (object);

  g_free (priv->font_name);
  priv->font_name = NULL;

  g_free (priv->title);
  priv->title = NULL;

  G_OBJECT_CLASS (picman_font_select_button_parent_class)->finalize (object);
}

static void
picman_font_select_button_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  PicmanFontSelectButton        *button = PICMAN_FONT_SELECT_BUTTON (object);
  PicmanFontSelectButtonPrivate *priv;

  priv = PICMAN_FONT_SELECT_BUTTON_GET_PRIVATE (button);

  switch (property_id)
    {
    case PROP_TITLE:
      priv->title = g_value_dup_string (value);
      break;
    case PROP_FONT_NAME:
      picman_font_select_button_set_font (button,
                                        g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_font_select_button_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  PicmanFontSelectButton        *button = PICMAN_FONT_SELECT_BUTTON (object);
  PicmanFontSelectButtonPrivate *priv;

  priv = PICMAN_FONT_SELECT_BUTTON_GET_PRIVATE (button);

  switch (property_id)
    {
    case PROP_TITLE:
      g_value_set_string (value, priv->title);
      break;
    case PROP_FONT_NAME:
      g_value_set_string (value, priv->font_name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_font_select_button_callback (const gchar *font_name,
                                  gboolean     dialog_closing,
                                  gpointer     user_data)
{
  PicmanFontSelectButton        *button;
  PicmanFontSelectButtonPrivate *priv;
  PicmanSelectButton            *select_button;

  button = PICMAN_FONT_SELECT_BUTTON (user_data);

  priv = PICMAN_FONT_SELECT_BUTTON_GET_PRIVATE (button);
  select_button = PICMAN_SELECT_BUTTON (button);

  g_free (priv->font_name);
  priv->font_name = g_strdup (font_name);

  gtk_label_set_text (GTK_LABEL (priv->label), font_name);

  if (dialog_closing)
    select_button->temp_callback = NULL;

  g_signal_emit (button, font_button_signals[FONT_SET], 0,
                 font_name, dialog_closing);
  g_object_notify (G_OBJECT (button), "font-name");
}

static void
picman_font_select_button_clicked (PicmanFontSelectButton *button)
{
  PicmanFontSelectButtonPrivate *priv;
  PicmanSelectButton            *select_button;

  priv = PICMAN_FONT_SELECT_BUTTON_GET_PRIVATE (button);
  select_button = PICMAN_SELECT_BUTTON (button);

  if (select_button->temp_callback)
    {
      /*  calling picman_fonts_set_popup() raises the dialog  */
      picman_fonts_set_popup (select_button->temp_callback,
                            priv->font_name);
    }
  else
    {
      select_button->temp_callback =
        picman_font_select_new (priv->title, priv->font_name,
                              picman_font_select_button_callback,
                              button);
    }
}

static void
picman_font_select_drag_data_received (PicmanFontSelectButton *button,
                                     GdkDragContext       *context,
                                     gint                  x,
                                     gint                  y,
                                     GtkSelectionData     *selection,
                                     guint                 info,
                                     guint                 time)
{
  gint   length = gtk_selection_data_get_length (selection);
  gchar *str;

  if (gtk_selection_data_get_format (selection) != 8 || length < 1)
    {
      g_warning ("%s: received invalid font data", G_STRFUNC);
      return;
    }

  str = g_strndup ((const gchar *) gtk_selection_data_get_data (selection),
                   length);

  if (g_utf8_validate (str, -1, NULL))
    {
      gint     pid;
      gpointer unused;
      gint     name_offset = 0;

      if (sscanf (str, "%i:%p:%n", &pid, &unused, &name_offset) >= 2 &&
          pid == picman_getpid () && name_offset > 0)
        {
          gchar *name = str + name_offset;

          picman_font_select_button_set_font (button, name);
        }
    }

  g_free (str);
}

static GtkWidget *
picman_font_select_button_create_inside (PicmanFontSelectButton *font_button)
{
  GtkWidget                   *button;
  GtkWidget                   *hbox;
  GtkWidget                   *image;
  PicmanFontSelectButtonPrivate *priv;

  priv = PICMAN_FONT_SELECT_BUTTON_GET_PRIVATE (font_button);

  gtk_widget_push_composite_child ();

  button = gtk_button_new ();

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_container_add (GTK_CONTAINER (button), hbox);

  image = gtk_image_new_from_stock (PICMAN_STOCK_FONT, GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  priv->label = gtk_label_new (priv->font_name);
  gtk_box_pack_start (GTK_BOX (hbox), priv->label, TRUE, TRUE, 4);

  gtk_widget_show_all (button);

  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (picman_font_select_button_clicked),
                            font_button);

  gtk_drag_dest_set (GTK_WIDGET (button),
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_DROP,
                     &target, 1,
                     GDK_ACTION_COPY);

  g_signal_connect_swapped (button, "drag-data-received",
                            G_CALLBACK (picman_font_select_drag_data_received),
                            font_button);

  gtk_widget_pop_composite_child ();

  return button;
}
