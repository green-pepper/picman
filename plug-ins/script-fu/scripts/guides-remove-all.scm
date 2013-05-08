;; -*-scheme-*-

(define (script-fu-guides-remove image drawable)
  (let* ((guide-id 0))
    (picman-image-undo-group-start image)

    (set! guide-id (car (picman-image-find-next-guide image 0)))
    (while (> guide-id 0)
      (picman-image-delete-guide image guide-id)
      (set! guide-id (car (picman-image-find-next-guide image 0)))
    )

    (picman-image-undo-group-end image)
    (picman-displays-flush)
  )
)

(script-fu-register "script-fu-guides-remove"
  _"_Remove all Guides"
  _"Remove all horizontal and vertical guides"
  "Alan Horkan"
  "Alan Horkan, 2004. Public Domain."
  "April 2004"
  "*"
  SF-IMAGE    "Image"    0
  SF-DRAWABLE "Drawable" 0
)

(script-fu-menu-register "script-fu-guides-remove"
                         "<Image>/Image/Guides")
