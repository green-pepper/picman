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


(define (script-fu-coffee-stain inImage inLayer inNumber inDark)
  (let* (
        (theImage inImage)
        (theHeight (car (picman-image-height theImage)))
        (theWidth (car (picman-image-width theImage)))
        (theNumber inNumber)
        (theSize (min theWidth theHeight))
        (theStain 0)
        )

    (picman-context-push)
    (picman-context-set-defaults)

    (picman-image-undo-group-start theImage)

    (while (> theNumber 0)
      (set! theNumber (- theNumber 1))
      (set! theStain (car (picman-layer-new theImage theSize theSize
                                          RGBA-IMAGE _"Stain" 100
                                          (if (= inDark TRUE)
                                              DARKEN-ONLY-MODE NORMAL-MODE))))

      (picman-image-insert-layer theImage theStain 0 0)
      (picman-selection-all theImage)
      (picman-edit-clear theStain)

      (let ((blobSize (/ (rand (- theSize 40)) (+ (rand 3) 1))))
        (picman-image-select-ellipse theImage
                             CHANNEL-OP-REPLACE
                             (/ (- theSize blobSize) 2)
                             (/ (- theSize blobSize) 2)
                             blobSize blobSize)
      )

      (script-fu-distress-selection theImage theStain
                                    (* (+ (rand 15) 1) (+ (rand 15) 1))
                                    (/ theSize 25) 4 2 TRUE TRUE)

      (picman-context-set-gradient "Coffee")

      (picman-edit-blend theStain CUSTOM-MODE NORMAL-MODE
                       GRADIENT-SHAPEBURST-DIMPLED 100 0 REPEAT-NONE FALSE
                       FALSE 0 0 TRUE
                       0 0 0 0)

      (picman-layer-set-offsets theStain
                              (- (rand theWidth) (/ theSize 2))
                              (- (rand theHeight) (/ theSize 2)))
    )

    (picman-selection-none theImage)

    (picman-image-undo-group-end theImage)

    (picman-displays-flush)

    (picman-context-pop)
  )
)

; Register the function with PICMAN:

(script-fu-register "script-fu-coffee-stain"
  _"_Coffee Stain..."
  _"Add realistic looking coffee stains to the image"
  "Chris Gutteridge"
  "1998, Chris Gutteridge / ECS dept, University of Southampton, England."
  "25th April 1998"
  "RGB*"
  SF-IMAGE       "The image"   0
  SF-DRAWABLE    "The layer"   0
  SF-ADJUSTMENT _"Stains"      '(3 1 10 1 1 0 0)
  SF-TOGGLE     _"Darken only" TRUE
)

(script-fu-menu-register "script-fu-coffee-stain" "<Image>/Filters/Decor")
