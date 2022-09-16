#pragma once
#ifndef _COMPRESSED_TUPLE_
#define _COMPRESSED_TUPLE_

#include <tuple>

namespace xstl {
    namespace {
        template <class _Curr, class _Alloc>
        using _Uses_allocator0 = std::enable_if_t<!std::uses_allocator_v<_Curr, _Alloc>, int>;
        template <class _Curr, class _Alloc, class... _Args>
        using _Uses_allocator1 = std::enable_if_t<std::uses_allocator_v<_Curr, _Alloc> && std::is_constructible_v<_Curr, std::allocator_arg_t, const _Alloc&, _Args...>, int>;
        template <class _Curr, class _Alloc, class... _Args>
        using _Uses_allocator2 = std::enable_if_t<std::uses_allocator_v<_Curr, _Alloc> && !std::is_constructible_v<_Curr, std::allocator_arg_t, const _Alloc&, _Args...>, int>;

        template <class _Tp>
        struct _Unrefwrap {
            using type = _Tp;
        };
        template <class _Tp>
        struct _Unrefwrap<std::reference_wrapper<_Tp>> {
            using type = _Tp&;
        };

        struct _Synth_three_way {
            template <class _Ty1, class _Ty2>
            [[nodiscard]] constexpr auto operator()(const _Ty1& lhs, const _Ty2& rhs) const {
                if constexpr (std::three_way_comparable_with<_Ty1, _Ty2>)
                    return lhs <=> rhs;
                else {
                    return lhs < rhs ? std::weak_ordering::less : rhs < lhs ? std::weak_ordering::greater : std::weak_ordering::equivalent;
                }
            }
        };
        template <class _Ty1, class _Ty2 = _Ty1>
        using _Synth_three_way_result = decltype(_Synth_three_way{}(std::declval<_Ty1&>(), std::declval<_Ty2&>()));

        using _Ignore_t = decltype(std::ignore);

        template <size_t _Idx, class _Curr, bool = std::is_empty_v<_Curr> && !std::is_final_v<_Curr>>
        struct _Curr_type : public _Curr {  // for empty base class optimization
            constexpr _Curr_type() : _Curr() {}
            constexpr _Curr_type(const _Curr& value) : _Curr(value) {}
            constexpr _Curr_type(const _Curr_type&) = default;
            constexpr _Curr_type(_Curr_type&&)      = default;

            template <class _Tp>
            constexpr _Curr_type(_Tp&& value) : _Curr(std::forward<_Tp>(value)) {}

            template <class _Alloc, class... _Args, _Uses_allocator0<_Curr, _Alloc> = 0>
            constexpr _Curr_type(const _Alloc&, std::allocator_arg_t, _Args&&... args) : _Curr(std::forward<_Args>(args)...) {}

            template <class _Alloc, class... _Args, _Uses_allocator1<_Curr, _Alloc, _Args...> = 0>
            constexpr _Curr_type(const _Alloc& alloc, std::allocator_arg_t, _Args&&... args) : _Curr(std::allocator_arg, alloc, std::forward<_Args>(args)...) {}

            template <class _Alloc, class... _Args, _Uses_allocator2<_Curr, _Alloc, _Args...> = 0>
            constexpr _Curr_type(const _Alloc& alloc, std::allocator_arg_t, _Args&&... args) : _Curr(std::forward<_Args>(args)..., alloc) {}

            constexpr _Curr&       _Get_curr() noexcept { return *this; }
            constexpr const _Curr& _Get_curr() const noexcept { return *this; }
        };

        template <size_t _Idx, class _Curr>
        struct _Curr_type<_Idx, _Curr, false> {
            constexpr _Curr_type() : _value() {}
            constexpr _Curr_type(const _Curr& value) : _value(value) {}
            constexpr _Curr_type(const _Curr_type&) = default;
            constexpr _Curr_type(_Curr_type&&)      = default;

            template <class _Tp>
            constexpr _Curr_type(_Tp&& value) : _value(std::forward<_Tp>(value)) {}

            template <class _Alloc, class... _Args, _Uses_allocator0<_Curr, _Alloc> = 0>
            constexpr _Curr_type(const _Alloc&, std::allocator_arg_t, _Args&&... args) : _value(std::forward<_Args>(args)...) {}

