; PICMAN - The GNU Image Manipulation Program
; Copyright (C) 1995 Spencer Kimball and Peter Mattis
;
; Weave script --- make an image look as if it were woven
; Copyright (C) 1997 Federico Mena Quintero
; federico@nuclecu.unam.mx
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


; Copies the specified rectangle from/to the specified drawable

(define (copy-rectangle img
                        drawable
                        x1
                        y1
                        width
                        height
                        dest-x
                        dest-y)
  (picman-image-select-rectangle img CHANNEL-OP-REPLACE x1 y1 width height)
  (picman-edit-copy drawable)
  (let ((floating-sel (car (picman-edit-paste drawable FALSE))))
    (picman-layer-set-offsets floating-sel dest-x dest-y)
    (picman-floating-sel-anchor floating-sel))
  (picman-selection-none img))

; Creates a single weaving tile

(define (create-weave-tile ribbon-width
                           ribbon-spacing
                           shadow-darkness
                           shadow-depth)
  (let* ((tile-size (+ (* 2 ribbon-width) (* 2 ribbon-spacing)))
         (darkness (* 255 (/ (- 100 shadow-darkness) 100)))
         (img (car (picman-image-new tile-size tile-size RGB)))
         (drawable (car (picman-layer-new img tile-size tile-size RGB-IMAGE
                                        "Weave tile" 100 NORMAL-MODE))))

    (picman-image-undo-disable img)
    (picman-image-insert-layer img drawable 0 0)

    (picman-context-set-background '(0 0 0))
    (picman-edit-fill drawable BACKGROUND-FILL)

    ; Create main horizontal ribbon

    (picman-context-set-foreground '(255 255 255))
    (picman-context-set-background (list darkness darkness darkness))

    (picman-image-select-rectangle img
                                 CHANNEL-OP-REPLACE
                                 0
                                 ribbon-spacing
                                 (+ (* 2 ribbon-spacing) ribbon-width)
                                 ribbon-width)

    (picman-edit-blend drawable FG-BG-RGB-MODE NORMAL-MODE
                     GRADIENT-BILINEAR 100 (- 100 shadow-depth) REPEAT-NONE FALSE
                     FALSE 0 0 TRUE
                     (/ (+ (* 2 ribbon-spacing) ribbon-width -1) 2) 0 0 0)

    ; Create main vertical ribbon

    (picman-image-select-rectangle img
                                 CHANNEL-OP-REPLACE
                                 (+ (* 2 ribbon-spacing) ribbon-width)
                                 0
                                 ribbon-width
                                 (+ (* 2 ribbon-spacing) ribbon-width))

    (picman-edit-blend drawable FG-BG-RGB-MODE NORMAL-MODE
                     GRADIENT-BILINEAR 100 (- 100 shadow-depth) REPEAT-NONE FALSE
                     FALSE 0 0 TRUE
                     0 (/ (+ (* 2 ribbon-spacing) ribbon-width -1) 2) 0 0)

    ; Create the secondary horizontal ribbon

    (copy-rectangle img
                    drawable
                    0
                    ribbon-spacing
                    (+ ribbon-width ribbon-spacing)
                    ribbon-width
                    (+ ribbon-width ribbon-spacing)
                    (+ (* 2 ribbon-spacing) ribbon-width))

    (copy-rectangle img
                    drawable
                    (+ ribbon-width ribbon-spacing)
                    ribbon-spacing
                    ribbon-spacing
                    ribbon-width
                    0
                    (+ (* 2 ribbon-spacing) ribbon-width))

    ; Create the secondary vertical ribbon

    (copy-rectangle img
                    drawable
                    (+ (* 2 ribbon-spacing) ribbon-width)
                    0
                    ribbon-width
                    (+ ribbon-width ribbon-spacing)
                    ribbon-spacing
                    (+ ribbon-width ribbon-spacing))

    (copy-rectangle img
                    drawable
                    (+ (* 2 ribbon-spacing) ribbon-width)
                    (+ ribbon-width ribbon-spacing)
                    ribbon-width
                    ribbon-spacing
                    ribbon-spacing
                    0)

    ; Done

    (picman-image-undo-enable img)
    (list img drawable)))

; Creates a complete weaving mask

(define (create-weave width
                      height
                      ribbon-width
                      ribbon-spacing
                      shadow-darkness
                      shadow-depth)
  (let* ((tile (create-weave-tile ribbon-width ribbon-spacing shadow-darkness
                                  shadow-depth))
         (tile-img (car tile))
         (tile-layer (cadr tile))
          (weaving (plug-in-tile RUN-NONINTERACTIVE tile-img tile-layer width height TRUE)))
    (picman-image-delete tile-img)
    weaving))

; Creates a single tile for masking

(define (create-mask-tile ribbon-width
                          ribbon-spacing
                          r1-x1
                          r1-y1
                          r1-width
                          r1-height
                          r2-x1
                          r2-y1
                          r2-width
                          r2-height
                          r3-x1
                          r3-y1
                          r3-width
                          r3-height)
  (let* ((tile-size (+ (* 2 ribbon-width) (* 2 ribbon-spacing)))
         (img (car (picman-image-new tile-size tile-size RGB)))
         (drawable (car (picman-layer-new img tile-size tile-size RGB-IMAGE
                                        "Mask" 100 NORMAL-MODE))))
    (picman-image-undo-disable img)
    (picman-image-insert-layer img drawable 0 0)

    (picman-context-set-background '(0 0 0))
    (picman-edit-fill drawable BACKGROUND-FILL)

    (picman-image-select-rectangle img CHANNEL-OP-REPLACE r1-x1 r1-y1 r1-width r1-height)
    (picman-image-select-rectangle img CHANNEL-OP-ADD r2-x1 r2-y1 r2-width r2-height)
    (picman-image-select-rectangle img CHANNEL-OP-ADD r3-x1 r3-y1 r3-width r3-height)

    (picman-context-set-background '(255 255 255))
    (picman-edit-fill drawable BACKGROUND-FILL)
    (picman-selection-none img)

    (picman-image-undo-enable img)

    (list img drawable)))

; Creates a complete mask image

(define (create-mask final-width
                     final-height
                     ribbon-width
                     ribbon-spacing
                     r1-x1
                     r1-y1
                     r1-width
                     r1-height
                     r2-x1
                     r2-y1
                     r2-width
                     r2-height
                     r3-x1
                     r3-y1
                     r3-width
                     r3-height)
  (let* ((tile (create-mask-tile ribbon-width ribbon-spacing
                                 r1-x1 r1-y1 r1-width r1-height
                                 r2-x1 r2-y1 r2-width r2-height
                                 r3-x1 r3-y1 r3-width r3-height))
         (tile-img (car tile))
         (tile-layer (cadr tile))
         (mask (plug-in-tile RUN-NONINTERACTIVE tile-img tile-layer final-width final-height
                             TRUE)))
    (picman-image-delete tile-img)
    mask))

; Creates the mask for horizontal ribbons

(define (create-horizontal-mask ribbon-width
                                ribbon-spacing
                                final-width
                                final-height)
  (create-mask final-width
               final-height
               ribbon-width
               ribbon-spacing
               0
               ribbon-spacing
               (+ (* 2 ribbon-spacing) ribbon-width)
               ribbon-width
               0
               (+ (* 2 ribbon-spacing) ribbon-width)
               ribbon-spacing
               ribbon-width
               (+ ribbon-width ribbon-spacing)
               (+ (* 2 ribbon-spacing) ribbon-width)
               (+ ribbon-width ribbon-spacing)
               ribbon-width))

; Creates the mask for vertical ribbons

(define (create-vertical-mask ribbon-width
                              ribbon-spacing
                              final-width
                              final-height)
  (create-mask final-width
               final-height
               ribbon-width
               ribbon-spacing
               (+ (* 2 ribbon-spacing) ribbon-width)
               0
               ribbon-width
               (+ (* 2 ribbon-spacing) ribbon-width)
               ribbon-spacing
               0
               ribbon-width
               ribbon-spacing
               ribbon-spacing
               (+ ribbon-width ribbon-spacing)
               ribbon-width
               (+ ribbon-width ribbon-spacing)))

; Adds a threads layer at a certain orientation to the specified image

(define (create-threads-layer img
                              width
                              height
                              length
                              density
                              orientation)
  (let* ((drawable (car (picman-layer-new img width height RGBA-IMAGE
                                        "Threads" 100 NORMAL-MODE)))
         (dense (/ density 100.0)))
    (picman-image-insert-layer img drawable 0 -1)
    (picman-context-set-background '(255 255 255))
    (picman-edit-fill drawable BACKGROUND-FILL)
    (plug-in-noisify RUN-NONINTERACTIVE img drawable FALSE dense dense dense dense)
    (plug-in-c-astretch RUN-NONINTERACTIVE img drawable)
    (cond ((eq? orientation 'horizontal)
           (plug-in-gauss-rle RUN-NONINTERACTIVE img drawable length TRUE FALSE))
          ((eq? orientation 'vertical)
           (plug-in-gauss-rle RUN-NONINTERACTIVE img drawable length FALSE TRUE)))
    (plug-in-c-astretch RUN-NONINTERACTIVE img drawable)
    drawable))

(define (create-complete-weave width
                               height
                               ribbon-width
                               ribbon-spacing
                               shadow-darkness
                               shadow-depth
                               thread-length
                               thread-density
                               thread-intensity)
  (let* ((weave (create-weave width height ribbon-width ribbon-spacing
                              shadow-darkness shadow-depth))
         (w-img (car weave))
         (w-layer (cadr weave))

         (h-layer (create-threads-layer w-img width height thread-length
                                        thread-density 'horizontal))
         (h-mask (car (picman-layer-create-mask h-layer ADD-WHITE-MASK)))

         (v-layer (create-threads-layer w-img width height thread-length
                                        thread-density 'vertical))
         (v-mask (car (picman-layer-create-mask v-layer ADD-WHITE-MASK)))

         (hmask (create-horizontal-mask ribbon-width ribbon-spacing
                                        width height))
         (hm-img (car hmask))
         (hm-layer (cadr hmask))

         (vmask (create-vertical-mask ribbon-width ribbon-spacing width height))
         (vm-img (car vmask))
         (vm-layer (cadr vmask)))

    (picman-layer-add-mask h-layer h-mask)
    (picman-selection-all hm-img)
    (picman-edit-copy hm-layer)
    (picman-image-delete hm-img)
    (picman-floating-sel-anchor (car (picman-edit-paste h-mask FALSE)))
    (picman-layer-set-opacity h-layer thread-intensity)
    (picman-layer-set-mode h-layer MULTIPLY-MODE)

    (picman-layer-add-mask v-layer v-mask)
    (picman-selection-all vm-img)
    (picman-edit-copy vm-layer)
    (picman-image-delete vm-img)
    (picman-floating-sel-anchor (car (picman-edit-paste v-mask FALSE)))
    (picman-layer-set-opacity v-layer thread-intensity)
    (picman-layer-set-mode v-layer MULTIPLY-MODE)

    ; Uncomment this if you want to keep the weaving mask image
    ; (picman-display-new (car (picman-image-duplicate w-img)))

    (list w-img
          (car (picman-image-flatten w-img)))))

; The main weave function

(define (script-fu-weave img
                         drawable
                         ribbon-width
                         ribbon-spacing
                         shadow-darkness
                         shadow-depth
                         thread-length
                         thread-density
                         thread-intensity)
  (let* (
        (d-img (car (picman-item-get-image drawable)))
        (d-width (car (picman-drawable-width drawable)))
        (d-height (car (picman-drawable-height drawable)))
        (d-offsets (picman-drawable-offsets drawable))

        (weaving (create-complete-weave d-width
                                        d-height
                                        ribbon-width
                                        ribbon-spacing
                                        shadow-darkness
                                        shadow-depth
                                        thread-length
                                        thread-density
                                        thread-intensity))
        (w-img (car weaving))
        (w-layer (cadr weaving))
        )

    (picman-context-push)
    (picman-context-set-feather FALSE)

    (picman-selection-all w-img)
    (picman-edit-copy w-layer)
    (picman-image-delete w-img)
    (let ((floating-sel (car (picman-edit-paste drawable FALSE))))
      (picman-layer-set-offsets floating-sel
                              (car d-offsets)
                              (cadr d-offsets))
      (picman-layer-set-mode floating-sel MULTIPLY-MODE)
      (picman-floating-sel-to-layer floating-sel)
    )

    (picman-displays-flush)

    (picman-context-pop)
  )
)

(script-fu-register "script-fu-weave"
  _"_Weave..."
  _"Create a new layer filled with a weave effect to be used as an overlay or bump map"
  "Federico Mena Quintero"
  "Federico Mena Quintero"
  "June 1997"
  "RGB* GRAY*"
  SF-IMAGE       "Image to Weave"    0
  SF-DRAWABLE    "Drawable to Weave" 0
  SF-ADJUSTMENT _"Ribbon width"     '(30  0 256 1 10 1 1)
  SF-ADJUSTMENT _"Ribbon spacing"   '(10  0 256 1 10 1 1)
  SF-ADJUSTMENT _"Shadow darkness"  '(75  0 100 1 10 1 1)
  SF-ADJUSTMENT _"Shadow depth"     '(75  0 100 1 10 1 1)
  SF-ADJUSTMENT _"Thread length"    '(200 0 256 1 10 1 1)
  SF-ADJUSTMENT _"Thread density"   '(50  0 100 1 10 1 1)
  SF-ADJUSTMENT _"Thread intensity" '(100 0 100 1 10 1 1)
)

(script-fu-menu-register "script-fu-weave"
                         "<Image>/Filters/Artistic")
