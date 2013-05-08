;
;  burn-in-anim.scm V2.1  -  script-fu for PICMAN 1.1 and higher
;
;  Copyright (C) 9/2000  Roland Berger
;  roland@fuchur.leute.server.de
;  http://fuchur.leute.server.de
;
;  Let text appear and fade out with a "burn-in" like SFX.
;  Works on an image with a text and a background layer
;
;  Copying Policy:  GNU Public License http://www.gnu.org
;

(define (script-fu-burn-in-anim org-img
                                org-layer
                                glow-color
                                fadeout
                                bl-width
                                corona-width
                                after-glow
                                show-glow
                                optimize
                                speed)

  (let* (
        ;--- main variable: "bl-x" runs from 0 to layer-width
        (bl-x 0)
        (frame-nr 0)
        (img 0)
        (source-layer 0)
        (bg-source-layer 0)
        (source-layer-width 0)
        (bg-layer 0)
        (bg-layer-name 0)
        (bl-layer 0)
        (bl-layer-name 0)
        (bl-mask 0)
        (bl-layer-width 0)
        (bl-height 0)
        (bl-x-off 0)
        (bl-y-off 0)
        (nofadeout-bl-x-off 0)
        (nofadeout-bl-width 0)
        (blended-layer 0)
        (img-display 0)
        )

    (if (< speed 1)
        (set! speed (* -1 speed)) )

    ;--- check image and work on a copy
    (if (and (= (car (picman-image-get-layers org-img)) 2)
             (= (car (picman-image-get-floating-sel org-img)) -1))

        ;--- main program structure starts here, begin of "if-1"
        (begin
          (picman-context-push)
          (picman-context-set-defaults)

          (set! img (car (picman-image-duplicate org-img)))
          (picman-image-undo-disable img)
          (if (> (car (picman-drawable-type org-layer)) 1 )
              (picman-image-convert-rgb img))
          (set! source-layer    (aref (cadr (picman-image-get-layers img)) 0 ))
          (set! bg-source-layer (aref (cadr (picman-image-get-layers img)) 1 ))
          (set! source-layer-width (car (picman-drawable-width  source-layer)))

          ;--- hide layers, cause we want to "merge visible layers" later
          (picman-item-set-visible source-layer FALSE)
          (picman-item-set-visible bg-source-layer     FALSE)

          ;--- process image horizontal with pixel-speed
          (while (< bl-x (+ source-layer-width bl-width))
              (set! bl-layer (car (picman-layer-copy source-layer TRUE)))
              (set! bl-layer-name (string-append "fr-nr"
                                                 (number->string frame-nr 10) ) )

              (picman-image-insert-layer img bl-layer 0 -2)
              (picman-item-set-name bl-layer bl-layer-name)
              (picman-item-set-visible bl-layer TRUE)
              (picman-layer-set-lock-alpha bl-layer TRUE)
              (picman-layer-add-alpha bl-layer)

              ;--- add an alpha mask for blending and select it
              (picman-image-select-item img CHANNEL-OP-REPLACE bl-layer)
              (set! bl-mask (car (picman-layer-create-mask bl-layer ADD-BLACK-MASK)))
              (picman-layer-add-mask bl-layer bl-mask)

              ;--- handle layer geometry
              (set! bl-layer-width source-layer-width)
              (set! bl-height      (car (picman-drawable-height bl-layer)))
              (set! bl-x-off (- bl-x     bl-width))
              (set! bl-x-off (+ bl-x-off (car  (picman-drawable-offsets bl-layer))))
              (set! bl-y-off             (cadr (picman-drawable-offsets bl-layer)))

              ;--- select a rectangular area to blend
              (picman-image-select-rectangle img CHANNEL-OP-REPLACE bl-x-off bl-y-off bl-width bl-height)
              ;--- select at least 1 pixel!
              (picman-image-select-rectangle img CHANNEL-OP-ADD bl-x-off bl-y-off (+ bl-width 1) bl-height)

              (if (= fadeout FALSE)
                  (begin
                    (set! nofadeout-bl-x-off (car (picman-drawable-offsets bl-layer)))
                    (set! nofadeout-bl-width (+ nofadeout-bl-x-off bl-x))
                    (set! nofadeout-bl-width (max nofadeout-bl-width 1))
                    (picman-image-select-rectangle img CHANNEL-OP-REPLACE
                                                 nofadeout-bl-x-off bl-y-off
                                                 nofadeout-bl-width bl-height)
                  )
              )

              ;--- alpha blending text to trans (fadeout)
              (picman-context-set-foreground '(255 255 255))
              (picman-context-set-background '(  0   0   0))
              (if (= fadeout TRUE)
                  (begin
                    ; blend with 20% offset to get less transparency in the front
                    (picman-edit-blend bl-mask FG-BG-RGB-MODE NORMAL-MODE
                                     GRADIENT-LINEAR 100 20 REPEAT-NONE FALSE
                                     FALSE 0 0 TRUE
                                     (+ bl-x-off bl-width) 0 bl-x-off 0)
                  )
              )

              (if (= fadeout FALSE)
                  (begin
                    (picman-context-set-foreground '(255 255 255))
                    (picman-edit-bucket-fill bl-mask FG-BUCKET-FILL NORMAL-MODE
                                           100 255 0 0 0)
                  )
              )

              (picman-layer-remove-mask bl-layer MASK-APPLY)

              ;--- add bright glow in front
              (if (= show-glow TRUE)
                  (begin
                    ;--- add some brightness to whole text
                    (if (= fadeout TRUE)
                        (picman-brightness-contrast bl-layer 100 0)
                    )

          ;--- blend glow color inside the letters
          (picman-context-set-foreground glow-color)
          (picman-edit-blend bl-layer FG-TRANSPARENT-MODE NORMAL-MODE
                   GRADIENT-LINEAR 100 0 REPEAT-NONE FALSE
                   FALSE 0 0 TRUE
                   (+ bl-x-off bl-width) 0
                   (- (+ bl-x-off bl-width) after-glow) 0)

          ;--- add corona effect
          (picman-image-select-item img CHANNEL-OP-REPLACE bl-layer)
          (picman-selection-sharpen img)
          (picman-selection-grow img corona-width)
          (picman-layer-set-lock-alpha bl-layer FALSE)
          (picman-selection-feather img corona-width)
          (picman-context-set-foreground glow-color)
          (picman-edit-blend bl-layer FG-TRANSPARENT-MODE NORMAL-MODE
                   GRADIENT-LINEAR 100 0 REPEAT-NONE FALSE
                   FALSE 0 0 TRUE
                   (- (+ bl-x-off bl-width) corona-width) 0
                   (- (+ bl-x-off bl-width) after-glow) 0)
                  )
              )

              ;--- merge with bg layer
              (set! bg-layer (car (picman-layer-copy bg-source-layer FALSE)))
              (picman-image-insert-layer img bg-layer 0 -1)
              (picman-image-lower-item img bg-layer)
              (set! bg-layer-name (string-append "bg-"
                                                 (number->string frame-nr 10)))
              (picman-item-set-name bg-layer bg-layer-name)
              (picman-item-set-visible bg-layer TRUE)
              (set! blended-layer (car (picman-image-merge-visible-layers img
                                        CLIP-TO-IMAGE)))
              ;(set! blended-layer bl-layer)
              (picman-item-set-visible blended-layer FALSE)

              ;--- end of "while" loop
              (set! frame-nr (+ frame-nr 1))
              (set! bl-x     (+ bl-x speed))
          )

          ;--- finalize the job
          (picman-selection-none img)
          (picman-image-remove-layer img    source-layer)
          (picman-image-remove-layer img bg-source-layer)

          (picman-image-set-filename img "burn-in")

          (if (= optimize TRUE)
              (begin
                (picman-image-convert-indexed img 1 WEB-PALETTE 250 FALSE TRUE "")
                (set! img-out (car (plug-in-animationoptimize RUN-NONINTERACTIVE
                                                              img
                                                              bl-layer)))
              )
          )

          (picman-item-set-visible (aref (cadr (picman-image-get-layers img)) 0)
                                  TRUE)
          (picman-image-undo-enable img)
          (picman-image-clean-all img)
          (set! img-display (car (picman-display-new img)))

          (picman-displays-flush)

          (picman-context-pop)
        )

        ;--- false form of "if-1"
        (picman-message _"The Burn-In script needs two layers in total. A foreground layer with transparency and a background layer.")
    )
  )
)


(script-fu-register "script-fu-burn-in-anim"
    _"B_urn-In..."
    _"Create intermediate layers to produce an animated 'burn-in' transition between two layers"
    "Roland Berger roland@fuchur.leute.server.de"
    "Roland Berger"
    "January 2001"
    "RGBA GRAYA INDEXEDA"
    SF-IMAGE    "The image"            0
    SF-DRAWABLE "Layer to animate"     0
    SF-COLOR   _"Glow color"           "white"
    SF-TOGGLE  _"Fadeout"              FALSE
    SF-VALUE   _"Fadeout width"        "100"
    SF-VALUE   _"Corona width"         "7"
    SF-VALUE   _"After glow"           "50"
    SF-TOGGLE  _"Add glowing"          TRUE
    SF-TOGGLE  _"Prepare for GIF"      FALSE
    SF-VALUE   _"Speed (pixels/frame)" "50"
)

(script-fu-menu-register "script-fu-burn-in-anim"
                         "<Image>/Filters/Animation/Animators")
