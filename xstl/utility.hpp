/*
 *   Copyright (c) 2022 Kamichanw. All rights reserved.
 *   @file utility.hpp
 *   @brief The utility library contains many practical tools.
 *   @author Shen Xian e-mail: 865710157@qq.com
 *   @version 2.0
 */

#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_

#include <functional>
#include <iterator>
#include <type_traits>

namespace xstl {
    template <template <class...> class _Tp, class... _Args>
    class tmpl_check {
        template <template <class...> class _Ty, class... _Arg>
        static auto check(int, _Ty<_Arg...>* = 0) -> std::true_type;
        template <template <class...> class _Ty, class... _Arg>
        static auto check(...) -> std::false_type;

    public:
        static constexpr bool value = decltype(check<_Tp, _Args...>(0))::value;
    };

    template <bool _Legal, size_t _Size, template <class...> class _Tp, class... _Args>
    struct tmpl_max_args_impl : tmpl_max_args_impl<tmpl_check<_Tp, _Args..., void>::value, _Size + 1, _Tp, _Args..., void> {};

    template <size_t _Size, template <class...> class _Tp, class... _Args>
    struct tmpl_max_args_impl<false, _Size, _Tp, _Args...> {
        static constexpr size_t max = _Size - 1;
    };

#define _MAX_ARGS_COUNT 200
    template <template <class...> class _Tp, class... _Args>
    struct tmpl_max_args_impl<true, _MAX_ARGS_COUNT, _Tp, _Args...> {
        static constexpr size_t max = size_t(-1);
    };
#undef _MAX_ARGS_COUNT

    template <bool _Legal, size_t _Size, template <class...> class _Tp, class... _Args>
    struct tmpl_min_args_impl : tmpl_min_args_impl<tmpl_check<_Tp, _Args..., void>::value, _Size + 1, _Tp, _Args..., void> {};

    template <size_t _Size, template <class...> class _Tp, class... _Args>
    struct tmpl_min_args_impl<true, _Size, _Tp, _Args...> : tmpl_max_args_impl<tmpl_check<_Tp, _Args..., void>::value, sizeof...(_Args) + 1, _Tp, _Args..., void> {
        static constexpr size_t min = _Size - 1;
    };

    /**
     *	@class tmpl_max_args
     *	@brief get the maximum number of arguments of a template
     */
    template <template <class...> class _Tp>
    struct tmpl_max_args {
        static constexpr size_t value = tmpl_min_args_impl<tmpl_check<_Tp>::value, 1, _Tp>::max;
    };

    /**
     *	@class tmpl_min_args
     *	@brief get the minimum number of arguments of a template
     */
    template <template <class...> class _Tp>
    struct tmpl_min_args {
        static constexpr size_t value = tmpl_min_args_impl<tmpl_check<_Tp>::value, 1, _Tp>::min;
    };

    /**
     *	@class function_traits
     *	@brief get the detail info about a specific function
     */
    template <class _Fn, class... _Args>
    struct funtion_traits {
        using rv_type                  = decltype(std::declval<_Fn>()(std::declval<_Args>()...));
        static constexpr size_t args_n = funtion_traits<decltype(&_Fn::operator())>::args_n;
    };

    template <class _RvTp, class... _Args>
    struct funtion_traits<_RvTp(_Args...)> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _RvTp, class... _Args>
    struct funtion_traits<_RvTp (*)(_Args...)> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _RvTp, class... _Args>
    struct funtion_traits<_RvTp (&)(_Args...)> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _Tp, class _RvTp, class... _Args>
    struct funtion_traits<_RvTp (_Tp::*)(_Args...)> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _Tp, class _RvTp, class... _Args>
    struct funtion_traits<_RvTp (_Tp::*)(_Args...) const> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _RvTp, class... _Args>
    struct funtion_traits<std::function<_RvTp(_Args...)>> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _RvTp, class... _Args>
    struct funtion_traits<std::function<_RvTp (*)(_Args...)>> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _RvTp, class... _Args>
    struct funtion_traits<std::function<_RvTp (&)(_Args...)>> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _Tp, class _RvTp, class... _Args>
    struct funtion_traits<std::function<_RvTp (_Tp::*)(_Args...)>> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _Tp, class _RvTp, class... _Args>
    struct funtion_traits<std::function<_RvTp (_Tp::*)(_Args...) const>> {
        using rv_type                  = _RvTp;
        static constexpr size_t args_n = sizeof...(_Args);
    };

