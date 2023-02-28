#pragma once
#ifndef XSTL_CORE_HPP
#define XSTL_CORE_HPP

#include <cstdio>
#if __has_cpp_attribute(nodiscard)
#define XSTL_NODISCARD [[nodiscard]]
#else
#define XSTL_NODISCARD
#endif

#if __has_cpp_attribute(likely)
#define XSTL_LIKELY(...) (__VA_ARGS__) [[likely]]
#else
#define XSTL_LIKELY(...) (__VA_ARGS__)
#endif

#if __has_cpp_attribute(unlikely)
#define XSTL_UNLIKELY(...) (__VA_ARGS__) [[unlikely]]
#else
#define XSTL_UNLIKELY(...) (__VA_ARGS__)
#endif

#ifdef __cpp_lib_unreachable
#define UNREACHABLE() ::std::unreachable()
#elif defined __GNUC__  // GCC, Clang, ICC
#define UNREACHABLE() __builtin_unreachable()
#elif defined _MSC_VER  // MSVC
#define UNREACHABLE() __assume(false)
#endif

#define MISMATCH_ALLOCATOR_MESSAGE(CONTAINER, VALUE_TYPE)               \
    CONTAINER " requires that Allocator's value_type match " VALUE_TYPE \
              " (See N4659 26.2.1 [container.requirements.general]/16 allocator_type)"

#ifndef XSTL_HAS_CXX17
#if __cplusplus > 201402L
#define XSTL_HAS_CXX17 1
#else
#define XSTL_HAS_CXX17 0
#endif
#endif

#ifndef XSTL_HAS_CXX20
#if XSTL_HAS_CXX17 && __cplusplus > 201703L
#define XSTL_HAS_CXX20 1
#else
#define XSTL_HAS_CXX20 0
#endif
#endif

#ifndef XSTL_HAS_CXX23
#if XSTL_HAS_CXX20 && __cplusplus > 202002L
#define XSTL_HAS_CXX23 1
#else
#define XSTL_HAS_CXX23 0
#endif
#endif

#define XSTL_ASSUME(MSG, EXPR) static_cast<void>((EXPR) ? void(0) : UNREACHABLE())
#define XSTL_ASSERT(MSG, ...) \
    static_cast<void>(        \
        (__VA_ARGS__) ? void(0) : xstl::assert_failure(static_cast<const char*>(__FILE__), __LINE__, "assertion failed: " #MSG))

#ifdef NO_XSTL_SAFETY_VERIFY
#define XSTL_EXPECT(EXPR, MSG) XSTL_ASSUME(MSG, EXPR)
#else
#define XSTL_EXPECT(EXPR, MSG) XSTL_ASSERT(MSG, EXPR)
#endif

#define XSTL_CONCEPT_CAT_(X, Y) X##Y
#define XSTL_CONCEPT_CAT(X, Y) XSTL_CONCEPT_CAT_(X, Y)

/// Requires-clause emulation with SFINAE (for templates)
#define XSTL_REQUIRES_(...)                  \
    int XSTL_CONCEPT_CAT(_concept_requires_, \
                         __LINE__) = 42,     \
                         ::std::enable_if_t < (XSTL_CONCEPT_CAT(_concept_requires_, __LINE__) == 43) || (__VA_ARGS__), int > = 0

/// Requires-clause emulation with SFINAE (for "non-templates")
#define XSTL_REQUIRES(...)                                                                                           \
    template <int XSTL_CONCEPT_CAT(_concept_requires_, __LINE__)                                               = 42, \
              ::std::enable_if_t<(XSTL_CONCEPT_CAT(_concept_requires_, __LINE__) == 43) || (__VA_ARGS__), int> = 0>

namespace xstl {
    void assert_failure(char const* file, int line, char const* msg) {
        fprintf(stderr, "%s(%d): %s\n", file, line, msg);
        abort();
    }
}  // namespace xstl

#endif  // XSTL_CORE_HPP