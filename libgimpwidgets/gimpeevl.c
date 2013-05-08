/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmaneevl.c
 * Copyright (C) 2008 Fredrik Alstromer <roe@excu.se>
 * Copyright (C) 2008 Martin Nordholts <martinn@svn.gnome.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/* Introducing eevl eva, the evaluator. A straightforward recursive
 * descent parser, no fuss, no new dependencies. The lexer is hand
 * coded, tedious, not extremely fast but works. It evaluates the
 * expression as it goes along, and does not create a parse tree or
 * anything, and will not optimize anything. It uses doubles for
 * precision, with the given use case, that's enough to combat any
 * rounding errors (as opposed to optimizing the evalutation).
 *
 * It relies on external unit resolving through a callback and does
 * elementary dimensionality constraint check (e.g. "2 mm + 3 px * 4
 * in" is an error, as L + L^2 is a missmatch). It uses setjmp/longjmp
 * for try/catch like pattern on error, it uses g_strtod() for numeric
 * conversions and it's non-destructive in terms of the paramters, and
 * it's reentrant.
 *
 * EBNF:
 *
 *   expression    ::= term { ('+' | '-') term }*  |
 *                     <empty string> ;
 *
 *   term          ::= signed factor { ( '*' | '/' ) signed factor }* ;
 *
 *   signed factor ::= ( '+' | '-' )? factor ;
 *
 *   unit factor   ::= factor unit? ;
 *
 *   factor        ::= number | '(' expression ')' ;
 *
 *   number        ::= ? what g_strtod() consumes ? ;
 *
 *   unit          ::= ? what not g_strtod() consumes and not whitespace ? ;
 *
 * The code should match the EBNF rather closely (except for the
 * non-terminal unit factor, which is inlined into factor) for
 * maintainability reasons.
 *
 * It will allow 1++1 and 1+-1 (resulting in 2 and 0, respectively),
 * but I figured one might want that, and I don't think it's going to
 * throw anyone off.
 */

#include "config.h"

#include <setjmp.h>
#include <string.h>

#include <glib-object.h>

#include "picmaneevl.h"
#include "picmanwidgets-error.h"

#include "libpicman/libpicman-intl.h"


typedef enum
{
  PICMAN_EEVL_TOKEN_NUM        = 30000,
  PICMAN_EEVL_TOKEN_IDENTIFIER = 30001,

  PICMAN_EEVL_TOKEN_ANY        = 40000,

  PICMAN_EEVL_TOKEN_END        = 50000
} PicmanEevlTokenType;


typedef struct
{
  PicmanEevlTokenType type;

  union
  {
    gdouble fl;

    struct
    {
      const gchar *c;
      gint         size;
    };

  } value;

} PicmanEevlToken;

typedef struct
{
  const gchar             *string;
  PicmanEevlUnitResolverProc unit_resolver_proc;
  gpointer                 data;

  PicmanEevlToken            current_token;
  const gchar             *start_of_current_token;
  

  jmp_buf                  catcher;
  const gchar             *error_message;

} PicmanEevl;


static void             picman_eevl_init                     (PicmanEevl                 *eva,
                                                            const gchar              *string,
                                                            PicmanEevlUnitResolverProc  unit_resolver_proc,
                                                            gpointer                  data);
static PicmanEevlQuantity picman_eevl_complete                 (PicmanEevl                 *eva);
static PicmanEevlQuantity picman_eevl_expression               (PicmanEevl                 *eva);
static PicmanEevlQuantity picman_eevl_term                     (PicmanEevl                 *eva);
static PicmanEevlQuantity picman_eevl_signed_factor            (PicmanEevl                 *eva);
static PicmanEevlQuantity picman_eevl_factor                   (PicmanEevl                 *eva);
static gboolean         picman_eevl_accept                   (PicmanEevl                 *eva,
                                                            PicmanEevlTokenType         token_type,
                                                            PicmanEevlToken            *consumed_token);
static void             picman_eevl_lex                      (PicmanEevl                 *eva);
static void             picman_eevl_lex_accept_count          (PicmanEevl                 *eva,
                                                            gint                      count,
                                                            PicmanEevlTokenType         token_type);
static void             picman_eevl_lex_accept_to             (PicmanEevl                 *eva,
                                                            gchar                    *to,
                                                            PicmanEevlTokenType         token_type);
