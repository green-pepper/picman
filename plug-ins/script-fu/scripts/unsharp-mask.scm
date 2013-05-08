;;; unsharp-mask.scm
;;; Time-stamp: <1998/11/17 13:18:39 narazaki@picman.org>
;;; Author: Narazaki Shuji <narazaki@picman.org>
;;; Version 0.8

(define (script-fu-unsharp-mask img drw mask-size mask-opacity)
  (let* (
        (drawable-width (car (picman-drawable-width drw)))
        (drawable-height (car (picman-drawable-height drw)))
        (new-image (car (picman-image-new drawable-width drawable-height RGB)))
        (original-layer (car (picman-layer-new new-image
                                             drawable-width drawable-height
                                             RGB-IMAGE "Original"
                                             100 NORMAL-MODE)))
        (original-layer-for-darker 0)
        (original-layer-for-lighter 0)
        (blured-layer-for-darker 0)
        (blured-layer-for-lighter 0)
        (darker-layer 0)
        (lighter-layer 0)
        )

    (picman-selection-all img)
    (picman-edit-copy drw)

    (picman-image-undo-disable new-image)

    (picman-image-insert-layer new-image original-layer 0 0)
    (picman-floating-sel-anchor
      (car (picman-edit-paste original-layer FALSE)))

    (set! original-layer-for-darker (car (picman-layer-copy original-layer TRUE)))
    (set! original-layer-for-lighter (car (picman-layer-copy original-layer TRUE)))
    (set! blured-layer-for-darker (car (picman-layer-copy original-layer TRUE)))
    (picman-item-set-visible original-layer FALSE)
    (picman-display-new new-image)

    ;; make darker mask
    (picman-image-insert-layer new-image blured-layer-for-darker 0 -1)
    (plug-in-gauss-iir RUN-NONINTERACTIVE
		       new-image blured-layer-for-darker mask-size TRUE TRUE)
    (set! blured-layer-for-lighter
          (car (picman-layer-copy blured-layer-for-darker TRUE)))
    (picman-image-insert-layer new-image original-layer-for-darker 0 -1)
    (picman-layer-set-mode original-layer-for-darker SUBTRACT-MODE)
    (set! darker-layer
          (car (picman-image-merge-visible-layers new-image CLIP-TO-IMAGE)))
    (picman-item-set-name darker-layer "darker mask")
    (picman-item-set-visible darker-layer FALSE)

    ;; make lighter mask
    (picman-image-insert-layer new-image original-layer-for-lighter 0 -1)
    (picman-image-insert-layer new-image blured-layer-for-lighter 0 -1)
    (picman-layer-set-mode blured-layer-for-lighter SUBTRACT-MODE)
    (set! lighter-layer
          (car (picman-image-merge-visible-layers new-image CLIP-TO-IMAGE)))
    (picman-item-set-name lighter-layer "lighter mask")

    ;; combine them
    (picman-item-set-visible original-layer TRUE)
    (picman-layer-set-mode darker-layer SUBTRACT-MODE)
    (picman-layer-set-opacity darker-layer mask-opacity)
    (picman-item-set-visible darker-layer TRUE)
    (picman-layer-set-mode lighter-layer ADDITION-MODE)
    (picman-layer-set-opacity lighter-layer mask-opacity)
    (picman-item-set-visible lighter-layer TRUE)

    (picman-image-undo-enable new-image)
    (picman-displays-flush)
  )
)

(script-fu-register "script-fu-unsharp-mask"
  "Unsharp Mask..."
  "Make a new image from the current layer by applying the unsharp mask method"
  "Shuji Narazaki <narazaki@picman.org>"
  "Shuji Narazaki"
  "1997,1998"
  ""
  SF-IMAGE      "Image"             0
  SF-DRAWABLE   "Drawable to apply" 0
  SF-ADJUSTMENT _"Mask size"        '(5 1 100 1 1 0 1)
  SF-ADJUSTMENT _"Mask opacity"     '(50 0 100 1 1 0 1)
)
