/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancontroller.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PICMAN_ENABLE_CONTROLLER_UNDER_CONSTRUCTION
#error PicmanController is unstable API under construction
#endif

#ifndef __PICMAN_CONTROLLER_H__
#define __PICMAN_CONTROLLER_H__

G_BEGIN_DECLS

/* For information look at the html documentation */


/**
 * PicmanControllerEventType:
 * @PICMAN_CONTROLLER_EVENT_TRIGGER: the event is a simple trigger
 * @PICMAN_CONTROLLER_EVENT_VALUE:   the event carries a double value
 *
 * Event types for #PicmanController.
 **/
typedef enum
{
  PICMAN_CONTROLLER_EVENT_TRIGGER,
  PICMAN_CONTROLLER_EVENT_VALUE
} PicmanControllerEventType;


typedef struct _PicmanControllerEventAny     PicmanControllerEventAny;
typedef struct _PicmanControllerEventTrigger PicmanControllerEventTrigger;
typedef struct _PicmanControllerEventValue   PicmanControllerEventValue;
typedef union  _PicmanControllerEvent        PicmanControllerEvent;

struct _PicmanControllerEventAny
{
  PicmanControllerEventType  type;
  PicmanController          *source;
  gint                     event_id;
};

struct _PicmanControllerEventTrigger
{
  PicmanControllerEventType  type;
  PicmanController          *source;
  gint                     event_id;
};

struct _PicmanControllerEventValue
{
  PicmanControllerEventType  type;
  PicmanController          *source;
  gint                     event_id;
  GValue                   value;
};

union _PicmanControllerEvent
{
  PicmanControllerEventType    type;
  PicmanControllerEventAny     any;
  PicmanControllerEventTrigger trigger;
  PicmanControllerEventValue   value;
};


#define PICMAN_TYPE_CONTROLLER            (picman_controller_get_type ())
#define PICMAN_CONTROLLER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTROLLER, PicmanController))
#define PICMAN_CONTROLLER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTROLLER, PicmanControllerClass))
#define PICMAN_IS_CONTROLLER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTROLLER))
#define PICMAN_IS_CONTROLLER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTROLLER))
#define PICMAN_CONTROLLER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTROLLER, PicmanControllerClass))


typedef struct _PicmanControllerClass PicmanControllerClass;

struct _PicmanController
{
  GObject   parent_instance;

  gchar    *name;
  gchar    *state;
};

struct _PicmanControllerClass
{
  GObjectClass  parent_class;

  const gchar  *name;
  const gchar  *help_domain;
  const gchar  *help_id;

  /*  virtual functions  */
  gint          (* get_n_events)    (PicmanController            *controller);
  const gchar * (* get_event_name)  (PicmanController            *controller,
                                     gint                       event_id);
  const gchar * (* get_event_blurb) (PicmanController            *controller,
                                     gint                       event_id);

  /*  signals  */
  gboolean      (* event)           (PicmanController            *controller,
                                     const PicmanControllerEvent *event);

  const gchar  *stock_id;

  /* Padding for future expansion */
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType            picman_controller_get_type        (void) G_GNUC_CONST;
PicmanController * picman_controller_new             (GType           controller_type);

gint             picman_controller_get_n_events    (PicmanController *controller);
const gchar    * picman_controller_get_event_name  (PicmanController *controller,
                                                  gint            event_id);
const gchar    * picman_controller_get_event_blurb (PicmanController *controller,
                                                  gint            event_id);


/*  protected  */

gboolean         picman_controller_event (PicmanController            *controller,
                                        const PicmanControllerEvent *event);


G_END_DECLS

#endif /* __PICMAN_CONTROLLER_H__ */
