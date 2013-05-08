;  CARVE-IT
;   Carving, embossing, & stamping
;   Process taken from "The Photoshop 3 WOW! Book"
;   http://www.peachpit.com
;   This script requires a grayscale image containing a single layer.
;   This layer is used as the mask for the carving effect
;   NOTE: This script requires the image to be carved to either be an
;   RGB colour or grayscale image with a single layer. An indexed file
;   can not be used due to the use of picman-histogram and picman-levels.


(define (carve-brush brush-size)
  (cond ((<= brush-size 5) "Circle (05)")
        ((<= brush-size 7) "Circle (07)")
        ((<= brush-size 9) "Circle (09)")
        ((<= brush-size 11) "Circle (11)")
        ((<= brush-size 13) "Circle (13)")
        ((<= brush-size 15) "Circle (15)")
        ((<= brush-size 17) "Circle (17)")
        (else "Circle (19)")))

(define (carve-scale val scale)
  (* (sqrt val) scale))

(define (calculate-inset-gamma img layer)
  (let* ((stats (picman-histogram layer 0 0 255))
         (mean (car stats)))
    (cond ((< mean 127) (+ 1.0 (* 0.5 (/ (- 127 mean) 127.0))))
          ((>= mean 127) (- 1.0 (* 0.5 (/ (- mean 127) 127.0)))))))


(define (copy-layer-carve-it dest-image dest-drawable source-image source-drawable)
  (picman-selection-all dest-image)
  (picman-edit-clear dest-drawable)
  (picman-selection-none dest-image)
  (picman-selection-all source-image)
  (picman-edit-copy source-drawable)
      (let ((floating-sel (car (picman-edit-paste dest-drawable FALSE))))
        (picman-floating-sel-anchor floating-sel)))



