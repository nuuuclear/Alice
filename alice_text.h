/* alice_text.h - a bitmap text renderer for Alice

This has ASCII, UTF-8 and Shift-JIS support, 
And contains a built-in font (Misaki Gothic).

In exactly ONE source file:
    #define ALICE_IMPLEMENTATION
    #include "alice.h"

    #define ALICE_TEXT_IMPLEMENTATION
    #include "alice_text.h"

Optional macros:
    ALICE_TEXT_STATIC
        make all functions static
    ALICE_TEXT_NO_BUILTIN_FONT
        leave out the built-in glyph data
    ALICE_TEXT_SJIS_BASIC
        only JIS rows 1-8 and NEC row 13 (kana, symbols, Latin, Greek,
        Cyrillic, and box drawing)

The generated data (Shift-JIS tables, glyph bitmaps and range tables) is
all at the very end of this file.
*/
#ifndef ALICE_TEXT_H
#define ALICE_TEXT_H

#ifdef ALICE_HEADER
    #include ALICE_HEADER
#else
    #include "alice.h"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef ALICE_TEXT_API
    #ifdef ALICE_TEXT_STATIC
        #define ALICE_TEXT_API static
    #else
        #define ALICE_TEXT_API extern
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ALICE_REPLACEMENT_CHAR      0xFFFDu
#define ALICE_TEXT_NULL_TERMINATED   ((size_t)-1)

typedef enum alice_encoding {
    ALICE_ENCODING_UTF8 = 0,
    ALICE_ENCODING_ASCII,
    ALICE_ENCODING_SJIS
} alice_encoding;


//// encodings and converters

ALICE_TEXT_API bool alice_is_ascii(uint32_t code_point);
ALICE_TEXT_API bool alice_ascii_is_valid(const char *text, size_t length);
ALICE_TEXT_API int alice_ascii_upper(int c);
ALICE_TEXT_API int alice_ascii_lower(int c);

ALICE_TEXT_API size_t alice_utf8_decode(const char *s, const char *end, uint32_t *code_point);
ALICE_TEXT_API int alice_utf8_encode(uint32_t code_point, char *out);
ALICE_TEXT_API bool alice_utf8_is_valid(const char *text, size_t length);
ALICE_TEXT_API size_t alice_utf8_length(const char *text, size_t length);
ALICE_TEXT_API const char *alice_utf8_next(const char *s, const char *end);
ALICE_TEXT_API const char *alice_utf8_prev(const char *begin, const char *s);
ALICE_TEXT_API size_t alice_utf8_offset(const char *text, size_t length, size_t char_index);

ALICE_TEXT_API bool alice_sjis_is_lead(unsigned byte);
ALICE_TEXT_API bool alice_sjis_is_trail(unsigned byte);
ALICE_TEXT_API size_t alice_sjis_decode(const char *s, const char *end, uint32_t *code_point);
ALICE_TEXT_API int alice_sjis_encode(uint32_t code_point, char *out);
ALICE_TEXT_API int32_t alice_sjis_to_unicode(uint32_t sjis_code);
ALICE_TEXT_API int32_t alice_unicode_to_sjis(uint32_t code_point);
ALICE_TEXT_API bool alice_sjis_is_valid(const char *text, size_t length);
ALICE_TEXT_API size_t alice_sjis_length(const char *text, size_t length);
ALICE_TEXT_API uint32_t alice_sjis_to_jis(uint32_t sjis_code);
ALICE_TEXT_API uint32_t alice_jis_to_sjis(uint32_t jis_code);

ALICE_TEXT_API size_t alice_convert(
    alice_encoding from, const char *src, size_t src_length,
    alice_encoding to, char *dst, size_t dst_capacity
);
ALICE_TEXT_API size_t alice_utf8_to_sjis(const char *src, size_t src_length, char *dst, size_t dst_capacity);
ALICE_TEXT_API size_t alice_sjis_to_utf8(const char *src, size_t src_length, char *dst, size_t dst_capacity);
ALICE_TEXT_API size_t alice_utf8_to_ascii(const char *src, size_t src_length, char *dst, size_t dst_capacity);
ALICE_TEXT_API size_t alice_ascii_to_utf8(const char *src, size_t src_length, char *dst, size_t dst_capacity);
ALICE_TEXT_API size_t alice_sjis_to_ascii(const char *src, size_t src_length, char *dst, size_t dst_capacity);
ALICE_TEXT_API size_t alice_ascii_to_sjis(const char *src, size_t src_length, char *dst, size_t dst_capacity);
ALICE_TEXT_API size_t alice_utf8_to_utf32(const char *src, size_t src_length, uint32_t *dst, size_t dst_capacity);
ALICE_TEXT_API size_t alice_utf32_to_utf8(const uint32_t *src, size_t src_count, char *dst, size_t dst_capacity);

//// Fonts

typedef struct alice_font_range {
    uint32_t        first;
    uint32_t        count;
    int             width;
    const uint8_t  *bitmaps;
    const uint16_t *map;
    const uint8_t  *advances;
} alice_font_range;

typedef struct alice_font alice_font;
struct alice_font {
    int height;
    const alice_font_range *ranges;
    size_t range_count;
    uint32_t fallback;
    const alice_font *next;
};

typedef struct alice_glyph {
    const uint8_t *bits;
    int width;
    int height;
    int stride;
    int advance;
} alice_glyph;

ALICE_TEXT_API bool alice_font_glyph(const alice_font *font, uint32_t code_point, alice_glyph *glyph);

#ifndef ALICE_TEXT_NO_BUILTIN_FONT
ALICE_TEXT_API const alice_font *alice_font_builtin(void);          // everything below
ALICE_TEXT_API const alice_font *alice_font_builtin_latin(void);    // ASCII and the Latin-1 symbols
ALICE_TEXT_API const alice_font *alice_font_builtin_hiragana(void); // U+3041-309F
ALICE_TEXT_API const alice_font *alice_font_builtin_katakana(void); // U+30A0-30FF and U+FF61-FF9F
#endif

//// Drawing

typedef struct alice_text_style {
    const alice_font *font;
    alice_encoding encoding;
    uint32_t color;
    uint32_t background;
    bool draw_background;
    int scale;
    int spacing;
    int line_spacing;
    int tab_width;
} alice_text_style;

ALICE_TEXT_API void alice_text_style_init(alice_text_style *style);

ALICE_TEXT_API int alice_draw_text(
    alice_surface *surface, int x, int y, const char *text,
    const alice_text_style *style
);

ALICE_TEXT_API int alice_draw_text_n(
    alice_surface *surface, int x, int y, const char *text,
    size_t length, const alice_text_style *style
);

ALICE_TEXT_API int alice_draw_text_utf32(
    alice_surface *surface, int x, int y,
    const uint32_t *code_points, size_t count,
    const alice_text_style *style
);

ALICE_TEXT_API void alice_measure_text(
    const char *text, const alice_text_style *style,
    int *width, int *height
);

ALICE_TEXT_API void alice_measure_text_n(
    const char *text, size_t length,
    const alice_text_style *style, int *width, int *height
);

#ifdef __cplusplus
}
#endif

#endif /* ALICE_TEXT_H */

////////////////////////////////////////////////////////////
//   Implementation   //////////////////////////////
////////////////////////////////////////////

#if defined(ALICE_TEXT_IMPLEMENTATION) && !defined(ALICE_TEXT_IMPLEMENTATION_ONCE)
#define ALICE_TEXT_IMPLEMENTATION_ONCE

#include <string.h>

#if defined(__GNUC__)
    #define ALICE_TEXT_UNUSED __attribute__((unused))
#else
    #define ALICE_TEXT_UNUSED
#endif

#define ALICE_SJIS_USER_FIRST 0xE000u
#define ALICE_SJIS_USER_COUNT (10u * 188u)

#ifdef ALICE_TEXT_SJIS_BASIC
    #define ALICE_SJIS_ROWS     7
    #define ALICE_SJIS_REVERSE  598
#else
    #define ALICE_SJIS_ROWS     50
    #define ALICE_SJIS_REVERSE  7326
#endif
static const uint16_t alice_sjis_table[ALICE_SJIS_ROWS * 188];
static const uint16_t alice_sjis_by_unicode[ALICE_SJIS_REVERSE];
#ifndef ALICE_TEXT_NO_BUILTIN_FONT
static const alice_font alice_font_defs[4];
#endif


////////////////////////////////////////////////////////////
//   ASCII   ///////////////////////////////////////
////////////////////////////////////////////

bool alice_is_ascii(uint32_t cp) { return cp < 0x80u; }

bool alice_ascii_is_valid(const char *text, size_t length) {
    size_t i;
    if (!text)
        return true;
    if (length == ALICE_TEXT_NULL_TERMINATED)
        length = strlen(text);
    for (i = 0; i < length; ++i) {
        if ((unsigned char)text[i] >= 0x80u)
            return false;
    }
    return true;
}

int alice_ascii_upper(int c) {
    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

int alice_ascii_lower(int c) {
    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

////////////////////////////////////////////////////////////
//   UTF-8   ///////////////////////////////////////
////////////////////////////////////////////

size_t alice_utf8_decode(const char *s, const char *end, uint32_t *cp) {
    unsigned b0, lo = 0x80, hi = 0xBF, need, i;
    uint32_t c;

    if (s >= end) {
        *cp = ALICE_REPLACEMENT_CHAR;
        return 0;
    }
    b0 = (unsigned char)s[0];
    if (b0 < 0x80) {
        *cp = b0;
        return 1;
    }

    if (b0 >= 0xC2 && b0 <= 0xDF) {
        need = 1;
        c = b0 & 0x1Fu;
    } else if (b0 >= 0xE0 && b0 <= 0xEF) {
        need = 2;
        c = b0 & 0x0Fu;
        if (b0 == 0xE0) lo = 0xA0;
        if (b0 == 0xED) hi = 0x9F;
    } else if (b0 >= 0xF0 && b0 <= 0xF4) {
        need = 3;
        c = b0 & 0x07u;
        if (b0 == 0xF0) lo = 0x90;
        if (b0 == 0xF4) hi = 0x8F;
    } else {
        *cp = ALICE_REPLACEMENT_CHAR;
        return 1;
    }

    for (i = 1; i <= need; ++i) {
        unsigned b;
        if (s + i >= end) {
            *cp = ALICE_REPLACEMENT_CHAR;
            return i;
        }
        b = (unsigned char)s[i];
        if (b < lo || b > hi) {
            *cp = ALICE_REPLACEMENT_CHAR;
            return i;
        }
        lo = 0x80;
        hi = 0xBF;
        c = (c << 6) | (b & 0x3Fu);
    }
    *cp = c;
    return need + 1;
}

int alice_utf8_encode(uint32_t cp, char *out) {
    if (cp < 0x80u) {
        out[0] = (char)cp;
        return 1;
    }
    if (cp < 0x800u) {
        out[0] = (char)(0xC0u | (cp >> 6));
        out[1] = (char)(0x80u | (cp & 0x3Fu));
        return 2;
    }
    if (cp < 0x10000u) {
        if (cp >= 0xD800u && cp <= 0xDFFFu)
            return 0;
        out[0] = (char)(0xE0u | (cp >> 12));
        out[1] = (char)(0x80u | ((cp >> 6) & 0x3Fu));
        out[2] = (char)(0x80u | (cp & 0x3Fu));
        return 3;
    }
    if (cp <= 0x10FFFFu) {
        out[0] = (char)(0xF0u | (cp >> 18));
        out[1] = (char)(0x80u | ((cp >> 12) & 0x3Fu));
        out[2] = (char)(0x80u | ((cp >> 6) & 0x3Fu));
        out[3] = (char)(0x80u | (cp & 0x3Fu));
        return 4;
    }
    return 0;
}

bool alice_utf8_is_valid(const char *text, size_t length) {
    const char *p, *end;
    if (!text)
        return true;
    if (length == ALICE_TEXT_NULL_TERMINATED)
        length = strlen(text);
    p = text;
    end = text + length;
    while (p < end) {
        uint32_t cp;
        size_t n = alice_utf8_decode(p, end, &cp);
        if (cp == ALICE_REPLACEMENT_CHAR 
        && !(  
            n == 3
            && (unsigned char)p[0] == 0xEF
            && (unsigned char)p[1] == 0xBF
            && (unsigned char)p[2] == 0xBD
            )
        ) return false;
        
        p += n;
    }

    return true;
}

size_t alice_utf8_length(const char *text, size_t length) {
    const char *p, *end;
    size_t count = 0;
    if (!text) return 0;
    if (length == ALICE_TEXT_NULL_TERMINATED)
        length = strlen(text);
    p = text;
    end = text + length;
    while (p < end) {
        uint32_t cp;
        p += alice_utf8_decode(p, end, &cp);
        ++count;
    }

    return count;
}

const char *alice_utf8_next(const char *s, const char *end) {
    uint32_t cp;
    if (s >= end) return end;
    return s + alice_utf8_decode(s, end, &cp);
}

const char *alice_utf8_prev(const char *begin, const char *s) {
    int back = 0;
    if (s <= begin)
        return begin;
    --s;

    while (s > begin && ((unsigned char)*s & 0xC0u) == 0x80u && back < 3) {
        --s;
        ++back;
    }
    
    return s;
}

size_t alice_utf8_offset(const char *text, size_t length, size_t index) {
    const char *p, *end;
    if (!text)
        return 0;
    if (length == ALICE_TEXT_NULL_TERMINATED)
        length = strlen(text);
    p = text;
    end = text + length;
    while (p < end && index > 0) {
        uint32_t cp;
        p += alice_utf8_decode(p, end, &cp);
        --index;
    }
    return (size_t)(p - text);
}

////////////////////////////////////////////////////////////
//   Shift-JIS   ///////////////////////////////////
////////////////////////////////////////////

bool alice_sjis_is_lead(unsigned b) {
    return (b >= 0x81 && b <= 0x9F) || (b >= 0xE0 && b <= 0xFC);
}

bool alice_sjis_is_trail(unsigned b) {
    return (b >= 0x40 && b <= 0x7E) || (b >= 0x80 && b <= 0xFC);
}

static int alice_sjis_lead_index(unsigned lead) {
    if (lead >= 0x81 && lead <= 0x9F) return (int)lead - 0x81;
    if (lead >= 0xE0 && lead <= 0xEF) return 31 + (int)lead - 0xE0;
    if (lead >= 0xFA && lead <= 0xFC) return 47 + (int)lead - 0xFA;
    return -1;
}

static int alice_sjis_trail_index(unsigned t) {
    if (t >= 0x40 && t <= 0x7E) return (int)t - 0x40;
    if (t >= 0x80 && t <= 0xFC) return (int)t - 0x41;
    return -1;
}

static unsigned alice_sjis_trail_from_index(unsigned i) {
    return i <= 62 ? 0x40 + i : 0x41 + i;
}

static uint32_t alice_sjis_table_lookup(int lead_index, int trail_index) {
    return lead_index < ALICE_SJIS_ROWS ? alice_sjis_table[lead_index * 188 + trail_index] : 0;
}

int32_t alice_sjis_to_unicode(uint32_t code) {
    unsigned lead, trail;
    int li, ti;
    uint32_t u;

    if (code < 0x80u)
        return (int32_t)code;
    if (code >= 0xA1u && code <= 0xDFu)
        return (int32_t)(0xFF61u + (code - 0xA1u));
    if (code < 0x100u || code > 0xFFFFu)
        return -1;

    lead = (unsigned)(code >> 8);
    trail = (unsigned)(code & 0xFFu);
    ti = alice_sjis_trail_index(trail);
    if (ti < 0)
        return -1;
    if (lead >= 0xF0 && lead <= 0xF9)
        return (int32_t)(ALICE_SJIS_USER_FIRST + (lead - 0xF0) * 188u + (unsigned)ti);
    li = alice_sjis_lead_index(lead);
    if (li < 0)
        return -1;
    u = alice_sjis_table_lookup(li, ti);
    return u ? (int32_t)u : -1;
}

static const uint16_t alice_sjis_alias[][2] = {
    { 0x00A2, 0xFFE0 }, { 0x00A3, 0xFFE1 }, { 0x00AC, 0xFFE2 },
    { 0x2014, 0x2015 }, { 0x2016, 0x2225 }, { 0x2212, 0xFF0D },
    { 0x301C, 0xFF5E }
};

int32_t alice_unicode_to_sjis(uint32_t cp) {
    size_t lo, hi, i;

    if (cp < 0x80u)
        return (int32_t)cp;
    if (cp >= 0xFF61u && cp <= 0xFF9Fu)
        return (int32_t)(0xA1u + (cp - 0xFF61u));
    if (cp >= ALICE_SJIS_USER_FIRST && cp < ALICE_SJIS_USER_FIRST + ALICE_SJIS_USER_COUNT) {
        uint32_t off = cp - ALICE_SJIS_USER_FIRST;
        return (int32_t)(((0xF0u + off / 188u) << 8) | alice_sjis_trail_from_index((unsigned)(off % 188u)));
    }
    if (cp > 0xFFFFu) {
        return -1;
    }

    for (i = 0; i < sizeof(alice_sjis_alias) / sizeof(alice_sjis_alias[0]); ++i) {
        if (alice_sjis_alias[i][0] == cp) {
            cp = alice_sjis_alias[i][1];
            break;
        }
    }

    lo = 0;
    hi = sizeof(alice_sjis_by_unicode) / sizeof(alice_sjis_by_unicode[0]);
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        int32_t u = alice_sjis_to_unicode(alice_sjis_by_unicode[mid]);
        if ((uint32_t)u == cp)
            return (int32_t)alice_sjis_by_unicode[mid];
        if ((uint32_t)u < cp)
            lo = mid + 1;
        else
            hi = mid;
    }
    return -1;
}

size_t alice_sjis_decode(const char *s, const char *end, uint32_t *cp) {
    unsigned b;

    if (s >= end) {
        *cp = ALICE_REPLACEMENT_CHAR;
        return 0;
    }

    b = (unsigned char)s[0];
    if (b < 0x80 || (b >= 0xA1 && b <= 0xDF)) {
        *cp = (uint32_t)alice_sjis_to_unicode(b);
        return 1;
    }

    if (alice_sjis_is_lead(b) && s + 1 < end && alice_sjis_is_trail((unsigned char)s[1])) {
        int32_t u = alice_sjis_to_unicode(((uint32_t)b << 8) | (unsigned char)s[1]);
        *cp = u >= 0 ? (uint32_t)u : ALICE_REPLACEMENT_CHAR;
        return 2;
    }
    *cp = ALICE_REPLACEMENT_CHAR;
    return 1;
}

int alice_sjis_encode(uint32_t cp, char *out) {
    int32_t code = alice_unicode_to_sjis(cp);
    if (code < 0)
        return 0;
    if (code <= 0xFF) {
        out[0] = (char)code;
        return 1;
    }

    out[0] = (char)(code >> 8);
    out[1] = (char)(code & 0xFF);
    return 2;
}

static bool alice_sjis_char_ok(const char *p, const char *end, size_t *n) {
    unsigned b = (unsigned char)p[0];
    if (b < 0x80 || (b >= 0xA1 && b <= 0xDF)) {
        *n = 1;
        return true;
    }

    if (alice_sjis_is_lead(b) && p + 1 < end && alice_sjis_is_trail((unsigned char)p[1])) {
        *n = 2;
        return alice_sjis_to_unicode(((uint32_t)b << 8) | (unsigned char)p[1]) >= 0;
    }

    *n = 1;
    return false;
}

bool alice_sjis_is_valid(const char *text, size_t length) {
    const char *p, *end;

    if (!text) return true;
    
    if (length == ALICE_TEXT_NULL_TERMINATED)
        length = strlen(text);
    p = text;
    end = text + length;
    
    while (p < end) {
        size_t n;
        if (!alice_sjis_char_ok(p, end, &n))
            return false;
        p += n;
    }

    return true;
}

size_t alice_sjis_length(const char *text, size_t length) {
    const char *p, *end;
    size_t count = 0;

    if (!text) return 0;

    if (length == ALICE_TEXT_NULL_TERMINATED)
        length = strlen(text);
    p = text;
    end = text + length;

    while (p < end) {
        uint32_t cp;
        p += alice_sjis_decode(p, end, &cp);
        ++count;
    }

    return count;
}

uint32_t alice_sjis_to_jis(uint32_t sjis) {
    unsigned s1 = (unsigned)(sjis >> 8), s2 = (unsigned)(sjis & 0xFF);

    if (sjis > 0xFFFFu
    ||  !((s1 >= 0x81 && s1 <= 0x9F)
    ||  (s1 >= 0xE0 && s1 <= 0xEF))
    ||  !alice_sjis_is_trail(s2)
    ||  s2 == 0x7F
    ) return 0;
    
    s1 -= s1 <= 0x9F ? 0x71 : 0xB1;
    s1 = s1 * 2 + 1;
    
    if (s2 > 0x7F)
        s2 -= 1;
    if (s2 >= 0x9E) {
        s2 -= 0x7D;
        s1 += 1;
    } else {
        s2 -= 0x1F;
    }

    return ((uint32_t)s1 << 8) | s2;
}

uint32_t alice_jis_to_sjis(uint32_t jis) {
    unsigned j1 = (unsigned)(jis >> 8), j2 = (unsigned)(jis & 0xFF), s1, s2;

    if (jis > 0xFFFFu || j1 < 0x21 || j1 > 0x7E || j2 < 0x21 || j2 > 0x7E)
        return 0;
    if (j1 & 1) {
        s2 = j2 + 0x1F;
        if (s2 >= 0x7F) {
            s2 += 1;
        }
    } else {
        s2 = j2 + 0x7E;
    }

    s1 = ((j1 + 1) >> 1) + (j1 <= 0x5E ? 0x70 : 0xB0);
    return ((uint32_t)s1 << 8) | s2;
}

// text sourcing

typedef struct alice_iter {
    const char     *p, *end;
    alice_encoding  encoding;
    const uint32_t *u32;
    size_t          u32_left;
    bool            from_u32;
} alice_iter;

static void alice_iter_bytes(alice_iter *it, alice_encoding enc, const char *text, size_t length) {
    memset(it, 0, sizeof(*it));
    it->encoding = enc;
    if (!text) {
        length = 0;
    } else if (length == ALICE_TEXT_NULL_TERMINATED) {
        length = strlen(text);
    }
    
    it->p = text;
    it->end = text ? text + length : text;
}

static void alice_iter_u32(alice_iter *it, const uint32_t *cps, size_t count) {
    memset(it, 0, sizeof(*it));
    it->from_u32 = true;
    it->u32 = cps;
    it->u32_left = cps ? count : 0;
}

static bool alice_iter_next(alice_iter *it, uint32_t *cp) {
    if (it->from_u32) {
        if (it->u32_left == 0) {
            return false;
        }
        *cp = *it->u32++;
        --it->u32_left;
        return true;
    }

    if (it->p >= it->end)
        return false;
    
    switch (it->encoding) {
    case ALICE_ENCODING_UTF8:
        it->p += alice_utf8_decode(it->p, it->end, cp);
        break;

    case ALICE_ENCODING_SJIS:
        it->p += alice_sjis_decode(it->p, it->end, cp);
        break;

    default: {
        unsigned b = (unsigned char)*it->p++;
        *cp = b < 0x80 ? b : ALICE_REPLACEMENT_CHAR;

        break;
        }

    }
    return true;
}

typedef struct alice_out {
    char   *dst;
    size_t  capacity, needed, written;
    bool    full;
} alice_out;

static void alice_out_init(alice_out *o, char *dst, size_t capacity) {
    o->dst = dst;
    o->capacity = capacity;
    o->needed = 0;
    o->written = 0;
    o->full = false;
}

static void alice_out_put(alice_out *o, const char *bytes, size_t n) {
    o->needed += n;
    if (o->full || !o->dst || o->capacity == 0)
        return;
    if (o->written + n > o->capacity - 1) {
        o->full = true;
        return;
    }
    memcpy(o->dst + o->written, bytes, n);
    o->written += n;
}

static size_t alice_out_finish(alice_out *o) {
    if (o->dst && o->capacity > 0)
        o->dst[o->written] = '\0';
    return o->needed;
}

////////////////////////////////////////////////////////////
//   Converters   //////////////////////////////////
////////////////////////////////////////////

size_t alice_convert(
    alice_encoding from, const char *src, size_t src_length,
    alice_encoding to, char *dst, size_t dst_capacity
) {
    alice_iter it;
    alice_out out;
    uint32_t cp;

    alice_iter_bytes(&it, from, src, src_length);
    alice_out_init(&out, dst, dst_capacity);

    while (alice_iter_next(&it, &cp)) {
        char buf[4];
        int n;

        switch (to) {
        case ALICE_ENCODING_UTF8:
            n = alice_utf8_encode(cp, buf);
            if (n == 0)
                n = alice_utf8_encode(ALICE_REPLACEMENT_CHAR, buf);
            break;
        case ALICE_ENCODING_SJIS:
            n = alice_sjis_encode(cp, buf);
            break;
        default:
            n = cp < 0x80u ? (buf[0] = (char)cp, 1) : 0;
            break;
        }
        if (n == 0) {
            buf[0] = '?';
            n = 1;
        }

        alice_out_put(&out, buf, (size_t)n);
    }
    
    return alice_out_finish(&out);
}

size_t alice_utf8_to_sjis(const char *s, size_t n, char *d, size_t c) {
    return alice_convert(ALICE_ENCODING_UTF8, s, n, ALICE_ENCODING_SJIS, d, c);
}

size_t alice_sjis_to_utf8(const char *s, size_t n, char *d, size_t c) {
    return alice_convert(ALICE_ENCODING_SJIS, s, n, ALICE_ENCODING_UTF8, d, c);
}

size_t alice_utf8_to_ascii(const char *s, size_t n, char *d, size_t c) {
    return alice_convert(ALICE_ENCODING_UTF8, s, n, ALICE_ENCODING_ASCII, d, c);
}

size_t alice_ascii_to_utf8(const char *s, size_t n, char *d, size_t c) {
    return alice_convert(ALICE_ENCODING_ASCII, s, n, ALICE_ENCODING_UTF8, d, c);
}

size_t alice_sjis_to_ascii(const char *s, size_t n, char *d, size_t c) {
    return alice_convert(ALICE_ENCODING_SJIS, s, n, ALICE_ENCODING_ASCII, d, c);
}

size_t alice_ascii_to_sjis(const char *s, size_t n, char *d, size_t c) {
    return alice_convert(ALICE_ENCODING_ASCII, s, n, ALICE_ENCODING_SJIS, d, c);
}

size_t alice_utf8_to_utf32(const char *src, size_t src_length, uint32_t *dst, size_t capacity) {
    alice_iter it;
    uint32_t cp;
    size_t count = 0;

    alice_iter_bytes(&it, ALICE_ENCODING_UTF8, src, src_length);
    while (alice_iter_next(&it, &cp)) {
        if (dst && capacity > 0 && count < capacity - 1)
            dst[count] = cp;
        
        ++count;
    }
    
    if (dst && capacity > 0) {
        dst[count < capacity - 1 ? count : capacity - 1] = 0;
    }
    return count;
}

size_t alice_utf32_to_utf8(const uint32_t *src, size_t count, char *dst, size_t capacity) {
    alice_iter it;
    alice_out out;
    uint32_t cp;

    if (src && count == ALICE_TEXT_NULL_TERMINATED) {
        for (count = 0; src[count]; ++count) ;
    }

    alice_iter_u32(&it, src, count);
    alice_out_init(&out, dst, capacity);
    while (alice_iter_next(&it, &cp)) {
        char buf[4];
        int n = alice_utf8_encode(cp, buf);
        if (n == 0)
            n = alice_utf8_encode(ALICE_REPLACEMENT_CHAR, buf);
        alice_out_put(&out, buf, (size_t)n);
    }

    return alice_out_finish(&out);
}

////////////////////////////////////////////////////////////
//   Fonts   ///////////////////////////////////////
////////////////////////////////////////////

bool alice_font_glyph(const alice_font *font, uint32_t cp, alice_glyph *glyph) {
    const alice_font *f;
    size_t i;

    for (f = font; f; f = f->next) {
        for (i = 0; i < f->range_count; ++i) {
            const alice_font_range *r = &f->ranges[i];
            uint32_t index;
            int stride;

            if (cp < r->first || cp - r->first >= r->count || !r->bitmaps)
                continue;
            index = cp - r->first;
            if (r->map) {
                index = r->map[index];
                if (index == 0xFFFFu)
                    continue;
            }
            stride = (r->width + 7) / 8;
            glyph->bits    = r->bitmaps + (size_t)index * (size_t)stride * (size_t)f->height;
            glyph->width   = r->width;
            glyph->height  = f->height;
            glyph->stride  = stride;
            glyph->advance = r->advances ? r->advances[index] : r->width;

            return true;
        }
    }
    return false;
}

#ifndef ALICE_TEXT_NO_BUILTIN_FONT
const alice_font *alice_font_builtin(void)          { return &alice_font_defs[0]; }
const alice_font *alice_font_builtin_latin(void)    { return &alice_font_defs[1]; }
const alice_font *alice_font_builtin_hiragana(void) { return &alice_font_defs[2]; }
const alice_font *alice_font_builtin_katakana(void) { return &alice_font_defs[3]; }
#endif

////////////////////////////////////////////////////////////
//   Drawing   /////////////////////////////////////
////////////////////////////////////////////

void alice_text_style_init(alice_text_style *style) {
    if (!style)
        return;
    memset(style, 0, sizeof(*style));
#ifndef ALICE_TEXT_NO_BUILTIN_FONT
    style->font    = alice_font_builtin();
#endif
    style->encoding  = ALICE_ENCODING_UTF8;
    style->color     = 0xFFFFFFFFu;
    style->scale     = 1;
    style->tab_width = 4;
}

static int alice_text_clamp(int64_t value);

static void alice_text_blit(
    alice_surface *surface, const alice_glyph *g, int x, int y,
    int scale, uint32_t color
) {
    int row, col;

    if (!surface || !surface->pixels || !g || !g->bits
    ||  g->width <= 0 || g->height <= 0 || g->stride <= 0 || scale <= 0)
        return;

    for (row = 0; row < g->height; ++row) {
        const uint8_t *bits = g->bits + (size_t)row * (size_t)g->stride;
        int64_t py = (int64_t)y + (int64_t)row * scale;

        if (py >= surface->height || py + scale <= 0)
            continue;

        col = 0;
        while (col < g->width) {
            int start;
            if (!(bits[col >> 3] & (0x80u >> (col & 7)))) {
                ++col;
                continue;
            }

            start = col;
            while (col < g->width && (bits[col >> 3] & (0x80u >> (col & 7)))) {
                ++col;
            }
            
            alice_draw_rect(
                surface,
                alice_text_clamp((int64_t)x + (int64_t)start * scale),
                alice_text_clamp(py),
                alice_text_clamp((int64_t)(col - start) * scale),
                scale,
                color
            );
        }
    }
}

#define ALICE_TEXT_LIMIT        0x40000000
#define ALICE_TEXT_MAX_STYLE    0x1000000

static int alice_text_clamp(int64_t value) {
    return value > ALICE_TEXT_LIMIT ? ALICE_TEXT_LIMIT : value < -ALICE_TEXT_LIMIT ? -ALICE_TEXT_LIMIT : (int)value;
}

static int alice_text_clamp_style(int value) {
    return value > ALICE_TEXT_MAX_STYLE ? ALICE_TEXT_MAX_STYLE : value < -ALICE_TEXT_MAX_STYLE ? -ALICE_TEXT_MAX_STYLE : value;
}

static int alice_text_run(
    alice_surface *surface, const alice_text_style *style_in,
    int x, int y, alice_iter *it, int *out_w, int *out_h
) {
    alice_surface *target = surface && surface->pixels ? surface : NULL;
    alice_text_style style;
    const alice_font *font;
    alice_glyph g;
    uint32_t cp;

    int scale;
    int spacing;
    int line_spacing;
    int line_h;
    int space_adv;
    int tab_w;
    int left = alice_text_clamp(x);
    int pen_x = left, pen_y = alice_text_clamp(y);
    int64_t max_w = 0;
    int64_t lines = 1;

    if (style_in) {
        style = *style_in;
    } else {
        alice_text_style_init(&style);
    }
    font = style.font;
#ifndef ALICE_TEXT_NO_BUILTIN_FONT
    if (!font) font = alice_font_builtin();
#endif
    scale = style.scale > 0 ? alice_text_clamp_style(style.scale) : 1;
    spacing = alice_text_clamp_style(style.spacing);
    line_spacing = alice_text_clamp_style(style.line_spacing);
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!font || font->height <= 0 || !it)
        return x;

    line_h = alice_text_clamp((int64_t)font->height * scale);
    if (line_h <= 0)
        line_h = 1;
    space_adv = alice_font_glyph(font, ' ', &g) ? g.advance : font->height / 2;
    {
        int64_t tabs = style.tab_width > 0 ? style.tab_width : 4;
        if (tabs > 0x100000)
            tabs = 0x100000;
        tab_w = alice_text_clamp(tabs * ((int64_t)space_adv * scale + spacing));
    }
    if (tab_w <= 0) tab_w = 1;

    while (alice_iter_next(it, &cp)) {
        int adv;
        int64_t step;
        bool have;

        if (cp == '\n') {
            if ((int64_t)pen_x - left - spacing > max_w)
                max_w = (int64_t)pen_x - left - spacing;
            pen_x = left;
            pen_y = alice_text_clamp((int64_t)pen_y + line_h + line_spacing);
            ++lines;

            continue;
        }
        if (cp == '\t') {
            int64_t rel = (int64_t)pen_x - left, next = (rel / tab_w + 1) * tab_w;
            if (target && style.draw_background) {
                alice_draw_rect(target, pen_x, pen_y, alice_text_clamp(next - rel), line_h, style.background);
            }
            pen_x = alice_text_clamp(left + next);

            continue;
        }
        if (cp < 0x20 || (cp >= 0x7F && cp < 0xA0))
            continue;

        have = alice_font_glyph(font, cp, &g);
        if (!have && font->fallback) {
            have = alice_font_glyph(font, font->fallback, &g);
        }
        adv = have ? g.advance : space_adv;
        step = (int64_t)adv * scale + spacing;

        if (target) {
            if (style.draw_background)
                alice_draw_rect(target, pen_x, pen_y, alice_text_clamp(step), line_h, style.background);
            if (have)
                alice_text_blit(target, &g, pen_x, pen_y, scale, style.color);
        }
        pen_x = alice_text_clamp(pen_x + step);
    }

    if ((int64_t)pen_x - left - spacing > max_w) {
        max_w = (int64_t)pen_x - left - spacing;
    }
    if (out_w) *out_w = alice_text_clamp(max_w > 0 ? max_w : 0);
    if (out_h) *out_h = alice_text_clamp(lines * line_h + (lines - 1) * line_spacing);
    return pen_x;
}

