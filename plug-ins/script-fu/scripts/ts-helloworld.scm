; "Hello, World" Test v1.00 February 29, 2004
; by Kevin Cozens <kcozens@interlog.com>
;
; Creates an image with the text "Hello, World!"
; This was the first TinyScheme based script ever created and run for the
; 2.x version of PICMAN.

; PICMAN - The GNU Image Manipulation Program
; Copyright (C) 1995 Spencer Kimball and Peter Mattis
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
;
; Tiny-Fu first successfully ran this script at 2:07am on March 6, 2004.

(define (script-fu-helloworld text font size colour)
  (let* (
        (width 10)
        (height 10)
        (img (car (picman-image-new width height RGB)))
        (text-layer)
        )

    (picman-context-push)

    (picman-image-undo-disable img)
    (picman-context-set-foreground colour)

    (set! text-layer (car (picman-text-fontname img -1 0 0 text 10 TRUE size PIXELS font)))
    (set! width (car (picman-drawable-width text-layer)))
    (set! height (car (picman-drawable-height text-layer)))
    (picman-image-resize img width height 0 0)

    (picman-image-undo-enable img)
    (picman-display-new img)

    (picman-context-pop)
  )
)

(script-fu-register "script-fu-helloworld"
    "_Hello World..."
    "Creates an image with a user specified text string."
    "Kevin Cozens <kcozens@interlog.com>"
    "Kevin Cozens"
    "February 29, 2004"
    ""
    SF-STRING     "Text string"         "Hello, World!"
    SF-FONT       "Font"                "Sans"
    SF-ADJUSTMENT "Font size (pixels)"  '(100 2 1000 1 10 0 1)
    SF-COLOR      "Color"               '(0 0 0)
)

(script-fu-menu-register "script-fu-helloworld"
                         "<Image>/Filters/Languages/Script-Fu/Test")
