#pragma once
#ifndef _COMPRESSED_TUPLE_
#define _COMPRESSED_TUPLE_

#include "utility.hpp"
#include <tuple>

namespace xstl {
    namespace {
        template <class _This, class _Alloc>
        using _Uses_allocator0 = std::enable_if_t<!std::uses_allocator_v<_This, _Alloc>, int>;
        template <class _This, class _Alloc, class... _Args>
        using _Uses_allocator1 =
            std::enable_if_t<std::uses_allocator_v<_This, _Alloc>
                                 && std::is_constructible_v<_This, std::allocator_arg_t, const _Alloc&, _Args...>,
                             int>;
        template <class _This, class _Alloc, class... _Args>
        using _Uses_allocator2 =
            std::enable_if_t<std::uses_allocator_v<_This, _Alloc>
                                 && !std::is_constructible_v<_This, std::allocator_arg_t, const _Alloc&, _Args...>,
                             int>;

        template <class _Tp>
        struct _Unrefwrap {
            using type = _Tp;
        };
        template <class _Tp>
        struct _Unrefwrap<std::reference_wrapper<_Tp>> {
            using type = _Tp&;
        };

        using _Ignore_t = decltype(std::ignore);

        template <size_t _Idx, class _This, bool = std::is_empty_v<_This> && !std::is_final_v<_This>>
        struct _This_type : public _This {  // for empty base class optimization
            constexpr _This_type() : _This() {}
            constexpr _This_type(const _This& value) : _This(value) {}
            constexpr _This_type(const _This_type&) = default;
            constexpr _This_type(_This_type&&)      = default;

            template <class _Tp>
            constexpr _This_type(_Tp&& value) : _This(std::forward<_Tp>(value)) {}

            template <class _Alloc, class... _Args, _Uses_allocator0<_This, _Alloc> = 0>
            constexpr _This_type(const _Alloc&, std::allocator_arg_t, _Args&&... args) : _This(std::forward<_Args>(args)...) {}

            template <class _Alloc, class... _Args, _Uses_allocator1<_This, _Alloc, _Args...> = 0>
            constexpr _This_type(const _Alloc& alloc, std::allocator_arg_t, _Args&&... args)
                : _This(std::allocator_arg, alloc, std::forward<_Args>(args)...) {}

            template <class _Alloc, class... _Args, _Uses_allocator2<_This, _Alloc, _Args...> = 0>
            constexpr _This_type(const _Alloc& alloc, std::allocator_arg_t, _Args&&... args)
                : _This(std::forward<_Args>(args)..., alloc) {}

            constexpr _This&       _Get_first() noexcept { return *this; }
            constexpr const _This& _Get_first() const noexcept { return *this; }
        };

        template <size_t _Idx, class _This>
        struct _This_type<_Idx, _This, false> {
            constexpr _This_type() : _value() {}
            constexpr _This_type(const _This& value) : _value(value) {}
            constexpr _This_type(const _This_type&) = default;
            constexpr _This_type(_This_type&&)      = default;

            template <class _Tp>
            constexpr _This_type(_Tp&& value) : _value(std::forward<_Tp>(value)) {}

            template <class _Alloc, class... _Args, _Uses_allocator0<_This, _Alloc> = 0>
            constexpr _This_type(const _Alloc&, std::allocator_arg_t, _Args&&... args) : _value(std::forward<_Args>(args)...) {}

            template <class _Alloc, class... _Args, _Uses_allocator1<_This, _Alloc, _Args...> = 0>
            constexpr _This_type(const _Alloc& alloc, std::allocator_arg_t, _Args&&... args)
                : _value(std::allocator_arg, alloc, std::forward<_Args>(args)...) {}

            template <class _Alloc, class... _Args, _Uses_allocator2<_This, _Alloc, _Args...> = 0>
            constexpr _This_type(const _Alloc& alloc, std::allocator_arg_t, _Args&&... args)
                : _value(std::forward<_Args>(args)..., alloc) {}

            constexpr _This&       _Get_first() noexcept { return _value; }
            constexpr const _This& _Get_first() const noexcept { return _value; }

            _This _value;
        };

        template <size_t _Idx, class... _Args>  //_Idx is used for preventing the invalidity of empty base class optimization
        struct _Compressed_tuple;

        template <size_t _Idx, class _This, class... _Rest>
        struct _Compressed_tuple<_Idx, _This, _Rest...> : public _Compressed_tuple<_Idx + 1, _Rest...>,
                                                          private _This_type<_Idx, _This> {

            using _Rest_tuple = _Compressed_tuple<_Idx + 1, _Rest...>;
            using _Base       = _This_type<_Idx, _This>;

            constexpr _This&       _Get_first() noexcept { return _Base::_Get_first(); }  // prevents ambiguity of invocation
            constexpr const _This& _Get_first() const noexcept { return _Base::_Get_first(); }
            constexpr _Rest_tuple& _Get_rest() noexcept { return *this; }  // get reference to rest of elements
            constexpr const _Rest_tuple& _Get_rest() const noexcept { return *this; }

            constexpr _Compressed_tuple() : _Rest_tuple(), _Base() {}

            explicit constexpr _Compressed_tuple(_Ignore_t, const _Rest&... rest) : _Rest_tuple(rest...), _Base() {}

            template <class... _URest, std::enable_if_t<sizeof...(_Rest) == sizeof...(_URest), int> = 0>
            explicit constexpr _Compressed_tuple(_Ignore_t, _URest&&... rest)
                : _Rest_tuple(std::forward<_URest>(rest)...), _Base() {}

            explicit constexpr _Compressed_tuple(const _This& curr, const _Rest&... rest) : _Rest_tuple(rest...), _Base(curr) {}

            template <class _UFirst, class... _URest, std::enable_if_t<sizeof...(_Rest) == sizeof...(_URest), int> = 0>
            explicit constexpr _Compressed_tuple(_UFirst&& curr, _URest&&... rest)
                : _Rest_tuple(std::forward<_URest>(rest)...), _Base(std::forward<_UFirst>(curr)) {}

            constexpr _Compressed_tuple(const _Compressed_tuple&)  = default;
            _Compressed_tuple& operator=(const _Compressed_tuple&) = delete;
            _Compressed_tuple(_Compressed_tuple&&)                 = default;

            template <class... _Args>
            constexpr _Compressed_tuple(const _Compressed_tuple<_Idx, _Args...>& other)
                : _Rest_tuple(other._Get_rest()), _Base(other._Get_first()) {}

            template <class _UFirst, class... _URest>
            constexpr _Compressed_tuple(_Compressed_tuple<_Idx, _UFirst, _URest...>&& other)
                : _Rest_tuple(std::move(other._Get_rest())), _Base(std::forward<_UFirst>(other._Get_first())) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc)
                : _Rest_tuple(tag, alloc), _Base(tag, alloc) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _This& curr, const _Rest&... rest)
                : _Rest_tuple(tag, alloc, rest...), _Base(alloc, curr) {}

