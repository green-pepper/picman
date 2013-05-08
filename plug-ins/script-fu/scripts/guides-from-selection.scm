;; -*-scheme-*-

(define (script-fu-guides-from-selection image drawable)
  (let* (
        (boundries (picman-selection-bounds image))
        ;; non-empty INT32 TRUE if there is a selection
        (selection (car boundries))
        (x1 (cadr boundries))
        (y1 (caddr boundries))
        (x2 (cadr (cddr boundries)))
        (y2 (caddr (cddr boundries)))
        )

    ;; need to check for a selection or we get guides right at edges of the image
    (if (= selection TRUE)
      (begin
        (picman-image-undo-group-start image)

        (picman-image-add-vguide image x1)
        (picman-image-add-hguide image y1)
        (picman-image-add-vguide image x2)
        (picman-image-add-hguide image y2)

        (picman-image-undo-group-end image)
        (picman-displays-flush)
      )
    )
  )
)

(script-fu-register "script-fu-guides-from-selection"
  _"New Guides from _Selection"
  _"Draw a grid as specified by the lists of X and Y locations using the current brush"
  "Alan Horkan"
  "Alan Horkan, 2004.  Public Domain."
  "2004-08-13"
  "*"
  SF-IMAGE    "Image"    0
  SF-DRAWABLE "Drawable" 0
)

(script-fu-menu-register "script-fu-guides-from-selection"
                         "<Image>/Image/Guides")
