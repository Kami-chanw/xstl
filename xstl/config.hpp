/*
 *	Copyright(c) 2022 Kamichanw.All rights reserved.
 *   @file config.hpp
 *   @brief The config.hpp contains all of containers' base and templates for iterators.
 *   @author Shen Xian e-mail: 865710157@qq.com
 *   @version 2.0
 */

#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include "allocator.hpp"
#include <cassert>
#ifdef __cpp_lib_three_way_comparison
#include <compare>
#endif
#define CAST2SCARY(CONT) static_cast<const _Scary_val*>(CONT)

#define XSTL_ASSUME(EXPR)
#define XSTL_ASSERT(...)        \
    static_cast<void>(          \
        (__VA_ARGS__) ? void(0) \
                      : xstl::assert_failure(static_cast<const char*>(__FILE__), __LINE__, "assertion failed: " #__VA_ARGS__))

#ifdef _NO_XSTL_SAFETY_VERIFT_
#define XSTL_EXPECT(EXPR) XSTL_ASSUME(EXPR)
#else
#define XSTL_EXPECT(EXPR) XSTL_ASSERT(EXPR)
#endif

#define XSTL_CONCEPT_CAT_(X, Y) X##Y
#define XSTL_CONCEPT_CAT(X, Y) XSTL_CONCEPT_CAT_(X, Y)

/// Requires-clause emulation with SFINAE (for templates)
#define XSTL_REQUIRES_(...)                                                                                                     \
    int XSTL_CONCEPT_CAT(_concept_requires_, __LINE__) = 42,                                                                    \
                                             typename ::std::enable_if < (XSTL_CONCEPT_CAT(_concept_requires_, __LINE__) == 43) \
                                                 || (__VA_ARGS__),                                                              \
                                             int > ::type = 0

/// Requires-clause emulation with SFINAE (for "non-templates")
#define XSTL_REQUIRES(...)                                                                                                        \
    template <int XSTL_CONCEPT_CAT(_concept_requires_, __LINE__)                                                            = 42, \
              typename ::std::enable_if<(XSTL_CONCEPT_CAT(_concept_requires_, __LINE__) == 43) || (__VA_ARGS__), int>::type = 0>

namespace xstl {
    [[noreturn]] void assert_failure(char const* file, int line, char const* msg) {
        fprintf(stderr, "%s(%d): %s\n", file, line, msg);
        abort();
    }

    template <class _Node, class _Rlsr>
    struct exception_guard {
        exception_guard(_Node* ptr, _Rlsr releaser) : _node(ptr), _releaser(releaser) {}
        _Node* release() { return std::exchange(_node, nullptr); }
        ~exception_guard() {
            if (_node)
                _releaser(_node);
        }

    private:
        _Node* _node;
        _Rlsr  _releaser;
    };

    template <class _Node, class _Rlsr>
    exception_guard(_Node*, _Rlsr) -> exception_guard<_Node, _Rlsr>;

    template <class _Keycmp, class _Lhs, class _Rhs = _Lhs>
    inline constexpr bool is_nothrow_comparable_v =
        noexcept(static_cast<bool>(std::declval<const _Keycmp&>()(std::declval<const _Lhs&>(), std::declval<const _Rhs&>())));

    struct container_val_base {};

#ifdef __cpp_lib_concepts
    class synth_three_way {
        template <class _Ty>
        concept _Boolean_testable_impl = std::convertible_to<_Ty, bool>;

        // clang-format off
        template <class _Ty>
        concept _Boolean_testable = 
            _Boolean_testable_impl<_Ty> && requires(_Ty&& b) {
                { !std::forward<_Ty>(b) } -> _Boolean_testable_impl;
            };

        // clang-format on 

    public:
        template <class _Ty1, class _Ty2>
        [[nodiscard]] constexpr auto operator()(const _Ty1& lhs, const _Ty2& rhs) const
        // clang-format off
        requires requires {
            { lhs < rhs } -> _Boolean_testable;
            { rhs < lhs } -> _Boolean_testable;
        }
        // clang-format on
        {
            if constexpr (std::three_way_comparable_with<_Ty1, _Ty2>)
                return lhs <=> rhs;
            else {
                return lhs < rhs   ? std::weak_ordering::less
                       : rhs < lhs ? std::weak_ordering::greater
                                   : std::weak_ordering::equivalent;
            }
        }
    };
    template <class _Ty1, class _Ty2 = _Ty1>
    using synth_three_way_result = decltype(synth_three_way{}(std::declval<_Ty1&>(), std::declval<_Ty2&>()));
#endif

}  // namespace xstl
#endif