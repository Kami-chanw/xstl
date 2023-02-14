#pragma once
#ifndef _XSTL_ITER_HPP_
#define _XSTL_ITER_HPP_
#include "utility.hpp"

#include <memory>  // for std::pointer_traits

namespace xstl {
    template <class _Ty, class = void>
    struct get_pointer {
        using type = typename _Ty::value_type*;
    };

    template <class _Ty>
    struct get_pointer<_Ty, std::void_t<typename _Ty::pointer>> {
        using type = typename _Ty::pointer;
    };

    template <class _Ty>
    using get_pointer_t = typename get_pointer<_Ty>::type;

    template <class _Ty, class = void>
    struct get_const_pointer {
        using type = typename std::pointer_traits<get_pointer_t<_Ty>>::template rebind<const typename _Ty::value_type>;
    };

    template <class _Ty>
    struct get_const_pointer<_Ty, std::void_t<typename _Ty::const_pointer>> {
        using type = typename _Ty::const_pointer;
    };

    template <class _Ty>
    using get_const_pointer_t = typename get_const_pointer<_Ty>::type;

    template <class _Ty, class = void>
    struct get_reference {
        using type = typename _Ty::value_type&;
    };

    template <class _Ty>
    struct get_reference<_Ty, std::void_t<typename _Ty::reference>> {
        using type = typename _Ty::reference;
    };

    template <class _Ty>
    using get_reference_t = typename get_reference<_Ty>::type;

    template <class _Ty, class = void>
    struct get_const_reference {
        using type = const typename _Ty::value_type&;
    };

    template <class _Ty>
    struct get_const_reference<_Ty, std::void_t<typename _Ty::const_reference>> {
        using type = typename _Ty::const_reference;
    };

    template <class _Ty>
    using get_const_reference_t = typename get_const_reference<_Ty>::type;

    template <class _Ty, class = void>
    struct get_difference {
        using type = typename std::pointer_traits<get_pointer_t<_Ty>>::difference_type;
    };

    template <class _Ty>
    struct get_difference<_Ty, std::void_t<typename _Ty::difference_type>> {
        using type = typename _Ty::difference_type;
    };

    template <class _Ty>
    using get_difference_t = typename get_difference<_Ty>::type;

    template <class _Dft, class _Ty, class = void>
    struct get_deref {
        using type = _Dft;
    };

    template <class _Dft, class _Ty>
    struct get_deref<_Dft, _Ty, std::void_t<typename _Ty::deref_type>> {
        using type = typename _Ty::deref_type;
    };

    template <class _Dft, class _Ty>
    using get_deref_t = typename get_deref<_Dft, _Ty>::type;

    template <class _Iter, class _Cat, class = void>
    struct is_iterator : std::false_type {};

    template <class _Iter, class _Cat>
    struct is_iterator<_Iter, _Cat, std::void_t<typename std::iterator_traits<_Iter>::iterator_category>>
        : std::is_convertible<typename std::iterator_traits<_Iter>::iterator_category, _Cat> {};

    template <class _Iter>
    using is_input_iterator = is_iterator<_Iter, std::input_iterator_tag>;

    template <class _Iter>
    inline constexpr bool is_input_iterator_v = is_input_iterator<_Iter>::value;

    template <class _Iter>
    using is_output_iterator = is_iterator<_Iter, std::output_iterator_tag>;

    template <class _Iter>
    inline constexpr bool is_output_iterator_v = is_output_iterator<_Iter>::value;

    template <class _Iter>
    using is_forward_iterator = is_iterator<_Iter, std::forward_iterator_tag>;

    template <class _Iter>
    inline constexpr bool is_forward_iterator_v = is_forward_iterator<_Iter>::value;

    template <class _Iter>
    using is_bidirectional_iterator = is_iterator<_Iter, std::bidirectional_iterator_tag>;

    template <class _Iter>
    inline constexpr bool is_bidirectional_iterator_v = is_bidirectional_iterator<_Iter>::value;

    template <class _Iter>
    using is_random_access_iterator = is_iterator<_Iter, std::random_access_iterator_tag>;

    template <class _Iter>
    inline constexpr bool is_random_access_iterator_v =
        std::is_convertible_v<typename std::iterator_traits<_Iter>::iterator_category, std::random_access_iterator_tag>;

#ifdef __cpp_lib_concepts
    template <class _Iter>
    using is_contiguous_iterator = std::is_convertible<typename _Iter::iterator_category, std::contiguous_iterator_tag>;
    template <class _Iter>
    inline constexpr bool is_contiguous_iterator_v = is_contiguous_iterator<_Iter>::value;
#endif

    struct container_val_base {};

