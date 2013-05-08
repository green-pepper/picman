;;; line-nova.scm for picman-1.1 -*-scheme-*-
;;; Time-stamp: <1998/11/25 13:26:44 narazaki@picman.org>
;;; Author Shuji Narazaki <narazaki@picman.org>
;;; Version 0.7

(define (script-fu-line-nova img drw num-of-lines corn-deg offset variation)
  (let* (
        (*points* (cons-array (* 3 2) 'double))
        (modulo fmod)                        ; in R4RS way
        (pi/2 (/ *pi* 2))
        (pi/4 (/ *pi* 4))
        (pi3/4 (* 3 pi/4))
        (pi5/4 (* 5 pi/4))
        (pi3/2 (* 3 pi/2))
        (pi7/4 (* 7 pi/4))
        (2pi (* 2 *pi*))
        (rad/deg (/ 2pi 360))
        (variation/2 (/ variation 2))
        (drw-width (car (picman-drawable-width drw)))
        (drw-height (car (picman-drawable-height drw)))
        (drw-offsets (picman-drawable-offsets drw))
        (old-selection FALSE)
        (radius (max drw-height drw-width))
        (index 0)
        (dir-deg/line (/ 360 num-of-lines))
        )
    (picman-context-push)
    (picman-context-set-defaults)

    (define (draw-vector beg-x beg-y direction)

      (define (set-point! index x y)
            (aset *points* (* 2 index) x)
            (aset *points* (+ (* 2 index) 1) y)
      )
      (define (deg->rad rad)
            (* (modulo rad 360) rad/deg)
      )
      (define (set-marginal-point beg-x beg-y direction)
        (let (
             (dir1 (deg->rad (+ direction corn-deg)))
             (dir2 (deg->rad (- direction corn-deg)))
             )

          (define (aux dir index)
                   (set-point! index
                               (+ beg-x (* (cos dir) radius))
                               (+ beg-y (* (sin dir) radius)))
          )

          (aux dir1 1)
          (aux dir2 2)
        )
      )

      (let (
           (dir0 (deg->rad direction))
           (off (+ offset (- (modulo (rand) variation) variation/2)))
           )

        (set-point! 0
                    (+ beg-x (* off (cos dir0)))
                    (+ beg-y (* off (sin dir0)))
        )
        (set-marginal-point beg-x beg-y direction)
        (picman-image-select-polygon img CHANNEL-OP-ADD 6 *points*)
      )
    )

    (picman-image-undo-group-start img)

    (set! old-selection
      (if (eq? (car (picman-selection-is-empty img)) TRUE)
         #f
         (car (picman-selection-save img))
      )
    )

    (picman-selection-none img)
    (srand (realtime))
    (while (< index num-of-lines)
      (draw-vector (+ (nth 0 drw-offsets) (/ drw-width 2))
                   (+ (nth 1 drw-offsets) (/ drw-height 2))
                   (* index dir-deg/line)
      )
      (set! index (+ index 1))
    )
    (picman-edit-bucket-fill drw FG-BUCKET-FILL NORMAL-MODE 100 0 FALSE 0 0)

    (if old-selection
      (begin
        (picman-image-select-item img CHANNEL-OP-REPLACE old-selection)
        ;; (picman-image-set-active-layer img drw)
        ;; delete extra channel by Sven Neumann <neumanns@uni-duesseldorf.de>
        (picman-image-remove-channel img old-selection)
      )
    )

    (picman-image-undo-group-end img)
    (picman-displays-flush)
    (picman-context-pop)
  )
)

(script-fu-register "script-fu-line-nova"
  _"Line _Nova..."
  _"Fill a layer with rays emanating outward from its center using the foreground color"
  "Shuji Narazaki <narazaki@picman.org>"
  "Shuji Narazaki"
  "1997,1998"
  "*"
  SF-IMAGE       "Image"               0
  SF-DRAWABLE    "Drawable"            0
  SF-ADJUSTMENT _"Number of lines"     '(200 40 1000 1 1 0 1)
  SF-ADJUSTMENT _"Sharpness (degrees)" '(1.0 0.0 10.0 0.1 1 1 1)
  SF-ADJUSTMENT _"Offset radius"       '(100 0 2000 1 1 0 1)
  SF-ADJUSTMENT _"Randomness"          '(30 1 2000 1 1 0 1)
)

(script-fu-menu-register "script-fu-line-nova"
                         "<Image>/Filters/Render")