            template <class _Alloc, class... _Args, _Uses_allocator1<_Curr, _Alloc, _Args...> = 0>
            constexpr _Curr_type(const _Alloc& alloc, std::allocator_arg_t, _Args&&... args) : _value(std::allocator_arg, alloc, std::forward<_Args>(args)...) {}

            template <class _Alloc, class... _Args, _Uses_allocator2<_Curr, _Alloc, _Args...> = 0>
            constexpr _Curr_type(const _Alloc& alloc, std::allocator_arg_t, _Args&&... args) : _value(std::forward<_Args>(args)..., alloc) {}

            constexpr _Curr&       _Get_curr() noexcept { return _value; }
            constexpr const _Curr& _Get_curr() const noexcept { return _value; }

            _Curr _value;
        };

        template <size_t _Idx, class... _Args>  //_Idx is used for preventing the invalidity of empty base class optimization
        struct _Compressed_tuple;

        template <size_t _Idx, class _Curr, class... _Rest>
        struct _Compressed_tuple<_Idx, _Curr, _Rest...> : public _Compressed_tuple<_Idx + 1, _Rest...>, private _Curr_type<_Idx, _Curr> {

            using _Rest_tuple = _Compressed_tuple<_Idx + 1, _Rest...>;
            using _Base       = _Curr_type<_Idx, _Curr>;

            constexpr _Curr&             _Get_curr() noexcept { return _Base::_Get_curr(); }  // prevents ambiguity of invocation
            constexpr const _Curr&       _Get_curr() const noexcept { return _Base::_Get_curr(); }
            constexpr _Rest_tuple&       _Get_rest() noexcept { return *this; }  // get reference to rest of elements
            constexpr const _Rest_tuple& _Get_rest() const noexcept { return *this; }

            constexpr _Compressed_tuple() : _Rest_tuple(), _Base() {}

            explicit constexpr _Compressed_tuple(_Ignore_t, const _Rest&... rest) : _Rest_tuple(rest...), _Base() {}

            template <class... _URest, std::enable_if_t<sizeof...(_Rest) == sizeof...(_URest), int> = 0>
            explicit constexpr _Compressed_tuple(_Ignore_t, _URest&&... rest) : _Rest_tuple(std::forward<_URest>(rest)...), _Base() {}

            explicit constexpr _Compressed_tuple(const _Curr& curr, const _Rest&... rest) : _Rest_tuple(rest...), _Base(curr) {}

            template <class _UCurr, class... _URest, std::enable_if_t<sizeof...(_Rest) == sizeof...(_URest), int> = 0>
            explicit constexpr _Compressed_tuple(_UCurr&& curr, _URest&&... rest) : _Rest_tuple(std::forward<_URest>(rest)...), _Base(std::forward<_UCurr>(curr)) {}

            constexpr _Compressed_tuple(const _Compressed_tuple&) = default;
            _Compressed_tuple& operator=(const _Compressed_tuple&) = delete;
            _Compressed_tuple(_Compressed_tuple&&)                 = default;

            template <class... _Args>
            constexpr _Compressed_tuple(const _Compressed_tuple<_Idx, _Args...>& other) : _Rest_tuple(other._Get_rest()), _Base(other._Get_curr()) {}

            template <class _UCurr, class... _URest>
            constexpr _Compressed_tuple(_Compressed_tuple<_Idx, _UCurr, _URest...>&& other) : _Rest_tuple(std::move(other._Get_rest())), _Base(std::forward<_UCurr>(other._Get_curr())) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc) : _Rest_tuple(tag, alloc), _Base(tag, alloc) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Curr& curr, const _Rest&... rest) : _Rest_tuple(tag, alloc, rest...), _Base(alloc, curr) {}

