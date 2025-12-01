/**
 *   ___    __ _   _ __
 *  / __|  / _` | | '_ \
 * | (__  | (_| | | |_) |
 *  \___|  \__,_| | .__/
 *                |_|
 *
 *  cap.h — v1.0.0
 *
 *
 *  This file is placed in the public domain.
 *  See end of file for license details.
 *
 *
 *  Example usage:
 *
 *
 *  Customization:
 *    - Memory allocation: Define any of the following function-like macros to override default allocation method:
 *      - CAPDEF                      Prefixed to all API functions, consider `static inline` if single-TU. Default is nothing.
 *      - CAP_DEFAULT_WRAP_COLUMN     The default column for which to wrap lines in help menu and usage string. Default is 78.
 *                                    This value is overridden by a call to cap_wrap_column() for a specific context.
 *      - CAP_REALLOC(addr, bytes)    Must match signature and behavior of libc realloc. Default is libc realloc
 *      - CAP_FREE(addr)              Must match signature and behavior of libc free. Default is libc free
 *      - CAP_ASSERT(...)             Must match signature of libc assert. Default is libc assert.
 *      - CAP_IGNORE_NODISCARD        By default [[nodiscard]] is added to all API functions. These can be removed
 *                                    by adding this definition.
 *
 **/

/**
 * TODO:
 *  - Add support for custom type options. cap_option_custom(ctx, void *). Users will have
 *    to supply their own parsing function. If they want to display a default value
 *    they would also need to supply a formatting function for the type. Hmm, not sure,
 *    will see in the future.
 *  - Rename StringBuilder to String, use String for dynamic string types inside
 *    context and options. This should be done after refactoring object ownership
 *    and hiding option and context details from the user.
 *  - Add support for arbitrarily nested subcommands, with own options and positionals as well as shared (yikes).
 **/

#ifndef CAP_H_
#define CAP_H_

// CAPDEF controls linkage/visibility of API functions.
// Override before including cap.h if you want e.g. `static inline`
// for single-TU usage, or __declspec(dllexport) for building a Windows DLL
#ifndef CAPDEF
#define CAPDEF
#endif

#ifndef CAP_DEFAULT_WRAP_COLUMN
#define CAP_DEFAULT_WRAP_COLUMN 78
#endif

#ifndef CAP_ASSERT
#include <assert.h>
#define CAP_ASSERT(...) assert(__VA_ARGS__)
#endif

#if defined(CAP_REALLOC) || defined(CAP_FREE)
#if !defined(CAP_REALLOC)
#error "If you define CAP_FREE you must also define CAP_REALLOC"
#endif
#if !defined(CAP_FREE)
#error "If you define CAP_REALLOC you must also define CAP_FREE"
#endif
#else
#include <stdlib.h>
#define CAP_REALLOC(addr, new_size_bytes) realloc(addr, new_size_bytes)
#define CAP_FREE(addr) free(addr)
#endif

#include <stdbool.h>
#include <stddef.h>

#ifndef __cplusplus
#define CAP_NO_PARAMS void
#else
#define CAP_NO_PARAMS
#endif

#if defined(__cplusplus) &&  __cplusplus >= 201703L && !defined(CAP_IGNORE_NODISCARD)
#define CAP_NODISCARD [[nodiscard]]
#else
#define CAP_NODISCARD
#endif

#ifdef __cplusplus
#define CAP_NOEXCEPT noexcept
#else
#define CAP_NOEXCEPT
#endif

typedef enum {
    CAP_OK,
    CAP_HELP_REQUESTED,
    CAP_VERSION_REQUESTED,
    CAP_ERR_UNKNOWN_OPTION,
    CAP_ERR_INVALID_VALUE,
    CAP_ERR_MISSING_OPTION,
    CAP_ERR_MISSING_VALUE,
    CAP_ERR_NO_IDENTIFIER,
    CAP_ERR_UNEXPECTED_POSITIONAL,
    CAP_ERR_TOO_MANY_POSITIONALS,
    CAP_ERR_TOO_FEW_POSITIONALS,
    CAP_ERR_MISSING_REQUIRED
} CapResult;

typedef struct CapContext CapContext;

CAPDEF CapContext *cap_context_new(CAP_NO_PARAMS) CAP_NOEXCEPT;
CAPDEF void        cap_context_free(CapContext *context) CAP_NOEXCEPT;

CAP_NODISCARD CAPDEF struct CapStrPosBuilder *cap_positional_string(CapContext  *ctx,
                                                                    const char **out_value) CAP_NOEXCEPT;

CAP_NODISCARD CAPDEF struct CapVarPosBuilder *cap_variadic(CapContext   *ctx,
                                                           char       ***out_arr,
                                                           size_t       *out_len) CAP_NOEXCEPT;

CAP_NODISCARD CAPDEF struct CapFlagBuilder *cap_flag(CapContext *ctx,
                                                     bool       *out_value) CAP_NOEXCEPT;

CAP_NODISCARD CAPDEF struct CapIntOptBuilder *cap_option_int(CapContext *ctx,
                                                             int        *out_value) CAP_NOEXCEPT;

CAP_NODISCARD CAPDEF struct CapDoubleOptBuilder *cap_option_double(CapContext *ctx,
                                                                   double     *out_value) CAP_NOEXCEPT;

CAP_NODISCARD CAPDEF struct CapStrOptBuilder *cap_option_string(CapContext  *ctx,
                                                                const char **out_value) CAP_NOEXCEPT;

// Custom enum type option.
#define cap_option_enum(ctx, EnumType, p_out_value)                     \
    cap_option_enum_((ctx), (p_out_value), sizeof(EnumType), ((EnumType)-1 < 0))

CAP_NODISCARD CAPDEF struct CapEnumOptBuilder *cap_option_enum_(CapContext *ctx,
                                                                void       *out_value,
                                                                size_t      type_size,
                                                                bool        is_signed) CAP_NOEXCEPT;


// Changes how arguments after `--` are handled.
//
// By default, cap treats all arguments that appear after `--` as positional.
// This lets users pass positional arguments starting with `-` by putting them
// after the `--` terminator.
//
// Calling this function changes that behavior: `--` will now mark the end of
// all parsing done by cap. Every argument after `--` is returned to the caller
// untouched, and cap will not try to interpret them as options or positionals.
// This is useful if the caller wants to implement their own parsing logic or
// forward the remainder of the arguments to another tool.
//
// Trade-off: with this behavior enabled, positional arguments can no longer
// start with `-`, since cap won't process or disambiguate anything after `--`.
// The help menu syntax will reflect this restriction.
CAPDEF void cap_capture_remainder(CapContext   *context,
                                  int          *out_remainder_argc,
                                  char       ***out_remainder_argv,
                                  const char   *remainder_description) CAP_NOEXCEPT;


CAPDEF void cap_set_program_description(CapContext *ctx, const char *desc) CAP_NOEXCEPT;
CAPDEF void cap_set_program_version(CapContext *ctx, const char *version) CAP_NOEXCEPT;

CAPDEF void cap_set_help_wrap_column(CapContext *ctx, size_t wrap_column) CAP_NOEXCEPT;


// Parses argv without printing or handling any output.
// Returns CAP_OK on success, CAP_HELP_REQUESTED or CAP_VERSION_REQUESTED
// if those flags were encountered, or a CAP_ERR_* code on error.
// cap_parse never modifies argv or the strings it points to.
CAP_NODISCARD CAPDEF CapResult cap_parse(CapContext  *context,
                                         int          argc,
                                         char       **argv) CAP_NOEXCEPT;

// Parses argv and automatically prints help, version, or errors to stdout/stderr.
// Returns CAP_EXIT if help, version, or an error occurred (exit_code is set to
// EXIT_SUCCESS or EXIT_FAILURE). Returns CAP_KEEP_GOING on successful parse.
//
// Example:
//   int code;
//   if (cap_parse_and_handle(ctx, argc, argv, &code) == CAP_EXIT)
//       return code;
//
typedef enum { CAP_EXIT, CAP_KEEP_GOING } CapAction;
CAP_NODISCARD CAPDEF CapAction cap_parse_and_handle(CapContext  *context,
                                                    int          argc,
                                                    char       **argv,
                                                    int         *exit_code) CAP_NOEXCEPT;


CAP_NODISCARD CAPDEF const char *cap_program_version(CapContext *ctx) CAP_NOEXCEPT;
CAP_NODISCARD CAPDEF const char *cap_program_description(CapContext *ctx) CAP_NOEXCEPT;

// Getting values, only after parsing!
CAP_NODISCARD CAPDEF const char *cap_program(CapContext *ctx) CAP_NOEXCEPT;
CAP_NODISCARD CAPDEF const char *cap_usage(CapContext *context) CAP_NOEXCEPT;
CAP_NODISCARD CAPDEF const char *cap_help_menu(CapContext *context) CAP_NOEXCEPT;
CAP_NODISCARD CAPDEF const char *cap_error(CapContext *context) CAP_NOEXCEPT;




// Builder types
typedef struct CapStrPosBuilder* (*CapStrPosBuilderSetLabel)(const char*);
typedef struct CapStrPosBuilder* (*CapStrPosBuilderSetDescription)(const char*);
typedef void (*CapStrPosBuilderDone)(CAP_NO_PARAMS);
typedef struct CapStrPosBuilder {
    CapStrPosBuilderSetLabel       label;
    CapStrPosBuilderSetDescription description;
    CapStrPosBuilderDone           done;
} CapStrPosBuilder;

typedef struct CapVarPosBuilder* (*CapVarPosBuilderSetLabel)(const char*);
typedef struct CapVarPosBuilder* (*CapVarPosBuilderSetDescription)(const char*);
typedef struct CapVarPosBuilder* (*CapVarPosBuilderSetMin)(size_t);
typedef struct CapVarPosBuilder* (*CapVarPosBuilderSetMax)(size_t);
typedef void (*CapVarPosBuilderDone)(CAP_NO_PARAMS);
typedef struct CapVarPosBuilder {
    CapVarPosBuilderSetLabel       label;
    CapVarPosBuilderSetDescription description;
    CapVarPosBuilderSetMin         minimum;
    CapVarPosBuilderSetMax         maximum;
    CapVarPosBuilderDone           done;
} CapVarPosBuilder;

typedef struct CapFlagBuilder* (*CapFlagBuilderSetLongName)(const char*);
typedef struct CapFlagBuilder* (*CapFlagBuilderSetShortName)(char);
typedef struct CapFlagBuilder* (*CapFlagBuilderSetDescription)(const char*);
typedef struct CapFlagBuilder* (*CapFlagBuilderInvert)(CAP_NO_PARAMS);
typedef void (*CapFlagBuilderDone)(CAP_NO_PARAMS);
typedef struct CapFlagBuilder {
    CapFlagBuilderSetLongName    long_name;
    CapFlagBuilderSetShortName   short_name;
    CapFlagBuilderSetDescription description;
    CapFlagBuilderInvert         invert;
    CapFlagBuilderDone           done;
} CapFlagBuilder;

typedef struct CapIntOptBuilder* (*CapIntOptBuilderSetLongName)(const char*);
typedef struct CapIntOptBuilder* (*CapIntOptBuilderSetShortName)(char);
typedef struct CapIntOptBuilder* (*CapIntOptBuilderSetDescription)(const char*);
typedef struct CapIntOptBuilder* (*CapIntOptBuilderSetMetavar)(const char*);
typedef struct CapIntOptBuilder* (*CapIntOptBuilderSetDefaultValue)(int);
typedef struct CapIntOptBuilder* (*CapIntOptBuilderRequired)(CAP_NO_PARAMS);
typedef void (*CapIntOptBuilderDone)(CAP_NO_PARAMS);
typedef struct CapIntOptBuilder {
    CapIntOptBuilderSetLongName     long_name;
    CapIntOptBuilderSetShortName    short_name;
    CapIntOptBuilderSetDescription  description;
    CapIntOptBuilderSetMetavar      metavar;
    CapIntOptBuilderSetDefaultValue default_value;
    CapIntOptBuilderRequired        required;
    CapIntOptBuilderDone            done;
} CapIntOptBuilder;

typedef struct CapDoubleOptBuilder* (*CapDoubleOptBuilderSetLongName)(const char*);
typedef struct CapDoubleOptBuilder* (*CapDoubleOptBuilderSetShortName)(char);
typedef struct CapDoubleOptBuilder* (*CapDoubleOptBuilderSetDescription)(const char*);
typedef struct CapDoubleOptBuilder* (*CapDoubleOptBuilderSetMetavar)(const char*);
typedef struct CapDoubleOptBuilder* (*CapDoubleOptBuilderSetDefaultValue)(double);
typedef struct CapDoubleOptBuilder* (*CapDoubleOptBuilderRequired)(CAP_NO_PARAMS);
typedef void (*CapDoubleOptBuilderDone)(CAP_NO_PARAMS);
typedef struct CapDoubleOptBuilder {
    CapDoubleOptBuilderSetLongName     long_name;
    CapDoubleOptBuilderSetShortName    short_name;
    CapDoubleOptBuilderSetDescription  description;
    CapDoubleOptBuilderSetMetavar      metavar;
    CapDoubleOptBuilderSetDefaultValue default_value;
    CapDoubleOptBuilderRequired        required;
    CapDoubleOptBuilderDone            done;
} CapDoubleOptBuilder;

typedef struct CapStrOptBuilder* (*CapStrOptBuilderSetLongName)(const char*);
typedef struct CapStrOptBuilder* (*CapStrOptBuilderSetShortName)(char);
typedef struct CapStrOptBuilder* (*CapStrOptBuilderSetDescription)(const char*);
typedef struct CapStrOptBuilder* (*CapStrOptBuilderSetMetavar)(const char*);
typedef struct CapStrOptBuilder* (*CapStrOptBuilderSetDefaultValue)(const char*);
typedef struct CapStrOptBuilder* (*CapStrOptBuilderRequired)(CAP_NO_PARAMS);
typedef void (*CapStrOptBuilderDone)(CAP_NO_PARAMS);
typedef struct CapStrOptBuilder {
    CapStrOptBuilderSetLongName     long_name;
    CapStrOptBuilderSetShortName    short_name;
    CapStrOptBuilderSetDescription  description;
    CapStrOptBuilderSetMetavar      metavar;
    CapStrOptBuilderSetDefaultValue default_value;
    CapStrOptBuilderRequired        required;
    CapStrOptBuilderDone            done;
} CapStrOptBuilder;