    namespace iter_adapter {
        template <class _Nodeptr_type, class _Value_type, class _Size_type, class _Difference_type = std::ptrdiff_t,
                  class _Pointer = _Value_type*, class _Const_pointer = const _Value_type*, class _Reference = _Value_type&,
                  class _Const_reference = const _Value_type&>
        struct scary_iter_types {  // for SCARY machinery
            using value_type      = _Value_type;
            using size_type       = _Size_type;
            using difference_type = _Difference_type;
            using pointer         = _Pointer;
            using const_pointer   = _Const_pointer;
            using reference       = _Reference;
            using const_reference = _Const_reference;
            using _Node           = std::remove_pointer_t<_Nodeptr_type>;
            using _Nodeptr        = _Nodeptr_type;
        };

        template <class _Scary_val>
        struct iter_base {
            using _Nodeptr = typename _Scary_val::_Nodeptr;
            static_assert(std::is_default_constructible_v<_Nodeptr>, "node pointer should be default constructible");

            constexpr iter_base() = default;
            constexpr iter_base(_Nodeptr node, const container_val_base* cont) noexcept : _node(node), _pcont(cont) {}

            XSTL_NODISCARD constexpr _Nodeptr                  base() const noexcept { return _node; }
            XSTL_NODISCARD constexpr const container_val_base* _Get_cont() const noexcept { return _pcont; }

        protected:
            const container_val_base* _pcont = nullptr;
            _Nodeptr                  _node{};
        };

        template <class _Scary_val>
        struct unchecked_iter_base {
            using _Nodeptr = typename _Scary_val::_Nodeptr;
            static_assert(std::is_default_constructible_v<_Nodeptr>, "node pointer should be default constructible");

            constexpr unchecked_iter_base() = default;
            constexpr unchecked_iter_base(_Nodeptr node, const container_val_base*) : _node(node) {}

            XSTL_NODISCARD constexpr _Nodeptr                  base() const noexcept { return _node; }
            XSTL_NODISCARD constexpr const container_val_base* _Get_cont() const noexcept { return nullptr; }

        protected:
            _Nodeptr _node{};
        };

#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
        template <class _Scary_val>
        using default_iter_base = iter_base<_Scary_val>;
#else
        template <class _Scary_val>
        using default_iter_base = unchecked_iter_base<_Scary_val>;
#endif

        template <class _Scary_val, class _Cat, class _Base = unchecked_iter_base<_Scary_val>>
        class unchecked_const_iterator : public _Base {
            using _Self    = unchecked_const_iterator<_Scary_val, _Cat, _Base>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_const_pointer_t<_Scary_val>;
            using reference         = get_const_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = _Cat;

        protected:
            using _ScaryVal = _Scary_val;

            // noexcept(++*this)
            template <class _Cate, auto = combined_flag_v<std::is_convertible<_Cate, std::random_access_iterator_tag>,
                                                          std::is_convertible<_Cate, std::forward_iterator_tag>>>
            struct is_nothrow_unchecked_pre_increasable : std::true_type {};
            template <class _Cate>
            struct is_nothrow_unchecked_pre_increasable<_Cate, 0b11>
                : std::bool_constant<noexcept(_Scary_val::fwd(std::declval<_Nodeptr&>(), difference_type{ 1 }))> {};
            template <class _Cate>
            struct is_nothrow_unchecked_pre_increasable<_Cate, 0b01>
                : std::bool_constant<noexcept(_Scary_val::incr(std::declval<_Nodeptr&>()))> {};
            static constexpr bool is_nothrow_unchecked_pre_increasable_v = is_nothrow_unchecked_pre_increasable<_Cat>::value;

            // noexcept(--*this)
            template <class _Cate, auto = combined_flag_v<std::is_convertible<_Cate, std::random_access_iterator_tag>,
                                                          std::is_convertible<_Cate, std::bidirectional_iterator_tag>>>
            struct is_nothrow_unchecked_pre_decreasable : std::true_type {};
            template <class _Cate>
            struct is_nothrow_unchecked_pre_decreasable<_Cate, 0b11>
                : std::bool_constant<noexcept(_Scary_val::fwd(std::declval<_Nodeptr&>(), difference_type{ -1 }))> {};
            template <class _Cate>
            struct is_nothrow_unchecked_pre_decreasable<_Cate, 0b01>
                : std::bool_constant<noexcept(_Scary_val::decr(std::declval<_Nodeptr&>()))> {};
            static constexpr bool is_nothrow_unchecked_pre_decreasable_v = is_nothrow_unchecked_pre_decreasable<_Cat>::value;

            // noexcept(*this++)
            static constexpr bool is_nothrow_unchecked_post_increasable_v =
                is_nothrow_unchecked_pre_increasable_v /*&& std::is_nothrow_copy_constructible_v<_Self>*/;

            // noexcept(*this--)
            static constexpr bool is_nothrow_unchecked_post_decreasable_v =
                is_nothrow_unchecked_pre_decreasable_v /* && std::is_nothrow_copy_constructible_v<_Self>*/;

            // noexcept(**this) / noexcept(this->operator->())
            static constexpr bool is_nothrow_unchecked_dereferable_v = noexcept(_Scary_val::extract(_Nodeptr{}));

