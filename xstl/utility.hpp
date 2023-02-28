/*
 *	Copyright(c) 2022 Kamichanw.All rights reserved.
 *   @file config.hpp
 *   @brief The utility.hpp contains some meta-programming utils and so on
 *   @author Shen Xian e-mail: 865710157@qq.com
 *   @version 2.0
 */

#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_

#include "xstl_core.hpp"
#include <cassert>
#include <cstdio>  // for stderr
#include <type_traits>

#ifdef __cpp_lib_three_way_comparison
#include <compare>
#endif
#define CAST2SCARY(CONT) static_cast<const _Scary_val*>(CONT)

namespace xstl {

    template <class _Rlsr>
    struct scoped_guard {
        scoped_guard(_Rlsr releaser) : _releaser(releaser) {}

        scoped_guard(const scoped_guard&)            = delete;
        scoped_guard& operator=(const scoped_guard&) = delete;

        void dismiss() noexcept { _ok = true; }

        ~scoped_guard() {
            if (!_ok)
                _releaser();
        }

    private:
        bool  _ok = false;
        _Rlsr _releaser;
    };

    template <class _Rlsr>
    scoped_guard(_Rlsr) -> scoped_guard<_Rlsr>;

    template <class _Keycmp, class _Lhs, class _Rhs = _Lhs>
    inline constexpr bool is_nothrow_comparable_v =
        noexcept(static_cast<bool>(std::declval<const _Keycmp&>()(std::declval<const _Lhs&>(), std::declval<const _Rhs&>())));

    template <class _Tp, class... _Types>
    inline constexpr bool is_any_of_v = std::disjunction_v<std::is_same<_Tp, _Types>...>;
    template <class _Tp>
    inline constexpr bool is_character_v =
        is_any_of_v<_Tp, char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t>;

    template <class _Tp, bool = std::is_enum_v<_Tp>>
    struct unwrap_enum {
        using type = std::underlying_type_t<_Tp>;
    };

    template <class _Tp>
    struct unwrap_enum<_Tp, false> {
        using type = _Tp;
    };

    template <class _Tp>
    using unwrap_enum_t = typename unwrap_enum<_Tp>::type;

    template <class>
    inline constexpr bool always_false = false;  // for static_assert

    /**
     *  Composite multiple type predicates in a mask.
     *  e.g. type_pred_mask_v<true_type, true_type, false_type, false_type> // 0b1101
     */
    template <class... _Args>
    struct type_pred_mask {
        static_assert(sizeof...(_Args) <= 512, "incompatible amount of arguments and mask type");
        static constexpr std::uint64_t value = type_pred_mask<_Args...>::value;
    };
    template <class _Tp, class... _Args>
    struct type_pred_mask<_Tp, _Args...> {
        static constexpr std::uint64_t value =
            (std::uint64_t{ _Tp::value * 1 } << sizeof...(_Args)) | type_pred_mask<_Args...>::value;
    };
    template <class _Tp>
    struct type_pred_mask<_Tp> {
        static constexpr std::uint64_t value = _Tp::value;
    };

    template <class... _Args>
    inline constexpr std::uint64_t type_pred_mask_v = type_pred_mask<_Args...>::value;

    /*
     * select type conditionally from type predicates (which has static data member 'value' and type alias 'type') whose value ==
     * true. if there is no suitable type predicate, the last argument will be selected as final result.
     * only one _Args::value can be true, otherwise static assertion will fall through.
     */
    template <class _Tp, class... _Args>
    struct select_type : select_type<std::bool_constant<_Tp::value>, _Tp, _Args...> {};

    template <class _Last>
    struct select_type<_Last> {
        /* if the last type is a type predicate:
         *     if last::value == true, select last::type
         *     else assertion fail
         *  else use the last type as default type
         */
        template <class _Ty, class = void>
        struct _Get_type {
            using type = _Ty;
        };
        template <class _Ty>
        struct _Get_type<_Ty, std::void_t<typename _Ty::type, decltype(_Ty::value)>> {
            static_assert(_Ty::value, "no valid type can be selected or no specified default type");
            using type = typename _Ty::type;
        };
        using type = typename _Get_type<_Last>::type;
    };

    template <class _Tp, class _Tp2, class... _Args>
    struct select_type<std::false_type, _Tp, _Tp2, _Args...> : select_type<std::bool_constant<_Tp2::value>, _Tp2, _Args...> {};

    template <class _Tp, class... _Args>
    struct select_type<std::true_type, _Tp, _Args...> {
        template <class... _Rest>
        struct _Check_rest {
            static constexpr bool value = _Check_rest<_Rest...>::value;
        };
        template <class _This, class... _Rest>
        struct _Check_rest<_This, _Rest...> {
            static constexpr bool value = _This::value && _Check_rest<_Rest...>;
        };
        template <class _Last>
        struct _Check_rest<_Last> {
            template <class _Ty, class = void>
            struct _Get_value : std::true_type {};
            template <class _Ty>
            struct _Get_value<_Ty, std::void_t<decltype(_Ty::value)>> {
                static constexpr bool value = !_Ty::value;
            };
            static constexpr bool value = _Get_value<_Last>::value;
        };
        static_assert(sizeof...(_Args) == 0 || _Check_rest<_Args...>::value, "more than one template Args::value == true");
        using type = typename _Tp::type;
    };

    template <class _Tp, class _Ty>
    struct select_type<std::false_type, _Tp, _Ty> {  // use default type
        using type = typename select_type<_Ty>::type;
    };

    template <class... _Args>
    using select_type_t = typename select_type<_Args...>::type;

#ifdef __cpp_lib_concepts
    // clang-format off
    template <class _Ty>
    concept boolean_testable = std::is_convertible_v<_Ty, bool> && 
        requires(_Ty&& b) {
            { !std::forward<_Ty>(b) } -> std::convertible_to<bool>;
        };
    // clang-format on
    class synth_three_way {
    public:
        template <class _Ty1, class _Ty2>
        XSTL_NODISCARD constexpr auto operator()(const _Ty1& lhs, const _Ty2& rhs) const
            // clang-format off
        requires requires {
            { lhs < rhs } -> boolean_testable;
            { rhs < lhs } -> boolean_testable;
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

    struct move_op_tag {  // tag to request move operation
        explicit move_op_tag() = default;
    };
    struct copy_op_tag {  // tag to request copy operation
        explicit copy_op_tag() = default;
    };

    template <size_t _Idx, class... _Args>
    struct args_element;

    template <size_t _Idx>
    struct args_element<_Idx> {
        static_assert(always_false<decltype(_Idx)>, "out of bound");
    };

    template <class _Tp, class... _Rest>
    struct args_element<0, _Tp, _Rest...> {
        using type = _Tp;
    };

    template <size_t _Idx, class _Tp, class... _Rest>
    struct args_element<_Idx, _Tp, _Rest...> : args_element<_Idx - 1, _Rest...> {};

    template <size_t _Idx, class... _Args>
    using args_element_t = args_element<_Idx, _Args...>::type;
}  // namespace xstl
#endif