            template <class _Alloc, class _UFirst, class... _URest,
                      std::enable_if_t<sizeof...(_Rest) == sizeof...(_URest), int> = 0>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _UFirst&& curr, _URest&&... rest)
                : _Rest_tuple(tag, alloc, std::forward<_URest>(rest)...), _Base(alloc, std::forward<_UFirst>(curr)) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Compressed_tuple& other)
                : _Rest_tuple(tag, alloc, other._Get_rest()), _Base(alloc, other._Get_first()) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _Compressed_tuple&& other)
                : _Rest_tuple(tag, alloc, std::move(other._Get_rest())), _Base(alloc, std::forward<_This>(other._Get_first())) {}

            template <class _Alloc, class _UFirst, class... _URest>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc,
                                        const _Compressed_tuple<_Idx, _UFirst, _URest...>& other)
                : _Rest_tuple(tag, alloc, other._Get_rest()), _Base(alloc, other._Get_first()) {}

            template <class _Alloc, class _UFirst, class... _URest>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc,
                                        _Compressed_tuple<_Idx, _UFirst, _URest...>&& other)
                : _Rest_tuple(tag, alloc, std::move(other._Get_rest())), _Base(alloc, std::forward<_UFirst>(other._Get_first())) {
            }

            template <class _UFirst, class... _URest>
            constexpr bool _Equals(const _Compressed_tuple<_Idx, _UFirst, _URest...>& other) const noexcept {
                return _Get_first() == other._Get_first() && this->_Get_rest()._Equals(other._Get_rest());
            }
#ifdef __cpp_lib_three_way_comparison
            template <class _UFirst, class... _URest>
            XSTL_NODISCARD constexpr std::common_comparison_category_t<xstl::synth_three_way_result<_This, _UFirst>>
            _Three_way_compare(const _Compressed_tuple<_Idx, _UFirst, _URest...>& other) const {
                if (auto _res = xstl::synth_three_way{}(_Get_first(), other._Get_first()); _res != 0)
                    return _res;
                return _Rest_tuple::_Three_way_compare(other._Get_rest());
            }
#else
            template <class... _Other>
            XSTL_NODISCARD constexpr bool _Less(const _Compressed_tuple<_Idx, _Other...>& rhs) const {
                return _Get_first() < rhs._Get_first()
                       || (!(rhs._Get_first() < _Get_first()) && _Rest_tuple::_Less(rhs._Get_rest()));
            }
#endif
            template <class... _Args>
            constexpr void _Assign(const _Compressed_tuple<_Idx, _Args...>& other) {
                _Get_first() = other._Get_first();
                this->_Get_rest()._Assign(other._Get_rest());
            }

            template <class _UFirst, class... _URest>
            constexpr void _Assign(_Compressed_tuple<_Idx, _UFirst, _URest...>&& other) {
                _Get_first() = std::forward<_UFirst>(other._Get_first());
                this->_Get_rest()._Assign(std::move(other._Get_rest()));
            }

            constexpr void swap(_Compressed_tuple& other) {
                using std::swap;
                swap(_Get_first(), other._Get_first());
                _Rest_tuple::swap(other._Get_rest());
            }
        };

        template <size_t _Idx, class _This>
        struct _Compressed_tuple<_Idx, _This> : private _This_type<_Idx, _This> {
            using _Base = _This_type<_Idx, _This>;
            using _Base::_Get_first;

            constexpr _Compressed_tuple() : _Base() {}
            constexpr _Compressed_tuple(_Ignore_t) : _Base() {}
            explicit constexpr _Compressed_tuple(const _This& curr) : _Base(curr) {}
            template <class _UFirst>
            explicit constexpr _Compressed_tuple(_UFirst&& curr) : _Base(std::forward<_UFirst>(curr)) {}

            constexpr _Compressed_tuple(const _Compressed_tuple&)  = default;
            _Compressed_tuple& operator=(const _Compressed_tuple&) = delete;

            constexpr _Compressed_tuple(_Compressed_tuple&& other) noexcept(std::is_nothrow_move_constructible_v<_This>)
                : _Base(static_cast<_Base&&>(other)) {}

            template <class _UFirst>
            constexpr _Compressed_tuple(const _Compressed_tuple<_Idx, _UFirst>& other) : _Base(other._Get_first()) {}

            template <class _UFirst>
            constexpr _Compressed_tuple(_Compressed_tuple<_Idx, _UFirst>&& other)
                : _Base(std::forward<_UFirst>(other._Get_first())) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc) : _Base(tag, alloc) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _This& curr) : _Base(alloc, curr) {}

            template <class _Alloc, class _UFirst>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _UFirst&& curr)
                : _Base(alloc, std::forward<_UFirst>(curr)) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Compressed_tuple& other)
                : _Base(alloc, other._Get_first()) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _Compressed_tuple&& other)
                : _Base(alloc, std::forward<_This>(other._Get_first())) {}

            template <class _Alloc, class _UFirst>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc,
                                        const _Compressed_tuple<_Idx, _UFirst>& other)
                : _Base(alloc, other._Get_first()) {}

            template <class _Alloc, class _UFirst>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _Compressed_tuple<_Idx, _UFirst>&& other)
                : _Base(alloc, std::forward<_UFirst>(other._Get_first())) {}

            template <class _UFirst>
            constexpr bool _Equals(const _Compressed_tuple<_Idx, _UFirst>& other) const noexcept {
                return _Get_first() == other._Get_first();
            }
#ifdef __cpp_lib_three_way_comparison
            template <class _UFirst>
            XSTL_NODISCARD constexpr xstl::synth_three_way_result<_This, _UFirst>
            _Three_way_compare(const _Compressed_tuple<_Idx, _UFirst>& other) const {
                if (auto _res = xstl::synth_three_way{}(_Get_first(), other._Get_first()); _res != 0)
                    return _res;
                return std::strong_ordering::equal;
            }
#else
            template <class _UFirst>
            XSTL_NODISCARD constexpr bool _Less(const _Compressed_tuple<_Idx, _UFirst>& other) const {
                return _Get_first() < other._Get_first();
            }