typedef struct CapEnumOptBuilder* (*CapEnumOptBuilderSetLongName)(const char*);
typedef struct CapEnumOptBuilder* (*CapEnumOptBuilderSetShortName)(char);
typedef struct CapEnumOptBuilder* (*CapEnumOptBuilderSetDescription)(const char*);
typedef struct CapEnumOptBuilder* (*CapEnumOptBuilderSetMetavar)(const char*);
typedef struct CapEnumOptBuilder* (*CapEnumOptBuilderAddEntry)(int, const char*);
typedef struct CapEnumOptBuilder* (*CapEnumOptBuilderSetDefaultValue)(int);
typedef struct CapEnumOptBuilder* (*CapEnumOptBuilderRequired)(CAP_NO_PARAMS);
typedef struct CapEnumOptBuilder* (*CapEnumOptBuilderCaseInsensitive)(CAP_NO_PARAMS);
typedef void (*CapEnumOptBuilderDone)(CAP_NO_PARAMS);
typedef struct CapEnumOptBuilder {
    CapEnumOptBuilderSetLongName     long_name;
    CapEnumOptBuilderSetShortName    short_name;
    CapEnumOptBuilderSetDescription  description;
    CapEnumOptBuilderSetMetavar      metavar;
    CapEnumOptBuilderAddEntry        entry;
    CapEnumOptBuilderSetDefaultValue default_value;
    CapEnumOptBuilderRequired        required;
    CapEnumOptBuilderCaseInsensitive case_insensitive;
    CapEnumOptBuilderDone            done;
} CapEnumOptBuilder;


#ifdef CAP_IMPLEMENTATION

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#ifndef __cplusplus
#define CAP_ZERO_INIT {0}
#else
#define CAP_ZERO_INIT {}
#endif

#define CAP_UNUSED(x) (void)(x)

#define CAP_MIN(a, b) (a) < (b) ? (a) : (b)
#define CAP_MAX(a, b) (a) > (b) ? (a) : (b)

#define CAP_ARRAY_LENGTH(arr) sizeof((arr)) / sizeof((arr)[0])

#define CAP_SHIFT(arr, len) (CAP_ASSERT((len) > 0), (len)--, *(arr)++)

// Disable unwanted warnings for clang, gcc and msvc
#if defined(_MSC_VER)
#define CAP_DISABLE_DUMB_WARNINGS()   \
    __pragma(warning(push))           \
    __pragma(warning(disable: 4047 4090 4244))
#define CAP_ENABLE_DUMB_WARNINGS()    \
    __pragma(warning(pop))
#elif defined(__clang__)
#define CAP_DISABLE_DUMB_WARNINGS()   \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wint-conversion\"")    \
    _Pragma("clang diagnostic ignored \"-Wpointer-sign\"")
#define CAP_ENABLE_DUMB_WARNINGS()    \
    _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#  ifdef __cplusplus
#define CAP_DISABLE_DUMB_WARNINGS()   \
    _Pragma("GCC diagnostic push")
#  else
#define CAP_DISABLE_DUMB_WARNINGS()   \
    _Pragma("GCC diagnostic push")    \
    _Pragma("GCC diagnostic ignored   \"-Wint-conversion\"")    \
    _Pragma("GCC diagnostic ignored   \"-Wpointer-sign\"")
#  endif
#define CAP_ENABLE_DUMB_WARNINGS()    \
    _Pragma("GCC diagnostic pop")
#else
#define CAP_DISABLE_DUMB_WARNINGS()
#define CAP_ENABLE_DUMB_WARNINGS()
#endif

CAP_DISABLE_DUMB_WARNINGS()


// String view based on https://github.com/rsore/sv/releases/tag/v1.0.0

typedef struct {
    const char *begin;
    size_t      length;
} CapStringView;

#ifndef __cplusplus
#define CAP_SV_LIT(s) ((CapStringView){ (const char *)(s), sizeof(s) - 1 })
#else
#define CAP_SV_LIT(s) (CapStringView{ (const char *)(s), sizeof(s) - 1 })
#endif

static inline CapStringView
cap_sv_empty(CAP_NO_PARAMS)
{
    CapStringView result = CAP_ZERO_INIT;
    return result;
}

static inline CapStringView
cap_sv_from_cstr(const char *cstr)
{
    if (!cstr) return cap_sv_empty();

    CapStringView sv = cap_sv_empty();
    sv.begin  = cstr;
    sv.length = strlen(cstr);
    return sv;
}

static inline CapStringView
cap_sv_from_parts(const char *begin, size_t length)
{
    CAP_ASSERT(begin != NULL || length == 0);

    CapStringView sv = cap_sv_empty();
    sv.begin  = begin;
    sv.length = length;
    return sv;
}

static inline char
cap_sv_at(CapStringView sv, size_t i)
{
    CAP_ASSERT(i < sv.length);
    return sv.begin[i];
}

static inline int
memcasecmp(const char *a, const char *b, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        unsigned char ca = (unsigned char) tolower((unsigned char) a[i]);
        unsigned char cb = (unsigned char) tolower((unsigned char) b[i]);
        if (ca != cb) return (int)ca - (int)cb;
    }
    return 0;
}

static inline bool
cap_sv_eq(CapStringView sv1, CapStringView sv2)
{
    if (sv1.length != sv2.length) return false;
    if (sv1.length == 0)          return true;

    return memcmp(sv1.begin, sv2.begin, sv1.length) == 0;
}

static inline bool
cap_sv_eq_ci(CapStringView sv1, CapStringView sv2)
{
    if (sv1.length != sv2.length) return false;
    if (sv1.length == 0)          return true;

    return memcasecmp(sv1.begin, sv2.begin, sv1.length) == 0;
}

static inline bool
cap_sv_eq_cstr_n(CapStringView sv, const char *cstr, size_t length)
{
    if (!cstr) return false;

    if (sv.length != length) return false;
    return sv.length == 0 || memcmp(sv.begin, cstr, length) == 0;
}

static inline bool
cap_sv_eq_cstr(CapStringView sv, const char *cstr)
{
    if (!cstr) return false;

    size_t length = strlen(cstr);
    return cap_sv_eq_cstr_n(sv, cstr, length);
}

static inline bool
cap_sv_starts_with(CapStringView sv, CapStringView prefix)
{
    if (prefix.length > sv.length) return false;

    return memcmp(sv.begin, prefix.begin, prefix.length) == 0;
}

static inline CapStringView
cap_sv_substr(CapStringView sv, size_t pos, size_t count)
{
    CAP_ASSERT(pos <= sv.length);
    CAP_ASSERT(count <= sv.length - pos);

    CapStringView result = cap_sv_empty();
    result.begin = sv.begin + pos;
    result.length = count;

    return result;
}

static inline CapStringView
cap_sv_drop(CapStringView sv, size_t n)
{
    CAP_ASSERT(n <= sv.length);
    return cap_sv_substr(sv, n, sv.length-n);
}

static inline CapStringView
cap_sv_take_and_consume(CapStringView *sv, size_t n)
{
    CAP_ASSERT(n <= sv->length);

    CapStringView out = cap_sv_empty();
    out.begin = sv->begin;
    out.length = n;
    sv->begin  += n;
    sv->length -= n;
    return out;
}

static inline void
cap_sv_split_first(CapStringView  sv,
                   char           delim,
                   CapStringView *out_before_delim,
                   CapStringView *out_after_delim)
{
    const void *p = (sv.length ? memchr(sv.begin, (unsigned char)delim, sv.length) : NULL);
    if (!p) {
        if (out_before_delim) *out_before_delim = sv;
        if (out_after_delim)  *out_after_delim  = cap_sv_empty();
        return;
    }

    size_t idx = (size_t)((const char *)p - sv.begin);
    if (out_before_delim) *out_before_delim = cap_sv_substr(sv, 0, idx);
    if (out_after_delim)  *out_after_delim  = cap_sv_substr(sv, idx+1, sv.length-idx-1);
}

static inline void *
cap_sv_memrchr(const void *s, int c, size_t n)
{
    const unsigned char *p = (const unsigned char *)s + n;
    while (n--) {
        if (*--p == (unsigned char)c) return (void *)p;
    }
    return NULL;
}

static inline size_t
cap_sv_rfind_char(CapStringView sv, char c)
{
    if (sv.length == 0) return SIZE_MAX;

    const void* p = cap_sv_memrchr(sv.begin, (unsigned char)c, sv.length);
    return p ? (size_t)((const char*)p - sv.begin) : SIZE_MAX;
}


// Returns true on success, false on invalid/overflow.
// Accepts optional leading '+' for u64.
static inline bool
cap_sv_to_uint64(CapStringView sv, uint64_t *out)
{
    if (!out || sv.length == 0) return false;

    const char *p = sv.begin;
    size_t      n = sv.length;

    // Optional '+'
    if (*p == '+') { p++; if (--n == 0) return false; }

    uint64_t acc = 0;
    bool     got_digit = false;

    while (n--) {
        unsigned char c = (unsigned char)*p++;
        if (c < '0' || c > '9') return false;
        uint32_t d = (uint32_t)(c - '0');

        // acc*10 + d <= UINT64_MAX  =>  acc <= UINT64_MAX/10
        if (acc > UINT64_MAX / 10ULL ||
            (acc == UINT64_MAX / 10ULL && d > (UINT64_MAX % 10ULL))) {
            return false; // overflow
        }
        acc = acc * 10ULL + (uint64_t)d;
        got_digit = true;
    }
    if (!got_digit) return false;

    *out = acc;
    return true;
}

// Returns true on success, false on invalid/overflow.
// Accepts leading '+' or '-'.
static inline bool
cap_sv_to_int64(CapStringView sv, int64_t *out)
{
    if (!out || sv.length == 0) return false;

    const char *p = sv.begin;
    size_t      n = sv.length;

    bool neg = false;
    if (*p == '+' || *p == '-') {
        neg = (*p == '-');
        p++; if (--n == 0) return false;
    }

    // We parse as unsigned then range-check against the signed limits.
    uint64_t acc = 0;
    const uint64_t pos_limit = (uint64_t)INT64_MAX;          //  9223372036854775807
    const uint64_t neg_limit = (uint64_t)INT64_MAX + 1ULL;   //  9223372036854775808
    const uint64_t limit     = neg ? neg_limit : pos_limit;

    bool got_digit = false;
    while (n--) {
        unsigned char c = (unsigned char)*p++;
        if (c < '0' || c > '9') return false;
        uint32_t d = (uint32_t)(c - '0');

        if (acc > limit / 10ULL ||
            (acc == limit / 10ULL && (uint64_t)d > (limit % 10ULL))) {
            return false; // overflow
        }
        acc       = acc * 10ULL + (uint64_t)d;
        got_digit = true;
    }
    if (!got_digit) return false;

    if (neg) {
        if (acc == neg_limit) { *out = INT64_MIN; return true; }
        *out = -(int64_t)acc;
    } else {
        *out = (int64_t)acc;
    }
    return true;
}


static inline bool
cap_sv_to_long(CapStringView sv, long *out)
{
    if (!out) return false;
#if LONG_MAX == 2147483647L  // 32-bit long
    int64_t tmp;
    if (!cap_sv_to_int64(sv, &tmp)) return false;
    if (tmp < LONG_MIN || tmp > LONG_MAX) return false;
    *out = (long)tmp;
    return true;
#else                         // 64-bit long
    int64_t tmp;
    if (!cap_sv_to_int64(sv, &tmp)) return false;
    *out = (long)tmp;
    return true;
#endif
}


CAPDEF bool
cap_sv_to_double(CapStringView sv, double *out)
{
    if (!out || sv.length == 0) return false;

    const char *p = sv.begin;
    size_t      n = sv.length;

    // Sign
    bool neg = false;
    if (*p == '+' || *p == '-') {
        neg = (*p == '-');
        p += 1;
        if (--n == 0) return false;
    }

    // Integer part
    unsigned long long int_part = 0;
    bool               have_int = false;
    while (n && *p >= '0' && *p <= '9') {
        unsigned d = (unsigned)(*p - '0');
        if (int_part <= 1844674407370955161ULL) { // floor(ULLONG_MAX/10)
            int_part = int_part * 10ULL + (unsigned long long)d;
        }
        p += 1;
        n -= 1;
        have_int = true;
    }

    // Fractional part
    unsigned long long frac_acc    = 0; // exact up to 19 digits
    int                frac_digits = 0;
    bool               have_frac   = false;

    // track tail contribution for digits beyond 19
    double frac_tail = 0.0;
    double tail_scale = 1e-20; // weight of the 20th fractional digit

    if (n && *p == '.') {
        p += 1;
        n -= 1;
        while (n && *p >= '0' && *p <= '9') {
            unsigned d = (unsigned)(*p - '0');
            if (frac_digits < 19) {
                frac_acc = frac_acc * 10ULL + (unsigned long long)d;
            } else {
                // Add contribution of extra digits as we parse them
                frac_tail  += (double)d * tail_scale;
                tail_scale *= 0.1; // next digit is 10x smaller
            }
            frac_digits += 1;
            p += 1;
            n -= 1;
            have_frac = true;
        }
        if (!have_frac) return false; // '.' must be followed by at least one digit
    }

    if (!have_int && !have_frac) return false;

    // Exponent
    bool have_exp = false;
    bool exp_neg  = false;
    int  exp_val  = 0;
    if (n && (*p == 'e' || *p == 'E')) {
        have_exp = true;
        p += 1;
        n -= 1;
        if (!n) return false;
        if (*p == '+' || *p == '-') {
            exp_neg = (*p == '-');
            p += 1;
            if (--n == 0) return false;
        }
        bool got_exp = false;
        while (n && *p >= '0' && *p <= '9') {
            int d = (int)(*p - '0');
            if (exp_val < 10000) exp_val = exp_val * 10 + d; // cap growth
            p += 1;
            n -= 1;
            got_exp = true;
        }
        if (!got_exp) return false;
    }

    // Must consume everything
    if (n != 0) return false;

    // Build base value = int_part + frac_acc / 10^k (k up to 19)
    static const double k_pow10[20] = {
        1.0,
        1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
        1e10,1e11,1e12,1e13,1e14,1e15,1e16,1e17,1e18,1e19
    };

    double value      = (double)int_part;
    if (frac_digits > 0) {
        int    used = (frac_digits <= 19) ? frac_digits : 19;
        double frac = (double)frac_acc / k_pow10[used];
        value += frac + frac_tail;
    }

    // Apply exponent via chunked scaling (no pow)
    long exp_total = have_exp ? (exp_neg ? -exp_val : exp_val) : 0;

    if (value == 0.0) {
        *out = neg ? -0.0 : 0.0;
        return true;
    }

    if (exp_total >  308)  return false; // overflow for double
    if (exp_total < -400) {  // strong underflow
        *out = neg ? -0.0 : 0.0;
        return true;
    }

    static const double p10[9]     = { 1e1,  1e2,  1e4,   1e8,  1e16,  1e32,  1e64,  1e128,  1e256 };
    static const double p10_inv[9] = { 1e-1, 1e-2, 1e-4,  1e-8, 1e-16, 1e-32, 1e-64, 1e-128, 1e-256 };
    static const int    p10_exp[9] = { 1,    2,    4,     8,    16,    32,    64,    128,    256 };

    #define CAP_SV_FINITE(x) (((x) - (x)) == 0.0)

    double scaled = value;
    long   e      = exp_total;

    if (e > 0) {
        for (int i = 8; i >= 0; --i) {
            if (e >= p10_exp[i]) {
                scaled *= p10[i];
                if (!CAP_SV_FINITE(scaled)) return false; // overflow to inf/NaN
                e -= p10_exp[i];
            }
        }
    } else if (e < 0) {
        e = -e;
        for (int i = 8; i >= 0; --i) {
            if (e >= p10_exp[i]) {
                scaled *= p10_inv[i]; // multiply by reciprocal to avoid division
                e -= p10_exp[i];
            }
        }
    }

    *out = neg ? -scaled : scaled;
    return true;

    #undef CAP_SV_FINITE
}



