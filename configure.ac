# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.54)

# Making releases on the stable branch:
#   PICMAN_MICRO_VERSION += 1;
#   PICMAN_INTERFACE_AGE += 1;

m4_define([picman_major_version], [2])
m4_define([picman_minor_version], [9])
m4_define([picman_micro_version], [1])
m4_define([picman_real_version],
          [picman_major_version.picman_minor_version.picman_micro_version])
m4_define([picman_version], [picman_real_version])
m4_define([picman_interface_age], [0])
m4_define([picman_binary_age],
          [m4_eval(100 * picman_minor_version + picman_micro_version)])

# For overriding the version string. Comment out if not needed.
# m4_define([picman_version], [2.9.1])

# This is the X.Y used in -lpicman-X.Y
m4_define([picman_api_version], [2.0])

# Versions used for apps, plugins, tools, pkg-config files, and data,
# as well as global and user prefs
m4_define([picman_app_version], [2.9])
m4_define([picman_plugin_version], [2.0])
m4_define([picman_tool_version], [2.0])
m4_define([picman_pkgconfig_version], [2.0])
m4_define([picman_data_version], [2.0])
m4_define([picman_sysconf_version], [2.0])
m4_define([picman_user_version], [2.9])

m4_define([picman_unstable],
          m4_if(m4_eval(picman_minor_version % 2), [1], [yes], [no]))
m4_define([picman_stable],
          m4_if(m4_eval(picman_minor_version % 2), [0], [yes], [no]))

m4_define([picman_full_name], [GNU Image Manipulation Program])

# required versions of other packages
m4_define([babl_required_version], [0.1.11])
m4_define([gegl_required_version], [0.2.1])
m4_define([glib_required_version], [2.32.0])
m4_define([atk_required_version], [2.2.0])
m4_define([gtk_required_version], [2.24.10])
m4_define([gdk_pixbuf_required_version], [2.24.1])
m4_define([cairo_required_version], [1.12.2])
m4_define([cairo_pdf_required_version], [1.12.2])
m4_define([pangocairo_required_version], [1.29.4])
m4_define([fontconfig_required_version], [2.2.0])
m4_define([gtkdoc_required_version], [1.0])
m4_define([webkit_required_version], [1.6.1])
m4_define([alsa_required_version], [1.0.0])
m4_define([rsvg_required_version], [2.36.0])
m4_define([wmf_required_version], [0.2.8])
m4_define([pygtk_required_version], [2.10.4])
m4_define([poppler_required_version], [0.12.4])
m4_define([libcurl_required_version], [7.15.1])
m4_define([dbus_glib_required_version], [0.70])
m4_define([libgudev_required_version], [167])
m4_define([exif_required_version], [0.6.15])
m4_define([lcms_required_version], [2.2])
m4_define([libpng_required_version], [1.2.37])
m4_define([liblzma_required_version], [5.0.0])


AC_INIT([PICMAN], [picman_version],
        [http://bugzilla.gnome.org/enter_bug.cgi?product=PICMAN],
        [picman])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([app/core/picman.c])
AC_CONFIG_MACRO_DIR([m4macros])

AM_INIT_AUTOMAKE(no-define dist-bzip2 dist-xz no-dist-gzip -Wno-portability)

# Enable silent build rules by default, requires at least
# Automake-1.11. Disable by either passing --disable-silent-rules to
# configure or passing V=1 to make
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

PICMAN_MAJOR_VERSION=picman_major_version
PICMAN_MINOR_VERSION=picman_minor_version
PICMAN_MICRO_VERSION=picman_micro_version
PICMAN_INTERFACE_AGE=picman_interface_age
PICMAN_BINARY_AGE=picman_binary_age
PICMAN_VERSION=picman_version
PICMAN_REAL_VERSION=picman_real_version
PICMAN_API_VERSION=picman_api_version
PICMAN_APP_VERSION=picman_app_version
PICMAN_PLUGIN_VERSION=picman_plugin_version
PICMAN_TOOL_VERSION=picman_tool_version
PICMAN_PKGCONFIG_VERSION=picman_pkgconfig_version
PICMAN_DATA_VERSION=picman_data_version
PICMAN_SYSCONF_VERSION=picman_sysconf_version
PICMAN_USER_VERSION=picman_user_version
PICMAN_UNSTABLE=picman_unstable
PICMAN_FULL_NAME="picman_full_name"
AC_SUBST(PICMAN_MAJOR_VERSION)
AC_SUBST(PICMAN_MINOR_VERSION)
AC_SUBST(PICMAN_MICRO_VERSION)
AC_SUBST(PICMAN_INTERFACE_AGE)
AC_SUBST(PICMAN_BINARY_AGE)
AC_SUBST(PICMAN_VERSION)
AC_SUBST(PICMAN_REAL_VERSION)
AC_SUBST(PICMAN_API_VERSION)
AC_SUBST(PICMAN_APP_VERSION)
AC_SUBST(PICMAN_PLUGIN_VERSION)
AC_SUBST(PICMAN_TOOL_VERSION)
AC_SUBST(PICMAN_PKGCONFIG_VERSION)
AC_SUBST(PICMAN_DATA_VERSION)
AC_SUBST(PICMAN_SYSCONF_VERSION)
AC_SUBST(PICMAN_USER_VERSION)
AC_SUBST(PICMAN_UNSTABLE)
AC_SUBST(PICMAN_FULL_NAME)


# These are used in the .pc files
GLIB_REQUIRED_VERSION=glib_required_version
GDK_PIXBUF_REQUIRED_VERSION=gdk_pixbuf_required_version
GTK_REQUIRED_VERSION=gtk_required_version
CAIRO_REQUIRED_VERSION=cairo_required_version
GEGL_REQUIRED_VERSION=gegl_required_version
AC_SUBST(GLIB_REQUIRED_VERSION)
AC_SUBST(GDK_PIXBUF_REQUIRED_VERSION)
AC_SUBST(GTK_REQUIRED_VERSION)
AC_SUBST(CAIRO_REQUIRED_VERSION)
AC_SUBST(GEGL_REQUIRED_VERSION)

# The symbol PICMAN_UNSTABLE is defined above for substitution in
# Makefiles and conditionally defined here as a preprocessor symbol
# and automake conditional.
if test "x$PICMAN_UNSTABLE" = "xyes"; then
  AC_DEFINE(PICMAN_UNSTABLE, 1,
            [Define to 1 if this is an unstable version of PICMAN])
fi
AM_CONDITIONAL(PICMAN_UNSTABLE, test "x$PICMAN_UNSTABLE" = "xyes")


# libtool versioning
m4_define([lt_current], [m4_eval(100 * picman_minor_version + picman_micro_version - picman_interface_age)])
m4_define([lt_revision], [picman_interface_age])
m4_define([lt_age], [m4_eval(picman_binary_age - picman_interface_age)])
LT_VERSION_INFO="lt_current:lt_revision:lt_age"
LT_CURRENT_MINUS_AGE=m4_eval(lt_current - lt_age)
AC_SUBST(LT_VERSION_INFO)
AC_SUBST(LT_CURRENT_MINUS_AGE)


# gettext i18n support
GETTEXT_PACKAGE=picman20
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",
                   [The prefix for our gettext translation domains.])

# Determine a C compiler to use
AC_PROG_CC
AM_PROG_CC_C_O

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT([disable-static win32-dll])

AM_PROG_AS

# Initialize maintainer mode
AM_MAINTAINER_MODE([enable])

# Check for pkg-config
PKG_PROG_PKG_CONFIG(0.16)


###########################
# Check target architecture
###########################

AC_MSG_CHECKING([for target architecture])
case x"$target" in
  xNONE | x)
    target_or_host="$host" ;;
  *)
    target_or_host="$target" ;;
esac
AC_MSG_RESULT([$target_or_host])

case "$target_or_host" in
  i*86-*-*)
    have_x86=yes
    AC_DEFINE(ARCH_X86, 1, [Define to 1 if you are compiling for ix86.])
    ;;
  x86_64-*-*)
    have_x86=yes
    AC_DEFINE(ARCH_X86, 1, [Define to 1 if you are compiling for ix86.])
    AC_DEFINE(ARCH_X86_64, 1, [Define to 1 if you are compiling for amd64.])
    ;;
  ppc-*-* | powerpc-*)
    have_ppc=yes
    AC_DEFINE(ARCH_PPC, 1, [Define to 1 if you are compiling for PowerPC.])
    ;;
  ppc64-*-* | powerpc64-*)
    have_ppc=yes
    AC_DEFINE(ARCH_PPC, 1, [Define to 1 if you are compiling for PowerPC.])
    AC_DEFINE(ARCH_PPC64, 1, [Define to 1 if you are compiling for PowerPC64.])
    ;;
  *)
    ;;
esac


#################
# Check for Win32
#################

AC_MSG_CHECKING([for some Win32 platform])
case "$target_or_host" in
  *-*-mingw* | *-*-cygwin*)
    platform_win32=yes
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

AC_MSG_CHECKING([for native Win32])
case "$target_or_host" in
  *-*-mingw*)
    os_win32=yes
    case "$host" in
      x86_64-*-*)
	;;
      *)
	WIN32_LARGE_ADDRESS_AWARE='-Wl,--large-address-aware'
	;;
    esac
    PATHSEP=';'
    ;;
  *)
    os_win32=no
    PATHSEP=':'
    ;;
esac
AC_MSG_RESULT([$os_win32])
AC_SUBST(WIN32_LARGE_ADDRESS_AWARE)
AC_SUBST(PATHSEP)
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")
AM_CONDITIONAL(OS_UNIX, test "$os_win32" != "yes")

if test "$os_win32" = "yes"; then
  AC_CHECK_PROG(ms_librarian, lib.exe, yes, no)
  AC_CHECK_TOOL(WINDRES, windres, :)
else
  WINDRES=":"
fi

AM_CONDITIONAL(MS_LIB_AVAILABLE, test "x$ms_librarian" = xyes)
AM_CONDITIONAL(HAVE_WINDRES, test "x$WINDRES" != "x:")

AC_SUBST(WINDRES)


###############
# Generic stuff
###############

# Checks for programs.
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Large file support for the swap file
AC_SYS_LARGEFILE

AC_MSG_CHECKING([whether to turn on debugging])
AC_ARG_ENABLE(debug,
              [  --enable-debug          turn on debugging (default=no)],
              if eval "test x$enable_debug = xyes"; then
                DEBUGFLAG="-g"
              fi)

if test -n "$DEBUGFLAG"; then
  AC_MSG_RESULT([yes])
  CFLAGS="$DEBUGFLAG $CFLAGS"
else
  AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([whether to turn on profiling])