        public:
            constexpr unchecked_const_iterator() = default;
            constexpr unchecked_const_iterator(_Nodeptr node, const container_val_base* cont) noexcept(
                std::is_nothrow_copy_constructible_v<_Nodeptr>)
                : _Base(node, cont) {}

            constexpr reference operator*() const noexcept(is_nothrow_unchecked_dereferable_v) {
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept(is_nothrow_unchecked_dereferable_v) {
                return std::addressof(_Scary_val::extract(this->_node));
            }

            XSTL_NODISCARD friend constexpr bool operator==(const _Self& lhs,
                                                            const _Self& rhs) noexcept(noexcept(rhs._node == lhs._node))
            // clang-format off
     #ifdef __cpp_lib_concepts
                requires requires {
                    { rhs._node == lhs._node } -> std::convertible_to<bool>;
                }
     #endif
            // clang-format on
            {
                return rhs._node == lhs._node;
            }

#ifdef __cpp_lib_three_way_comparison

            XSTL_NODISCARD friend constexpr std::compare_three_way_result_t<_Nodeptr>
            operator<=>(const _Self& lhs, const _Self& rhs) noexcept(noexcept(lhs._node <=> rhs._node)) {
                return lhs._node <=> rhs._node;
            }
#else

            XSTL_NODISCARD friend constexpr bool
            operator!=(const _Self& lhs, const _Self& rhs) noexcept(noexcept(std::declval<_Self&>() == std::declval<_Self&>())) {
                return !(lhs == rhs);
            }

            XSTL_NODISCARD friend constexpr bool operator<(const _Self& lhs,
                                                           const _Self& rhs) noexcept(noexcept(lhs._node < rhs._node))
            // clang-format off
     #ifdef __cpp_lib_concepts
                requires requires {
                    { lhs._node < rhs._node } -> std::convertible_to<bool>;
                }
     #endif
            // clang-format on
            {
                return lhs._node < rhs._node;
            }

            XSTL_NODISCARD friend constexpr bool operator>=(const _Self& lhs, const _Self& rhs) noexcept(noexcept(lhs < rhs)) {
                return !(lhs < rhs);
            }

            XSTL_NODISCARD friend constexpr bool operator>(const _Self& lhs, const _Self& rhs) noexcept(noexcept(rhs < lhs)) {
                return rhs < lhs;
            }

            XSTL_NODISCARD friend constexpr bool operator<=(const _Self& lhs, const _Self& rhs) noexcept(noexcept(rhs < lhs)) {
                return !(rhs < lhs);
            }
#endif

            constexpr _Self& operator++() noexcept(is_nothrow_unchecked_pre_increasable_v) {
                if constexpr (std::is_convertible_v<_Cat, std::random_access_iterator_tag>)
                    _Scary_val::fwd(this->_node, difference_type{ 1 });
                else
                    _Scary_val::incr(this->_node);
                return *this;
            }

            constexpr _Self operator++(int) noexcept(is_nothrow_unchecked_post_increasable_v) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() noexcept(is_nothrow_unchecked_pre_decreasable_v) {
                if constexpr (std::is_convertible_v<_Cat, std::random_access_iterator_tag>)
                    _Scary_val::fwd(this->_node, difference_type{ -1 });
                else
                    _Scary_val::decr(this->_node);
                return *this;
            }

            constexpr _Self operator--(int) noexcept(is_nothrow_unchecked_post_decreasable_v) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }

            constexpr _Self&
            operator+=(const difference_type off) noexcept(noexcept(_Scary_val::fwd(std::declval<_Nodeptr&>(), off))) {
                _Scary_val::fwd(this->_node, off);
                return *this;
            }

            XSTL_NODISCARD constexpr _Self operator+(const difference_type off) const
                noexcept(std::is_nothrow_copy_constructible_v<_Self>&& noexcept(std::declval<_Self&>() += off)) {
                return _Self(*this) += off;
            }

            XSTL_NODISCARD friend constexpr _Self operator+(const difference_type off,
                                                            const _Self&          rhs) noexcept(noexcept(rhs + off)) {
                return rhs + off;
            }

            constexpr _Self& operator-=(const difference_type off) noexcept(noexcept(std::declval<_Self&>() += -off)) {
                return *this += -off;
            }

            XSTL_NODISCARD constexpr _Self operator-(const difference_type off) const
                noexcept(std::is_nothrow_copy_constructible_v<_Self>&& noexcept(std::declval<_Self&>() -= off)) {
                return _Self(*this) -= off;
            }

            XSTL_NODISCARD constexpr difference_type operator-(const _Self& iter) const
                noexcept(noexcept(this->_node - iter._node)) {
                return this->_node - iter._node;
            }

            XSTL_NODISCARD constexpr reference operator[](const difference_type off) const
                noexcept(noexcept(*(std::declval<_Self&>() + off))) {
                return *(*this + off);
            }
        };