// String based on https://github.com/rsore/str/releases/tag/v2.0.0

typedef struct {
    char  *buffer;
    size_t capacity;
    size_t size;
} CapString;

static inline CapString
cap_str_init(CAP_NO_PARAMS)
{
    CapString result = {NULL, 0, 0};

    char *p = (char *)CAP_REALLOC(NULL, 1);
    if (p) {
        p[0] = '\0';
        result.buffer = p;
        result.capacity = 1;
        result.size = 0;
    }

    return result;
}

static inline bool
cap_str_grow_to_fit(CapString *str, size_t n)
{
    if (!str) return false;

    size_t np1 = n + 1; // 'n plus 1' to account for trailing NUL-terminator
    if (np1 < n) return false; // Overflow protection

    if (str->buffer && np1 <= str->capacity) {
        return true;
    }

    size_t new_cap = str->capacity ? str->capacity : 64;

    // Exponential growth until threshold
    while (new_cap < np1 && new_cap < 1u * 1024u * 1024u) {
        if (new_cap > SIZE_MAX / 2) { new_cap = SIZE_MAX; break; } // Overflow protection
        new_cap *= 2;
    }

    // Linear growth after threshold
    while (new_cap < np1) {
        if (new_cap > SIZE_MAX - 256u * 1024u) { new_cap = SIZE_MAX; break; } // Overflow protection
        new_cap += 256u * 1024u;
    }

    void *new_buffer = CAP_REALLOC(str->buffer, new_cap);
    if (!new_buffer) {
        return false;
    }

    str->buffer = (char *)new_buffer;
    str->capacity = new_cap;

    str->buffer[str->size] = '\0';

    return true;
}

static inline bool
cap_str_would_overflow(size_t a, size_t b)
{
    return b > SIZE_MAX - a;
}

static inline bool
cap_str_append_cstr_n(CapString *str, const char *cstr, size_t len)
{
    if (!str || (!cstr && len)) return false;

    if (cap_str_would_overflow(str->size, len)) return false;
    if (!cap_str_grow_to_fit(str, str->size + len)) return false;

    if (len) memcpy(str->buffer + str->size, cstr, len);
    str->size += len;
    str->buffer[str->size] = '\0';

    return true;
}

static inline bool
cap_str_append_cstr(CapString *str, const char *cstr)
{
    if (!str || !cstr) return false;
    size_t len = strlen(cstr);
    return cap_str_append_cstr_n(str, cstr, len);
}


static inline bool
cap_str_append_char(CapString *str, char c)
{
    if (!str) return false;
    if (cap_str_would_overflow(str->size, 1)) return false;
    if (!cap_str_grow_to_fit(str, str->size + 1)) return false;
    str->buffer[str->size++] = c;
    str->buffer[str->size] = '\0';
    return true;
}


static inline bool
cap_str_append_str(CapString *str, const CapString *app)
{
    if (!str) return false;
    if (!app || app->size == 0) return true;

    if (str == app) {
        size_t len = str->size;
        if (!cap_str_grow_to_fit(str, str->size + len)) return false;
        // source and destination are the same allocation, so use memmove
        memmove(str->buffer + str->size, str->buffer, len);
        str->size += len;
        str->buffer[str->size] = '\0';
        return true;
    }

    return cap_str_append_cstr_n(str, app->buffer, app->size);
}


static inline bool
cap_str_vappendf(CapString *str, const char *fmt, va_list args)
{
    if (!str || !fmt) return false;

    va_list ap;
    va_copy(ap, args);
    int need = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (need < 0) return false;

    if (!cap_str_grow_to_fit(str, str->size + (size_t)need)) return false;

    va_copy(ap, args);
    vsnprintf(str->buffer + str->size, (size_t)need + 1, fmt, ap);
    va_end(ap);

    str->size += (size_t)need;
    str->buffer[str->size] = '\0';
    return true;
}

static inline bool
cap_str_appendf(CapString *str, const char *fmt, ...)
{
    if (!str || !fmt) return false;
    va_list args;
    va_start(args, fmt);
    bool ok = cap_str_vappendf(str, fmt, args);
    va_end(args);
    return ok;
}

static inline CapString
cap_str_from_sv(CapStringView sv)
{
    CapString result = cap_str_init();
    cap_str_append_cstr_n(&result, sv.begin, sv.length);
    return result;
}

static inline CapStringView
cap_sv_from_str(CapString str)
{
    CapStringView result = cap_sv_from_parts(str.buffer, str.size);
    return result;
}

static inline char *
cap_str_release(CapString *str, size_t *out_len)
{
    if (!str) return NULL;

    if (!str->buffer) {
        char *z = (char *)CAP_REALLOC(NULL, 1);
        if (!z) return NULL;
        z[0] = '\0';
        if (out_len) *out_len = 0;
        return z;
    }

    str->buffer[str->size] = '\0';

    if (out_len) *out_len = str->size;
    char *result = str->buffer;

    str->buffer   = NULL;
    str->capacity = 0;
    str->size     = 0;

    return result;
}

static inline void
cap_str_clear(CapString *str)
{
    if (!str) return;

    str->size = 0;

    if (str->buffer) {
        str->buffer[0] = '\0';
    } else {
        char *p = (char *)CAP_REALLOC(NULL, 1);
        if (p) {
            p[0] = '\0';
            str->buffer = p;
            str->capacity = 1;
            str->size = 0;
        }
    }
}

static inline void
cap_str_free(CapString *str)
{
    if (!str) return;

    CAP_FREE(str->buffer);
    str->buffer   = NULL;
    str->capacity = 0;
    str->size     = 0;
}





typedef enum {
    CAP_VALUE_TYPE_BOOL, // flag
    CAP_VALUE_TYPE_STRING,
    CAP_VALUE_TYPE_INT,
    CAP_VALUE_TYPE_DOUBLE,
    CAP_VALUE_TYPE_ENUM
} CapValueType;

typedef struct {
    intmax_t  integer;
    char     *string;
} CapEnumEntry;

typedef struct {
    CapEnumEntry *buffer;
    size_t        capacity;
    size_t        size;
} CapEnumEntries;

typedef union {
    bool      boolean;
    int       integer;
    double    decimal;
    char     *string;
    intmax_t  enumeration;
} CapValue;

typedef struct {
    CapString long_name;

    bool has_short_name;
    char short_name;

    CapString description;
    CapString metavar;

    bool required;

    CapValueType  value_type;
    bool          has_default_value;
    CapValue      default_value;
    CapValue      *out_value;

    bool flag_invert;

    CapEnumEntries enum_entries;
    size_t         enum_type_size;
    bool           enum_type_is_signed;
    bool           enum_is_case_insensitive;

    bool           active;
} CapOption;

typedef struct {
    CapOption *buffer;
    size_t         size;
    size_t         capacity;
} CapOptions;

typedef struct {
    CapString label;
    CapString description;

    CapValueType  value_type;
    CapValue     *value;

    bool active;
} CapPositional;

typedef struct {
    CapPositional  *buffer;
    size_t          size;
    size_t          capacity;

    size_t parsed_count;
} CapPositionals;

typedef struct {
    char   **buffer;
    size_t   size;
    size_t   capacity;
} CapVariadic;

typedef struct {
    bool accept_variadic;

    CapString label;
    CapString description;

    size_t min;
    size_t max;

    // Points to user-passed array if variadic is accepted:
    char  ***variadic_out_arr;
    size_t  *variadic_out_len;
} CapVariadicSpec;

struct CapContext {
    // Config
    CapString program_description;
    CapString program_version;
    CapPositionals positionals;

    CapOptions options;
    CapVariadicSpec variadic_spec;

    bool      capture_remainder;
    CapString remainder_description;
    // Points to user-passed variables, we write them at end of parsing.
    int    *remainder_argc;
    char ***remainder_argv;

    size_t help_wrap_column;


    // build state
    bool building_positional;
    bool building_variadic;
    bool building_option;


    // Parse state
    CapString program;
    CapString usage;
    CapString help_menu;
    CapString error_msg;

    bool terminator_encountered;

    CapVariadic variadic;
};

static CapContext *active_ctx;

#ifdef _WIN32
static const char *cap_default_help_flags[] = {"--help", "-help", "-h", "-?", "/help", "/h", "/?"};
#else
static const char *cap_default_help_flags[] = {"--help", "-help", "-h"};
#endif

#ifdef _WIN32
static const char *cap_default_version_flags[] = {"--version", "-V", "/version", "/V"};
#else
static const char *cap_default_version_flags[] = {"--version", "-V"};
#endif

static inline bool
cap_string_in_array(const char    *str,
                    const char   **arr,
                    const size_t   arr_length)
{
    for (size_t i = 0; i < arr_length; ++i) {
        if (strcmp(str, arr[i]) == 0) return true;
    }
    return false;
}

#ifdef __cplusplus
#define CAP_DARRAY_NEW_BUFFER_TYPE(arr) decltype((arr)->buffer)
#else
#define CAP_DARRAY_NEW_BUFFER_TYPE(arr) void*
#endif
#define cap_darray_grow_to_fit(arr, new_capacity)                       \
    do {                                                                \
        if ((new_capacity) <= (arr)->capacity) break;                   \
        size_t cap_darray_new_capacity_ = (arr)->capacity ? (arr)->capacity * 2 : 16; \
        while (cap_darray_new_capacity_ < (new_capacity)) cap_darray_new_capacity_ *= 2; \
        const size_t cap_alloc_size_ = cap_darray_new_capacity_ * sizeof(*(arr)->buffer); \
        CAP_DARRAY_NEW_BUFFER_TYPE(arr) cap_new_buffer_ = (CAP_DARRAY_NEW_BUFFER_TYPE(arr))CAP_REALLOC((arr)->buffer, cap_alloc_size_); \
        CAP_ASSERT(cap_new_buffer_);                                    \
        (arr)->buffer = cap_new_buffer_;                                \
        (arr)->capacity = cap_darray_new_capacity_;                     \
    } while (0)

#define cap_darray_append(arr, new_element)             \
    do {                                                \
        cap_darray_grow_to_fit((arr), (arr)->size+1);   \
        (arr)->buffer[(arr)->size++] = (new_element);   \
    } while (0)

#define cap_darray_free(arr)                                            \
    do {                                                                \
        if ((arr)->buffer) CAP_FREE((arr)->buffer);                     \
        (arr)->buffer = NULL; (arr)->capacity = 0; (arr)->size = 0;     \
    } while (0)

#define cap_darray_foreach(arr, type, it)                               \
    for (type *it = (arr)->buffer; it < (arr)->buffer+(arr)->size; ++it)

#define cap_darray_copy(src, dest)                                      \
    do {                                                                \
        cap_darray_grow_to_fit((dest), (src)->size);                    \
        memcpy((dest)->buffer, (src)->buffer, (src)->size * sizeof(*(src)->buffer)); \
        (dest)->size = (src)->size;                                     \
    } while (0)

static inline bool
parse_int(CapStringView arg, int *out_value)
{
    long l;
    if (!cap_sv_to_long(arg, &l)) return false;

    *out_value = (int)l;

    return true;
}


static inline bool
parse_double(CapStringView arg, double *out_value)
{
    double d;
    if (!cap_sv_to_double(arg, &d)) return false;

    *out_value = d;

    return true;
}


static inline char *
cap_sv_strdup(CapStringView sv)
{
    const size_t size = sv.length + 1; // +1 for null-terminator
    char *str = (char *)CAP_REALLOC(NULL, size);
    if (str) {
        memcpy(str, sv.begin, sv.length);
        str[sv.length] = '\0';
    }
    return str;
}

static inline char *
cap_strdup(const char *str1)
{
    const size_t size = strlen(str1) + 1; // +1 for null-terminator
    char *str2 = (char *)CAP_REALLOC(NULL, size);
    if (str2) {
        memcpy(str2, str1, size);
    }
    return str2;
}


CAPDEF CapContext *
cap_context_new(CAP_NO_PARAMS) CAP_NOEXCEPT
{
    CapContext *ctx = (CapContext *)CAP_REALLOC(NULL, sizeof(CapContext));

    memset(ctx, 0, sizeof(CapContext));


    // Defaults
    ctx->help_wrap_column = CAP_DEFAULT_WRAP_COLUMN;

    return ctx;
}

CAPDEF void
cap_set_program_description(CapContext *context,
                            const char *description) CAP_NOEXCEPT
{
    CAP_ASSERT(description);

    cap_str_clear(&context->program_description);
    cap_str_append_cstr(&context->program_description, description);
}

CAPDEF void
cap_set_program_version(CapContext *context,
                        const char *version) CAP_NOEXCEPT
{
    cap_str_clear(&context->program_version);
    cap_str_append_cstr(&context->program_version, version);
}

CAPDEF void
cap_set_help_wrap_column(CapContext *ctx, size_t wrap_column) CAP_NOEXCEPT
{
    ctx->help_wrap_column = wrap_column;
}

CAPDEF const char *
cap_program(CapContext *ctx) CAP_NOEXCEPT
{
    return ctx->program.buffer;
}

CAPDEF const char *
cap_program_version(CapContext *ctx) CAP_NOEXCEPT
{
    return ctx->program_version.buffer;
}

CAPDEF const char *
cap_program_description(CapContext *ctx) CAP_NOEXCEPT
{
    return ctx->program_description.buffer;
}

