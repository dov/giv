/* GRegex -- regular expression API wrapper around PCRE.
 *
 * Copyright (C) 1999, 2000 Scott Wimer
 * Copyright (C) 2004, Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2005 - 2007, Marco Barisione <marco@barisione.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __GIV_REGEX_H__
#define __GIV_REGEX_H__

#include <glib/gerror.h>
#include <glib/gstring.h>

G_BEGIN_DECLS

typedef enum
{
  GIV_REGEX_ERROR_COMPILE,
  GIV_REGEX_ERROR_OPTIMIZE,
  GIV_REGEX_ERROR_REPLACE,
  GIV_REGEX_ERROR_MATCH,
  GIV_REGEX_ERROR_INTERNAL,

  /* These are the error codes from PCRE + 100 */
  GIV_REGEX_ERROR_STRAY_BACKSLASH = 101,
  GIV_REGEX_ERROR_MISSING_CONTROL_CHAR = 102,
  GIV_REGEX_ERROR_UNRECOGNIZED_ESCAPE = 103,
  GIV_REGEX_ERROR_QUANTIFIERS_OUT_OF_ORDER = 104,
  GIV_REGEX_ERROR_QUANTIFIER_TOO_BIG = 105,
  GIV_REGEX_ERROR_UNTERMINATED_CHARACTER_CLASS = 106,
  GIV_REGEX_ERROR_INVALID_ESCAPE_IN_CHARACTER_CLASS = 107,
  GIV_REGEX_ERROR_RANGE_OUT_OF_ORDER = 108,
  GIV_REGEX_ERROR_NOTHING_TO_REPEAT = 109,
  GIV_REGEX_ERROR_UNRECOGNIZED_CHARACTER = 112,
  GIV_REGEX_ERROR_POSIX_NAMED_CLASS_OUTSIDE_CLASS = 113,
  GIV_REGEX_ERROR_UNMATCHED_PARENTHESIS = 114,
  GIV_REGEX_ERROR_INEXISTENT_SUBPATTERN_REFERENCE = 115,
  GIV_REGEX_ERROR_UNTERMINATED_COMMENT = 118,
  GIV_REGEX_ERROR_EXPRESSION_TOO_LARGE = 120,
  GIV_REGEX_ERROR_MEMORY_ERROR = 121,
  GIV_REGEX_ERROR_VARIABLE_LENGTH_LOOKBEHIND = 125,
  GIV_REGEX_ERROR_MALFORMED_CONDITION = 126,
  GIV_REGEX_ERROR_TOO_MANY_CONDITIONAL_BRANCHES = 127,
  GIV_REGEX_ERROR_ASSERTION_EXPECTED = 128,
  GIV_REGEX_ERROR_UNKNOWN_POSIX_CLASS_NAME = 130,
  GIV_REGEX_ERROR_POSIX_COLLATING_ELEMENTS_NOT_SUPPORTED = 131,
  GIV_REGEX_ERROR_HEX_CODE_TOO_LARGE = 134,
  GIV_REGEX_ERROR_INVALID_CONDITION = 135,
  GIV_REGEX_ERROR_SINGLE_BYTE_MATCH_IN_LOOKBEHIND = 136,
  GIV_REGEX_ERROR_INFINITE_LOOP = 140,
  GIV_REGEX_ERROR_MISSING_SUBPATTERN_NAME_TERMINATOR = 142,
  GIV_REGEX_ERROR_DUPLICATE_SUBPATTERN_NAME = 143,
  GIV_REGEX_ERROR_MALFORMED_PROPERTY = 146,
  GIV_REGEX_ERROR_UNKNOWN_PROPERTY = 147,
  GIV_REGEX_ERROR_SUBPATTERN_NAME_TOO_LONG = 148,
  GIV_REGEX_ERROR_TOO_MANY_SUBPATTERNS = 149,
  GIV_REGEX_ERROR_INVALID_OCTAL_VALUE = 151,
  GIV_REGEX_ERROR_TOO_MANY_BRANCHES_IN_DEFINE = 154,
  GIV_REGEX_ERROR_DEFINE_REPETION = 155,
  GIV_REGEX_ERROR_INCONSISTENT_NEWLINE_OPTIONS = 156,
  GIV_REGEX_ERROR_MISSING_BACK_REFERENCE = 157
} GivRegexError;

#define GIV_REGEX_ERROR giv_regex_error_quark ()

GQuark giv_regex_error_quark (void);

/* Remember to update GIV_REGEX_COMPILE_MASK in gregex.c after
 * adding a new flag. */
typedef enum
{
  GIV_REGEX_CASELESS          = 1 << 0,
  GIV_REGEX_MULTILINE         = 1 << 1,
  GIV_REGEX_DOTALL            = 1 << 2,
  GIV_REGEX_EXTENDED          = 1 << 3,
  GIV_REGEX_ANCHORED          = 1 << 4,
  GIV_REGEX_DOLLAR_ENDONLY    = 1 << 5,
  GIV_REGEX_UNGREEDY          = 1 << 9,
  GIV_REGEX_RAW               = 1 << 11,
  GIV_REGEX_NO_AUTO_CAPTURE   = 1 << 12,
  GIV_REGEX_OPTIMIZE          = 1 << 13,
  GIV_REGEX_DUPNAMES          = 1 << 19,
  GIV_REGEX_NEWLINE_CR        = 1 << 20,
  GIV_REGEX_NEWLINE_LF        = 1 << 21,
  GIV_REGEX_NEWLINE_CRLF      = GIV_REGEX_NEWLINE_CR | GIV_REGEX_NEWLINE_LF
} GivRegexCompileFlags;

