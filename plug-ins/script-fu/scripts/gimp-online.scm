; PICMAN - The GNU Image Manipulation Program
; Copyright (C) 1995 Spencer Kimball and Peter Mattis
;
; picman-online.scm
; Copyright (C) 2003  Henrik Brix Andersen <brix@picman.org>
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

(define (picman-online-docs-web-site)
  (plug-in-web-browser "http://docs.picman.org/")
)

(define (picman-help-concepts-usage)
  (picman-help "" "picman-concepts-usage")
)

(define (picman-help-using-docks)
  (picman-help "" "picman-using-docks")
)

(define (picman-help-using-simpleobjects)
  (picman-help "" "picman-using-simpleobjects")
)

(define (picman-help-using-selections)
  (picman-help "" "picman-using-selections")
)

(define (picman-help-using-fileformats)
  (picman-help "" "picman-using-fileformats")
)

(define (picman-help-using-photography)
  (picman-help "" "picman-using-photography")
)

(define (picman-help-using-web)
  (picman-help "" "picman-using-web")
)

(define (picman-help-concepts-paths)
  (picman-help "" "picman-concepts-paths")
)


; shortcuts to help topics
(script-fu-register "picman-help-concepts-paths"
   _"Using _Paths"
   _"Bookmark to the user manual"
    "Roman Joost <romanofski@picman.org>"
    "Roman Joost <romanofski@picman.org>"
    "2006"
    ""
)

(script-fu-menu-register "picman-help-concepts-paths"
			 "<Image>/Help/User Manual")


(script-fu-register "picman-help-using-web"
   _"_Preparing your Images for the Web"
   _"Bookmark to the user manual"
    "Roman Joost <romanofski@picman.org>"
    "Roman Joost <romanofski@picman.org>"
    "2006"
    ""
)

(script-fu-menu-register "picman-help-using-web"
			 "<Image>/Help/User Manual")


(script-fu-register "picman-help-using-photography"
   _"_Working with Digital Camera Photos"
   _"Bookmark to the user manual"
    "Roman Joost <romanofski@picman.org>"
    "Roman Joost <romanofski@picman.org>"
    "2006"
    ""
)

(script-fu-menu-register "picman-help-using-photography"
			 "<Image>/Help/User Manual")


(script-fu-register "picman-help-using-fileformats"
   _"Create, Open and Save _Files"
   _"Bookmark to the user manual"
    "Roman Joost <romanofski@picman.org>"
    "Roman Joost <romanofski@picman.org>"
    "2006"
    ""
)

(script-fu-menu-register "picman-help-using-fileformats"
			 "<Image>/Help/User Manual")


(script-fu-register "picman-help-concepts-usage"
   _"_Basic Concepts"
   _"Bookmark to the user manual"
    "Roman Joost <romanofski@picman.org>"
    "Roman Joost <romanofski@picman.org>"
    "2006"
    ""
)

(script-fu-menu-register "picman-help-concepts-usage"
			 "<Image>/Help/User Manual")


(script-fu-register "picman-help-using-docks"
   _"How to Use _Dialogs"
   _"Bookmark to the user manual"
    "Roman Joost <romanofski@picman.org>"
    "Roman Joost <romanofski@picman.org>"
    "2006"
    ""
)

(script-fu-menu-register "picman-help-using-docks"
			 "<Image>/Help/User Manual")


(script-fu-register "picman-help-using-simpleobjects"
   _"Drawing _Simple Objects"
   _"Bookmark to the user manual"
    "Roman Joost <romanofski@picman.org>"
    "Roman Joost <romanofski@picman.org>"
    "2006"
    ""
)

(script-fu-menu-register "picman-help-using-simpleobjects"
			 "<Image>/Help/User Manual")


(script-fu-register "picman-help-using-selections"
   _"Create and Use _Selections"
   _"Bookmark to the user manual"
    "Roman Joost <romanofski@picman.org>"
    "Roman Joost <romanofski@picman.org>"
    "2006"
    ""
)

(script-fu-menu-register "picman-help-using-simpleobjects"
			 "<Image>/Help/User Manual")


;; Links to PICMAN related web sites

(define (picman-online-main-web-site)
  (plug-in-web-browser "http://www.picman.org/")
)

(define (picman-online-developer-web-site)
  (plug-in-web-browser "http://developer.picman.org/")
)

(define (picman-online-plug-in-web-site)
  (plug-in-web-browser "http://registry.picman.org/")
)


(script-fu-register "picman-online-main-web-site"
   _"_Main Web Site"
   _"Bookmark to the PICMAN web site"
    "Henrik Brix Andersen <brix@picman.org>"
    "Henrik Brix Andersen <brix@picman.org>"
    "2003"
    ""
)

(script-fu-menu-register "picman-online-main-web-site"
                         "<Image>/Help/PICMAN Online")


(script-fu-register "picman-online-developer-web-site"
   _"_Developer Web Site"
   _"Bookmark to the PICMAN web site"
    "Henrik Brix Andersen <brix@picman.org>"
    "Henrik Brix Andersen <brix@picman.org>"
    "2003"
    ""
)

(script-fu-menu-register "picman-online-developer-web-site"
                         "<Image>/Help/PICMAN Online")


(script-fu-register "picman-online-docs-web-site"
   _"_User Manual Web Site"
   _"Bookmark to the PICMAN web site"
    "Roman Joost <romanofski@picman.org>"
    "Roman Joost <romanofski@picman.org>"
    "2006"
    ""
)

(script-fu-menu-register "picman-online-docs-web-site"
                         "<Image>/Help/PICMAN Online")


(script-fu-register "picman-online-plug-in-web-site"
   _"Plug-in _Registry"
   _"Bookmark to the PICMAN web site"
    "Henrik Brix Andersen <brix@picman.org>"
    "Henrik Brix Andersen <brix@picman.org>"
    "2003"
    ""
)

(script-fu-menu-register "picman-online-plug-in-web-site"
                         "<Image>/Help/PICMAN Online")