        template <class _Base>
        class iterator_impl : public _Base {
            using _Self      = iterator_impl<_Base>;
            using _Scary_val = typename _Base::_ScaryVal;
            using _Nodeptr   = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_pointer_t<_Scary_val>;
            using reference         = get_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = typename _Base::iterator_category;

            constexpr iterator_impl() = default;
            constexpr iterator_impl(_Nodeptr                  node,
                                    const container_val_base* cont) noexcept(std::is_nothrow_copy_constructible_v<_Base>)
                : _Base(node, cont) {}

            constexpr decltype(auto) operator*() const noexcept(*std::declval<_Base&>()) { return (reference)_Base::operator*(); }
            constexpr decltype(auto) operator->() const noexcept(*std::declval<_Base&>()) {
                return (get_deref_t<pointer, _Scary_val>)_Base::operator->();
            }

            constexpr _Self& operator++() noexcept(++std::declval<_Base&>()) {
                _Base::operator++();
                return *this;
            }
            constexpr _Self operator++(int) noexcept(std::declval<_Base&>()++) {
                _Self  _tmp = *this;
                _Base::operator++();
                return _tmp;
            }

            constexpr _Self& operator--() noexcept(--std::declval<_Base&>()) {
                _Base::operator--();
                return *this;
            }

            constexpr _Self operator--(int) noexcept(std::declval<_Base&>()--) {
                _Self  _tmp = *this;
                _Base::operator--();
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) noexcept(noexcept(std::declval<_Base&>() += off)) {
                _Base::operator+=(off);
                return *this;
            }

            XSTL_NODISCARD constexpr _Self operator+(const difference_type off) const
                noexcept(std::is_nothrow_copy_constructible_v<_Self>&& noexcept(std::declval<_Self&>() += off)) {
                return _Self(*this) += off;
            }

            XSTL_NODISCARD constexpr friend _Self operator+(const difference_type off,
                                                            const _Self&          rhs) noexcept(noexcept(rhs + off)) {
                return rhs + off;
            }

            constexpr _Self& operator-=(const difference_type off) noexcept(noexcept(std::declval<_Self&>() += -off)) {
                return *this += -off;
            }

            XSTL_NODISCARD constexpr _Self operator-(const difference_type off) const
                noexcept(std::is_nothrow_copy_constructible_v<_Self>&& noexcept(std::declval<_Self&>() -= off)) {
                return _Self(*this) -= off;
            }

            XSTL_NODISCARD constexpr reference operator[](const difference_type off) const
                noexcept(noexcept(*(std::declval<_Self&>() + off))) {
                return *(*this + off);
            }
        };

        template <class _Scary_val, class _Cat>
        class xstl_const_iterator : public unchecked_const_iterator<_Scary_val, _Cat, default_iter_base<_Scary_val>> {
            using _Self    = xstl_const_iterator<_Scary_val, _Cat>;
            using _Base    = unchecked_const_iterator<_Scary_val, _Cat, default_iter_base<_Scary_val>>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_const_pointer_t<_Scary_val>;
            using reference         = get_const_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = _Cat;

            // protected:
            using _ScaryVal = _Scary_val;

            // noexcept(range_verify)
            template <class _Cate, bool = std::is_convertible_v<_Cate, std::random_access_iterator_tag>>
            struct is_nothrow_range_verify : std::true_type {};
            template <class _Cate>
            struct is_nothrow_range_verify<_Cate, true>
                : std::bool_constant<noexcept(_Scary_val::range_verify(CAST2SCARY(nullptr), _Nodeptr{}, difference_type{ 1 }))> {
            };
            static constexpr bool is_nothrow_range_verify_v = is_nothrow_range_verify<_Cat>::value;

            // noexcept(++*this) && noexcept(range_verify OR incresable)
            template <class _Cate, auto = combined_flag_v<std::is_convertible<_Cate, std::random_access_iterator_tag>,
                                                          std::is_convertible<_Cate, std::forward_iterator_tag>>>
            struct is_nothrow_pre_increasable : std::true_type {};
            template <class _Cate>
            struct is_nothrow_pre_increasable<_Cate, 0b11>
                : std::conjunction<is_nothrow_range_verify<_Cate>,
                                   typename _Base::template is_nothrow_unchecked_pre_increasable<_Cate>> {};
            template <class _Cate>
            struct is_nothrow_pre_increasable<_Cate, 0b01>
                : std::bool_constant<_Base::is_nothrow_unchecked_pre_increasable_v
                                     && noexcept(_Scary_val::increasable(CAST2SCARY(nullptr), _Nodeptr{}))> {};
            static constexpr bool is_nothrow_pre_increasable_v = is_nothrow_pre_increasable<_Cat>::value;