static inline int
cap_levenshtein_distance(CapStringView sv1, CapStringView sv2)
{
    const size_t rows = sv1.length + 1;
    const size_t cols = sv2.length + 1;

    int result;

#define CAP_LD_MAT_(i, j) matrix[(i) * cols + (j)]
    int *matrix = (int *)CAP_REALLOC(NULL, rows * cols * sizeof(int));

    for (int i = 0; i < (int)rows; ++i) CAP_LD_MAT_(i, 0) = i;
    for (int j = 0; j < (int)cols; ++j) CAP_LD_MAT_(0, j) = j;

    for (size_t i = 1; i < rows; ++i) {
        for (size_t j = 1; j < cols; ++j) {
            int cost           = (cap_sv_at(sv1, i-1) == cap_sv_at(sv2, j-1)) ? 0 : 1;
            int deletion       = CAP_LD_MAT_(i - 1, j) + 1;
            int insertion      = CAP_LD_MAT_(i, j - 1) + 1;
            int substitution   = CAP_LD_MAT_(i - 1, j - 1) + cost;
            CAP_LD_MAT_(i, j) = CAP_MIN(CAP_MIN(deletion, insertion), substitution);
        }
    }

    result = CAP_LD_MAT_(sv1.length, sv2.length);
    CAP_FREE(matrix);
#undef CAP_LD_MAT_

    return result;
}


// TODO: Merge the two closest_match functions. Take a StringViewArray, and search
//       through that. May require the called to build an array of views, but whatever.
//       I don't want this duplication of closest-match searching. Alternatively
//       generalize the problem enough that the two functions can remain separate
//       but brief, calling out to a generalized function.

static inline const char *
cap_find_closest_long_name(CapOptions    *options,
                           CapStringView  input,
                           double        *out_similarity)
{
    CAP_ASSERT(out_similarity);

    if (input.length <= 1) {
        *out_similarity = 0.0f;
        return NULL;
    }

    const char *closest_match = NULL;
    int closest_match_distance = INT_MAX;
    cap_darray_foreach(options, CapOption, candidate) {
        if (candidate->long_name.size > 0) {
            const int distance = cap_levenshtein_distance(input, cap_sv_from_str(candidate->long_name));
            if (distance < closest_match_distance) {
                closest_match_distance = distance;
                closest_match = candidate->long_name.buffer; // TODO: Change when we return StringView in the future
            }
        }
    }

    if (closest_match) {
        const size_t closest_match_length = strlen(closest_match);
        double denom = (double)CAP_MAX(input.length, closest_match_length);
        *out_similarity = denom > 0.0 ? 1.0 - (closest_match_distance / denom) : 0.0;
    }

    return closest_match;
}

static inline const char *
cap_find_closest_enum(const CapEnumEntries *entries,
                      CapStringView         input,
                      double               *out_similarity)
{
    CAP_ASSERT(out_similarity);
    if (!entries || entries->size == 0) {
        *out_similarity = 0.0f;
        return NULL;
    }

    if (input.length <= 1) {
        *out_similarity = 0.0f;
        return NULL;
    }

    const char *closest_match = NULL;
    int closest_match_distance = INT_MAX;
    cap_darray_foreach(entries, CapEnumEntry, entry) {
        CapStringView candidate = cap_sv_from_cstr(entry->string);
        const int distance = cap_levenshtein_distance(input, candidate);
        if (distance < closest_match_distance) {
            closest_match_distance = distance;
            closest_match = entry->string;
        }
    }

    if (closest_match) {
        const size_t closest_match_length = strlen(closest_match);
        double denom = (double)CAP_MAX(input.length, closest_match_length);
        *out_similarity = denom > 0.0 ? 1.0 - (closest_match_distance / denom) : 0.0;
    }

    return closest_match;
}


static inline CapResult
cap_parse_positional(CapContext *context, CapStringView arg)
{
    CapString *err = &context->error_msg;

    // Are we even expecting positionals?
    if (context->positionals.size == 0 && !context->variadic_spec.accept_variadic)  {
        cap_str_appendf(err, "Error: Unexpected positional argument '%.*s'.\n", arg.length, arg.begin);
        return CAP_ERR_UNEXPECTED_POSITIONAL;
    }


    // Handle positional overflow
    if (context->positionals.parsed_count >= context->positionals.size) {
        if (!context->variadic_spec.accept_variadic) {
            cap_str_appendf(err,
                           "Error: Expected %zu positional %s, argument '%.*s' surpassed this.\n",
                           context->positionals.size,
                           context->positionals.size == 1 ? "argument" : "arguments",
                           arg.length, arg.begin);
            return CAP_ERR_TOO_MANY_POSITIONALS;
        } else if (context->variadic_spec.max && (context->variadic.size >= context->variadic_spec.max)) {
            const size_t total_max_args = context->positionals.size + context->variadic_spec.max;
            cap_str_appendf(err,
                           "Error: Expected at most %zu %s arguments, argument '%.*s' surpassed this.\n",
                           total_max_args,
                           context->positionals.size == 1 ? "positional" : "positionals",
                           arg.length, arg.begin);
            return CAP_ERR_TOO_MANY_POSITIONALS;
        }
    }


    // Actually parse the arg

    if (context->positionals.parsed_count >= context->positionals.size) {
        // Variadic value
        cap_darray_append(&context->variadic, cap_sv_strdup(arg));
        return CAP_OK;
    }

    // Labeled positional value
    CapPositional *positional = &context->positionals.buffer[context->positionals.parsed_count++];
    switch (positional->value_type) {
    case CAP_VALUE_TYPE_INT: {
        int value = 0;
        if (!parse_int(arg, &value)) {
            cap_str_appendf(err, "Error: Expected integer, got '%.*s'.\n", arg.length, arg.begin);
            return CAP_ERR_INVALID_VALUE;
        }
        (*positional->value).integer = value;
    } break;
    case CAP_VALUE_TYPE_STRING: {
        (*positional->value).string = cap_sv_strdup(arg);
    } break;
    default:
        CAP_ASSERT(false && "Unhandled value type");
    }

    positional->active = true;
    return CAP_OK;
}


// TODO: Take CapStringViewArray instead of raw c strings. We want to adjust cap_parse()
//       to do one pass over all argv values, creating a StringViewArray instead, and work
//       on those during parsing. We are currently having to do a lot of cap_sv_from_cstr,
//       which is noisy and takes time to do, since length is unknown. Perhaps the default
//       help flags can be string views too?
static inline bool
cap_was_help_requested(int          argc,
                       char *const *argv)
{
    const size_t default_help_flags_length = CAP_ARRAY_LENGTH(cap_default_help_flags);

    for (size_t i = 0; i < (size_t)argc; ++i) {
        const char *arg = argv[i];
        if (strcmp(arg, "--") == 0) break;
        if (cap_string_in_array(arg, cap_default_help_flags, default_help_flags_length)) {
            return true;
        }
    }

    return false;
}

// TODO: See todo for cap_was_help_requested
static inline bool
cap_was_version_requested(int           argc,
                          char *const  *argv)
{
    const size_t default_version_flags_length = CAP_ARRAY_LENGTH(cap_default_version_flags);

    for (size_t i = 0; i < (size_t)argc; ++i) {
        const char *arg = argv[i];
        if (strcmp(arg, "--") == 0) break;
        if (cap_string_in_array(arg, cap_default_version_flags, default_version_flags_length)) {
            return true;
        }
    }

    return false;
}

// TODO: Take CapStringView instead of cstr
static inline void
cap_write_wrapped_text_at_column(const char *text,
                                 size_t      start_column,
                                 size_t      wrap_column,
                                 CapString  *out)
{
    if (!text) return;

    size_t line_width = (wrap_column > start_column) ? (wrap_column - start_column) : 1;

    const char *p = text;
    while (*p) {
        const char *paragraph_end = p;
        while (*paragraph_end && *paragraph_end != '\n') paragraph_end++;

        while (p < paragraph_end) {
            size_t remaining = (size_t)(paragraph_end - p);
            size_t take = remaining < line_width ? remaining : line_width;

            size_t break_len = take;
            if (take == line_width && p[break_len] && !isspace((unsigned char)p[break_len])) {
                const char *q = p + take;
                while (q > p && !isspace((unsigned char)q[-1])) q--;
                if (q > p) break_len = (size_t)(q - p);
            }

            cap_str_appendf(out, "%.*s", (int)break_len, p);
            p += break_len;


            while (p < paragraph_end && isspace((unsigned char)*p) && *p != '\n') p++;

            if (p < paragraph_end) {
                cap_str_appendf(out, "\n%*s", (int)start_column, "");
            }
        }

        if (*paragraph_end == '\n') {
            p = paragraph_end + 1;
            if (*p) {
                cap_str_appendf(out, "\n%*s", (int)start_column, "");
            }
        } else {
            p = paragraph_end;
        }
    }
}

static inline void
cap_format_positional_label(CapPositional *positional, CapString *out)
{
    cap_str_appendf(out, "<%s>", positional->label.buffer);
}

static inline void
cap_format_variadic_label(CapVariadicSpec *spec, CapString *out)
{
    if (!spec->min) cap_str_append_char(out, '[');
    cap_str_appendf(out, "<%s>...", spec->label.buffer);
    if (!spec->min) cap_str_append_char(out, ']');
}

static inline void
cap_format_option_short_name(CapOption *option, CapString *out)
{
    if (!option->has_short_name) return;
    cap_str_appendf(out, "-%c", option->short_name);
}

static inline void
cap_format_option_long_name(CapOption *option, CapString *out)
{
    if (option->long_name.size == 0) return;
    cap_str_appendf(out, "--%s", option->long_name.buffer);
}

static inline void
cap_format_option_metavar(CapOption *option, CapString *out)
{
    if (option->value_type == CAP_VALUE_TYPE_BOOL) return;
    cap_str_appendf(out, "<%s>", option->metavar.buffer);
}

static inline void
cap_write_positionals_for_usage(CapContext *context, CapString *out)
{
    cap_darray_foreach(&context->positionals, CapPositional, positional) {
        cap_str_append_char(out, ' ');
        cap_format_positional_label(positional, out);
    }

    if (context->variadic_spec.accept_variadic) {
        cap_str_append_char(out, ' ');
        cap_format_variadic_label(&context->variadic_spec, out);
    }
}

static inline void
cap_write_options_for_usage(CapContext *context, CapString *out)
{
    CapOptions *options = &context->options;

    if (options->size > 0) {
        cap_str_append_cstr(out, " [<option>...]");
    }

    cap_darray_foreach(options, CapOption, option) {

        if (option->required) {
            if (option->long_name.size > 0)  {
                cap_str_append_char(out, ' ');
                cap_format_option_long_name(option, out);
            } else {
                cap_str_append_char(out, ' ');
                cap_format_option_short_name(option, out);
            }

            if (option->value_type != CAP_VALUE_TYPE_BOOL) {
                cap_str_append_char(out, ' ');
                cap_format_option_metavar(option, out);
            }
        }
    }
}

static inline void
cap_write_usage_to_str(CapContext *context, CapString *str)
{
    CapString temp = cap_str_init();

    if (context->program.size > 0) cap_str_appendf(&temp, "Usage: %s", context->program.buffer);
    else                                       cap_str_append_cstr(&temp, "Usage: program");

    const size_t start_col = temp.size+1;
    cap_str_append_str(str, &temp);
    cap_str_clear(&temp);

    cap_write_options_for_usage(context, &temp);
    cap_write_positionals_for_usage(context, &temp);

    if (context->positionals.size > 0
        || context->variadic_spec.accept_variadic
        || context->capture_remainder) {
        cap_str_append_cstr(&temp, " [-- [<arg>...]]");
    }

    cap_write_wrapped_text_at_column(temp.buffer, start_col, context->help_wrap_column, str);
    cap_str_append_char(str, '\n');

    cap_str_free(&temp);
}

static inline void
cap_write_usage_with_help_hint(CapContext *context)
{
    CapString *err = &context->error_msg;
    cap_write_usage_to_str(context, err);
    cap_str_append_cstr(err, "Hint: Use '--help' for help menu\n");
}

CAPDEF const char *
cap_usage(CapContext *context) CAP_NOEXCEPT
{
    cap_str_clear(&context->usage);
    cap_write_usage_to_str(context, &context->usage);
    return context->usage.buffer;
}


static inline void
cap_assert_options_dont_collide(CapContext *ctx)
{
#ifndef NDEBUG
    CapOptions *options = &ctx->options;
    for (size_t i = 0; i < options->size; ++i) {
        CapOption *option = &options->buffer[i];

        // Check against reserved help flags
        const size_t default_help_flags_length = CAP_ARRAY_LENGTH(cap_default_help_flags);
        for (size_t j = 0; j < default_help_flags_length; ++j) {
            const char *help_flag = cap_default_help_flags[j];
            while (help_flag[0] == '-' || help_flag[0] == '/') help_flag++;
            if (option->long_name.size > 0) CAP_ASSERT(strcmp(option->long_name.buffer, help_flag) != 0 &&
                                                    "Long option name collides with help flag.");
            if (option->has_short_name) {
                const char short_name_cstr[2] = {option->short_name, '\0'};
                CAP_ASSERT(strcmp(short_name_cstr, help_flag) != 0 &&
                           "Short option name collides with help flag.");
            }
        }

        // Check against reserved version flags
        const size_t default_version_flags_length = CAP_ARRAY_LENGTH(cap_default_version_flags);
        for (size_t j = 0; j < default_version_flags_length; ++j) {
            const char *version_flag = cap_default_version_flags[j];
            while (version_flag[0] == '-' || version_flag[0] == '/') version_flag++;
            if (option->long_name.size > 0) CAP_ASSERT(strcmp(option->long_name.buffer, version_flag) != 0 &&
                                                    "Long option name collides with version flag.");
            if (option->has_short_name) {
                const char short_name_cstr[2] = {option->short_name, '\0'};
                CAP_ASSERT(strcmp(short_name_cstr, version_flag) != 0 &&
                           "Short option name collides with version flag.");
            }
        }

        // Check against all other options
        if (i == options->size-1) continue;
        for (size_t j = i+1; j < options->size; ++j) {
            CapOption *other_option = &options->buffer[j];
            if (option->long_name.size > 0 && other_option->long_name.size > 0)
                CAP_ASSERT(strcmp(option->long_name.buffer, other_option->long_name.buffer) != 0 &&
                           "Detected collision in long option names.");
            if (option->has_short_name && other_option->has_short_name)
                CAP_ASSERT((option->short_name != other_option->short_name) &&
                           "Detected collision in short option names.");
        }
    }
#endif // NDEBUG
}


static inline CapOption *
cap_find_option_by_long_name(CapOptions *options, CapStringView long_name)
{
    cap_darray_foreach(options, CapOption, option) {
        if (option->long_name.size > 0 && cap_sv_eq(long_name, cap_sv_from_str(option->long_name))) {
            return option;
        }
    }
    return NULL;
}