static alice_encoding alice_text_encoding(const alice_text_style *style) {
    return style ? style->encoding : ALICE_ENCODING_UTF8;
}

int alice_draw_text_n(
    alice_surface *surface, int x, int y, const char *text, size_t length,
    const alice_text_style *style
) {
    alice_iter it;
    if (!surface || !surface->pixels)
        return x;
    alice_iter_bytes(&it, alice_text_encoding(style), text, length);
    return alice_text_run(surface, style, x, y, &it, NULL, NULL);
}

int alice_draw_text(
    alice_surface *surface, int x, int y, const char *text,
    const alice_text_style *style
) {
    return alice_draw_text_n(surface, x, y, text, ALICE_TEXT_NULL_TERMINATED, style);
}

int alice_draw_text_utf32(
    alice_surface *surface, int x, int y, const uint32_t *cps, size_t count,
    const alice_text_style *style
) {
    alice_iter it;
    if (!surface || !surface->pixels)
        return x;
    alice_iter_u32(&it, cps, count);
    return alice_text_run(surface, style, x, y, &it, NULL, NULL);
}

void alice_measure_text_n(
    const char *text, size_t length, const alice_text_style *style,
    int *width, int *height
) {
    alice_iter it;
    alice_iter_bytes(&it, alice_text_encoding(style), text, length);
    alice_text_run(NULL, style, 0, 0, &it, width, height);
}

void alice_measure_text(const char *text, const alice_text_style *style, int *width, int *height) {
    alice_measure_text_n(text, ALICE_TEXT_NULL_TERMINATED, style, width, height);
}

////////////////////////////////////////////////////////////
//  Tables & Font Data   ///////////////////////////
////////////////////////////////////////////

// These were generated using a python script