            // noexcept(--*this) && noexcept(range_verify OR decresable)
            template <class _Cate, auto = combined_flag_v<std::is_convertible<_Cate, std::random_access_iterator_tag>,
                                                          std::is_convertible<_Cate, std::bidirectional_iterator_tag>>>
            struct is_nothrow_pre_decreasable : std::true_type {};
            template <class _Cate>
            struct is_nothrow_pre_decreasable<_Cate, 0b11>
                : std::conjunction<is_nothrow_range_verify<_Cate>,
                                   typename _Base::template is_nothrow_unchecked_pre_decreasable<_Cate>> {};
            template <class _Cate>
            struct is_nothrow_pre_decreasable<_Cate, 0b01>
                : std::bool_constant<_Base::is_nothrow_unchecked_pre_decreasable_v
                                     && noexcept(_Scary_val::decreasable(CAST2SCARY(nullptr), _Nodeptr{}))> {};
            static constexpr bool is_nothrow_pre_decreasable_v = is_nothrow_pre_decreasable<_Cat>::value;

            // noexcept(*this++) && noexcept(range_verify OR decresable)
            static constexpr bool is_nothrow_post_increasable_v =
                is_nothrow_pre_increasable_v /*&& std::is_nothrow_copy_constructible_v<_Self>*/;

            // noexcept(*this--) && noexcept(range_verify OR decresable)
            static constexpr bool is_nothrow_post_decreasable_v =
                is_nothrow_pre_decreasable_v /*&& std::is_nothrow_copy_constructible_v<_Self>*/;

            static constexpr bool is_nothrow_dereferable_v =
                _Base::is_nothrow_unchecked_dereferable_v&& noexcept(_Scary_val::dereferable(CAST2SCARY(nullptr), _Nodeptr{}));

        public:
            constexpr xstl_const_iterator() = default;
            constexpr xstl_const_iterator(_Nodeptr                  node,
                                          const container_val_base* cont) noexcept(std::is_nothrow_copy_constructible_v<_Nodeptr>)
                : _Base(node, cont) {}

            constexpr typename _Base::reference operator*() const noexcept(is_nothrow_dereferable_v) {
                XSTL_EXPECT(this->_Get_cont(), "iterators haven't binded to any containers");
                XSTL_EXPECT(_Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node), "iterators cannot dereference");
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<typename _Base::pointer, _Scary_val> operator->() const noexcept(is_nothrow_dereferable_v) {
                XSTL_EXPECT(this->_Get_cont(), "iterators haven't binded to any containers");
                XSTL_EXPECT(_Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node), "iterators cannot dereference");
                return std::addressof(_Scary_val::extract(this->_node));
            }

            XSTL_NODISCARD friend constexpr bool operator==(const _Self& lhs,
                                                            const _Self& rhs) noexcept(noexcept(rhs._node == lhs._node)) {
                XSTL_EXPECT(lhs._Get_cont() == rhs._Get_cont(), "iterators incompatible");
                return rhs._node == lhs._node;
            }
#ifdef __cpp_lib_three_way_comparison

            XSTL_NODISCARD friend constexpr std::compare_three_way_result_t<_Nodeptr>
            operator<=>(const _Self& lhs, const _Self& rhs) noexcept(noexcept(lhs._node <=> rhs._node)) {
                XSTL_EXPECT(lhs._Get_cont() == rhs._Get_cont(), "iterators incompatible");
                return lhs._node <=> rhs._node;
            }
#else
            XSTL_NODISCARD friend constexpr bool operator!=(const _Self& lhs, const _Self& rhs) noexcept(noexcept(lhs == rhs)) {
                return !(lhs == rhs);
            }

            XSTL_NODISCARD friend constexpr bool operator<(const _Self& lhs, const _Self& rhs) noexcept(noexcept(lhs < rhs)) {
                XSTL_EXPECT(lhs._Get_cont() == rhs._Get_cont(), "iterators incompatible");
                return lhs._node < rhs._node;
            }

            XSTL_NODISCARD friend constexpr bool operator>=(const _Self& lhs, const _Self& rhs) noexcept(noexcept(lhs < rhs)) {
                return !(lhs < rhs);
            }

            XSTL_NODISCARD friend constexpr bool operator>(const _Self& lhs, const _Self& rhs) noexcept(noexcept(rhs < lhs)) {
                return rhs < lhs;
            }

            XSTL_NODISCARD friend constexpr bool operator<=(const _Self& lhs, const _Self& rhs) noexcept(noexcept(rhs < lhs)) {
                return !(rhs < lhs);
            }
#endif

            constexpr _Self& operator++() noexcept(is_nothrow_pre_increasable_v) {
                XSTL_EXPECT(this->_Get_cont(), "iterators haven't binded to any containers");
                if constexpr (std::is_convertible_v<_Cat, std::random_access_iterator_tag>)
                    XSTL_EXPECT(_Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, difference_type{ 1 }),
                                "iterators cannot increment");
                else
                    XSTL_EXPECT(_Scary_val::increasable(CAST2SCARY(this->_Get_cont()), this->_node),
                                "iterators cannot increment");
                _Base::operator++();
                return *this;
            }