/* Remember to update GIV_REGEX_MATCH_MASK in gregex.c after
 * adding a new flag. */
typedef enum
{
  GIV_REGEX_MATCH_ANCHORED      = 1 << 4,
  GIV_REGEX_MATCH_NOTBOL        = 1 << 7,
  GIV_REGEX_MATCH_NOTEOL        = 1 << 8,
  GIV_REGEX_MATCH_NOTEMPTY      = 1 << 10,
  GIV_REGEX_MATCH_PARTIAL       = 1 << 15,
  GIV_REGEX_MATCH_NEWLINE_CR    = 1 << 20,
  GIV_REGEX_MATCH_NEWLINE_LF    = 1 << 21,
  GIV_REGEX_MATCH_NEWLINE_CRLF  = GIV_REGEX_MATCH_NEWLINE_CR | GIV_REGEX_MATCH_NEWLINE_LF,
  GIV_REGEX_MATCH_NEWLINE_ANY   = 1 << 22
} GivRegexMatchFlags;

typedef struct _GivRegex		GivRegex;
typedef struct _GivMatchInfo	GivMatchInfo;

typedef gboolean (*GivRegexEvalCallback)		(const GivMatchInfo *match_info,
						 GString          *result,
						 gpointer          user_data);


GivRegex		 *giv_regex_new			(const gchar         *pattern,
						 GivRegexCompileFlags   compile_options,
						 GivRegexMatchFlags     match_options,
						 GError             **error);
GivRegex           *giv_regex_ref			(GivRegex              *regex);
void		  giv_regex_unref			(GivRegex              *regex);
const gchar	 *giv_regex_get_pattern		(const GivRegex        *regex);
gint		  giv_regex_get_max_backref	(const GivRegex        *regex);
gint		  giv_regex_get_capture_count	(const GivRegex        *regex);
gint		  giv_regex_get_string_number	(const GivRegex        *regex, 
						 const gchar         *name);
gchar		 *giv_regex_escape_string		(const gchar         *string,
						 gint                 length);

/* Matching. */
gboolean	  giv_regex_match_simple		(const gchar         *pattern,
						 const gchar         *string,
						 GivRegexCompileFlags   compile_options,
						 GivRegexMatchFlags     match_options);
gboolean	  giv_regex_match			(const GivRegex        *regex,
						 const gchar         *string,
						 GivRegexMatchFlags     match_options,
						 GivMatchInfo         **match_info);
gboolean	  giv_regex_match_full		(const GivRegex        *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 GivRegexMatchFlags     match_options,
						 GivMatchInfo         **match_info,
						 GError             **error);
gboolean	  giv_regex_match_all		(const GivRegex        *regex,
						 const gchar         *string,
						 GivRegexMatchFlags     match_options,
						 GivMatchInfo         **match_info);
gboolean	  giv_regex_match_all_full	(const GivRegex        *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 GivRegexMatchFlags     match_options,
						 GivMatchInfo         **match_info,
						 GError             **error);

/* String splitting. */
gchar		**giv_regex_split_simple		(const gchar         *pattern,
						 const gchar         *string,
						 GivRegexCompileFlags   compile_options,
						 GivRegexMatchFlags     match_options);
gchar		**giv_regex_split			(const GivRegex        *regex,
						 const gchar         *string,
						 GivRegexMatchFlags     match_options);
gchar		**giv_regex_split_full		(const GivRegex        *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 GivRegexMatchFlags     match_options,
						 gint                 max_tokens,
						 GError             **error);

/* String replacement. */
gchar		 *giv_regex_replace		(const GivRegex        *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 const gchar         *replacement,
						 GivRegexMatchFlags     match_options,
						 GError             **error);
gchar		 *giv_regex_replace_literal	(const GivRegex        *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 const gchar         *replacement,
						 GivRegexMatchFlags     match_options,
						 GError             **error);
gchar		 *giv_regex_replace_eval		(const GivRegex        *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 GivRegexMatchFlags     match_options,
						 GivRegexEvalCallback   eval,
						 gpointer             user_data,
						 GError             **error);
gboolean	  giv_regex_check_replacement	(const gchar         *replacement,
						 gboolean            *has_references,
						 GError             **error);

/* Match info */
GivRegex		 *giv_match_info_get_regex	(const GivMatchInfo    *match_info);
const gchar      *giv_match_info_get_string       (const GivMatchInfo    *match_info);

void		  giv_match_info_free		(GivMatchInfo          *match_info);
gboolean	  giv_match_info_next		(GivMatchInfo          *match_info,
						 GError             **error);
gboolean	  giv_match_info_matches		(const GivMatchInfo    *match_info);
gint		  giv_match_info_get_match_count	(const GivMatchInfo    *match_info);
gboolean	  giv_match_info_is_partial_match	(const GivMatchInfo    *match_info);
gchar		 *giv_match_info_expand_references(const GivMatchInfo    *match_info,
						 const gchar         *string_to_expand,
						 GError             **error);
gchar		 *giv_match_info_fetch		(const GivMatchInfo    *match_info,
						 gint                 match_num);
gboolean	  giv_match_info_fetch_pos	(const GivMatchInfo    *match_info,
						 gint                 match_num,
						 gint                *start_pos,
						 gint                *end_pos);
gchar		 *giv_match_info_fetch_named	(const GivMatchInfo    *match_info,
						 const gchar         *name);
gboolean	  giv_match_info_fetch_named_pos	(const GivMatchInfo    *match_info,
						 const gchar         *name,
						 gint                *start_pos,
						 gint                *end_pos);
gchar		**giv_match_info_fetch_all	(const GivMatchInfo    *match_info);

G_END_DECLS

#endif  /*  __GIV_REGEX_H__ */