            template <class _Alloc, class _UCurr, class... _URest, std::enable_if_t<sizeof...(_Rest) == sizeof...(_URest), int> = 0>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _UCurr&& curr, _URest&&... rest)
                : _Rest_tuple(tag, alloc, std::forward<_URest>(rest)...), _Base(alloc, std::forward<_UCurr>(curr)) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Compressed_tuple& other) : _Rest_tuple(tag, alloc, other._Get_rest()), _Base(alloc, other._Get_curr()) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _Compressed_tuple&& other)
                : _Rest_tuple(tag, alloc, std::move(other._Get_rest())), _Base(alloc, std::forward<_Curr>(other._Get_curr())) {}

            template <class _Alloc, class _UCurr, class... _URest>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Compressed_tuple<_Idx, _UCurr, _URest...>& other)
                : _Rest_tuple(tag, alloc, other._Get_rest()), _Base(alloc, other._Get_curr()) {}

            template <class _Alloc, class _UCurr, class... _URest>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _Compressed_tuple<_Idx, _UCurr, _URest...>&& other)
                : _Rest_tuple(tag, alloc, std::move(other._Get_rest())), _Base(alloc, std::forward<_UCurr>(other._Get_curr())) {}

            template <class _UCurr, class... _URest>
            constexpr bool _Equals(const _Compressed_tuple<_Idx, _UCurr, _URest...>& other) const noexcept {
                return _Get_curr() == other._Get_curr() && this->_Get_rest()._Equals(other._Get_rest());
            }

            template <class _UCurr, class... _URest>
            [[nodiscard]] constexpr std::common_comparison_category_t<xstl::_Synth_three_way_result<_Curr, _UCurr>> _Three_way_compare(const _Compressed_tuple<_Idx, _UCurr, _URest...>& other) const {
                if (auto _res = xstl::_Synth_three_way{}(_Get_curr(), other._Get_curr()); _res != 0)
                    return _res;
                return _Rest_tuple::_Three_way_compare(other._Get_rest());
            }

            template <class... _Args>
            constexpr void _Assign(const _Compressed_tuple<_Idx, _Args...>& other) {
                _Get_curr = other._Get_curr();
                this->_Get_rest()._Assign(other._Get_rest());
            }

            template <class _UCurr, class... _URest>
            constexpr void _Assign(_Compressed_tuple<_Idx, _UCurr, _URest...>&& other) {
                _Get_curr() = std::forward<_UCurr>(other._Get_curr());
                this->_Get_rest()._Assign(std::move(other._Get_rest()));
            }

            constexpr void swap(_Compressed_tuple& other) {
                using std::swap;
                swap(_Get_curr(), other._Get_curr());
                _Rest_tuple::swap(other._Get_rest());
            }
        };

        template <size_t _Idx, class _Curr>
        struct _Compressed_tuple<_Idx, _Curr> : private _Curr_type<_Idx, _Curr> {
            using _Base = _Curr_type<_Idx, _Curr>;
            using _Base::_Get_curr;

            constexpr _Compressed_tuple() : _Base() {}
            constexpr _Compressed_tuple(_Ignore_t) : _Base() {}
            explicit constexpr _Compressed_tuple(const _Curr& curr) : _Base(curr) {}
            template <class _UCurr>
            explicit constexpr _Compressed_tuple(_UCurr&& curr) : _Base(std::forward<_UCurr>(curr)) {}

            constexpr _Compressed_tuple(const _Compressed_tuple&) = default;
            _Compressed_tuple& operator=(const _Compressed_tuple&) = delete;

            constexpr _Compressed_tuple(_Compressed_tuple&& other) noexcept(std::is_nothrow_move_constructible_v<_Curr>) : _Base(static_cast<_Base&&>(other)) {}

            template <class _UCurr>
            constexpr _Compressed_tuple(const _Compressed_tuple<_Idx, _UCurr>& other) : _Base(other._Get_curr()) {}

            template <class _UCurr>
            constexpr _Compressed_tuple(_Compressed_tuple<_Idx, _UCurr>&& other) : _Base(std::forward<_UCurr>(other._Get_curr())) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc) : _Base(tag, alloc) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Curr& curr) : _Base(alloc, curr) {}

            template <class _Alloc, class _UCurr>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _UCurr&& curr) : _Base(alloc, std::forward<_UCurr>(curr)) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Compressed_tuple& other) : _Base(alloc, other._Get_curr()) {}

            template <class _Alloc>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _Compressed_tuple&& other) : _Base(alloc, std::forward<_Curr>(other._Get_curr())) {}

            template <class _Alloc, class _UCurr>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Compressed_tuple<_Idx, _UCurr>& other) : _Base(alloc, other._Get_curr()) {}

            template <class _Alloc, class _UCurr>
            constexpr _Compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _Compressed_tuple<_Idx, _UCurr>&& other) : _Base(alloc, std::forward<_UCurr>(other._Get_curr())) {}

            template <class _UCurr>
            constexpr bool _Equals(const _Compressed_tuple<_Idx, _UCurr>& other) const noexcept {
                return _Get_curr() == other._Get_curr();
            }

            template <class _UCurr>
            [[nodiscard]] constexpr xstl::_Synth_three_way_result<_Curr, _UCurr> _Three_way_compare(const _Compressed_tuple<_Idx, _UCurr>& other) const {
                if (auto _res = xstl::_Synth_three_way{}(_Get_curr(), other._Get_curr()); _res != 0)
                    return _res;
                return std::strong_ordering::equal;
            }

            template <class _UCurr>
            constexpr void _Assign(const _Compressed_tuple<_Idx, _UCurr>& other) {
                other._Get_curr();
            }

            template <class _UCurr>
            constexpr void _Assign(_Compressed_tuple<_Idx, _UCurr>&& other) {
                _Get_curr() = std::forward<_UCurr>(other._Get_curr());
            }

            constexpr void swap(_Compressed_tuple& other) {
                using std::swap;
                swap(this->_Get_curr(), other._Get_curr());
            }
        };

    }  // namespace

    template <class... _Args>
    class compressed_tuple : public _Compressed_tuple<0, _Args...> {
    public:
        using _Base = _Compressed_tuple<0, _Args...>;

        template <class _Tuple, class = compressed_tuple, class = std::remove_cvref_t<_Tuple>>
        struct _Use_other_ctor : std::false_type {};
        template <class _Tuple, class _Tp, class _Ty>
        struct _Use_other_ctor<_Tuple, compressed_tuple<_Tp>, compressed_tuple<_Ty>> : std::disjunction<std::is_convertible<_Tuple, _Tp>, std::is_constructible<_Tp, _Tuple>> {};
        template <class _Tuple, class _Tp>
        struct _Use_other_ctor<_Tuple, compressed_tuple<_Tp>, compressed_tuple<_Tp>> : std::true_type {};

        template <class _Ty>
        void _Implicitly_default_construct(const _Ty&) {}  // silence warning
        template <class _Ty, class = void>
        struct _Implicitly_default_constructible : std::false_type {};
        template <class _Ty>
        struct _Implicitly_default_constructible<_Ty, std::void_t<decltype(_Implicitly_default_construct<_Ty>({}))>> : std::true_type {};

        template <class...>
        struct _Valid_args : std::false_type {};
        template <class _Tp>
        struct _Valid_args<_Tp> : std::conjunction<std::bool_constant<sizeof...(_Args) == 1>, std::negation<std::is_same<compressed_tuple, std::remove_cvref_t<_Tp>>>> {};
        template <class _Arg1, class _Arg2, class... _URest>
        struct _Valid_args<_Arg1, _Arg2, _URest...> : std::bool_constant<(sizeof...(_URest) + 2) == sizeof...(_Args)> {};

        template <class... _UArgs>
        static constexpr bool _Tuple_constructible_v = sizeof...(_Args) == sizeof...(_UArgs) && std::conjunction_v<std::is_constructible<_Args, _UArgs>...>;
        template <class... _UArgs>
        static constexpr bool _Tuple_convertible_v = std::conjunction_v<std::is_convertible<_UArgs, _Args>...>;
        template <class... _UArgs>
        static constexpr bool _Tuple_nothrow_constructible_v = std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>;
        template <class... _UArgs>
        static constexpr bool _Tuple_assignable_v = sizeof...(_Args) == sizeof...(_UArgs) && std::conjunction_v<std::is_assignable<_Args&, _UArgs>...>;
        template <class... _UArgs>
        static constexpr bool _Tuple_nothrow_assignable_v = std::conjunction_v<std::is_nothrow_assignable<_Args&, _UArgs>...>;
        template <bool>
        static constexpr bool _Default_constructible_v = std::conjunction_v<std::is_default_constructible<_Args>...>;
        template <bool>
        static constexpr bool _Copy_constructible_v = sizeof...(_Args) >= 1 && std::conjunction_v<std::disjunction<std::is_convertible<_Args, _Ignore_t>, std::is_copy_constructible<_Args>>...>;

    public:
        template <class _Dummy = void, std::enable_if_t<_Default_constructible_v<std::is_void_v<_Dummy>>, int> = 0>  //_Dummy is used for SFINAE
        constexpr explicit(!std::conjunction_v<_Implicitly_default_constructible<_Args>...>) compressed_tuple() noexcept(std::conjunction_v<std::is_nothrow_default_constructible<_Args>...>)
            : _Base() {}

        template <class _Dummy = void, std::enable_if_t<_Copy_constructible_v<std::is_void_v<_Dummy>>, int> = 0>
        constexpr explicit(!std::conjunction_v<std::is_convertible<const _Args&, _Args>...>)
            compressed_tuple(const _Args&... args) noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<_Args>...>)
            : _Base(args...) {}

        template <class... _UArgs,
                  std::enable_if_t<std::conjunction_v<_Valid_args<_UArgs...>, std::disjunction<std::is_convertible<_UArgs, _Ignore_t>, std::is_constructible<_Args, _UArgs&&>>...>, int> = 0>
        constexpr explicit(!std::conjunction_v<std::is_convertible<_UArgs&&, _Args>...>)
            compressed_tuple(_UArgs&&... args) noexcept(std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(std::forward<_UArgs>(args)...) {}

        constexpr compressed_tuple(const compressed_tuple&) = default;
        constexpr compressed_tuple(compressed_tuple&&)      = default;

        template <class... _UArgs, std::enable_if_t<(sizeof...(_UArgs) == sizeof...(_Args))
                                                        && std::conjunction_v<std::negation<_Use_other_ctor<const compressed_tuple<_UArgs...>&>>, std::is_constructible<_Args, const _UArgs&>...>,
                                                    int> = 0>
        constexpr explicit(!std::conjunction_v<std::is_convertible<const _UArgs&, _Args>...>)
            compressed_tuple(const compressed_tuple<_UArgs...>& other) noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<_UArgs>...>)
            : _Base(static_cast<const _Compressed_tuple<0, _UArgs...>&>(other)) {}

        template <class... _UArgs,
                  std::enable_if_t<
                      (sizeof...(_UArgs) == sizeof...(_Args)) && std::conjunction_v<std::negation<_Use_other_ctor<compressed_tuple<_UArgs...>&&>>, std::is_constructible<_Args, _UArgs&&>...>, int> = 0>
        constexpr explicit(!std::conjunction_v<std::is_convertible<_UArgs&&, _Args>...>)
            compressed_tuple(compressed_tuple<_UArgs...>&& other) noexcept(std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(static_cast<_Compressed_tuple<0, _UArgs...>&&>(other)) {}

        template <class _First, class _Second, std::enable_if_t<_Tuple_constructible_v<const _First&, const _Second&>, int> = 0>
        constexpr explicit(!_Tuple_convertible_v<const _First&, const _Second&>)
            compressed_tuple(const std::pair<_First, _Second>& other) noexcept(_Tuple_nothrow_constructible_v<const _First&, const _Second&>)
            : _Base(other.first, other.second) {}

        template <class _First, class _Second, std::enable_if_t<_Tuple_constructible_v<_First, _Second>, int> = 0>
        constexpr explicit(!_Tuple_convertible_v<_First, _Second>) compressed_tuple(std::pair<_First, _Second>&& other) noexcept(_Tuple_nothrow_constructible_v<_First, _Second>)
            : _Base(std::forward<_First>(other.first), std::forward<_Second>(other.second)) {}

        //// Allocator-extended constructors.

        template <class _Alloc, std::enable_if_t<_Default_constructible_v<std::is_object_v<_Alloc>>, int> = 0>
        constexpr explicit(!std::conjunction_v<_Implicitly_default_constructible<_Args>...>) compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc) : _Base(tag, alloc) {}

        template <class _Alloc, std::enable_if_t<_Copy_constructible_v<std::is_object_v<_Alloc>>, int> = 0>
        constexpr explicit(!std::conjunction_v<std::is_convertible<const _Args&, _Args>...>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const _Args&... args) noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<_Args>...>)
            : _Base(tag, args...) {}

        template <class _Alloc, class... _UArgs, std::enable_if_t<std::conjunction_v<_Valid_args<_UArgs...>, std::is_constructible<_Args, _UArgs&&>...>, int> = 0>
        constexpr explicit(!std::conjunction_v<std::is_convertible<_UArgs&&, _Args>...>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, _UArgs&&... args) noexcept(std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(tag, std::forward<_UArgs>(args)...) {}

        template <class _Alloc>
        constexpr compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const compressed_tuple& other) : _Base(tag, alloc, static_cast<const _Base&>(other)) {}

        template <class _Alloc>
        constexpr compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, compressed_tuple&& other) : _Base(tag, alloc, static_cast<_Base&&>(other)) {}

        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<(sizeof...(_UArgs) == sizeof...(_Args))
                                       && std::conjunction_v<std::negation<_Use_other_ctor<const compressed_tuple<_UArgs...>&>>, std::is_constructible<_Args, const _UArgs&>...>,
                                   int> = 0>
        constexpr explicit(!std::conjunction_v<std::is_convertible<const _UArgs&, _Args>...>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const compressed_tuple<_UArgs...>& other) noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<_UArgs>...>)
            : _Base(tag, alloc, static_cast<const _Compressed_tuple<0, _UArgs...>&>(other)) {}

        template <class _Alloc, class... _UArgs,
                  std::enable_if_t<
                      (sizeof...(_UArgs) == sizeof...(_Args)) && std::conjunction_v<std::negation<_Use_other_ctor<compressed_tuple<_UArgs...>&&>>, std::is_constructible<_Args, _UArgs&&>...>, int> = 0>
        constexpr explicit(!std::conjunction_v<std::is_convertible<_UArgs&&, _Args>...>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, compressed_tuple<_UArgs...>&& other) noexcept(std::conjunction_v<std::is_nothrow_constructible<_Args, _UArgs>...>)
            : _Base(tag, alloc, static_cast<_Compressed_tuple<0, _UArgs...>&&>(other)) {}

        template <class _Alloc, class _First, class _Second, std::enable_if_t<_Tuple_constructible_v<const _First&, const _Second&>, int> = 0>
        constexpr explicit(!_Tuple_convertible_v<const _First&, const _Second&>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, const std::pair<_First, _Second>& other) noexcept(_Tuple_nothrow_constructible_v<const _First&, const _Second&>)
            : _Base(tag, alloc, other.first, other.second) {}

        template <class _Alloc, class _First, class _Second, std::enable_if_t<_Tuple_constructible_v<_First, _Second>, int> = 0>
        constexpr explicit(!_Tuple_convertible_v<_First, _Second>)
            compressed_tuple(std::allocator_arg_t tag, const _Alloc& alloc, std::pair<_First, _Second>&& other) noexcept(_Tuple_nothrow_constructible_v<_First, _Second>)
            : _Base(tag, alloc, std::forward<_First>(other.first), std::forward<_Second>(other.second)) {}

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

        template <class... _UArgs, std::enable_if_t<sizeof...(_Args) == sizeof...(_UArgs) && std::conjunction_v<std::is_assignable<_Args&, const _UArgs&>...>, int> = 0>
        constexpr compressed_tuple& operator=(const compressed_tuple<_UArgs...>& other) noexcept(std::conjunction_v<std::is_nothrow_copy_assignable<_UArgs>...>) {
            this->_Assign(other);
            return *this;
        }

        template <class... _UArgs, std::enable_if_t<sizeof...(_Args) == sizeof...(_UArgs) && std::conjunction_v<std::is_assignable<_Args&, _UArgs>...>, int> = 0>
        constexpr compressed_tuple& operator=(compressed_tuple<_UArgs...>&& other) noexcept(std::conjunction_v<std::is_nothrow_move_assignable<_UArgs>...>) {
            this->_Assign(std::move(other));
            return *this;
        }

        template <class _First, class _Second, std::enable_if_t<_Tuple_assignable_v<const _First&, const _Second&>, int> = 0>
        constexpr compressed_tuple& operator=(const std::pair<_First, _Second>& other) noexcept(_Tuple_nothrow_assignable_v<const _First&, const _Second&>) {
            this->_Get_curr()             = std::forward<_First>(other.first);
            this->_Get_rest()._Get_curr() = std::forward<_Second>(other.second);
            return *this;
        }

        template <class _First, class _Second, std::enable_if_t<_Tuple_assignable_v<_First, _Second>, int> = 0>
        constexpr compressed_tuple& operator=(std::pair<_First, _Second>&& other) noexcept(_Tuple_nothrow_assignable_v<_First, _Second>) {
            this->_Get_curr()             = std::forward<_First>(other.first);
            this->_Get_rest()._Get_curr() = std::forward<_Second>(other.second);
            return *this;
        }

        // compressed_tuple swap
        constexpr void swap(compressed_tuple& other) noexcept(std::conjunction_v<std::is_nothrow_swappable<_Args>...>) { _Base::swap(other); }
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

        [[nodiscard]] constexpr std::strong_ordering _Three_way_compare(const compressed_tuple&) const noexcept { return std::strong_ordering::equal; }
    };

    template <class... _Args1, class... _Args2>
    [[nodiscard]] constexpr bool operator==(const compressed_tuple<_Args1...>& lhs, const compressed_tuple<_Args2...>& rhs) {
        static_assert(sizeof...(_Args1) == sizeof...(_Args2), "cannot compare compressed_tuples of different sizes");
        return lhs._Equals(rhs);
    }

    template <class... _Args1, class... _Args2>
    [[nodiscard]] constexpr std::common_comparison_category_t<_Synth_three_way_result<_Args1, _Args2>...> operator<=>(const compressed_tuple<_Args1...>& lhs, const compressed_tuple<_Args2...>& rhs) {
        static_assert(sizeof...(_Args1) == sizeof...(_Args2), "cannot compare compressed_tuples of different sizes");
        return lhs._Three_way_compare(rhs);
    }

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
    [[nodiscard]] constexpr auto make_ctuple(_Args&&... args) {

        return compressed_tuple<_Unrefwrap<std::decay_t<_Args>>::type...>(std::forward<_Args>(args)...);
    }

    template <class... _Args>
    [[nodiscard]] constexpr auto forward_as_ctuple(_Args&&... args) noexcept {
        return compressed_tuple<_Args&&...>(std::forward<_Args>(args)...);
    }

    template <class... _Args>
    [[nodiscard]] constexpr auto ctie(_Args&... args) noexcept {
        return compressed_tuple<_Args&...>(args...);
    }

    namespace {
        template <size_t _Idx, class _Curr, class... _Rest>
        constexpr _Curr& _Get_helper(_Compressed_tuple<_Idx, _Curr, _Rest...>& ctuple) noexcept {
            return ctuple._Get_curr();
        }

        template <size_t _Idx, class _Curr, class... _Rest>
        constexpr const _Curr& _Get_helper(const _Compressed_tuple<_Idx, _Curr, _Rest...>& ctuple) noexcept {
            return ctuple._Get_curr();
        }
        template <size_t _Idx, class... _Rest>
        std::enable_if_t<(_Idx >= sizeof...(_Rest))> _Get_helper(const compressed_tuple<_Rest...>&) = delete;
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
        template <size_t>
        constexpr static bool _False = false;  // false value attached to a dependent name (for static_assert)
        static_assert(_False<_Idx>, "compressed_tuple index out of bounds");
    };

    template <class _Curr, class... _Rest>
    struct tuple_element<0, xstl::compressed_tuple<_Curr, _Rest...>> {
        using type   = _Curr;
        using _Ttype = xstl::compressed_tuple<_Curr, _Rest...>;
    };

    template <size_t _Idx, class _Curr, class... _Rest>
    struct tuple_element<_Idx, xstl::compressed_tuple<_Curr, _Rest...>> : tuple_element<_Idx - 1, xstl::compressed_tuple<_Rest...>> {};

    template <size_t _Idx, class... _Args>
    constexpr tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>& get(xstl::compressed_tuple<_Args...>& ctuple) noexcept {
        return xstl::_Get_helper<_Idx>(ctuple);
    }

    template <size_t _Idx, class... _Args>
    constexpr const tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>& get(const xstl::compressed_tuple<_Args...>& ctuple) noexcept {
        return xstl::_Get_helper<_Idx>(ctuple);
    }

    template <size_t _Idx, class... _Args>
    constexpr tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>&& get(xstl::compressed_tuple<_Args...>&& ctuple) noexcept {
        return std::forward<tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>>(xstl::_Get_helper<_Idx>(ctuple));
    }

    template <size_t _Idx, class... _Args>
    constexpr const tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>&& get(const xstl::compressed_tuple<_Args...>&& ctuple) noexcept {
        return std::forward<const tuple_element_t<_Idx, xstl::compressed_tuple<_Args...>>>(xstl::_Get_helper<_Idx>(ctuple));
    }

    template <class... _Args, class _Alloc>
    struct uses_allocator<xstl::compressed_tuple<_Args...>, _Alloc> : true_type {};
}  // namespace std