/* ---- Shift-JIS (Windows-31J) mapping, generated from CP932 ---- */
static const uint16_t alice_sjis_table[ALICE_SJIS_ROWS * 188] = {
 12288,12289,12290,65292,65294,12539,65306,65307,65311,65281,12443,12444,180,65344,168,65342,65507,65343,12541,12542,
 12445,12446,12291,20189,12293,12294,12295,12540,8213,8208,65295,65340,65374,8741,65372,8230,8229,8216,8217,8220,8221,
 65288,65289,12308,12309,65339,65341,65371,65373,12296,12297,12298,12299,12300,12301,12302,12303,12304,12305,65291,
 65293,177,215,247,65309,8800,65308,65310,8806,8807,8734,8756,9794,9792,176,8242,8243,8451,65509,65284,65504,65505,
 65285,65283,65286,65290,65312,167,9734,9733,9675,9679,9678,9671,9670,9633,9632,9651,9650,9661,9660,8251,12306,8594,
 8592,8593,8595,12307,0,0,0,0,0,0,0,0,0,0,0,8712,8715,8838,8839,8834,8835,8746,8745,0,0,0,0,0,0,0,0,8743,8744,65506,
 8658,8660,8704,8707,0,0,0,0,0,0,0,0,0,0,0,8736,8869,8978,8706,8711,8801,8786,8810,8811,8730,8765,8733,8757,8747,8748,
 0,0,0,0,0,0,0,8491,8240,9839,9837,9834,8224,8225,182,0,0,0,0,9711,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65296,65297,65298,
 65299,65300,65301,65302,65303,65304,65305,0,0,0,0,0,0,0,65313,65314,65315,65316,65317,65318,65319,65320,65321,65322,
 65323,65324,65325,65326,65327,65328,65329,65330,65331,65332,65333,65334,65335,65336,65337,65338,0,0,0,0,0,0,65345,
 65346,65347,65348,65349,65350,65351,65352,65353,65354,65355,65356,65357,65358,65359,65360,65361,65362,65363,65364,
 65365,65366,65367,65368,65369,65370,0,0,0,0,12353,12354,12355,12356,12357,12358,12359,12360,12361,12362,12363,12364,
 12365,12366,12367,12368,12369,12370,12371,12372,12373,12374,12375,12376,12377,12378,12379,12380,12381,12382,12383,
 12384,12385,12386,12387,12388,12389,12390,12391,12392,12393,12394,12395,12396,12397,12398,12399,12400,12401,12402,
 12403,12404,12405,12406,12407,12408,12409,12410,12411,12412,12413,12414,12415,12416,12417,12418,12419,12420,12421,
 12422,12423,12424,12425,12426,12427,12428,12429,12430,12431,12432,12433,12434,12435,0,0,0,0,0,0,0,0,0,0,0,12449,
 12450,12451,12452,12453,12454,12455,12456,12457,12458,12459,12460,12461,12462,12463,12464,12465,12466,12467,12468,
 12469,12470,12471,12472,12473,12474,12475,12476,12477,12478,12479,12480,12481,12482,12483,12484,12485,12486,12487,
 12488,12489,12490,12491,12492,12493,12494,12495,12496,12497,12498,12499,12500,12501,12502,12503,12504,12505,12506,
 12507,12508,12509,12510,12511,12512,12513,12514,12515,12516,12517,12518,12519,12520,12521,12522,12523,12524,12525,
 12526,12527,12528,12529,12530,12531,12532,12533,12534,0,0,0,0,0,0,0,0,913,914,915,916,917,918,919,920,921,922,923,
 924,925,926,927,928,929,931,932,933,934,935,936,937,0,0,0,0,0,0,0,0,945,946,947,948,949,950,951,952,953,954,955,956,
 957,958,959,960,961,963,964,965,966,967,968,969,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,1040,1041,1042,1043,1044,1045,1025,1046,1047,1048,1049,1050,1051,1052,1053,1054,1055,1056,1057,1058,1059,
 1060,1061,1062,1063,1064,1065,1066,1067,1068,1069,1070,1071,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1072,1073,1074,1075,1076,
 1077,1105,1078,1079,1080,1081,1082,1083,1084,1085,1086,1087,1088,1089,1090,1091,1092,1093,1094,1095,1096,1097,1098,
 1099,1100,1101,1102,1103,0,0,0,0,0,0,0,0,0,0,0,0,0,9472,9474,9484,9488,9496,9492,9500,9516,9508,9524,9532,9473,9475,
 9487,9491,9499,9495,9507,9523,9515,9531,9547,9504,9519,9512,9527,9535,9501,9520,9509,9528,9538,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9312,9313,9314,9315,9316,9317,9318,9319,9320,9321,9322,9323,9324,9325,9326,
 9327,9328,9329,9330,9331,8544,8545,8546,8547,8548,8549,8550,8551,8552,8553,0,13129,13076,13090,13133,13080,13095,
 13059,13110,13137,13143,13069,13094,13091,13099,13130,13115,13212,13213,13214,13198,13199,13252,13217,0,0,0,0,0,0,0,
 0,13179,12317,12319,8470,13261,8481,12964,12965,12966,12967,12968,12849,12850,12857,13182,13181,13180,8786,8801,8747,
 8750,8721,8730,8869,8736,8735,8895,8757,8745,8746,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,
#ifndef ALICE_TEXT_SJIS_BASIC
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20124,21782,23043,38463,21696,24859,25384,
 23030,36898,33909,33564,31312,24746,25569,28197,26093,33894,33446,39925,26771,22311,26017,25201,23451,22992,34427,
 39156,32098,32190,39822,25110,31903,34999,23433,24245,25353,26263,26696,38343,38797,26447,20197,20234,20301,20381,
 20553,22258,22839,22996,23041,23561,24799,24847,24944,26131,26885,28858,30031,30064,31227,32173,32239,32963,33806,
 34915,35586,36949,36986,21307,20117,20133,22495,32946,37057,30959,19968,22769,28322,36920,31282,33576,33419,39983,
 20801,21360,21693,21729,22240,23035,24341,39154,28139,32996,34093,38498,38512,38560,38907,21515,21491,23431,28879,
 32701,36802,38632,21359,40284,31418,19985,30867,33276,28198,22040,21764,27421,34074,39995,23013,21417,28006,29916,
 38287,22082,20113,36939,38642,33615,39180,21473,21942,23344,24433,26144,26355,26628,27704,27891,27945,29787,30408,
 31310,38964,33521,34907,35424,37613,28082,30123,30410,39365,24742,35585,36234,38322,27022,21421,20870,22290,22576,
 22852,23476,24310,24616,25513,25588,27839,28436,28814,28948,29017,29141,29503,32257,33398,33489,34199,36960,37467,
 40219,22633,26044,27738,29989,20985,22830,22885,24448,24540,25276,26106,27178,27431,27572,29579,32705,35158,40236,
 40206,40644,23713,27798,33659,20740,23627,25014,33222,26742,29281,20057,20474,21368,24681,28201,31311,38899,19979,
 21270,20206,20309,20285,20385,20339,21152,21487,22025,22799,23233,23478,23521,31185,26247,26524,26550,27468,27827,
 28779,29634,31117,31166,31292,31623,33457,33499,33540,33655,33775,33747,34662,35506,22057,36008,36838,36942,38686,
 34442,20420,23784,25105,29273,30011,33253,33469,34558,36032,38597,39187,39381,20171,20250,35299,22238,22602,22730,
 24315,24555,24618,24724,24674,25040,25106,25296,25913,39745,26214,26800,28023,28784,30028,30342,32117,33445,34809,
 38283,38542,35997,20977,21182,22806,21683,23475,23830,24936,27010,28079,30861,33995,34903,35442,37799,39608,28012,
 39336,34521,22435,26623,34510,37390,21123,22151,21508,24275,25313,25785,26684,26680,27579,29554,30906,31339,35226,
 35282,36203,36611,37101,38307,38548,38761,23398,23731,27005,38989,38990,25499,31520,27179,27263,26806,39949,28511,
 21106,21917,24688,25324,27963,28167,28369,33883,35088,36676,19988,39993,21494,26907,27194,38788,26666,20828,31427,
 33970,37340,37772,22107,40232,26658,33541,33841,31909,21000,33477,29926,20094,20355,20896,23506,21002,21208,21223,
 24059,21914,22570,23014,23436,23448,23515,24178,24185,24739,24863,24931,25022,25563,25954,26577,26707,26874,27454,
 27475,27735,28450,28567,28485,29872,29976,30435,30475,31487,31649,31777,32233,32566,32752,32925,33382,33694,35251,
 35532,36011,36996,37969,38291,38289,38306,38501,38867,39208,33304,20024,21547,23736,24012,29609,30284,30524,23721,
 32747,36107,38593,38929,38996,39000,20225,20238,21361,21916,22120,22522,22855,23305,23492,23696,24076,24190,24524,
 25582,26426,26071,26082,26399,26827,26820,27231,24112,27589,27671,27773,30079,31048,23395,31232,32000,24509,35215,
 35352,36020,36215,36556,36637,39138,39438,39740,20096,20605,20736,22931,23452,25135,25216,25836,27450,29344,30097,
 31047,32681,34811,35516,35696,25516,33738,38816,21513,21507,21931,26708,27224,35440,30759,26485,40653,21364,23458,
 33050,34384,36870,19992,20037,20167,20241,21450,21560,23470,24339,24613,25937,26429,27714,27762,27875,28792,29699,
 31350,31406,31496,32026,31998,32102,26087,29275,21435,23621,24040,25298,25312,25369,28192,34394,35377,36317,37624,
 28417,31142,39770,20136,20139,20140,20379,20384,20689,20807,31478,20849,20982,21332,21281,21375,21483,21932,22659,
 23777,24375,24394,24623,24656,24685,25375,25945,27211,27841,29378,29421,30703,33016,33029,33288,34126,37111,37857,
 38911,39255,39514,20208,20957,23597,26241,26989,23616,26354,26997,29577,26704,31873,20677,21220,22343,24062,37670,
 26020,27427,27453,29748,31105,31165,31563,32202,33465,33740,34943,35167,35641,36817,37329,21535,37504,20061,20534,
 21477,21306,29399,29590,30697,33510,36527,39366,39368,39378,20855,24858,34398,21936,31354,20598,23507,36935,38533,
 20018,27355,37351,23633,23624,25496,31391,27795,38772,36705,31402,29066,38536,31874,26647,32368,26705,37740,21234,
 21531,34219,35347,32676,36557,37089,21350,34952,31041,20418,20670,21009,20804,21843,22317,29674,22411,22865,24418,
 24452,24693,24950,24935,25001,25522,25658,25964,26223,26690,28179,30054,31293,31995,32076,32153,32331,32619,33550,
 33610,34509,35336,35427,35686,36605,38938,40335,33464,36814,39912,21127,25119,25731,28608,38553,26689,20625,27424,
 27770,28500,31348,32080,34880,35363,26376,20214,20537,20518,20581,20860,21048,21091,21927,22287,22533,23244,24314,
 25010,25080,25331,25458,26908,27177,29309,29356,29486,30740,30831,32121,30476,32937,35211,35609,36066,36562,36963,
 37749,38522,38997,39443,40568,20803,21407,21427,24187,24358,28187,28304,29572,29694,32067,33335,35328,35578,38480,
 20046,20491,21476,21628,22266,22993,23396,24049,24235,24359,25144,25925,26543,28246,29392,31946,34996,32929,32993,
 33776,34382,35463,36328,37431,38599,39015,40723,20116,20114,20237,21320,21577,21566,23087,24460,24481,24735,26791,
 27278,29786,30849,35486,35492,35703,37264,20062,39881,20132,20348,20399,20505,20502,20809,20844,21151,21177,21246,
 21402,21475,21521,21518,21897,22353,22434,22909,23380,23389,23439,24037,24039,24055,24184,24195,24218,24247,24344,
 24658,24908,25239,25304,25511,25915,26114,26179,26356,26477,26657,26775,27083,27743,27946,28009,28207,28317,30002,
 30343,30828,31295,31968,32005,32024,32094,32177,32789,32771,32943,32945,33108,33167,33322,33618,34892,34913,35611,
 36002,36092,37066,37237,37489,30783,37628,38308,38477,38917,39321,39640,40251,21083,21163,21495,21512,22741,25335,
 28640,35946,36703,40633,20811,21051,21578,22269,31296,37239,40288,40658,29508,28425,33136,29969,24573,24794,39592,
 29403,36796,27492,38915,20170,22256,22372,22718,23130,24680,25031,26127,26118,26681,26801,28151,30165,32058,33390,
 39746,20123,20304,21449,21766,23919,24038,24046,26619,27801,29811,30722,35408,37782,35039,22352,24231,25387,20661,
 20652,20877,26368,21705,22622,22971,23472,24425,25165,25505,26685,27507,28168,28797,37319,29312,30741,30758,31085,
 25998,32048,33756,35009,36617,38555,21092,22312,26448,32618,36001,20916,22338,38442,22586,27018,32948,21682,23822,
 22524,30869,40442,20316,21066,21643,25662,26152,26388,26613,31364,31574,32034,37679,26716,39853,31545,21273,20874,
 21047,23519,25334,25774,25830,26413,27578,34217,38609,30352,39894,25420,37638,39851,30399,26194,19977,20632,21442,
 23665,24808,25746,25955,26719,29158,29642,29987,31639,32386,34453,35715,36059,37240,39184,26028,26283,27531,20181,
 20180,20282,20351,21050,21496,21490,21987,22235,22763,22987,22985,23039,23376,23629,24066,24107,24535,24605,25351,
 25903,23388,26031,26045,26088,26525,27490,27515,27663,29509,31049,31169,31992,32025,32043,32930,33026,33267,35222,
 35422,35433,35430,35468,35566,36039,36060,38604,39164,27503,20107,20284,20365,20816,23383,23546,24904,25345,26178,
 27425,28363,27835,29246,29885,30164,30913,31034,32780,32819,33258,33940,36766,27728,40575,24335,35672,40235,31482,
 36600,23437,38635,19971,21489,22519,22833,23241,23460,24713,28287,28422,30142,36074,23455,34048,31712,20594,26612,
 33437,23649,34122,32286,33294,20889,23556,25448,36198,26012,29038,31038,32023,32773,35613,36554,36974,34503,37034,
 20511,21242,23610,26451,28796,29237,37196,37320,37675,33509,23490,24369,24825,20027,21462,23432,25163,26417,27530,
 29417,29664,31278,33131,36259,37202,39318,20754,21463,21610,23551,25480,27193,32172,38656,22234,21454,21608,23447,
 23601,24030,20462,24833,25342,27954,31168,31179,32066,32333,32722,33261,33311,33936,34886,35186,35728,36468,36655,
 36913,37195,37228,38598,37276,20160,20303,20805,21313,24467,25102,26580,27713,28171,29539,32294,37325,37507,21460,
 22809,23487,28113,31069,32302,31899,22654,29087,20986,34899,36848,20426,23803,26149,30636,31459,33308,39423,20934,
 24490,26092,26991,27529,28147,28310,28516,30462,32020,24033,36981,37255,38918,20966,21021,25152,26257,26329,28186,
 24246,32210,32626,26360,34223,34295,35576,21161,21465,22899,24207,24464,24661,37604,38500,20663,20767,21213,21280,
 21319,21484,21736,21830,21809,22039,22888,22974,23100,23477,23558,23567,23569,23578,24196,24202,24288,24432,25215,
 25220,25307,25484,25463,26119,26124,26157,26230,26494,26786,27167,27189,27836,28040,28169,28248,28988,28966,29031,
 30151,30465,30813,30977,31077,31216,31456,31505,31911,32057,32918,33750,33931,34121,34909,35059,35359,35388,35412,
 35443,35937,36062,37284,37478,37758,37912,38556,38808,19978,19976,19998,20055,20887,21104,22478,22580,22732,23330,
 24120,24773,25854,26465,26454,27972,29366,30067,31331,33976,35698,37304,37664,22065,22516,39166,25325,26893,27542,
 29165,32340,32887,33394,35302,39135,34645,36785,23611,20280,20449,20405,21767,23072,23517,23529,24515,24910,25391,
 26032,26187,26862,27035,28024,28145,30003,30137,30495,31070,31206,32051,33251,33455,34218,35242,35386,36523,36763,
 36914,37341,38663,20154,20161,20995,22645,22764,23563,29978,23613,33102,35338,36805,38499,38765,31525,35535,38920,
 37218,22259,21416,36887,21561,22402,24101,25512,27700,28810,30561,31883,32736,34928,36930,37204,37648,37656,38543,
 29790,39620,23815,23913,25968,26530,36264,38619,25454,26441,26905,33733,38935,38592,35070,28548,25722,23544,19990,
 28716,30045,26159,20932,21046,21218,22995,24449,24615,25104,25919,25972,26143,26228,26866,26646,27491,28165,29298,
 29983,30427,31934,32854,22768,35069,35199,35488,35475,35531,36893,37266,38738,38745,25993,31246,33030,38587,24109,
 24796,25114,26021,26132,26512,30707,31309,31821,32318,33034,36012,36196,36321,36447,30889,20999,25305,25509,25666,
 25240,35373,31363,31680,35500,38634,32118,33292,34633,20185,20808,21315,21344,23459,23554,23574,24029,25126,25159,
 25776,26643,26676,27849,27973,27927,26579,28508,29006,29053,26059,31359,31661,32218,32330,32680,33146,33307,33337,
 34214,35438,36046,36341,36984,36983,37549,37521,38275,39854,21069,21892,28472,28982,20840,31109,32341,33203,31950,
 22092,22609,23720,25514,26366,26365,26970,29401,30095,30094,30990,31062,31199,31895,32032,32068,34311,35380,38459,
 36961,40736,20711,21109,21452,21474,20489,21930,22766,22863,29245,23435,23652,21277,24803,24819,25436,25475,25407,
 25531,25805,26089,26361,24035,27085,27133,28437,29157,20105,30185,30456,31379,31967,32207,32156,32865,33609,33624,
 33900,33980,34299,35013,36208,36865,36973,37783,38684,39442,20687,22679,24974,33235,34101,36104,36896,20419,20596,
 21063,21363,24687,25417,26463,28204,36275,36895,20439,23646,36042,26063,32154,21330,34966,20854,25539,23384,23403,
 23562,25613,26449,36956,20182,22810,22826,27760,35409,21822,22549,22949,24816,25171,26561,33333,26965,38464,39364,
 39464,20307,22534,23550,32784,23729,24111,24453,24608,24907,25140,26367,27888,28382,32974,33151,33492,34955,36024,
 36864,36910,38538,40667,39899,20195,21488,22823,31532,37261,38988,40441,28381,28711,21331,21828,23429,25176,25246,
 25299,27810,28655,29730,35351,37944,28609,35582,33592,20967,34552,21482,21481,20294,36948,36784,22890,33073,24061,
 31466,36799,26842,35895,29432,40008,27197,35504,20025,21336,22022,22374,25285,25506,26086,27470,28129,28251,28845,
 30701,31471,31658,32187,32829,32966,34507,35477,37723,22243,22727,24382,26029,26262,27264,27573,30007,35527,20516,
 30693,22320,24347,24677,26234,27744,30196,31258,32622,33268,34584,36933,39347,31689,30044,31481,31569,33988,36880,
 31209,31378,33590,23265,30528,20013,20210,23449,24544,25277,26172,26609,27880,34411,34935,35387,37198,37619,39376,
 27159,28710,29482,33511,33879,36015,19969,20806,20939,21899,23541,24086,24115,24193,24340,24373,24427,24500,25074,
 25361,26274,26397,28526,29266,30010,30522,32884,33081,33144,34678,35519,35548,36229,36339,37530,38263,38914,40165,
 21189,25431,30452,26389,27784,29645,36035,37806,38515,27941,22684,26894,27084,36861,37786,30171,36890,22618,26626,
 25524,27131,20291,28460,26584,36795,34086,32180,37716,26943,28528,22378,22775,23340,32044,29226,21514,37347,40372,
 20141,20302,20572,20597,21059,35998,21576,22564,23450,24093,24213,24237,24311,24351,24716,25269,25402,25552,26799,
 27712,30855,31118,31243,32224,33351,35330,35558,36420,36883,37048,37165,37336,40718,27877,25688,25826,25973,28404,
 30340,31515,36969,37841,28346,21746,24505,25764,36685,36845,37444,20856,22635,22825,23637,24215,28155,32399,29980,
 36028,36578,39003,28857,20253,27583,28593,30000,38651,20814,21520,22581,22615,22956,23648,24466,26007,26460,28193,
 30331,33759,36077,36884,37117,37709,30757,30778,21162,24230,22303,22900,24594,20498,20826,20908,20941,20992,21776,
 22612,22616,22871,23445,23798,23947,24764,25237,25645,26481,26691,26812,26847,30423,28120,28271,28059,28783,29128,
 24403,30168,31095,31561,31572,31570,31958,32113,21040,33891,34153,34276,35342,35588,35910,36367,36867,36879,37913,
 38518,38957,39472,38360,20685,21205,21516,22530,23566,24999,25758,27934,30643,31461,33012,33796,36947,37509,23776,
 40199,21311,24471,24499,28060,29305,30563,31167,31716,27602,29420,35501,26627,27233,20984,31361,26932,23626,40182,
 33515,23493,37193,28702,22136,23663,24775,25958,27788,35930,36929,38931,21585,26311,37389,22856,37027,20869,20045,
 20970,34201,35598,28760,25466,37707,26978,39348,32260,30071,21335,26976,36575,38627,27741,20108,23612,24336,36841,
 21250,36049,32905,34425,24319,26085,20083,20837,22914,23615,38894,20219,22922,24525,35469,28641,31152,31074,23527,
 33905,29483,29105,24180,24565,25467,25754,29123,31896,20035,24316,20043,22492,22178,24745,28611,32013,33021,33075,
 33215,36786,35223,34468,24052,25226,25773,35207,26487,27874,27966,29750,30772,23110,32629,33453,39340,20467,24259,
 25309,25490,25943,26479,30403,29260,32972,32954,36649,37197,20493,22521,23186,26757,26995,29028,29437,36023,22770,
 36064,38506,36889,34687,31204,30695,33833,20271,21093,21338,25293,26575,27850,30333,31636,31893,33334,34180,36843,
 26333,28448,29190,32283,33707,39361,40614,20989,31665,30834,31672,32903,31560,27368,24161,32908,30033,30048,20843,
 37474,28300,30330,37271,39658,20240,32624,25244,31567,38309,40169,22138,22617,34532,38588,20276,21028,21322,21453,
 21467,24070,25644,26001,26495,27710,27726,29256,29359,29677,30036,32321,33324,34281,36009,31684,37318,29033,38930,
 39151,25405,26217,30058,30436,30928,34115,34542,21290,21329,21542,22915,24199,24444,24754,25161,25209,25259,26000,
 27604,27852,30130,30382,30865,31192,32203,32631,32933,34987,35513,36027,36991,38750,39131,27147,31800,20633,23614,
 24494,26503,27608,29749,30473,32654,40763,26570,31255,21305,30091,39661,24422,33181,33777,32920,24380,24517,30050,
 31558,36924,26727,23019,23195,32016,30334,35628,20469,24426,27161,27703,28418,29922,31080,34920,35413,35961,24287,
 25551,30149,31186,33495,37672,37618,33948,34541,39981,21697,24428,25996,27996,28693,36007,36051,38971,25935,29942,
 19981,20184,22496,22827,23142,23500,20904,24067,24220,24598,25206,25975,26023,26222,28014,29238,31526,33104,33178,
 33433,35676,36000,36070,36212,38428,38468,20398,25771,27494,33310,33889,34154,37096,23553,26963,39080,33914,34135,
 20239,21103,24489,24133,26381,31119,33145,35079,35206,28149,24343,25173,27832,20175,29289,39826,20998,21563,22132,
 22707,24996,25198,28954,22894,31881,31966,32027,38640,25991,32862,19993,20341,20853,22592,24163,24179,24330,26564,
 20006,34109,38281,38491,31859,38913,20731,22721,30294,30887,21029,30629,34065,31622,20559,22793,29255,31687,32232,
 36794,36820,36941,20415,21193,23081,24321,38829,20445,33303,37610,22275,25429,27497,29995,35036,36628,31298,21215,
 22675,24917,25098,26286,27597,31807,33769,20515,20472,21253,21574,22577,22857,23453,23792,23791,23849,24214,25265,
 25447,25918,26041,26379,27861,27873,28921,30770,32299,32990,33459,33804,34028,34562,35090,35370,35914,37030,37586,
 39165,40179,40300,20047,20129,20621,21078,22346,22952,24125,24536,24537,25151,26292,26395,26576,26834,20882,32033,
 32938,33192,35584,35980,36031,37502,38450,21536,38956,21271,20693,21340,22696,25778,26420,29287,30566,31302,37350,
 21187,27809,27526,22528,24140,22868,26412,32763,20961,30406,25705,30952,39764,40635,22475,22969,26151,26522,27598,
 21737,27097,24149,33180,26517,39850,26622,40018,26717,20134,20451,21448,25273,26411,27819,36804,20397,32365,40639,
 19975,24930,28288,28459,34067,21619,26410,39749,24051,31637,23724,23494,34588,28234,34001,31252,33032,22937,31885,
 27665,30496,21209,22818,28961,29279,30683,38695,40289,26891,23167,23064,20901,21517,21629,26126,30431,36855,37528,
 40180,23018,29277,28357,20813,26825,32191,32236,38754,40634,25720,27169,33538,22916,23391,27611,29467,30450,32178,
 32791,33945,20786,26408,40665,30446,26466,21247,39173,23588,25147,31870,36016,21839,24758,32011,38272,21249,20063,
 20918,22812,29242,32822,37326,24357,30690,21380,24441,32004,34220,35379,36493,38742,26611,34222,37971,24841,24840,
 27833,30290,35565,36664,21807,20305,20778,21191,21451,23461,24189,24736,24962,25558,26377,26586,28263,28044,29494,
 29495,30001,31056,35029,35480,36938,37009,37109,38596,34701,22805,20104,20313,19982,35465,36671,38928,20653,24188,
 22934,23481,24248,25562,25594,25793,26332,26954,27096,27915,28342,29076,29992,31407,32650,32768,33865,33993,35201,
 35617,36362,36965,38525,39178,24958,25233,27442,27779,28020,32716,32764,28096,32645,34746,35064,26469,33713,38972,
 38647,27931,32097,33853,37226,20081,21365,23888,27396,28651,34253,34349,35239,21033,21519,23653,26446,26792,29702,
 29827,30178,35023,35041,37324,38626,38520,24459,29575,31435,33870,25504,30053,21129,27969,28316,29705,30041,30827,
 31890,38534,31452,40845,20406,24942,26053,34396,20102,20142,20698,20001,20940,23534,26009,26753,28092,29471,30274,
 30637,31260,31975,33391,35538,36988,37327,38517,38936,21147,32209,20523,21400,26519,28107,29136,29747,33256,36650,
 38563,40023,40607,29792,22593,28057,32047,39006,20196,20278,20363,20919,21169,23994,24604,29618,31036,33491,37428,
 38583,38646,38666,40599,40802,26278,27508,21015,21155,28872,35010,24265,24651,24976,28451,29001,31806,32244,32879,
 34030,36899,37676,21570,39791,27347,28809,36034,36335,38706,21172,23105,24266,24324,26391,27004,27028,28010,28431,
 29282,29436,31725,32769,32894,34635,37070,20845,40595,31108,32907,37682,35542,20525,21644,35441,27498,36036,33031,
 24785,26528,40434,20121,20120,39952,35435,34241,34152,26880,28286,30871,33109,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24332,19984,19989,20010,20017,20022,20028,20031,20034,20054,20056,
 20098,20101,35947,20106,33298,24333,20110,20126,20127,20128,20130,20144,20147,20150,20174,20173,20164,20166,20162,
 20183,20190,20205,20191,20215,20233,20314,20272,20315,20317,20311,20295,20342,20360,20367,20376,20347,20329,20336,
 20369,20335,20358,20374,20760,20436,20447,20430,20440,20443,20433,20442,20432,20452,20453,20506,20520,20500,20522,
 20517,20485,20252,20470,20513,20521,20524,20478,20463,20497,20486,20547,20551,26371,20565,20560,20552,20570,20566,
 20588,20600,20608,20634,20613,20660,20658,20681,20682,20659,20674,20694,20702,20709,20717,20707,20718,20729,20725,
 20745,20737,20738,20758,20757,20756,20762,20769,20794,20791,20796,20795,20799,20800,20818,20812,20820,20834,31480,
 20841,20842,20846,20864,20866,22232,20876,20873,20879,20881,20883,20885,20886,20900,20902,20898,20905,20906,20907,
 20915,20913,20914,20912,20917,20925,20933,20937,20955,20960,34389,20969,20973,20976,20981,20990,20996,21003,21012,
 21006,21031,21034,21038,21043,21049,21071,21060,21067,21068,21086,21076,21098,21108,21097,21107,21119,21117,21133,
 21140,21138,21105,21128,21137,36776,36775,21164,21165,21180,21173,21185,21197,21207,21214,21219,21222,39149,21216,
 21235,21237,21240,21241,21254,21256,30008,21261,21264,21263,21269,21274,21283,21295,21297,21299,21304,21312,21318,
 21317,19991,21321,21325,20950,21342,21353,21358,22808,21371,21367,21378,21398,21408,21414,21413,21422,21424,21430,
 21443,31762,38617,21471,26364,29166,21486,21480,21485,21498,21505,21565,21568,21548,21549,21564,21550,21558,21545,
 21533,21582,21647,21621,21646,21599,21617,21623,21616,21650,21627,21632,21622,21636,21648,21638,21703,21666,21688,
 21669,21676,21700,21704,21672,21675,21698,21668,21694,21692,21720,21733,21734,21775,21780,21757,21742,21741,21754,
 21730,21817,21824,21859,21836,21806,21852,21829,21846,21847,21816,21811,21853,21913,21888,21679,21898,21919,21883,
 21886,21912,21918,21934,21884,21891,21929,21895,21928,21978,21957,21983,21956,21980,21988,21972,22036,22007,22038,
 22014,22013,22043,22009,22094,22096,29151,22068,22070,22066,22072,22123,22116,22063,22124,22122,22150,22144,22154,
 22176,22164,22159,22181,22190,22198,22196,22210,22204,22209,22211,22208,22216,22222,22225,22227,22231,22254,22265,
 22272,22271,22276,22281,22280,22283,22285,22291,22296,22294,21959,22300,22310,22327,22328,22350,22331,22336,22351,
 22377,22464,22408,22369,22399,22409,22419,22432,22451,22436,22442,22448,22467,22470,22484,22482,22483,22538,22486,
 22499,22539,22553,22557,22642,22561,22626,22603,22640,27584,22610,22589,22649,22661,22713,22687,22699,22714,22750,
 22715,22712,22702,22725,22739,22737,22743,22745,22744,22757,22748,22756,22751,22767,22778,22777,22779,22780,22781,
 22786,22794,22800,22811,26790,22821,22828,22829,22834,22840,22846,31442,22869,22864,22862,22874,22872,22882,22880,
 22887,22892,22889,22904,22913,22941,20318,20395,22947,22962,22982,23016,23004,22925,23001,23002,23077,23071,23057,
 23068,23049,23066,23104,23148,23113,23093,23094,23138,23146,23194,23228,23230,23243,23234,23229,23267,23255,23270,
 23273,23254,23290,23291,23308,23307,23318,23346,23248,23338,23350,23358,23363,23365,23360,23377,23381,23386,23387,
 23397,23401,23408,23411,23413,23416,25992,23418,23424,23427,23462,23480,23491,23495,23497,23508,23504,23524,23526,
 23522,23518,23525,23531,23536,23542,23539,23557,23559,23560,23565,23571,23584,23586,23592,23608,23609,23617,23622,
 23630,23635,23632,23631,23409,23660,23662,20066,23670,23673,23692,23697,23700,22939,23723,23739,23734,23740,23735,
 23749,23742,23751,23769,23785,23805,23802,23789,23948,23786,23819,23829,23831,23900,23839,23835,23825,23828,23842,
 23834,23833,23832,23884,23890,23886,23883,23916,23923,23926,23943,23940,23938,23970,23965,23980,23982,23997,23952,
 23991,23996,24009,24013,24019,24018,24022,24027,24043,24050,24053,24075,24090,24089,24081,24091,24118,24119,24132,
 24131,24128,24142,24151,24148,24159,24162,24164,24135,24181,24182,24186,40636,24191,24224,24257,24258,24264,24272,
 24271,24278,24291,24285,24282,24283,24290,24289,24296,24297,24300,24305,24307,24304,24308,24312,24318,24323,24329,
 24413,24412,24331,24337,24342,24361,24365,24376,24385,24392,24396,24398,24367,24401,24406,24407,24409,24417,24429,
 24435,24439,24451,24450,24447,24458,24456,24465,24455,24478,24473,24472,24480,24488,24493,24508,24534,24571,24548,
 24568,24561,24541,24755,24575,24609,24672,24601,24592,24617,24590,24625,24603,24597,24619,24614,24591,24634,24666,
 24641,24682,24695,24671,24650,24646,24653,24675,24643,24676,24642,24684,24683,24665,24705,24717,24807,24707,24730,
 24708,24731,24726,24727,24722,24743,24715,24801,24760,24800,24787,24756,24560,24765,24774,24757,24792,24909,24853,
 24838,24822,24823,24832,24820,24826,24835,24865,24827,24817,24845,24846,24903,24894,24872,24871,24906,24895,24892,
 24876,24884,24893,24898,24900,24947,24951,24920,24921,24922,24939,24948,24943,24933,24945,24927,24925,24915,24949,
 24985,24982,24967,25004,24980,24986,24970,24977,25003,25006,25036,25034,25033,25079,25032,25027,25030,25018,25035,
 32633,25037,25062,25059,25078,25082,25076,25087,25085,25084,25086,25088,25096,25097,25101,25100,25108,25115,25118,
 25121,25130,25134,25136,25138,25139,25153,25166,25182,25187,25179,25184,25192,25212,25218,25225,25214,25234,25235,
 25238,25300,25219,25236,25303,25297,25275,25295,25343,25286,25812,25288,25308,25292,25290,25282,25287,25243,25289,
 25356,25326,25329,25383,25346,25352,25327,25333,25424,25406,25421,25628,25423,25494,25486,25472,25515,25462,25507,
 25487,25481,25503,25525,25451,25449,25534,25577,25536,25542,25571,25545,25554,25590,25540,25622,25652,25606,25619,
 25638,25654,25885,25623,25640,25615,25703,25711,25718,25678,25898,25749,25747,25765,25769,25736,25788,25818,25810,
 25797,25799,25787,25816,25794,25841,25831,33289,25824,25825,25260,25827,25839,25900,25846,25844,25842,25850,25856,
 25853,25880,25884,25861,25892,25891,25899,25908,25909,25911,25910,25912,30027,25928,25942,25941,25933,25944,25950,
 25949,25970,25976,25986,25987,35722,26011,26015,26027,26039,26051,26054,26049,26052,26060,26066,26075,26073,26080,
 26081,26097,26482,26122,26115,26107,26483,26165,26166,26164,26140,26191,26180,26185,26177,26206,26205,26212,26215,
 26216,26207,26210,26224,26243,26248,26254,26249,26244,26264,26269,26305,26297,26313,26302,26300,26308,26296,26326,
 26330,26336,26175,26342,26345,26352,26357,26359,26383,26390,26398,26406,26407,38712,26414,26431,26422,26433,26424,
 26423,26438,26462,26464,26457,26467,26468,26505,26480,26537,26492,26474,26508,26507,26534,26529,26501,26551,26607,
 26548,26604,26547,26601,26552,26596,26590,26589,26594,26606,26553,26574,26566,26599,27292,26654,26694,26665,26688,
 26701,26674,26702,26803,26667,26713,26723,26743,26751,26783,26767,26797,26772,26781,26779,26755,27310,26809,26740,
 26805,26784,26810,26895,26765,26750,26881,26826,26888,26840,26914,26918,26849,26892,26829,26836,26855,26837,26934,
 26898,26884,26839,26851,26917,26873,26848,26863,26920,26922,26906,26915,26913,26822,27001,26999,26972,27000,26987,
 26964,27006,26990,26937,26996,26941,26969,26928,26977,26974,26973,27009,26986,27058,27054,27088,27071,27073,27091,
 27070,27086,23528,27082,27101,27067,27075,27047,27182,27025,27040,27036,27029,27060,27102,27112,27138,27163,27135,
 27402,27129,27122,27111,27141,27057,27166,27117,27156,27115,27146,27154,27329,27171,27155,27204,27148,27250,27190,
 27256,27207,27234,27225,27238,27208,27192,27170,27280,27277,27296,27268,27298,27299,27287,34327,27323,27331,27330,
 27320,27315,27308,27358,27345,27359,27306,27354,27370,27387,27397,34326,27386,27410,27414,39729,27423,27448,27447,
 30428,27449,39150,27463,27459,27465,27472,27481,27476,27483,27487,27489,27512,27513,27519,27520,27524,27523,27533,
 27544,27541,27550,27556,27562,27563,27567,27570,27569,27571,27575,27580,27590,27595,27603,27615,27628,27627,27635,
 27631,40638,27656,27667,27668,27675,27684,27683,27742,27733,27746,27754,27778,27789,27802,27777,27803,27774,27752,
 27763,27794,27792,27844,27889,27859,27837,27863,27845,27869,27822,27825,27838,27834,27867,27887,27865,27882,27935,
 34893,27958,27947,27965,27960,27929,27957,27955,27922,27916,28003,28051,28004,27994,28025,27993,28046,28053,28644,
 28037,28153,28181,28170,28085,28103,28134,28088,28102,28140,28126,28108,28136,28114,28101,28154,28121,28132,28117,
 28138,28142,28205,28270,28206,28185,28274,28255,28222,28195,28267,28203,28278,28237,28191,28227,28218,28238,28196,
 28415,28189,28216,28290,28330,28312,28361,28343,28371,28349,28335,28356,28338,28372,28373,28303,28325,28354,28319,
 28481,28433,28748,28396,28408,28414,28479,28402,28465,28399,28466,28364,28478,28435,28407,28550,28538,28536,28545,
 28544,28527,28507,28659,28525,28546,28540,28504,28558,28561,28610,28518,28595,28579,28577,28580,28601,28614,28586,
 28639,28629,28652,28628,28632,28657,28654,28635,28681,28683,28666,28689,28673,28687,28670,28699,28698,28532,28701,
 28696,28703,28720,28734,28722,28753,28771,28825,28818,28847,28913,28844,28856,28851,28846,28895,28875,28893,28889,
 28937,28925,28956,28953,29029,29013,29064,29030,29026,29004,29014,29036,29071,29179,29060,29077,29096,29100,29143,
 29113,29118,29138,29129,29140,29134,29152,29164,29159,29173,29180,29177,29183,29197,29200,29211,29224,29229,29228,
 29232,29234,29243,29244,29247,29248,29254,29259,29272,29300,29310,29314,29313,29319,29330,29334,29346,29351,29369,
 29362,29379,29382,29380,29390,29394,29410,29408,29409,29433,29431,20495,29463,29450,29468,29462,29469,29492,29487,
 29481,29477,29502,29518,29519,40664,29527,29546,29544,29552,29560,29557,29563,29562,29640,29619,29646,29627,29632,
 29669,29678,29662,29858,29701,29807,29733,29688,29746,29754,29781,29759,29791,29785,29761,29788,29801,29808,29795,
 29802,29814,29822,29835,29854,29863,29898,29903,29908,29681,29920,29923,29927,29929,29934,29938,29936,29937,29944,
 29943,29956,29955,29957,29964,29966,29965,29973,29971,29982,29990,29996,30012,30020,30029,30026,30025,30043,30022,
 30042,30057,30052,30055,30059,30061,30072,30070,30086,30087,30068,30090,30089,30082,30100,30106,30109,30117,30115,
 30146,30131,30147,30133,30141,30136,30140,30129,30157,30154,30162,30169,30179,30174,30206,30207,30204,30209,30192,
 30202,30194,30195,30219,30221,30217,30239,30247,30240,30241,30242,30244,30260,30256,30267,30279,30280,30278,30300,
 30296,30305,30306,30312,30313,30314,30311,30316,30320,30322,30326,30328,30332,30336,30339,30344,30347,30350,30358,
 30355,30361,30362,30384,30388,30392,30393,30394,30402,30413,30422,30418,30430,30433,30437,30439,30442,34351,30459,
 30472,30471,30468,30505,30500,30494,30501,30502,30491,30519,30520,30535,30554,30568,30571,30555,30565,30591,30590,
 30585,30606,30603,30609,30624,30622,30640,30646,30649,30655,30652,30653,30651,30663,30669,30679,30682,30684,30691,
 30702,30716,30732,30738,31014,30752,31018,30789,30862,30836,30854,30844,30874,30860,30883,30901,30890,30895,30929,
 30918,30923,30932,30910,30908,30917,30922,30956,30951,30938,30973,30964,30983,30994,30993,31001,31020,31019,31040,
 31072,31063,31071,31066,31061,31059,31098,31103,31114,31133,31143,40779,31146,31150,31155,31161,31162,31177,31189,
 31207,31212,31201,31203,31240,31245,31256,31257,31264,31263,31104,31281,31291,31294,31287,31299,31319,31305,31329,
 31330,31337,40861,31344,31353,31357,31368,31383,31381,31384,31382,31401,31432,31408,31414,31429,31428,31423,36995,
 31431,31434,31437,31439,31445,31443,31449,31450,31453,31457,31458,31462,31469,31472,31490,31503,31498,31494,31539,
 31512,31513,31518,31541,31528,31542,31568,31610,31492,31565,31499,31564,31557,31605,31589,31604,31591,31600,31601,
 31596,31598,31645,31640,31647,31629,31644,31642,31627,31634,31631,31581,31641,31691,31681,31692,31695,31668,31686,
 31709,31721,31761,31764,31718,31717,31840,31744,31751,31763,31731,31735,31767,31757,31734,31779,31783,31786,31775,
 31799,31787,31805,31820,31811,31828,31823,31808,31824,31832,31839,31844,31830,31845,31852,31861,31875,31888,31908,
 31917,31906,31915,31905,31912,31923,31922,31921,31918,31929,31933,31936,31941,31938,31960,31954,31964,31970,39739,
 31983,31986,31988,31990,31994,32006,32002,32028,32021,32010,32069,32075,32046,32050,32063,32053,32070,32115,32086,
 32078,32114,32104,32110,32079,32099,32147,32137,32091,32143,32125,32155,32186,32174,32163,32181,32199,32189,32171,
 32317,32162,32175,32220,32184,32159,32176,32216,32221,32228,32222,32251,32242,32225,32261,32266,32291,32289,32274,
 32305,32287,32265,32267,32290,32326,32358,32315,32309,32313,32323,32311,32306,32314,32359,32349,32342,32350,32345,
 32346,32377,32362,32361,32380,32379,32387,32213,32381,36782,32383,32392,32393,32396,32402,32400,32403,32404,32406,
 32398,32411,32412,32568,32570,32581,32588,32589,32590,32592,32593,32597,32596,32600,32607,32608,32616,32617,32615,
 32632,32642,32646,32643,32648,32647,32652,32660,32670,32669,32666,32675,32687,32690,32697,32686,32694,32696,35697,
 32709,32710,32714,32725,32724,32737,32742,32745,32755,32761,39132,32774,32772,32779,32786,32792,32793,32796,32801,
 32808,32831,32827,32842,32838,32850,32856,32858,32863,32866,32872,32883,32882,32880,32886,32889,32893,32895,32900,
 32902,32901,32923,32915,32922,32941,20880,32940,32987,32997,32985,32989,32964,32986,32982,33033,33007,33009,33051,
 33065,33059,33071,33099,38539,33094,33086,33107,33105,33020,33137,33134,33125,33126,33140,33155,33160,33162,33152,
 33154,33184,33173,33188,33187,33119,33171,33193,33200,33205,33214,33208,33213,33216,33218,33210,33225,33229,33233,
 33241,33240,33224,33242,33247,33248,33255,33274,33275,33278,33281,33282,33285,33287,33290,33293,33296,33302,33321,
 33323,33336,33331,33344,33369,33368,33373,33370,33375,33380,33378,33384,33386,33387,33326,33393,33399,33400,33406,
 33421,33426,33451,33439,33467,33452,33505,33507,33503,33490,33524,33523,33530,33683,33539,33531,33529,33502,33542,
 33500,33545,33497,33589,33588,33558,33586,33585,33600,33593,33616,33605,33583,33579,33559,33560,33669,33690,33706,
 33695,33698,33686,33571,33678,33671,33674,33660,33717,33651,33653,33696,33673,33704,33780,33811,33771,33742,33789,
 33795,33752,33803,33729,33783,33799,33760,33778,33805,33826,33824,33725,33848,34054,33787,33901,33834,33852,34138,
 33924,33911,33899,33965,33902,33922,33897,33862,33836,33903,33913,33845,33994,33890,33977,33983,33951,34009,33997,
 33979,34010,34000,33985,33990,34006,33953,34081,34047,34036,34071,34072,34092,34079,34069,34068,34044,34112,34147,
 34136,34120,34113,34306,34123,34133,34176,34212,34184,34193,34186,34216,34157,34196,34203,34282,34183,34204,34167,
 34174,34192,34249,34234,34255,34233,34256,34261,34269,34277,34268,34297,34314,34323,34315,34302,34298,34310,34338,
 34330,34352,34367,34381,20053,34388,34399,34407,34417,34451,34467,34473,34474,34443,34444,34486,34479,34500,34502,
 34480,34505,34851,34475,34516,34526,34537,34540,34527,34523,34543,34578,34566,34568,34560,34563,34555,34577,34569,
 34573,34553,34570,34612,34623,34615,34619,34597,34601,34586,34656,34655,34680,34636,34638,34676,34647,34664,34670,
 34649,34643,34659,34666,34821,34722,34719,34690,34735,34763,34749,34752,34768,38614,34731,34756,34739,34759,34758,
 34747,34799,34802,34784,34831,34829,34814,34806,34807,34830,34770,34833,34838,34837,34850,34849,34865,34870,34873,
 34855,34875,34884,34882,34898,34905,34910,34914,34923,34945,34942,34974,34933,34941,34997,34930,34946,34967,34962,
 34990,34969,34978,34957,34980,34992,35007,34993,35011,35012,35028,35032,35033,35037,35065,35074,35068,35060,35048,
 35058,35076,35084,35082,35091,35139,35102,35109,35114,35115,35137,35140,35131,35126,35128,35148,35101,35168,35166,
 35174,35172,35181,35178,35183,35188,35191,35198,35203,35208,35210,35219,35224,35233,35241,35238,35244,35247,35250,
 35258,35261,35263,35264,35290,35292,35293,35303,35316,35320,35331,35350,35344,35340,35355,35357,35365,35382,35393,
 35419,35410,35398,35400,35452,35437,35436,35426,35461,35458,35460,35496,35489,35473,35493,35494,35482,35491,35524,
 35533,35522,35546,35563,35571,35559,35556,35569,35604,35552,35554,35575,35550,35547,35596,35591,35610,35553,35606,
 35600,35607,35616,35635,38827,35622,35627,35646,35624,35649,35660,35663,35662,35657,35670,35675,35674,35691,35679,
 35692,35695,35700,35709,35712,35724,35726,35730,35731,35734,35737,35738,35898,35905,35903,35912,35916,35918,35920,
 35925,35938,35948,35960,35962,35970,35977,35973,35978,35981,35982,35988,35964,35992,25117,36013,36010,36029,36018,
 36019,36014,36022,36040,36033,36068,36067,36058,36093,36090,36091,36100,36101,36106,36103,36111,36109,36112,40782,
 36115,36045,36116,36118,36199,36205,36209,36211,36225,36249,36290,36286,36282,36303,36314,36310,36300,36315,36299,
 36330,36331,36319,36323,36348,36360,36361,36351,36381,36382,36368,36383,36418,36405,36400,36404,36426,36423,36425,
 36428,36432,36424,36441,36452,36448,36394,36451,36437,36470,36466,36476,36481,36487,36485,36484,36491,36490,36499,
 36497,36500,36505,36522,36513,36524,36528,36550,36529,36542,36549,36552,36555,36571,36579,36604,36603,36587,36606,
 36618,36613,36629,36626,36633,36627,36636,36639,36635,36620,36646,36659,36667,36665,36677,36674,36670,36684,36681,
 36678,36686,36695,36700,36706,36707,36708,36764,36767,36771,36781,36783,36791,36826,36837,36834,36842,36847,36999,
 36852,36869,36857,36858,36881,36885,36897,36877,36894,36886,36875,36903,36918,36917,36921,36856,36943,36944,36945,
 36946,36878,36937,36926,36950,36952,36958,36968,36975,36982,38568,36978,36994,36989,36993,36992,37002,37001,37007,
 37032,37039,37041,37045,37090,37092,25160,37083,37122,37138,37145,37170,37168,37194,37206,37208,37219,37221,37225,
 37235,37234,37259,37257,37250,37282,37291,37295,37290,37301,37300,37306,37312,37313,37321,37323,37328,37334,37343,
 37345,37339,37372,37365,37366,37406,37375,37396,37420,37397,37393,37470,37463,37445,37449,37476,37448,37525,37439,
 37451,37456,37532,37526,37523,37531,37466,37583,37561,37559,37609,37647,37626,37700,37678,37657,37666,37658,37667,
 37690,37685,37691,37724,37728,37756,37742,37718,37808,37804,37805,37780,37817,37846,37847,37864,37861,37848,37827,
 37853,37840,37832,37860,37914,37908,37907,37891,37895,37904,37942,37931,37941,37921,37946,37953,37970,37956,37979,
 37984,37986,37982,37994,37417,38000,38005,38007,38013,37978,38012,38014,38017,38015,38274,38279,38282,38292,38294,
 38296,38297,38304,38312,38311,38317,38332,38331,38329,38334,38346,28662,38339,38349,38348,38357,38356,38358,38364,
 38369,38373,38370,38433,38440,38446,38447,38466,38476,38479,38475,38519,38492,38494,38493,38495,38502,38514,38508,
 38541,38552,38549,38551,38570,38567,38577,38578,38576,38580,38582,38584,38585,38606,38603,38601,38605,35149,38620,
 38669,38613,38649,38660,38662,38664,38675,38670,38673,38671,38678,38681,38692,38698,38704,38713,38717,38718,38724,
 38726,38728,38722,38729,38748,38752,38756,38758,38760,21202,38763,38769,38777,38789,38780,38785,38778,38790,38795,
 38799,38800,38812,38824,38822,38819,38835,38836,38851,38854,38856,38859,38876,38893,40783,38898,31455,38902,38901,
 38927,38924,38968,38948,38945,38967,38973,38982,38991,38987,39019,39023,39024,39025,39028,39027,39082,39087,39089,
 39094,39108,39107,39110,39145,39147,39171,39177,39186,39188,39192,39201,39197,39198,39204,39200,39212,39214,39229,
 39230,39234,39241,39237,39248,39243,39249,39250,39244,39253,39319,39320,39333,39341,39342,39356,39391,39387,39389,
 39384,39377,39405,39406,39409,39410,39419,39416,39425,39439,39429,39394,39449,39467,39479,39493,39490,39488,39491,
 39486,39509,39501,39515,39511,39519,39522,39525,39524,39529,39531,39530,39597,39600,39612,39616,39631,39633,39635,
 39636,39646,39647,39650,39651,39654,39663,39659,39662,39668,39665,39671,39675,39686,39704,39706,39711,39714,39715,
 39717,39719,39720,39721,39722,39726,39727,39730,39748,39747,39759,39757,39758,39761,39768,39796,39827,39811,39825,
 39830,39831,39839,39840,39848,39860,39872,39882,39865,39878,39887,39889,39890,39907,39906,39908,39892,39905,39994,
 39922,39921,39920,39957,39956,39945,39955,39948,39942,39944,39954,39946,39940,39982,39963,39973,39972,39969,39984,
 40007,39986,40006,39998,40026,40032,40039,40054,40056,40167,40172,40176,40201,40200,40171,40195,40198,40234,40230,
 40367,40227,40223,40260,40213,40210,40257,40255,40254,40262,40264,40285,40286,40292,40273,40272,40281,40306,40329,
 40327,40363,40303,40314,40346,40356,40361,40370,40388,40385,40379,40376,40378,40390,40399,40386,40409,40403,40440,
 40422,40429,40431,40445,40474,40475,40478,40565,40569,40573,40577,40584,40587,40588,40594,40597,40593,40605,40613,
 40617,40632,40618,40621,38753,40652,40654,40655,40656,40660,40668,40670,40669,40672,40677,40680,40687,40692,40694,
 40695,40697,40699,40700,40701,40711,40712,30391,40725,40737,40748,40766,40778,40786,40788,40803,40799,40800,40801,
 40806,40807,40812,40810,40823,40818,40822,40853,40860,40864,22575,27079,36953,29796,20956,29081,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32394,35100,37704,
 37512,34012,20425,28859,26161,26824,37625,26363,24389,20008,20193,20220,20224,20227,20281,20310,20370,20362,20378,
 20372,20429,20544,20514,20479,20510,20550,20592,20546,20628,20724,20696,20810,20836,20893,20926,20972,21013,21148,
 21158,21184,21211,21248,21255,21284,21362,21395,21426,21469,64014,21660,21642,21673,21759,21894,22361,22373,22444,
 22472,22471,64015,64016,22686,22706,22795,22867,22875,22877,22883,22948,22970,23382,23488,29999,23512,23532,23582,
 23718,23738,23797,23847,23891,64017,23874,23917,23992,23993,24016,24353,24372,24423,24503,24542,24669,24709,24714,
 24798,24789,24864,24818,24849,24887,24880,24984,25107,25254,25589,25696,25757,25806,25934,26112,26133,26171,26121,
 26158,26142,26148,26213,26199,26201,64018,26227,26265,26272,26290,26303,26362,26382,63785,26470,26555,26706,26560,
 26625,26692,26831,64019,26984,64020,27032,27106,27184,27243,27206,27251,27262,27362,27364,27606,27711,27740,27782,
 27759,27866,27908,28039,28015,28054,28076,28111,28152,28146,28156,28217,28252,28199,28220,28351,28552,28597,28661,
 28677,28679,28712,28805,28843,28943,28932,29020,28998,28999,64021,29121,29182,29361,29374,29476,64022,29559,29629,
 29641,29654,29667,29650,29703,29685,29734,29738,29737,29742,29794,29833,29855,29953,30063,30338,30364,30366,30363,
 30374,64023,30534,21167,30753,30798,30820,30842,31024,64024,64025,64026,31124,64027,31131,31441,31463,64028,31467,
 31646,64029,32072,32092,32183,32160,32214,32338,32583,32673,64030,33537,33634,33663,33735,33782,33864,33972,34131,
 34137,34155,64031,34224,64032,64033,34823,35061,35346,35383,35449,35495,35518,35551,64034,35574,35667,35711,36080,
 36084,36114,36214,64035,36559,64036,64037,36967,37086,64038,37141,37159,37338,37335,37342,37357,37358,37348,37349,
 37382,37392,37386,37434,37440,37436,37454,37465,37457,37433,37479,37543,37495,37496,37607,37591,37593,37584,64039,
 37589,37600,37587,37669,37665,37627,64040,37662,37631,37661,37634,37744,37719,37796,37830,37854,37880,37937,37957,
 37960,38290,63964,64041,38557,38575,38707,38715,38723,38733,38735,38737,38741,38999,39013,64042,64043,39207,64044,
 39326,39502,39641,39644,39797,39794,39823,39857,39867,39936,40304,40299,64045,40473,40657,0,0,8560,8561,8562,8563,
 8564,8565,8566,8567,8568,8569,65506,65508,65287,65282,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8560,8561,8562,8563,8564,8565,8566,
 8567,8568,8569,8544,8545,8546,8547,8548,8549,8550,8551,8552,8553,65506,65508,65287,65282,12849,8470,8481,8757,32394,
 35100,37704,37512,34012,20425,28859,26161,26824,37625,26363,24389,20008,20193,20220,20224,20227,20281,20310,20370,
 20362,20378,20372,20429,20544,20514,20479,20510,20550,20592,20546,20628,20724,20696,20810,20836,20893,20926,20972,
 21013,21148,21158,21184,21211,21248,21255,21284,21362,21395,21426,21469,64014,21660,21642,21673,21759,21894,22361,
 22373,22444,22472,22471,64015,64016,22686,22706,22795,22867,22875,22877,22883,22948,22970,23382,23488,29999,23512,
 23532,23582,23718,23738,23797,23847,23891,64017,23874,23917,23992,23993,24016,24353,24372,24423,24503,24542,24669,
 24709,24714,24798,24789,24864,24818,24849,24887,24880,24984,25107,25254,25589,25696,25757,25806,25934,26112,26133,
 26171,26121,26158,26142,26148,26213,26199,26201,64018,26227,26265,26272,26290,26303,26362,26382,63785,26470,26555,
 26706,26560,26625,26692,26831,64019,26984,64020,27032,27106,27184,27243,27206,27251,27262,27362,27364,27606,27711,
 27740,27782,27759,27866,27908,28039,28015,28054,28076,28111,28152,28146,28156,28217,28252,28199,28220,28351,28552,
 28597,28661,28677,28679,28712,28805,28843,28943,28932,29020,28998,28999,64021,29121,29182,29361,29374,29476,64022,
 29559,29629,29641,29654,29667,29650,29703,29685,29734,29738,29737,29742,29794,29833,29855,29953,30063,30338,30364,
 30366,30363,30374,64023,30534,21167,30753,30798,30820,30842,31024,64024,64025,64026,31124,64027,31131,31441,31463,
 64028,31467,31646,64029,32072,32092,32183,32160,32214,32338,32583,32673,64030,33537,33634,33663,33735,33782,33864,
 33972,34131,34137,34155,64031,34224,64032,64033,34823,35061,35346,35383,35449,35495,35518,35551,64034,35574,35667,
 35711,36080,36084,36114,36214,64035,36559,64036,64037,36967,37086,64038,37141,37159,37338,37335,37342,37357,37358,
 37348,37349,37382,37392,37386,37434,37440,37436,37454,37465,37457,37433,37479,37543,37495,37496,37607,37591,37593,
 37584,64039,37589,37600,37587,37669,37665,37627,64040,37662,37631,37661,37634,37744,37719,37796,37830,37854,37880,
 37937,37957,37960,38290,63964,64041,38557,38575,38707,38715,38723,38733,38735,38737,38741,38999,39013,64042,64043,
 39207,64044,39326,39502,39641,39644,39797,39794,39823,39857,39867,39936,40304,40299,64045,40473,40657,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#endif
};