            constexpr _Self operator++(int) noexcept(is_nothrow_post_increasable_v) {
                _Self  _tmp = *this;
                _Base::operator++();
                return _tmp;
            }

            constexpr _Self& operator--() noexcept(is_nothrow_pre_decreasable_v) {
                XSTL_EXPECT(this->_Get_cont(), "iterators haven't binded to any containers");
                if constexpr (std::is_convertible_v<_Cat, std::random_access_iterator_tag>)
                    XSTL_EXPECT(_Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, difference_type{ -1 }),
                                "iterators cannot decrement");
                else
                    XSTL_EXPECT(_Scary_val::decreasable(CAST2SCARY(this->_Get_cont()), this->_node),
                                "iterators cannot decrement");
                _Base::operator--();
                return *this;
            }

            constexpr _Self operator--(int) noexcept(is_nothrow_post_decreasable_v) {
                _Self  _tmp = *this;
                _Base::operator--();
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) noexcept(noexcept(std::declval<_Base&>() += off)
                                                                            && is_nothrow_range_verify_v) {
                XSTL_EXPECT(this->_Get_cont(), "iterators haven't binded to any containers");
                XSTL_EXPECT(_Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, off),
                            "iterators cannot move backwards");

                _Base::operator+=(off);
                return *this;
            }

            XSTL_NODISCARD constexpr _Self operator+(const difference_type off) const
                noexcept(std::is_nothrow_copy_constructible_v<_Self>&& noexcept(std::declval<_Self&>() += off)) {
                return _Self(*this) += off;
            }

            XSTL_NODISCARD constexpr friend _Self operator+(const difference_type off,
                                                            const _Self&          rhs) noexcept(noexcept(rhs + off)) {
                return rhs + off;
            }

            constexpr _Self& operator-=(const difference_type off) noexcept(noexcept(std::declval<_Self&>() += off)
                                                                            && is_nothrow_range_verify_v) {
                XSTL_EXPECT(this->_Get_cont(), "iterators haven't binded to any containers");
                XSTL_EXPECT(_Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, -off),
                            "iterators cannot move forwards");

                _Base::operator-=(off);
                return *this;
            }

            XSTL_NODISCARD constexpr _Self operator-(const difference_type off) const noexcept(noexcept(_Self(*this) -= off)) {
                return _Self(*this) -= off;
            }

            XSTL_NODISCARD constexpr difference_type operator-(const _Self& iter) const
                noexcept(noexcept(this->_node - iter._node)) {
                XSTL_EXPECT(this->_Get_cont() == iter._Get_cont(), "iterators are not compatible");
                return this->_node - iter._node;
            }

            XSTL_NODISCARD constexpr reference operator[](const difference_type off) const
                noexcept(noexcept(*(std::declval<_Self&>() + off))) {
                return *(*this + off);
            }
        };

        template <class _Scary_val, class _Cat>
        using unchecked_iterator = iterator_impl<unchecked_const_iterator<_Scary_val, _Cat>>;
        template <class _Scary_val, class _Cat>
        using xstl_iterator = iterator_impl<xstl_const_iterator<_Scary_val, _Cat>>;

        // forward iterators
        template <class _Scary_val>
        using unchecked_fwd_citer = unchecked_const_iterator<_Scary_val, std::forward_iterator_tag>;
        template <class _Scary_val>
        using unchecked_fwd_iter = unchecked_iterator<_Scary_val, std::forward_iterator_tag>;
        template <class _Scary_val>
        using fwd_citer = xstl_const_iterator<_Scary_val, std::forward_iterator_tag>;
        template <class _Scary_val>
        using fwd_iter = xstl_iterator<_Scary_val, std::forward_iterator_tag>;
        template <class _Scary_val>

        // bidirectional iterators
        template <class _Scary_val>
        using unchecked_bid_citer = unchecked_const_iterator<_Scary_val, std::bidirectional_iterator_tag>;
        template <class _Scary_val>
        using unchecked_bid_iter = unchecked_iterator<_Scary_val, std::bidirectional_iterator_tag>;
        template <class _Scary_val>
        using bid_citer = xstl_const_iterator<_Scary_val, std::bidirectional_iterator_tag>;
        template <class _Scary_val>
        using bid_iter = xstl_iterator<_Scary_val, std::bidirectional_iterator_tag>;

        // random access iterators
        template <class _Scary_val>
        using unchecked_rand_citer = unchecked_const_iterator<_Scary_val, std::random_access_iterator_tag>;
        template <class _Scary_val>
        using unchecked_rand_iter = unchecked_iterator<_Scary_val, std::random_access_iterator_tag>;
        template <class _Scary_val>
        using rand_citer = xstl_const_iterator<_Scary_val, std::random_access_iterator_tag>;
        template <class _Scary_val>
        using rand_iter = xstl_iterator<_Scary_val, std::random_access_iterator_tag>;

        /**
         *	@class reverse_iterator
         *	@brief an xstl_iterator adaptor that reverses the direction of a given xstl_iterator.
         *	@attention It's designed for containers whose rbegin() is undecrable, which is different from
         std::reverse_iterator
         */
        template <class _Iter>
        class reverse_iterator {
            template <class _Ty>
            _Ty _Fake_copy_init(_Ty) noexcept {}  // silence warning

            template <class _From, class _To, bool = std::is_convertible_v<_From, _To>, bool = std::is_void_v<_To>>
            struct _Is_nothrow_convertible : std::bool_constant<noexcept(_Fake_copy_init<_To>(std::declval<_From>()))> {};
            template <class _From, class _To, bool _IsVoid>
            struct _Is_nothrow_convertible<_From, _To, false, _IsVoid> : std::false_type {};
            template <class _From, class _To>
            struct _Is_nothrow_convertible<_From, _To, true, true> : std::true_type {};

            template <class _It, class _Pointer, bool = std::is_pointer_v<std::remove_cv_t<std::remove_reference_t<_It>>>>
            struct _Has_nothrow_operator_arrow : _Is_nothrow_convertible<_It, _Pointer> {};

            template <class _It, class _Pointer>
            struct _Has_nothrow_operator_arrow<_It, _Pointer, false>
                : std::bool_constant<noexcept(_Fake_copy_init<_Pointer>(std::declval<_It>().operator->()))> {};

        public:
            using iterator_category = typename std::iterator_traits<_Iter>::iterator_category;
            using value_type        = typename std::iterator_traits<_Iter>::value_type;
            using difference_type   = typename std::iterator_traits<_Iter>::difference_type;
            using pointer           = typename std::iterator_traits<_Iter>::pointer;
            using reference         = typename std::iterator_traits<_Iter>::reference;

            constexpr reverse_iterator() = default;
            constexpr explicit reverse_iterator(_Iter rhs) noexcept(std::is_nothrow_move_constructible_v<_Iter>)
                : _iter(std::move(rhs)) {}
            template <class _Other>
#ifdef __cpp_lib_concepts
                requires(!std::is_same_v<_Other, _Iter>) && std::convertible_to<const _Other&, _Iter>
#endif
            constexpr reverse_iterator(const reverse_iterator<_Other>& rhs) noexcept(
                std::is_nothrow_constructible_v<_Iter, const _Other&>)
                : _iter(rhs._iter) {
            }

            template <class _Other>
#ifdef __cpp_lib_concepts
                requires(!std::is_same_v<_Other, _Iter>)
                        && std::is_convertible_v<const _Other&, _Iter> && std::is_assignable_v<_Iter&, const _Other&>
#endif
            constexpr reverse_iterator&
            operator=(const reverse_iterator<_Other>& rhs) noexcept(std::is_nothrow_assignable_v<_Iter&, const _Other&>) {
                _iter = rhs._iter;
                return *this;
            }

            XSTL_NODISCARD constexpr _Iter base() const noexcept(std::is_nothrow_copy_constructible_v<_Iter>) { return _iter; }

            XSTL_NODISCARD constexpr reference operator*() const noexcept(noexcept(*(std::declval<_Iter&>()))) { return *_iter; }

            XSTL_NODISCARD constexpr auto operator->() const
                noexcept(std::is_nothrow_copy_constructible_v<_Iter>&& _Has_nothrow_operator_arrow<_Iter&, pointer>::value)
#ifdef __cpp_lib_concepts
                requires(std::is_pointer_v<_Iter> || requires(const _Iter i) { i.operator->(); })
#endif
            {
                if constexpr (std::is_pointer_v<_Iter>)
                    return _iter;
                else
                    return _iter.operator->();
            }

            constexpr reverse_iterator& operator++() noexcept(--_iter) {
                --_iter;
                return *this;
            }

            constexpr reverse_iterator operator++(int) noexcept(noexcept(--_iter)
                                                                && std::is_nothrow_copy_constructible_v<_Iter>) {
                reverse_iterator _tmp = *this;
                --_iter;
                return _tmp;
            }

            constexpr reverse_iterator& operator--() noexcept(++_iter) {
                ++_iter;
                return *this;
            }

            constexpr reverse_iterator operator--(int) noexcept(noexcept(++_iter)
                                                                && std::is_nothrow_copy_constructible_v<_Iter>) {
                reverse_iterator _tmp = *this;
                ++_iter;
                return _tmp;
            }

            XSTL_NODISCARD constexpr reverse_iterator operator+(const difference_type off) const
                noexcept(noexcept(reverse_iterator(_iter - off))) {
                return reverse_iterator(_iter - off);
            }

            XSTL_NODISCARD constexpr friend reverse_iterator
            operator+(const difference_type off, const reverse_iterator& rhs) noexcept(noexcept(rhs + off)) {
                return rhs + off;
            }

            constexpr reverse_iterator& operator+=(const difference_type off) noexcept(noexcept(_iter -= off)) {
                _iter -= off;
                return *this;
            }

            XSTL_NODISCARD constexpr reverse_iterator operator-(const difference_type off) const
                noexcept(noexcept(reverse_iterator(_iter + off))) {
                return reverse_iterator(_iter + off);
            }

            constexpr reverse_iterator& operator-=(const difference_type off) noexcept(noexcept(_iter += off)) {
                _iter += off;
                return *this;
            }

            XSTL_NODISCARD constexpr reference operator[](const difference_type off) const
                noexcept(noexcept(_Fake_copy_init<reference>(_iter[off]))) {
                return _iter[static_cast<difference_type>(-off - 1)];
            }

            template <class _Iter1, class _Iter2>
            XSTL_NODISCARD friend constexpr bool
            operator==(const reverse_iterator<_Iter1>& lhs,
                       const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() == rhs.base())))
            // clang-format off
     #ifdef __cpp_lib_concepts
                requires requires {
                    { lhs.base() == rhs.base() } -> std::convertible_to<bool>;
                }
     #endif
            // clang-format on
            {
                return lhs.base() == rhs.base();
            }

            template <class _Iter1, class _Iter2>
            XSTL_NODISCARD friend constexpr bool
            operator!=(const reverse_iterator<_Iter1>& lhs,
                       const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() != rhs.base())))
            // clang-format off
     #ifdef __cpp_lib_concepts
                requires requires {
                    { lhs.base() != rhs.base() } -> std::convertible_to<bool>;
                }
     #endif
            // clang-format on
            {
                return lhs.base() != rhs.base();
            }

            template <class _Iter1, class _Iter2>
            XSTL_NODISCARD friend constexpr bool
            operator<(const reverse_iterator<_Iter1>& lhs,
                      const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() > rhs.base())))
            // clang-format off
     #ifdef __cpp_lib_concepts
                requires requires {
                    { lhs.base() > rhs.base() } -> std::convertible_to<bool>;
                }
     #endif
            // clang-format on
            {
                return lhs.base() > rhs.base();
            }

            template <class _Iter1, class _Iter2>
            XSTL_NODISCARD friend constexpr bool
            operator>(const reverse_iterator<_Iter1>& lhs,
                      const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() < rhs.base())))
            // clang-format off
     #ifdef __cpp_lib_concepts
                requires requires {
                    { lhs.base() < rhs.base() } -> std::convertible_to<bool>;
                }
     #endif
            // clang-format on
            {
                return lhs.base() < rhs.base();
            }

            template <class _Iter1, class _Iter2>
            XSTL_NODISCARD friend constexpr bool
            operator<=(const reverse_iterator<_Iter1>& lhs,
                       const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() >= rhs.base())))
            // clang-format off
     #ifdef __cpp_lib_concepts
                requires requires {
                    { lhs.base() >= rhs.base() } -> std::convertible_to<bool>;
                }
     #endif
            // clang-format on
            {
                return lhs.base() >= rhs.base();
            }

            template <class _Iter1, class _Iter2>
            XSTL_NODISCARD friend constexpr bool
            operator>=(const reverse_iterator<_Iter1>& lhs,
                       const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() <= rhs.base())))
            // clang-format off
     #ifdef __cpp_lib_concepts
                requires requires {
                    { lhs.base() <= rhs.base() } -> std::convertible_to<bool>;
                }
     #endif
            // clang-format on
            {
                return lhs.base() <= rhs.base();
            }

        private:
            _Iter _iter{};
        };

