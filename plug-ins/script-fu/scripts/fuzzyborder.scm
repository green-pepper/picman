;
; fuzzy-border
;
; Do a cool fade to a given colour at the border of an image (optional shadow)
; Will make image RGB if it isn't already.
;
; Chris Gutteridge (cjg@ecs.soton.ac.uk)
; At ECS Dept, University of Southampton, England.

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

; Define the function:

(define (script-fu-fuzzy-border inImage
                                inLayer
                                inColor
                                inSize
                                inBlur
                                inGranu
                                inShadow
                                inShadWeight
                                inCopy
                                inFlatten
        )

  (define (chris-color-edge inImage inLayer inColor inSize)
    (picman-selection-all inImage)
    (picman-selection-shrink inImage inSize)
    (picman-selection-invert inImage)
    (picman-context-set-background inColor)
    (picman-edit-fill inLayer BACKGROUND-FILL)
    (picman-selection-none inImage)
  )

  (let (
       (theWidth (car (picman-image-width inImage)))
       (theHeight (car (picman-image-height inImage)))
       (theImage 0)
       (theLayer 0)
       )

    (picman-context-push)
    (picman-context-set-defaults)

    (picman-selection-all inImage)
    (set! theImage (if (= inCopy TRUE)
                     (car (picman-image-duplicate inImage))
                     inImage
                   )
    )
    (if (> (car (picman-drawable-type inLayer)) 1)
        (picman-image-convert-rgb theImage)
    )

    (set! theLayer (car (picman-layer-new theImage
                                        theWidth
                                        theHeight
                                        RGBA-IMAGE
                                        "layer 1"
                                        100
                                        NORMAL-MODE)))

    (picman-image-insert-layer theImage theLayer 0 0)


    (picman-edit-clear theLayer)
    (chris-color-edge theImage theLayer inColor inSize)

    (picman-layer-scale theLayer
                      (/ theWidth inGranu)
                      (/ theHeight inGranu)
                      TRUE)

    (plug-in-spread RUN-NONINTERACTIVE
                    theImage
                    theLayer
                    (/ inSize inGranu)
                    (/ inSize inGranu))
    (chris-color-edge theImage theLayer inColor 1)
    (picman-layer-scale theLayer theWidth theHeight TRUE)

    (picman-image-select-item theImage CHANNEL-OP-REPLACE theLayer)
    (picman-selection-invert theImage)
    (picman-edit-clear theLayer)
    (picman-selection-invert theImage)
    (picman-edit-clear theLayer)
    (picman-context-set-background inColor)
    (picman-edit-fill theLayer BACKGROUND-FILL)
    (picman-selection-none inImage)
    (chris-color-edge theImage theLayer inColor 1)

    (if (= inBlur TRUE)
        (plug-in-gauss-rle RUN-NONINTERACTIVE
                           theImage theLayer inSize TRUE TRUE)
    )
    (if (= inShadow TRUE)
        (begin
          (picman-image-insert-layer theImage
                                   (car (picman-layer-copy theLayer FALSE)) 0 -1)
          (picman-layer-scale theLayer
                            (- theWidth inSize) (- theHeight inSize) TRUE)
          (picman-desaturate theLayer)
          (picman-brightness-contrast theLayer 127 127)
          (picman-invert theLayer)
          (picman-layer-resize theLayer
                             theWidth
                             theHeight
                             (/ inSize 2)
                             (/ inSize 2))
          (plug-in-gauss-rle RUN-NONINTERACTIVE
                             theImage
                             theLayer
                             (/ inSize 2)
                             TRUE
                             TRUE)
          (picman-layer-set-opacity theLayer inShadWeight)
        )
    )
    (if (= inFlatten TRUE)
        (picman-image-flatten theImage)
    )
    (if (= inCopy TRUE)
      (begin
        (picman-image-clean-all theImage)
        (picman-display-new theImage)
      )
    )
    (picman-displays-flush)

    (picman-context-pop)
  )
)

(script-fu-register "script-fu-fuzzy-border"
  _"_Fuzzy Border..."
  _"Add a jagged, fuzzy border to an image"
  "Chris Gutteridge"
  "1998, Chris Gutteridge / ECS dept, University of Southampton, England."
  "3rd April 1998"
  "RGB* GRAY*"
  SF-IMAGE      "The image"               0
  SF-DRAWABLE   "The layer"               0
  SF-COLOR      _"Color"                  "white"
  SF-ADJUSTMENT _"Border size"            '(16 1 300 1 10 0 1)
  SF-TOGGLE     _"Blur border"            TRUE
  SF-ADJUSTMENT _"Granularity (1 is Low)" '(4 1 16 0.25 5 2 0)
  SF-TOGGLE     _"Add shadow"             FALSE
  SF-ADJUSTMENT _"Shadow weight (%)"      '(100 0 100 1 10 0 0)
  SF-TOGGLE     _"Work on copy"           TRUE
  SF-TOGGLE     _"Flatten image"          TRUE
)

(script-fu-menu-register "script-fu-fuzzy-border"
                         "<Image>/Filters/Decor")