#endif

            template <class _UFirst>
            constexpr void _Assign(const _Compressed_tuple<_Idx, _UFirst>& other) {
                other._Get_first();
            }

            template <class _UFirst>
            constexpr void _Assign(_Compressed_tuple<_Idx, _UFirst>&& other) {
                _Get_first() = std::forward<_UFirst>(other._Get_first());
            }

            constexpr void swap(_Compressed_tuple& other) {
                using std::swap;
                swap(this->_Get_first(), other._Get_first());
            }
        };

    }  // namespace

    template <class... _Args>
    class compressed_tuple : public _Compressed_tuple<0, _Args...> {
    public:
        using _Base = _Compressed_tuple<0, _Args...>;

        template <class _Tuple, class = compressed_tuple, class = std::remove_cv_t<std::remove_reference_t<_Tuple>>>
        struct _Use_other_ctor : std::false_type {};
        template <class _Tuple, class _Tp, class _Ty>
        struct _Use_other_ctor<_Tuple, compressed_tuple<_Tp>, compressed_tuple<_Ty>>
            : std::disjunction<std::is_convertible<_Tuple, _Tp>, std::is_constructible<_Tp, _Tuple>> {};
        template <class _Tuple, class _Tp>
        struct _Use_other_ctor<_Tuple, compressed_tuple<_Tp>, compressed_tuple<_Tp>> : std::true_type {};

        template <class _Ty>
        void _Implicitly_default_construct(const _Ty&) {}  // silence warning
        template <class _Ty, class = void>
        struct _Implicitly_default_constructible : std::false_type {};
        template <class _Ty>
        struct _Implicitly_default_constructible<_Ty, std::void_t<decltype(_Implicitly_default_construct<_Ty>({}))>>
            : std::true_type {};

        template <class...>
        struct _Valid_args : std::false_type {};
        template <class _Tp>
        struct _Valid_args<_Tp>
            : std::conjunction<std::bool_constant<sizeof...(_Args) == 1>,
                               std::negation<std::is_same<compressed_tuple, std::remove_cv_t<std::remove_reference_t<_Tp>>>>> {};
        template <class _Arg1, class _Arg2, class... _URest>
        struct _Valid_args<_Arg1, _Arg2, _URest...> : std::bool_constant<(sizeof...(_URest) + 2) == sizeof...(_Args)> {};

        template <class... _UArgs>
        static constexpr bool _Tuple_constructible_v =
            sizeof...(_Args) == sizeof...(_UArgs) && std::conjunction_v<std::is_constructible<_Args, _UArgs>...>;
        template <class... _UArgs>
        static constexpr bool _Tuple_convertible_v = std::conjunction_v<std::is_convertible<_UArgs, _Args>...>;
        template <class... _UArgs>
        static constexpr bool _Tuple_nothrow_constructible_v =
            std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>;
        template <class... _UArgs>
        static constexpr bool _Tuple_assignable_v =
            sizeof...(_Args) == sizeof...(_UArgs) && std::conjunction_v<std::is_assignable<_Args&, _UArgs>...>;
        template <class... _UArgs>
        static constexpr bool _Tuple_nothrow_assignable_v = std::conjunction_v<std::is_nothrow_assignable<_Args&, _UArgs>...>;
        template <bool>
        static constexpr bool _Default_constructible_v = std::conjunction_v<std::is_default_constructible<_Args>...>;
        template <bool>
        static constexpr bool _Copy_constructible_v =
            sizeof...(_Args) >= 1
            && std::conjunction_v<std::disjunction<std::is_convertible<_Args, _Ignore_t>, std::is_copy_constructible<_Args>>...>;

        template <bool>
        static constexpr bool _Explicit_default_ctor = !std::conjunction_v<_Implicitly_default_constructible<_Args>...>;
        template <class... _UArgs>
        static constexpr bool _Explicit_cref_ctor = !std::conjunction_v<std::is_convertible<const _UArgs&, _Args>...>;
        template <class... _UArgs>
        static constexpr bool _Explicit_rref_ctor = !std::conjunction_v<std::is_convertible<_UArgs&&, _Args>...>;

    public:
#if defined(__cpp_conditional_explicit) || defined(_MSC_VER)  // due to MSVC bug
        template <class _Dummy = void, std::enable_if_t<_Default_constructible_v<std::is_void_v<_Dummy>>, int> = 0>
        constexpr explicit(_Explicit_default_ctor<std::is_void_v<_Dummy>>)  //_Dummy is used for SFINAE
            compressed_tuple() noexcept(std::conjunction_v<std::is_nothrow_default_constructible<_Args>...>)
            : _Base() {}

        template <class _Dummy = void, std::enable_if_t<_Copy_constructible_v<std::is_void_v<_Dummy>>, int> = 0>
        constexpr explicit(_Explicit_cref_ctor<_Args...>)
            compressed_tuple(const _Args&... args) noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<_Args>...>)
            : _Base(args...) {}

        template <class... _UArgs,
                  std::enable_if_t<
                      std::conjunction_v<_Valid_args<_UArgs...>, std::disjunction<std::is_convertible<_UArgs, _Ignore_t>,
                                                                                  std::is_constructible<_Args, _UArgs&&>>...>,
                      int> = 0>
        constexpr explicit(_Explicit_rref_ctor<_UArgs...>)
            compressed_tuple(_UArgs&&... args) noexcept(std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(std::forward<_UArgs>(args)...) {}

        template <class... _UArgs,
                  std::enable_if_t<(sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<const compressed_tuple<_UArgs...>&>>,
                                                             std::is_constructible<_Args, const _UArgs&>...>,
                                   int> = 0>
        constexpr explicit(_Explicit_cref_ctor<_UArgs...>) compressed_tuple(const compressed_tuple<_UArgs...>& other) noexcept(
            std::conjunction_v<std::is_nothrow_copy_constructible<_UArgs>...>)
            : _Base(static_cast<const _Compressed_tuple<0, _UArgs...>&>(other)) {}

        template <class... _UArgs,
                  std::enable_if_t<(sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<compressed_tuple<_UArgs...>&&>>,
                                                             std::is_constructible<_Args, _UArgs&&>...>,
                                   int> = 0>
        constexpr explicit(_Explicit_rref_ctor<_UArgs...>) compressed_tuple(compressed_tuple<_UArgs...>&& other) noexcept(
            std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(static_cast<_Compressed_tuple<0, _UArgs...>&&>(other)) {}

        template <class _First, class _Second, std::enable_if_t<_Tuple_constructible_v<const _First&, const _Second&>, int> = 0>
        constexpr explicit(!_Tuple_convertible_v<const _First&, const _Second&>) compressed_tuple(
            const std::pair<_First, _Second>& other) noexcept(_Tuple_nothrow_constructible_v<const _First&, const _Second&>)
            : _Base(other.first, other.second) {}

        template <class _First, class _Second, std::enable_if_t<_Tuple_constructible_v<_First, _Second>, int> = 0>
        constexpr explicit(!_Tuple_convertible_v<_First, _Second>)
            compressed_tuple(std::pair<_First, _Second>&& other) noexcept(_Tuple_nothrow_constructible_v<_First, _Second>)
            : _Base(std::forward<_First>(other.first), std::forward<_Second>(other.second)) {}

        //// Allocator-extended constructors.

        template <class _Alloc, std::enable_if_t<_Default_constructible_v<std::is_object_v<_Alloc>>, int> = 0>
        constexpr explicit(_Explicit_default_ctor<std::is_object_v<_Alloc>>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc)
            : _Base(tag, alloc) {}

        template <class _Alloc, std::enable_if_t<_Copy_constructible_v<std::is_object_v<_Alloc>>, int> = 0>
        constexpr explicit(_Explicit_cref_ctor<_Args...>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc,
                             const _Args&... args) noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<_Args>...>)
            : _Base(tag, args...) {}

        template <
            class _Alloc, class... _UArgs,
            std::enable_if_t<std::conjunction_v<_Valid_args<_UArgs...>, std::is_constructible<_Args, _UArgs&&>...>, int> = 0>
        constexpr explicit(_Explicit_rref_ctor<_UArgs...>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc,
                             _UArgs&&... args) noexcept(std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(tag, std::forward<_UArgs>(args)...) {}

        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<(sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<const compressed_tuple<_UArgs...>&>>,
                                                             std::is_constructible<_Args, const _UArgs&>...>,
                                   int> = 0>
        constexpr explicit(_Explicit_cref_ctor<_UArgs...>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const compressed_tuple<_UArgs...>& other) noexcept(
                std::conjunction_v<std::is_nothrow_copy_constructible<_UArgs>...>)
            : _Base(tag, alloc, static_cast<const _Compressed_tuple<0, _UArgs...>&>(other)) {}

        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<(sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<compressed_tuple<_UArgs...>&&>>,
                                                             std::is_constructible<_Args, _UArgs&&>...>,
                                   int> = 0>
        constexpr explicit(_Explicit_rref_ctor<_UArgs...>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, compressed_tuple<_UArgs...>&& other) noexcept(
                std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(tag, alloc, static_cast<_Compressed_tuple<0, _UArgs...>&&>(other)) {}

        template <class _Alloc, class _First, class _Second,
                  std::enable_if_t<_Tuple_constructible_v<const _First&, const _Second&>, int> = 0>
        constexpr explicit(!_Tuple_convertible_v<const _First&, const _Second&>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const std::pair<_First, _Second>& other) noexcept(
                _Tuple_nothrow_constructible_v<const _First&, const _Second&>)
            : _Base(tag, alloc, other.first, other.second) {}

        template <class _Alloc, class _First, class _Second, std::enable_if_t<_Tuple_constructible_v<_First, _Second>, int> = 0>
        constexpr explicit(!_Tuple_convertible_v<_First, _Second>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc,
                             std::pair<_First, _Second>&& other) noexcept(_Tuple_nothrow_constructible_v<_First, _Second>)
            : _Base(tag, alloc, std::forward<_First>(other.first), std::forward<_Second>(other.second)) {}
#else
        template <class _Dummy = void, std::enable_if_t<_Default_constructible_v<std::is_void_v<_Dummy>>
                                                            && !_Explicit_default_ctor<std::is_void_v<_Dummy>>,
                                                        int> = 0>  //_Dummy is used for SFINAE
        constexpr compressed_tuple() noexcept(std::conjunction_v<std::is_nothrow_default_constructible<_Args>...>) : _Base() {}
        template <class _Dummy = void, std::enable_if_t<_Default_constructible_v<std::is_void_v<_Dummy>>
                                                            && _Explicit_default_ctor<std::is_void_v<_Dummy>>,
                                                        int> = 0>
        constexpr explicit compressed_tuple() noexcept(std::conjunction_v<std::is_nothrow_default_constructible<_Args>...>)
            : _Base() {}

        template <class _Dummy = void,
                  std::enable_if_t<_Copy_constructible_v<std::is_void_v<_Dummy>> && !_Explicit_cref_ctor<_Args...>, int> = 0>
        constexpr compressed_tuple(const _Args&... args) noexcept(
            std::conjunction_v<std::is_nothrow_copy_constructible<_Args>...>)
            : _Base(args...) {}
        template <class _Dummy = void,
                  std::enable_if_t<_Copy_constructible_v<std::is_void_v<_Dummy>> && _Explicit_cref_ctor<_Args...>, int> = 0>
        constexpr explicit compressed_tuple(const _Args&... args) noexcept(
            std::conjunction_v<std::is_nothrow_copy_constructible<_Args>...>)
            : _Base(args...) {}

        template <class... _UArgs,
                  std::enable_if_t<!_Explicit_rref_ctor<_UArgs...>
                                       && std::conjunction_v<_Valid_args<_UArgs...>,
                                                             std::disjunction<std::is_convertible<_UArgs, _Ignore_t>,
                                                                              std::is_constructible<_Args, _UArgs&&>>...>,
                                   int> = 0>
        constexpr compressed_tuple(_UArgs&&... args) noexcept(std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(std::forward<_UArgs>(args)...) {}
        template <class... _UArgs,
                  std::enable_if_t<_Explicit_rref_ctor<_UArgs...>
                                       && std::conjunction_v<_Valid_args<_UArgs...>,
                                                             std::disjunction<std::is_convertible<_UArgs, _Ignore_t>,
                                                                              std::is_constructible<_Args, _UArgs&&>>...>,
                                   int> = 0>
        constexpr explicit compressed_tuple(_UArgs&&... args) noexcept(
            std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(std::forward<_UArgs>(args)...) {}

        template <class... _UArgs,
                  std::enable_if_t<!_Explicit_cref_ctor<_UArgs...> && (sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<const compressed_tuple<_UArgs...>&>>,
                                                             std::is_constructible<_Args, const _UArgs&>...>,
                                   int> = 0>
        constexpr compressed_tuple(const compressed_tuple<_UArgs...>& other) noexcept(
            std::conjunction_v<std::is_nothrow_copy_constructible<_UArgs>...>)
            : _Base(static_cast<const _Compressed_tuple<0, _UArgs...>&>(other)) {}
        template <class... _UArgs,
                  std::enable_if_t<_Explicit_cref_ctor<_UArgs...> && (sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<const compressed_tuple<_UArgs...>&>>,
                                                             std::is_constructible<_Args, const _UArgs&>...>,
                                   int> = 0>
        constexpr explicit compressed_tuple(const compressed_tuple<_UArgs...>& other) noexcept(
            std::conjunction_v<std::is_nothrow_copy_constructible<_UArgs>...>)
            : _Base(static_cast<const _Compressed_tuple<0, _UArgs...>&>(other)) {}

        template <class... _UArgs,
                  std::enable_if_t<!_Explicit_rref_ctor<_UArgs...> && (sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<compressed_tuple<_UArgs...>&&>>,
                                                             std::is_constructible<_Args, _UArgs&&>...>,
                                   int> = 0>
        constexpr compressed_tuple(compressed_tuple<_UArgs...>&& other) noexcept(
            std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(static_cast<_Compressed_tuple<0, _UArgs...>&&>(other)) {}
        template <class... _UArgs,
                  std::enable_if_t<_Explicit_rref_ctor<_UArgs...> && (sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<compressed_tuple<_UArgs...>&&>>,
                                                             std::is_constructible<_Args, _UArgs&&>...>,
                                   int> = 0>
        constexpr explicit compressed_tuple(compressed_tuple<_UArgs...>&& other) noexcept(
            std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(static_cast<_Compressed_tuple<0, _UArgs...>&&>(other)) {}

        template <class _First, class _Second,
                  std::enable_if_t<_Tuple_convertible_v<const _First&, const _Second&>
                                       && _Tuple_constructible_v<const _First&, const _Second&>,
                                   int> = 0>
        constexpr compressed_tuple(const std::pair<_First, _Second>& other) noexcept(
            _Tuple_nothrow_constructible_v<const _First&, const _Second&>)
            : _Base(other.first, other.second) {}
        template <class _First, class _Second,
                  std::enable_if_t<!_Tuple_convertible_v<const _First&, const _Second&>
                                       && _Tuple_constructible_v<const _First&, const _Second&>,
                                   int> = 0>
        constexpr explicit compressed_tuple(const std::pair<_First, _Second>& other) noexcept(
            _Tuple_nothrow_constructible_v<const _First&, const _Second&>)
            : _Base(other.first, other.second) {}

        template <class _First, class _Second,
                  std::enable_if_t<_Tuple_convertible_v<_First, _Second> && _Tuple_constructible_v<_First, _Second>, int> = 0>
        constexpr compressed_tuple(std::pair<_First, _Second>&& other) noexcept(_Tuple_nothrow_constructible_v<_First, _Second>)
            : _Base(std::forward<_First>(other.first), std::forward<_Second>(other.second)) {}
        template <class _First, class _Second,
                  std::enable_if_t<!_Tuple_convertible_v<_First, _Second> && _Tuple_constructible_v<_First, _Second>, int> = 0>
        constexpr explicit compressed_tuple(std::pair<_First, _Second>&& other) noexcept(
            _Tuple_nothrow_constructible_v<_First, _Second>)
            : _Base(std::forward<_First>(other.first), std::forward<_Second>(other.second)) {}

        //// Allocator-extended constructors.
        template <class _Alloc, std::enable_if_t<!_Explicit_default_ctor<std::is_object_v<_Alloc>>
                                                     && _Default_constructible_v<std::is_object_v<_Alloc>>,
                                                 int> = 0>
        constexpr compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc) : _Base(tag, alloc) {}
        template <class _Alloc, std::enable_if_t<_Explicit_default_ctor<std::is_object_v<_Alloc>>
                                                     && _Default_constructible_v<std::is_object_v<_Alloc>>,
                                                 int> = 0>
        constexpr explicit compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc) : _Base(tag, alloc) {}

        template <class _Alloc,
                  std::enable_if_t<!_Explicit_cref_ctor<_Args...> && _Copy_constructible_v<std::is_object_v<_Alloc>>, int> = 0>
        constexpr compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Args&... args) noexcept(
            std::conjunction_v<std::is_nothrow_copy_constructible<_Args>...>)
            : _Base(tag, args...) {}
        template <class _Alloc,
                  std::enable_if_t<_Explicit_cref_ctor<_Args...> && _Copy_constructible_v<std::is_object_v<_Alloc>>, int> = 0>
        constexpr explicit compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Args&... args) noexcept(
            std::conjunction_v<std::is_nothrow_copy_constructible<_Args>...>)
            : _Base(tag, args...) {}

        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<!_Explicit_rref_ctor<_UArgs...>
                                       && std::conjunction_v<_Valid_args<_UArgs...>, std::is_constructible<_Args, _UArgs&&>...>,
                                   int> = 0>
        constexpr compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc,
                                   _UArgs&&... args) noexcept(std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(tag, std::forward<_UArgs>(args)...) {}
        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<_Explicit_rref_ctor<_UArgs...>
                                       && std::conjunction_v<_Valid_args<_UArgs...>, std::is_constructible<_Args, _UArgs&&>...>,
                                   int> = 0>
        constexpr explicit compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _UArgs&&... args) noexcept(
            std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(tag, std::forward<_UArgs>(args)...) {}

        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<!_Explicit_cref_ctor<_UArgs...> && (sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<const compressed_tuple<_UArgs...>&>>,
                                                             std::is_constructible<_Args, const _UArgs&>...>,
                                   int> = 0>
        constexpr compressed_tuple(
            std::allocator_arg_t tag, const _Alloc& alloc,
            const compressed_tuple<_UArgs...>& other) noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<_UArgs>...>)
            : _Base(tag, alloc, static_cast<const _Compressed_tuple<0, _UArgs...>&>(other)) {}
        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<_Explicit_cref_ctor<_UArgs...> && (sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<const compressed_tuple<_UArgs...>&>>,
                                                             std::is_constructible<_Args, const _UArgs&>...>,
                                   int> = 0>
        constexpr explicit compressed_tuple(
            std::allocator_arg_t tag, const _Alloc& alloc,
            const compressed_tuple<_UArgs...>& other) noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<_UArgs>...>)
            : _Base(tag, alloc, static_cast<const _Compressed_tuple<0, _UArgs...>&>(other)) {}

        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<!_Explicit_rref_ctor<_UArgs...> && (sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<compressed_tuple<_UArgs...>&&>>,
                                                             std::is_constructible<_Args, _UArgs&&>...>,
                                   int> = 0>
        constexpr compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, compressed_tuple<_UArgs...>&& other) noexcept(
            std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(tag, alloc, static_cast<_Compressed_tuple<0, _UArgs...>&&>(other)) {}
        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<_Explicit_rref_ctor<_UArgs...> && (sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<compressed_tuple<_UArgs...>&&>>,
                                                             std::is_constructible<_Args, _UArgs&&>...>,
                                   int> = 0>
        constexpr explicit compressed_tuple(
            std::allocator_arg_t tag, const _Alloc& alloc,
            compressed_tuple<_UArgs...>&& other) noexcept(std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(tag, alloc, static_cast<_Compressed_tuple<0, _UArgs...>&&>(other)) {}

        template <class _Alloc, class _First, class _Second,
                  std::enable_if_t<_Tuple_convertible_v<const _First&, const _Second&>
                                       && _Tuple_constructible_v<const _First&, const _Second&>,
                                   int> = 0>
        constexpr compressed_tuple(
            std::allocator_arg_t tag, const _Alloc& alloc,
            const std::pair<_First, _Second>& other) noexcept(_Tuple_nothrow_constructible_v<const _First&, const _Second&>)
            : _Base(tag, alloc, other.first, other.second) {}
        template <class _Alloc, class _First, class _Second,
                  std::enable_if_t<!_Tuple_convertible_v<const _First&, const _Second&>
                                       && _Tuple_constructible_v<const _First&, const _Second&>,
                                   int> = 0>
        constexpr explicit compressed_tuple(
            std::allocator_arg_t tag, const _Alloc& alloc,
            const std::pair<_First, _Second>& other) noexcept(_Tuple_nothrow_constructible_v<const _First&, const _Second&>)
            : _Base(tag, alloc, other.first, other.second) {}

        template <class _Alloc, class _First, class _Second,
                  std::enable_if_t<_Tuple_convertible_v<_First, _Second> && _Tuple_constructible_v<_First, _Second>, int> = 0>
        constexpr compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc,
                                   std::pair<_First, _Second>&& other) noexcept(_Tuple_nothrow_constructible_v<_First, _Second>)
            : _Base(tag, alloc, std::forward<_First>(other.first), std::forward<_Second>(other.second)) {}
        template <class _Alloc, class _First, class _Second,
                  std::enable_if_t<!_Tuple_convertible_v<_First, _Second> && _Tuple_constructible_v<_First, _Second>, int> = 0>
        constexpr explicit compressed_tuple(
            std::allocator_arg_t tag, const _Alloc& alloc,
            std::pair<_First, _Second>&& other) noexcept(_Tuple_nothrow_constructible_v<_First, _Second>)
            : _Base(tag, alloc, std::forward<_First>(other.first), std::forward<_Second>(other.second)) {}
#endif

        constexpr compressed_tuple(const compressed_tuple&) = default;
        constexpr compressed_tuple(compressed_tuple&&)      = default;

        template <class _Alloc>
        constexpr compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const compressed_tuple& other)
            : _Base(tag, alloc, static_cast<const _Base&>(other)) {}

        template <class _Alloc>
        constexpr compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, compressed_tuple&& other)
            : _Base(tag, alloc, static_cast<_Base&&>(other)) {}
        //// compressed_tuple assignment
        constexpr std::conditional_t<std::conjunction_v<std::is_copy_assignable<_Args>...>, compressed_tuple&, void>
        operator=(const compressed_tuple& other) noexcept(std::conjunction_v<std::is_nothrow_copy_assignable<_Args>...>) {
            this->_Assign(other);
            return *this;
        }

        constexpr std::conditional_t<std::conjunction_v<std::is_move_assignable<_Args>...>, compressed_tuple&, void>
        operator=(compressed_tuple&& other) noexcept(std::conjunction_v<std::is_nothrow_move_assignable<_Args>...>) {
            this->_Assign(std::move(other));
            return *this;
        }

        template <class... _UArgs, std::enable_if_t<sizeof...(_Args) == sizeof...(_UArgs)
                                                        && std::conjunction_v<std::is_assignable<_Args&, const _UArgs&>...>,
                                                    int> = 0>
        constexpr compressed_tuple& operator=(const compressed_tuple<_UArgs...>& other) noexcept(
            std::conjunction_v<std::is_nothrow_copy_assignable<_UArgs>...>) {
            this->_Assign(other);
            return *this;
        }

        template <class... _UArgs, std::enable_if_t<sizeof...(_Args) == sizeof...(_UArgs)
                                                        && std::conjunction_v<std::is_assignable<_Args&, _UArgs>...>,
                                                    int> = 0>
        constexpr compressed_tuple&
        operator=(compressed_tuple<_UArgs...>&& other) noexcept(std::conjunction_v<std::is_nothrow_move_assignable<_UArgs>...>) {
            this->_Assign(std::move(other));
            return *this;
        }

        template <class _First, class _Second, std::enable_if_t<_Tuple_assignable_v<const _First&, const _Second&>, int> = 0>
        constexpr compressed_tuple&
        operator=(const std::pair<_First, _Second>& other) noexcept(_Tuple_nothrow_assignable_v<const _First&, const _Second&>) {
            this->_Get_first()             = std::forward<_First>(other.first);
            this->_Get_rest()._Get_first() = std::forward<_Second>(other.second);
            return *this;
        }

        template <class _First, class _Second, std::enable_if_t<_Tuple_assignable_v<_First, _Second>, int> = 0>
        constexpr compressed_tuple&
        operator=(std::pair<_First, _Second>&& other) noexcept(_Tuple_nothrow_assignable_v<_First, _Second>) {
            this->_Get_first()             = std::forward<_First>(other.first);
            this->_Get_rest()._Get_first() = std::forward<_Second>(other.second);
            return *this;
        }

        // compressed_tuple swap
        constexpr void swap(compressed_tuple& other) noexcept(std::conjunction_v<std::is_nothrow_swappable<_Args>...>) {
            _Base::swap(other);
        }
    };

    template <>
    class compressed_tuple<> {
    public:
        constexpr compressed_tuple() = default;
        constexpr compressed_tuple(const compressed_tuple&) noexcept {}

        template <class _Alloc>
        constexpr compressed_tuple(std::allocator_arg_t, const _Alloc&) noexcept {}
        template <class _Alloc>
        constexpr compressed_tuple(std::allocator_arg_t, const _Alloc&, const compressed_tuple&) noexcept {}

        constexpr compressed_tuple& operator=(const compressed_tuple&) = default;

        constexpr void swap(compressed_tuple&) noexcept {}

        constexpr bool _Equals(const compressed_tuple&) const noexcept { return true; }
#ifdef __cpp_lib_three_way_comparison
        XSTL_NODISCARD constexpr std::strong_ordering _Three_way_compare(const compressed_tuple&) const noexcept {
            return std::strong_ordering::equal;
        }
#else
        XSTL_NODISCARD constexpr bool _Less(const compressed_tuple&) const noexcept { return false; }
#endif
    };

    template <class... _Args1, class... _Args2>
    XSTL_NODISCARD constexpr bool operator==(const compressed_tuple<_Args1...>& lhs, const compressed_tuple<_Args2...>& rhs) {
        static_assert(sizeof...(_Args1) == sizeof...(_Args2), "cannot compare compressed_tuples of different sizes");
        return lhs._Equals(rhs);
    }

#ifdef __cpp_lib_three_way_comparison
    template <class... _Args1, class... _Args2>
    XSTL_NODISCARD constexpr std::common_comparison_category_t<synth_three_way_result<_Args1, _Args2>...>
    operator<=>(const compressed_tuple<_Args1...>& lhs, const compressed_tuple<_Args2...>& rhs) {
        static_assert(sizeof...(_Args1) == sizeof...(_Args2), "cannot compare compressed_tuples of different sizes");
        return lhs._Three_way_compare(rhs);
    }
#else
    template <class... _Args1, class... _Args2>
    XSTL_NODISCARD constexpr bool operator!=(const compressed_tuple<_Args1...>& lhs, const compressed_tuple<_Args2...>& rhs) {
        return !(lhs == rhs);
    }

    template <class... _Args1, class... _Args2>
    XSTL_NODISCARD constexpr bool operator<(const compressed_tuple<_Args1...>& lhs, const compressed_tuple<_Args2...>& rhs) {
        static_assert(sizeof...(_Args1) == sizeof...(_Args2), "cannot compare tuples of different sizes");
        return lhs._Less(rhs);
    }

    template <class... _Args1, class... _Args2>
    XSTL_NODISCARD constexpr bool operator>=(const compressed_tuple<_Args1...>& lhs, const compressed_tuple<_Args2...>& rhs) {
        return !(lhs < rhs);
    }

    template <class... _Args1, class... _Args2>
    XSTL_NODISCARD constexpr bool operator>(const compressed_tuple<_Args1...>& lhs, const compressed_tuple<_Args2...>& rhs) {
        return rhs < lhs;
    }

    template <class... _Args1, class... _Args2>
    XSTL_NODISCARD constexpr bool operator<=(const compressed_tuple<_Args1...>& lhs, const compressed_tuple<_Args2...>& rhs) {
        return !(rhs < lhs);
    }
#endif

    template <class... _Args>
    compressed_tuple(_Args...) -> compressed_tuple<_Args...>;
    template <class _First, class _Second>
    compressed_tuple(std::pair<_First, _Second>) -> compressed_tuple<_First, _Second>;
    template <class _Alloc, class... _Args>
    compressed_tuple(std::allocator_arg_t, _Alloc, _Args...) -> compressed_tuple<_Args...>;
    template <class _Alloc, class _First, class _Second>
    compressed_tuple(std::allocator_arg_t, _Alloc, std::pair<_First, _Second>) -> compressed_tuple<_First, _Second>;
    template <class _Alloc, class... _Args>
    compressed_tuple(std::allocator_arg_t, _Alloc, compressed_tuple<_Args...>) -> compressed_tuple<_Args...>;

    template <class... _Args>
    XSTL_NODISCARD constexpr auto make_ctuple(_Args&&... args) {
        return compressed_tuple<_Unrefwrap<std::decay_t<_Args>>::type...>(std::forward<_Args>(args)...);
    }

    template <class... _Args>
    XSTL_NODISCARD constexpr auto forward_as_ctuple(_Args&&... args) noexcept {
        return compressed_tuple<_Args&&...>(std::forward<_Args>(args)...);
    }

    template <class... _Args>
    XSTL_NODISCARD constexpr auto ctie(_Args&... args) noexcept {
        return compressed_tuple<_Args&...>(args...);
    }

    namespace {
        template <size_t _Idx, class _This, class... _Rest>
        constexpr _This& _Get_helper(_Compressed_tuple<_Idx, _This, _Rest...>& ctuple) noexcept {
            return ctuple._Get_first();
        }

        template <size_t _Idx, class _This, class... _Rest>
        constexpr const _This& _Get_helper(const _Compressed_tuple<_Idx, _This, _Rest...>& ctuple) noexcept {
            return ctuple._Get_first();
        }
        template <size_t _Idx, class... _Rest>
        std::enable_if_t<(_Idx >= sizeof...(_Rest))> _Get_helper(const compressed_tuple<_Rest...>&) = delete;

        template <class _Ty, class _Tuple>
        struct _Tuple_element {};

        template <class _This, class... _Rest>
        struct _Tuple_element<_This, compressed_tuple<_This, _Rest...>> {
            static_assert(!std::disjunction_v<std::is_same<_This, _Rest>...>, "duplicate type T in get<T>(tuple)");
            using type = compressed_tuple<_This, _Rest...>;
        };

        template <class _Ty, class _This, class... _Rest>
        struct _Tuple_element<_Ty, compressed_tuple<_This, _Rest...>> {
            using type = typename _Tuple_element<_Ty, compressed_tuple<_Rest...>>::type;
        };
    }  // namespace
}  // namespace xstl

namespace std {
    template <class... _Args>
    struct tuple_size<xstl::compressed_tuple<_Args...>> : public integral_constant<size_t, sizeof...(_Args)> {};

    template <class... _Args>
    inline constexpr size_t tuple_size_v<xstl::compressed_tuple<_Args...>> = sizeof...(_Args);

    template <class... _Args>
    inline constexpr size_t tuple_size_v<const xstl::compressed_tuple<_Args...>> = sizeof...(_Args);

    template <size_t _Idx>
    struct tuple_element<_Idx, xstl::compressed_tuple<>> {
        static_assert(xstl::always_false<decltype(_Idx)>, "compressed_tuple index out of bounds");
    };

    template <class _This, class... _Rest>
    struct tuple_element<0, xstl::compressed_tuple<_This, _Rest...>> {
        using type = _This;
    };

    template <size_t _Idx, class _This, class... _Rest>
    struct tuple_element<_Idx, xstl::compressed_tuple<_This, _Rest...>>
        : tuple_element<_Idx - 1, xstl::compressed_tuple<_Rest...>> {};

    template <size_t _Idx, class... _Args>
    constexpr tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>& get(xstl::compressed_tuple<_Args...>& ctuple) noexcept {
        return xstl::_Get_helper<_Idx>(ctuple);
    }

    template <size_t _Idx, class... _Args>
    constexpr const tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>&
    get(const xstl::compressed_tuple<_Args...>& ctuple) noexcept {
        return xstl::_Get_helper<_Idx>(ctuple);
    }

    template <size_t _Idx, class... _Args>
    constexpr tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>&& get(xstl::compressed_tuple<_Args...>&& ctuple) noexcept {
        return std::forward<tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>>(xstl::_Get_helper<_Idx>(ctuple));
    }

    template <size_t _Idx, class... _Args>
    constexpr const tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>&&
    get(const xstl::compressed_tuple<_Args...>&& ctuple) noexcept {
        return std::forward<const tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>>(xstl::_Get_helper<_Idx>(ctuple));
    }

    template <class _Ty, class... _Types>
    XSTL_NODISCARD constexpr _Ty& get(xstl::compressed_tuple<_Types...>& tuple) noexcept {
        using type = typename xstl::_Tuple_element<_Ty, xstl::compressed_tuple<_Types...>>::type;
        return static_cast<type&>(tuple)._Get_first();
    }

    template <class _Ty, class... _Types>
    XSTL_NODISCARD constexpr const _Ty& get(const xstl::compressed_tuple<_Types...>& tuple) noexcept {
        using type = typename xstl::_Tuple_element<_Ty, xstl::compressed_tuple<_Types...>>::type;
        return static_cast<const type&>(tuple)._Get_first();
    }

    template <class _Ty, class... _Types>
    XSTL_NODISCARD constexpr _Ty&& get(xstl::compressed_tuple<_Types...>&& tuple) noexcept {
        using type = typename xstl::_Tuple_element<_Ty, xstl::compressed_tuple<_Types...>>::type;
        return static_cast<_Ty&&>(static_cast<type&>(tuple)._Get_first());
    }

    template <class _Ty, class... _Types>
    XSTL_NODISCARD constexpr const _Ty&& get(const xstl::compressed_tuple<_Types...>&& tuple) noexcept {
        using type = typename xstl::_Tuple_element<_Ty, xstl::compressed_tuple<_Types...>>::type;
        return static_cast<const _Ty&&>(static_cast<const type&>(tuple)._Get_first());
    }

    template <class... _Args, class _Alloc>
    struct uses_allocator<xstl::compressed_tuple<_Args...>, _Alloc> : true_type {};
}  // namespace std

namespace xstl {  // implementation of ctuple_cat, which needs to use overload of std::get for compressed_tuple
    namespace {
        template <size_t, class, class, size_t>
        struct _Make_tuple_impl;
        template <size_t _Idx, class tuple, class... _Args, size_t _Size>
        struct _Make_tuple_impl<_Idx, compressed_tuple<_Args...>, tuple, _Size>
            : _Make_tuple_impl<_Idx + 1, compressed_tuple<_Args..., std::tuple_element_t<_Idx, tuple>>, tuple, _Size> {};
        template <size_t _Size, class tuple, class... _Args>
        struct _Make_tuple_impl<_Size, compressed_tuple<_Args...>, tuple, _Size> {
            using type = compressed_tuple<_Args...>;
        };
        template <class tuple>
        struct _Make_tuple : _Make_tuple_impl<0, compressed_tuple<>, tuple, std::tuple_size<tuple>::value> {};

        template <class...>
        struct _Make_indices;
        template <>
        struct _Make_indices<> {
            using type = std::index_sequence<>;
        };
        template <class tuple, class... _RestTpls>
        struct _Make_indices<tuple, _RestTpls...> {
            using type = typename std::make_index_sequence<std::tuple_size_v<typename std::remove_reference<tuple>::type>>;
        };
        template <class... _Tuples>
        using _Indices_t = typename _Make_indices<_Tuples...>::type;

        template <class _Ret, class _Indices, class... _RestTpls>
        struct _Tuple_concater;
        template <class _Ret, size_t... _Is, class tuple, class... _RestTpls>
        struct _Tuple_concater<_Ret, std::index_sequence<_Is...>, tuple, _RestTpls...> {
            template <class... _Args>
            static constexpr _Ret _Concat(tuple&& curr, _RestTpls&&... rest, _Args&&... args) {
                return _Tuple_concater<_Ret, _Indices_t<_RestTpls...>, _RestTpls...>::_Concat(
                    std::forward<_RestTpls>(rest)..., std::forward<_Args>(args)..., std::get<_Is>(std::forward<tuple>(curr))...);
            }
        };
        template <class _Ret>
        struct _Tuple_concater<_Ret, std::index_sequence<>> {
            template <class... _Args>
            static constexpr _Ret _Concat(_Args&&... args) {
                return _Ret(std::forward<_Args>(args)...);
            }
        };
        template <class...>
        struct _Combine_tuples;
        template <>
        struct _Combine_tuples<> {
            using type = compressed_tuple<>;
        };
        template <class... _Args>
        struct _Combine_tuples<compressed_tuple<_Args...>> {
            using type = compressed_tuple<_Args...>;
        };
        template <class... _Args1, class... _Args2, class... _RestTpls>
        struct _Combine_tuples<compressed_tuple<_Args1...>, compressed_tuple<_Args2...>, _RestTpls...> {
            using type = typename _Combine_tuples<compressed_tuple<_Args1..., _Args2...>, _RestTpls...>::type;
        };

        template <class... _Tuple>
        struct _Tuple_cat_result {
            using type =
                typename _Combine_tuples<typename _Make_tuple<std::remove_cv_t<std::remove_reference_t<_Tuple>>>::type...>::type;
        };
    }  // namespace
    template <class... _Tuples>
    XSTL_NODISCARD constexpr auto ctuple_cat(_Tuples&&... _RestTpls) {
        return _Tuple_concater<typename _Tuple_cat_result<_Tuples...>::type, _Indices_t<_Tuples...>, _Tuples...>::_Concat(
            std::forward<_Tuples>(_RestTpls)...);
    }
}  // namespace xstl
#endif