static CapOption *
cap_find_option_by_short_name(CapOptions *options, char short_name)
{
    cap_darray_foreach(options, CapOption, option) {
        if (option->has_short_name && option->short_name == short_name) {
            return option;
        }
    }
    return NULL;
}

typedef enum {
    CAP_POSITIONAL,
    CAP_SINGLE_DASH_OPTION,
    CAP_DOUBLE_DASH_OPTION,
    CAP_TERMINATOR
} CapArgumentKind;

static inline CapArgumentKind
cap_classify_arg(CapContext *context, CapStringView arg)
{
    CAP_ASSERT(arg.length > 0);

    if (context->terminator_encountered) {
        return CAP_POSITIONAL;
    }

    if (!cap_sv_starts_with(arg, CAP_SV_LIT("-"))) return CAP_POSITIONAL;
    if (cap_sv_eq(arg, CAP_SV_LIT("-")))           return CAP_POSITIONAL;

    if (cap_sv_eq(arg, CAP_SV_LIT("--"))) return CAP_TERMINATOR;

    if (cap_sv_starts_with(arg, CAP_SV_LIT("--"))) return CAP_DOUBLE_DASH_OPTION;

    return CAP_SINGLE_DASH_OPTION;
}

// Enum underlying type is implementation-defined, so we cannot make assumption.
// So when we want to write a value back to the user's enum value we must
// go through this function.
static void cap_write_enum(void     *out,
                           size_t    type_size,
                           int       is_signed,
                           intmax_t  value)
{
    if (is_signed) {
        // Range-check for signed n-byte integer
        if (type_size != 1 && type_size != 2 && type_size != 4 && type_size != 8) {
            CAP_ASSERT(false && "Cannot write enum value for specified type size");
        }
        intmax_t min = -( (intmax_t)1 << (type_size*8 - 1) );
        intmax_t max =   ( (intmax_t)1 << (type_size*8 - 1) ) - 1;
        if (value < min || value > max) {
            CAP_ASSERT(false && "Failed to write signed enum value, out of range");
        }

        switch (type_size) {
            case 1: { int8_t  t = (int8_t)value;  memcpy(out, &t, 1); break; }
            case 2: { int16_t t = (int16_t)value; memcpy(out, &t, 2); break; }
            case 4: { int32_t t = (int32_t)value; memcpy(out, &t, 4); break; }
            case 8: { int64_t t = (int64_t)value; memcpy(out, &t, 8); break; }
        }
    } else {
        // Range-check for unsigned n-byte integer
        if (value < 0) {
            CAP_ASSERT(false && "Cannot write enum value of less than 0 for unsigned type");
        }
        if (type_size != 1 && type_size != 2 && type_size != 4 && type_size != 8) {
            CAP_ASSERT(false && "Cannot write enum value for specified type size");
        }
        uintmax_t uvalue = (uintmax_t)value;
        uintmax_t umax = (type_size == 8) ? UINT64_MAX : (((uintmax_t)1 << (type_size*8)) - 1);
        if (uvalue > umax) {
            CAP_ASSERT(false && "Failed to write unsigned enum value, out of range");
        }

        switch (type_size) {
            case 1: { uint8_t  t = (uint8_t)uvalue;  memcpy(out, &t, 1); break; }
            case 2: { uint16_t t = (uint16_t)uvalue; memcpy(out, &t, 2); break; }
            case 4: { uint32_t t = (uint32_t)uvalue; memcpy(out, &t, 4); break; }
            case 8: { uint64_t t = (uint64_t)uvalue; memcpy(out, &t, 8); break; }
        }
    }
}

static inline CapResult
cap_parse_option_value(CapContext    *context,
                       CapOption     *option,
                       CapStringView  opt_arg,
                       CapStringView  value_arg)
{
    CapString *err = &context->error_msg;

    option->active = true;

    switch (option->value_type) {
    case CAP_VALUE_TYPE_STRING: {
        option->out_value->string = cap_sv_strdup(value_arg);
    } break;
    case CAP_VALUE_TYPE_INT: {
        int value = 0;
        if (!parse_int(value_arg, &value)) {
            cap_str_appendf(err, "Error: Option '%.*s' expected an integer, but got '%.*s'.\n",
                           opt_arg.length, opt_arg.begin, value_arg.length, value_arg.begin);
            return CAP_ERR_INVALID_VALUE;
        } else {
            option->out_value->integer = value;
        }
    } break;
    case CAP_VALUE_TYPE_DOUBLE: {
        double value = 0;
        if (!parse_double(value_arg, &value)) {
            cap_str_appendf(err, "Error: Option '%.*s' expected a decimal number, but got '%.*s'.\n",
                           opt_arg.length, opt_arg.begin, value_arg.length, value_arg.begin);
            return CAP_ERR_INVALID_VALUE;
        } else {
            option->out_value->decimal = value;
        }
    } break;
    case CAP_VALUE_TYPE_ENUM: {
        size_t idx = SIZE_MAX;
        cap_darray_foreach(&option->enum_entries, CapEnumEntry, entry) {
            CapStringView entry_sv = cap_sv_from_cstr(entry->string);
            if ((option->enum_is_case_insensitive && cap_sv_eq_ci(value_arg, entry_sv)) || cap_sv_eq(value_arg, entry_sv)) {
                idx = entry - option->enum_entries.buffer;
                break;
            }
        }
        if (idx == SIZE_MAX) {
            // Unrecognized value
            cap_str_appendf(err, "Error: Option '%.*s' expects one of '", opt_arg.length, opt_arg.begin);
            size_t i = 0;
            cap_darray_foreach(&option->enum_entries, CapEnumEntry, entry) {
                if (i++ > 0) cap_str_append_char(err, '|');
                cap_str_appendf(err, "%s", entry->string);
            }
            cap_str_appendf(err, "' but got '%.*s'.", value_arg.length, value_arg.begin);
            double similarity = 0.0;
            const char *suggested = cap_find_closest_enum(&option->enum_entries,
                                                          value_arg,
                                                          &similarity);
            if (suggested && similarity > 0.5) {
                cap_str_appendf(err, " Did you mean '%s'?", suggested);
            }
            cap_str_append_char(err, '\n');

            return CAP_ERR_INVALID_VALUE;
        }
        cap_write_enum(option->out_value,
                       option->enum_type_size,
                       option->enum_type_is_signed,
                       option->enum_entries.buffer[idx].integer);
    } break;
    default:
        CAP_ASSERT(false && "Unhandled value type");
    }

    return CAP_OK;
}

static inline void
cap_fill_defaults(CapOptions *options)
{
    // Fill in default values where user didn't pass a value
    cap_darray_foreach(options, CapOption, opt) {
        if (opt->has_default_value && !opt->active) {
            switch (opt->value_type) {
            case     CAP_VALUE_TYPE_STRING: opt->out_value->string  = cap_strdup(opt->default_value.string);  break;
            case     CAP_VALUE_TYPE_INT:    opt->out_value->integer = opt->default_value.integer; break;
            case     CAP_VALUE_TYPE_DOUBLE: opt->out_value->decimal = opt->default_value.decimal; break;
            case     CAP_VALUE_TYPE_ENUM: cap_write_enum(opt->out_value, opt->enum_type_size, opt->enum_type_is_signed, opt->default_value.enumeration); break;
            default: CAP_ASSERT(false && "Unhandled value type");
            }
            opt->active = true;
        }
    }

}

static inline bool
cap_verify_positionals(CapContext *context)
{
    CapString *err = &context->error_msg;

    size_t min          = context->positionals.size;
    size_t total_parsed = context->positionals.parsed_count;
    if (context->variadic_spec.accept_variadic) {
        min          += context->variadic_spec.min;
        total_parsed += context->variadic.size;
    }
    if (total_parsed < min) {
        cap_str_appendf(err, "Error: Expected %s%zu positional %s, got %zu.\n",
                       context->variadic_spec.accept_variadic ? "at least " : "",
                       min,
                       min == 1 ? "argument" : "arguments",
                       total_parsed);
        return false;
    }
    return true;
}

static inline bool
cap_check_required_options(CapContext *context)
{
    CapString *err = &context->error_msg;

    CapOptions *options = &context->options;

    bool success = true;
    cap_darray_foreach(options, CapOption, opt) {
        if (opt->required && !opt->active) {
            cap_str_append_cstr(err, "Error: Required option '");
            if (opt->long_name.size > 0) cap_format_option_long_name(opt, err);
            else                               cap_format_option_short_name(opt, err);
            cap_str_append_cstr(err, "' was not specified\n");
            success = false;
        }
    }
    return success;
}

static inline CapResult
cap_parse_user_args(CapContext  *context,
                    int          argc,
                    char       **argv)
{
    while (argc) {
        CapStringView arg = cap_sv_from_cstr(CAP_SHIFT(argv, argc));
        CapArgumentKind kind = cap_classify_arg(context, arg);

        switch (kind) {

        case CAP_POSITIONAL: {
            CapResult r = cap_parse_positional(context, arg);
            if (r != CAP_OK) {
                cap_write_usage_with_help_hint(context);
                return r;
            }
        } break;

        case CAP_DOUBLE_DASH_OPTION: {
            CapStringView long_name = cap_sv_drop(arg, 2);
            CapStringView value = cap_sv_empty();
            cap_sv_split_first(long_name, '=', &long_name, &value); // In case user passes --name=value
            CapOption *match = cap_find_option_by_long_name(&context->options, long_name);
            if (!match) {
                cap_str_appendf(&context->error_msg, "Error: Unrecognized option '%.*s'.", arg.length, arg.begin);
                double similarity = 0.0;
                const char *suggested = cap_find_closest_long_name(&context->options,
                                                                   long_name,
                                                                   &similarity);
                if (suggested && similarity >= 0.75) {
                    cap_str_appendf(&context->error_msg, " Did you mean '--%s'?\n", suggested);
                } else {
                    cap_str_append_cstr(&context->error_msg, "\n");
                }
                cap_write_usage_with_help_hint(context);
                return CAP_ERR_UNKNOWN_OPTION;
            }

            if (match->value_type == CAP_VALUE_TYPE_BOOL) {
                // Flag
                match->active             = true;
                match->out_value->boolean = match->flag_invert ? false : true;
            } else {
                if (value.length == 0) {
                    // No '=', assume space-delimited
                    if (argc == 0) {
                        cap_str_appendf(&context->error_msg, "Error: Expected value following option '%.*s'.\n", arg.length, arg.begin);
                        cap_write_usage_with_help_hint(context);
                        return CAP_ERR_MISSING_VALUE;
                    }
                    value = cap_sv_from_cstr(CAP_SHIFT(argv, argc));
                }
                CapResult r = cap_parse_option_value(context, match, arg, value);
                if (r != CAP_OK) {
                    cap_write_usage_with_help_hint(context);
                    return r;
                }
            }
        } break;

        case CAP_SINGLE_DASH_OPTION: {
            CapStringView without_dash = cap_sv_drop(arg, 1);

            for (size_t i = 0; i < without_dash.length; ++i) {
                CapStringView rest = cap_sv_drop(without_dash, i);
                char first_char = cap_sv_at(rest, 0);
                CapOption *match = cap_find_option_by_short_name(&context->options, first_char);
                if (!match) {
                    if (without_dash.length == 1) {
                        cap_str_appendf(&context->error_msg, "Error: Unrecognized option '%.*s'\n", arg.length, arg.begin);
                    } else {
                        cap_str_appendf(&context->error_msg, "Error: Unrecognized option '-%c' in cluster '%.*s'\n", first_char, arg.length, arg.begin);
                    }
                    cap_write_usage_with_help_hint(context);
                    return CAP_ERR_UNKNOWN_OPTION;
                }

                const char opt_arg_cstr[2] = {'-', first_char};
                CapStringView opt_arg = cap_sv_from_parts(opt_arg_cstr, 2);
                if (match->value_type == CAP_VALUE_TYPE_BOOL) {
                    // Flag
                    match->active             = true;
                    match->out_value->boolean = match->flag_invert ? false : true;
                    continue;
                }

                // Option expecting value, consume the rest as value, or take next argument as value
                CapStringView value;
                if (rest.length > 1) {
                    // Format is like `-j16`
                    value = cap_sv_drop(rest, 1); // Drop short name
                } else {
                    // Format is like `-j 16`
                    if (argc == 0) {
                        cap_str_appendf(&context->error_msg, "Error: Expected value following option '%.*s'.\n", opt_arg.length, opt_arg.begin);
                        cap_write_usage_with_help_hint(context);
                        return CAP_ERR_MISSING_VALUE;
                    }
                    value = cap_sv_from_cstr(CAP_SHIFT(argv, argc));
                }
                CapResult r = cap_parse_option_value(context, match, opt_arg, value);
                if (r != CAP_OK) {
                    cap_write_usage_with_help_hint(context);
                    return r;
                }
                break;
            }
        } break;

        case CAP_TERMINATOR: {
            context->terminator_encountered = true;
            if (context->capture_remainder) {
                *context->remainder_argc = argc;
                *context->remainder_argv = argv;
                return CAP_OK;
            }
        } break;

        default: {
            CAP_ASSERT(false && "Unhandled argument kind");
        } break;

        }
    }

    return CAP_OK;
}

static inline CapStringView
cap_parse_program_name(CapStringView path)
{
    size_t last_slash_index = cap_sv_rfind_char(path, '/');
    if (last_slash_index == SIZE_MAX) last_slash_index = 0;
#ifdef _WIN32
    size_t last_backslash_index = cap_sv_rfind_char(path, '\\');
    if (last_backslash_index == SIZE_MAX) last_backslash_index = 0;
    last_slash_index = CAP_MAX(last_slash_index, last_backslash_index);
#endif
    if (last_slash_index > 0) last_slash_index += 1;
    return cap_sv_drop(path, last_slash_index);
}

