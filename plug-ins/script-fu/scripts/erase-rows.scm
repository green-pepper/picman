(define (script-fu-erase-rows img drawable orientation which type)
  (let* (
        (width (car (picman-drawable-width drawable)))
        (height (car (picman-drawable-height drawable)))
        (position-x (car (picman-drawable-offsets drawable)))
        (position-y (cadr (picman-drawable-offsets drawable)))
        )

    (picman-context-push)
    (picman-context-set-feather FALSE)

    (picman-image-undo-group-start img)
    (letrec ((loop (lambda (i max)
                     (if (< i max)
                         (begin
                           (if (= orientation 0)
                               (picman-image-select-rectangle img CHANNEL-OP-REPLACE position-x (+ i position-y) width 1)
                               (picman-image-select-rectangle img CHANNEL-OP-REPLACE (+ i position-x) position-y 1 height))
                           (if (= type 0)
                               (picman-edit-clear drawable)
                               (picman-edit-fill drawable BACKGROUND-FILL))
                           (loop (+ i 2) max))))))
      (loop (if (= which 0)
                0
                1)
            (if (= orientation 0)
                height
                width)
      )
    )
    (picman-selection-none img)
    (picman-image-undo-group-end img)
    (picman-context-pop)
    (picman-displays-flush)
  )
)

(script-fu-register "script-fu-erase-rows"
  _"_Erase Every Other Row..."
  _"Erase every other row or column"
  "Federico Mena Quintero"
  "Federico Mena Quintero"
  "June 1997"
  "RGB* GRAY* INDEXED*"
  SF-IMAGE    "Image"      0
  SF-DRAWABLE "Drawable"   0
  SF-OPTION  _"Rows/cols"  '(_"Rows" _"Columns")
  SF-OPTION  _"Even/odd"   '(_"Even" _"Odd")
  SF-OPTION  _"Erase/fill" '(_"Erase" _"Fill with BG")
)

(script-fu-menu-register "script-fu-erase-rows"
                         "<Image>/Filters/Distorts")