static const uint16_t alice_sjis_by_unicode[ALICE_SJIS_REVERSE] = {
#ifndef ALICE_TEXT_SJIS_BASIC
 33176,33102,33163,33149,33100,33271,33150,33152,33695,33696,33697,33698,33699,33700,33701,33702,33703,33704,33705,
 33706,33707,33708,33709,33710,33711,33712,33713,33714,33715,33716,33717,33718,33727,33728,33729,33730,33731,33732,
 33733,33734,33735,33736,33737,33738,33739,33740,33741,33742,33743,33744,33745,33746,33747,33748,33749,33750,33862,
 33856,33857,33858,33859,33860,33861,33863,33864,33865,33866,33867,33868,33869,33870,33871,33872,33873,33874,33875,
 33876,33877,33878,33879,33880,33881,33882,33883,33884,33885,33886,33887,33888,33904,33905,33906,33907,33908,33909,
 33911,33912,33913,33914,33915,33916,33917,33918,33920,33921,33922,33923,33924,33925,33926,33927,33928,33929,33930,
 33931,33932,33933,33934,33935,33936,33937,33910,33117,33116,33125,33126,33127,33128,33269,33270,33124,33123,33265,
 33164,33165,33190,33166,34690,34692,33264,34644,34645,34646,34647,34648,34649,34650,34651,34652,34653,61167,61168,
 61169,61170,61171,61172,61173,61174,61175,61176,33193,33194,33192,33195,33227,33228,33229,33245,33230,33246,33208,
 33209,34708,33251,33253,33159,34712,33242,33121,33224,33225,33215,33214,33255,33256,34707,33160,33254,33252,33248,
 33154,33247,33157,33158,33249,33250,33212,33213,33210,33211,33243,34713,33244,34624,34625,34626,34627,34628,34629,
 34630,34631,34632,34633,34634,34635,34636,34637,34638,34639,34640,34641,34642,34643,33951,33962,33952,33963,33953,
 33964,33954,33965,33956,33967,33955,33966,33957,33978,33973,33968,33959,33980,33975,33970,33958,33974,33979,33969,
 33960,33976,33981,33971,33961,33977,33982,33972,33185,33184,33187,33186,33189,33188,33183,33182,33179,33181,33180,
 33276,33178,33177,33162,33161,33268,33267,33266,33088,33089,33090,33110,33112,33113,33114,33137,33138,33139,33140,
 33141,33142,33143,33144,33145,33146,33191,33196,33131,33132,34688,34689,33439,33440,33441,33442,33443,33444,33445,
 33446,33447,33448,33449,33450,33451,33452,33453,33454,33455,33456,33457,33458,33459,33460,33461,33462,33463,33464,
 33465,33466,33467,33468,33469,33470,33471,33472,33473,33474,33475,33476,33477,33478,33479,33480,33481,33482,33483,
 33484,33485,33486,33487,33488,33489,33490,33491,33492,33493,33494,33495,33496,33497,33498,33499,33500,33501,33502,
 33503,33504,33505,33506,33507,33508,33509,33510,33511,33512,33513,33514,33515,33516,33517,33518,33519,33520,33521,
 33098,33099,33108,33109,33600,33601,33602,33603,33604,33605,33606,33607,33608,33609,33610,33611,33612,33613,33614,
 33615,33616,33617,33618,33619,33620,33621,33622,33623,33624,33625,33626,33627,33628,33629,33630,33631,33632,33633,
 33634,33635,33636,33637,33638,33639,33640,33641,33642,33643,33644,33645,33646,33647,33648,33649,33650,33651,33652,
 33653,33654,33655,33656,33657,33658,33659,33660,33661,33662,33664,33665,33666,33667,33668,33669,33670,33671,33672,
 33673,33674,33675,33676,33677,33678,33679,33680,33681,33682,33683,33684,33685,33686,33093,33115,33106,33107,34698,
 34699,34700,34693,34694,34695,34696,34697,34661,34665,34656,34659,34657,34667,34666,34660,34668,34662,34670,34655,
 34669,34658,34663,34664,34686,34703,34702,34701,34674,34675,34671,34672,34673,34677,34676,34691,35050,37530,36533,
 38556,36836,36431,36835,35258,38259,38750,39072,35150,35470,39073,37026,39360,35701,38328,36837,38844,38336,60748,
 39074,37510,39075,35832,39076,35547,37455,36581,39077,39078,39079,37972,35702,37974,37857,36033,38482,58728,39080,
 36838,39081,35251,35811,36078,38631,39844,38800,37883,35491,35668,39082,39083,38841,38748,37256,39085,36502,37873,
 39088,35165,36061,36060,35044,39018,39017,36273,34975,39089,39090,39091,38483,39092,36080,35045,38546,35740,35741,
 35742,37600,38842,39093,39094,39095,36972,36697,36973,39100,39098,39099,35703,36257,35310,39097,39096,38311,36453,
 36452,37308,39101,38260,37093,33111,39102,39104,60749,37347,38879,35016,39103,35260,35778,37511,35983,39105,37955,
 60750,60751,35561,60752,39106,35017,36062,35562,38298,38064,35704,35311,39141,37728,38028,39108,38074,38880,36940,
 60753,36454,36503,35262,37583,37441,39112,35018,37601,36698,36274,38723,37324,35261,60754,39111,38749,39107,39109,
 36332,39110,39747,39118,39121,39119,35264,38329,39113,39117,36081,36455,35492,39122,39114,60756,38881,36504,39115,
 39120,60755,60758,39123,39116,60757,35743,35019,35744,35263,39748,38553,38286,36082,36942,38837,38358,35927,37283,
 35298,60741,36722,60759,39127,39132,39130,39125,37293,39128,39131,39129,38363,39126,36941,38547,39133,39134,36675,
 39147,37999,38229,39142,38382,35252,39146,60762,39140,39149,37233,36034,38011,57541,39148,37756,39137,36084,36083,
 39135,60763,36568,39143,60761,38381,37484,39139,35985,39136,39144,39138,38863,39145,39008,35812,35984,60760,60766,
 39150,60764,39151,39155,35020,38350,39154,39153,39157,39156,37602,35986,39158,60765,36547,37284,37603,35828,39159,
 35669,39160,39162,38484,35974,60767,36432,38133,39161,36291,38754,39164,39234,39163,36290,36765,35928,39235,35789,
 39232,39233,37805,37276,35745,38508,39236,60769,38843,39237,39240,39238,37229,39239,39241,60768,39243,39242,38342,
 35670,39245,39246,35245,39244,36594,39249,39248,39247,39124,39250,36766,39251,38724,38615,39253,39252,39255,39254,
 39256,39257,35058,36019,35930,36699,37531,35746,37094,36085,60770,36238,39259,38598,37733,36505,39258,39260,37757,
 35477,39261,60771,37884,37203,39263,39264,38058,36086,39002,39265,35748,38330,37300,35823,37716,35987,39266,39267,
 37856,35198,39270,36347,39269,36292,39271,58348,39272,38496,39273,39274,39275,36839,36554,60772,35493,39278,39276,
 38587,39277,38265,39279,39280,39281,37758,39285,39283,39284,39282,36321,39286,38632,38882,39287,60773,37030,39288,
 36729,39289,37532,38845,37760,39363,39290,60067,35779,39291,38525,36744,37370,39293,37858,60774,39294,39296,35405,
 39297,35749,37834,35226,36719,38047,39298,37761,36974,39299,38314,37080,35488,35495,39300,39302,35929,39301,60775,
 38897,36745,38075,38346,39303,38808,39304,39305,37790,39306,37031,36348,35988,39307,36456,36239,37604,39309,37285,
 36333,39310,39311,37199,39308,39313,38485,36228,39312,35989,36316,38029,39316,39314,38299,36840,39323,35460,39317,
 39315,37230,39319,39318,35427,35968,39324,38827,39320,39325,39322,39321,38861,60776,36087,35265,38898,60777,36757,
 37751,36229,39328,39329,61019,38883,38986,39331,36088,39330,35406,60778,39332,38517,37562,38725,38359,39333,59603,
 37806,39334,35496,38577,60779,36767,39335,38373,39339,37032,39336,35790,39337,35497,35917,39340,39341,39342,39343,
 36569,36089,38620,60780,38630,37877,38383,39344,60781,39345,39347,39349,39348,39350,35259,38507,36346,39351,37240,
 36768,35751,39352,60782,38105,39353,39354,39355,39356,38211,35814,35043,37821,39357,36700,37095,39359,39358,36769,
 36063,39361,38076,39362,38106,37298,37356,35750,37868,37456,38030,38509,39364,37096,35924,39365,39366,35147,35059,
 35563,60783,37286,35696,38801,39369,35253,39368,35752,39370,38639,60784,39371,38864,36090,36020,39372,39374,39373,
 36990,35160,35197,39375,39376,60785,36021,39377,35726,36433,39378,38548,36275,35705,38726,37231,38077,36603,36710,
 36582,36595,36758,38078,60786,39381,35170,37232,36091,36035,35813,39385,37440,37372,35753,36770,39386,39384,35266,
 37348,36534,36458,35141,35472,36230,36457,39387,39388,35688,35429,36231,35687,37597,35140,37807,38588,36160,38809,
 37734,36092,35918,39397,35809,38505,38107,39396,35548,39391,39392,39394,39395,35706,36993,38315,39393,39389,36065,
 39390,38979,38384,37606,36064,36240,39398,37851,39402,36604,36596,39405,39403,38561,39400,39409,39404,39407,36036,
 38589,39408,39410,39412,60789,36334,39009,39401,39399,39411,39406,60788,39414,39490,39416,39420,60790,39488,39417,
 39517,36327,35408,39415,39492,35060,39491,34979,38249,39489,39418,39413,39419,36294,39493,35061,39502,39494,39495,
 36771,38537,39500,39499,37710,39501,39498,60791,35155,36276,36943,39496,37762,39497,34976,39507,38722,36773,39513,
 39512,39503,37313,39504,37357,39509,36772,39506,38626,35931,39510,39511,39508,39514,39505,39520,39525,39521,39516,
 39526,37200,60792,39528,36161,39518,37533,39522,39515,35499,35564,35461,39523,39519,35990,39529,39527,37234,35689,
 35754,39524,35826,35171,39533,39531,39589,39536,39530,39534,39532,36459,39535,39538,39543,39541,39540,37457,35267,
 39537,39539,36774,35154,39542,35292,39554,36858,39549,39547,39548,39550,35164,37208,39544,39545,35482,39553,35565,
 39556,39552,39555,38316,37843,38070,39558,39557,35428,39559,39562,39561,39560,37976,39563,39564,39566,39565,39568,
 39571,39569,39567,39570,39572,39573,39574,39575,39576,39268,36602,36460,35313,35062,37475,39577,36258,35021,36989,
 39578,36037,36241,39580,39579,38366,39581,39583,39582,39584,39585,35991,35200,39586,39588,39587,39590,37753,39591,
 34995,36317,35932,37486,39592,39593,39595,39596,36322,35791,38486,39594,39597,36287,36162,60793,39601,36259,60794,
 37458,39598,37592,39602,36994,39600,39603,35934,39604,39605,36163,35423,39607,39608,60795,39609,39606,39599,39610,
 39611,60797,60796,38532,36841,39613,39614,39612,39616,37975,35046,38261,39617,36859,36535,38012,35566,36329,38520,
 37808,35992,37325,39615,39618,37314,39619,39620,39622,37607,35500,60063,35201,38385,36842,37735,36324,39628,38331,
 38875,35314,39624,37209,39627,37763,37736,37764,38071,37579,36295,39623,35222,37717,39625,39621,36975,39629,36717,
 35755,39630,38374,37277,37572,60801,39632,38510,39633,39638,60802,38317,39637,39631,39634,39636,36260,38343,39639,
 37476,35315,36843,39641,39640,36232,39642,39644,39643,39646,39635,39648,39647,39645,36461,36976,37235,39649,37050,
 35051,38020,37593,39651,39650,39652,39653,39654,39655,38351,39656,60803,35268,39657,38747,35407,39367,36711,37309,
 39658,38633,38578,39660,37349,37718,37310,38262,39661,39662,35227,36536,39663,35022,39664,39665,35202,35567,37854,
 38386,39669,37236,39668,35935,60804,38522,39667,37765,39671,39670,60805,60806,39673,39672,60807,35228,39674,36775,
 39676,37444,39675,38321,36759,37754,39744,36164,39745,37952,38108,38607,37956,39754,35671,38756,38573,39850,39746,
 39749,60808,37315,38487,37737,39750,38533,60809,36296,36776,39751,36463,36462,34999,36038,37033,35023,39755,39756,
 39753,35159,35501,39752,38595,38224,34982,35063,36464,35024,34977,39761,39759,38586,39762,39760,39758,36944,39757,
 38360,36066,39766,39767,36777,39763,38987,37995,39765,36261,39768,38263,39769,39764,38585,38013,39770,38225,39771,
 39775,39772,35269,39774,36537,39773,35993,39787,39780,39777,37508,39776,39778,39779,39781,39782,35568,39784,39783,
 39785,36844,39788,37594,35172,39786,39789,39790,39793,39791,39792,36465,39794,36165,39795,60810,36506,37302,39796,
 39797,36473,36166,38608,35655,36039,39798,35447,39799,37303,39800,39841,39801,39802,39803,39805,39806,39808,37358,
 35142,36583,35008,37238,35502,36531,36167,37766,36672,35503,37512,37608,34998,35672,38387,36544,35697,37097,36538,
 38727,39809,35707,36297,35409,35203,36778,35270,39810,38757,36712,60811,36578,39811,35569,37840,38567,39812,39813,
 38264,39815,35494,35829,39814,60813,35504,36945,39819,36416,35271,39818,39816,39820,39817,37962,40651,36946,39821,
 60814,38846,39822,39824,37534,39823,37025,36507,37326,36597,38293,37098,36555,39825,36779,39826,39827,35025,37304,
 36977,39828,37809,36780,36781,39829,37099,36782,60815,39830,39831,38622,39832,35780,36673,39833,39834,36570,36939,
 37874,36979,38134,37953,35783,39835,35727,39836,35836,37837,35246,36466,39837,39840,39839,35835,39838,37719,37294,
 37738,36550,37239,38810,39842,39843,37844,36434,39845,39846,39847,35570,39848,39849,35242,60816,37210,35554,39851,
 38566,37328,35448,39853,39855,35549,60817,39852,39854,39857,39856,39858,39859,37819,35756,35299,39860,39865,39863,
 38389,38388,60818,37767,39862,36723,39861,37010,39866,36328,39872,39873,39867,35410,39868,39877,39876,39875,39871,
 39870,39874,60819,38390,60822,39881,39878,39880,38802,39879,60820,39869,37011,39882,60823,36277,39883,39884,39887,
 39886,39885,37768,39864,39893,39889,39888,39890,39891,39894,60824,60825,38884,39895,39892,39896,35550,39897,60826,
 39899,39898,39900,39901,37100,36674,36740,37251,36168,36278,36169,35728,39902,36279,36040,39903,38564,37986,39904,
 36170,35498,37446,35792,36467,38266,38079,39905,35571,39908,37535,39907,39906,39909,37609,36995,36468,37064,37329,
 35649,37536,39910,39911,36845,38488,39914,39913,39912,38301,39921,38521,39915,39917,38539,39916,39918,38054,39919,
 38332,39920,35505,38333,37966,39922,39923,36171,35506,39924,36022,38755,38728,35572,39926,37537,36172,36783,38109,
 36784,36760,37610,38391,37720,36173,38267,39927,37752,36288,36041,37611,35009,36750,36174,38758,39928,39929,38000,
 39930,38901,38988,39932,39931,35430,40000,40003,40004,40002,38239,36785,40006,40005,40001,40007,40008,40009,40012,
 40010,40011,40013,35204,37612,40014,35994,35316,37973,40015,37881,38361,40016,38989,40017,38334,40020,39071,39087,
 36526,37875,40021,35708,37538,35064,40022,38308,36175,37487,37613,60827,38637,36023,36042,40023,40024,40030,36579,
 60828,37539,35757,40025,38218,37477,40026,60747,40027,35758,40028,40029,40031,37782,40032,40033,40034,40019,40018,
 40035,35936,38214,60829,36298,38230,37540,38250,40036,36786,35173,40037,40038,38640,38110,40041,35229,37034,40040,
 40039,35937,37330,40045,40043,40042,38821,36067,36761,40044,37739,36701,37822,40048,40047,40046,40049,36068,40050,
 38300,36730,40051,38135,37823,37541,60830,37711,40052,35658,36947,38219,35573,37957,40053,36469,38489,38490,35230,
 40058,60831,37513,40055,35317,40107,40057,37967,40056,40054,36250,40060,40067,40073,40065,37755,40070,38268,40064,
 40069,38885,36470,37331,40061,35709,40072,37035,35205,40066,35318,40071,35759,40068,40074,40076,40086,40084,40081,
 40080,38902,40082,35760,36176,36762,40089,40075,60832,40079,40062,35320,40083,40085,37488,36262,35254,40077,40088,
 40087,35761,37287,35462,35938,40078,40090,40093,40095,60833,36539,60834,40101,37614,40091,40099,35319,40097,40098,
 40094,40096,36069,38729,35507,35192,40100,37977,34987,38111,40059,40106,40110,38627,40103,37769,40108,36846,40109,
 37845,39014,40105,60836,40111,36251,37065,60835,35026,40104,40102,37241,40092,36435,37316,40123,60838,37242,40118,
 40115,40116,36580,40119,40122,40117,36676,40120,40114,38650,38649,40124,40125,35027,60839,40113,35824,34980,35508,
 60837,40121,40129,40128,40133,60841,40134,60840,40132,40135,40127,40131,40136,40137,40126,36508,40130,37332,36177,
 40112,36948,40150,38375,40140,40141,40142,40149,40148,38557,35509,40146,35940,35411,40143,38838,40145,35028,40147,
 40138,40144,40151,35939,40139,38780,38730,40154,40158,37278,38903,40159,40156,40153,60842,40152,40157,38318,37810,
 35941,40160,40155,40161,35995,35247,40169,35510,40167,40168,36263,40166,40164,40163,40170,40162,40172,35321,40174,
 40173,37542,40177,40175,40165,35996,40176,40180,40179,40181,40178,40182,40183,40184,38376,40186,40185,36702,37036,
 35300,35322,60843,40187,35005,37066,40188,59073,40256,35969,40257,37101,40258,40259,35673,40260,40261,40262,37333,
 36043,38623,38491,36746,40263,37102,59323,38112,36584,36299,40264,37317,38309,37359,40267,40265,40268,40266,40269,
 38319,34997,38269,38113,40270,40273,36787,35674,40271,40278,36788,40272,37987,38781,40274,40275,40279,37770,40276,
 36178,37084,40293,38066,37360,60844,38114,40363,38392,37615,38549,40282,35231,37514,40291,37459,40285,40292,40287,
 40294,40290,40289,38031,40283,35323,40281,35729,37361,40277,40280,36179,37081,36789,40288,38001,35730,35431,35463,
 36928,40296,40301,40297,35997,40302,36417,36233,36677,40284,36509,40299,36471,40300,35010,40295,37543,35731,35762,
 40298,34981,36289,36949,37616,38098,40304,37245,37288,36426,40305,40307,40303,38367,37563,37243,38393,36556,40320,
 40318,37016,35998,40312,36791,37862,37968,40310,37244,36598,40315,36790,40309,40314,38002,40308,35904,35452,40316,
 38825,36300,37460,40313,37082,36180,36996,35206,37211,40311,35684,35942,37581,40317,37246,40321,40323,37301,40329,
 40324,40326,38240,37617,40327,38731,38759,35511,34988,40325,40322,35574,35207,60845,40328,38760,40332,37305,40339,
 40333,40330,40337,40306,40334,40338,38080,37771,40331,40335,35943,36335,37083,40343,37701,60846,40340,38528,40341,
 40342,38604,37024,35970,40349,36436,40346,40345,37969,60847,37811,37712,40347,40348,38287,37988,36418,37103,38511,
 35432,40355,40350,38761,40357,40353,40354,37248,60848,40352,40286,40356,40351,40361,40362,37702,40364,36419,40359,
 35675,40365,40358,40369,40368,40367,40370,40372,36847,40371,40375,40373,40374,40336,40377,40376,40344,40378,40366,
 36472,40379,40380,40382,40381,40383,35324,36181,38394,37037,36044,40385,40388,60849,38257,35710,40387,40386,38003,
 40389,35763,40391,40390,35512,36437,37846,35944,37012,40392,37038,37703,38270,40393,40394,40395,38326,39804,37060,
 38251,36310,38115,38081,37740,38847,40397,36558,40398,34996,35794,37067,38272,40399,36449,37478,36474,36950,40400,
 38395,35223,36475,40403,40401,40404,38839,40402,37113,40405,37296,40406,35576,40408,40407,40409,40410,35577,37882,
 37461,35724,36476,37249,36731,34990,40411,35232,40415,60850,36182,40414,36265,36792,60853,40413,36793,38590,36264,
 35029,37068,60851,40420,60855,37039,35174,60856,36724,38534,36336,36794,60854,37029,60743,40419,40417,40418,60852,
 37515,40517,40424,36510,36183,40422,40423,36951,40421,36430,60858,60859,40426,40425,40430,40431,40427,60857,35393,
 40428,40429,38099,38273,35945,40432,60861,37040,36795,37489,35781,40433,40437,35273,40434,40436,40435,36747,37479,
 35011,40438,60862,40439,60863,37544,38895,36450,38377,60864,38492,40513,40441,40444,40443,60865,40440,40512,37852,
 40442,40514,36748,40515,38762,38040,40516,40518,40519,40520,35784,35175,36184,40521,40522,36753,37250,60866,60746,
 39382,37213,37212,37334,36293,39152,35982,38732,38396,38302,60867,40523,36337,37565,40524,38990,38493,37545,40525,
 35578,40526,40527,38616,38562,38550,38523,36420,40529,36585,38512,40531,40534,40533,35575,35712,40530,40532,40535,
 37017,38811,35015,36318,37306,36571,36849,40538,37741,40536,37289,40537,36848,38619,40539,40540,38792,60869,40545,
 36185,38004,40542,37772,40412,40416,35694,37990,40544,36796,38082,40550,38136,40541,40547,40546,37069,38541,38865,
 38535,35274,36477,39015,40549,37013,40548,40543,36045,40555,40553,35275,40551,40557,40563,60870,60872,37318,38335,
 40565,38209,40564,38032,38494,35513,37109,36703,37585,38733,40560,40559,40561,40558,40566,40556,40554,40562,40552,
 37516,38646,36548,36338,36280,38543,35424,60873,37580,37832,35176,37104,37042,35913,40568,36186,35484,40570,35476,
 40577,40573,37105,35434,36266,35433,36301,40571,35973,35946,37773,60874,40569,35012,40572,40574,35787,35915,60871,
 35514,35690,40578,36343,38545,36438,40579,38223,40591,35249,40580,40597,40581,38848,40588,38014,40596,40583,34994,
 40585,36187,40587,40586,40582,40593,36797,39659,36070,38812,40584,37618,35394,36267,40576,40592,35457,40590,40594,
 37774,35580,40624,60744,38599,40599,35579,40606,60875,38495,40607,40609,40613,40601,37449,37775,40617,40604,40614,
 40608,36952,40618,37041,40616,35515,39023,40598,40612,35030,40600,38584,40605,36929,37573,40595,40611,37018,40621,
 35473,35999,40623,40602,40622,40615,40603,40619,40620,40637,37836,40610,40633,40635,37590,38763,38294,40630,37320,
 40636,37214,40627,40640,40639,37869,40638,37864,60877,40642,40629,35782,40632,36732,38016,40634,35785,40626,40628,
 40625,38991,35449,40631,40641,35412,36325,35196,40658,38992,40661,60879,36953,40660,40659,40656,40644,40673,40643,
 40662,40654,40649,40646,40647,40655,60064,40652,36188,37574,37252,40650,40645,40648,38764,38538,40653,40663,60880,
 40671,40664,40677,40675,40670,40669,37582,37253,40667,40665,40672,40678,38131,40684,40679,40682,40676,37524,38231,
 40666,40674,36798,38605,40694,40681,36000,35233,35454,40657,60881,36799,40686,40693,36599,35474,37453,40683,60883,
 40688,40692,35764,35691,40690,35648,37833,40689,40691,60882,40685,60884,40687,60885,35456,37480,40698,40696,36071,
 40695,40768,40567,40697,40699,40700,40779,40775,40589,40774,40773,40770,40680,40772,40771,40777,38981,40780,35833,
 40776,40778,60886,60887,38053,40781,40785,40782,38803,40783,40668,40786,40787,35156,40789,35975,36511,35795,35234,
 38782,40791,40790,40793,35676,35796,35516,40796,40795,40797,35276,37462,40798,35517,40800,40799,40801,40802,40803,
 36478,37043,36255,38288,38368,39011,36501,36302,38896,40804,40805,36480,40806,40807,40809,40808,38519,36733,36586,
 36451,40810,40812,36930,40811,40813,40814,40815,40816,40817,40819,40818,40820,35235,37481,40821,36421,35435,40822,
 37729,39626,35650,40823,40824,38378,38536,37829,40825,38116,60888,38137,38609,40826,40828,40827,40830,40829,40833,
 36481,38575,40834,40835,35651,40836,40838,40837,36997,38232,35177,38083,60889,37619,36704,35713,38084,36524,40840,
 35518,35224,60890,37872,40839,36189,37490,40841,40849,40842,60892,37311,35714,40850,35976,35652,40848,40846,40843,
 38784,60891,37566,37847,40844,40852,40851,35906,35243,36281,40845,40847,38518,37362,38551,40860,40861,35277,38310,
 38651,40863,36513,36800,40856,40862,35208,35765,40853,40858,37106,38033,38117,40855,38464,40857,40866,60893,40864,
 40859,38465,37991,35715,37700,37517,40867,40865,37335,40854,35178,60894,38765,40878,40877,37108,40874,38796,37812,
 40868,37571,35179,36190,40871,36678,40876,40875,40870,40873,35464,40872,37992,38828,36850,37107,40884,40882,38252,
 40879,40881,35161,36191,38993,35420,38274,60896,38785,35395,36954,40883,40888,60895,36801,38735,40885,40880,40886,
 60897,38876,37779,37824,60898,35413,35188,40892,40895,38849,38788,40902,40896,40893,38866,40899,60899,36713,40901,
 40906,37777,40904,40898,37463,40905,40894,40900,40907,35066,40897,40908,36955,60901,36734,38307,36268,60900,40889,
 40903,37721,60902,37044,35465,36303,36802,40891,36705,35947,40890,40912,36749,36024,40927,40921,35732,37742,40916,
 40925,34989,35153,60905,35255,40918,37290,40909,40911,36192,40928,60903,40923,60906,40915,40922,38569,40920,40924,
 36046,36803,37464,60904,40914,38734,40917,40910,37778,40913,40919,39024,36540,38558,40929,38060,40941,36025,36736,
 40931,38829,36193,40944,35052,40942,40930,40936,40938,38766,40933,37709,40935,60907,40943,40937,38597,40932,36512,
 40956,35466,40934,40939,40940,37354,37336,40948,40954,40952,37704,57410,40949,40950,40926,35737,38233,36541,36247,
 38994,40946,57409,35209,37254,38041,35519,38904,38559,37584,40953,40955,37201,57408,40951,40945,35521,35977,57422,
 57417,37110,35459,36737,57426,57419,37546,57416,37591,57451,57413,57412,57421,57415,57414,57420,37023,57411,60908,
 57423,57424,35520,57429,57428,57430,57433,37730,57427,60909,57431,35971,37367,57425,37978,57432,57437,57435,57438,
 57441,57434,36234,37959,40887,38804,57436,57440,37363,57439,57418,60910,59529,57444,57448,57446,60911,60912,57442,
 57443,57447,57445,38253,57453,57450,57449,57452,37842,57454,37525,37355,60913,37027,57455,57457,57456,40947,57458,
 37861,57459,35278,37780,35396,35716,36572,36304,60914,38982,36998,35210,57461,57460,60915,57464,37465,57467,57462,
 57466,57465,37727,35031,60742,38899,57469,35143,57472,57470,57468,57463,38466,57474,60917,57473,60916,35211,57476,
 38320,57475,38579,36805,37202,36804,60919,60920,38905,57482,37111,57478,57483,35212,60918,57481,38017,57477,57480,
 36806,38095,57484,36559,37112,57487,57479,35910,57485,38767,57488,60068,36718,57489,57490,37965,57492,57493,60922,
 37970,37781,57495,57497,38867,57494,57496,35213,57491,39546,57498,37255,36439,57500,57499,36931,39383,57501,57503,
 57486,57502,60923,57504,38042,57505,57506,57507,57508,37596,57510,57509,57511,57512,36573,38275,38634,57513,57514,
 37237,36514,57515,57516,57517,38352,38085,57518,38006,37547,57519,35301,35725,38596,38580,35250,38995,38513,38312,
 37045,57520,37825,36001,57521,36306,57523,57522,57524,57525,57526,35677,57527,57528,36002,38086,60924,57530,36851,
 57529,60992,35766,57531,57533,57532,57534,36047,57535,35815,37215,36253,57537,57538,57536,36587,37830,35767,57540,
 37451,57539,38996,38018,57543,57545,57542,38610,57544,57546,38850,60993,57550,57549,37526,37964,36003,57548,57547,
 38736,38737,57551,35214,36246,36482,57552,57553,57555,36706,57557,57556,57558,35436,57560,60995,57559,57562,57561,
 36026,38822,35786,35236,35816,35551,38886,57564,57566,60996,57567,35279,57563,60997,36440,37567,57565,61000,60998,
 57570,36588,60999,57568,35933,38087,57569,57596,61002,57575,36027,35717,57572,38813,61001,38830,37364,57574,61003,
 61005,61004,61006,57576,38868,35797,38138,37993,57577,57579,57582,57578,57581,36072,35180,57583,37008,57580,38874,
 61007,57586,60066,57584,57587,57573,57585,36282,57588,57589,38814,61008,57590,57591,61009,57571,57592,35522,36515,
 57593,57594,57595,35162,57664,38234,57665,35490,57666,57667,57668,57670,57671,57669,38258,57673,57672,61010,57675,
 57674,57676,57677,57679,57678,36249,57681,57680,35523,36978,37723,57682,37046,36441,35225,57683,38768,38369,57684,
 60812,37731,38738,36194,36956,37482,39346,37548,35302,57685,57686,57691,57689,57688,40384,35397,57687,35032,38056,
 38088,38831,57692,57690,37499,37028,38057,38220,57694,38826,35948,57695,57693,38100,57696,57697,61011,35033,36852,
 57702,57699,37867,57698,35653,57705,57700,57701,57704,57703,38212,37217,37216,35678,57706,57707,57708,57710,57709,
 35189,57718,38118,57712,57714,57716,36957,57717,57715,36542,57711,57713,38241,36807,57720,57719,57721,36516,36269,
 37783,57722,37577,57724,38815,57723,37257,57730,57732,57733,37491,57731,57728,57725,57726,57729,57736,57734,57735,
 57737,57739,57740,57741,57742,57738,57744,57743,57745,38851,57748,57746,57747,35552,38652,38344,57750,57749,57751,
 57752,57756,57753,57754,57755,57757,57758,57759,57760,57761,38061,37743,57762,38034,38227,57763,61012,57764,37705,
 35398,36195,57765,57766,57767,36424,57769,57768,57770,57771,61015,61013,61014,61016,38119,57772,57773,60041,57774,
 57775,57776,36429,57777,38005,38526,35181,35190,57778,57780,57779,37776,37047,40792,57781,38591,57782,35524,38101,
 57783,57784,57785,38618,38611,37564,37258,57787,36738,36808,57790,57789,57788,38139,35525,36007,57796,57793,36958,
 38576,57792,57794,57795,57791,57797,57798,37549,35553,37509,61018,57799,57800,57803,36999,37826,57804,38514,57801,
 57802,57807,57806,57805,57809,57808,57810,57812,57811,38347,36725,38852,57813,37813,57814,57815,57819,57817,57818,
 57816,57820,57821,57822,57823,38581,57824,38638,57825,37485,38026,35817,37466,57826,35768,37070,57827,36283,57828,
 57829,36004,36307,57831,61020,37749,36308,35693,38467,37994,37750,36219,57833,61021,36809,61022,38832,36196,36005,
 38049,57835,61023,57837,36073,57836,37620,57839,35414,57834,38120,35151,36330,39025,57838,57840,38345,37079,57842,
 57843,57841,35437,57849,57848,36517,57850,57845,57851,57846,38102,57844,57847,57921,57920,38529,57852,35049,57923,
 57922,36810,57924,37218,57926,57925,57927,57830,57832,57929,57928,61024,36518,38887,36560,57930,35926,35679,35654,
 36483,38739,57936,57935,37219,57932,57934,36714,36959,57933,57931,37961,36811,38235,36309,37784,57937,57938,57960,
 35798,39004,37204,57939,35280,37621,38303,61028,61030,57940,35738,57941,57943,57944,37960,57945,57946,57947,35799,
 35281,37827,36679,36484,57948,36680,35272,38242,57949,38121,37220,57952,57953,38025,36960,57950,37505,57951,36812,
 35034,35656,57954,37622,57955,37061,38571,38210,57956,57957,37492,38853,57959,57958,36589,57961,35054,57964,57962,
 35282,35949,57963,36197,36242,38372,57965,38515,57967,37071,35182,35256,34986,57966,57968,57969,36853,57970,35438,
 57972,35978,35718,57973,35827,57974,37114,37835,37086,36339,57975,37506,37259,57977,57979,57976,57978,35905,57980,
 35909,35719,38769,57982,57984,35149,57987,35478,57986,57985,57989,57981,57990,38823,57991,57992,61031,39666,57994,
 57993,57995,57996,38835,57997,59629,36813,57998,57999,36726,37814,58000,61032,37447,61034,58001,37467,58002,35747,
 39262,37500,36529,35526,58003,58016,58006,35720,58005,58018,58004,36814,58008,58009,37706,58010,35453,36985,38276,
 58012,37350,58007,58011,58013,36345,58020,38221,38052,37785,35800,58019,58017,38067,58014,37501,37787,37786,36340,
 58038,58022,58024,58027,58028,58025,58026,58023,58021,58015,38349,35283,58035,58032,58037,58036,38035,38565,36442,
 58030,58039,58034,58033,58029,61035,58031,35527,37468,37115,38048,58044,38050,37087,58041,38093,58045,38353,37498,
 58040,58042,58043,58046,36546,37828,58051,58050,58047,38997,58056,58060,58057,58053,58054,58059,58048,39379,58055,
 58049,58058,58064,35528,58061,58062,58063,58066,58065,38132,58067,38906,38379,58072,58069,58068,37072,58071,58073,
 58070,58077,58074,58075,58052,58076,58078,58079,38340,58080,38624,35788,35912,58081,38322,37000,38574,58082,38833,
 38036,37221,37971,36716,35006,58087,58085,58083,35487,36815,58088,58086,58084,58092,58091,58090,58089,58093,58094,
 37048,58095,58097,58096,36048,37207,58099,37788,58098,58100,38323,37260,36198,58101,38854,58103,58104,58105,58106,
 36485,58107,35950,35722,35657,58176,38641,36199,58108,58179,38628,37979,38226,36739,58178,36561,36200,36486,35721,
 38324,58177,37222,38497,36341,36487,37595,58182,38877,36311,58183,36961,58185,36816,36270,58184,36681,36028,37223,
 58180,58186,61037,58181,35951,58189,58193,35979,58188,58197,61038,36201,38797,35002,58194,35723,58191,58192,37789,
 58190,58187,35399,37090,36006,58199,58196,58198,58195,35952,37297,58200,37262,58213,61040,58209,58203,58207,36600,
 35035,58202,58210,58214,36202,38612,37588,58204,61039,58212,58201,37469,58206,35003,38600,58205,35801,38122,37261,
 38862,36751,58254,61041,58215,37116,58211,58216,58218,37623,58221,58217,38354,35529,38601,35036,58220,38907,58219,
 35215,37866,58222,58229,58223,58230,58226,38043,36552,58228,58225,58231,58224,36707,38468,36715,58227,58240,58235,
 58238,58236,58241,58234,58208,37073,38089,58237,58232,37184,35953,36682,61042,36932,37205,58244,58246,58247,58243,
 58245,58233,58242,58250,58249,38554,35914,58248,58252,58251,58255,58257,36443,58253,58258,58259,60736,58260,58266,
 37722,58262,58261,58263,58264,58265,58267,58268,35530,58269,58270,58271,61043,58272,58273,58274,58275,58276,58278,
 58277,58279,58280,58281,58284,58282,58283,36319,35954,37493,38065,36752,37996,38123,58285,40171,58286,58288,38789,
 58287,58290,58289,38770,58291,38140,58292,58295,58294,58293,61044,58296,35921,37185,35680,58300,58297,58298,58301,
 58302,58299,35144,35237,58304,58305,58306,38786,36683,58308,58307,37001,58309,58310,58311,35555,35531,58312,58313,
 38524,38787,38771,38998,36204,58316,36562,58315,58317,36519,37327,58318,36203,38613,58319,58320,58321,58322,58323,
 36520,38635,58325,37470,58324,58327,58326,58328,37049,58329,58330,38327,58331,37263,58332,58333,38908,58336,58335,
 58334,37550,58337,36933,58338,58339,38999,58340,58341,58343,58342,38051,37879,39005,38055,58345,36817,38217,58346,
 58344,35532,36050,36488,38124,36008,38498,58349,58347,36205,36206,35047,36326,38008,35037,58354,37471,38007,37337,
 58356,58352,58355,58350,58353,38469,36051,35067,58351,58358,58359,37815,35769,58437,37980,36489,35770,37062,39013,
 38572,58357,37074,35698,58360,58362,58361,58363,37445,37981,37551,58434,58433,58364,36980,38277,58436,58435,36207,
 39026,58452,58440,58441,36590,58439,36248,58438,58442,37552,38304,37186,37338,58446,58447,58443,58444,58445,36208,
 58453,58449,38278,38540,38215,58448,58451,58450,38499,58454,58455,37206,58456,58458,58462,58459,58457,37982,58460,
 58461,35248,58468,58463,58464,58465,37279,58467,58466,58469,58470,58471,36962,35303,58472,38869,36521,36684,36490,
 37494,58473,58474,35152,58475,58476,58477,58478,58479,35771,40360,58480,37091,58481,36553,58482,39086,58483,38364,
 35546,37187,36727,38289,36685,58484,36209,58485,38090,58500,58487,37319,38037,36029,58486,37188,58488,37624,58490,
 58489,58492,58491,58493,58496,58494,35533,58497,58498,58499,36271,38855,58501,36934,35216,58502,58503,58504,35056,
 58505,58506,38279,36549,58508,35400,34992,58507,58510,37997,36963,35284,38470,35964,35802,58509,35304,35489,35217,
 58514,38888,37339,38243,58526,35285,58524,58522,58513,58511,58512,36577,35818,37527,37839,35184,58516,58515,58521,
 58517,58520,61046,38606,58519,35286,35485,58523,58525,35955,58529,58538,58539,34985,58546,35055,58537,58536,58531,
 58530,58528,58527,37507,37369,58533,58532,58535,37264,35956,35168,58534,36210,37265,61047,58552,58553,35287,35244,
 58550,61048,58540,58548,58555,58549,58547,58518,58545,58541,35534,58543,58554,58544,58556,58542,38044,38793,58551,
 58573,58565,37019,61049,35685,35803,58560,35289,36818,58563,36312,37744,58568,38380,58559,35288,36052,38216,58569,
 58557,61050,58566,58576,58561,58562,37816,58567,58564,38471,58570,35038,58558,58572,58571,38027,58578,58589,35486,
 58592,58574,58579,38798,58588,61051,38772,38824,37528,35467,38290,58594,37791,34991,58587,58583,37266,58577,58585,
 58590,37963,34984,58582,58591,38296,58586,58581,36819,36686,36522,38614,38246,58597,58606,58584,35479,61052,36854,
 58595,58600,37267,58596,58603,37502,58604,38773,58593,35415,58599,58602,38570,58605,58598,58601,60740,38472,38976,
 58609,58616,58608,36545,58575,38348,38560,58615,58614,58610,58611,35157,58613,58607,37587,58612,35068,37280,38337,
 58617,58688,38103,58620,36820,36551,58690,35772,61053,58691,38297,58619,61054,58580,58618,39022,37792,38291,61056,
 58698,58704,58705,58692,38038,58702,58694,58696,58706,58695,58699,35218,37859,58700,58703,58693,37189,58697,36422,
 36964,35919,38642,38647,36754,61058,58710,58708,39021,58707,38805,58709,58711,58712,58715,58713,37793,58714,38091,
 58701,36755,58716,58721,37268,58720,58689,58722,37224,58717,58719,58718,40784,40769,58724,58723,38806,57786,58725,
 58726,58727,36053,35699,58729,39292,35733,38840,35825,58730,58731,37518,58732,37880,35000,35297,58737,58738,58733,
 36444,58734,37985,58735,58736,58746,58740,58743,58739,58741,58742,36566,58744,37472,35957,35425,58747,35422,58753,
 58748,58752,38072,58749,58750,38247,38104,58754,37371,58764,58760,35305,58758,38473,58759,58756,58757,58762,58765,
 58763,58761,58755,37495,58772,38568,58770,58771,58766,58768,58769,58767,37092,39000,58776,58777,58783,36937,58779,
 58782,58774,58773,58784,35290,58780,58785,58781,58778,37553,58775,38024,58789,38746,58788,58787,58796,58790,58798,
 38790,58801,58792,58793,58797,58800,58799,58791,58794,58811,58804,58802,58803,58808,58809,35401,35681,58807,58786,
 61061,58806,58810,58805,58812,58814,58813,58816,58815,58745,58820,58817,58818,58819,58821,35980,58823,58822,36687,
 36211,40869,58824,36720,35416,58825,35185,36821,58826,36212,58827,35039,38236,58828,37002,58835,58832,37519,58833,
 58830,35804,58829,58836,35925,37340,58842,58838,37299,58837,58840,58831,58841,58843,38125,58839,58844,58846,36049,
 58834,35007,58845,36313,38900,58847,58848,37269,38816,58849,38740,58850,58851,38370,58852,36286,38817,58857,58858,
 36822,58856,61062,38791,58853,58855,37051,37022,58854,58859,38305,58861,58860,35468,38474,58862,60737,58874,58864,
 58865,58866,58867,58871,58872,58870,58868,58863,58869,58873,59573,35238,58876,35805,58875,58945,58944,58947,58946,
 58948,36688,58949,58950,58951,37052,38774,58952,38306,37989,58953,58954,36009,35659,58955,36491,37984,58956,35439,
 58957,58959,38807,58958,36965,58960,58961,58962,35535,58963,58964,58965,58966,35440,58967,58968,58969,35312,36935,
 58970,58971,58972,36030,37625,58973,35958,36981,58976,37794,58975,61063,35920,58974,37365,35660,58977,58978,36823,
 35981,58979,38475,37085,35734,38643,37225,58980,61064,36966,37520,36824,58981,58984,58985,36284,37312,58983,36825,
 38237,58982,36492,35186,58989,35959,36494,36493,39020,58988,58987,37190,35692,39010,35417,36826,61065,58986,58991,
 58992,58990,36054,38751,36495,37958,58995,37054,37473,38741,58998,36074,37053,58994,58999,36075,58996,58997,61066,
 58993,37088,37831,37454,35291,38126,35682,61067,37554,59002,59000,37483,37055,35536,59001,36986,38856,39007,59003,
 59015,37555,59014,61068,59011,59019,59012,59008,37626,59006,59004,38720,36496,59009,59005,61070,59013,36756,36031,
 37368,38500,35193,35040,37795,59017,59016,37860,59021,59010,59020,59022,36010,59018,36213,36563,59023,38775,59026,
 59029,59027,38228,59024,35806,59028,59030,59034,59031,59033,59032,61071,59035,36527,59037,59036,38280,59039,35960,
 59038,59040,59041,35683,58303,36855,59042,36076,59043,61072,59044,36445,40396,59045,59046,36689,59047,59048,59049,
 59050,59051,37450,59052,59054,59053,37796,59055,38476,59056,59057,59058,59059,37848,36827,59060,36235,39084,59061,
 59062,38238,59063,59071,59064,59066,59065,59067,38501,59068,59069,59070,59072,35404,37605,38281,36320,36214,38254,
 35293,38092,59075,35537,37075,59074,59079,37529,38625,59077,59078,35661,59080,38019,37341,38127,37724,59076,38502,
 35306,59082,38983,37568,39012,36497,59081,37295,59098,37191,37878,38255,59085,36446,36498,36828,38021,36011,59084,
 59083,38282,36543,37745,61073,61074,59087,59088,36215,59086,59089,59090,59092,37281,59091,35556,59094,59093,59095,
 61075,59097,59099,59100,37076,36557,59101,35441,59102,37270,59103,59104,38283,61076,35662,59105,37556,35194,59106,
 36591,37014,37291,59109,59108,59107,59115,59113,59110,59112,59111,59114,35735,59118,37077,59119,36055,59116,59117,
 38984,37557,37192,59120,59123,59121,59122,38776,37797,59126,59124,59125,59127,59208,59130,59131,59129,59128,37627,
 59200,59204,59201,59132,59202,59203,59210,59205,37078,59207,59209,59206,59212,36690,59211,59213,59214,59217,59216,
 59215,59219,59218,38644,59221,59220,59222,59223,59225,59224,36967,59226,35819,59227,59229,59230,59231,59228,59232,
 36564,59233,35663,35922,61078,36012,59234,37870,37725,59235,59238,36530,59237,59236,35961,59239,35442,59241,36314,
 59240,59249,59243,59245,38371,59242,59244,59248,59246,35664,59247,59250,38009,38870,36691,59251,38721,59253,59252,
 59256,38752,59255,35469,59254,59259,59258,59257,37713,59260,59261,59262,36236,35908,59264,59265,59266,36968,59267,
 36523,59268,59269,39327,39326,59270,58256,59271,37443,36938,37983,59272,38355,37586,36254,37448,35145,38552,36982,
 35965,35807,38356,59273,59275,59274,35294,37876,59276,38039,37714,59277,36721,59279,38592,59294,59281,59282,37575,
 37342,37271,37798,59280,35700,59289,59286,59299,37799,37504,59283,37628,37746,59284,59288,36992,38023,37578,37056,
 59287,37292,37282,59285,34983,38977,59290,37343,36692,36969,59292,59291,35053,59293,38222,59301,37849,37003,37496,
 35830,59300,38742,35166,38357,35295,59295,59296,59297,59298,37817,37442,35041,59302,59303,60065,37307,59304,35219,
 37227,36013,38777,61081,59305,37707,37272,36565,59306,59309,36741,59307,37194,37193,35042,38857,59311,38128,59313,
 59312,59310,57988,35538,59278,59315,59314,59316,38743,37855,38477,59317,36567,59318,59319,59320,37696,35048,36216,
 39001,59324,61082,35923,59321,59322,38292,35443,38744,35773,37747,59325,59326,61084,59327,61085,37697,59329,59328,
 37841,59330,36693,36574,38010,37521,36592,37004,59331,59332,36988,59333,59334,59335,38799,36694,59337,59336,36217,
 36243,36447,59340,36742,59339,59338,37351,36077,37057,38062,36696,59341,36829,59344,59342,59343,59346,59345,36856,
 59347,59348,59349,38094,36305,36575,59350,59351,38818,36708,38636,38858,59352,35808,59353,61087,37698,61086,59356,
 35480,36970,61088,59354,59355,37598,61091,61092,38516,35834,61089,61090,59358,59359,59357,59361,61093,61095,37853,
 35426,61094,59365,59362,59364,59360,59502,59363,38889,36056,61102,61096,61098,59373,61097,37715,59368,59371,59369,
 59374,61099,59375,61101,59367,61100,59380,35220,59366,38059,59370,36830,61103,36218,61105,61106,38503,35810,36709,
 37818,60739,37196,59378,59372,59377,38593,37558,59379,59376,61104,37195,59383,59382,59381,61110,38478,61114,61112,
 61108,61109,61113,36763,61107,59384,38365,35187,38245,37522,35736,60745,59386,61117,36220,61120,61122,36427,59385,
 37005,37006,59456,59458,61121,61119,36857,61116,59457,59459,61115,35793,38244,36576,38978,59388,36342,39006,59461,
 59460,59462,59387,60738,37863,37748,37589,59467,61124,37474,59463,59464,35916,59466,61123,36014,59465,36831,35481,
 59471,36285,37273,37576,61125,35418,59469,59470,37569,59468,59472,59478,61126,59481,59480,37708,59473,59474,59477,
 59479,61127,35774,59482,59476,59475,61128,59486,59487,59488,59485,59484,36832,37800,59483,59492,59490,61129,59491,
 59489,37366,59493,59494,59496,61130,61131,35539,59495,38648,59507,59497,59500,59498,59499,59501,59503,59504,59505,
 59508,59506,59509,59511,59510,37559,38629,59512,37197,59513,38338,59514,35402,35163,35541,61132,35540,59515,59516,
 59517,59518,59520,35542,35444,36221,38068,59522,59521,59523,35195,59526,59525,59524,59527,59530,35013,59528,59532,
 59531,59534,59533,59535,37804,59536,59537,59539,59538,38284,59540,59541,36323,59542,59543,38504,37226,34978,37321,
 59544,38285,59547,59545,36222,59546,36032,38339,59549,59551,59550,59552,35136,36983,36764,35543,59553,38022,59555,
 35137,59554,37570,38859,37801,59548,38820,36015,38778,35831,38834,35911,37344,58432,59556,35403,37007,35445,59558,
 59559,59557,35972,36315,36833,61135,35138,38871,59561,59308,59560,61136,59564,59562,59563,59565,59566,38890,59567,
 59568,37063,38073,37021,35557,38745,35307,36695,36057,59571,59570,36499,59572,59569,36423,59576,58795,39380,37015,
 59574,38819,37871,35146,37089,36532,38325,35167,38891,38795,59577,37732,36601,59578,59579,36971,59580,38892,59575,
 59582,59584,59583,59581,59585,59586,37274,35296,59587,38582,59588,59589,38985,61137,40528,59590,61138,59591,59592,
 59596,61139,59593,59594,59595,59597,61140,61141,61142,37058,61143,38645,37059,59598,38129,59599,60018,38602,59600,
 59601,59602,35446,59604,36984,59605,35907,59606,59610,59608,59609,35475,59607,59611,59612,35014,59613,59614,36834,
 59615,35686,59618,59617,59616,59025,38362,59619,59620,59621,59622,59623,59624,35544,59625,59626,37954,59628,35257,
 59631,59630,35139,35775,38341,37560,36256,36224,36743,36987,59633,59632,38753,35558,38096,37850,37020,38860,35962,
 59636,59635,38506,37802,35183,59637,59634,38256,38794,59638,59639,59641,37352,35450,35451,59640,35559,36016,61144,
 35560,37726,38878,61145,36058,59642,59643,59644,59712,59714,59713,38295,59715,59716,59717,59718,59720,59719,59721,
 38130,58314,36936,35665,59722,59723,39338,40794,38097,35065,35001,36500,38479,36860,59724,38621,59725,38779,35169,
 36448,59726,35308,59727,59728,59730,59731,59733,59729,59732,61148,35545,59734,59735,59736,59737,59738,59740,59739,
 59742,59745,59741,59743,59744,59746,35776,36593,59747,59748,36225,61150,59749,35421,37998,59750,59751,37497,37865,
 59752,38045,37322,35191,35820,35821,37523,59757,35822,35309,59756,59754,59755,59753,59767,59758,59759,59760,59761,
 59763,59762,36728,59764,59766,35666,59765,37275,36017,59768,37323,59769,37803,59770,59776,59773,59772,59774,59771,
 59778,61151,59777,59780,35777,59779,59781,59782,59784,59783,59785,59787,59786,36252,59788,59789,35419,59790,59791,
 37009,59792,59793,59794,59795,36226,61152,61153,59796,59797,59798,59799,59800,38063,59802,38213,59803,59801,59805,
 59804,59806,59807,59808,59809,59810,59811,59812,59813,59814,59815,59816,59817,59818,59819,59820,40788,59821,58102,
 35667,35392,36272,59823,59822,38563,59825,59826,59824,59827,38530,59828,35739,38980,61155,59829,61154,59831,35004,
 61156,59832,38313,59830,59833,59834,59835,59836,59837,38542,36428,36344,37198,61157,59838,59841,61158,59839,59842,
 36079,59840,59843,59844,59845,59849,36425,37346,59850,59847,59846,59848,35966,59854,59853,59852,34993,61159,59864,
 59860,59861,59857,59863,59859,35458,39019,59862,59858,59856,59855,59866,59869,59868,59867,38248,59865,35057,59870,
 59872,35471,59851,35158,59874,59873,59871,37452,38544,38872,59875,59876,59877,59878,59879,37561,59880,38069,59885,
 59881,59882,38480,38594,37838,59886,59887,37820,59884,59883,35240,59895,59894,35221,59892,59891,59889,35483,59888,
 36528,35239,36227,59898,59897,59896,59893,59899,59900,59972,59971,59973,35148,59968,59969,36244,38583,59970,61161,
 38481,59978,61160,59974,59979,59976,59975,35963,59980,59981,59982,59977,59890,59983,37599,59987,59988,59986,59985,
 59991,59984,59989,59990,59993,59992,59995,59996,59997,39016,59994,37353,36331,59998,61163,59999,60000,60001,60002,
 36018,60003,60004,36525,60005,60006,60007,60008,60011,60009,39003,60010,38893,60012,38873,60013,38046,60014,60016,
 60017,60015,36237,38603,38531,39925,40832,38555,35241,60019,35695,60020,60021,60022,61164,36245,60023,57554,38617,
 37345,60024,60026,60025,60027,60028,60029,60030,60032,60033,60034,60035,60036,60037,60038,60039,60040,37699,36059,
 60042,37228,60043,60044,38208,60045,60046,57942,59096,59627,60047,60048,60050,60051,60052,38894,60049,60053,60054,
 60056,60055,60058,60059,60057,38836,60060,60061,57971,60062,60868,61133,60787,60798,60800,60821,60860,60876,60878,
 60921,60994,61017,61025,61026,61027,61029,61033,61036,61045,61057,61059,61060,61069,61077,61079,61080,61083,61111,
 61118,61134,61146,61147,61149,61162,33097,61180,33172,33168,33171,33173,61179,33129,33130,33174,33147,33091,33148,
 33092,33118,33359,33360,33361,33362,33363,33364,33365,33366,33367,33368,33094,33095,33155,33153,33156,33096,33175,
 33376,33377,33378,33379,33380,33381,33382,33383,33384,33385,33386,33387,33388,33389,33390,33391,33392,33393,33394,
 33395,33396,33397,33398,33399,33400,33401,33133,33119,33134,33103,33105,33101,33409,33410,33411,33412,33413,33414,
 33415,33416,33417,33418,33419,33420,33421,33422,33423,33424,33425,33426,33427,33428,33429,33430,33431,33432,33433,
 33434,33135,33122,33136,33120,33169,33170,33226,33104,61178,33167,
#else
 33176,33102,33163,33149,33100,33271,33150,33152,33695,33696,33697,33698,33699,33700,33701,33702,33703,33704,33705,
 33706,33707,33708,33709,33710,33711,33712,33713,33714,33715,33716,33717,33718,33727,33728,33729,33730,33731,33732,
 33733,33734,33735,33736,33737,33738,33739,33740,33741,33742,33743,33744,33745,33746,33747,33748,33749,33750,33862,
 33856,33857,33858,33859,33860,33861,33863,33864,33865,33866,33867,33868,33869,33870,33871,33872,33873,33874,33875,
 33876,33877,33878,33879,33880,33881,33882,33883,33884,33885,33886,33887,33888,33904,33905,33906,33907,33908,33909,
 33911,33912,33913,33914,33915,33916,33917,33918,33920,33921,33922,33923,33924,33925,33926,33927,33928,33929,33930,
 33931,33932,33933,33934,33935,33936,33937,33910,33117,33116,33125,33126,33127,33128,33269,33270,33124,33123,33265,
 33164,33165,33190,33166,34690,34692,33264,34644,34645,34646,34647,34648,34649,34650,34651,34652,34653,33193,33194,
 33192,33195,33227,33228,33229,33245,33230,33246,33208,33209,34708,33251,33253,33159,34712,33242,33121,33224,33225,
 33215,33214,33255,33256,34707,33160,33254,33252,33248,33154,33247,33157,33158,33249,33250,33212,33213,33210,33211,
 33243,34713,33244,34624,34625,34626,34627,34628,34629,34630,34631,34632,34633,34634,34635,34636,34637,34638,34639,
 34640,34641,34642,34643,33951,33962,33952,33963,33953,33964,33954,33965,33956,33967,33955,33966,33957,33978,33973,
 33968,33959,33980,33975,33970,33958,33974,33979,33969,33960,33976,33981,33971,33961,33977,33982,33972,33185,33184,
 33187,33186,33189,33188,33183,33182,33179,33181,33180,33276,33178,33177,33162,33161,33268,33267,33266,33088,33089,
 33090,33110,33112,33113,33114,33137,33138,33139,33140,33141,33142,33143,33144,33145,33146,33191,33196,33131,33132,
 34688,34689,33439,33440,33441,33442,33443,33444,33445,33446,33447,33448,33449,33450,33451,33452,33453,33454,33455,
 33456,33457,33458,33459,33460,33461,33462,33463,33464,33465,33466,33467,33468,33469,33470,33471,33472,33473,33474,
 33475,33476,33477,33478,33479,33480,33481,33482,33483,33484,33485,33486,33487,33488,33489,33490,33491,33492,33493,
 33494,33495,33496,33497,33498,33499,33500,33501,33502,33503,33504,33505,33506,33507,33508,33509,33510,33511,33512,
 33513,33514,33515,33516,33517,33518,33519,33520,33521,33098,33099,33108,33109,33600,33601,33602,33603,33604,33605,
 33606,33607,33608,33609,33610,33611,33612,33613,33614,33615,33616,33617,33618,33619,33620,33621,33622,33623,33624,
 33625,33626,33627,33628,33629,33630,33631,33632,33633,33634,33635,33636,33637,33638,33639,33640,33641,33642,33643,
 33644,33645,33646,33647,33648,33649,33650,33651,33652,33653,33654,33655,33656,33657,33658,33659,33660,33661,33662,
 33664,33665,33666,33667,33668,33669,33670,33671,33672,33673,33674,33675,33676,33677,33678,33679,33680,33681,33682,
 33683,33684,33685,33686,33093,33115,33106,33107,34698,34699,34700,34693,34694,34695,34696,34697,34661,34665,34656,
 34659,34657,34667,34666,34660,34668,34662,34670,34655,34669,34658,34663,34664,34686,34703,34702,34701,34674,34675,
 34671,34672,34673,34677,34676,34691,33111,33097,33172,33168,33171,33173,33129,33130,33174,33147,33091,33148,33092,
 33118,33359,33360,33361,33362,33363,33364,33365,33366,33367,33368,33094,33095,33155,33153,33156,33096,33175,33376,
 33377,33378,33379,33380,33381,33382,33383,33384,33385,33386,33387,33388,33389,33390,33391,33392,33393,33394,33395,
 33396,33397,33398,33399,33400,33401,33133,33119,33134,33103,33105,33101,33409,33410,33411,33412,33413,33414,33415,
 33416,33417,33418,33419,33420,33421,33422,33423,33424,33425,33426,33427,33428,33429,33430,33431,33432,33433,33434,
 33135,33122,33136,33120,33169,33170,33226,33104,33167,
#endif
};