namespace xstl {  // implementation of ctuple_cat, which needs to use overload of std::get for compressed_tuple
    namespace {
        template <size_t, class, class, size_t>
        struct _Make_tuple_impl;
        template <size_t _Idx, class _Tuple, class... _Args, size_t _Size>
        struct _Make_tuple_impl<_Idx, compressed_tuple<_Args...>, _Tuple, _Size> : _Make_tuple_impl<_Idx + 1, compressed_tuple<_Args..., std::tuple_element_t<_Idx, _Tuple>>, _Tuple, _Size> {};
        template <size_t _Size, class _Tuple, class... _Args>
        struct _Make_tuple_impl<_Size, compressed_tuple<_Args...>, _Tuple, _Size> {
            using type = compressed_tuple<_Args...>;
        };
        template <class _Tuple>
        struct _Make_tuple : _Make_tuple_impl<0, compressed_tuple<>, _Tuple, std::tuple_size<_Tuple>::value> {};

        template <class...>
        struct _Make_indices;
        template <>
        struct _Make_indices<> {
            using type = std::index_sequence<>;
        };
        template <class _Tuple, class... _RestTpls>
        struct _Make_indices<_Tuple, _RestTpls...> {
            using type = typename std::make_index_sequence<std::tuple_size_v<typename std::remove_reference<_Tuple>::type>>;
        };
        template <class... _Tuples>
        using _Indices_t = typename _Make_indices<_Tuples...>::type;

        template <class _Ret, class _Indices, class... _RestTpls>
        struct _Tuple_concater;
        template <class _Ret, size_t... _Is, class _Tuple, class... _RestTpls>
        struct _Tuple_concater<_Ret, std::index_sequence<_Is...>, _Tuple, _RestTpls...> {
            template <class... _Args>
            static constexpr _Ret _Concat(_Tuple&& curr, _RestTpls&&... rest, _Args&&... args) {
                return _Tuple_concater<_Ret, _Indices_t<_RestTpls...>, _RestTpls...>::_Concat(std::forward<_RestTpls>(rest)..., std::forward<_Args>(args)...,
                                                                                              std::get<_Is>(std::forward<_Tuple>(curr))...);
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
            using type = typename _Combine_tuples<typename _Make_tuple<std::remove_cvref_t<_Tuple>>::type...>::type;
        };
    }  // namespace
    template <class... _Tuples>
    [[nodiscard]] constexpr auto ctuple_cat(_Tuples&&... _RestTpls) {
        return _Tuple_concater<typename _Tuple_cat_result<_Tuples...>::type, _Indices_t<_Tuples...>, _Tuples...>::_Concat(std::forward<_Tuples>(_RestTpls)...);
    }
}  // namespace xstl
#endif