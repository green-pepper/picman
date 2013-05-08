/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorbutton.c
 * Copyright (C) 1999-2001 Sven Neumann
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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"

#include "picmanwidgetstypes.h"

#include "picmancolorarea.h"
#include "picmancolorbutton.h"
#include "picmancolornotebook.h"
#include "picmancolorselection.h"
#include "picmandialog.h"
#include "picmanhelpui.h"
#include "picmanstock.h"
#include "picmanwidgets-private.h"
#include "picman3migration.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmancolorbutton
 * @title: PicmanColorButton
 * @short_description: Widget for selecting a color from a simple button.
 * @see_also: #libpicmancolor-picmancolorspace
 *
 * This widget provides a simple button with a preview showing the
 * color.
 *
 * On click a color selection dialog is opened. Additionally the
 * button supports Drag and Drop and has a right-click menu that
 * allows one to choose the color from the current FG or BG color. If
 * the user changes the color, the "color-changed" signal is emitted.
 **/


#define COLOR_SELECTION_KEY "picman-color-selection"
#define RESPONSE_RESET      1

#define TODOUBLE(i) (i / 65535.0)
#define TOUINT16(d) ((guint16) (d * 65535 + 0.5))


#define PICMAN_COLOR_BUTTON_COLOR_FG    "color-button-use-foreground"
#define PICMAN_COLOR_BUTTON_COLOR_BG    "color-button-use-background"
#define PICMAN_COLOR_BUTTON_COLOR_BLACK "color-button-use-black"
#define PICMAN_COLOR_BUTTON_COLOR_WHITE "color-button-use-white"


enum
{
  COLOR_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_TITLE,
  PROP_COLOR,
  PROP_TYPE,
  PROP_UPDATE,
  PROP_AREA_WIDTH,
  PROP_AREA_HEIGHT
};


static void     picman_color_button_class_init   (PicmanColorButtonClass *klass);
static void     picman_color_button_init         (PicmanColorButton      *button,
                                                PicmanColorButtonClass *klass);

static void     picman_color_button_finalize          (GObject         *object);
static void     picman_color_button_dispose           (GObject         *object);
static void     picman_color_button_get_property      (GObject         *object,
                                                     guint            property_id,
                                                     GValue          *value,
                                                     GParamSpec      *pspec);
static void     picman_color_button_set_property      (GObject         *object,
                                                     guint            property_id,
                                                     const GValue    *value,
                                                     GParamSpec      *pspec);

static gboolean picman_color_button_button_press      (GtkWidget       *widget,
                                                     GdkEventButton  *bevent);
static void     picman_color_button_state_changed     (GtkWidget       *widget,
                                                     GtkStateType     prev_state);
static void     picman_color_button_clicked           (GtkButton       *button);
static GType    picman_color_button_get_action_type   (PicmanColorButton *button);

static void     picman_color_button_dialog_response   (GtkWidget       *dialog,
                                                     gint             response_id,
                                                     PicmanColorButton *button);
static void     picman_color_button_use_color         (GtkAction       *action,
                                                     PicmanColorButton *button);
static void     picman_color_button_area_changed      (GtkWidget       *color_area,
                                                     PicmanColorButton *button);
static void     picman_color_button_selection_changed (GtkWidget       *selection,
                                                     PicmanColorButton *button);
static void     picman_color_button_help_func         (const gchar     *help_id,
                                                     gpointer         help_data);


static const GtkActionEntry actions[] =
{
  { "color-button-popup", NULL,
    "Color Button Menu", NULL, NULL,
    NULL
  },

  { PICMAN_COLOR_BUTTON_COLOR_FG, NULL,
    N_("_Foreground Color"), NULL, NULL,
    G_CALLBACK (picman_color_button_use_color)
  },
  { PICMAN_COLOR_BUTTON_COLOR_BG, NULL,
    N_("_Background Color"), NULL, NULL,
    G_CALLBACK (picman_color_button_use_color)
  },
  { PICMAN_COLOR_BUTTON_COLOR_BLACK, NULL,
    N_("Blac_k"), NULL, NULL,
    G_CALLBACK (picman_color_button_use_color)
  },
  { PICMAN_COLOR_BUTTON_COLOR_WHITE, NULL,
    N_("_White"), NULL, NULL,
    G_CALLBACK (picman_color_button_use_color)
  }
};