#ifndef ALICE_TEXT_NO_BUILTIN_FONT

// Font generated from Misaki Gothic
static const uint8_t alice_font_pool[6184] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x40,0x40,0x40,0x00,0x40,0x00,0x00,0xA0,0xA0,0x00,0x00,0x00,0x00,0x00,0x00,
 0xA0,0xE0,0xA0,0xA0,0xE0,0xA0,0x00,0x00,0x40,0xE0,0xC0,0x60,0xE0,0x40,0x00,0x00,0x00,0x80,0x20,0x40,0x80,0x20,0x00,0x00,
 0x40,0xA0,0x40,0xA0,0x80,0x60,0x00,0x00,0x40,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x40,0x40,0x40,0x40,0x40,0x20,0x00,
 0x80,0x40,0x40,0x40,0x40,0x40,0x80,0x00,0x40,0xE0,0x40,0xA0,0x00,0x00,0x00,0x00,0x00,0x40,0x40,0xE0,0x40,0x40,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x40,0x80,0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,
 0x00,0x00,0x20,0x40,0x80,0x00,0x00,0x00,0x00,0x40,0xA0,0xE0,0xA0,0x40,0x00,0x00,0x00,0x40,0xC0,0x40,0x40,0xE0,0x00,0x00,
 0x00,0xC0,0x20,0x40,0x80,0xE0,0x00,0x00,0x00,0xC0,0x20,0x40,0x20,0xC0,0x00,0x00,0x00,0x20,0x60,0xA0,0xE0,0x20,0x00,0x00,
 0x00,0xE0,0x80,0xC0,0x20,0xC0,0x00,0x00,0x00,0x60,0x80,0xC0,0xA0,0x40,0x00,0x00,0x00,0xE0,0x20,0x40,0x40,0x40,0x00,0x00,
 0x00,0x40,0xA0,0x40,0xA0,0x40,0x00,0x00,0x00,0x40,0xA0,0x60,0x20,0xC0,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x40,0x00,0x00,
 0x00,0x00,0x40,0x00,0x00,0x40,0x80,0x00,0x00,0x20,0x40,0x80,0x40,0x20,0x00,0x00,0x00,0x00,0xE0,0x00,0xE0,0x00,0x00,0x00,
 0x00,0x80,0x40,0x20,0x40,0x80,0x00,0x00,0x40,0xA0,0x20,0x40,0x00,0x40,0x00,0x00,0x40,0xA0,0x20,0x60,0xA0,0x40,0x00,0x00,
 0x40,0xA0,0xA0,0xE0,0xA0,0xA0,0x00,0x00,0xC0,0xA0,0xC0,0xA0,0xA0,0xC0,0x00,0x00,0x60,0x80,0x80,0x80,0x80,0x60,0x00,0x00,
 0xC0,0xA0,0xA0,0xA0,0xA0,0xC0,0x00,0x00,0xE0,0x80,0xC0,0x80,0x80,0xE0,0x00,0x00,0xE0,0x80,0xC0,0x80,0x80,0x80,0x00,0x00,
 0x60,0x80,0x80,0xA0,0xA0,0x60,0x00,0x00,0xA0,0xA0,0xA0,0xE0,0xA0,0xA0,0x00,0x00,0xE0,0x40,0x40,0x40,0x40,0xE0,0x00,0x00,
 0x20,0x20,0x20,0x20,0xA0,0x40,0x00,0x00,0xA0,0xA0,0xC0,0xA0,0xA0,0xA0,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0xE0,0x00,0x00,
 0xA0,0xE0,0xE0,0xA0,0xA0,0xA0,0x00,0x00,0xC0,0xA0,0xA0,0xA0,0xA0,0xA0,0x00,0x00,0x40,0xA0,0xA0,0xA0,0xA0,0x40,0x00,0x00,
 0xC0,0xA0,0xA0,0xC0,0x80,0x80,0x00,0x00,0x40,0xA0,0xA0,0xA0,0xA0,0x40,0x20,0x00,0xC0,0xA0,0xA0,0xC0,0xA0,0xA0,0x00,0x00,
 0x60,0x80,0x40,0x20,0x20,0xC0,0x00,0x00,0xE0,0x40,0x40,0x40,0x40,0x40,0x00,0x00,0xA0,0xA0,0xA0,0xA0,0xA0,0xE0,0x00,0x00,
 0xA0,0xA0,0xA0,0xA0,0xC0,0x80,0x00,0x00,0xA0,0xA0,0xA0,0xE0,0xE0,0xA0,0x00,0x00,0xA0,0xA0,0x40,0x40,0xA0,0xA0,0x00,0x00,
 0xA0,0xA0,0x40,0x40,0x40,0x40,0x00,0x00,0xE0,0x20,0x40,0x40,0x80,0xE0,0x00,0x00,0x60,0x40,0x40,0x40,0x40,0x40,0x60,0x00,
 0x00,0x00,0x80,0x40,0x20,0x00,0x00,0x00,0xC0,0x40,0x40,0x40,0x40,0x40,0xC0,0x00,0x40,0xA0,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,0x40,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0xA0,0xA0,0x60,0x00,0x00,
 0x80,0x80,0xC0,0xA0,0xA0,0xC0,0x00,0x00,0x00,0x00,0x60,0x80,0x80,0x60,0x00,0x00,0x20,0x20,0x60,0xA0,0xA0,0x60,0x00,0x00,
 0x00,0x00,0x60,0xE0,0x80,0x60,0x00,0x00,0x60,0x40,0xE0,0x40,0x40,0x40,0x00,0x00,0x00,0x00,0x60,0xA0,0x60,0x20,0xC0,0x00,
 0x80,0x80,0xC0,0xA0,0xA0,0xA0,0x00,0x00,0x40,0x00,0x40,0x40,0x40,0x40,0x00,0x00,0x40,0x00,0x40,0x40,0x40,0x40,0x80,0x00,
 0x80,0x80,0xA0,0xC0,0xA0,0xA0,0x00,0x00,0xC0,0x40,0x40,0x40,0x40,0x40,0x00,0x00,0x00,0x00,0xC0,0xE0,0xE0,0xA0,0x00,0x00,
 0x00,0x00,0xC0,0xA0,0xA0,0xA0,0x00,0x00,0x00,0x00,0x40,0xA0,0xA0,0x40,0x00,0x00,0x00,0x00,0xC0,0xA0,0xA0,0xC0,0x80,0x00,
 0x00,0x00,0x60,0xA0,0xA0,0x60,0x20,0x00,0x00,0x00,0xA0,0xC0,0x80,0x80,0x00,0x00,0x00,0x00,0x60,0xC0,0x60,0xC0,0x00,0x00,
 0x00,0x40,0xE0,0x40,0x40,0x60,0x00,0x00,0x00,0x00,0xA0,0xA0,0xA0,0xE0,0x00,0x00,0x00,0x00,0xA0,0xA0,0xC0,0x80,0x00,0x00,
 0x00,0x00,0xA0,0xA0,0xE0,0xE0,0x00,0x00,0x00,0x00,0xA0,0x40,0x40,0xA0,0x00,0x00,0x00,0x00,0xA0,0xA0,0x60,0x20,0xC0,0x00,
 0x00,0x00,0xE0,0x20,0x40,0xE0,0x00,0x00,0x20,0x40,0x40,0x80,0x40,0x40,0x20,0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00,
 0x80,0x40,0x40,0x20,0x40,0x40,0x80,0x00,0x00,0x00,0xC0,0x60,0x00,0x00,0x00,0x00,0x00,0x40,0xE0,0xC0,0xE0,0x40,0x00,0x00,
 0x60,0x40,0xE0,0x40,0xC0,0xE0,0x00,0x00,0xA0,0x40,0xE0,0x40,0xE0,0x40,0x00,0x00,0x40,0x40,0x40,0x00,0x40,0x40,0x40,0x00,
 0x1C,0x20,0x18,0x24,0x18,0x04,0x38,0x00,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x20,0x00,0x00,0x00,
 0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x90,0x90,0x60,0x00,0x00,0x00,0x00,
 0x10,0x10,0xFE,0x10,0x10,0x00,0xFE,0x00,0x08,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x74,0x74,0x34,0x14,0x14,0x14,0x00,
 0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x00,0x00,0x10,0x00,0xFE,0x00,0x10,0x00,0x00,0x00,0x10,0x3C,0x10,0x3C,0x5A,0x32,0x00,
 0x20,0x7C,0x20,0x3C,0x6A,0xB2,0x64,0x00,0x00,0x00,0x48,0x44,0x44,0x50,0x20,0x00,0x00,0x88,0x84,0x82,0x82,0x50,0x20,0x00,
 0x00,0x38,0x00,0x38,0x44,0x08,0x30,0x00,0x3C,0x00,0x3C,0x42,0x02,0x04,0x38,0x00,0x00,0x38,0x00,0x78,0x10,0x30,0x4C,0x00,
 0x3C,0x00,0x7C,0x08,0x18,0x28,0x46,0x00,0x00,0x20,0x74,0x20,0x38,0x64,0x28,0x00,0x20,0xF4,0x22,0x3C,0x62,0xA2,0x6C,0x00,
 0x20,0x20,0xF4,0x2A,0x4A,0x48,0xB0,0x00,0x2A,0x20,0xF4,0x2A,0x4A,0x48,0xB0,0x00,0x10,0x7C,0x08,0x7E,0x24,0x40,0x3C,0x00,
 0x14,0x7C,0x08,0x7E,0x24,0x40,0x3C,0x00,0x04,0x08,0x30,0x40,0x30,0x08,0x04,0x00,0x04,0x08,0x34,0x40,0x30,0x08,0x04,0x00,
 0x88,0x88,0xBE,0x88,0x88,0x88,0x10,0x00,0x8A,0x88,0xBE,0x88,0x88,0x88,0x10,0x00,0x00,0x3C,0x00,0x00,0x20,0x40,0x3E,0x00,
 0x0A,0x3C,0x00,0x00,0x20,0x40,0x3E,0x00,0x08,0x08,0x7E,0x04,0x24,0x40,0x3C,0x00,0x0A,0x08,0x7E,0x04,0x24,0x40,0x3C,0x00,
 0x20,0x20,0x20,0x20,0x20,0x22,0x1C,0x00,0x2A,0x20,0x20,0x20,0x20,0x22,0x1C,0x00,0x08,0xFE,0x18,0x28,0x18,0x08,0x10,0x00,
 0x0A,0xFE,0x18,0x28,0x18,0x08,0x10,0x00,0x24,0x24,0xFE,0x24,0x2C,0x20,0x1E,0x00,0x2A,0x24,0xFE,0x24,0x2C,0x20,0x1E,0x00,
 0x3C,0x08,0x10,0x7E,0x10,0x10,0x0C,0x00,0x3C,0x0A,0x10,0x7E,0x10,0x10,0x0C,0x00,0x20,0xF0,0x2E,0x40,0x48,0x50,0x8E,0x00,
 0x2A,0xF0,0x2E,0x40,0x48,0x50,0x8E,0x00,0x08,0x7E,0x10,0x1C,0x22,0x02,0x1C,0x00,0x0A,0x7E,0x10,0x1C,0x22,0x02,0x1C,0x00,
 0x00,0x00,0x00,0x18,0x64,0x04,0x18,0x00,0x00,0x3C,0xC2,0x02,0x02,0x1C,0x00,0x00,0x0A,0x3C,0xC2,0x02,0x02,0x1C,0x00,0x00,
 0x0E,0x74,0x08,0x10,0x10,0x08,0x06,0x00,0x0E,0x74,0x0A,0x10,0x10,0x08,0x06,0x00,0x20,0x20,0x24,0x18,0x20,0x40,0x3E,0x00,
 0x2A,0x20,0x24,0x18,0x20,0x40,0x3E,0x00,0x20,0xF4,0x22,0x44,0x9C,0x26,0x18,0x00,0x80,0x9C,0x80,0x80,0x90,0xA0,0x9E,0x00,
 0x08,0x48,0x5C,0x6A,0xB2,0xA6,0x56,0x00,0x20,0x2C,0xF2,0x22,0x66,0xAA,0x26,0x00,0x00,0x38,0x54,0x92,0xA2,0x44,0x18,0x00,
 0x84,0x84,0xBE,0x84,0x9C,0xA4,0x9A,0x00,0x8A,0x84,0xBE,0x84,0x9C,0xA4,0x9A,0x00,0x84,0x8A,0xBE,0x84,0x9C,0xA4,0x9A,0x00,
 0x28,0xE4,0x26,0x44,0x44,0x44,0x38,0x00,0x2A,0xE4,0x26,0x44,0x44,0x44,0x38,0x00,0x2C,0xEA,0x26,0x44,0x44,0x44,0x38,0x00,
 0x10,0x08,0x10,0x10,0x4C,0x4A,0xB2,0x00,0x10,0x0A,0x10,0x10,0x4C,0x4A,0xB2,0x00,0x14,0x0A,0x14,0x10,0x4C,0x4A,0xB2,0x00,
 0x00,0x20,0x50,0x88,0x06,0x00,0x00,0x00,0x0A,0x20,0x50,0x88,0x06,0x00,0x00,0x00,0x04,0x2A,0x54,0x88,0x06,0x00,0x00,0x00,
 0x80,0xBE,0x88,0xBE,0x88,0xBC,0xBA,0x00,0x8A,0xBE,0x88,0xBE,0x88,0xBC,0xBA,0x00,0x84,0xBA,0x8C,0xBE,0x88,0xBC,0xBA,0x00,
 0x08,0x7E,0x08,0x7E,0x08,0x7C,0x7A,0x00,0x70,0x10,0x24,0x7C,0xA6,0xC4,0x18,0x00,0x20,0xF4,0x22,0x60,0xA0,0x62,0x3C,0x00,
 0x08,0x48,0x5C,0x6A,0xB2,0xA2,0x54,0x00,0x10,0x7C,0x20,0x7C,0x22,0x22,0x1C,0x00,0x00,0x28,0x2C,0x72,0x14,0x10,0x08,0x00,
 0x48,0x5C,0xE2,0x24,0x20,0x10,0x10,0x00,0x00,0x10,0x58,0x74,0x54,0x18,0x20,0x00,0x10,0xBC,0xD2,0x92,0xBC,0x10,0x20,0x00,
 0x00,0x08,0x08,0x0C,0x38,0x48,0x34,0x00,0x08,0x08,0x0E,0x08,0x38,0x4C,0x32,0x00,0x30,0x08,0x40,0x5C,0x62,0x02,0x3C,0x00,
 0x58,0x64,0x44,0x44,0x04,0x08,0x30,0x00,0x3C,0x08,0x10,0x3C,0x42,0x32,0x3C,0x00,0x20,0x2C,0xF4,0x24,0x64,0xA4,0x22,0x00,
 0x3C,0x08,0x10,0x3C,0x42,0x02,0x3C,0x00,0x00,0x10,0x14,0x7A,0x32,0x52,0x14,0x00,0x20,0x2C,0xF2,0x22,0x62,0xA2,0x2C,0x00,
 0x70,0x10,0x3C,0x52,0x96,0xAA,0x46,0x00,0x38,0x10,0x38,0x44,0x18,0x6C,0x92,0x00,0x10,0x7C,0x20,0x76,0x98,0x28,0x1E,0x00,
 0x10,0x10,0x20,0x20,0x50,0x52,0x8C,0x00,0xA0,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0xA0,0x40,0x00,0x00,0x00,0x00,0x00,
 0x00,0x20,0x10,0x08,0x0C,0x30,0x00,0x00,0x0A,0x20,0x10,0x08,0x0C,0x30,0x00,0x00,0x00,0x00,0x7C,0x14,0x18,0x10,0x20,0x00,
 0x7E,0x02,0x14,0x18,0x10,0x10,0x20,0x00,0x00,0x00,0x04,0x08,0x18,0x68,0x08,0x00,0x02,0x04,0x08,0x18,0x68,0x08,0x08,0x00,
 0x00,0x00,0x10,0x7C,0x44,0x08,0x30,0x00,0x10,0x7E,0x42,0x02,0x04,0x08,0x30,0x00,0x00,0x00,0x00,0x38,0x10,0x10,0x7C,0x00,
 0x00,0x7C,0x10,0x10,0x10,0xFE,0x00,0x00,0x00,0x00,0x08,0x7C,0x18,0x68,0x18,0x00,0x08,0xFE,0x08,0x18,0x28,0xC8,0x18,0x00,
 0x10,0x7E,0x12,0x12,0x12,0x22,0x46,0x00,0x14,0x7E,0x12,0x12,0x12,0x22,0x46,0x00,0x10,0x7C,0x10,0x10,0x7E,0x08,0x08,0x00,
 0x14,0x7C,0x10,0x10,0x7E,0x08,0x08,0x00,0x10,0x1E,0x22,0x42,0x04,0x08,0x30,0x00,0x14,0x1E,0x22,0x42,0x04,0x08,0x30,0x00,
 0x40,0x7E,0x48,0x88,0x08,0x10,0x20,0x00,0x4A,0x7E,0x48,0x88,0x08,0x10,0x20,0x00,0x00,0x7E,0x02,0x02,0x02,0x02,0x7E,0x00,
 0x0A,0x7E,0x02,0x02,0x02,0x02,0x7E,0x00,0x24,0xFE,0x24,0x24,0x04,0x08,0x30,0x00,0x2A,0xFE,0x24,0x24,0x04,0x08,0x30,0x00,
 0x40,0x20,0x42,0x22,0x04,0x08,0x70,0x00,0x4A,0x20,0x42,0x22,0x04,0x08,0x70,0x00,0x00,0x7C,0x04,0x08,0x08,0x34,0xC2,0x00,
 0x0A,0x7C,0x04,0x08,0x08,0x34,0xC2,0x00,0x20,0x20,0xFE,0x22,0x24,0x20,0x1E,0x00,0x2A,0x20,0xFE,0x22,0x24,0x20,0x1E,0x00,
 0x42,0x22,0x22,0x02,0x04,0x08,0x30,0x00,0x4A,0x20,0x22,0x02,0x04,0x08,0x30,0x00,0x10,0x1E,0x22,0x5A,0x04,0x08,0x30,0x00,
 0x14,0x1E,0x22,0x5A,0x04,0x08,0x30,0x00,0x0C,0x70,0x10,0xFE,0x10,0x10,0x20,0x00,0x0A,0x70,0x10,0xFE,0x10,0x10,0x20,0x00,
 0x00,0x00,0x00,0x54,0x54,0x08,0x30,0x00,0x00,0xA2,0x52,0x52,0x04,0x08,0x30,0x00,0x0A,0xA0,0x52,0x52,0x04,0x08,0x30,0x00,
 0x7C,0x00,0xFE,0x10,0x10,0x10,0x20,0x00,0x7A,0x00,0xFE,0x10,0x10,0x10,0x20,0x00,0x20,0x20,0x20,0x38,0x24,0x20,0x20,0x00,
 0x2A,0x20,0x20,0x38,0x24,0x20,0x20,0x00,0x10,0x10,0xFE,0x10,0x10,0x20,0x40,0x00,0x00,0x7C,0x00,0x00,0x00,0xFE,0x00,0x00,
 0x7C,0x04,0x34,0x08,0x18,0x24,0xC0,0x00,0x10,0x7C,0x08,0x10,0x34,0xD2,0x10,0x00,0x04,0x04,0x04,0x08,0x10,0x20,0xC0,0x00,
 0x00,0x28,0x24,0x24,0x42,0x42,0x82,0x00,0x0A,0x20,0x24,0x24,0x42,0x42,0x82,0x00,0x04,0x2A,0x24,0x24,0x42,0x42,0x82,0x00,
 0x40,0x40,0x46,0x78,0x40,0x40,0x3E,0x00,0x4A,0x40,0x46,0x78,0x40,0x40,0x3E,0x00,0x44,0x4A,0x46,0x78,0x40,0x40,0x3E,0x00,
 0x00,0x7E,0x02,0x02,0x04,0x08,0x30,0x00,0x0A,0x7E,0x02,0x02,0x04,0x08,0x30,0x00,0x04,0x7A,0x06,0x02,0x04,0x08,0x30,0x00,
 0x00,0x20,0x50,0x88,0x04,0x02,0x00,0x00,0x0A,0x20,0x50,0x88,0x04,0x02,0x00,0x00,0x04,0x2A,0x54,0x88,0x04,0x02,0x00,0x00,
 0x10,0x10,0xFE,0x10,0x54,0x92,0x30,0x00,0x1A,0x10,0xFE,0x10,0x54,0x92,0x30,0x00,0x14,0x1A,0xF4,0x10,0x54,0x92,0x30,0x00,
 0x00,0xFE,0x02,0x04,0x28,0x10,0x08,0x00,0x70,0x0C,0x20,0x18,0x00,0x70,0x0C,0x00,0x10,0x10,0x20,0x20,0x48,0x44,0xFA,0x00,
 0x04,0x04,0x74,0x08,0x14,0x24,0xC0,0x00,0x3C,0x10,0x10,0x7E,0x10,0x10,0x0E,0x00,0x00,0x00,0x20,0x3C,0x64,0x10,0x10,0x00,
 0x20,0x2E,0xF2,0x24,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x38,0x08,0x08,0x7C,0x00,0x00,0x78,0x08,0x08,0x08,0xFE,0x00,0x00,
 0x00,0x00,0x3C,0x04,0x1C,0x04,0x3C,0x00,0x00,0x7E,0x02,0x3E,0x02,0x02,0x7E,0x00,0x3C,0x00,0x7E,0x02,0x04,0x08,0x30,0x00,
 0x44,0x44,0x44,0x44,0x04,0x08,0x30,0x00,0x08,0x28,0x28,0x28,0x2A,0x4C,0x88,0x00,0x20,0x20,0x20,0x22,0x24,0x28,0x30,0x00,
 0x00,0x7E,0x42,0x42,0x42,0x42,0x7E,0x00,0x00,0x00,0x7C,0x44,0x04,0x08,0x30,0x00,0x00,0x7E,0x42,0x02,0x04,0x08,0x30,0x00,
 0x04,0x7E,0x24,0x24,0xFE,0x04,0x04,0x00,0x00,0x7C,0x08,0x10,0x10,0xFE,0x00,0x00,0x7E,0x02,0x3E,0x02,0x04,0x08,0x30,0x00,
 0x40,0x20,0x02,0x02,0x04,0x08,0x70,0x00,0x14,0x7E,0x42,0x02,0x04,0x08,0x30,0x00,0x00,0x00,0x10,0x7C,0x14,0x24,0x4C,0x00,
 0x00,0x00,0x20,0x3C,0x48,0x08,0x10,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00,0x00,0x80,0x7E,0x00,0x00,0x00,0x00,
 0x00,0x00,0x20,0x10,0x08,0x08,0x00,0x00,0x0A,0x00,0x20,0x10,0x08,0x08,0x00,0x00,0x00,0x00,0x00,0x40,0xA0,0x40,0x00,0x00,
 0xE0,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x40,0x00,0x00,
 0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0xE0,0x20,0xE0,0x20,0x40,0x80,0x00,0x00,0x00,0x00,0xE0,0x60,0x40,0x80,0x00,0x00,
 0x00,0x00,0x20,0x40,0xC0,0x40,0x00,0x00,0x00,0x00,0x40,0xE0,0xA0,0x20,0x00,0x00,0x00,0x00,0x00,0xE0,0x40,0xE0,0x00,0x00,
 0x00,0x00,0x20,0xE0,0x60,0xA0,0x00,0x00,0x00,0x00,0x40,0xE0,0x60,0x40,0x00,0x00,0x00,0x00,0x00,0xC0,0x40,0xE0,0x00,0x00,
 0x00,0x00,0xE0,0x60,0x20,0xE0,0x00,0x00,0x00,0x00,0xE0,0xE0,0x20,0xC0,0x00,0x00,0x00,0x00,0x80,0x60,0x00,0x00,0x00,0x00,
 0xE0,0x20,0x60,0x40,0x40,0x80,0x00,0x00,0x20,0x20,0x40,0xC0,0x40,0x40,0x00,0x00,0x40,0xE0,0xA0,0x20,0x20,0x40,0x00,0x00,
 0x00,0xE0,0x40,0x40,0x40,0xE0,0x00,0x00,0x20,0xE0,0x20,0x60,0xA0,0x20,0x00,0x00,0x40,0xE0,0x60,0x60,0xA0,0xA0,0x00,0x00,
 0x40,0xE0,0x40,0xE0,0x40,0x40,0x00,0x00,0x40,0x60,0xA0,0x20,0x40,0x80,0x00,0x00,0x80,0xE0,0xC0,0x40,0x40,0x80,0x00,0x00,
 0x00,0xE0,0x20,0x20,0x20,0xE0,0x00,0x00,0xA0,0xE0,0xA0,0x20,0x40,0x80,0x00,0x00,0xC0,0x00,0xC0,0x00,0x20,0xC0,0x00,0x00,
 0xE0,0x20,0x20,0x40,0x40,0xA0,0x00,0x00,0x80,0xE0,0xA0,0xA0,0x80,0xE0,0x00,0x00,0xA0,0x60,0x60,0x20,0x40,0x80,0x00,0x00,
 0x40,0x60,0xA0,0x60,0x40,0x80,0x00,0x00,0xE0,0x40,0xE0,0x40,0x40,0x80,0x00,0x00,0x00,0xE0,0xE0,0x20,0x20,0xC0,0x00,0x00,
 0xE0,0x00,0xE0,0x40,0x40,0x80,0x00,0x00,0x80,0x80,0xC0,0xA0,0x80,0x80,0x00,0x00,0x40,0x40,0xE0,0x40,0x40,0x80,0x00,0x00,
 0x00,0x60,0x00,0x00,0x00,0xE0,0x00,0x00,0xE0,0x20,0xA0,0x40,0x40,0xA0,0x00,0x00,0x40,0xE0,0x20,0x40,0xE0,0x40,0x00,0x00,
 0x20,0x20,0x20,0x20,0x40,0x80,0x00,0x00,0x40,0x20,0xA0,0xA0,0xA0,0xA0,0x00,0x00,0x80,0x80,0xE0,0x80,0x80,0x60,0x00,0x00,
 0xE0,0x20,0x20,0x20,0x40,0x80,0x00,0x00,0x40,0x40,0xA0,0xA0,0x20,0x00,0x00,0x00,0x40,0xE0,0x40,0xE0,0xE0,0x40,0x00,0x00,
 0xE0,0x20,0x20,0xC0,0x40,0x20,0x00,0x00,0x00,0xE0,0x00,0x60,0x00,0xE0,0x00,0x00,0x40,0x40,0x40,0x80,0xA0,0xE0,0x00,0x00,
 0x20,0xA0,0x40,0x40,0xA0,0x80,0x00,0x00,0xE0,0x40,0xE0,0x40,0x40,0x60,0x00,0x00,0x40,0xE0,0x60,0x60,0x40,0x40,0x00,0x00,
 0xC0,0x40,0x40,0x40,0x40,0xE0,0x00,0x00,0xE0,0x20,0xE0,0x20,0x20,0xE0,0x00,0x00,0xE0,0x00,0xE0,0x20,0x20,0xC0,0x00,0x00,
 0xA0,0xA0,0xA0,0x20,0x20,0x40,0x00,0x00,0x40,0x40,0xC0,0xC0,0xC0,0xE0,0x00,0x00,0x80,0x80,0x80,0x80,0xA0,0xC0,0x00,0x00,
 0x00,0xE0,0xA0,0xA0,0xA0,0xE0,0x00,0x00,0xE0,0xA0,0x20,0x20,0x40,0x80,0x00,0x00,0xC0,0x00,0x00,0x20,0x20,0xC0,0x00,0x00,
 0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0xA0,0x40,0x00,0x00,0x00,0x00,0x00,0x10,0x28,0x28,0x44,0x7C,0x82,0x82,0x00,
 0x7C,0x42,0x42,0x7C,0x42,0x42,0x7C,0x00,0x7E,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x10,0x28,0x28,0x44,0x44,0x82,0xFE,0x00,
 0x7E,0x40,0x40,0x7C,0x40,0x40,0x7E,0x00,0x7E,0x02,0x04,0x08,0x10,0x20,0x7E,0x00,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x00,
 0x38,0x44,0x82,0xBA,0x82,0x44,0x38,0x00,0x38,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x42,0x44,0x48,0x50,0x68,0x44,0x42,0x00,
 0x10,0x28,0x28,0x44,0x44,0x82,0x82,0x00,0x82,0xC6,0xAA,0xAA,0x92,0x92,0x82,0x00,0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0x00,
 0x7E,0x00,0x00,0x3C,0x00,0x00,0x7E,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x7E,0x42,0x42,0x42,0x42,0x42,0x42,0x00,
 0x7C,0x42,0x42,0x7C,0x40,0x40,0x40,0x00,0x7E,0x20,0x10,0x08,0x10,0x20,0x7E,0x00,0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x00,
 0x82,0x44,0x28,0x10,0x10,0x10,0x10,0x00,0x10,0x7C,0x92,0x92,0x92,0x7C,0x10,0x00,0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x00,
 0x92,0x54,0x54,0x38,0x10,0x10,0x10,0x00,0x38,0x44,0x82,0x82,0x44,0x28,0xEE,0x00,0x00,0x00,0x1A,0x24,0x48,0x4A,0x34,0x00,
 0x0C,0x12,0x2C,0x22,0x32,0x4C,0x40,0x00,0x00,0x00,0x64,0x18,0x10,0x30,0x20,0x00,0x18,0x24,0x10,0x28,0x48,0x48,0x30,0x00,
 0x00,0x00,0x18,0x24,0x30,0x44,0x38,0x00,0x2C,0x10,0x20,0x20,0x18,0x04,0x18,0x00,0x00,0x00,0x2C,0x52,0x12,0x24,0x04,0x00,
 0x18,0x24,0x24,0x7C,0x48,0x48,0x30,0x00,0x00,0x00,0x10,0x10,0x20,0x28,0x10,0x00,0x00,0x00,0x24,0x28,0x30,0x50,0x4C,0x00,
 0x10,0x08,0x08,0x18,0x28,0x28,0x44,0x00,0x00,0x00,0x12,0x12,0x24,0x3A,0x40,0x00,0x00,0x00,0x44,0x24,0x28,0x30,0x20,0x00,
 0x1C,0x20,0x3C,0x40,0x38,0x04,0x18,0x00,0x00,0x00,0x18,0x24,0x44,0x48,0x30,0x00,0x00,0x00,0x3E,0x54,0x14,0x24,0x46,0x00,
 0x00,0x00,0x1C,0x22,0x22,0x5C,0x40,0x00,0x00,0x00,0x1E,0x24,0x44,0x48,0x30,0x00,0x00,0x00,0x3E,0x48,0x10,0x14,0x08,0x00,
 0x00,0x00,0x32,0x52,0x22,0x24,0x18,0x00,0x04,0x04,0x3C,0x4A,0x4A,0x3C,0x10,0x00,0x00,0x00,0x34,0x08,0x10,0x30,0x4C,0x00,
 0x04,0x04,0x68,0x2A,0x4A,0x3C,0x10,0x00,0x00,0x00,0x22,0x4A,0x4A,0x5A,0x24,0x00,0x24,0x7E,0x40,0x7C,0x40,0x40,0x7E,0x00,
 0x10,0x28,0x28,0x44,0x7C,0x82,0x82,0x00,0x7E,0x40,0x40,0x7C,0x42,0x42,0x7C,0x00,0x7C,0x42,0x42,0x7C,0x42,0x42,0x7C,0x00,
 0x7E,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x3C,0x24,0x24,0x24,0x44,0xFE,0x82,0x00,0x7E,0x40,0x40,0x7C,0x40,0x40,0x7E,0x00,
 0x92,0x54,0x54,0x38,0x54,0x54,0x92,0x00,0x3C,0x42,0x02,0x1C,0x02,0x42,0x3C,0x00,0x42,0x42,0x46,0x4A,0x52,0x62,0x42,0x00,
 0x24,0x5A,0x46,0x4A,0x52,0x62,0x42,0x00,0x46,0x48,0x48,0x70,0x48,0x44,0x42,0x00,0x3E,0x22,0x22,0x22,0x22,0x22,0xC2,0x00,
 0x44,0x44,0x44,0xAA,0xAA,0x92,0x92,0x00,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,
 0x7E,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x7C,0x42,0x42,0x7C,0x40,0x40,0x40,0x00,0x3C,0x42,0x40,0x40,0x40,0x42,0x3C,0x00,
 0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x42,0x24,0x24,0x18,0x08,0x10,0x60,0x00,0x10,0x7C,0x92,0x92,0x92,0x7C,0x10,0x00,
 0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x00,0x44,0x44,0x44,0x44,0x44,0x7E,0x02,0x00,0x42,0x42,0x42,0x42,0x3E,0x02,0x02,0x00,
 0x92,0x92,0x92,0x92,0x92,0x92,0xFE,0x00,0x92,0x92,0x92,0x92,0x92,0xFE,0x02,0x00,0x60,0x20,0x20,0x3C,0x22,0x22,0x3C,0x00,
 0x42,0x42,0x42,0x72,0x4A,0x4A,0x72,0x00,0x20,0x20,0x20,0x3C,0x22,0x22,0x3C,0x00,0x3C,0x42,0x02,0x3E,0x02,0x42,0x3C,0x00,
 0x9C,0xA2,0xA2,0xE2,0xA2,0xA2,0x9C,0x00,0x3E,0x42,0x42,0x3E,0x22,0x22,0x42,0x00,0x00,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,
 0x1C,0x20,0x78,0x44,0x44,0x44,0x38,0x00,0x00,0x00,0x78,0x44,0x78,0x44,0x78,0x00,0x00,0x00,0x7C,0x40,0x40,0x40,0x40,0x00,
 0x00,0x00,0x38,0x28,0x28,0x7C,0x44,0x00,0x00,0x00,0x38,0x44,0x7C,0x40,0x38,0x00,0x00,0x00,0x92,0x54,0x38,0x54,0x92,0x00,
 0x00,0x00,0x38,0x44,0x18,0x44,0x38,0x00,0x00,0x00,0x44,0x4C,0x54,0x64,0x44,0x00,0x28,0x10,0x44,0x4C,0x54,0x64,0x44,0x00,
 0x00,0x00,0x4C,0x50,0x70,0x48,0x44,0x00,0x00,0x00,0x3C,0x24,0x24,0x24,0xC4,0x00,0x00,0x00,0x44,0x6C,0x54,0x54,0x44,0x00,
 0x00,0x00,0x44,0x44,0x7C,0x44,0x44,0x00,0x00,0x00,0x38,0x44,0x44,0x44,0x38,0x00,0x00,0x00,0x7C,0x44,0x44,0x44,0x44,0x00,
 0x00,0x00,0x78,0x44,0x78,0x40,0x40,0x00,0x00,0x00,0x38,0x44,0x40,0x44,0x38,0x00,0x00,0x00,0x7C,0x10,0x10,0x10,0x10,0x00,
 0x00,0x00,0x44,0x28,0x28,0x10,0x60,0x00,0x10,0x10,0x38,0x54,0x54,0x38,0x10,0x00,0x00,0x00,0x44,0x28,0x10,0x28,0x44,0x00,
 0x00,0x00,0x48,0x48,0x48,0x7C,0x04,0x00,0x00,0x00,0x44,0x44,0x44,0x3C,0x04,0x00,0x00,0x00,0x54,0x54,0x54,0x54,0x7C,0x00,
 0x00,0x00,0x54,0x54,0x54,0x7E,0x02,0x00,0x00,0x00,0x60,0x20,0x3C,0x22,0x3C,0x00,0x00,0x00,0x44,0x44,0x74,0x4C,0x74,0x00,
 0x00,0x00,0x40,0x40,0x78,0x44,0x78,0x00,0x00,0x00,0x38,0x44,0x1C,0x44,0x38,0x00,0x00,0x00,0x4C,0x52,0x72,0x52,0x4C,0x00,
 0x00,0x00,0x3C,0x44,0x3C,0x24,0x44,0x00,0x28,0x00,0x38,0x44,0x7C,0x40,0x38,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x00,
 0x04,0x08,0x0C,0x00,0x00,0x00,0x00,0x00,0x60,0x20,0x40,0x00,0x00,0x00,0x00,0x00,0x24,0x48,0x6C,0x00,0x00,0x00,0x00,0x00,
 0x6C,0x24,0x48,0x00,0x00,0x00,0x00,0x00,0x10,0x38,0x10,0x10,0x10,0x10,0x10,0x00,0x10,0x38,0x10,0x10,0x10,0x38,0x10,0x00,
 0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x92,0x00,0x00,0x00,0x00,0x44,0xA8,0x50,0x20,0x54,0xAA,0x14,0x00,
 0x20,0x20,0x40,0x00,0x00,0x00,0x00,0x00,0x28,0x28,0x50,0x00,0x00,0x00,0x00,0x00,0x92,0x44,0x28,0x92,0x28,0x44,0x92,0x00,
 0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0xAC,0x52,0x10,0x10,0x12,0x0C,0x00,0x90,0x90,0xD4,0xBA,0x9A,0x9A,0x94,0x00,
 0xE2,0x42,0x4A,0x56,0x5A,0x52,0x4A,0x00,0x10,0x28,0x10,0x28,0x44,0x7C,0x82,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,
 0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x00,0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x00,0xA2,0xA2,0x94,0x94,0x94,0x88,0x88,0x00,
 0x44,0x44,0x28,0x28,0x28,0x10,0x10,0x00,0x8A,0x8A,0x52,0x52,0x52,0x22,0x22,0x00,0xAA,0xAA,0xAA,0xAA,0xAA,0x4A,0x4A,0x00,
 0xBA,0xBA,0xBA,0xBA,0x5A,0x5A,0x5A,0x00,0xA2,0x94,0x94,0x88,0x94,0x94,0xA2,0x00,0x44,0x28,0x28,0x10,0x28,0x28,0x44,0x00,
 0x00,0x20,0x40,0xFE,0x40,0x20,0x00,0x00,0x10,0x38,0x54,0x10,0x10,0x10,0x10,0x00,0x00,0x08,0x04,0xFE,0x04,0x08,0x00,0x00,
 0x10,0x10,0x10,0x10,0x54,0x38,0x10,0x00,0x00,0x08,0xFC,0x02,0xFC,0x08,0x00,0x00,0x00,0x28,0x7C,0x82,0x7C,0x28,0x00,0x00,
 0x82,0x82,0x7C,0x44,0x28,0x28,0x10,0x00,0x18,0x24,0x04,0x34,0x48,0x48,0x30,0x00,0x7E,0x02,0x02,0x7E,0x02,0x02,0x7E,0x00,
 0xFE,0x82,0x44,0x44,0x28,0x28,0x10,0x00,0x00,0x3E,0x40,0x7E,0x40,0x3E,0x00,0x00,0x00,0x7C,0x02,0x7E,0x02,0x7C,0x00,0x00,
 0x7E,0x20,0x10,0x08,0x10,0x20,0x7E,0x00,0x1E,0x10,0x20,0x20,0xE0,0x40,0x40,0x00,0x00,0x6C,0x92,0x90,0x92,0x6C,0x00,0x00,
 0x00,0x6C,0x92,0x92,0x92,0x6C,0x00,0x00,0x00,0x40,0x40,0x40,0x40,0x40,0x7E,0x00,0x00,0x02,0x04,0x08,0x10,0x20,0x7E,0x00,
 0x14,0x14,0x28,0x28,0x28,0x50,0x50,0x00,0x10,0x10,0x28,0x28,0x44,0x44,0x82,0x00,0x82,0x44,0x44,0x28,0x28,0x10,0x10,0x00,
 0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x00,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,0x18,0x20,0x20,0x10,0x08,0x08,0x30,0x00,
 0x36,0x48,0x48,0x24,0x12,0x12,0x6C,0x00,0x18,0x20,0x38,0x54,0x38,0x08,0x30,0x00,0x00,0x10,0x00,0x00,0x00,0x44,0x00,0x00,
 0x00,0x44,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x4C,0x92,0x92,0x92,0x64,0x00,0x00,0x40,0x00,0xFE,0x00,0xFE,0x00,0x04,0x00,
 0x00,0x04,0xFE,0x10,0xFE,0x40,0x00,0x00,0x00,0x7E,0x00,0x7E,0x00,0x7E,0x00,0x00,0x06,0x38,0xC0,0x38,0x06,0x00,0xFE,0x00,
 0xC0,0x38,0x06,0x38,0xC0,0x00,0xFE,0x00,0x00,0x24,0x48,0x90,0x48,0x24,0x00,0x00,0x00,0x48,0x24,0x12,0x24,0x48,0x00,0x00,
 0x00,0x3E,0x40,0x40,0x40,0x3E,0x00,0x00,0x00,0x7C,0x02,0x02,0x02,0x7C,0x00,0x00,0x3E,0x40,0x40,0x40,0x3E,0x00,0x7E,0x00,
 0x7C,0x02,0x02,0x02,0x7C,0x00,0x7E,0x00,0x00,0x10,0x10,0x10,0x10,0x10,0xFE,0x00,0x00,0x02,0x06,0x0A,0x12,0x22,0x7E,0x00,
 0x7C,0x82,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x92,0xB2,0x92,0x92,0x92,0x7C,0x00,0x7C,0xB2,0x8A,0x92,0xA2,0xBA,0x7C,0x00,
 0x7C,0xB2,0x8A,0x92,0x8A,0xB2,0x7C,0x00,0x7C,0x8A,0x9A,0xAA,0xBA,0x8A,0x7C,0x00,0x7C,0xBA,0xA2,0xB2,0x8A,0xB2,0x7C,0x00,
 0x7C,0x9A,0xA2,0xB2,0xAA,0x92,0x7C,0x00,0x7C,0xBA,0x8A,0x92,0x92,0x92,0x7C,0x00,0x7C,0x92,0xAA,0x92,0xAA,0x92,0x7C,0x00,
 0x7C,0x92,0xAA,0x9A,0x8A,0xB2,0x7C,0x00,0x7C,0xA2,0xEE,0xAA,0xAE,0xA2,0x7C,0x00,0x7C,0xA6,0xEE,0xA6,0xA6,0xA6,0x7C,0x00,
 0x7C,0xAA,0xE6,0xAE,0xAA,0xAE,0x7C,0x00,0x7C,0xAA,0xE6,0xAA,0xA6,0xAA,0x7C,0x00,0x7C,0xAA,0xEA,0xAE,0xAE,0xA6,0x7C,0x00,
 0x7C,0xAE,0xEA,0xAE,0xA6,0xAA,0x7C,0x00,0x7C,0xA6,0xEA,0xAE,0xAE,0xAE,0x7C,0x00,0x7C,0xAE,0xE6,0xAA,0xAA,0xAA,0x7C,0x00,
 0x7C,0xAE,0xEE,0xA2,0xAE,0xAE,0x7C,0x00,0x7C,0xAE,0xEE,0xAE,0xA6,0xAA,0x7C,0x00,0x7C,0xE2,0xAE,0xEA,0xCE,0xE2,0x7C,0x00,
 0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x1F,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0x1F,0x1F,0x18,0x18,0x18,
 0x00,0x00,0x00,0xF0,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0xF8,0xF8,0x18,0x18,0x18,0x10,0x10,0x10,0x1F,0x00,0x00,0x00,0x00,
 0x18,0x18,0x18,0x1F,0x1F,0x00,0x00,0x00,0x10,0x10,0x10,0xF0,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0xF8,0xF8,0x00,0x00,0x00,
 0x10,0x10,0x10,0x1F,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0x1F,0x10,0x10,0x10,0x18,0x18,0x18,0x1F,0x18,0x18,0x18,0x18,
 0x18,0x18,0x18,0x1F,0x1F,0x18,0x18,0x18,0x10,0x10,0x10,0xF0,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0xF0,0xF0,0x10,0x10,0x10,
 0x18,0x18,0x18,0xF8,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0xF8,0xF8,0x18,0x18,0x18,0x00,0x00,0x00,0xFF,0x10,0x10,0x10,0x10,
 0x00,0x00,0x00,0xFF,0xFF,0x10,0x10,0x10,0x00,0x00,0x00,0xFF,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0xFF,0xFF,0x18,0x18,0x18,
 0x10,0x10,0x10,0xFF,0x00,0x00,0x00,0x00,0x10,0x10,0x10,0xFF,0xFF,0x00,0x00,0x00,0x18,0x18,0x18,0xFF,0x00,0x00,0x00,0x00,
 0x18,0x18,0x18,0xFF,0xFF,0x00,0x00,0x00,0x10,0x10,0x10,0xFF,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0xFF,0xFF,0x10,0x10,0x10,
 0x18,0x18,0x18,0xFF,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0xFF,0xFF,0x18,0x18,0x18,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0x00,
 0xFE,0x82,0x82,0x82,0x82,0x82,0xFE,0x00,0x10,0x10,0x38,0x38,0x7C,0x7C,0xFE,0x00,0x10,0x10,0x28,0x28,0x44,0x44,0xFE,0x00,
 0xFE,0x7C,0x7C,0x38,0x38,0x10,0x10,0x00,0xFE,0x44,0x44,0x28,0x28,0x10,0x10,0x00,0x10,0x38,0x7C,0xFE,0x7C,0x38,0x10,0x00,
 0x10,0x28,0x44,0x82,0x44,0x28,0x10,0x00,0x38,0x44,0x82,0x82,0x82,0x44,0x38,0x00,0x38,0x44,0xBA,0xAA,0xBA,0x44,0x38,0x00,
 0x38,0x7C,0xFE,0xFE,0xFE,0x7C,0x38,0x00,0x7C,0x82,0x82,0x82,0x82,0x82,0x7C,0x00,0x10,0x10,0xFE,0x38,0x38,0x6C,0x44,0x00,
 0x10,0x10,0xFE,0x28,0x38,0x6C,0x44,0x00,0x38,0x44,0x44,0x38,0x10,0x7C,0x10,0x00,0x1E,0x06,0x0A,0x72,0x90,0x90,0x60,0x00,
 0x10,0x18,0x14,0x14,0x30,0x70,0x60,0x00,0x20,0x20,0x28,0x34,0x24,0x28,0x30,0x00,0x04,0x26,0x3C,0x66,0x3C,0x64,0x20,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x40,0x00,0x00,0x00,0x00,0x00,0x40,0xA0,0x40,0x00,
 0x00,0x00,0x24,0x24,0x24,0x48,0x00,0x00,0x00,0x20,0x3C,0x44,0xA8,0x10,0x08,0x00,0x02,0x34,0x48,0x94,0xA4,0xC0,0x80,0x00,
 0x00,0x38,0x44,0x82,0x82,0x44,0x38,0x00,0x04,0x08,0x10,0x20,0x10,0x08,0x04,0x00,0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00,
 0x0A,0x14,0x28,0x50,0x28,0x14,0x0A,0x00,0xA0,0x50,0x28,0x14,0x28,0x50,0xA0,0x00,0x1E,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
 0x00,0x10,0x10,0x10,0x10,0x10,0xF0,0x00,0x3E,0x22,0x2E,0x28,0x28,0x38,0x00,0x00,0x00,0x38,0x28,0x28,0xE8,0x88,0xF8,0x00,
 0x1C,0x18,0x10,0x10,0x10,0x18,0x1C,0x00,0x70,0x30,0x10,0x10,0x10,0x30,0x70,0x00,0xFE,0x00,0xFE,0x10,0x10,0x10,0x10,0x00,
 0x00,0xFE,0xFE,0x00,0xFE,0xFE,0x00,0x00,0x0C,0x30,0x20,0x20,0x20,0x30,0x0C,0x00,0x60,0x18,0x08,0x08,0x08,0x18,0x60,0x00,
 0x00,0x00,0x60,0x92,0x0C,0x00,0x00,0x00,0x14,0x14,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xA0,0x50,0x50,0x00,
 0x44,0xF2,0xBE,0xEA,0xFE,0xAA,0x6C,0x00,0x54,0xBA,0x92,0xBE,0xD6,0x9E,0x54,0x00,0x44,0xAA,0xBE,0xEA,0xAA,0xA6,0x44,0x00,
 0x7C,0x92,0x9A,0x92,0x92,0xBA,0x7C,0x00,0x7C,0x92,0xFE,0xD6,0xFE,0x92,0x7C,0x00,0x7C,0xBA,0x92,0x9A,0x92,0x92,0x7C,0x00,
 0x7C,0xFA,0xA2,0xBA,0xD2,0xBA,0x7C,0x00,0x7C,0xA2,0xFA,0xA2,0xDA,0x9A,0x7C,0x00,0xF0,0x56,0x60,0x80,0x60,0x60,0xB0,0x00,
 0xEE,0x6A,0xAE,0x00,0xA0,0xAE,0x20,0x00,0x40,0xE0,0x40,0xEE,0x4A,0x0A,0x0E,0x00,0x76,0xAE,0x22,0xCC,0x20,0x50,0xF0,0x00,
 0x8C,0xE0,0xA2,0xCC,0x40,0xE0,0x40,0x00,0x8C,0xE0,0xA2,0xCC,0x40,0x60,0x40,0x00,0xA8,0x80,0xE0,0x80,0x14,0x14,0x26,0x00,
 0x80,0x80,0xE0,0x8C,0x00,0x02,0x0C,0x00,0x70,0xB6,0xA0,0x00,0x94,0xC6,0xDC,0x00,0x46,0xAA,0x02,0x6C,0xBC,0x4C,0x96,0x00,
 0x50,0xA6,0x10,0xC0,0xD0,0x20,0xC0,0x00,0xE0,0x00,0x60,0x0A,0xEA,0x02,0x04,0x00,0xEA,0x6A,0x72,0x00,0x6C,0xBC,0xB6,0x00,
 0xA0,0x4E,0xA0,0x00,0x8C,0xCC,0x96,0x00,0xA0,0xAE,0x22,0x00,0x8C,0xCC,0x96,0x00,0xE0,0xAE,0x22,0x40,0x40,0x60,0x40,0x00,
 0xE4,0x4E,0xEC,0x4C,0xEE,0x4C,0x52,0x00,0x6C,0xE6,0xEE,0xA6,0x8E,0x6C,0x64,0x00,0x4E,0x44,0xE4,0x4E,0x4C,0xAC,0xAE,0x00,
 0x6A,0xE4,0xEC,0xE6,0x68,0x6E,0xAE,0x00,0x00,0x00,0xC6,0xEA,0xE6,0xE2,0xAC,0x00,0x80,0x80,0xA6,0xAA,0xC6,0xA2,0xAC,0x00,
 0x00,0x00,0xCC,0xEE,0xEE,0xEE,0xAA,0x00,0x00,0x00,0x6C,0x8E,0x8E,0x8E,0x6A,0x00,0x80,0x80,0xAC,0xAE,0xCE,0xAE,0xAA,0x00,
 0x06,0x02,0xD4,0xAE,0xA8,0xA8,0xA8,0x00,0x00,0x00,0x66,0x88,0x88,0x88,0x66,0x00,0xAA,0xAA,0xAA,0xCC,0xAA,0xAA,0xAA,0x00,
 0x10,0x28,0xC6,0x7C,0x10,0x10,0xFE,0x00,0x10,0x10,0x10,0x10,0x10,0x00,0x10,0x00,0x28,0x28,0x00,0x00,0x00,0x00,0x00,0x00,
 0x14,0x7E,0x28,0x28,0x28,0xFC,0x50,0x00,0x08,0x3E,0x48,0x3C,0x12,0x7C,0x10,0x00,0x42,0xA4,0x48,0x10,0x24,0x4A,0x84,0x00,
 0x30,0x48,0x50,0x24,0x54,0x88,0x76,0x00,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x08,0x10,0x10,0x10,0x08,0x04,0x00,
 0x40,0x20,0x10,0x10,0x10,0x20,0x40,0x00,0x10,0x54,0x38,0x10,0x38,0x54,0x10,0x00,0x10,0x10,0x10,0xFE,0x10,0x10,0x10,0x00,
 0x00,0x00,0x00,0x00,0x60,0x20,0x40,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,
 0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x00,0x3C,0x42,0x46,0x5A,0x62,0x42,0x3C,0x00,0x10,0x30,0x10,0x10,0x10,0x10,0x38,0x00,
 0x3C,0x42,0x02,0x0C,0x30,0x40,0x7E,0x00,0x3C,0x42,0x02,0x1C,0x02,0x42,0x3C,0x00,0x04,0x0C,0x14,0x24,0x44,0x7E,0x04,0x00,
 0x7E,0x40,0x7C,0x42,0x02,0x42,0x3C,0x00,0x1C,0x20,0x40,0x7C,0x42,0x42,0x3C,0x00,0x7E,0x02,0x04,0x08,0x08,0x10,0x10,0x00,
 0x3C,0x42,0x42,0x3C,0x42,0x42,0x3C,0x00,0x3C,0x42,0x42,0x3E,0x02,0x04,0x38,0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x00,0x00,
 0x00,0x30,0x30,0x00,0x30,0x10,0x20,0x00,0x00,0x06,0x38,0xC0,0x38,0x06,0x00,0x00,0x00,0x00,0xFE,0x00,0xFE,0x00,0x00,0x00,
 0x00,0xC0,0x38,0x06,0x38,0xC0,0x00,0x00,0x3C,0x42,0x02,0x0C,0x10,0x00,0x10,0x00,0x38,0x44,0x9A,0xAA,0xB4,0x40,0x38,0x00,
 0x10,0x28,0x28,0x44,0x7C,0x82,0x82,0x00,0x7C,0x42,0x42,0x7C,0x42,0x42,0x7C,0x00,0x3C,0x42,0x40,0x40,0x40,0x42,0x3C,0x00,
 0x78,0x44,0x42,0x42,0x42,0x44,0x78,0x00,0x7E,0x40,0x40,0x7C,0x40,0x40,0x7E,0x00,0x7E,0x40,0x40,0x7C,0x40,0x40,0x40,0x00,
 0x3C,0x42,0x40,0x4E,0x42,0x42,0x3C,0x00,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x00,0x38,0x10,0x10,0x10,0x10,0x10,0x38,0x00,
 0x02,0x02,0x02,0x02,0x02,0x42,0x3C,0x00,0x42,0x44,0x48,0x50,0x68,0x44,0x42,0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0x00,
 0x82,0xC6,0xAA,0xAA,0x92,0x92,0x82,0x00,0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,
 0x7C,0x42,0x42,0x7C,0x40,0x40,0x40,0x00,0x3C,0x42,0x42,0x42,0x4A,0x44,0x3A,0x00,0x7C,0x42,0x42,0x7C,0x48,0x44,0x42,0x00,
 0x3C,0x42,0x40,0x3C,0x02,0x42,0x3C,0x00,0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,
 0x82,0x82,0x44,0x44,0x28,0x28,0x10,0x00,0x82,0x92,0x92,0xAA,0xAA,0x44,0x44,0x00,0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x00,
 0x82,0x44,0x28,0x10,0x10,0x10,0x10,0x00,0x7E,0x02,0x04,0x08,0x10,0x20,0x7E,0x00,0x1C,0x10,0x10,0x10,0x10,0x10,0x1C,0x00,
 0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x70,0x00,0x10,0x28,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,
 0x40,0x40,0x58,0x64,0x44,0x44,0x78,0x00,0x00,0x00,0x38,0x44,0x40,0x44,0x38,0x00,0x04,0x04,0x34,0x4C,0x44,0x44,0x3C,0x00,
 0x00,0x00,0x38,0x44,0x7C,0x40,0x38,0x00,0x0C,0x10,0x38,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0x3C,0x44,0x3C,0x04,0x38,0x00,
 0x40,0x40,0x58,0x64,0x44,0x44,0x44,0x00,0x10,0x00,0x10,0x10,0x10,0x10,0x10,0x00,0x08,0x00,0x08,0x08,0x08,0x48,0x30,0x00,
 0x20,0x20,0x24,0x28,0x30,0x28,0x24,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0x68,0x54,0x54,0x54,0x54,0x00,
 0x00,0x00,0x58,0x64,0x44,0x44,0x44,0x00,0x00,0x00,0x38,0x44,0x44,0x44,0x38,0x00,0x00,0x00,0x78,0x44,0x78,0x40,0x40,0x00,
 0x00,0x00,0x3C,0x44,0x3C,0x04,0x04,0x00,0x00,0x00,0x58,0x64,0x40,0x40,0x40,0x00,0x00,0x00,0x3C,0x40,0x38,0x04,0x78,0x00,
 0x00,0x20,0x78,0x20,0x20,0x24,0x18,0x00,0x00,0x00,0x44,0x44,0x44,0x4C,0x34,0x00,0x00,0x00,0x44,0x44,0x28,0x28,0x10,0x00,
 0x00,0x00,0x44,0x54,0x54,0x28,0x28,0x00,0x00,0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00,0x00,0x44,0x28,0x28,0x10,0x60,0x00,
 0x00,0x00,0x7C,0x08,0x10,0x20,0x7C,0x00,0x0C,0x10,0x10,0x20,0x10,0x10,0x0C,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,
 0x60,0x10,0x10,0x08,0x10,0x10,0x60,0x00,0x00,0x00,0x60,0x92,0x0C,0x00,0x00,0x00,0x04,0x3C,0x4A,0x48,0x52,0x3C,0x20,0x00,
 0x18,0x24,0x70,0x20,0x60,0x92,0x6C,0x00,0x00,0x00,0x00,0xFE,0x02,0x02,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x10,0x10,0x10,0x00,0x10,0x10,0x10,0x00,0x44,0x44,0x28,0x7C,0x10,0x7C,0x10,0x00,
};
#define X 0xFFFF
static const uint16_t alice_font_maps[803] = {
 0,1,X,2,3,4,5,X,X,X,6,7,X,8,9,10,X,X,11,X,12,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
 27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,
 66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,X,X,X,X,X,X,X,83,84,85,86,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
 16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,
 55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,X,X,X,X,86,87,88,89,0,1,
 2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,X,17,18,19,20,21,22,23,X,X,X,X,X,X,X,24,25,26,27,28,29,30,31,32,33,34,35,36,37,
 38,39,40,X,41,42,43,44,45,46,47,0,X,X,X,X,X,X,X,X,X,X,X,X,X,X,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,
 22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
 61,62,63,64,X,65,0,1,2,3,4,5,6,X,7,8,X,X,9,10,X,X,11,12,X,X,X,13,14,X,X,X,X,X,X,X,X,X,15,X,16,17,X,X,X,X,X,X,X,18,X,
 X,19,0,X,X,X,X,X,X,X,X,X,X,1,X,X,X,X,X,X,X,X,X,2,0,X,1,0,X,1,2,X,X,X,3,4,X,X,5,X,X,X,X,X,6,X,X,X,X,X,X,X,X,7,X,X,8,9,
 10,11,X,X,X,X,12,X,13,14,15,16,17,18,X,19,X,X,X,X,X,20,21,X,X,X,X,X,X,X,22,0,X,X,X,X,X,X,X,X,X,X,X,X,X,1,2,X,X,X,X,3,
 4,X,X,5,6,0,1,X,X,2,3,0,1,2,3,X,X,X,X,X,X,X,X,4,X,X,5,6,X,X,7,8,X,X,9,10,X,X,11,12,13,X,X,14,X,X,15,16,17,X,X,18,X,X,
 19,20,X,X,21,22,X,X,23,24,X,X,25,26,X,X,27,28,X,X,29,X,X,30,X,X,X,X,X,X,X,X,31,0,1,X,X,X,X,X,X,X,X,2,3,X,X,X,X,X,X,X,
 X,4,5,X,X,X,6,X,X,7,8,0,X,1,0,X,X,1,X,2,0,1,2,3,X,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,X,X,X,X,X,X,21,22,X,
 23,0,1,X,X,X,X,X,X,2,0,X,X,X,X,X,X,X,X,X,1,X,X,X,X,X,X,2,X,X,X,3,X,X,X,X,X,X,X,X,X,4,5,X,X,6,7,X,X,X,8,X,X,X,X,X,X,X,
 X,X,X,9,X,X,X,X,10,X,X,X,X,X,X,X,X,X,X,X,X,X,11,12,X,X,13,X,X,X,14,X,X,X,X,X,15,0,1,2,3,X,X,X,X,X,X,X,X,X,X,X,X,X,X,
 X,4,5,X,X,X,X,X,X,X,X,X,X,X,X,6,7,8,X,X,9,0,X,X,X,X,X,X,X,X,1,
};
#undef X
static const uint8_t alice_font_advances[33] = {
 4,4,4,4,8,8,4,8,4,8,8,8,8,8,4,4,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
};

