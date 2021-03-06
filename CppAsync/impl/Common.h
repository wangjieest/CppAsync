/*
* Copyright 2015-2016 Valentin Milea
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include "../Config.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <type_traits>
#include <utility>

//
// Helper macro for defining labels
//

#define _ut_concatenate(s1, s2)          s1##s2
#define _ut_concatenate_indirect(s1, s2) _ut_concatenate(s1, s2)
#define _ut_anonymous_label(str)         _ut_concatenate_indirect(str, __LINE__)

//
// Helpers for wrapping multi-line macros
//

#ifdef _MSC_VER

#define _ut_multi_line_macro_begin \
    do { \
    __pragma(warning(push)) \
    __pragma(warning(disable:4127))

#define _ut_multi_line_macro_end \
    } while(0) \
    __pragma(warning(pop))

#else

#define _ut_multi_line_macro_begin do {
#define _ut_multi_line_macro_end   } while(0)

#endif // _MSC_VER

//
// Noexcept emulation for MSVC
//

#if defined(_MSC_VER) && _MSC_VER < 1900
#define _ut_noexcept throw()
#else
#define _ut_noexcept noexcept
#endif

//
// Aliases to deal with exceptions ban
//

#ifdef UT_NO_EXCEPTIONS

namespace ut
{
    class Error
    {
    public:
        using value_type = UT_CUSTOM_ERROR_TYPE;

        static_assert(std::is_nothrow_move_constructible<value_type>::value &&
            std::is_nothrow_move_assignable<value_type>::value,
            "Error type may not throw on move construction or assignment. "
            "Add noexcept or throw() specifier");

        Error() _ut_noexcept
            : mValue(value_type()) { }

        Error(value_type value) _ut_noexcept
            : mValue(std::move(value)) { }

        Error(Error&& other) _ut_noexcept
            : mValue(std::move(other.mValue)) { }

        Error& operator=(Error&& other) _ut_noexcept
        {
            mValue = std::move(other.mValue);

            return *this;
        }

        const value_type& get() const
        {
            return mValue;
        }

        value_type& get()
        {
            return mValue;
        }

    private:
        Error(const Error& other) = delete;
        Error& operator=(const Error& other) = delete;

        value_type mValue;
    };

    inline bool operator==(const Error& a, const Error& b)
    {
        return a.get() == b.get();
    }

    inline bool operator!=(const Error& a, const Error& b)
    {
        return !(a == b);
    }

    inline bool isNil(const Error& error) _ut_noexcept
    {
        return error == Error();
    }

    inline void reset(Error& error) _ut_noexcept
    {
        error = Error();
    }
}

#else

namespace ut
{
    using Error = std::exception_ptr;

    template <class E>
    Error makeExceptionPtr(E&& e) _ut_noexcept
    {
        return std::make_exception_ptr(std::forward<E>(e));
    }

    inline void rethrowException(Error eptr)
    {
        std::rethrow_exception(eptr);
    }

    inline bool uncaughtException() _ut_noexcept
    {
        // WARNING: With GCC versions from 4.8.2 to 4.9.3 and from 5.0 to 5.2,
        //          uncaught_exception() returns true after rethrow_exception().
        //          See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=62258.
        //
        return std::uncaught_exception();
    }

    inline Error currentException() _ut_noexcept
    {
        return std::current_exception();
    }

    inline bool isNil(const Error& eptr) _ut_noexcept
    {
        return eptr == nullptr;
    }

    inline void reset(Error& eptr) _ut_noexcept
    {
        eptr = nullptr;
    }
}

#endif // UT_NO_EXCEPTIONS

//
// Common types for overloading and placeholders
//

namespace ut {

struct InPlaceTag { };

template <class T>
struct TypeInPlaceTag
{
    using type = T;
};

struct Nothing { };

struct Dummy { };

}
