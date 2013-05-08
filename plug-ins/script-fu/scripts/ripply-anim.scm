; "Rippling Image" animation generator (ripply-anim.scm)
; Adam D. Moss (adam@foxbox.org)
; 97/05/18
;
; Designed to be used in conjunction with a plugin capable
; of saving animations (i.e. the GIF plugin).
;

(define (script-fu-ripply-anim img drawable displacement num-frames edge-type)

  (define (copy-layer-ripple dest-image dest-drawable source-image source-drawable)
    (picman-selection-all dest-image)
    (picman-edit-clear dest-drawable)
    (picman-selection-none dest-image)
    (picman-selection-all source-image)
    (picman-edit-copy source-drawable)
    (picman-selection-none source-image)
    (let ((floating-sel (car (picman-edit-paste dest-drawable FALSE))))
      (picman-floating-sel-anchor (car (picman-edit-paste dest-drawable FALSE)))
    )
  )

  (let* (
        (width (car (picman-drawable-width drawable)))
        (height (car (picman-drawable-height drawable)))
        (ripple-image (car (picman-image-new width height GRAY)))
        (ripple-layer (car (picman-layer-new ripple-image width height GRAY-IMAGE "Ripple Texture" 100 NORMAL-MODE)))
        (rippletiled-ret 0)
        (rippletiled-image 0)
        (rippletiled-layer 0)
        (remaining-frames 0)
        (xpos 0)
        (ypos 0)
        (xoffset 0)
        (yoffset 0)
        (dup-image 0)
        (layer-name 0)
        (this-image 0)
        (this-layer 0)
        (dup-layer 0)
        )

    (picman-context-push)

    ; this script generates its own displacement map

    (picman-image-undo-disable ripple-image)
    (picman-context-set-background '(127 127 127))
    (picman-image-insert-layer ripple-image ripple-layer 0 0)
    (picman-edit-fill ripple-layer BACKGROUND-FILL)
    (plug-in-noisify RUN-NONINTERACTIVE ripple-image ripple-layer FALSE 1.0 1.0 1.0 0.0)
    ; tile noise
    (set! rippletiled-ret (plug-in-tile RUN-NONINTERACTIVE ripple-image ripple-layer (* width 3) (* height 3) TRUE))
    (picman-image-undo-enable ripple-image)
    (picman-image-delete ripple-image)

    (set! rippletiled-image (car rippletiled-ret))
    (set! rippletiled-layer (cadr rippletiled-ret))
    (picman-image-undo-disable rippletiled-image)

    ; process tiled noise into usable displacement map
    (plug-in-gauss-iir RUN-NONINTERACTIVE rippletiled-image rippletiled-layer 35 TRUE TRUE)
    (picman-equalize rippletiled-layer TRUE)
    (plug-in-gauss-rle RUN-NONINTERACTIVE rippletiled-image rippletiled-layer 5 TRUE TRUE)
    (picman-equalize rippletiled-layer TRUE)

    ; displacement map is now in rippletiled-layer of rippletiled-image

    ; loop through the desired frames

    (set! remaining-frames num-frames)
    (set! xpos (/ width 2))
    (set! ypos (/ height 2))
    (set! xoffset (/ width num-frames))
    (set! yoffset (/ height num-frames))

    (let* ((out-imagestack (car (picman-image-new width height RGB))))

    (picman-image-undo-disable out-imagestack)

    (while (> remaining-frames 0)
      (set! dup-image (car (picman-image-duplicate rippletiled-image)))
      (picman-image-undo-disable dup-image)
      (picman-image-crop dup-image width height xpos ypos)

      (set! layer-name (string-append "Frame "
                 (number->string (- num-frames remaining-frames) 10)
                 " (replace)"))
      (set! this-layer (car (picman-layer-new out-imagestack
                                            width height RGB
                                            layer-name 100 NORMAL-MODE)))
      (picman-image-insert-layer out-imagestack this-layer 0 0)

      (copy-layer-ripple out-imagestack this-layer img drawable)

      (set! dup-layer (car (picman-image-get-active-layer dup-image)))
      (plug-in-displace RUN-NONINTERACTIVE out-imagestack this-layer
                        displacement displacement
                        TRUE TRUE dup-layer dup-layer edge-type)

      (picman-image-undo-enable dup-image)
      (picman-image-delete dup-image)

      (set! remaining-frames (- remaining-frames 1))
      (set! xpos (+ xoffset xpos))
      (set! ypos (+ yoffset ypos))
    )

    (picman-image-undo-enable rippletiled-image)
    (picman-image-delete rippletiled-image)
    (picman-image-undo-enable out-imagestack)
    (picman-display-new out-imagestack))

    (picman-context-pop)
  )
)

(script-fu-register "script-fu-ripply-anim"
  _"_Rippling..."
  _"Create a multi-layer image by adding a ripple effect to the current image"
  "Adam D. Moss (adam@foxbox.org)"
  "Adam D. Moss"
  "1997"
  "RGB* GRAY*"
  SF-IMAGE      "Image to animage"    0
  SF-DRAWABLE   "Drawable to animate" 0
  SF-ADJUSTMENT _"Rippling strength"  '(3 0 256 1 10 1 0)
  SF-ADJUSTMENT _"Number of frames"   '(15 0 256 1 10 0 1)
  SF-OPTION     _"Edge behavior"      '(_"Wrap" _"Smear" _"Black")
)

(script-fu-menu-register "script-fu-ripply-anim"
                         "<Image>/Filters/Animation/Animators")
