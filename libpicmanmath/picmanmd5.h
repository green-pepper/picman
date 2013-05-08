/* LIBPICMAN - The PICMAN Library
 *
 * picmanmd5.h
 *
 * Use of this code is deprecated! Use %GChecksum from GLib instead.
 */

#if !defined (__PICMAN_MATH_H_INSIDE__) && !defined (PICMAN_MATH_COMPILATION)
#error "Only <libpicmanmath/picmanmath.h> can be included directly."
#endif

#ifndef __PICMAN_MD5_H__
#define __PICMAN_MD5_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */

PICMAN_DEPRECATED_FOR(GChecksum)
void picman_md5_get_digest (const gchar *buffer,
                          gint         buffer_size,
                          guchar       digest[16]);

G_END_DECLS

#endif  /* __PICMAN_MD5_H__ */