(define (script-fu-carve-it mask-img mask-drawable bg-layer carve-white)
  (let* (
        (width (car (picman-drawable-width mask-drawable)))
        (height (car (picman-drawable-height mask-drawable)))
        (type (car (picman-drawable-type bg-layer)))
        (img (car (picman-image-new width height (cond ((= type RGB-IMAGE) RGB)
                                                     ((= type RGBA-IMAGE) RGB)
                                                     ((= type GRAY-IMAGE) GRAY)
                                                     ((= type GRAYA-IMAGE) GRAY)
                                                     ((= type INDEXED-IMAGE) INDEXED)
                                                     ((= type INDEXEDA-IMAGE) INDEXED)))))
        (size (min width height))
        (offx (carve-scale size 0.33))
        (offy (carve-scale size 0.25))
        (feather (carve-scale size 0.3))
        (brush-size (carve-scale size 0.3))
        (mask-fs 0)
        (mask (car (picman-channel-new img width height "Engraving Mask" 50 '(0 0 0))))
        (inset-gamma (calculate-inset-gamma (car (picman-item-get-image bg-layer)) bg-layer))
        (mask-fat 0)
        (mask-emboss 0)
        (mask-highlight 0)
        (mask-shadow 0)
        (shadow-layer 0)
        (highlight-layer 0)
        (cast-shadow-layer 0)
        (csl-mask 0)
        (inset-layer 0)
        (il-mask 0)
        (bg-width (car (picman-drawable-width bg-layer)))
        (bg-height (car (picman-drawable-height bg-layer)))
        (bg-type (car (picman-drawable-type bg-layer)))
        (bg-image (car (picman-item-get-image bg-layer)))
        (layer1 (car (picman-layer-new img bg-width bg-height bg-type "Layer1" 100 NORMAL-MODE)))
        )

    (picman-context-push)
    (picman-context-set-defaults)

    (picman-image-undo-disable img)

    (picman-image-insert-layer img layer1 0 0)

    (picman-selection-all img)
    (picman-edit-clear layer1)
    (picman-selection-none img)
    (copy-layer-carve-it img layer1 bg-image bg-layer)

    (picman-edit-copy mask-drawable)
    (picman-image-insert-channel img mask -1 0)

    (plug-in-tile RUN-NONINTERACTIVE img layer1 width height FALSE)
    (set! mask-fs (car (picman-edit-paste mask FALSE)))
    (picman-floating-sel-anchor mask-fs)
    (if (= carve-white FALSE)
        (picman-invert mask))

    (set! mask-fat (car (picman-channel-copy mask)))
    (picman-image-insert-channel img mask-fat -1 0)
    (picman-image-select-item img CHANNEL-OP-REPLACE mask-fat)
    (picman-context-set-brush (carve-brush brush-size))
    (picman-context-set-foreground '(255 255 255))
    (picman-edit-stroke mask-fat)
    (picman-selection-none img)

    (set! mask-emboss (car (picman-channel-copy mask-fat)))
    (picman-image-insert-channel img mask-emboss -1 0)
    (plug-in-gauss-rle RUN-NONINTERACTIVE img mask-emboss feather TRUE TRUE)
    (plug-in-emboss RUN-NONINTERACTIVE img mask-emboss 315.0 45.0 7 TRUE)

    (picman-context-set-background '(180 180 180))
    (picman-image-select-item img CHANNEL-OP-REPLACE mask-fat)
    (picman-selection-invert img)
    (picman-edit-fill mask-emboss BACKGROUND-FILL)
    (picman-image-select-item img CHANNEL-OP-REPLACE mask)
    (picman-edit-fill mask-emboss BACKGROUND-FILL)
    (picman-selection-none img)

    (set! mask-highlight (car (picman-channel-copy mask-emboss)))
    (picman-image-insert-channel img mask-highlight -1 0)
    (picman-levels mask-highlight 0 180 255 1.0 0 255)

    (set! mask-shadow mask-emboss)
    (picman-levels mask-shadow 0 0 180 1.0 0 255)

    (picman-edit-copy mask-shadow)
    (set! shadow-layer (car (picman-edit-paste layer1 FALSE)))
    (picman-floating-sel-to-layer shadow-layer)
    (picman-layer-set-mode shadow-layer MULTIPLY-MODE)

    (picman-edit-copy mask-highlight)
    (set! highlight-layer (car (picman-edit-paste shadow-layer FALSE)))
    (picman-floating-sel-to-layer highlight-layer)
    (picman-layer-set-mode highlight-layer SCREEN-MODE)

    (picman-edit-copy mask)
    (set! cast-shadow-layer (car (picman-edit-paste highlight-layer FALSE)))
    (picman-floating-sel-to-layer cast-shadow-layer)
    (picman-layer-set-mode cast-shadow-layer MULTIPLY-MODE)
    (picman-layer-set-opacity cast-shadow-layer 75)
    (plug-in-gauss-rle RUN-NONINTERACTIVE img cast-shadow-layer feather TRUE TRUE)
    (picman-layer-translate cast-shadow-layer offx offy)

    (set! csl-mask (car (picman-layer-create-mask cast-shadow-layer ADD-BLACK-MASK)))
    (picman-layer-add-mask cast-shadow-layer csl-mask)
    (picman-image-select-item img CHANNEL-OP-REPLACE mask)
    (picman-context-set-background '(255 255 255))
    (picman-edit-fill csl-mask BACKGROUND-FILL)

    (set! inset-layer (car (picman-layer-copy layer1 TRUE)))
    (picman-image-insert-layer img inset-layer 0 1)

    (set! il-mask (car (picman-layer-create-mask inset-layer ADD-BLACK-MASK)))
    (picman-layer-add-mask inset-layer il-mask)
    (picman-image-select-item img CHANNEL-OP-REPLACE mask)
    (picman-context-set-background '(255 255 255))
    (picman-edit-fill il-mask BACKGROUND-FILL)
    (picman-selection-none img)
    (picman-selection-none bg-image)
    (picman-levels inset-layer 0 0 255 inset-gamma 0 255)
    (picman-image-remove-channel img mask)
    (picman-image-remove-channel img mask-fat)
    (picman-image-remove-channel img mask-highlight)
    (picman-image-remove-channel img mask-shadow)

    (picman-item-set-name layer1 _"Carved Surface")
    (picman-item-set-name shadow-layer _"Bevel Shadow")
    (picman-item-set-name highlight-layer _"Bevel Highlight")
    (picman-item-set-name cast-shadow-layer _"Cast Shadow")
    (picman-item-set-name inset-layer _"Inset")

    (picman-display-new img)
    (picman-image-undo-enable img)

    (picman-context-pop)
  )
)

(script-fu-register "script-fu-carve-it"
    _"Stencil C_arve..."
    _"Use the specified drawable as a stencil to carve from the specified image."
    "Spencer Kimball"
    "Spencer Kimball"
    "1997"
    "GRAY"
    SF-IMAGE     "Mask image"        0
    SF-DRAWABLE  "Mask drawable"     0
    SF-DRAWABLE _"Image to carve"    0
    SF-TOGGLE   _"Carve white areas" TRUE
)

(script-fu-menu-register "script-fu-carve-it"
                         "<Image>/Filters/Decor")