static void             picman_eevl_move_past_whitespace     (PicmanEevl                 *eva);
static gboolean         picman_eevl_unit_identifier_start    (gunichar                  c);
static gboolean         picman_eevl_unit_identifier_continue (gunichar                  c);
static gint             picman_eevl_unit_identifier_size     (const gchar              *s,
                                                            gint                      start);
static void             picman_eevl_expect                   (PicmanEevl                 *eva,
                                                            PicmanEevlTokenType         token_type,
                                                            PicmanEevlToken            *value);
static void             picman_eevl_error                    (PicmanEevl                 *eva,
                                                            gchar                    *msg);


/**
 * picman_eevl_evaluate:
 * @string:             The NULL-terminated string to be evaluated.
 * @unit_resolver_proc: Unit resolver callback.
 * @result:             Result of evaluation.
 * @data:               Data passed to unit resolver.
 * @error_pos:          Will point to the poisiton within the string,
 *                      before which the parse / evaluation error
 *                      occurred. Will be set to null of no error occurred.
 * @error_message:      Will point to a static string with a semi-descriptive
 *                      error message if parsing / evaluation failed.
 *
 * Evaluates the given arithmetic expression, along with an optional dimension
 * analysis, and basic unit conversions.
 *
 * All units conversions factors are relative to some implicit
 * base-unit (which in PICMAN is inches). This is also the unit of the
 * returned value.
 *
 * Returns: A #PicmanEevlQuantity with a value given in the base unit along with
 * the order of the dimension (i.e. if the base unit is inches, a dimension
 * order of two menas in^2).
 **/