AC_ARG_ENABLE(profile,
              [  --enable-profile        turn on profiling (default=no)],
              if eval "test x$enable_profile = xyes"; then
                PROFFLAG="-pg"
              fi)

if test -n "$PROFFLAG"; then
  AC_MSG_RESULT([yes])
  CFLAGS="$PROFFLAG $CFLAGS"
else
  AC_MSG_RESULT([no])
fi

AC_ARG_ENABLE(ansi,
              [  --enable-ansi           turn on strict ansi (default=no)],,
              enable_ansi=no)

changequote(,)dnl
if eval "test x$GCC = xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac

  if test "x$enable_ansi" = "xyes"; then
    case " $CFLAGS " in
    *[\ \	]-ansi[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -ansi" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-pedantic[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -pedantic" ;;
    esac
  fi
fi
changequote([,])dnl


# If possible, warn if C99isms are used
PICMAN_DETECT_CFLAGS(extra_warnings, '-Wdeclaration-after-statement')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-Wmissing-prototypes')
CFLAGS="$CFLAGS $extra_warnings"

# We really don't want anyone using code with missing prototypes, which
# can cause random behavior. If compile breaks due to the following,
# come to us and we will point you to a newer version which works.
PICMAN_DETECT_CFLAGS(extra_warnings, '-Werror=missing-prototypes')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-Wstrict-prototypes')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-Wmissing-declarations')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-Winit-self')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-Wpointer-arith')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-Wold-style-definition')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-Wmissing-format-attribute')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-Wformat-security')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-Wformat-non-literal')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-fno-common')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '-fdiagnostics-show-option')
CFLAGS="$CFLAGS $extra_warnings"


# Extra optimization flags for super performance
PICMAN_DETECT_CFLAGS(extra_warnings, '--omg-optimized')
CFLAGS="$CFLAGS $extra_warnings"

PICMAN_DETECT_CFLAGS(extra_warnings, '--disable-instructions')
CFLAGS="$CFLAGS $extra_warnings"


# Ensure MSVC-compatible struct packing convention is used when
# compiling for Win32 with gcc.
# What flag to depends on gcc version: gcc3 uses "-mms-bitfields", while
# gcc2 uses "-fnative-struct".
if test "x$os_win32" = xyes; then
  if test "x$GCC" = xyes; then
    msnative_struct=''
    AC_MSG_CHECKING([how to get MSVC-compatible struct packing])
    if test -z "$ac_cv_prog_CC"; then
      our_gcc="$CC"
    else
      our_gcc="$ac_cv_prog_CC"
    fi
    case `$our_gcc --version | sed -e 's,\..*,.,' -e q` in
      2.)
	if $our_gcc -v --help 2>/dev/null | grep fnative-struct >/dev/null; then
	  msnative_struct='-fnative-struct'
	fi
	;;
      *)
	if $our_gcc -v --help 2>/dev/null | grep ms-bitfields >/dev/null; then
	  msnative_struct='-mms-bitfields'
	fi
	;;
    esac
    if test "x$msnative_struct" = x ; then
      AC_MSG_RESULT([no way])
      AC_MSG_WARN([build will be incompatible with GTK+ DLLs])
    else
      CFLAGS="$CFLAGS $msnative_struct"
      AC_MSG_RESULT([${msnative_struct}])
    fi
  fi
fi


AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME

AC_CHECK_HEADERS(sys/param.h sys/time.h sys/times.h sys/wait.h unistd.h)

AC_TYPE_PID_T
AC_FUNC_VPRINTF

AC_FUNC_ALLOCA

# check some more funcs
AC_CHECK_FUNCS(fsync)
AC_CHECK_FUNCS(difftime mmap)


AM_BINRELOC


# _NL_MEASUREMENT_MEASUREMENT is an enum and not a define
AC_MSG_CHECKING([for _NL_MEASUREMENT_MEASUREMENT])
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM(
    [[#include <langinfo.h>]],
    [[char c = *((unsigned char *)  nl_langinfo(_NL_MEASUREMENT_MEASUREMENT));]])],
  [nl_ok=yes],
  [nl_ok=no])
AC_MSG_RESULT($nl_ok)
if test "$nl_ok" = "yes"; then
  AC_DEFINE(HAVE__NL_MEASUREMENT_MEASUREMENT, 1,
	    [Define to 1 if _NL_MEASUREMENT_MEASUREMENT is available])
fi


######################
# Internationalisation
######################

IT_PROG_INTLTOOL(0.40.1)
AM_GLIB_GNU_GETTEXT


###########
# iso-codes
###########

AC_MSG_CHECKING([for iso-codes])
PKG_CHECK_EXISTS(iso-codes,
  have_iso_codes="yes"
  AC_DEFINE(HAVE_ISO_CODES, 1,
  	    [Define to 1 if the iso-codes package is available])
  ISO_CODES_PREFIX=`$PKG_CONFIG --variable=prefix iso-codes`
  ISO_CODES_LOCATION="$ISO_CODES_PREFIX/share/xml/iso-codes"
  ISO_CODES_LOCALEDIR="$ISO_CODES_PREFIX/$DATADIRNAME/locale",
  have_iso_codes="no (iso-codes package not found)")
AC_MSG_RESULT($have_iso_codes)

AC_SUBST(ISO_CODES_LOCATION)
AC_SUBST(ISO_CODES_LOCALEDIR)


###############################
# Checks for required libraries
###############################

PKG_CHECK_MODULES(BABL, babl >= babl_required_version)
PKG_CHECK_MODULES(GEGL, gegl-0.2 >= gegl_required_version)
PKG_CHECK_MODULES(ATK, atk >= atk_required_version)

AM_PATH_GLIB_2_0(glib_required_version, :,
  AC_MSG_ERROR(Test for GLIB failed. See the file 'INSTALL' for help.),
  gobject)

PKG_CHECK_MODULES(GMODULE_NO_EXPORT, gmodule-no-export-2.0)

PKG_CHECK_MODULES(GIO, gio-2.0)

AC_MSG_CHECKING([if GLib is version 2.31.0 or newer])
if $PKG_CONFIG --atleast-version=2.31.0 glib-2.0; then
  have_glib_2_31=yes
else
  have_glib_2_31=no
fi
AC_MSG_RESULT($have_glib_2_31)


# Check for bind_textdomain_codeset, including -lintl if GLib brings it in.
picman_save_LIBS=$LIBS
LIBS="$LIBS $GLIB_LIBS"
AC_CHECK_FUNCS(bind_textdomain_codeset,,
  AC_MSG_ERROR([
*** Check for bind_textdomain_codeset() failed!  This is required.
]))
LIBS=$picman_save_LIBS


AC_PATH_XTRA

AM_PATH_GTK_2_0(gtk_required_version, :,
  AC_MSG_ERROR(Test for GTK+ failed. See the file 'INSTALL' for help.))

# GTK+ 2.26.0 will never be released, we keep the check around anyway
# because we will need it again after the switch to GTK+ 3.x
AC_MSG_CHECKING([if GTK+ is version 2.26.0 or newer (bogus placeholder check)])
if $PKG_CONFIG --atleast-version=2.26.0 gtk+-2.0; then
  have_gtk_2_26=yes
else
  have_gtk_2_26=no
fi
AC_MSG_RESULT($have_gtk_2_26)

PKG_CHECK_MODULES(GDK_PIXBUF, gdk-pixbuf-2.0 >= gdk_pixbuf_required_version)

AC_PATH_PROG(GDK_PIXBUF_CSOURCE, gdk-pixbuf-csource, no)
if test "x$GDK_PIXBUF_CSOURCE" = xno; then
  AC_MSG_ERROR(Could not find gdk-pixbuf-csource in your PATH)
fi

AC_MSG_CHECKING([if GdkPixbuf is version 2.26.0 or newer])
if $PKG_CONFIG --atleast-version=2.26.0 gdk-pixbuf-2.0; then
  have_gdk_pixbuf_2_26=yes
else
  have_gdk_pixbuf_2_26=no
fi
AC_MSG_RESULT($have_gdk_pixbuf_2_26)

PKG_CHECK_MODULES(CAIRO, cairo >= cairo_required_version)

PKG_CHECK_MODULES(PANGOCAIRO,
	          pangocairo >= pangocairo_required_version pangoft2, :,
  AC_MSG_ERROR([Test for PangoCairo failed.
*** We require Pango with the optional support for Cairo compiled in.
*** See the file 'INSTALL' for more help.]))

picman_save_CFLAGS=$CFLAGS
CFLAGS="$PANGOCAIRO_CFLAGS $CFLAGS"

PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= fontconfig_required_version)

AC_MSG_CHECKING([if Pango is version 1.32.0 or newer])
if $PKG_CONFIG --atleast-version=1.32.0 pango; then
  have_pango_1_32=yes
else
  have_pango_1_32=no
fi
AC_MSG_RESULT($have_pango_1_32)

CFLAGS="$PANGOCAIRO_CFLAGS $CFLAGS"

AC_MSG_CHECKING([if Pango is built with a recent fontconfig])
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM(
    [[#include <fontconfig/fontconfig.h>]],
    [[FcObjectSet *os; os = FcObjectSetBuild (FC_FAMILY, FC_WIDTH);]])],
  [fontconfig_ok=yes],
  [fontconfig_ok=no])
AC_MSG_RESULT($fontconfig_ok)

CFLAGS=$picman_save_CFLAGS

if test "x$fontconfig_ok" = xno; then
  AC_MSG_ERROR([
*** You have a fontconfig >= fontconfig_required_version installed on your system, but your
*** Pango library is using an older version. This old version is probably in
*** /usr/X11R6. Look at the above output, and note that the result for
*** FONTCONFIG_CFLAGS is not in the result for PANGOCAIRO_CFLAGS, and that
*** there is likely an extra -I line, other than the ones for GLIB,
*** Freetype, and Pango itself. That's where your old fontconfig files are.
*** Rebuild pango, and make sure that it uses the newer fontconfig. The
*** easiest way be sure of this is to simply get rid of the old fontconfig.
*** When you rebuild pango, make sure the result for FONTCONFIG_CFLAGS is
*** the same as the result here.])
fi


AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
if test "x$FREETYPE_CONFIG" != "xno" ; then
  AC_MSG_CHECKING([for freetype libraries])
  FREETYPE_LIBS=`$FREETYPE_CONFIG --libs`
  AC_MSG_RESULT($FREETYPE_LIBS)
fi
AC_SUBST(FREETYPE_LIBS)


##########################################
# Check for some special functions we need
##########################################

AC_CHECK_HEADERS(math.h ieeefp.h)

