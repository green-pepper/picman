; PICMAN - The GNU Image Manipulation Program
; Copyright (C) 1995 Spencer Kimball and Peter Mattis
;
; xach effect script
; Copyright (c) 1997 Adrian Likins
; aklikins@eos.ncsu.edu
;
; based on a idea by Xach Beane <xach@mint.net>
;
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


(define (script-fu-xach-effect image
                               drawable
                               hl-offset-x
                               hl-offset-y
                               hl-color
                               hl-opacity-comp
                               ds-color
                               ds-opacity
                               ds-blur
                               ds-offset-x
                               ds-offset-y
                               keep-selection)
  (let* (
        (ds-blur (max ds-blur 0))
        (ds-opacity (min ds-opacity 100))
        (ds-opacity (max ds-opacity 0))
        (type (car (picman-drawable-type-with-alpha drawable)))
        (image-width (car (picman-image-width image)))
        (hl-opacity (list hl-opacity-comp hl-opacity-comp hl-opacity-comp))
        (image-height (car (picman-image-height image)))
        (active-selection 0)
        (from-selection 0)
        (theLayer 0)
        (hl-layer 0)
        (shadow-layer 0)
        (mask 0)
        )

    (picman-context-push)
    (picman-context-set-defaults)

    (picman-image-undo-group-start image)
    (picman-layer-add-alpha drawable)

    (if (= (car (picman-selection-is-empty image)) TRUE)
        (begin
          (picman-image-select-item image CHANNEL-OP-REPLACE drawable)
          (set! active-selection (car (picman-selection-save image)))
          (set! from-selection FALSE))
        (begin
          (set! from-selection TRUE)
          (set! active-selection (car (picman-selection-save image)))))

    (set! hl-layer (car (picman-layer-new image image-width image-height type _"Highlight" 100 NORMAL-MODE)))
    (picman-image-insert-layer image hl-layer 0 -1)

    (picman-selection-none image)
    (picman-edit-clear hl-layer)
    (picman-image-select-item image CHANNEL-OP-REPLACE active-selection)

    (picman-context-set-background hl-color)
    (picman-edit-fill hl-layer BACKGROUND-FILL)
    (picman-selection-translate image hl-offset-x hl-offset-y)
    (picman-edit-fill hl-layer BACKGROUND-FILL)
    (picman-selection-none image)
    (picman-image-select-item image CHANNEL-OP-REPLACE active-selection)

    (set! mask (car (picman-layer-create-mask hl-layer ADD-WHITE-MASK)))
    (picman-layer-add-mask hl-layer mask)

    (picman-context-set-background hl-opacity)
    (picman-edit-fill mask BACKGROUND-FILL)

    (set! shadow-layer (car (picman-layer-new image
                                            image-width
                                            image-height
                                            type
                                            _"Shadow"
                                            ds-opacity
                                            NORMAL-MODE)))
    (picman-image-insert-layer image shadow-layer 0 -1)
    (picman-selection-none image)
    (picman-edit-clear shadow-layer)
    (picman-image-select-item image CHANNEL-OP-REPLACE active-selection)
    (picman-selection-translate image ds-offset-x ds-offset-y)
    (picman-context-set-background ds-color)
    (picman-edit-fill shadow-layer BACKGROUND-FILL)
    (picman-selection-none image)
    (plug-in-gauss-rle RUN-NONINTERACTIVE image shadow-layer ds-blur TRUE TRUE)
    (picman-image-select-item image CHANNEL-OP-REPLACE active-selection)
    (picman-edit-clear shadow-layer)
    (picman-image-lower-item image shadow-layer)

    (if (= keep-selection FALSE)
        (picman-selection-none image))

    (picman-image-set-active-layer image drawable)
    (picman-image-remove-channel image active-selection)
    (picman-image-undo-group-end image)
    (picman-displays-flush)

    (picman-context-pop)
  )
)

(script-fu-register "script-fu-xach-effect"
  _"_Xach-Effect..."
  _"Add a subtle translucent 3D effect to the selected region (or alpha)"
  "Adrian Likins <adrian@picman.org>"
  "Adrian Likins"
  "9/28/97"
  "RGB* GRAY*"
  SF-IMAGE       "Image"                   0
  SF-DRAWABLE    "Drawable"                0
  SF-ADJUSTMENT _"Highlight X offset"      '(-1 -100 100 1 10 0 1)
  SF-ADJUSTMENT _"Highlight Y offset"      '(-1 -100 100 1 10 0 1)
  SF-COLOR      _"Highlight color"         "white"
  SF-ADJUSTMENT _"Highlight opacity"       '(66 0 255 1 10 0 0)
  SF-COLOR      _"Drop shadow color"       "black"
  SF-ADJUSTMENT _"Drop shadow opacity"     '(100 0 100 1 10 0 0)
  SF-ADJUSTMENT _"Drop shadow blur radius" '(12 0 255 1 10 0 1)
  SF-ADJUSTMENT _"Drop shadow X offset"    '(5 0 255 1 10 0 1)
  SF-ADJUSTMENT _"Drop shadow Y offset"    '(5 0 255 1 10 0 1)
  SF-TOGGLE     _"Keep selection"          TRUE
)

(script-fu-menu-register "script-fu-xach-effect"
                         "<Image>/Filters/Light and Shadow/Shadow")