#define R(f, n, w, g) {f, n, w, alice_font_pool + 8 * (g), NULL, NULL}
#define RM(f, n, w, g, m) {f, n, w, alice_font_pool + 8 * (g), alice_font_maps + (m), NULL}
#define RA(f, n, w, g, a) {f, n, w, alice_font_pool + 8 * (g), NULL, alice_font_advances + (a)}
#define RMA(f, n, w, g, m, a) {f, n, w, alice_font_pool + 8 * (g), alice_font_maps + (m), alice_font_advances + (a)}
static const alice_font_range alice_font_ranges[38] = {
    R(0x20, 95, 4, 0),
    RMA(0xa2, 21, 8, 95, 0, 0),
    R(0xd7, 1, 8, 108),
    R(0xf7, 1, 8, 109),
    RM(0x3041, 94, 8, 110, 21),
    RM(0x30a1, 94, 8, 197, 115),
    R(0xff61, 63, 4, 287),
    RM(0x391, 57, 8, 350, 209),
    RM(0x401, 81, 8, 398, 266),
    RMA(0x2010, 47, 8, 464, 347, 13),
    R(0x2103, 1, 8, 484),
    RM(0x2116, 22, 8, 485, 394),
    R(0x2160, 10, 8, 488),
    R(0x2190, 4, 8, 498),
    RM(0x21d2, 3, 8, 502, 416),
    RM(0x2200, 62, 8, 504, 419),
    RM(0x2252, 26, 8, 527, 481),
    RM(0x2282, 6, 8, 534, 507),
    R(0x22a5, 1, 8, 538),
    R(0x22bf, 1, 8, 539),
    R(0x2312, 1, 8, 540),
    R(0x2460, 20, 8, 541),
    RM(0x2500, 76, 8, 561, 513),
    R(0x25a0, 2, 8, 593),
    RM(0x25b2, 30, 8, 595, 589),
    R(0x25ef, 1, 8, 604),
    R(0x2605, 2, 8, 605),
    RM(0x2640, 3, 8, 607, 619),
    RM(0x266a, 6, 8, 609, 622),
    RM(0x3000, 32, 8, 612, 628),
    RM(0x3231, 9, 8, 636, 660),
    R(0x32a4, 5, 8, 639),
    RM(0x3303, 85, 8, 644, 669),
    RM(0x337b, 39, 8, 660, 754),
    RM(0x33c4, 10, 8, 670, 793),
    R(0x4edd, 1, 8, 672),
    R(0xff01, 94, 8, 673),
    R(0xffe0, 6, 8, 767),
};
#undef R
#undef RM
#undef RA
#undef RMA

static const alice_font alice_font_defs[4] = {
    { 8, alice_font_ranges,     38, '?', NULL },
    { 8, alice_font_ranges + 0,  4, '?', NULL },
    { 8, alice_font_ranges + 4,  1, '?', NULL },
    { 8, alice_font_ranges + 5,  2, '?', NULL },
};

#endif /* ALICE_TEXT_NO_BUILTIN_FONT */


#endif /* ALICE_TEXT_IMPLEMENTATION */