#ifdef __cpp_lib_concepts
        template <class _Iter1, std::three_way_comparable_with<_Iter1> _Iter2>
        XSTL_NODISCARD constexpr std::compare_three_way_result_t<_Iter1, _Iter2>
        operator<=>(const reverse_iterator<_Iter1>& lhs,
                    const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(rhs.base() <=> lhs.base())) {
            return rhs.base() <=> lhs.base();
        }
#endif

        template <class _Iter1, class _Iter2>
        XSTL_NODISCARD constexpr auto operator-(const reverse_iterator<_Iter1>& lhs,
                                                const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(rhs.base() - lhs.base()))
            -> decltype(rhs.base() - lhs.base()) {
            return rhs.base() - lhs.base();
        }

        template <class _Iter>
        XSTL_NODISCARD constexpr reverse_iterator<_Iter>
        operator+(typename reverse_iterator<_Iter>::difference_type off,
                  const reverse_iterator<_Iter>&                    rhs) noexcept(noexcept(rhs + off)) {
            return rhs + off;
        }

        template <class _Iter>
        XSTL_NODISCARD constexpr reverse_iterator<_Iter>
        make_reverse_iterator(_Iter iter) noexcept(std::is_nothrow_move_constructible_v<_Iter>) {
            return reverse_iterator<_Iter>(std::move(iter));
        }
    }  // namespace iter_adapter
}  // namespace xstl
#endif