    template <class _Fn, class... _Args>
    using ret_value_of_t = typename funtion_traits<_Fn, _Args...>::rv_type;

    template <class _Fn>
    static constexpr size_t args_n_v = funtion_traits<_Fn>::args_n;

    template <class _Tp>
    concept function_type = requires {
        typename funtion_traits<_Tp>::rv_type;
    };

    template <function_type _Fn>
    size_t get_args_count(_Fn) {
        return args_count<decltype(&_Fn::operator())>::args_n;
    }

    template <size_t _Idx, class _Fn>
    struct args_of {
        using type = typename args_of<_Idx, decltype(&_Fn::operator())>::type;
    };

    template <size_t _Idx, class _RvTp, class _Arg0, class... _Args>
    struct args_of<_Idx, _RvTp (*)(_Arg0, _Args...)> {
        using type = typename args_of<_Idx - 1, _RvTp (*)(_Args...)>::type;
    };
    template <class _RvTp, class _Arg0, class... _Args>
    struct args_of<0, _RvTp (*)(_Arg0, _Args...)> {
        using type = _Arg0;
    };
    template <size_t _Idx, class _RvTp>
    struct args_of<_Idx, _RvTp (*)()> {
        using type = void;
    };

    template <size_t _Idx, class _RvTp, class _Arg0, class... _Args>
    struct args_of<_Idx, _RvTp(_Arg0, _Args...)> {
        using type = typename args_of<_Idx - 1, _RvTp (*)(_Args...)>::type;
    };
    template <class _RvTp, class _Arg0, class... _Args>
    struct args_of<0, _RvTp(_Arg0, _Args...)> {
        using type = _Arg0;
    };
    template <size_t _Idx, class _RvTp>
    struct args_of<_Idx, _RvTp()> {
        using type = void;
    };

    template <size_t _Idx, class _RvTp, class _Arg0, class... _Args>
    struct args_of<_Idx, _RvTp (&)(_Arg0, _Args...)> {
        using type = typename args_of<_Idx - 1, _RvTp (*)(_Args...)>::type;
    };
    template <class _RvTp, class _Arg0, class... _Args>
    struct args_of<0, _RvTp (&)(_Arg0, _Args...)> {
        using type = _Arg0;
    };
    template <size_t _Idx, class _RvTp>
    struct args_of<_Idx, _RvTp (&)()> {
        using type = void;
    };

    template <size_t _Idx, class _Tp, class _RvTp, class _Arg0, class... _Args>
    struct args_of<_Idx, _RvTp (_Tp::*)(_Arg0, _Args...)> {
        using type = typename args_of<_Idx - 1, _RvTp (*)(_Args...)>::type;
    };
    template <class _RvTp, class _Tp, class _Arg0, class... _Args>
    struct args_of<0, _RvTp (_Tp::*)(_Arg0, _Args...)> {
        using type = _Arg0;
    };
    template <size_t _Idx, class _Tp, class _RvTp>
    struct args_of<_Idx, _RvTp (_Tp::*)()> {
        using type = void;
    };

    template <size_t _Idx, class _Tp, class _RvTp, class _Arg0, class... _Args>
    struct args_of<_Idx, _RvTp (_Tp::*)(_Arg0, _Args...) const> {
        using type = typename args_of<_Idx - 1, _RvTp (*)(_Args...)>::type;
    };
    template <class _RvTp, class _Tp, class _Arg0, class... _Args>
    struct args_of<0, _RvTp (_Tp::*)(_Arg0, _Args...) const> {
        using type = _Arg0;
    };
    template <size_t _Idx, class _Tp, class _RvTp>
    struct args_of<_Idx, _RvTp (_Tp::*)() const> {
        using type = void;
    };

    template <size_t _Idx, class _Fn>
    using args_of_t = typename args_of<_Idx, _Fn>::type;

}  // namespace xstl
#endif