static guint   picman_color_button_signals[LAST_SIGNAL] = { 0 };

static PicmanButtonClass * parent_class = NULL;


GType
picman_color_button_get_type (void)
{
  static GType button_type = 0;

  if (! button_type)
    {
      const GTypeInfo button_info =
      {
        sizeof (PicmanColorButtonClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) picman_color_button_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (PicmanColorButton),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) picman_color_button_init,
      };

      button_type = g_type_register_static (PICMAN_TYPE_BUTTON,
                                            "PicmanColorButton",
                                            &button_info, 0);
    }

  return button_type;
}

static void
picman_color_button_class_init (PicmanColorButtonClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkButtonClass *button_class = GTK_BUTTON_CLASS (klass);
  PicmanRGB         color;

  parent_class = g_type_class_peek_parent (klass);

  picman_color_button_signals[COLOR_CHANGED] =
    g_signal_new ("color-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorButtonClass, color_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->finalize           = picman_color_button_finalize;
  object_class->dispose            = picman_color_button_dispose;
  object_class->get_property       = picman_color_button_get_property;
  object_class->set_property       = picman_color_button_set_property;

  widget_class->button_press_event = picman_color_button_button_press;
  widget_class->state_changed      = picman_color_button_state_changed;

  button_class->clicked            = picman_color_button_clicked;

  klass->color_changed             = NULL;
  klass->get_action_type           = picman_color_button_get_action_type;

  picman_rgba_set (&color, 0.0, 0.0, 0.0, 1.0);

  /**
   * PicmanColorButton:title:
   *
   * The title to be used for the color selection dialog.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_TITLE,
                                   g_param_spec_string ("title", NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  /**
   * PicmanColorButton:color:
   *
   * The color displayed in the button's color area.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_COLOR,
                                   picman_param_spec_rgb ("color", NULL, NULL,
                                                        TRUE, &color,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  /**
   * PicmanColorButton:type:
   *
   * The type of the button's color area.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_TYPE,
                                   g_param_spec_enum ("type", NULL, NULL,
                                                      PICMAN_TYPE_COLOR_AREA_TYPE,
                                                      PICMAN_COLOR_AREA_FLAT,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));
  /**
   * PicmanColorButton:continuous-update:
   *
   * The update policy of the color button.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_UPDATE,
                                   g_param_spec_boolean ("continuous-update",
                                                         NULL, NULL,
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  /**
   * PicmanColorButton:area-width:
   *
   * The minimum width of the button's #PicmanColorArea.
   *
   * Since: PICMAN 2.8
   */
  g_object_class_install_property (object_class, PROP_AREA_WIDTH,
                                   g_param_spec_int ("area-width",
                                                     NULL, NULL,
                                                     1, G_MAXINT, 16,
                                                     G_PARAM_WRITABLE |
                                                     G_PARAM_CONSTRUCT));

  /**
   * PicmanColorButton:area-height:
   *
   * The minimum height of the button's #PicmanColorArea.
   *
   * Since: PICMAN 2.8
   */
  g_object_class_install_property (object_class, PROP_AREA_HEIGHT,
                                   g_param_spec_int ("area-height",
                                                     NULL, NULL,
                                                     1, G_MAXINT, 16,
                                                     G_PARAM_WRITABLE |
                                                     G_PARAM_CONSTRUCT));
}

static void
picman_color_button_init (PicmanColorButton      *button,
                        PicmanColorButtonClass *klass)
{
  GtkActionGroup *group;
  GtkUIManager   *ui_manager;
  gint            i;

  button->title  = NULL;
  button->dialog = NULL;

  button->color_area = g_object_new (PICMAN_TYPE_COLOR_AREA,
                                     "drag-mask", GDK_BUTTON1_MASK,
                                     NULL);

  g_signal_connect (button->color_area, "color-changed",
                    G_CALLBACK (picman_color_button_area_changed),
                    button);

  gtk_container_add (GTK_CONTAINER (button), button->color_area);
  gtk_widget_show (button->color_area);

  /* right-click opens a popup */
  button->popup_menu = ui_manager = gtk_ui_manager_new ();

  group = gtk_action_group_new ("color-button");

  for (i = 0; i < G_N_ELEMENTS (actions); i++)
    {
      const gchar *label   = gettext (actions[i].label);
      const gchar *tooltip = gettext (actions[i].tooltip);
      GtkAction   *action;

      action = g_object_new (klass->get_action_type (button),
                             "name",     actions[i].name,
                             "label",    label,
                             "tooltip",  tooltip,
                             "stock-id", actions[i].stock_id,
                             NULL);

      if (actions[i].callback)
        g_signal_connect (action, "activate",
                          actions[i].callback,
                          button);

      gtk_action_group_add_action_with_accel (group, action,
                                              actions[i].accelerator);

      g_object_unref (action);
    }

  gtk_ui_manager_insert_action_group (ui_manager, group, -1);
  g_object_unref (group);

  gtk_ui_manager_add_ui_from_string
    (ui_manager,
     "<ui>\n"
     "  <popup action=\"color-button-popup\">\n"
     "    <menuitem action=\"" PICMAN_COLOR_BUTTON_COLOR_FG "\" />\n"
     "    <menuitem action=\"" PICMAN_COLOR_BUTTON_COLOR_BG "\" />\n"
     "    <separator />\n"
     "    <menuitem action=\"" PICMAN_COLOR_BUTTON_COLOR_BLACK "\" />\n"
     "    <menuitem action=\"" PICMAN_COLOR_BUTTON_COLOR_WHITE "\" />\n"
     "  </popup>\n"
     "</ui>\n",
     -1, NULL);
}

static void
picman_color_button_finalize (GObject *object)
{
  PicmanColorButton *button = PICMAN_COLOR_BUTTON (object);

  if (button->title)
    {
      g_free (button->title);
      button->title = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_color_button_dispose (GObject *object)
{
  PicmanColorButton *button = PICMAN_COLOR_BUTTON (object);

  if (button->dialog)
    {
      gtk_widget_destroy (button->dialog);
      button->dialog = NULL;
    }

  if (button->color_area)
    {
      gtk_widget_destroy (button->color_area);
      button->color_area = NULL;
    }

  if (button->popup_menu)
    {
      g_object_unref (button->popup_menu);
      button->popup_menu = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_color_button_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanColorButton *button = PICMAN_COLOR_BUTTON (object);

  switch (property_id)
    {
    case PROP_TITLE:
      g_value_set_string (value, button->title);
      break;

    case PROP_COLOR:
      g_object_get_property (G_OBJECT (button->color_area), "color", value);
      break;

    case PROP_TYPE:
      g_object_get_property (G_OBJECT (button->color_area), "type", value);
      break;

    case PROP_UPDATE:
      g_value_set_boolean (value, button->continuous_update);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_button_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanColorButton *button = PICMAN_COLOR_BUTTON (object);
  gint             other;

  switch (property_id)
    {
    case PROP_TITLE:
      g_free (button->title);
      button->title = g_value_dup_string (value);
      break;

    case PROP_COLOR:
      g_object_set_property (G_OBJECT (button->color_area), "color", value);
      break;

    case PROP_TYPE:
      g_object_set_property (G_OBJECT (button->color_area), "type", value);
      break;

    case PROP_UPDATE:
      picman_color_button_set_update (button, g_value_get_boolean (value));
      break;

    case PROP_AREA_WIDTH:
      gtk_widget_get_size_request (button->color_area, NULL, &other);
      gtk_widget_set_size_request (button->color_area,
                                   g_value_get_int (value), other);
      break;

    case PROP_AREA_HEIGHT:
      gtk_widget_get_size_request (button->color_area, &other, NULL);
      gtk_widget_set_size_request (button->color_area,
                                   other, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_color_button_button_press (GtkWidget      *widget,
                                GdkEventButton *bevent)
{
  PicmanColorButton *button = PICMAN_COLOR_BUTTON (widget);

  if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
    {
      GtkWidget *menu = gtk_ui_manager_get_widget (button->popup_menu,
                                                   "/color-button-popup");

      gtk_menu_set_screen (GTK_MENU (menu), gtk_widget_get_screen (widget));

      gtk_menu_popup (GTK_MENU (menu),
                      NULL, NULL, NULL, NULL,
                      bevent->button, bevent->time);
    }

  if (GTK_WIDGET_CLASS (parent_class)->button_press_event)
    return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, bevent);

  return FALSE;
}

static void
picman_color_button_state_changed (GtkWidget    *widget,
                                 GtkStateType  prev_state)
{
  g_return_if_fail (PICMAN_IS_COLOR_BUTTON (widget));

  if (! gtk_widget_is_sensitive (widget) && PICMAN_COLOR_BUTTON (widget)->dialog)
    gtk_widget_hide (PICMAN_COLOR_BUTTON (widget)->dialog);

  if (GTK_WIDGET_CLASS (parent_class)->state_changed)
    GTK_WIDGET_CLASS (parent_class)->state_changed (widget, prev_state);
}

static void
picman_color_button_clicked (GtkButton *button)
{
  PicmanColorButton *color_button = PICMAN_COLOR_BUTTON (button);

  if (! color_button->dialog)
    {
      GtkWidget *dialog;
      GtkWidget *selection;
      PicmanRGB    color;

      picman_color_button_get_color (color_button, &color);

      dialog = color_button->dialog =
        picman_dialog_new (color_button->title, COLOR_SELECTION_KEY,
                         gtk_widget_get_toplevel (GTK_WIDGET (button)), 0,
                         picman_color_button_help_func, NULL,

                         PICMAN_STOCK_RESET, RESPONSE_RESET,
                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                         GTK_STOCK_OK,     GTK_RESPONSE_OK,

                         NULL);

      gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                               RESPONSE_RESET,
                                               GTK_RESPONSE_OK,
                                               GTK_RESPONSE_CANCEL,
                                               -1);

      g_signal_connect (dialog, "response",
                        G_CALLBACK (picman_color_button_dialog_response),
                        color_button);
      g_signal_connect (dialog, "destroy",
                        G_CALLBACK (gtk_widget_destroyed),
                        &color_button->dialog);

      selection = picman_color_selection_new ();
      gtk_container_set_border_width (GTK_CONTAINER (selection), 6);
      picman_color_selection_set_show_alpha (PICMAN_COLOR_SELECTION (selection),
                                           picman_color_button_has_alpha (color_button));
      picman_color_selection_set_color (PICMAN_COLOR_SELECTION (selection), &color);
      picman_color_selection_set_old_color (PICMAN_COLOR_SELECTION (selection),
                                          &color);
      gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                          selection, TRUE, TRUE, 0);
      gtk_widget_show (selection);

      g_signal_connect (selection, "color-changed",
                        G_CALLBACK (picman_color_button_selection_changed),
                        button);

      g_object_set_data (G_OBJECT (color_button->dialog), COLOR_SELECTION_KEY,
                         selection);
    }

  gtk_window_present (GTK_WINDOW (color_button->dialog));
}

static GType
picman_color_button_get_action_type (PicmanColorButton *button)
{
  return GTK_TYPE_ACTION;
}


/*  public functions  */

/**
 * picman_color_button_new:
 * @title:  String that will be used as title for the color_selector.
 * @width:  Width of the colorpreview in pixels.
 * @height: Height of the colorpreview in pixels.
 * @color:  A pointer to a #PicmanRGB color.
 * @type:   The type of transparency to be displayed.
 *
 * Creates a new #PicmanColorButton widget.
 *
 * This returns a button with a preview showing the color.
 * When the button is clicked a GtkColorSelectionDialog is opened.
 * If the user changes the color the new color is written into the
 * array that was used to pass the initial color and the "color-changed"
 * signal is emitted.
 *
 * Returns: Pointer to the new #PicmanColorButton widget.
 **/
GtkWidget *
picman_color_button_new (const gchar       *title,
                       gint               width,
                       gint               height,
                       const PicmanRGB     *color,
                       PicmanColorAreaType  type)
{
  g_return_val_if_fail (color != NULL, NULL);
  g_return_val_if_fail (width > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);

  return g_object_new (PICMAN_TYPE_COLOR_BUTTON,
                       "title",       title,
                       "type",        type,
                       "color",       color,
                       "area-width",  width,
                       "area-height", height,
                       NULL);
}

/**
 * picman_color_button_set_color:
 * @button: Pointer to a #PicmanColorButton.
 * @color:  Pointer to the new #PicmanRGB color.
 *
 * Sets the @button to the given @color.
 **/
void
picman_color_button_set_color (PicmanColorButton *button,
                             const PicmanRGB   *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_BUTTON (button));
  g_return_if_fail (color != NULL);

  picman_color_area_set_color (PICMAN_COLOR_AREA (button->color_area), color);

  g_object_notify (G_OBJECT (button), "color");
}

/**
 * picman_color_button_get_color:
 * @button: Pointer to a #PicmanColorButton.
 * @color:  Pointer to a #PicmanRGB struct used to return the color.
 *
 * Retrieves the currently set color from the @button.
 **/
void
picman_color_button_get_color (PicmanColorButton *button,
                             PicmanRGB         *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_BUTTON (button));
  g_return_if_fail (color != NULL);

  picman_color_area_get_color (PICMAN_COLOR_AREA (button->color_area), color);
}

/**
 * picman_color_button_has_alpha:
 * @button: Pointer to a #PicmanColorButton.
 *
 * Checks whether the @buttons shows transparency information.
 *
 * Returns: %TRUE if the @button shows transparency information, %FALSE
 * otherwise.
 **/
gboolean
picman_color_button_has_alpha (PicmanColorButton *button)
{
  g_return_val_if_fail (PICMAN_IS_COLOR_BUTTON (button), FALSE);

  return picman_color_area_has_alpha (PICMAN_COLOR_AREA (button->color_area));
}

/**
 * picman_color_button_set_type:
 * @button: Pointer to a #PicmanColorButton.
 * @type: the new #PicmanColorAreaType
 *
 * Sets the @button to the given @type. See also picman_color_area_set_type().
 **/
void
picman_color_button_set_type (PicmanColorButton   *button,
                            PicmanColorAreaType  type)
{
  g_return_if_fail (PICMAN_IS_COLOR_BUTTON (button));

  picman_color_area_set_type (PICMAN_COLOR_AREA (button->color_area), type);

  g_object_notify (G_OBJECT (button), "type");
}

/**
 * picman_color_button_get_update:
 * @button: A #PicmanColorButton widget.
 *
 * Returns the color button's @continuous_update property.
 *
 * Return value: the @continuous_update property.
 **/
gboolean
picman_color_button_get_update (PicmanColorButton *button)
{
  g_return_val_if_fail (PICMAN_IS_COLOR_BUTTON (button), FALSE);

  return button->continuous_update;
}

/**
 * picman_color_button_set_update:
 * @button:     A #PicmanColorButton widget.
 * @continuous: The new setting of the @continuous_update property.
 *
 * When set to #TRUE, the @button will emit the "color-changed"
 * continuously while the color is changed in the color selection
 * dialog.
 **/
void
picman_color_button_set_update (PicmanColorButton *button,
                              gboolean         continuous)
{
  g_return_if_fail (PICMAN_IS_COLOR_BUTTON (button));

  if (continuous != button->continuous_update)
    {
      button->continuous_update = continuous ? TRUE : FALSE;

      if (button->dialog)
        {
          PicmanColorSelection *selection;
          PicmanRGB             color;

          selection = g_object_get_data (G_OBJECT (button->dialog),
                                         COLOR_SELECTION_KEY);

          if (button->continuous_update)
            {
              picman_color_selection_get_color (selection, &color);
              picman_color_button_set_color (button, &color);
            }
          else
            {
              picman_color_selection_get_old_color (selection, &color);
              picman_color_button_set_color (button, &color);
            }
        }

      g_object_notify (G_OBJECT (button), "continuous-update");
    }
}


/*  private functions  */

static void
picman_color_button_dialog_response (GtkWidget       *dialog,
                                   gint             response_id,
                                   PicmanColorButton *button)
{
  GtkWidget *selection;
  PicmanRGB    color;

  selection = g_object_get_data (G_OBJECT (dialog), COLOR_SELECTION_KEY);

  switch (response_id)
    {
    case RESPONSE_RESET:
      picman_color_selection_reset (PICMAN_COLOR_SELECTION (selection));
      break;

    case GTK_RESPONSE_OK:
      if (! button->continuous_update)
        {
          picman_color_selection_get_color (PICMAN_COLOR_SELECTION (selection),
                                          &color);
          picman_color_button_set_color (button, &color);
        }

      gtk_widget_hide (dialog);
      break;

    default:
      if (button->continuous_update)
        {
          picman_color_selection_get_old_color (PICMAN_COLOR_SELECTION (selection),
                                              &color);
          picman_color_button_set_color (button, &color);
        }

      gtk_widget_hide (dialog);
      break;
    }
}

static void
picman_color_button_use_color (GtkAction       *action,
                             PicmanColorButton *button)
{
  const gchar *name;
  PicmanRGB      color;

  name = gtk_action_get_name (action);
  picman_color_button_get_color (button, &color);

  if (! strcmp (name, PICMAN_COLOR_BUTTON_COLOR_FG))
    {
      if (_picman_get_foreground_func)
        _picman_get_foreground_func (&color);
      else
        picman_rgba_set (&color, 0.0, 0.0, 0.0, 1.0);
    }
  else if (! strcmp (name, PICMAN_COLOR_BUTTON_COLOR_BG))
    {
      if (_picman_get_background_func)
        _picman_get_background_func (&color);
      else
        picman_rgba_set (&color, 1.0, 1.0, 1.0, 1.0);
    }
  else if (! strcmp (name, PICMAN_COLOR_BUTTON_COLOR_BLACK))
    {
      picman_rgba_set (&color, 0.0, 0.0, 0.0, 1.0);
    }
  else if (! strcmp (name, PICMAN_COLOR_BUTTON_COLOR_WHITE))
    {
      picman_rgba_set (&color, 1.0, 1.0, 1.0, 1.0);
    }

  picman_color_button_set_color (button, &color);
}

static void
picman_color_button_area_changed (GtkWidget       *color_area,
                                PicmanColorButton *button)
{
  if (button->dialog)
    {
      PicmanColorSelection *selection;
      PicmanRGB             color;

      selection = g_object_get_data (G_OBJECT (button->dialog),
                                     COLOR_SELECTION_KEY);

      picman_color_button_get_color (button, &color);

      g_signal_handlers_block_by_func (selection,
                                       picman_color_button_selection_changed,
                                       button);

      picman_color_selection_set_color (selection, &color);

      g_signal_handlers_unblock_by_func (selection,
                                         picman_color_button_selection_changed,
                                         button);
    }

  g_signal_emit (button, picman_color_button_signals[COLOR_CHANGED], 0);
}

static void
picman_color_button_selection_changed (GtkWidget       *selection,
                                     PicmanColorButton *button)
{
  if (button->continuous_update)
    {
      PicmanRGB color;

      picman_color_selection_get_color (PICMAN_COLOR_SELECTION (selection), &color);

      g_signal_handlers_block_by_func (button->color_area,
                                       picman_color_button_area_changed,
                                       button);

      picman_color_area_set_color (PICMAN_COLOR_AREA (button->color_area), &color);

      g_signal_handlers_unblock_by_func (button->color_area,
                                         picman_color_button_area_changed,
                                         button);

      g_signal_emit (button, picman_color_button_signals[COLOR_CHANGED], 0);
    }
}

static void
picman_color_button_help_func (const gchar *help_id,
                             gpointer     help_data)
{
  PicmanColorSelection *selection;
  PicmanColorNotebook  *notebook;

  selection = g_object_get_data (G_OBJECT (help_data), COLOR_SELECTION_KEY);

  notebook = PICMAN_COLOR_NOTEBOOK (selection->notebook);

  help_id = PICMAN_COLOR_SELECTOR_GET_CLASS (notebook->cur_page)->help_id;

  picman_standard_help_func (help_id, NULL);
}
