/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef builtin_intl_CommonFunctions_h
#define builtin_intl_CommonFunctions_h

#include "mozilla/Assertions.h"
#include "mozilla/TypeTraits.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "builtin/intl/ICUHeader.h"
#include "js/RootingAPI.h"
#include "js/Vector.h"
#include "vm/String.h"

namespace JS { class Value; }

class JSObject;

namespace js {

namespace intl {

/**
 * Initialize a new Intl.* object using the named self-hosted function.
 */
extern bool
InitializeObject(JSContext* cx, HandleObject obj, Handle<PropertyName*> initializer,
                 HandleValue locales, HandleValue options);

/**
 * Initialize an existing object as an Intl.* object using the named
 * self-hosted function.  This is only for a few old Intl.* constructors, for
 * legacy reasons -- new ones should use the function above instead.
 */

enum class DateTimeFormatOptions
{
    Standard,
    EnableMozExtensions,
};

extern bool
LegacyIntlInitialize(JSContext* cx, HandleObject obj, Handle<PropertyName*> initializer,
                     HandleValue thisValue, HandleValue locales, HandleValue options,
                     DateTimeFormatOptions dtfOptions, MutableHandleValue result);
/**
 * Returns the object holding the internal properties for obj.
 */
extern JSObject*
GetInternalsObject(JSContext* cx, JS::Handle<JSObject*> obj);

/** Report an Intl internal error not directly tied to a spec step. */
extern void
ReportInternalError(JSContext* cx);

static inline bool
StringsAreEqual(const char* s1, const char* s2)
{
    return !strcmp(s1, s2);
}

static inline bool
StringsAreEqual(JSAutoByteString& s1, const char* s2)
{
    return !strcmp(s1.ptr(), s2);
}

/**
 * The last-ditch locale is used if none of the available locales satisfies a
 * request. "en-GB" is used based on the assumptions that English is the most
 * common second language, that both en-GB and en-US are normally available in
 * an implementation, and that en-GB is more representative of the English used
 * in other locales.
 */
static inline const char* LastDitchLocale() { return "en-GB"; }

/**
 * Certain old, commonly-used language tags that lack a script, are expected to
 * nonetheless imply one. This object maps these old-style tags to modern
 * equivalents.
 */
struct OldStyleLanguageTagMapping {
  const char* const oldStyle;
  const char* const modernStyle;

  // Provide a constructor to catch missing initializers in the mappings array.
  constexpr OldStyleLanguageTagMapping(const char* oldStyle,
                                       const char* modernStyle)
      : oldStyle(oldStyle), modernStyle(modernStyle) {}
};

extern const OldStyleLanguageTagMapping oldStyleLanguageTagMappings[5];

static inline const char*
IcuLocale(const char* locale)
{
    if (StringsAreEqual(locale, "und"))
        return ""; // ICU root locale

    return locale;
}

// Starting with ICU 59, UChar defaults to char16_t.
static_assert(mozilla::IsSame<UChar, char16_t>::value,
              "SpiderMonkey doesn't support redefining UChar to a different type");

// The inline capacity we use for a Vector<char16_t>.  Use this to ensure that
// our uses of ICU string functions, below and elsewhere, will try to fill the
// buffer's entire inline capacity before growing it and heap-allocating.
static const size_t INITIAL_CHAR_BUFFER_SIZE = 32;

template <typename ICUStringFunction, typename CharT, size_t InlineCapacity>
static int32_t
CallICU(JSContext* cx, Vector<CharT, InlineCapacity>& chars, const ICUStringFunction& strFn)
{
    MOZ_ASSERT(chars.length() == 0);
    MOZ_ALWAYS_TRUE(chars.resize(InlineCapacity));

    UErrorCode status = U_ZERO_ERROR;
    int32_t size = strFn(chars.begin(), InlineCapacity, &status);
    if (status == U_BUFFER_OVERFLOW_ERROR) {
        MOZ_ASSERT(size >= 0);
        if (!chars.resize(size_t(size)))
            return -1;
        status = U_ZERO_ERROR;
        strFn(chars.begin(), size, &status);
    }
    if (U_FAILURE(status)) {
        ReportInternalError(cx);
        return -1;
    }

    MOZ_ASSERT(size >= 0);
    if (!chars.resize(size_t(size)))
        return -1;
    return size;
}

template <typename ICUStringFunction>
static JSString*
CallICU(JSContext* cx, const ICUStringFunction& strFn)
{
    Vector<char16_t, INITIAL_CHAR_BUFFER_SIZE> chars(cx);

    int32_t size = CallICU(cx, chars, strFn);
    if (size < 0)
        return nullptr;

    return NewStringCopyN<CanGC>(cx, chars.begin(), size_t(size));
}

} // namespace intl

} // namespace js

#endif /* builtin_intl_CommonFunctions_h */