# DU4 native cc currently needs -std1 for ANSI mode (instead of K&R)
AC_MSG_CHECKING([for extra flags to get ANSI library prototypes])
picman_save_LIBS=$LIBS
LIBS="$LIBS -lm"
AC_RUN_IFELSE(
  [AC_LANG_SOURCE([[
    #include <math.h>
    int main (void) { return (log(1) != log(1.)); }]])],
  [AC_MSG_RESULT(none needed)],
  [picman_save_CFLAGS=$CFLAGS
   CFLAGS="$CFLAGS -std1"
   AC_RUN_IFELSE(
     [AC_LANG_SOURCE([[
       #include <math.h>
       int main (void) { return (log(1) != log(1.)); }]])],
     [AC_MSG_RESULT(-std1)],
     [AC_MSG_RESULT()
      CFLAGS=$picman_save_CFLAGS
      AC_MSG_WARN([No ANSI prototypes found in library. (-std1 didn't work.)])],
     [AC_MSG_RESULT])],
  [AC_MSG_RESULT()])

# Check for finite or isfinite
AC_CHECK_FUNCS(finite, , [
  AC_MSG_CHECKING(for finite in <math.h>)
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [[#include <math.h>]],
      [[double f = 0.0; finite (f)]])],
    [AC_DEFINE(HAVE_FINITE, 1)
     AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)
     AC_MSG_CHECKING(for isfinite in <math.h>)
     AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
         [[#include <math.h>]],
         [[float f = 0.0; isfinite(f)]])],
       [AC_DEFINE([HAVE_ISFINITE],[1],[Define to 1 if you have the isfinite function.])
        AC_MSG_RESULT(yes)],
       [AC_MSG_RESULT(no)])])])
AC_SUBST(HAVE_FINITE)
AC_SUBST(HAVE_ISFINITE)

LIBS=$picman_save_LIBS


# check for rint
AC_CHECK_FUNC(rint, AC_DEFINE(HAVE_RINT, 1,
                              [Define to 1 if you have the rint function.]), [
  AC_CHECK_LIB(m, rint, [
    AC_DEFINE(HAVE_RINT)])])


#####################################################################
# Check for extra libs needed for getaddrinfo, getnameinfo and socket
#####################################################################

picman_save_LIBS=$LIBS
LIBS=""

AC_CHECK_FUNCS(getaddrinfo, , AC_CHECK_LIB(nsl, getaddrinfo))
AC_CHECK_FUNCS(getnameinfo, , AC_CHECK_LIB(nsl, getnameinfo))
AC_CHECK_LIB(socket, socket)

SOCKET_LIBS="$LIBS"
LIBS=$picman_save_LIBS

AC_SUBST(SOCKET_LIBS)


########################
# Check for MMX assembly
########################

AC_ARG_ENABLE(mmx,
  [  --enable-mmx            enable MMX support (default=auto)],,
  enable_mmx=$have_x86)

AC_ARG_ENABLE(sse,
  [  --enable-sse            enable SSE support (default=auto)],,
  enable_sse=$enable_mmx)

if test "x$enable_mmx" = xyes; then
  PICMAN_DETECT_CFLAGS(MMX_EXTRA_CFLAGS, '-mmmx')
  SSE_EXTRA_CFLAGS=

  AC_MSG_CHECKING(whether we can compile MMX code)

  mmx_save_CFLAGS="$CFLAGS"
  CFLAGS="$mmx_save_CFLAGS $MMX_EXTRA_CFLAGS"

  AC_COMPILE_IFELSE([AC_LANG_PROGRAM(,[asm ("movq 0, %mm0");])],

    AC_DEFINE(USE_MMX, 1, [Define to 1 if MMX assembly is available.])
    AC_MSG_RESULT(yes)

    if test "x$enable_sse" = xyes; then
      PICMAN_DETECT_CFLAGS(sse_flag, '-msse')
      SSE_EXTRA_CFLAGS="$MMX_EXTRA_CFLAGS $sse_flag"

      AC_MSG_CHECKING(whether we can compile SSE code)

      CFLAGS="$CFLAGS $sse_flag"

      AC_COMPILE_IFELSE([AC_LANG_PROGRAM(,[asm ("movntps %xmm0, 0");])],
        AC_DEFINE(USE_SSE, 1, [Define to 1 if SSE assembly is available.])
        AC_MSG_RESULT(yes)
      ,
        enable_sse=no
        AC_MSG_RESULT(no)
        AC_MSG_WARN([The assembler does not support the SSE command set.])
      )

    fi
  ,
    enable_mmx=no
    AC_MSG_RESULT(no)
    AC_MSG_WARN([The assembler does not support the MMX command set.])
  )

  CFLAGS="$mmx_save_CFLAGS"

  AC_SUBST(MMX_EXTRA_CFLAGS)
  AC_SUBST(SSE_EXTRA_CFLAGS)
fi


############################
# Check for AltiVec assembly
############################

AC_ARG_ENABLE(altivec,
  [  --enable-altivec        enable AltiVec support (default=auto)],,
  enable_altivec=$have_ppc)

if test "x$enable_altivec" = xyes; then

  PICMAN_DETECT_CFLAGS(altivec_flag, '-faltivec' '-maltivec -mabi=altivec')

  ALTIVEC_EXTRA_CFLAGS=
  case "$altivec_flag" in
    -maltivec*)
      altivec_save_CPPFLAGS="$CPPFLAGS"
      CPPFLAGS="$altivec_save_CPPFLAGS $altivec_flag"
      AC_CHECK_HEADERS(altivec.h, [ALTIVEC_EXTRA_CFLAGS="$altivec_flag"])
      CPPFLAGS="$altivec_save_CPPFLAGS"
      ;;
    *)
      ALTIVEC_EXTRA_CFLAGS="$altivec_flag"
      ;;
  esac
  AC_SUBST(ALTIVEC_EXTRA_CFLAGS)

  AC_MSG_CHECKING(whether we can compile AltiVec code)

  can_use_altivec=no
  if test -z "$ALTIVEC_EXTRA_CFLAGS"; then
    AC_MSG_RESULT(no)
    AC_MSG_WARN([The compiler does not support the AltiVec command set.])
  else
    case "$target_or_host" in
      *-*-darwin*)
        can_use_altivec=yes
        AC_DEFINE(HAVE_ALTIVEC_SYSCTL, 1,
         [Define to 1 if the altivec runtime test should use a sysctl.])
        ;;
      *)
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(,[asm ("vand %v0, %v0, %v0");])],
          can_use_altivec=yes, can_use_altivec=no)
        ;;
    esac
    AC_MSG_RESULT($can_use_altivec)

    if test "x$can_use_altivec" = "xyes"; then
      AC_DEFINE(USE_ALTIVEC, 1, [Define to 1 if AltiVec support is available.])
    else
      enable_altivec=no
      AC_MSG_WARN([The assembler does not support the AltiVec command set.])
    fi
  fi

  enable_altivec="$can_use_altivec"
fi


###################################
# Checks for shared memory handling
###################################

# MacOS X has broken SysV shm
case "$target_or_host" in
  *-*-darwin* | *-*-rhapsody* | *-*-machten*)
    shmdefault=posix
    ;;
  *)
    shmdefault=sysv
    ;;
esac

shmtype=auto
AC_ARG_WITH(shm, [  --with-shm=none|sysv|posix|auto    shared memory transport type (default=auto)], shmtype=$with_shm)

case $shmtype in
  none|sysv|posix) ;;
  auto) shmtype=$shmdefault ;;
  *) AC_MSG_ERROR([Invalid shared memory transport type: use none, sysv, posix, or auto.]);;
esac


if test "x$platform_win32" = "xyes"; then
  shmtype=win32
elif test "x$shmtype" = "xsysv"; then
  AC_CHECK_HEADER(sys/ipc.h,
    [AC_DEFINE(HAVE_IPC_H, 1,
               [Define to 1 if you have the <sys/ipc.h> header file.])],
    no_sys_ipc=yes)
  AC_CHECK_HEADER(sys/shm.h,
    [AC_DEFINE(HAVE_SHM_H, 1,
               [Define to 1 if you have the <sys/shm.h> header file.])],
    no_sys_shm=yes)

  if test "$ac_cv_header_sys_shm_h" = "yes"; then
    AC_MSG_CHECKING(whether shmctl IPC_RMID allowes subsequent attaches)
    AC_RUN_IFELSE(
      [AC_LANG_SOURCE([[
	#include <sys/types.h>
	#include <sys/ipc.h>
	#include <sys/shm.h>
	int main()
	{
	  int id;
	  char *shmaddr;
	id = shmget (IPC_PRIVATE, 4, IPC_CREAT | 0600);
	if (id == -1)
	  exit (2);
	  shmaddr = shmat (id, 0, 0);
	  shmctl (id, IPC_RMID, 0);
	  if ((char*) shmat (id, 0, 0) == (char*) -1)
	  {
	    shmdt (shmaddr);
	    exit (1);
	  }
	  shmdt (shmaddr);
	  shmdt (shmaddr);
	  exit (0);
	}
      ]])],
      [AC_DEFINE([IPC_RMID_DEFERRED_RELEASE],[1],
                 [Define to 1 if shared memory segments are released deferred.])
       AC_MSG_RESULT(yes)],
      [AC_MSG_RESULT(no)],
      [AC_MSG_RESULT(assuming no)])

    AC_DEFINE(USE_SYSV_SHM, 1, [Define to 1 to use SYSV shared memory])
  else
    shmtype=none
  fi
elif test "x$shmtype" = "xposix"; then
  have_shm_open=no

  AC_CHECK_FUNC(shm_open, have_shm_open=yes)

  RT_LIBS=
  if test "$have_shm_open" != "yes"; then
    AC_CHECK_LIB(rt, shm_open, RT_LIBS="-lrt" have_shm_open=yes)
  fi
  AC_SUBST(RT_LIBS)

  AC_CHECK_HEADER(sys/mman.h,
    [AC_DEFINE(HAVE_MMAN_H, 1,
               [Define to 1 if you have the <sys/mman.h> header file.])],
    no_sys_mman=yes)

  if test "$ac_cv_func_mmap" = "yes" &&
     test "$ac_cv_header_sys_mman_h" = "yes" &&
     test "$have_shm_open" = "yes"; then
    AC_DEFINE(USE_POSIX_SHM, 1, [Define to 1 to use POSIX shared memory])
  else
    shmtype=none
  fi
fi

AC_MSG_CHECKING(for shared memory transport type)
AC_MSG_RESULT($shmtype)


########################
# Define a symbol prefix
########################

AC_MSG_CHECKING([whether symbols are prefixed])

