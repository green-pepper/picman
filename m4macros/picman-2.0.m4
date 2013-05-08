# Configure paths for PICMAN-2.0
# Manish Singh, Sven Neumann
# Large parts shamelessly stolen from Owen Taylor

dnl AM_PATH_PICMAN_2_0([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for PICMAN, and define PICMAN_CFLAGS and PICMAN_LIBS
dnl
AC_DEFUN([AM_PATH_PICMAN_2_0],
[dnl 
dnl Get the cflags and libraries from pkg-config
dnl

AC_ARG_ENABLE(picmantest, [  --disable-picmantest      do not try to compile and run a test PICMAN program],, enable_picmantest=yes)

  pkg_name=picman-2.0
  pkg_config_args="$pkg_name picmanui-2.0"

  no_picman=""

  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

  if test x$PKG_CONFIG != xno ; then
    if pkg-config --atleast-pkgconfig-version 0.7 ; then
      :
    else
      echo *** pkg-config too old; version 0.7 or better required.
      no_picman=yes
      PKG_CONFIG=no
    fi
  else
    no_picman=yes
  fi

  min_picman_version=ifelse([$1], ,2.0.0,$1)
  AC_MSG_CHECKING(for PICMAN - version >= $min_picman_version)

  if test x$PKG_CONFIG != xno ; then
    ## don't try to run the test against uninstalled libtool libs
    if $PKG_CONFIG --uninstalled $pkg_config_args; then
	  echo "Will use uninstalled version of PICMAN found in PKG_CONFIG_PATH"
	  enable_picmantest=no
    fi

    if $PKG_CONFIG --atleast-version $min_picman_version $pkg_config_args; then
	  :
    else
	  no_picman=yes
    fi
  fi

  if test x"$no_picman" = x ; then
    PICMAN_CFLAGS=`$PKG_CONFIG $pkg_config_args --cflags`
    PICMAN_LIBS=`$PKG_CONFIG $pkg_config_args --libs`
    PICMAN_CFLAGS_NOUI=`$PKG_CONFIG $pkg_name --cflags`
    PICMAN_LIBS_NOUI=`$PKG_CONFIG $pkg_name --libs`
    PICMAN_DATA_DIR=`$PKG_CONFIG $pkg_name --variable=picmandatadir`
    PICMAN_PLUGIN_DIR=`$PKG_CONFIG $pkg_name --variable=picmanlibdir`

    picman_pkg_major_version=`$PKG_CONFIG --modversion $pkg_name | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    picman_pkg_minor_version=`$PKG_CONFIG --modversion $pkg_name | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    picman_pkg_micro_version=`$PKG_CONFIG --modversion $pkg_name | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_picmantest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $PICMAN_CFLAGS"
      LIBS="$PICMAN_LIBS $LIBS"

dnl
dnl Now check if the installed PICMAN is sufficiently new. (Also sanity
dnl checks the results of pkg-config to some extent
dnl
      rm -f conf.picmantest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>

#include <libpicman/picman.h>

PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc */
  NULL,  /* quit_proc */
  NULL,  /* query_proc */
  NULL   /* run_proc */
};

int main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.picmantest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_picman_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_picman_version");
     exit(1);
   }

    if (($picman_pkg_major_version > major) ||
        (($picman_pkg_major_version == major) && ($picman_pkg_minor_version > minor)) ||
        (($picman_pkg_major_version == major) && ($picman_pkg_minor_version == minor) && ($picman_pkg_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'pkg-config --modversion %s' returned %d.%d.%d, but the minimum version\n", "$pkg_name", $picman_pkg_major_version, $picman_pkg_minor_version, $picman_pkg_micro_version);
      printf("*** of PICMAN required is %d.%d.%d. If pkg-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If pkg-config was wrong, set the environment variable PKG_CONFIG_PATH\n");
      printf("*** to point to the correct the correct configuration files\n");
      return 1;
    }
}

],, no_picman=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_picman" = x ; then
     AC_MSG_RESULT(yes (version $picman_pkg_major_version.$picman_pkg_minor_version.$picman_pkg_micro_version))
     ifelse([$2], , :, [$2])     
  else
     if test "$PKG_CONFIG" = "no" ; then
       echo "*** A new enough version of pkg-config was not found."
       echo "*** See http://www.freedesktop.org/software/pkgconfig/"
     else
       if test -f conf.picmantest ; then
        :
       else
          echo "*** Could not run PICMAN test program, checking why..."
          CFLAGS="$CFLAGS $PICMAN_CFLAGS"
          LIBS="$LIBS $PICMAN_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <libpicman/picman.h>

PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc */
  NULL,  /* quit_proc */
  NULL,  /* query_proc */
  NULL   /* run_proc */
};
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding PICMAN or finding the wrong"
          echo "*** version of PICMAN. If it is not finding PICMAN, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occurred. This usually means PICMAN is incorrectly installed."])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     PICMAN_CFLAGS=""
     PICMAN_LIBS=""
     PICMAN_CFLAGS_NOUI=""
     PICMAN_LIBS_NOUI=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(PICMAN_CFLAGS)
  AC_SUBST(PICMAN_LIBS)
  AC_SUBST(PICMAN_CFLAGS_NOUI)
  AC_SUBST(PICMAN_LIBS_NOUI)
  AC_SUBST(PICMAN_DATA_DIR)
  AC_SUBST(PICMAN_PLUGIN_DIR)
  rm -f conf.picmantest
])