CAPDEF CapResult
cap_parse(CapContext *context,
          int         argc,
          char      **argv) CAP_NOEXCEPT
{
    CAP_ASSERT(!context->building_positional && "A positional is not done building, did you forget .done()?");
    CAP_ASSERT(!context->building_variadic   && "A variadic is not done building, did you forget .done()?");
    CAP_ASSERT(!context->building_option     && "An option is not done building, did you forget .done()?");

    cap_assert_options_dont_collide(context);

    cap_str_clear(&context->error_msg);

    CAP_ASSERT(argc >= 1);

    // TODO: Do one pass over argv and make a CapStringView array, and work on this throughout
    //       parsing instead. Should alleviate a lot of casting between const char * and string
    //       views, at the small cost of building up that array.

    context->program = cap_str_from_sv(cap_parse_program_name(cap_sv_from_cstr(CAP_SHIFT(argv, argc))));

    if (cap_was_help_requested(argc, argv)) {
        return CAP_HELP_REQUESTED;
    }

    if (context->program_version.size > 0 && cap_was_version_requested(argc, argv)) {
        return CAP_VERSION_REQUESTED;
    }

    CapResult r = cap_parse_user_args(context, argc, argv);
    if (r != CAP_OK) {
        return r;
    }

    if (context->variadic_spec.accept_variadic) {
        *context->variadic_spec.variadic_out_arr = context->variadic.buffer;
        *context->variadic_spec.variadic_out_len = context->variadic.size;
    }

    cap_fill_defaults(&context->options);

    if (!cap_verify_positionals(context)) {
        cap_write_usage_with_help_hint(context);
        return CAP_ERR_TOO_FEW_POSITIONALS;
    }

    if (!cap_check_required_options(context)) {
        cap_write_usage_with_help_hint(context);
        return CAP_ERR_MISSING_REQUIRED;
    }

    return CAP_OK;
}


CAPDEF CapAction cap_parse_and_handle(CapContext  *context,
                                      int          argc,
                                      char       **argv,
                                      int         *exit_code) CAP_NOEXCEPT
{
    const CapResult parse_result = cap_parse(context, argc, argv);

    if (parse_result == CAP_HELP_REQUESTED) {
        fprintf(stdout, "%s", cap_help_menu(context));
        if (exit_code) *exit_code = EXIT_SUCCESS;
        return CAP_EXIT;
    }

    if (parse_result == CAP_VERSION_REQUESTED) {
        fprintf(stdout, "%s %s\n", cap_program(context), cap_program_version(context));
        if (exit_code) *exit_code = EXIT_SUCCESS;
        return CAP_EXIT;
    }

    if (parse_result != CAP_OK) {
        fprintf(stderr, "%s", cap_error(context));
        if (exit_code) *exit_code = EXIT_FAILURE;
        return CAP_EXIT;
    }

    *exit_code = EXIT_SUCCESS;
    return CAP_KEEP_GOING;
}

// TODO: this can definitely be cleaned up. Separate smaller jobs
//       into separate functions, like cap_positionals_free,
//       cap_options_free etc.
CAPDEF void
cap_context_free(CapContext *context) CAP_NOEXCEPT
{
    cap_str_free(&context->program_description);
    cap_str_free(&context->program_version);

    CapOptions *options = &context->options;
    cap_darray_foreach(options, CapOption, option) {
        cap_str_free(&option->long_name);
        cap_str_free(&option->description);
        cap_str_free(&option->metavar);
        if (option->has_default_value && option->value_type == CAP_VALUE_TYPE_STRING) {
            CAP_FREE(option->default_value.string);
        }

        if (option->active && option->out_value && option->value_type == CAP_VALUE_TYPE_STRING) {
            CAP_FREE(option->out_value->string);
        }

        CapEnumEntries *enums = &option->enum_entries;
        cap_darray_foreach(enums, CapEnumEntry, entry) {
            CAP_FREE(entry->string);
        }
        cap_darray_free(enums);

    }
    cap_darray_free(options);

    if (context->variadic_spec.accept_variadic){
        cap_darray_foreach(&context->variadic, char *, pos) {
            CAP_FREE(*pos);
        }
        cap_str_free(&context->variadic_spec.label);
        cap_str_free(&context->variadic_spec.description);
        cap_darray_free(&context->variadic);
    }

    CapPositionals *positionals = &context->positionals;
    cap_darray_foreach(positionals, CapPositional, positional) {
        cap_str_free(&positional->label);
        cap_str_free(&positional->description);
        if (positional->active && positional->value && positional->value_type == CAP_VALUE_TYPE_STRING) {
            CAP_FREE((*positional->value).string);
        }
    }
    cap_darray_free(positionals);

    cap_str_free(&context->remainder_description);

    cap_str_free(&context->program);

    cap_str_free(&context->usage);
    cap_str_free(&context->help_menu);

    cap_str_free(&context->error_msg);

    memset(context, 0, sizeof(CapContext));
    CAP_FREE(context);
}

static inline void
cap_write_positionals_for_help_menu(CapContext *context, CapString *out)
{
    CapPositionals *positionals = &context->positionals;
    CapVariadicSpec *variadic_spec = &context->variadic_spec;

    if (positionals->size == 0 && !variadic_spec->accept_variadic) return;

    CapString temp = cap_str_init();

    size_t longest_label_length = 0;
    cap_darray_foreach(positionals, CapPositional, positional) {
        cap_str_clear(&temp);
        cap_format_positional_label(positional, &temp);
        longest_label_length = CAP_MAX(temp.size, longest_label_length);
    }
    if (variadic_spec->accept_variadic) {
        cap_str_clear(&temp);
        cap_format_variadic_label(variadic_spec, &temp);
        longest_label_length = CAP_MAX(temp.size, longest_label_length);
    }


    const size_t minimum_padding    = 4;
    const size_t leading_label_pad  = 2;
    const size_t description_column = leading_label_pad + longest_label_length + minimum_padding;

    cap_str_append_cstr(out, "Positionals:\n");

    cap_darray_foreach(positionals, CapPositional, positional) {
        cap_str_clear(&temp);
        cap_format_positional_label(positional, &temp);
        size_t len = temp.size;
        size_t padding = longest_label_length + minimum_padding - len;

        cap_str_appendf(out, "%*s%s%*s", (int)leading_label_pad, "", temp.buffer, (int)padding, "");

        cap_write_wrapped_text_at_column(positional->description.buffer,
                                         description_column,
                                         context->help_wrap_column,
                                         out);
        cap_str_append_cstr(out, "\n");
    }

    if (context->variadic_spec.accept_variadic) {
        cap_str_clear(&temp);
        cap_format_variadic_label(variadic_spec, &temp);
        size_t len = temp.size;
        size_t padding = longest_label_length + minimum_padding - len;

        cap_str_appendf(out, "%*s%s%*s", (int)leading_label_pad, "", temp.buffer, (int)padding, "");

        cap_write_wrapped_text_at_column(variadic_spec->description.buffer,
                                         description_column,
                                         context->help_wrap_column,
                                         out);

        // Print min and max if defined
        if (variadic_spec->min != 0 || variadic_spec->max != 0) {
            cap_str_appendf(out, "\n%*s(", (int)description_column, "");
            bool first = true;
            if (variadic_spec->min != 0) {
                cap_str_appendf(out, "min: %zu", variadic_spec->min);
                first = false;
            }
            if (variadic_spec->max != 0) {
                if (!first) { cap_str_append_cstr(out, ", "); }
                cap_str_appendf(out, "max: %zu", variadic_spec->max);
            }
            cap_str_append_cstr(out, ")");
        }
        cap_str_append_cstr(out, "\n");
    }

    cap_str_free(&temp);
}

static inline int
cap_option_sort_func(const void *option_a_erased, const void *option_b_erased)
{
    const CapOption *option_a = (const CapOption *)option_a_erased;
    const CapOption *option_b = (const CapOption *)option_b_erased;

    char a_short_name_cstr[2] = {'\0', '\0'};
    if (option_a->has_short_name) a_short_name_cstr[0] = option_a->short_name;

    char b_short_name_cstr[2] = {'\0', '\0'};
    if (option_b->has_short_name) b_short_name_cstr[0] = option_b->short_name;

    const char *a = option_a->has_short_name ? a_short_name_cstr : option_a->long_name.buffer;
    const char *b = option_b->has_short_name ? b_short_name_cstr : option_b->long_name.buffer;

    CAP_ASSERT(a && *a);
    CAP_ASSERT(b && *b);

    return strcmp(a, b);
}

// Used purely for help menu rendering
static inline CapOption
cap_make_virtual_help_option(void)
{
    CapOption option = CAP_ZERO_INIT;

#ifdef __cplusplus
    option.long_name = CapString{(char *)"help", 4, 4};
#else
    option.long_name = (CapString){"help", 4, 4};
#endif

    option.short_name     = 'h';
    option.has_short_name = true;

#ifdef __cplusplus
    option.description = CapString{(char *)"Show this help menu.", 20, 20};
#else
    option.description = (CapString){"Show this help menu.", 20, 20};
#endif

    option.value_type     = CAP_VALUE_TYPE_BOOL;

    return option;
}

// Used purely for help menu rendering
static inline CapOption
cap_make_virtual_version_option(void)
{
    CapOption option = CAP_ZERO_INIT;

#ifdef __cplusplus
    option.long_name = CapString{(char *)"version", 7, 7};
#else
    option.long_name = (CapString){"version", 7, 7};
#endif

    option.short_name        = 'V';
    option.has_short_name    = true;

#ifdef __cplusplus
    option.description = CapString{(char *)"Show program version.", 21, 21};
#else
    option.description = (CapString){"Show program version.", 21, 21};
#endif

    option.value_type        = CAP_VALUE_TYPE_BOOL;

    return option;
}

static inline void
cap_write_option_for_help_menu(CapContext *context, CapString *out)
{
    CapOptions *options = &context->options;

    CapOptions sorted_options = CAP_ZERO_INIT;
    cap_darray_copy(options, &sorted_options);

    // Add virtual help option since it's an internal implementation
    // detail, and not part of the user-managed option list
    CapOption help = cap_make_virtual_help_option();
    cap_darray_append(&sorted_options, help);

    if (context->program_version.size > 0) {
        // Add virtual version option since it's an internal implementation
        // detail, and not part of the user-managed option list
        CapOption version = cap_make_virtual_version_option();
        cap_darray_append(&sorted_options, version);
    }

    qsort(sorted_options.buffer, sorted_options.size, sizeof(CapOption), cap_option_sort_func);

    CapString temp = cap_str_init();

    size_t longest_short_name_length = 0;
    size_t longest_long_name_length  = 0;
    size_t longest_metavar_length = 0;

    cap_darray_foreach(&sorted_options, CapOption, option) {
        CAP_ASSERT(option->has_short_name || option->long_name.size > 0);

        if (option->has_short_name) {
            cap_str_clear(&temp);
            cap_format_option_short_name(option, &temp);
            longest_short_name_length = CAP_MAX(longest_short_name_length, temp.size);
        }

        if (option->long_name.size > 0) {
            cap_str_clear(&temp);
            cap_format_option_long_name(option, &temp);
            longest_long_name_length = CAP_MAX(longest_long_name_length, temp.size);
        }

        if (option->value_type != CAP_VALUE_TYPE_BOOL) {
            cap_str_clear(&temp);
            cap_format_option_metavar(option, &temp);
            longest_metavar_length = CAP_MAX(longest_metavar_length, temp.size);
        }
    }


    size_t max_label_width = 0;
    cap_darray_foreach(&sorted_options, CapOption, option) {
        size_t width = 0;

        if (option->has_short_name) {
            cap_str_clear(&temp);
            cap_format_option_short_name(option, &temp);
            width += temp.size; // "-x"
            if (option->long_name.size > 0) width += 2;     // ", "
        } else {
            width += longest_short_name_length + 2; // indent used when short is missing
        }

        if (option->long_name.size > 0) {
            cap_str_clear(&temp);
            cap_format_option_long_name(option, &temp);
            width += temp.size; // "--long"
        }

        if (option->value_type != CAP_VALUE_TYPE_BOOL && option->metavar.size > 0) {
            cap_str_clear(&temp);
            cap_format_option_metavar(option, &temp);
            width += 1 + temp.size; // space + "<metavar>"
        }

        max_label_width = CAP_MAX(max_label_width, width);
    }


    const size_t minimum_padding    = 4;
    const size_t leading_label_pad  = 2;
    const size_t description_column = leading_label_pad + max_label_width + minimum_padding;
    const size_t marker_pad         = 1;

    cap_str_append_cstr(out, "Options:\n");
    cap_darray_foreach(&sorted_options, CapOption, option) {
        cap_str_appendf(out, "%*s", (int)leading_label_pad, ""); // leading label pad (not counted in label width)

        size_t current_label_width = 0;

        if (option->has_short_name) {
            cap_str_clear(&temp);
            cap_format_option_short_name(option, &temp);
            cap_str_append_str(out, &temp);
            current_label_width += temp.size;

            if (option->long_name.size > 0) {
                cap_str_append_cstr(out, ", ");
                current_label_width += 2;
            }
        } else {
            cap_str_appendf(out, "%*s  ", (int)longest_short_name_length, "");
            current_label_width += longest_short_name_length + 2;
        }

        if (option->long_name.size > 0) {
            cap_str_clear(&temp);
            cap_format_option_long_name(option, &temp);
            cap_str_append_str(out, &temp);
            current_label_width += temp.size;
        }

        if (option->value_type != CAP_VALUE_TYPE_BOOL && option->metavar.size > 0) {
            cap_str_clear(&temp);
            cap_format_option_metavar(option, &temp);
            cap_str_append_char(out, ' ');
            cap_str_append_str(out, &temp);
            current_label_width += 1 + temp.size;
        }

        const size_t padding = minimum_padding + (max_label_width - current_label_width);
        cap_str_appendf(out, "%*s", (int)padding, "");

        cap_write_wrapped_text_at_column(option->description.buffer,
                                         description_column,
                                         context->help_wrap_column,
                                         out);

        if (option->value_type == CAP_VALUE_TYPE_ENUM) {
            cap_str_appendf(out, "\n%*s", (int)description_column + marker_pad, "");
            cap_str_clear(&temp);
            cap_str_append_cstr(&temp, "(choices: ");
            size_t idx = 0;
            cap_darray_foreach(&option->enum_entries, CapEnumEntry, entry) {
                if (idx++ > 0) cap_str_append_cstr(&temp, ", ");
                cap_str_appendf(&temp, "%s", entry->string);
            }
            cap_str_append_char(&temp, ')');
            cap_write_wrapped_text_at_column(temp.buffer,
                                             description_column + marker_pad + 1,
                                             context->help_wrap_column,
                                             out);
        }

        if (option->has_default_value) {
            cap_str_appendf(out, "\n%*s", (int)description_column + marker_pad, "");
            cap_str_clear(&temp);
            cap_str_append_cstr(&temp, "(default: ");
            switch (option->value_type) {
            case CAP_VALUE_TYPE_INT:
                cap_str_appendf(&temp, "%d", option->default_value.integer);
                break;
            case CAP_VALUE_TYPE_DOUBLE:
                cap_str_appendf(&temp, "%f", option->default_value.decimal);
                break;
            case CAP_VALUE_TYPE_STRING:
                if (option->default_value.string) {
                    cap_str_appendf(&temp, "%s", option->default_value.string);
                } else {
                    cap_str_append_cstr(&temp, "(null)");
                }
                break;
            case CAP_VALUE_TYPE_ENUM:
                cap_darray_foreach(&option->enum_entries, CapEnumEntry, entry) {
                    if (entry->integer == option->default_value.enumeration) {
                        cap_str_appendf(&temp, "%s", entry->string);
                        break;
                    }
                }
                break;
            default:
                CAP_ASSERT(false && "unhandled value type");
                break;
            }
            cap_str_append_cstr(&temp, ")");
            cap_write_wrapped_text_at_column(temp.buffer,
                                             description_column + marker_pad + 1,
                                             context->help_wrap_column,
                                             out);
        }

        if (option->required) {
            cap_str_appendf(out, "\n%*s(required)", (int)description_column + marker_pad, "");
        }

        cap_str_append_cstr(out, "\n");
    }

    cap_str_free(&temp);

    cap_darray_free(&sorted_options);
}