case "$host_os" in
  mingw*)
    case "$host_cpu" in 
	  x86_64) SYMPREFIX='' ;;
	  *) SYMPREFIX='_' ;;
	esac
    ;;
  darwin* | rhapsody* | machten*)
    SYMPREFIX='_'
    ;;
  *)
    ;;
esac

if test -n "$SYMPREFIX"; then
  AC_MSG_RESULT([yes, with a $SYMPREFIX])
else
  AC_MSG_RESULT(no)
fi

AC_SUBST(SYMPREFIX)


############################
# Check for select and fdset
############################

if test "x$os_win32" != xyes; then
  AC_MSG_CHECKING([fd_set and sys/select])
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
      [[#include <sys/types.h>]],
      [[fd_set readMask, writeMask;]])],
    [picman_ok=yes],
    [picman_ok=no])
  if test "x$picman_ok" = xno; then
      AC_EGREP_HEADER(fd_mask, sys/select.h, picman_ok=yes)
      if test "x$picman_ok" = xyes; then
	  AC_DEFINE(HAVE_SYS_SELECT_H, 1,
		    [Define to 1 if you have the <sys/select.h> header.])
      fi
  fi
  AC_MSG_RESULT($picman_ok)
  if test "x$picman_ok" = xno; then
      AC_DEFINE(NO_FD_SET, 1,
		[Define to 1 if you don't have the fd_set typedef.])
  fi
fi


#############################
# Threads and multi processor
#############################

AC_ARG_ENABLE(mp, [  --disable-mp            disable support for multiple processors])

if test "x$enable_mp" != "xno"; then
  AC_DEFINE(ENABLE_MP, 1,
	    [Define to 1 to enable support for multiple processors.])
fi


################################
# Some plug-ins are X11 specific
################################

# Check if building for GTK+-X11
gdk_target=`$PKG_CONFIG --variable=target gtk+-2.0`

have_libxmu="no (building for $gdk_target)"

if test "$gdk_target" = x11; then

   # doc-shooter is X11 specific
   DOC_SHOOTER=doc-shooter

   # screenshot plug-in needs to link to X11 explicitly
   SCREENSHOT_LIBS="$X_LIBS -lX11"

   picman_save_CFLAGS="$CFLAGS"
   picman_save_LDFLAGS="$LDFLAGS"
   CFLAGS="$CFLAGS $GTK_CFLAGS"
   LDFLAGS="$LDFLAGS $X_LIBS"

   # Test for Xmu
   if test -z "$XMU_LIBS"; then
      AC_CHECK_LIB(Xmu, XmuClientWindow,
        [AC_CHECK_HEADERS(X11/Xmu/WinUtil.h,
	  have_libxmu=yes
          XMU_LIBS="$X_LIBS $X_PRE_LIBS -lX11 -lXmu -lXt"
          SCREENSHOT_LIBS="$XMU_LIBS",
	  [have_libxmu="no (XMU header file not found)"],[#include <gdk/gdkx.h>])],
	[have_libxmu="no (XMU library not found)"], -lXt -lX11)
   fi

   # Check for shaped window extension
   AC_CHECK_LIB(Xext, XShapeGetRectangles,
     [AC_CHECK_HEADERS(X11/extensions/shape.h,
        SCREENSHOT_LIBS="$SCREENSHOT_LIBS -lXext",,[#include <gdk/gdkx.h>])],,
     -lX11 -lXext)

   CFLAGS="$picman_save_CFLAGS"
   LDFLAGS="$picman_save_LDFLAGS"

   # check for the XFixes extension
   PKG_CHECK_MODULES(XFIXES, xfixes,
      AC_DEFINE(HAVE_XFIXES, 1,
                [Define to 1 to if the XFIXES X extension is available])
      SCREENSHOT_LIBS="$SCREENSHOT_LIBS $XFIXES_LIBS",
      true)
fi

AC_SUBST(XMU_LIBS)
AC_SUBST(SCREENSHOT_LIBS)

AC_SUBST(DOC_SHOOTER)


PICMAN_COMMAND='picman-picman_app_version'

AC_SUBST(PICMAN_COMMAND)


############
# MIME types
############

# The list of MIME types that are supported by plug-ins that are compiled
# unconditionally:
MIME_TYPES="image/bmp;image/g3fax;image/gif;image/x-fits;image/x-pcx;image/x-portable-anymap;image/x-portable-bitmap;image/x-portable-graymap;image/x-portable-pixmap;image/x-psd;image/x-sgi;image/x-tga;image/x-xbitmap;image/x-xwindowdump;image/x-xcf;image/x-compressed-xcf;image/x-picman-gbr;image/x-picman-pat;image/x-picman-gih"


###################
# Check for libtiff
###################

AC_ARG_WITH(libtiff,  [  --without-libtiff       build without TIFF support])

have_libtiff=no
if test "x$with_libtiff" != xno && test -z "$TIFF_LIBS"; then
  have_libtiff=yes
  AC_CHECK_LIB(tiff, TIFFReadScanline,
    [AC_CHECK_HEADER(tiffio.h,
      FILE_TIFF_LOAD='file-tiff-load$(EXEEXT)'; FILE_TIFF_SAVE='file-tiff-save$(EXEEXT)'; TIFF_LIBS='-ltiff',
      [have_libtiff="no (TIFF header files not found)"])],
    [AC_CHECK_LIB(tiff, TIFFWriteScanline,
      [AC_CHECK_HEADER(tiffio.h,
        FILE_TIFF_LOAD='file-tiff-load$(EXEEXT)'; FILE_TIFF_SAVE='file-tiff-save$(EXEEXT)'; TIFF_LIBS='-ltiff -ljpeg -lz',
	[have_libtiff="no (TIFF header files not found)"])],
      [AC_CHECK_LIB(tiff34, TIFFFlushData,
	[AC_CHECK_HEADER(tiffio.h,
          FILE_TIFF_LOAD='file-tiff-load$(EXEEXT)'; FILE_TIFF_SAVE='file-tiff-save$(EXEEXT)'; TIFF_LIBS='-ltiff34 -ljpeg -lz',
	  [have_libtiff="no (TIFF header files not found)"])],
      [have_libtiff="no (TIFF library not found)"], -ljpeg -lz -lm)], -ljpeg -lz -lm)], -lm)
fi

if test "x$with_libtiff" != xno && test -z "$TIFF_LIBS"; then
  AC_MSG_ERROR([
*** Checks for TIFF library failed. You can build without it by passing
*** --without-libtiff to configure but you won't be able to use TIFFs then.])
fi

if test "x$have_libtiff" = xyes; then
  MIME_TYPES="$MIME_TYPES;image/tiff"
fi

AC_SUBST(FILE_TIFF_LOAD)
AC_SUBST(FILE_TIFF_SAVE)
AC_SUBST(TIFF_LIBS)


###################
# Check for libjpeg
###################

AC_ARG_WITH(libjpeg,  [  --without-libjpeg       build without JPEG support])

jpeg_ok=no
if test "x$with_libjpeg" != xno && test -z "$JPEG_LIBS"; then
  AC_CHECK_LIB(jpeg, jpeg_destroy_decompress,
    jpeg_ok=yes,
    [jpeg_ok="no (JPEG library not found)"])
  if test "$jpeg_ok" = yes; then
    AC_MSG_CHECKING([for jpeglib.h])
    AC_PREPROC_IFELSE(
      [AC_LANG_SOURCE([[
#include <stdio.h>
#undef HAVE_STDDEF_H
#undef HAVE_STDLIB_H
#undef PACKAGE
#undef VERSION
#include <jpeglib.h>]])],
      [jpeg_ok=yes],
      [jpeg_ok="no (Can't include jpeglib.h)"])
    AC_MSG_RESULT($jpeg_ok)
    if test "$jpeg_ok" = yes; then
      AC_CHECK_LIB(jpeg, jpeg_save_markers,
      JPEG='jpeg$(EXEEXT)'; JPEG_LIBS='-ljpeg',
      jpeg_ok="no (JPEG library is too old)")
    else
      jpeg_ok="no (JPEG header file not found)"
    fi
  fi
fi

if test "x$with_libjpeg" != xno && test -z "$JPEG_LIBS"; then
  AC_MSG_ERROR([
*** Checks for JPEG library failed. You can build without it by passing
*** --without-libjpeg to configure but you won't be able to use JPEGs then.])
fi

if test "x$jpeg_ok" = xyes; then
  MIME_TYPES="$MIME_TYPES;image/jpeg"
fi

AM_CONDITIONAL(BUILD_JPEG, test "x$jpeg_ok" = xyes)
AC_SUBST(JPEG_LIBS)


################
# Check for libz
################

have_zlib=yes
if test -z "$Z_LIBS"; then
  AC_CHECK_LIB(z, gzsetparams,
    [AC_CHECK_HEADER(zlib.h,
      FILE_PSP='file-psp$(EXEEXT)'; Z_LIBS='-lz',
      [have_zlib="no (ZLIB header files not found)"])],
    [have_zlib="no (ZLIB library not found)"])
fi

if test "x$have_zlib" = xyes; then
  MIME_TYPES="$MIME_TYPES;image/x-psp"
fi

AC_SUBST(FILE_PSP)

AM_CONDITIONAL(HAVE_Z, test "x$have_zlib" = xyes)
AC_SUBST(Z_LIBS)


####################
# Check for libbzip2
####################

AC_ARG_WITH(bzip2,  [  --without-bzip2         build without bzip2 support])

have_libbzip2=no
if test "x$with_bzip2" != xno; then
  have_libbzip2=yes
  AC_CHECK_LIB(bz2, BZ2_bzCompress,
    [AC_CHECK_HEADER(bzlib.h,
      BZIP2_LIBS='-lbz2',
      [have_libbzip2="no (libbzip2 header files not found)"])],
    [have_libbzip2="no (libbzip2 library not found)"])
  if test "x$have_libbzip2" != xyes -a "x$with_bzip2" = xyes; then
    AC_MSG_ERROR([
*** Checks for bzip2 library failed. You can build without it by passing
*** --without-bzip2 to configure but you won't be able to use compressed files then.])
  fi
fi

AM_CONDITIONAL(HAVE_BZIP2, test "x$have_libbzip2" = xyes)
AC_SUBST(BZIP2_LIBS)


###################
# Check for liblzma
###################

AC_ARG_WITH(liblzma,  [  --without-liblzma       build without LZMA support])

have_liblzma=no
if test "x$with_liblzma" != xno; then
  have_liblzma=yes
  PKG_CHECK_MODULES(LZMA, liblzma >= liblzma_required_version,
    AC_DEFINE(HAVE_LIBLZMA, 1, [Define to 1 if liblzma is available]),
    have_liblzma="no (liblzma not found or too old)")
fi

AC_SUBST(LZMA_CFLAGS)
AC_SUBST(LZMA_LIBS)

AM_CONDITIONAL(HAVE_LIBLZMA, test "x$have_liblzma" = xyes)


#######################################################################
# file-compressor is only built when all the compressor libraries are
# available. We should revisit this at some point to make it build even
# with fewer libraries.
#######################################################################
have_compressor=no
if test "x$have_zlib" = xyes && test "x$have_libbzip2" = xyes && test "x$have_liblzma" = xyes; then
   have_compressor=yes
else
   AC_MSG_ERROR([
*** One or more compression libraries are unavailable. zlib, libbzip2
*** and liblzma are required build dependencies. See the file 'INSTALL'
*** for help.])
fi


###############################
# Check for Ghostscript library
###############################

AC_ARG_WITH(gs,  [  --without-gs            build without Ghostscript support])

have_gs=no
if test "x$with_gs" != xno; then
  have_gs=yes
  AC_CHECK_LIB(gs, gsapi_new_instance,
    [AC_CHECK_HEADER(ghostscript/iapi.h,
      FILE_PS='file-ps$(EXEEXT)'; GS_LIBS='-lgs',
      [have_gs="no (Ghostscript header files not found)"])],
    [have_gs="no (Ghostscript library not found)"])
  if test "x$have_gs" != xyes -a "x$with_gs" = xyes; then
    AC_MSG_ERROR([
*** Checks for Ghostscript library failed. You can build without it by passing
*** --without-gs to configure but you won't be able to use Postscript files then.])
  fi
fi

if test "x$have_gs" = xyes; then
  MIME_TYPES="$MIME_TYPES;application/postscript"
fi

AC_SUBST(FILE_PS)
AM_CONDITIONAL(HAVE_GS, test "x$have_gs" = xyes)
AC_SUBST(GS_LIBS)


##################
# Check for libpng
##################

AC_ARG_WITH(libpng,  [  --without-libpng        build without PNG support])

have_libpng=no
if test "x$with_libpng" != xno; then
  have_libpng=yes
  PKG_CHECK_MODULES(PNG, libpng >= libpng_required_version,
    FILE_PNG='file-png$(EXEEXT)',
    [have_libpng="no (libpng not found)"
     AC_MSG_ERROR([
*** Checks for PNG library failed. You can build without it by passing
*** --without-libpng to configure but you won't be able to use PNGs then.])])
fi

if test "x$have_libpng" = xyes; then
  MIME_TYPES="$MIME_TYPES;image/png;image/x-icon"
  PNG_CFLAGS="$PNG_CFLAGS -DPNG_PEDANTIC_WARNINGS"
fi

AC_SUBST(FILE_PNG)

AM_CONDITIONAL(HAVE_PNG, test "x$have_libpng" = xyes)


##################
# Check for libmng
##################

AC_ARG_WITH(libmng,  [  --without-libmng        build without MNG support])

have_libmng=no
if test "x$with_libmng" != xno && test -z "$MNG_LIBS" &&
   test -n "$PNG_LIBS" && test -n "$JPEG_LIBS" && test -n "$Z_LIBS"; then
  AC_CHECK_LIB(mng, mng_create,
    [AC_CHECK_HEADER(libmng.h, mng_ok=yes)],
    [have_libmng="no (MNG library not found)"], -lz -lm)
  if test "$mng_ok" = yes; then
    have_libmng=yes
    FILE_MNG='file-mng$(EXEEXT)'; MNG_LIBS="-lmng $JPEG_LIBS $PNG_LIBS"; MNG_CFLAGS="$PNG_CFLAGS"
  else
    have_libmng="no (MNG header file not found)"
  fi
fi

AC_SUBST(FILE_MNG)
AC_SUBST(MNG_LIBS)
AC_SUBST(MNG_CFLAGS)


############################################################
# libexif: Library to allow exif tags to be read from, and
# saved to, jpeg files. Currently, this permits exif data to
# avoid destruction, but no data modification is performed.
############################################################

AC_ARG_WITH(libexif,  [  --without-libexif       build without EXIF support])

have_libexif=no
if test "x$with_libexif" != xno && test -z "$EXIF_LIBS" && test -n "$JPEG_LIBS"; then
  have_libexif=yes
  PKG_CHECK_MODULES(EXIF, libexif >= exif_required_version,
    AC_DEFINE(HAVE_LIBEXIF, 1, [Define to 1 if libexif is available]),
    have_libexif="no (libexif not found or too old)")
fi

AC_SUBST(EXIF_CFLAGS)
AC_SUBST(EXIF_LIBS)

AM_CONDITIONAL(HAVE_LIBEXIF, test "x$have_libexif" = xyes)


#################
# Check for libaa
#################

AC_ARG_WITH(aa,  [  --without-aa            do not build the AA plug-in])

have_libaa=no
if test "x$with_aa" != xno && test -z "$AA_LIBS"; then
  have_libaa=yes
  AC_CHECK_LIB(aa, aa_printf,
    [AC_CHECK_HEADER(aalib.h,
      FILE_AA='file-aa$(EXEEXT)'; AA_LIBS='-laa',
      [have_libaa="no (AA header file not found)"])],
    [have_libaa="no (AA library not found)"])
fi

AC_SUBST(FILE_AA)
AC_SUBST(AA_LIBS)


##################
# Check for libxpm
##################

AC_ARG_WITH(libxpm,  [  --without-libxpm        build without XPM support])

picman_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $X_CFLAGS"
picman_save_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $X_LIBS"
if test "$gdk_target" = x11; then
  lib_X11=-lX11
fi
have_libxpm=no
if test "x$with_libxpm" != xno && test -z "$XPM_LIBS"; then
  have_libxpm=yes
  AC_CHECK_LIB(Xpm, XpmReadFileToXpmImage,
    [AC_CHECK_HEADER(X11/xpm.h,
      FILE_XPM='file-xpm$(EXEEXT)'; XPM_LIBS="$X_LIBS $X_PRE_LIBS $lib_X11 -lXpm",
      [have_libxpm="no (XPM header file not found)"])],
    [have_libxpm="no (XPM library not found)"], $X_PRE_LIBS $lib_X11)
fi
CFLAGS="$picman_save_CFLAGS"
LDFLAGS="$picman_save_LDFLAGS"

if test "x$have_libxpm" = xyes; then
  MIME_TYPES="$MIME_TYPES;image/x-xpixmap"
fi

AC_SUBST(FILE_XPM)
AC_SUBST(XPM_LIBS)


##################
# Check for webkit
##################

AC_ARG_WITH(webkit, [  --without-webkit        don't build the help-browser plug-in])

have_webkit=no
if test "x$with_webkit" != xno; then
  PKG_CHECK_MODULES(WEBKIT, webkit-1.0 >= webkit_required_version,
    WEB_PAGE='web-page$(EXEEXT)'
    have_webkit=yes,
    have_webkit="no (WebKit not found)")
else
  have_webkit="no (WebKit support disabled)"
fi

AC_SUBST(WEB_PAGE)
AM_CONDITIONAL(HAVE_WEBKIT, test "x$have_webkit" = xyes)


###################
# Check for librsvg
###################

AC_ARG_WITH(librsvg,[  --without-librsvg       build without librsvg support])

have_librsvg=no
if test "x$with_librsvg" != xno; then
  have_librsvg=yes
  PKG_CHECK_MODULES(SVG, librsvg-2.0 >= rsvg_required_version,
    FILE_SVG='file-svg$(EXEEXT)',
    have_librsvg="no (librsvg not found)")
else
   have_librsvg="no (librsvg support disabled)"
fi

if test "x$have_librsvg" = xyes; then
  MIME_TYPES="$MIME_TYPES;image/svg+xml"
fi

AC_SUBST(FILE_SVG)


####################################
# Allow to disable the print plug-in
####################################

AC_ARG_WITH(print,[  --without-print         build without print support])

enable_print="no (disabled)"
if test "x$with_print" != xno; then
  enable_print="yes"
fi

AM_CONDITIONAL(BUILD_PRINT, test "x$enable_print" = xyes)


######################
# Check for libpoppler
######################

AC_ARG_WITH(poppler,[  --without-poppler       build without poppler support])

have_poppler=no
if test "x$with_poppler" != xno; then
  PKG_CHECK_MODULES(POPPLER,
    poppler-glib >= poppler_required_version,
    FILE_PDF_LOAD='file-pdf-load$(EXEEXT)'
    AC_DEFINE(HAVE_POPPLER, 1, [Define to 1 if libpoppler is available])
    have_poppler=yes,
    have_poppler="Using PostScript plug-in (libpoppler not found)")
else
  have_poppler="Using PostScript plug-in (libpoppler support disabled)"
fi

AC_SUBST(FILE_PDF_LOAD)


#####################
# Check for cairo-pdf
#####################

AC_ARG_WITH(cairo_pdf,[  --without-cairo-pdf     build without cairo-pdf support])
have_cairo_pdf=no
if test "x$with_cairo_pdf" != xno; then
  have_cairo_pdf=yes
  PKG_CHECK_MODULES(CAIRO_PDF, cairo-pdf >= cairo_pdf_required_version,
    FILE_PDF_SAVE='file-pdf-save$(EXEEXT)',
    have_cairo_pdf="no (cairo-pdf not found)")
else
   have_cairo_pdf="no (cairo-pdf support disabled)"
fi

AC_SUBST(FILE_PDF_SAVE)

## If both poppler and cairo-pdf are enabled then we have complete PDF support
if test "x$have_cairo_pdf" = xyes && test "x$have_poppler" = xyes; then
  MIME_TYPES="$MIME_TYPES;application/pdf"
fi


################################
# Check for gio/gvfs and libcurl
################################

uri_plugin=no


AC_ARG_WITH(gvfs, [  --without-gvfs          build without GIO/GVfs support])

AM_CONDITIONAL(HAVE_GVFS, test "x$with_gvfs" != xno)


AC_ARG_WITH(libcurl,  [  --without-libcurl       build without curl support])

have_libcurl="no (disabled)"

if test "x$with_libcurl" != xno; then

PKG_CHECK_MODULES(CURL, libcurl >= libcurl_required_version,
  have_libcurl=yes,
  have_libcurl="no (libcurl not found)")

fi

AM_CONDITIONAL(HAVE_LIBCURL, test "x$have_libcurl" = xyes)


if test "x$with_gvfs" != xno; then
  uri_plugin="yes (using GIO/GVfs)"
elif test "x$have_libcurl" = xyes; then
  uri_plugin="yes (using libcurl)"
elif test "x$os_win32" = xno; then
  uri_plugin="yes (using wget)"
fi


###################
# Check for libwmf2
###################

AC_ARG_WITH(wmf, [  --without-wmf           build without WMF support])

have_libwmf=no
if test "x$with_wmf" != xno; then
  AC_PATH_PROG(WMF_CONFIG, libwmf-config, no)
  if test "x$WMF_CONFIG" != "xno" ; then
    have_libwmf=yes
    AC_MSG_CHECKING(for libwmf >= wmf_required_version)
    wmf_version=`$WMF_CONFIG --version`
    if expr $wmf_version \>= wmf_required_version >/dev/null; then
      AC_MSG_RESULT([yes (version $wmf_version)])
      FILE_WMF='file-wmf$(EXEEXT)'
      WMF_LIBS=`$WMF_CONFIG --libs`
      WMF_CFLAGS=`$WMF_CONFIG --cflags`
    else
      have_libwmf="no (libwmf is too old)"
   fi
  else
    have_libwmf="no (libwmf not found)"
  fi
fi

if test "x$have_libwmf" = xyes; then
  MIME_TYPES="$MIME_TYPES;image/x-wmf"
fi

AC_SUBST(FILE_WMF)
AC_SUBST(WMF_LIBS)
AC_SUBST(WMF_CFLAGS)


#####################
# Check for libjasper
#####################

AC_ARG_WITH(libjasper,  [  --without-libjasper     build without JPEG-2000 support])

have_jp2=no
if test "x$with_libjasper" != xno && test -z "$JASPER_LIBS"; then
  have_jp2=yes
  AC_CHECK_LIB(jasper, jas_init,
    FILE_JP2_LOAD='file-jp2-load$(EXEEXT)' ; JP2_LIBS=-ljasper,
    [have_jp2="no (JasPer library not found)"
    AC_MSG_WARN(*** JP2 plug-in will not be built (JasPer library not found) ***)])
else
   have_jp2="no (JasPer support disabled)"
fi

if test "x$have_jp2" = xyes; then
  MIME_TYPES="$MIME_TYPES;image/jp2;image/jpeg2000;image/jpx"
fi

AM_CONDITIONAL(BUILD_JP2, test "x$have_jp2" = xyes)
AC_SUBST(FILE_JP2_LOAD)
AC_SUBST(JP2_LIBS)


################
# Check for lcms
################

AC_ARG_WITH(lcms, [  --without-lcms          build without lcms support])

have_lcms="no (lcms support disabled)"
if test "x$with_lcms" != xno; then
  have_lcms=yes
  PKG_CHECK_MODULES(LCMS, lcms2 >= lcms_required_version,
    AC_DEFINE(HAVE_LCMS, 1, [Define to 1 if lcms is available])
    LCMS='lcms$(EXEEXT)',
    have_lcms="no (lcms not found or unusable)")
fi

AC_SUBST(LCMS)
AM_CONDITIONAL(HAVE_LCMS, test "x$have_lcms" = xyes)


######################
# Check for libXcursor
######################

AC_ARG_WITH(xmc,[  --without-xmc           build without Xcursor support])

have_xmc=no
if test "x$with_xmc" != xno; then
  have_xmc=yes
  PKG_CHECK_MODULES(XMC, xcursor,
    FILE_XMC='file-xmc$(EXEEXT)',
    have_xmc="no (libXcursor not found)")
else
   have_xmc="no (libXcursor support disabled)"
fi

if test "x$have_xmc" = xyes; then
  MIME_TYPES="$MIME_TYPES;image/x-xcursor"
fi

AC_SUBST(FILE_XMC)


################
# Check for alsa
################

AC_ARG_WITH(alsa, [  --without-alsa          disable alsa support in midi input controller])

have_alsa="no (alsa support disabled)"
if test "x$with_alsa" != xno; then
  have_alsa=yes
  AM_PATH_ALSA(alsa_required_version,
               AC_DEFINE(HAVE_ALSA, 1, [Define to 1 if alsa is available]),
               have_alsa="no (libasound not found or unusable)")
fi


#######################
# Check for Linux Input
#######################

AC_ARG_WITH(linux-input, [  --without-linux-input   don't build linux input event controller module])

have_linux_input="no (linux input support disabled)"
if test "x$with_linux_input" != "xno"; then
  AC_CHECK_HEADER(linux/input.h,
	AC_CHECK_DECL(KEY_OK,
		have_linux_input=yes,
		have_linux_input="no (needs Linux 2.6)",
		[#include <linux/input.h>]))
fi

AM_CONDITIONAL(HAVE_LINUX_INPUT, test "x$have_linux_input" = xyes)


###############################
# Check for DirectX DirectInput
###############################

AC_ARG_WITH(directx-sdk, [  --with-directx-sdk=PFX  prefix where the DirectX SDK is installed(optional)])

have_dx_dinput=no
if test "x$os_win32" = xyes; then
  case "$with_directx_sdk" in
    *" "*) AC_MSG_WARN([The DirectX SDK should be accessible through a path without spaces. Use MSYS mounts. Ignoring your --with-directx-sdk switch.])
           with_directx_sdk=;;
    *\\*) AC_MSG_WARN([Use plain (forward) slashes only in your --with-directx-sdk switch. Ignoring your --with-directx-sdk switch.])
           with_directx_sdk=;;
  esac

  dinput_save_CPPFLAGS="$CPPFLAGS"
  dinput_save_LDFLAGS="$LDFLAGS"
  if test "x$with_directx_sdk" != x; then
    CPPFLAGS="-I $with_directx_sdk/Include $CPPFLAGS"
    LDFLAGS="-L$with_directx_sdk/Lib/x86 $LDFLAGS"
    AC_CHECK_HEADER(dinput.h,
                    [AC_CHECK_LIB(dxguid, GUID_XAxis, have_dx_dinput=yes)])
  fi

  if test "x$have_dx_dinput" = xyes; then
    AC_DEFINE(HAVE_DX_DINPUT, 1, [Define to 1 if you have DirectX DirectInput headers and libraries])
  else
    CPPFLAGS="$dinput_save_CPPFLAGS"
    LDFLAGS="$dinput_save_LDFLAGS"
  fi
fi

AM_CONDITIONAL(HAVE_DX_DINPUT, test "x$have_dx_dinput" = xyes)


###############################
# Check for D-Bus GLib bindings
###############################

AC_ARG_WITH(dbus,  [  --without-dbus          build without D-Bus support])

have_dbus_glib="no (disabled)"

if test "x$with_dbus" != xno; then
  PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1 >= dbus_glib_required_version,
    have_dbus_glib=yes,
    have_dbus_glib=no)
fi

if test "x$have_dbus_glib" = xyes; then
  AC_PATH_PROG(DBUS_BINDING_TOOL, dbus-binding-tool, no)
  if test "x$DBUS_BINDING_TOOL" = xno; then
    AC_MSG_WARN([Could not find dbus-binding-tool in your PATH])
    have_dbus_glib="no (dbus-binding-tool not found)"
  fi
fi

if test "x$have_dbus_glib" = xyes; then
  AC_DEFINE(HAVE_DBUS_GLIB, 1,
            [Define to 1 if D-Bus GLib bindings are available])
  PICMAN_COMMAND='picman-picman_app_version'
fi


####################
# Check for libgudev
####################

AC_ARG_WITH(gudev, [  --without-gudev         build without GUdev support])

have_libgudev=no

if test "x$with_gudev" = xno; then
  have_libgudev="no (disabled)"
fi

if test "x$have_linux_input" = xyes && test "x$have_dbus_glib" = xyes &&
   test "x$with_gudev" != xno; then
  PKG_CHECK_MODULES(GUDEV, gudev-1.0 >= libgudev_required_version,
    have_libgudev=yes,
    have_libgudev="no (libgudev not found)")
fi

if test "x$have_libgudev" = xyes; then
  AC_DEFINE(HAVE_LIBGUDEV, 1, [Define to 1 if libgudev is available])
fi


############################################################
# GIF compression: Allow builder to specify at compile time
# which compression style she wants to use for GIFs.
# LZW: Unisys patented Lempel-Ziff encoding.
# RLE: Runlength encoding (produces files about twice as big
#			   as the LZW compressed equivalent).
# None: Raw data.
############################################################

AC_ARG_WITH(gif-compression,
            [  --with-gif-compression=lzw|rle|none     GIF compression style (default=lzw)])

case x"$with_gif_compression" in
  xnone)
    AC_DEFINE(GIF_UN, 1, [Define to 1 for no GIF compression]) ;;
  xrle)
    AC_DEFINE(GIF_RLE, 1, [Define to 1 for RLE GIF compression]) ;;
esac


####################
# Check for sendmail
####################

AC_ARG_WITH(sendmail,[  --with-sendmail=DIR     set sendmail command location])

if test "x$with_sendmail" != "x"; then
  sendmail_path=$with_sendmail
else
  sendmail_path=$PATH:/usr/sbin:/usr/lib
fi

AC_PATH_PROG(SENDMAIL, sendmail, , $sendmail_path)
if test "x$SENDMAIL" != "x"; then
  AC_DEFINE_UNQUOTED(SENDMAIL, "$SENDMAIL",
	             [The MTA used by the mail plug-in.])
fi


################
# Check for perl
################

AC_PATH_PROGS(PERL,perl5 perl perl5.005 perl5.004,perl)


AC_ARG_WITH(pdbgen,
            [  --with-pdbgen           use 'pdbgen' code generation tool],,
  with_pdbgen=$USE_MAINTAINER_MODE)

AM_CONDITIONAL(WITH_PDBGEN, test "x$with_pdbgen" = xyes)


##################
# Check for python
##################

# Pypicman configure stuff ...
AC_ARG_ENABLE(python,
  AS_HELP_STRING([--disable-python],
                 [do not build the python extension]))

m4_define([pycheck_error], [
*** Could not find $1.
*** Please install $2, or skip building the python scripting extension by
*** passing --disable-python to configure (but then you will not be able
*** to use scripts for PICMAN that are written in Python).])

if test "x$enable_python" != xno; then
  enable_python="yes"
  # check for Python
  AM_PATH_PYTHON(2.5.0,,
    [AC_MSG_ERROR([pycheck_error([Python 2.5.0 or newer], [it])])])
  AM_CHECK_PYTHON_HEADERS(,
    [AC_MSG_ERROR([pycheck_error([Python headers], [them])])])

  # Win32 needs all symbols resolved for linking, even for DLLs
  # Assume the link library is in $exec_prefix/libs
  # This might be able to be figured out from distutils, but it's
  # not documented so we won't rely on internal implementation
  PYLINK_LIBS=
  if test "x$platform_win32" = "xyes"; then
    PYBIN_PATH="$py_exec_prefix\pythonw.exe"
    PYTHON_INCLUDES=`echo "$PYTHON_INCLUDES" | sed -e 's/\\\\/\\//g'`
    py_exec_prefix=`echo "$py_exec_prefix" | sed -e 's/\\\\/\\//g'`
    pylibversion=`echo $PYTHON_VERSION | sed -e 's/\\.//'`
    PYLINK_LIBS="-L${py_exec_prefix}/libs -lpython${pylibversion}"
  else
    PYBIN_PATH="$PYTHON"
  fi
  AC_SUBST(PYLINK_LIBS)
  AC_SUBST(PYBIN_PATH)

  # check for PyGTK
  PKG_CHECK_MODULES(PYGTK, pygtk-2.0 >= pygtk_required_version,,
    [AC_MSG_ERROR([pycheck_error([PyGTK pygtk_required_version or newer], [it])])])

  AC_PATH_PROG(PYGTK_CODEGEN, pygtk-codegen-2.0, no)
  if test "x$PYGTK_CODEGEN" = xno; then
    AC_MSG_ERROR([pycheck_error([pygtk-codegen-2.0 script], [it])])
  fi

  AC_MSG_CHECKING(for pygtk defs)
  PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
  AC_SUBST(PYGTK_DEFSDIR)
  AC_MSG_RESULT($PYGTK_DEFSDIR)

  PICMAN_DETECT_CFLAGS(PYPICMAN_EXTRA_CFLAGS, '-fno-strict-aliasing')
  AC_SUBST(PYPICMAN_EXTRA_CFLAGS)
fi
AM_CONDITIONAL(BUILD_PYTHON, test "x$enable_python" != xno)


#########################################
# Check whether Script-Fu should be built
#########################################

have_script_fu=yes
AC_ARG_WITH(script-fu,
  AS_HELP_STRING([--without-script-fu],
                 [do not build the script-fu extension]))

if test "x$with_script_fu" = "xno"; then
  have_script_fu="no (script-fu extension disabled)"
fi
AM_CONDITIONAL(BUILD_SCRIPT_FU, test "x$with_script_fu" != "xno")


###########################################################
# Some plug-ins don't build on Win32, others are Win32-only
###########################################################

if test "x$os_win32" = xno; then
  MAIL='mail$(EXEEXT)'
  SCREENSHOT='screenshot$(EXEEXT)'
fi

AC_SUBST(MAIL)
AC_SUBST(SCREENSHOT)


####################
# Check for Mac OS X
####################

platform_osx=no
AC_MSG_CHECKING([if compiling for Mac OS X])
case "$target_or_host" in
  *-*-darwin*)
     AC_MSG_RESULT(yes)
     AC_DEFINE(PLATFORM_OSX, 1, [define to 1 if compiling for Mac OS X])
     platform_osx=yes
     ;;
  *)
     AC_MSG_RESULT(no)
     ;;
esac

AM_CONDITIONAL(PLATFORM_OSX, test "x$platform_osx" = xyes)


####################################
# Check for Mac OS X TWAIN framework
####################################

AC_ARG_WITH(mac-twain,  [  --without-mac-twain     don't build the Mac TWAIN plug-in])

mac_twain_ok=no
if test "x$with_mac_twain" != xno; then
  AC_MSG_CHECKING([for Mac OS X TWAIN support])
  AC_PREPROC_IFELSE(
    [AC_LANG_SOURCE([[
#include <Carbon/Carbon.h>
#include <TWAIN/TWAIN.h>
#ifdef __x86_64__
#error Please somebody port this to non-deprecated and non-removed APIs
#endif]])],
    [mac_twain_ok=yes])
  AC_MSG_RESULT($mac_twain_ok)
fi

AM_CONDITIONAL(HAVE_MAC_TWAIN, test "x$mac_twain_ok" = xyes)


##########################################################
# Determine where to install the desktop & mime info files
##########################################################

AC_ARG_WITH(desktop-dir, [  --with-desktop-dir=PATH where to install desktop files (default=PREFIX/share)])

if test "x$with_desktop_dir" != x; then
  DESKTOP_DATADIR=$with_desktop_dir
else
  DESKTOP_DATADIR="$datadir"
fi

AM_CONDITIONAL(DESKTOP_DATADIR, test "x$with_desktop_dir" != xno)
AC_SUBST(DESKTOP_DATADIR)


#####################
# Check for XML tools
#####################

AC_PATH_PROG(XMLLINT, xmllint, no)
AM_CONDITIONAL(HAVE_XMLLINT, test "x$XMLLINT" != "xno")

AC_PATH_PROG(XSLTPROC, xsltproc, no)
AM_CONDITIONAL(HAVE_XSLTPROC, test "x$XSLTPROC" != "xno")


####################
# Check for xvfb-run
####################

AC_ARG_WITH(xvfb-run,  [  --without-xvfb-run      do not use xvfb-run for UI-dependent automatic tests])

have_xvfb_run="no (disabled)"
if test "x$with_xvfb_run" != "xno"; then
  AC_PATH_PROG(XVFB_RUN, xvfb-run, no)
  if test "x$XVFB_RUN" != "xno"; then
    have_xvfb_run="yes"
  else
    have_xvfb_run="no (not found)"
  fi
fi
AM_CONDITIONAL(HAVE_XVFB_RUN, test "x$have_xvfb_run" == "xyes")


######################################
# Checks for gtk-doc and docbook-tools
######################################

# Check for GTK_DOC_CHECK availability. The GTK_DOC_CHECK invocation
# must be on its own line, gtkdocize relies on it
m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.0])
])

# NOTE: We need to use a separate automake conditional for this
#       to make this work with the tarballs.
AM_CONDITIONAL(ENABLE_GTK_DOC, test "x$enable_gtk_doc" = xyes)

AC_ARG_ENABLE(gtk-doc-app, [  --enable-gtk-doc-app    build developer documentation for app (default=no)], , enable_gtk_doc_app=no)
AM_CONDITIONAL(ENABLE_GTK_DOC_APP, test "x$enable_gtk_doc_app" = xyes)


#########################################
# Allow to configure the PICMAN environment
#########################################

AC_ARG_ENABLE(default-binary, [  --enable-default-binary install this as the default picman binary (default=picman_stable)], , enable_default_binary=picman_stable)

AM_CONDITIONAL(DEFAULT_BINARY, test "x$enable_default_binary" = xyes)


AC_ARG_ENABLE(picman-console, [  --enable-picman-console   build a console-only binary which does not link GTK+], , enable_picman_console=yes)

AM_CONDITIONAL(ENABLE_PICMAN_CONSOLE, test "x$enable_picman_console" != xno)


# Possibly change default picmandir from .picman-major.minor
picmandir=PICMAN
AC_ARG_WITH(picmandir, [  --with-picmandir=DIR      change default picmandir from $XDG_CONFIG_HOME/PICMAN/picman_user_version to $XDG_CONFIG_HOME/DIR/picman_user_version],
if eval "test x$with_picmandir != x"; then
  if eval "test x$with_picmandir != xyes"; then
    picmandir=$with_picmandir
  fi
fi)

picmandatadir="$datadir/$PACKAGE/picman_data_version"
picmanplugindir="$libdir/$PACKAGE/picman_plugin_version"
picmansysconfdir="$sysconfdir/$PACKAGE/picman_sysconf_version"

AC_SUBST(picmandir)
AC_SUBST(picmandatadir)
AC_SUBST(picmanplugindir)
AC_SUBST(picmansysconfdir)


# GNU format message catalogs go into $datadir/locale,
# standard location is $libdir/locale. DATADIRNAME set accordingly.

picmanlocaledir="$datadir/locale"
if test "$DATADIRNAME" = "lib"; then
   picmanlocaledir="$libdir/locale"
fi

AC_SUBST(picmanlocaledir)


############################
# Some defines for the build
############################

AM_CONDITIONAL(STATICLIBS, test "x$enable_static" = xyes)

# a tuned version of glib-mkenums
PICMAN_MKENUMS="\$(PERL) \$(top_srcdir)/tools/picman-mkenums"
AC_SUBST(PICMAN_MKENUMS)

#Add required trailing ; to list
MIME_TYPES="$MIME_TYPES;"

AC_SUBST(MIME_TYPES)


#########################
# Disable deprecated APIs
#########################

CPPFLAGS="${CPPFLAGS} -DPICMAN_DISABLE_DEPRECATED -DBABL_DISABLE_DEPRECATED -DGSEAL_ENABLE"

# Make sure not to disable deprecated APIs for unreleased versions.
# We must build without problems with future releases of libraries
# and disabling deprecated API risks breaking the build

if test "x$have_glib_2_31" != "xyes"; then
  CPPFLAGS="${CPPFLAGS} -DG_DISABLE_DEPRECATED"
fi

if test "x$have_gtk_2_26" != "xyes"; then
  CPPFLAGS="${CPPFLAGS} -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"
fi

if test "x$have_gdk_pixbuf_2_26" != "xyes"; then
  CPPFLAGS="${CPPFLAGS} -DGDK_PIXBUF_DISABLE_DEPRECATED"
fi

if test "x$have_pango_1_32" != "xyes"; then
  CPPFLAGS="${CPPFLAGS} -DPANGO_DISABLE_DEPRECATED"
fi


############################
# Require multihead safe API
############################

CPPFLAGS="${CPPFLAGS} -DGDK_MULTIHEAD_SAFE -DGTK_MULTIHEAD_SAFE"


##################
# configure output
##################

# easy way to skip the plug-in build
PICMAN_PLUGINS=plug-ins
PICMAN_MODULES=modules
AC_SUBST(PICMAN_PLUGINS)
AC_SUBST(PICMAN_MODULES)

AC_CONFIG_COMMANDS([libpicmanbase/picmanversion.h], [
  outfile=picmanversion.h-tmp
  cat > $outfile <<\_______EOF
/* picmanversion.h
 *
 * This is a generated file.  Please modify 'configure.ac'
 */

#if !defined (__PICMAN_BASE_H_INSIDE__) && !defined (PICMAN_BASE_COMPILATION)
#error "Only <libpicmanbase/picmanbase.h> can be included directly."
#endif

#ifndef __PICMAN_VERSION_H__
#define __PICMAN_VERSION_H__

G_BEGIN_DECLS


/**
 * SECTION: picmanversion
 * @title: picmanversion
 * @short_description: Macros and constants useful for determining
 *                     PICMAN's version number and capabilities.
 *
 * Macros and constants useful for determining PICMAN's version number and
 * capabilities.
 **/

_______EOF

  cat >>$outfile <<_______EOF
/**
 * PICMAN_MAJOR_VERSION:
 *
 * The major PICMAN version number.
 **/
#define PICMAN_MAJOR_VERSION                              ($picman_major_ver)

/**
 * PICMAN_MINOR_VERSION:
 *
 * The minor PICMAN version number.
 **/
#define PICMAN_MINOR_VERSION                              ($picman_minor_ver)

/**
 * PICMAN_MICRO_VERSION:
 *
 * The micro PICMAN version number.
 **/
#define PICMAN_MICRO_VERSION                              ($picman_micro_ver)

/**
 * PICMAN_VERSION:
 *
 * The PICMAN version as a string.
 **/
#define PICMAN_VERSION                                    "$picman_ver"

/**
 * PICMAN_API_VERSION:
 *
 * Since: PICMAN 2.2
 **/
#define PICMAN_API_VERSION                                "$picman_api_ver"

_______EOF

  cat >>$outfile <<\_______EOF
#define PICMAN_CHECK_VERSION(major, minor, micro) \
    (PICMAN_MAJOR_VERSION > (major) || \
     (PICMAN_MAJOR_VERSION == (major) && PICMAN_MINOR_VERSION > (minor)) || \
     (PICMAN_MAJOR_VERSION == (major) && PICMAN_MINOR_VERSION == (minor) && \
      PICMAN_MICRO_VERSION >= (micro)))


G_END_DECLS

#endif /* __PICMAN_VERSION_H__ */
_______EOF

  if cmp -s $outfile libpicmanbase/picmanversion.h; then
    AC_MSG_NOTICE([libpicmanbase/picmanversion.h is unchanged])
    rm -f $outfile
  else
    mv $outfile libpicmanbase/picmanversion.h
  fi
],[
  picman_major_ver=picman_major_version
  picman_minor_ver=picman_minor_version
  picman_micro_ver=picman_micro_version
  picman_ver=picman_version
  picman_api_ver=picman_api_version
])

AC_CONFIG_FILES([
tools/Makefile
tools/pdbgen/Makefile
libpicman/Makefile
libpicmanbase/Makefile
libpicmanconfig/Makefile
libpicmancolor/Makefile
libpicmanmath/Makefile
libpicmanmodule/Makefile
libpicmanthumb/Makefile
libpicmanwidgets/Makefile
app/Makefile
app/actions/Makefile
app/config/Makefile
app/core/Makefile
app/operations/Makefile
app/operations/tests/Makefile
app/gegl/Makefile
app/dialogs/Makefile
app/display/Makefile
app/file/Makefile
app/gui/Makefile
app/menus/Makefile
app/paint/Makefile
app/pdb/Makefile
app/plug-in/Makefile
app/text/Makefile
app/tools/Makefile
app/vectors/Makefile
app/widgets/Makefile
app/xcf/Makefile
app/tests/Makefile
app/tests/files/Makefile
app/tests/picmandir-empty/Makefile
app/tests/picmandir-empty/brushes/Makefile
app/tests/picmandir-empty/gradients/Makefile
app/tests/picmandir-empty/patterns/Makefile
app/tests/picmandir/Makefile
app/tests/picmandir/brushes/Makefile
app/tests/picmandir/gradients/Makefile
app/tests/picmandir/patterns/Makefile
build/Makefile
build/windows/Makefile
build/windows/picman.rc
build/windows/picman-plug-ins.rc
plug-ins/Makefile
plug-ins/color-rotate/Makefile
plug-ins/color-rotate/images/Makefile
plug-ins/file-bmp/Makefile
plug-ins/file-compressor/Makefile
plug-ins/file-faxg3/Makefile
plug-ins/file-fits/Makefile
plug-ins/file-fli/Makefile
plug-ins/file-ico/Makefile
plug-ins/file-jpeg/Makefile
plug-ins/file-psd/Makefile
plug-ins/file-sgi/Makefile
plug-ins/file-uri/Makefile
plug-ins/flame/Makefile
plug-ins/fractal-explorer/Makefile
plug-ins/fractal-explorer/examples/Makefile
plug-ins/gfig/Makefile
plug-ins/gfig/gfig-examples/Makefile
plug-ins/gfig/images/Makefile
plug-ins/ui/Makefile
plug-ins/picmanressionist/Brushes/Makefile
plug-ins/picmanressionist/Makefile
plug-ins/picmanressionist/Paper/Makefile
plug-ins/picmanressionist/Presets/Makefile
plug-ins/gradient-flare/Makefile
plug-ins/gradient-flare/flares/Makefile
plug-ins/help-browser/Makefile
plug-ins/help/Makefile
plug-ins/ifs-compose/Makefile
plug-ins/imagemap/Makefile
plug-ins/imagemap/images/Makefile
plug-ins/lighting/Makefile
plug-ins/lighting/images/Makefile
plug-ins/map-object/Makefile
plug-ins/maze/Makefile
plug-ins/metadata/Makefile
plug-ins/pagecurl/Makefile
plug-ins/print/Makefile
plug-ins/pypicman/Makefile
plug-ins/pypicman/plug-ins/Makefile
plug-ins/script-fu/Makefile
plug-ins/script-fu/ftx/Makefile
plug-ins/script-fu/scripts/Makefile
plug-ins/script-fu/scripts/images/Makefile
plug-ins/script-fu/tinyscheme/Makefile
plug-ins/selection-to-path/Makefile
plug-ins/twain/Makefile
plug-ins/win-snap/Makefile
plug-ins/common/Makefile
modules/Makefile
devel-docs/Makefile
devel-docs/app/Makefile
devel-docs/app/version
devel-docs/libpicman/Makefile
devel-docs/libpicman/version
devel-docs/libpicmanbase/Makefile
devel-docs/libpicmanbase/version
devel-docs/libpicmanconfig/Makefile
devel-docs/libpicmanconfig/version
devel-docs/libpicmancolor/Makefile
devel-docs/libpicmancolor/version
devel-docs/libpicmanmath/Makefile
devel-docs/libpicmanmath/version
devel-docs/libpicmanmodule/Makefile
devel-docs/libpicmanmodule/version
devel-docs/libpicmanthumb/Makefile
devel-docs/libpicmanthumb/version
devel-docs/libpicmanwidgets/Makefile
devel-docs/libpicmanwidgets/version
devel-docs/tools/Makefile
docs/Makefile
menus/Makefile
cursors/Makefile
themes/Makefile
themes/Default/Makefile
themes/Default/images/Makefile
themes/Default/images/preferences/Makefile
themes/Small/Makefile
data/Makefile
data/brushes/Makefile
data/brushes/Basic/Makefile
data/brushes/picman-obsolete-files/Makefile
data/brushes/Media/Makefile
data/brushes/Legacy/Makefile
data/brushes/Sketch/Makefile
data/brushes/Splatters/Makefile
data/brushes/Texture/Makefile
data/dynamics/Makefile
data/dynamics/Basic/Makefile
data/dynamics/FX/Makefile
data/environ/Makefile
data/gradients/Makefile
data/images/Makefile
data/interpreters/Makefile
data/palettes/Makefile
data/patterns/Makefile
data/tags/Makefile
data/tips/Makefile
data/tool-presets/Makefile
data/tool-presets/FX/Makefile
data/tool-presets/Paint/Makefile
data/tool-presets/Selection/Makefile
data/tool-presets/Sketch/Makefile
desktop/Makefile
desktop/picman.desktop.in
etc/Makefile
m4macros/Makefile
po/Makefile.in
po-libpicman/Makefile.in
po-plug-ins/Makefile.in
po-python/Makefile.in
po-script-fu/Makefile.in
po-tips/Makefile.in
picman-zip
Makefile
])

# Files with versions in their names
AC_CONFIG_FILES(
docs/picman-picman_app_version.1:docs/picman.1.in
docs/picmanrc-picman_app_version.5:docs/picmanrc.5.in
docs/picmantool-picman_tool_version.1:docs/picmantool.1.in
picman-picman_pkgconfig_version.pc:picman.pc.in
picmanthumb-picman_pkgconfig_version.pc:picmanthumb.pc.in
picmanui-picman_pkgconfig_version.pc:picmanui.pc.in
)

AC_CONFIG_COMMANDS([sed-po-makefiles],
[sed -e "/POTFILES =/r po-libpicman/POTFILES" po-libpicman/Makefile.in > po-libpicman/Makefile && touch po-libpicman/stamp-it
sed -e "/POTFILES =/r po-python/POTFILES" po-python/Makefile.in > po-python/Makefile && touch po-python/stamp-it
sed -e "/POTFILES =/r po-plug-ins/POTFILES" po-plug-ins/Makefile.in > po-plug-ins/Makefile && touch po-plug-ins/stamp-it
sed -e "/POTFILES =/r po-script-fu/POTFILES" po-script-fu/Makefile.in > po-script-fu/Makefile && touch po-script-fu/stamp-it
sed -e "/POTFILES =/r po-tips/POTFILES" po-tips/Makefile.in > po-tips/Makefile && touch po-tips/stamp-it])

AC_OUTPUT

# Print a summary of features enabled/disabled:
AC_MSG_RESULT([
Building PICMAN with prefix=$prefix, datarootdir=$datarootdir
Desktop files install into $DESKTOP_DATADIR

Extra Binaries:
  picman-console:        $enable_picman_console

Optional Features:
  D-Bus service:       $have_dbus_glib
  Language selection:  $have_iso_codes

Optional Plug-Ins:
  Ascii Art:           $have_libaa
  Compressor:          $have_compressor
  Ghostscript:         $have_gs
  Help Browser:        $have_webkit
  LCMS:                $have_lcms
  JPEG:                $jpeg_ok
  JPEG 2000:           $have_jp2
  MNG:                 $have_libmng
  PDF (import):        $have_poppler
  PDF (export):        $have_cairo_pdf
  PNG:                 $have_libpng
  Print:               $enable_print
  PSP:                 $have_zlib
  Python:              $enable_python
  Script-Fu:           $have_script_fu
  SVG:                 $have_librsvg
  TIFF:                $have_libtiff
  TWAIN (MacOS X):     $mac_twain_ok
  TWAIN (Win32):       $os_win32
  URI:                 $uri_plugin
  Webpage:             $have_webkit
  Windows ICO:         $have_libpng
  WMF:                 $have_libwmf
  X11 Mouse Cursor:    $have_xmc
  XPM:                 $have_libxpm

Plug-In Features:
  EXIF support:        $have_libexif

Optional Modules:
  ALSA (MIDI Input):   $have_alsa
  Linux Input:         $have_linux_input (GUdev support: $have_libgudev)
  DirectInput (Win32): $have_dx_dinput
  Color Correction:    $have_lcms
  Soft Proof:          $have_lcms

Tests:
  Use xvfb-run         $have_xvfb_run
]);
