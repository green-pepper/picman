; PICMAN - The GNU Image Manipulation Program
; Copyright (C) 1995 Spencer Kimball and Peter Mattis
;
; Predator effect
; Copyright (c) 1997 Adrian Likins
; aklikins@eos.ncsu.ed
;
;  The idea here is too make the image/selection look sort of like
;  the view the predator had in the movies. ie, kind of a thermogram
;  type of thing. Works best on colorful rgb images.
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.


(define (script-fu-predator image
                            drawable
                            edge-amount
                            pixelize
                            pixel-size
                            keep-selection
                            separate-layer)
  (let* (
        (type (car (picman-drawable-type-with-alpha drawable)))
        (image-width (car (picman-image-width image)))
        (image-height (car (picman-image-height image)))
        (active-selection 0)
        (from-selection 0)
        (selection-bounds 0)
        (select-offset-x 0)
        (select-offset-y 0)
        (select-width 0)
        (select-height 0)
        (effect-layer 0)
        (active-layer 0)
        )

    (picman-context-push)
    (picman-context-set-defaults)
    (picman-image-undo-group-start image)
    (picman-layer-add-alpha drawable)

    (if (= (car (picman-selection-is-empty image)) TRUE)
        (begin
          (picman-image-select-item image CHANNEL-OP-REPLACE drawable)
          (set! active-selection (car (picman-selection-save image)))
          (set! from-selection FALSE)
        )
        (begin
          (set! from-selection TRUE)
          (set! active-selection (car (picman-selection-save image)))
        )
    )

    (set! selection-bounds (picman-selection-bounds image))
    (set! select-offset-x (cadr selection-bounds))
    (set! select-offset-y (caddr selection-bounds))
    (set! select-width (- (cadr (cddr selection-bounds)) select-offset-x))
    (set! select-height (- (caddr (cddr selection-bounds)) select-offset-y))

    (if (= separate-layer TRUE)
        (begin
          (set! effect-layer (car (picman-layer-new image
                                                  select-width
                                                  select-height
                                                  type
                                                  "glow layer"
                                                  100
                                                  NORMAL-MODE))
          )

          (picman-layer-set-offsets effect-layer select-offset-x select-offset-y)
          (picman-image-insert-layer image effect-layer 0 -1)
          (picman-selection-none image)
          (picman-edit-clear effect-layer)

          (picman-image-select-item image CHANNEL-OP-REPLACE active-selection)
          (picman-edit-copy drawable)
          (let ((floating-sel (car (picman-edit-paste effect-layer FALSE))))
            (picman-floating-sel-anchor floating-sel)
          )
          (picman-image-set-active-layer image effect-layer)
        )
        (set! effect-layer drawable)
    )
    (set! active-layer effect-layer)

    ; all the fun stuff goes here
    (if (= pixelize TRUE)
        (plug-in-pixelize RUN-NONINTERACTIVE image active-layer pixel-size)
    )
    (plug-in-max-rgb RUN-NONINTERACTIVE image active-layer 0)
    (plug-in-edge RUN-NONINTERACTIVE image active-layer edge-amount 1 0)

    ; clean up the selection copy
    (picman-image-select-item image CHANNEL-OP-REPLACE active-selection)

    (if (= keep-selection FALSE)
        (picman-selection-none image)
    )

    (picman-image-set-active-layer image drawable)
    (picman-image-remove-channel image active-selection)
    (picman-image-undo-group-end image)
    (picman-displays-flush)
    (picman-context-pop)
  )
)

(script-fu-register "script-fu-predator"
  _"_Predator..."
  _"Add a 'Predator' effect to the selected region (or alpha)"
  "Adrian Likins <adrian@picman.org>"
  "Adrian Likins"
  "10/12/97"
  "RGB*"
  SF-IMAGE       "Image"          0
  SF-DRAWABLE    "Drawable"       0
  SF-ADJUSTMENT _"Edge amount"    '(2 0 24 1 1 0 0)
  SF-TOGGLE     _"Pixelize"       TRUE
  SF-ADJUSTMENT _"Pixel amount"   '(3 1 16 1 1 0 0)
  SF-TOGGLE     _"Keep selection" TRUE
  SF-TOGGLE     _"Separate layer" TRUE
)

(script-fu-menu-register "script-fu-predator"
                         "<Image>/Filters/Artistic")