static inline void
cap_write_syntax_notes_for_help_menu(CapContext *context, CapString *out)
{
    cap_str_appendf(out, "Syntax:\n");

    CapOption *long_name_equivalence_example  = NULL;
    CapOption *short_name_equivalence_example = NULL;

#define CLUSTERING_FLAG_EXAMPLES_MAX 3
    size_t         clustering_flag_examples_count = 0;
    CapOption *clustering_flag_examples[CLUSTERING_FLAG_EXAMPLES_MAX];
    CapOption *clustering_option_example = NULL;

    cap_darray_foreach(&context->options, CapOption, option) {
        if (option->long_name.size > 0 && option->value_type != CAP_VALUE_TYPE_BOOL) {
            long_name_equivalence_example = option;
        }
        if (option->has_short_name && option->value_type != CAP_VALUE_TYPE_BOOL) {
            short_name_equivalence_example = option;
        }
        if (option->has_short_name) {
            if ((option->value_type == CAP_VALUE_TYPE_BOOL) && (clustering_flag_examples_count < CLUSTERING_FLAG_EXAMPLES_MAX)) {
                clustering_flag_examples[clustering_flag_examples_count++] = option;
            } else if ((option->value_type != CAP_VALUE_TYPE_BOOL) && !clustering_option_example) {
                clustering_option_example = option;
            }
        }
    }
#undef CLUSTERING_FLAG_EXAMPLES_MAX

    CapString temp = cap_str_init();

    if (context->options.size > 0 && (context->variadic_spec.accept_variadic || (context->positionals.size > 0))) {
        cap_str_clear(&temp);
        cap_str_append_cstr(&temp, "Options may appear before, between, or after positional arguments.");

        cap_str_append_cstr(out, "  * ");
        cap_write_wrapped_text_at_column(temp.buffer, 4, context->help_wrap_column, out);
        cap_str_append_char(out, '\n');
    }

    if (long_name_equivalence_example) {
        cap_str_clear(&temp);
        cap_str_appendf(&temp, "For long options, both `--%s=<%s>` and `--%s <%s>` are valid.\n",
                       long_name_equivalence_example->long_name.buffer, long_name_equivalence_example->metavar.buffer,
                       long_name_equivalence_example->long_name.buffer, long_name_equivalence_example->metavar.buffer);

        cap_str_append_cstr(out, "  * ");
        cap_write_wrapped_text_at_column(temp.buffer, 4, context->help_wrap_column, out);
        cap_str_append_char(out, '\n');
    }

    if (short_name_equivalence_example) {
        cap_str_clear(&temp);
        cap_str_appendf(&temp, "For short options, both `-%c<%s>` and `-%c <%s>` are valid.\n",
                       short_name_equivalence_example->short_name, short_name_equivalence_example->metavar.buffer,
                       short_name_equivalence_example->short_name, short_name_equivalence_example->metavar.buffer);

        cap_str_append_cstr(out, "  * ");
        cap_write_wrapped_text_at_column(temp.buffer, 4, context->help_wrap_column, out);
        cap_str_append_char(out, '\n');
    }

    if (clustering_flag_examples_count > 1) {
        cap_str_clear(&temp);
        cap_str_append_cstr(&temp, "Clustering of short options: `-");
        for (size_t i = 0; i < clustering_flag_examples_count; ++i) cap_str_append_char(&temp, clustering_flag_examples[i]->short_name);
        cap_str_append_cstr(&temp, "' is equivalent to '");
        for (size_t i = 0; i < clustering_flag_examples_count; ++i) {
            cap_str_append_char(&temp, '-');
            cap_str_append_char(&temp, clustering_flag_examples[i]->short_name);
            if (i < clustering_flag_examples_count-1) cap_str_append_char(&temp, ' ');
        }
        cap_str_append_cstr(&temp, "`.");

        cap_str_append_cstr(out, "  * ");
        cap_write_wrapped_text_at_column(temp.buffer, 4, context->help_wrap_column, out);
        cap_str_append_char(out, '\n');
    }

    if (clustering_flag_examples_count > 0 && clustering_option_example) {
        cap_str_clear(&temp);
        cap_str_append_cstr(&temp, "When a short option expects a value, it must be last in a cluster: `-");
        const size_t example_count = CAP_MIN(2, clustering_flag_examples_count); // Reduce example count when adding option
        for (size_t i = 0; i < example_count; ++i) cap_str_append_char(&temp, clustering_flag_examples[i]->short_name);
        cap_str_append_char(&temp, clustering_option_example->short_name);
        cap_str_appendf(&temp, "<%s>", clustering_option_example->metavar.buffer);
        cap_str_append_cstr(&temp, "` is equivalent to `");
        for (size_t i = 0; i < example_count; ++i) {
            cap_str_append_char(&temp, '-');
            cap_str_append_char(&temp, clustering_flag_examples[i]->short_name);
            if (i < example_count-1) cap_str_append_char(&temp, ' ');
        }
        cap_str_appendf(&temp, " -%c<%s>`.", clustering_option_example->short_name, clustering_option_example->metavar.buffer);

        cap_str_append_cstr(out, "  * ");
        cap_write_wrapped_text_at_column(temp.buffer, 4, context->help_wrap_column, out);
        cap_str_append_char(out, '\n');
    }

    if (context->positionals.size > 0 || context->variadic_spec.accept_variadic || context->capture_remainder) {
        cap_str_clear(&temp);
        if (context->capture_remainder) {
            cap_str_appendf(&temp, "Arguments following `--`: %s.\n", context->remainder_description.buffer);
        } else  {
            cap_str_append_cstr(&temp, "`--` ends option parsing; all following arguments are treated as positional arguments.\n");
        }

        cap_str_append_cstr(out, "  * ");
        cap_write_wrapped_text_at_column(temp.buffer, 4, context->help_wrap_column, out);
        cap_str_append_char(out, '\n');
    }

    cap_str_free(&temp);
}

static inline void
cap_generate_help_menu(CapContext *context)
{
    CapString *help = &context->help_menu;
    cap_str_clear(help);

    if (context->program_description.size > 0) {
        CapString temp = cap_str_init();
        if (context->program.size > 0) {
            cap_str_appendf(&temp, "%s - ", context->program.buffer);
        }
        cap_write_wrapped_text_at_column(context->program_description.buffer, temp.size, context->help_wrap_column, &temp);
        cap_write_wrapped_text_at_column(temp.buffer, 0, context->help_wrap_column, help);
        cap_str_append_cstr(help, "\n\n");
        cap_str_free(&temp);
    }

    cap_write_usage_to_str(context, help);

    if (context->positionals.size > 0 || context->variadic_spec.accept_variadic) {
        cap_str_append_cstr(help, "\n");
        cap_write_positionals_for_help_menu(context, help);
    }

    cap_str_append_cstr(help, "\n");
    cap_write_option_for_help_menu(context, help);

    cap_str_append_cstr(help, "\n");
    cap_write_syntax_notes_for_help_menu(context, help);
}

CAPDEF const char *
cap_help_menu(CapContext *context) CAP_NOEXCEPT
{
    cap_generate_help_menu(context);
    return context->help_menu.buffer;
}

CAPDEF const char *
cap_error(CapContext *context) CAP_NOEXCEPT
{
    return context->error_msg.buffer;
}

static inline void
cap_touch_context(CapContext *ctx)
{
    active_ctx = ctx;
}

static inline void
cap_assert_active_context(CAP_NO_PARAMS)
{
    CAP_ASSERT(active_ctx && "There is no active CapContext");
}

// String positional builder

static inline CapStrPosBuilder *cap_get_str_pos_builder(CAP_NO_PARAMS);

static inline void
cap_assert_building_positional(CAP_NO_PARAMS)
{
    CAP_ASSERT(active_ctx->building_positional && "There is no positional currently being built.");
}

static inline CapStrPosBuilder *
cap_str_pos_builder_set_label(const char *label)
{
    cap_assert_active_context();
    cap_assert_building_positional();

    CapPositional *pos = &active_ctx->positionals.buffer[active_ctx->positionals.size-1];
    cap_str_clear(&pos->label);
    cap_str_append_cstr(&pos->label, label);
    return cap_get_str_pos_builder();
}

static inline CapStrPosBuilder *
cap_str_pos_builder_set_description(const char *description)
{
    cap_assert_active_context();
    cap_assert_building_positional();

    CapPositional *pos = &active_ctx->positionals.buffer[active_ctx->positionals.size-1];
    cap_str_clear(&pos->description);
    cap_str_append_cstr(&pos->description, description);
    return cap_get_str_pos_builder();
}

static inline void
cap_str_pos_builder_done(CAP_NO_PARAMS)
{
    cap_assert_active_context();
    cap_assert_building_positional();

    CapPositional *pos = &active_ctx->positionals.buffer[active_ctx->positionals.size-1];
    CAP_UNUSED(pos); // When CAP_ASSERT is undefined
    CAP_ASSERT(pos->label.size > 0 && "A positional must have a label, use .label()");
    CAP_ASSERT(pos->description.size > 0 && "A positional must have a description, use .description()");
    active_ctx->building_positional = false;
}

static CapStrPosBuilder cap_str_pos_builder = {
    cap_str_pos_builder_set_label,
    cap_str_pos_builder_set_description,
    cap_str_pos_builder_done
};

static inline CapStrPosBuilder *
cap_get_str_pos_builder(CAP_NO_PARAMS)
{
    return &cap_str_pos_builder;
}

CAPDEF CapStrPosBuilder *
cap_positional_string(CapContext *ctx, const char **out_value) CAP_NOEXCEPT
{
    cap_touch_context(ctx);

    CAP_ASSERT(!ctx->building_positional && "A positional is already being built, did you forget .done()?");
    ctx->building_positional = true;

    CapPositional pos = CAP_ZERO_INIT;

    pos.value_type  = CAP_VALUE_TYPE_STRING;
    pos.value       = (CapValue *)out_value;

    cap_darray_append(&ctx->positionals, pos);
    return cap_get_str_pos_builder();
}



// Variadic positional builder

static inline CapVarPosBuilder *cap_get_var_pos_builder(CAP_NO_PARAMS);

static inline void
cap_assert_building_variadic(CAP_NO_PARAMS)
{
    CAP_ASSERT(active_ctx->building_variadic && "There is no variadic currently being built.");
}

static inline CapVarPosBuilder *
cap_var_pos_builder_set_label(const char *label)
{
    cap_assert_active_context();
    cap_assert_building_variadic();

    CapVariadicSpec *spec = &active_ctx->variadic_spec;
    cap_str_clear(&spec->label);
    cap_str_append_cstr(&spec->label, label);
    return cap_get_var_pos_builder();
}

static inline CapVarPosBuilder *
cap_var_pos_builder_set_description(const char *description)
{
    cap_assert_active_context();
    cap_assert_building_variadic();

    CapVariadicSpec *spec = &active_ctx->variadic_spec;
    cap_str_clear(&spec->description);
    cap_str_append_cstr(&spec->description, description);
    return cap_get_var_pos_builder();
}

static inline CapVarPosBuilder *
cap_var_pos_builder_set_min(size_t min)
{
    cap_assert_active_context();
    cap_assert_building_variadic();

    CapVariadicSpec *spec = &active_ctx->variadic_spec;
    spec->min = min;
    return cap_get_var_pos_builder();
}

static inline CapVarPosBuilder *
cap_var_pos_builder_set_max(size_t max)
{
    cap_assert_active_context();
    cap_assert_building_variadic();

    CapVariadicSpec *spec = &active_ctx->variadic_spec;
    spec->max = max;
    return cap_get_var_pos_builder();
}

static inline void
cap_var_pos_builder_done(CAP_NO_PARAMS)
{
    cap_assert_active_context();
    cap_assert_building_variadic();

    CapVariadicSpec *spec = &active_ctx->variadic_spec;
    CAP_UNUSED(spec); // When CAP_ASSERT is undefined
    CAP_ASSERT(spec->label.size > 0 && "A variadic must have a label, use .label()");
    CAP_ASSERT(spec->description.size > 0 && "A variadic must have a description, use .description()");
    active_ctx->building_variadic = false;
}

static CapVarPosBuilder cap_var_pos_builder = {
    cap_var_pos_builder_set_label,
    cap_var_pos_builder_set_description,
    cap_var_pos_builder_set_min,
    cap_var_pos_builder_set_max,
    cap_var_pos_builder_done
};

static inline CapVarPosBuilder *
cap_get_var_pos_builder(CAP_NO_PARAMS)
{
    return &cap_var_pos_builder;
}


CAPDEF CapVarPosBuilder *
cap_variadic(CapContext *ctx, char ***out_arr, size_t *out_len) CAP_NOEXCEPT
{
    cap_touch_context(ctx);

    CAP_ASSERT(!ctx->building_variadic && "A positional is already being built, did you forget .done()?");
    CAP_ASSERT(!ctx->variadic_spec.accept_variadic && "A variadic has already been defined, cannot have multiple.");
    ctx->building_variadic = true;

    ctx->variadic_spec.accept_variadic   = true;
    ctx->variadic_spec.variadic_out_arr = out_arr;
    ctx->variadic_spec.variadic_out_len = out_len;

    return cap_get_var_pos_builder();
}



// Generic option builder

static inline void
cap_assert_building_option(CAP_NO_PARAMS)
{
    CAP_ASSERT(active_ctx->building_option && "There is no option currently being built.");
}

static inline CapOption *
cap_get_last_option_from_active_context(CAP_NO_PARAMS)
{
    return &active_ctx->options.buffer[active_ctx->options.size-1];
}

static inline void
cap_builder_set_long_name(const char *long_name)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    cap_str_clear(&option->long_name);
    cap_str_append_cstr(&option->long_name, long_name);
}

