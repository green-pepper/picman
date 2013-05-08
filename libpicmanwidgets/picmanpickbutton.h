/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpickbutton.h
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
 *
 * based on gtk-2-0/gtk/gtkcolorsel.c
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

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_PICK_BUTTON_H__
#define __PICMAN_PICK_BUTTON_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_PICK_BUTTON            (picman_pick_button_get_type ())
#define PICMAN_PICK_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PICK_BUTTON, PicmanPickButton))
#define PICMAN_PICK_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PICK_BUTTON, PicmanPickButtonClass))
#define PICMAN_IS_PICK_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PICK_BUTTON))
#define PICMAN_IS_PICK_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PICK_BUTTON))
#define PICMAN_PICK_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PICK_BUTTON, PicmanPickButtonClass))


typedef struct _PicmanPickButtonClass PicmanPickButtonClass;

struct _PicmanPickButton
{
  GtkButton   parent_instance;

  /*< private >*/
  GdkCursor  *cursor;
  GtkWidget  *grab_widget;
};

struct _PicmanPickButtonClass
{
  GtkButtonClass  parent_class;

  void (* color_picked) (PicmanPickButton *button,
                         const PicmanRGB  *color);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_pick_button_get_type (void) G_GNUC_CONST;
GtkWidget * picman_pick_button_new      (void);


G_END_DECLS

#endif /* __PICMAN_PICK_BUTTON_H__ */