gboolean
picman_eevl_evaluate (const gchar               *string,
                    PicmanEevlUnitResolverProc   unit_resolver_proc,
                    PicmanEevlQuantity          *result,
                    gpointer                   data,
                    const gchar              **error_pos,
                    GError                   **error)
{
  PicmanEevl eva;

  g_return_val_if_fail (g_utf8_validate (string, -1, NULL), FALSE);
  g_return_val_if_fail (unit_resolver_proc != NULL, FALSE);
  g_return_val_if_fail (result != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  picman_eevl_init (&eva,
                  string,
                  unit_resolver_proc,
                  data);

  if (!setjmp (eva.catcher))  /* try... */
    {
      *result = picman_eevl_complete (&eva);

      return TRUE;
    }
  else   /* catch.. */
    {
      if (error_pos)
        *error_pos = eva.start_of_current_token;

      g_set_error_literal (error,
                           PICMAN_WIDGETS_ERROR,
                           PICMAN_WIDGETS_PARSE_ERROR,
                           eva.error_message);

      return FALSE;
    }
}

static void
picman_eevl_init (PicmanEevl                  *eva,
                const gchar               *string,
                PicmanEevlUnitResolverProc   unit_resolver_proc,
                gpointer                   data)
{
  eva->string              = string;
  eva->unit_resolver_proc  = unit_resolver_proc;
  eva->data                = data;

  eva->current_token.type  = PICMAN_EEVL_TOKEN_END;

  eva->error_message       = NULL;

  /* Preload symbol... */
  picman_eevl_lex (eva);
}

static PicmanEevlQuantity
picman_eevl_complete (PicmanEevl *eva)
{
  PicmanEevlQuantity result = {0, 0};
  PicmanEevlQuantity default_unit_factor;

  /* Empty expression evaluates to 0 */
  if (picman_eevl_accept (eva, PICMAN_EEVL_TOKEN_END, NULL))
    return result;

  result = picman_eevl_expression (eva);

  /* There should be nothing left to parse by now */
  picman_eevl_expect (eva, PICMAN_EEVL_TOKEN_END, 0);

  eva->unit_resolver_proc (NULL,
                           &default_unit_factor,
                           eva->data);

  /* Entire expression is dimensionless, apply default unit if
   * applicable
   */
  if (result.dimension == 0 && default_unit_factor.dimension != 0)
    {
      result.value     /= default_unit_factor.value;
      result.dimension  = default_unit_factor.dimension;
    }
  return result;
}

static PicmanEevlQuantity
picman_eevl_expression (PicmanEevl *eva)
{
  gboolean         subtract;
  PicmanEevlQuantity evaluated_terms;

  evaluated_terms = picman_eevl_term (eva);

  /* continue evaluating terms, chained with + or -. */
  for (subtract = FALSE;
       picman_eevl_accept (eva, '+', NULL) ||
       (subtract = picman_eevl_accept (eva, '-', NULL));
       subtract = FALSE)
    {
      PicmanEevlQuantity new_term = picman_eevl_term (eva);

      /* If dimensions missmatch, attempt default unit assignent */
      if (new_term.dimension != evaluated_terms.dimension)
        {
          PicmanEevlQuantity default_unit_factor;

          eva->unit_resolver_proc (NULL,
                                   &default_unit_factor,
                                   eva->data);

          if (new_term.dimension == 0 &&
              evaluated_terms.dimension == default_unit_factor.dimension)
            {
              new_term.value     /= default_unit_factor.value;
              new_term.dimension  = default_unit_factor.dimension;
            }
          else if (evaluated_terms.dimension == 0 &&
                   new_term.dimension == default_unit_factor.dimension)
            {
              evaluated_terms.value     /= default_unit_factor.value;
              evaluated_terms.dimension  = default_unit_factor.dimension;
            }
          else
            {
              picman_eevl_error (eva, "Dimension missmatch during addition");
            }
        }

      evaluated_terms.value += (subtract ? -new_term.value : new_term.value);
    }

  return evaluated_terms;
}

static PicmanEevlQuantity
picman_eevl_term (PicmanEevl *eva)
{
  gboolean         division;
  PicmanEevlQuantity evaluated_signed_factors;

  evaluated_signed_factors = picman_eevl_signed_factor (eva);

  for (division = FALSE;
       picman_eevl_accept (eva, '*', NULL) ||
       (division = picman_eevl_accept (eva, '/', NULL));
       division = FALSE)
    {
      PicmanEevlQuantity new_signed_factor = picman_eevl_signed_factor (eva);

      if (division)
        {
          evaluated_signed_factors.value     /= new_signed_factor.value;
          evaluated_signed_factors.dimension -= new_signed_factor.dimension;

        }
      else
        {
          evaluated_signed_factors.value     *= new_signed_factor.value;
          evaluated_signed_factors.dimension += new_signed_factor.dimension;
        }
    }

  return evaluated_signed_factors;
}

static PicmanEevlQuantity
picman_eevl_signed_factor (PicmanEevl *eva)
{
  PicmanEevlQuantity result;
  gboolean         negate = FALSE;

  if (! picman_eevl_accept (eva, '+', NULL))
    negate = picman_eevl_accept (eva, '-', NULL);

  result = picman_eevl_factor (eva);

  if (negate) result.value = -result.value;

  return result;
}

static PicmanEevlQuantity
picman_eevl_factor (PicmanEevl *eva)
{
  PicmanEevlQuantity evaluated_factor = { 0, 0 };
  PicmanEevlToken    consumed_token;

  if (picman_eevl_accept (eva,
                        PICMAN_EEVL_TOKEN_NUM,
                        &consumed_token))
    {
      evaluated_factor.value = consumed_token.value.fl;
    }
  else if (picman_eevl_accept (eva, '(', NULL))
    {
      evaluated_factor = picman_eevl_expression (eva);
      picman_eevl_expect (eva, ')', 0);
    }
  else
    {
      picman_eevl_error (eva, "Expected number or '('");
    }

  if (eva->current_token.type == PICMAN_EEVL_TOKEN_IDENTIFIER)
    {
      gchar            *identifier;
      PicmanEevlQuantity  result;

      picman_eevl_accept (eva,
                        PICMAN_EEVL_TOKEN_ANY,
                        &consumed_token);

      identifier = g_newa (gchar, consumed_token.value.size + 1);

      strncpy (identifier, consumed_token.value.c, consumed_token.value.size);
      identifier[consumed_token.value.size] = '\0';

      if (eva->unit_resolver_proc (identifier,
                                   &result,
                                   eva->data))
        {
          evaluated_factor.value      /= result.value;
          evaluated_factor.dimension  += result.dimension;
        }
      else
        {
          picman_eevl_error (eva, "Unit was not resolved");
        }
    }

  return evaluated_factor;
}

static gboolean
picman_eevl_accept (PicmanEevl          *eva,
                  PicmanEevlTokenType  token_type,
                  PicmanEevlToken     *consumed_token)
{
  gboolean existed = FALSE;

  if (token_type == eva->current_token.type ||
      token_type == PICMAN_EEVL_TOKEN_ANY)
    {
      existed = TRUE;

      if (consumed_token)
        *consumed_token = eva->current_token;

      /* Parse next token */
      picman_eevl_lex (eva);
    }

  return existed;
}

static void
picman_eevl_lex (PicmanEevl *eva)
{
  const gchar *s;

  picman_eevl_move_past_whitespace (eva);
  s = eva->string;
  eva->start_of_current_token = s;

  if (! s || s[0] == '\0')
    {
      /* We're all done */
      eva->current_token.type = PICMAN_EEVL_TOKEN_END;
    }
  else if (s[0] == '+' || s[0] == '-')
    {
      /* Snatch these before the g_strtod() does, othewise they might
       * be used in a numeric conversion.
       */
      picman_eevl_lex_accept_count (eva, 1, s[0]);
    }
  else
    
    {
      /* Attempt to parse a numeric value */
      gchar  *endptr = NULL;
      gdouble value      = g_strtod (s, &endptr);

      if (endptr && endptr != s)
        {
          /* A numeric could be parsed, use it */
          eva->current_token.value.fl = value;

          picman_eevl_lex_accept_to (eva, endptr, PICMAN_EEVL_TOKEN_NUM);
        }
      else if (picman_eevl_unit_identifier_start (s[0]))
        {
          /* Unit identifier */
          eva->current_token.value.c    = s;
          eva->current_token.value.size = picman_eevl_unit_identifier_size (s, 0);

          picman_eevl_lex_accept_count (eva,
                                      eva->current_token.value.size,
                                      PICMAN_EEVL_TOKEN_IDENTIFIER);
        }
      else
        {
          /* Everything else is a single character token */
          picman_eevl_lex_accept_count (eva, 1, s[0]);
        }
    }
}

static void
picman_eevl_lex_accept_count (PicmanEevl          *eva,
                            gint               count,
                            PicmanEevlTokenType  token_type)
{
  eva->current_token.type  = token_type;
  eva->string             += count;
}

static void
picman_eevl_lex_accept_to (PicmanEevl          *eva,
                         gchar             *to,
                         PicmanEevlTokenType  token_type)
{
  eva->current_token.type = token_type;
  eva->string             = to;
}

static void
picman_eevl_move_past_whitespace (PicmanEevl *eva)
{
  if (! eva->string)
    return;

  while (g_ascii_isspace (*eva->string))
    eva->string++;
}

static gboolean
picman_eevl_unit_identifier_start (gunichar c)
{
  return (g_unichar_isalpha (c) ||
          c == (gunichar) '%'   ||
          c == (gunichar) '\'');
}

static gboolean
picman_eevl_unit_identifier_continue (gunichar c)
{
  return (picman_eevl_unit_identifier_start (c) ||
          g_unichar_isdigit (c));
}

/**
 * picman_eevl_unit_identifier_size:
 * @s:
 * @start:
 *
 * Returns: Size of identifier in bytes (not including NULL
 * terminator).
 **/
static gint
picman_eevl_unit_identifier_size (const gchar *string,
                                gint         start_offset)
{
  const gchar *start  = g_utf8_offset_to_pointer (string, start_offset);
  const gchar *s      = start;
  gunichar     c      = g_utf8_get_char (s);
  gint         length = 0;

  if (picman_eevl_unit_identifier_start (c))
    {
      s = g_utf8_next_char (s);
      c = g_utf8_get_char (s);
      length++;

      while (picman_eevl_unit_identifier_continue (c))
        {
          s = g_utf8_next_char (s);
          c = g_utf8_get_char (s);
          length++;
        }
    }
  
  return g_utf8_offset_to_pointer (start, length) - start;
}

static void
picman_eevl_expect (PicmanEevl          *eva,
                  PicmanEevlTokenType  token_type,
                  PicmanEevlToken     *value)
{
  if (! picman_eevl_accept (eva, token_type, value))
    picman_eevl_error (eva, "Unexpected token");
}

static void
picman_eevl_error (PicmanEevl *eva,
                 gchar    *msg)
{
  eva->error_message = msg;
  longjmp (eva->catcher, 1);
}