static inline void
cap_builder_set_short_name(char short_name)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    option->short_name = short_name;
    option->has_short_name = true;
}

static inline void
cap_builder_set_description(const char *description)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    cap_str_clear(&option->description);
    cap_str_append_cstr(&option->description, description);
}

static inline void
cap_builder_set_metavar(const char *metavar)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    cap_str_clear(&option->metavar);
    cap_str_append_cstr(&option->metavar, metavar);
}

static inline void
cap_builder_required(CAP_NO_PARAMS)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    option->required = true;
}

static inline void
cap_opt_builder_done(CAP_NO_PARAMS)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    CAP_ASSERT(option->description.size > 0 && "An option must have a description, use .description()");
    CAP_ASSERT((option->long_name.size > 0 || option->short_name) &&
               "An option must have long name or short name, use .short_name() or .long_name()");

    CAP_ASSERT(!(option->required && option->has_default_value) &&
               "An option cannot be both required and have a default value");

    if (option->metavar.size == 0) cap_str_append_cstr(&option->metavar, cap_strdup("value"));

    active_ctx->building_option = false;
    active_ctx = NULL;
}



// Flag builder

static inline CapFlagBuilder *cap_get_flag_builder(CAP_NO_PARAMS);

static inline CapFlagBuilder *
cap_flag_builder_set_long_name(const char *long_name)
{
    cap_builder_set_long_name(long_name);
    return cap_get_flag_builder();
}

static inline CapFlagBuilder *
cap_flag_builder_set_short_name(char short_name)
{
    cap_builder_set_short_name(short_name);
    return cap_get_flag_builder();
}

static inline CapFlagBuilder *
cap_flag_builder_set_description(const char *description)
{
    cap_builder_set_description(description);
    return cap_get_flag_builder();
}

static inline CapFlagBuilder *
cap_flag_builder_invert(CAP_NO_PARAMS)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    option->flag_invert = true;
    option->out_value->boolean = true;
    return cap_get_flag_builder();
}

static inline void
cap_flag_builder_done(CAP_NO_PARAMS)
{
    cap_opt_builder_done();
}

static CapFlagBuilder cap_flag_builder = {
    cap_flag_builder_set_long_name,
    cap_flag_builder_set_short_name,
    cap_flag_builder_set_description,
    cap_flag_builder_invert,
    cap_flag_builder_done
};

static inline CapFlagBuilder *
cap_get_flag_builder(CAP_NO_PARAMS)
{
    return &cap_flag_builder;
}

CAPDEF CapFlagBuilder *
cap_flag(CapContext *ctx, bool *out_value) CAP_NOEXCEPT
{
    cap_touch_context(ctx);

    CAP_ASSERT(!ctx->building_option && "An option is already being built, did you forget .done()?");
    ctx->building_option = true;

    CapOption option  = CAP_ZERO_INIT;
    option.value_type = CAP_VALUE_TYPE_BOOL;
    option.out_value  = (CapValue *)out_value;
    cap_darray_append(&ctx->options, option);

    return cap_get_flag_builder();
}

// Int option builder

static inline CapIntOptBuilder *cap_get_int_opt_builder(CAP_NO_PARAMS);

static inline CapIntOptBuilder *
cap_int_opt_builder_set_long_name(const char *long_name)
{
    cap_builder_set_long_name(long_name);
    return cap_get_int_opt_builder();
}

static inline CapIntOptBuilder *
cap_int_opt_builder_set_short_name(char short_name)
{
    cap_builder_set_short_name(short_name);
    return cap_get_int_opt_builder();
}

static inline CapIntOptBuilder *
cap_int_opt_builder_set_description(const char *description)
{
    cap_builder_set_description(description);
    return cap_get_int_opt_builder();
}

static inline CapIntOptBuilder *
cap_int_opt_builder_set_metavar(const char *metavar)
{
    cap_builder_set_metavar(metavar);
    return cap_get_int_opt_builder();
}

static inline CapIntOptBuilder *
cap_int_opt_builder_set_default_value(int default_value)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    option->default_value.integer = default_value;
    option->has_default_value     = true;
    return cap_get_int_opt_builder();
}

static inline CapIntOptBuilder *
cap_int_opt_builder_required(CAP_NO_PARAMS)
{
    cap_builder_required();
    return cap_get_int_opt_builder();
}

static inline void
cap_int_opt_builder_done(CAP_NO_PARAMS)
{
    cap_opt_builder_done();
}

static CapIntOptBuilder cap_int_opt_builder = {
    cap_int_opt_builder_set_long_name,
    cap_int_opt_builder_set_short_name,
    cap_int_opt_builder_set_description,
    cap_int_opt_builder_set_metavar,
    cap_int_opt_builder_set_default_value,
    cap_int_opt_builder_required,
    cap_int_opt_builder_done
};

static inline CapIntOptBuilder *
cap_get_int_opt_builder(CAP_NO_PARAMS)
{
    return &cap_int_opt_builder;
}

CAPDEF CapIntOptBuilder *
cap_option_int(CapContext *ctx, int *out_value) CAP_NOEXCEPT
{
    cap_touch_context(ctx);

    CAP_ASSERT(!ctx->building_option && "An option is already being built, did you forget .done()?");
    ctx->building_option = true;

    CapOption option = CAP_ZERO_INIT;
    option.value_type = CAP_VALUE_TYPE_INT;
    option.out_value  = (CapValue *)out_value;
    cap_darray_append(&ctx->options, option);

    return cap_get_int_opt_builder();
}



// Double option builder

static inline CapDoubleOptBuilder *cap_get_double_opt_builder(CAP_NO_PARAMS);

static inline CapDoubleOptBuilder *
cap_double_opt_builder_set_long_name(const char *long_name)
{
    cap_builder_set_long_name(long_name);
    return cap_get_double_opt_builder();
}

static inline CapDoubleOptBuilder *
cap_double_opt_builder_set_short_name(char short_name)
{
    cap_builder_set_short_name(short_name);
    return cap_get_double_opt_builder();
}

static inline CapDoubleOptBuilder *
cap_double_opt_builder_set_description(const char *description)
{
    cap_builder_set_description(description);
    return cap_get_double_opt_builder();
}

static inline CapDoubleOptBuilder *
cap_double_opt_builder_set_metavar(const char *metavar)
{
    cap_builder_set_metavar(metavar);
    return cap_get_double_opt_builder();
}

static inline CapDoubleOptBuilder *
cap_double_opt_builder_set_default_value(double default_value)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    option->default_value.decimal = default_value;
    option->has_default_value     = true;
    return cap_get_double_opt_builder();
}

static inline CapDoubleOptBuilder *
cap_double_opt_builder_required(CAP_NO_PARAMS)
{
    cap_builder_required();
    return cap_get_double_opt_builder();
}

static inline void
cap_double_opt_builder_done(CAP_NO_PARAMS)
{
    cap_opt_builder_done();
}

static CapDoubleOptBuilder cap_double_opt_builder = {
    cap_double_opt_builder_set_long_name,
    cap_double_opt_builder_set_short_name,
    cap_double_opt_builder_set_description,
    cap_double_opt_builder_set_metavar,
    cap_double_opt_builder_set_default_value,
    cap_double_opt_builder_required,
    cap_double_opt_builder_done
};

static inline CapDoubleOptBuilder *
cap_get_double_opt_builder(CAP_NO_PARAMS)
{
    return &cap_double_opt_builder;
}

CAPDEF CapDoubleOptBuilder *
cap_option_double(CapContext *ctx, double *out_value) CAP_NOEXCEPT
{
    cap_touch_context(ctx);

    CAP_ASSERT(!ctx->building_option && "An option is already being built, did you forget .done()?");
    ctx->building_option = true;

    CapOption option = CAP_ZERO_INIT;
    option.value_type = CAP_VALUE_TYPE_DOUBLE;
    option.out_value  = (CapValue *)out_value;
    cap_darray_append(&ctx->options, option);

    return cap_get_double_opt_builder();
}




// String Option Builder

static inline CapStrOptBuilder *cap_get_str_opt_builder(CAP_NO_PARAMS);

static inline CapStrOptBuilder *
cap_str_opt_builder_set_long_name(const char *long_name)
{
    cap_builder_set_long_name(long_name);
    return cap_get_str_opt_builder();
}

static inline CapStrOptBuilder *
cap_str_opt_builder_set_short_name(char short_name)
{
    cap_builder_set_short_name(short_name);
    return cap_get_str_opt_builder();
}

static inline CapStrOptBuilder *
cap_str_opt_builder_set_description(const char *description)
{
    cap_builder_set_description(description);
    return cap_get_str_opt_builder();
}

static inline CapStrOptBuilder *
cap_str_opt_builder_set_metavar(const char *metavar)
{
    cap_builder_set_metavar(metavar);
    return cap_get_str_opt_builder();
}

static inline CapStrOptBuilder *
cap_str_opt_builder_set_default_value(const char *default_value)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    option->default_value.string = cap_strdup(default_value);
    option->has_default_value    = true;
    return cap_get_str_opt_builder();
}

static inline CapStrOptBuilder *
cap_str_opt_builder_required(CAP_NO_PARAMS)
{
    cap_builder_required();
    return cap_get_str_opt_builder();
}

static inline void
cap_str_opt_builder_done(CAP_NO_PARAMS)
{
    cap_opt_builder_done();
}

static CapStrOptBuilder cap_str_opt_builder = {
    cap_str_opt_builder_set_long_name,
    cap_str_opt_builder_set_short_name,
    cap_str_opt_builder_set_description,
    cap_str_opt_builder_set_metavar,
    cap_str_opt_builder_set_default_value,
    cap_str_opt_builder_required,
    cap_str_opt_builder_done
};

static inline CapStrOptBuilder *
cap_get_str_opt_builder(CAP_NO_PARAMS)
{
    return &cap_str_opt_builder;
}


CAPDEF CapStrOptBuilder *
cap_option_string(CapContext *ctx, const char **out_value) CAP_NOEXCEPT
{
    cap_touch_context(ctx);

    CAP_ASSERT(!ctx->building_option && "An option is already being built, did you forget .done()?");
    ctx->building_option = true;

    CapOption option = CAP_ZERO_INIT;
    option.value_type = CAP_VALUE_TYPE_STRING;
    option.out_value  = (CapValue *)out_value;
    cap_darray_append(&ctx->options, option);

    return cap_get_str_opt_builder();
}

// Enum option

static inline CapEnumOptBuilder *cap_get_enum_opt_builder(CAP_NO_PARAMS);

static inline CapEnumOptBuilder *
cap_enum_opt_builder_set_long_name(const char *long_name)
{
    cap_builder_set_long_name(long_name);
    return cap_get_enum_opt_builder();
}

static inline CapEnumOptBuilder *
cap_enum_opt_builder_set_short_name(char short_name)
{
    cap_builder_set_short_name(short_name);
    return cap_get_enum_opt_builder();
}

static inline CapEnumOptBuilder *
cap_enum_opt_builder_set_description(const char *description)
{
    cap_builder_set_description(description);
    return cap_get_enum_opt_builder();
}

static inline CapEnumOptBuilder *
cap_enum_opt_builder_set_metavar(const char *metavar)
{

    cap_builder_set_metavar(metavar);
    return cap_get_enum_opt_builder();
}

static inline CapEnumOptBuilder *
cap_enum_opt_builder_add_entry(int enumeration, const char *string_val)
{
    cap_assert_active_context();
    cap_assert_building_option();
    CapOption *option = cap_get_last_option_from_active_context();

    CapEnumEntry entry = {enumeration, cap_strdup(string_val)};
    cap_darray_append(&option->enum_entries, entry);
    return cap_get_enum_opt_builder();
}

static inline CapEnumOptBuilder *
cap_enum_opt_builder_set_default_value(int default_value)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();

    option->default_value.enumeration = (intmax_t)default_value;
    option->has_default_value         = true;
    return cap_get_enum_opt_builder();
}

static inline CapEnumOptBuilder *
cap_enum_opt_builder_required(CAP_NO_PARAMS)
{
    cap_builder_required();
    return cap_get_enum_opt_builder();
}

static inline CapEnumOptBuilder *
cap_enum_opt_builder_case_insensitive(CAP_NO_PARAMS)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();

    option->enum_is_case_insensitive = true;
    return cap_get_enum_opt_builder();
}

static inline void
cap_enum_opt_builder_done(CAP_NO_PARAMS)
{
    cap_assert_active_context();
    cap_assert_building_option();

    CapOption *option = cap_get_last_option_from_active_context();
    CAP_UNUSED(option); // When CAP_ASSERT is undefined
    CAP_ASSERT(option->enum_entries.size > 0 && "An enum option must have at least one entry, use .entry()");

    cap_opt_builder_done();
}

static CapEnumOptBuilder cap_enum_opt_builder = {
    cap_enum_opt_builder_set_long_name,
    cap_enum_opt_builder_set_short_name,
    cap_enum_opt_builder_set_description,
    cap_enum_opt_builder_set_metavar,
    cap_enum_opt_builder_add_entry,
    cap_enum_opt_builder_set_default_value,
    cap_enum_opt_builder_required,
    cap_enum_opt_builder_case_insensitive,
    cap_enum_opt_builder_done
};

static inline CapEnumOptBuilder *
cap_get_enum_opt_builder(CAP_NO_PARAMS)
{
    return &cap_enum_opt_builder;
}

CAPDEF CapEnumOptBuilder *
cap_option_enum_(CapContext *ctx,
                 void       *out_value,
                 size_t      type_size,
                 bool        is_signed) CAP_NOEXCEPT
{
    cap_touch_context(ctx);

    CAP_ASSERT(!ctx->building_option && "An option is already being built, did you forget .done()?");
    ctx->building_option = true;

    CapOption option         = CAP_ZERO_INIT;
    option.value_type          = CAP_VALUE_TYPE_ENUM;
    option.enum_type_size      = type_size;
    option.enum_type_is_signed = is_signed;
    option.out_value           = (CapValue *)out_value;
    cap_darray_append(&ctx->options, option);

    return &cap_enum_opt_builder;
}

CAPDEF void
cap_capture_remainder(CapContext *context,
                      int        *out_remainder_argc,
                      char     ***out_remainder_argv,
                      const char *remainder_description) CAP_NOEXCEPT
{
    context->capture_remainder = true;
    context->remainder_argc = out_remainder_argc;
    context->remainder_argv = out_remainder_argv;

    cap_str_clear(&context->remainder_description);
    cap_str_append_cstr(&context->remainder_description, remainder_description);
}

CAP_ENABLE_DUMB_WARNINGS()

#endif /* CAP_IMPLEMENTATION */

#endif /* CAP_H_ */


/*
  LICENSE

  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
