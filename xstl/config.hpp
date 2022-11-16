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
#include <compare>
#define CAST2SCARY(CONT) static_cast<const _Scary_val*>(CONT)

namespace xstl {

    template <class _Node, class _Rlsr>
    struct exception_guard {
        exception_guard(_Node* ptr, _Rlsr releaser) : _node(ptr), _releaser(releaser) {}
        _Node* release() { return std::exchange(_node, nullptr); }
        ~exception_guard() {
            if (_node)
                (*_releaser)(_node);
        }

    private:
        _Node* _node;
        _Rlsr  _releaser;
    };

    template <class _Node, class _Rlsr>
    exception_guard(_Node*, _Rlsr) -> exception_guard<_Node, _Rlsr>;

    template <class _Keycmp, class _Lhs, class _Rhs = _Lhs>
    inline constexpr bool is_nothrow_comparable_v = noexcept(static_cast<bool>(std::declval<const _Keycmp&>()(std::declval<const _Lhs&>(), std::declval<const _Rhs&>())));

    struct container_val_base {  // for SCARY machinery
    };

    namespace xstl_iterator {
        template <class _Value_type, class _Size_type, class _Difference_type, class _Pointer, class _Const_pointer, class _Reference, class _Const_reference, class _Nodeptr_type>
        struct scary_iter_types {  // for SCARY machinery
            using value_type      = _Value_type;
            using size_type       = _Size_type;
            using difference_type = _Difference_type;
            using pointer         = _Pointer;
            using const_pointer   = _Const_pointer;
            using reference       = _Reference;
            using const_reference = _Const_reference;

            using _Nodeptr = _Nodeptr_type;
        };

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

        template <class _Scary_val>
        struct iter_base {
            using _Nodeptr = typename _Scary_val::_Nodeptr;
            constexpr iter_base() = default;
            constexpr iter_base(_Nodeptr node, const container_val_base* cont) : _node(node), _pcont(cont) {}

            [[nodiscard]] constexpr _Nodeptr        base() const noexcept { return _node; }
            [[nodiscard]] const container_val_base* _Get_cont() const noexcept { return _pcont; }

        protected:
            const container_val_base* _pcont = nullptr;
            _Nodeptr                  _node{};
        };

        template <class _Scary_val>
        struct unchecked_iter_base {
            using _Nodeptr = typename _Scary_val::_Nodeptr;
            constexpr unchecked_iter_base(_Nodeptr node, const container_val_base*) : _node(node) {}

            [[nodiscard]] constexpr _Nodeptr        base() const noexcept { return _node; }
            [[nodiscard]] const container_val_base* _Get_cont() const noexcept { return nullptr; }

        protected:
            _Nodeptr _node{};
        };

#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
        template <class _Scary_val>
        using default_iter_base = iter_base<_Scary_val>;
#else
        template <class _Pack>
        using default_iter_base = unchecked_iter_base<_Scary_val>;
#endif

        template <class _Scary_val, class _Base = unchecked_iter_base<_Scary_val>>
        class unchecked_fwd_citer : public _Base {
            using _Self    = unchecked_fwd_citer<_Scary_val, _Base>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_const_pointer_t<_Scary_val>;
            using reference         = get_const_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = std::forward_iterator_tag;

            constexpr unchecked_fwd_citer() = default;
            constexpr unchecked_fwd_citer(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr reference                        operator*() const noexcept { return _Scary_val::extract(this->_node); }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept { return std::addressof(_Scary_val::extract(this->_node)); }
            [[nodiscard]] constexpr bool               operator==(const _Self& rhs) const noexcept(noexcept(std::declval<_Nodeptr>() == std::declval<_Nodeptr>())) { return rhs._node == this->_node; }
            [[nodiscard]] constexpr bool               operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) { return !(*this == rhs); }

            constexpr _Self& operator++() {
                _Scary_val::incr(this->_node);
                return *this;
            }

            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }
        };

        template <class _Scary_val>
        class unchecked_fwd_iter : public unchecked_fwd_citer<_Scary_val> {
            using _Self    = unchecked_fwd_iter<_Scary_val>;
            using _Base    = unchecked_fwd_citer<_Scary_val>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_pointer_t<_Scary_val>;
            using reference         = get_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = std::forward_iterator_tag;

            constexpr unchecked_fwd_iter() = default;
            constexpr unchecked_fwd_iter(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr reference                        operator*() const noexcept { return _Scary_val::extract(this->_node); }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept { return std::addressof(_Scary_val::extract(this->_node)); }

            constexpr _Self& operator++() {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }
        };

        template <class _Scary_val>
        class fwd_citer : public unchecked_fwd_citer<_Scary_val, default_iter_base<_Scary_val>> {
            using _Self    = fwd_citer<_Scary_val>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;
            using _Base    = unchecked_fwd_citer<_Scary_val, default_iter_base<_Scary_val>>;

        public:
            constexpr fwd_citer() = default;
            constexpr fwd_citer(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr typename _Base::reference operator*() const noexcept {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));
#endif
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<typename _Base::pointer, _Scary_val> operator->() const noexcept {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));
#endif
                return std::addressof(_Scary_val::extract(this->_node));
            }

            [[nodiscard]] constexpr bool operator==(const _Self& rhs) const noexcept(noexcept(std::declval<_Nodeptr>() == std::declval<_Nodeptr>())) {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators incompatible", this->_Get_cont() == rhs._Get_cont()));
#endif
                return rhs._node == this->_node;
            }
            [[nodiscard]] constexpr bool operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) { return !(*this == rhs); }

            constexpr _Self& operator++() {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot increase", _Scary_val::incrable(CAST2SCARY(this->_Get_cont()), this->_node)));
#endif
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }
        };

        template <class _Scary_val>
        class fwd_iter : public fwd_citer<_Scary_val> {
            using _Self    = fwd_iter<_Scary_val>;
            using _Base    = fwd_citer<_Scary_val>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_pointer_t<_Scary_val>;
            using reference         = get_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = std::forward_iterator_tag;

            constexpr fwd_iter() = default;
            constexpr fwd_iter(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr decltype(auto) operator*() const noexcept { return (reference) * static_cast<const _Base&>(*this); }
            constexpr decltype(auto) operator->() const noexcept { return (get_deref_t<pointer, _Scary_val>)static_cast<const _Base&>(*this).operator->(); }

            constexpr _Self& operator++() {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }
        };

        template <class _Scary_val, class _Base = unchecked_iter_base<_Scary_val>>
        class unchecked_bid_citer : public _Base {
            using _Self    = unchecked_bid_citer<_Scary_val, _Base>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_const_pointer_t<_Scary_val>;
            using reference         = get_const_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = std::bidirectional_iterator_tag;

            constexpr unchecked_bid_citer() = default;
            constexpr unchecked_bid_citer(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr reference                        operator*() const noexcept { return _Scary_val::extract(this->_node); }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept { return std::addressof(_Scary_val::extract(this->_node)); }
            [[nodiscard]] constexpr bool               operator==(const _Self& rhs) const noexcept(noexcept(std::declval<_Nodeptr>() == std::declval<_Nodeptr>())) { return rhs._node == this->_node; }
            [[nodiscard]] constexpr bool               operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) { return !(*this == rhs); }

            constexpr _Self& operator++() {
                _Scary_val::incr(this->_node);
                return *this;
            }

            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() {
                _Scary_val::decr(this->_node);
                return *this;
            }

            constexpr _Self operator--(int) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }
        };

        template <class _Scary_val>
        class unchecked_bid_iter : public unchecked_bid_citer<_Scary_val> {
            using _Self    = unchecked_bid_iter<_Scary_val>;
            using _Base    = unchecked_bid_citer<_Scary_val>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_pointer_t<_Scary_val>;
            using reference         = get_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = std::bidirectional_iterator_tag;

            constexpr unchecked_bid_iter() = default;
            constexpr unchecked_bid_iter(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr reference                        operator*() const noexcept { return _Scary_val::extract(this->_node); }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept { return std::addressof(_Scary_val::extract(this->_node)); }

            constexpr _Self& operator++() {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() {
                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }
        };

        template <class _Scary_val>
        class bid_citer : public unchecked_bid_citer<_Scary_val, default_iter_base<_Scary_val>> {
            using _Self    = bid_citer<_Scary_val>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;
            using _Base    = unchecked_bid_citer<_Scary_val, default_iter_base<_Scary_val>>;

        public:
            constexpr bid_citer() = default;
            constexpr bid_citer(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr typename _Base::reference operator*() const noexcept {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));
#endif
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<typename _Base::pointer, _Scary_val> operator->() const noexcept {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));
#endif
                return std::addressof(_Scary_val::extract(this->_node));
            }

            [[nodiscard]] constexpr bool operator==(const _Self& rhs) const noexcept(noexcept(std::declval<_Nodeptr>() == std::declval<_Nodeptr>())) {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators incompatible", this->_Get_cont() == rhs._Get_cont()));
#endif
                return rhs._node == this->_node;
            }
            [[nodiscard]] constexpr bool operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) { return !(*this == rhs); }

            constexpr _Self& operator++() {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot increase", _Scary_val::incrable(CAST2SCARY(this->_Get_cont()), this->_node)));
#endif
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot decrease", _Scary_val::decrable(CAST2SCARY(CAST2SCARY(this->_Get_cont())), this->_node)));
#endif
                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }
        };

        template <class _Scary_val>
        class bid_iter : public bid_citer<_Scary_val> {
            using _Self    = bid_iter<_Scary_val>;
            using _Base    = bid_citer<_Scary_val>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_pointer_t<_Scary_val>;
            using reference         = get_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = std::forward_iterator_tag;

            bid_iter() = default;
            bid_iter(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr decltype(auto) operator*() const noexcept { return (reference) * static_cast<const _Base&>(*this); }
            constexpr decltype(auto) operator->() const noexcept { return (get_deref_t<pointer, _Scary_val>)static_cast<const _Base&>(*this).operator->(); }

            constexpr _Self& operator++() {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() {
                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }
        };

        // random access const_iterator
        template <class _Scary_val, class _Base = unchecked_iter_base<_Scary_val>>
        class unchecked_rand_citer : public _Base {
            using _Self    = unchecked_rand_citer<_Scary_val, _Base>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_const_pointer_t<_Scary_val>;
            using reference         = get_const_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = std::random_access_iterator_tag;

            constexpr unchecked_rand_citer() = default;
            constexpr unchecked_rand_citer(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr reference                        operator*() const noexcept { return _Scary_val::extract(this->_node); }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept { return std::addressof(_Scary_val::extract(this->_node)); }

            [[nodiscard]] constexpr bool   operator==(const _Self& rhs) const noexcept(noexcept(std::declval<_Nodeptr>() == std::declval<_Nodeptr>())) { return rhs._node == this->_node; }
            constexpr std::strong_ordering operator<=>(const _Self& rhs) const { return this->_node <=> rhs._node; }

            constexpr _Self& operator++() {
                _Scary_val::fwd(this->_node, 1);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }
            constexpr _Self& operator--() {
                _Scary_val::fwd(this->_node, -1);
                return *this;
            }
            constexpr _Self operator--(int) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) {
                _Scary_val::fwd(this->_node, off);
                return *this;
            }

            constexpr _Self operator+(const difference_type off) const { return _Self(*this) += off; }

            [[nodiscard]] constexpr friend _Self operator+(const difference_type off, const _Self& rhs) { return rhs + off; }

            constexpr _Self& operator-=(const difference_type off) { return *this += -off; }

            [[nodiscard]] constexpr _Self operator-(const difference_type off) const { return _Self(*this) -= off; }

            [[nodiscard]] constexpr difference_type operator-(const _Self& iter) const { return this->_node - iter._node; }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const { return *(*this + off); }
        };

        template <class _Scary_val>
        class unchecked_rand_iter : public unchecked_rand_citer<_Scary_val> {
            using _Base    = unchecked_rand_citer<_Scary_val>;
            using _Self    = unchecked_rand_iter<_Scary_val>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_pointer_t<_Scary_val>;
            using reference         = get_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = std::random_access_iterator_tag;

            constexpr unchecked_rand_iter() = default;
            constexpr unchecked_rand_iter(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr decltype(auto) operator*() const noexcept { return (reference) * static_cast<const _Base&>(*this); }
            constexpr decltype(auto) operator->() const noexcept { return (get_deref_t<pointer, _Scary_val>)static_cast<const _Base&>(*this).operator->(); }

            constexpr _Self& operator++() {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() {
                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) {
                static_cast<_Base&>(*this) += off;
                return *this;
            }

            constexpr _Self operator+(const difference_type off) const { return _Self(*this) += off; }

            [[nodiscard]] constexpr friend _Self operator+(const difference_type off, const _Self& rhs) { return rhs + off; }

            constexpr _Self& operator-=(const difference_type off) { return *this += -off; }

            [[nodiscard]] constexpr _Self operator-(const difference_type off) const { return _Self(*this) -= off; }

            [[nodiscard]] constexpr difference_type operator-(const _Self& iter) const { return this->_node - iter._node; }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const { return *(*this + off); }
        };

        template <class _Scary_val>
        class rand_citer : public unchecked_rand_citer<_Scary_val, default_iter_base<_Scary_val>> {
            using _Self    = rand_citer<_Scary_val>;
            using _Base    = unchecked_rand_citer<_Scary_val, default_iter_base<_Scary_val>>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using typename _Base::difference_type;
            using typename _Base::reference;

            constexpr rand_citer() = default;
            constexpr rand_citer(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr typename _Base::reference operator*() const noexcept {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(CAST2SCARY(this->_Get_cont())), this->_node)));
#endif
                return _Scary_val::extract(this->_node);
            }
            constexpr auto operator->() const noexcept {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(CAST2SCARY(this->_Get_cont())), this->_node)));
#endif
                return (get_deref_t<typename _Base::pointer, _Scary_val>)std::addressof(_Scary_val::extract(this->_node));
            }

            [[nodiscard]] constexpr bool operator==(const _Self& rhs) const noexcept(noexcept(std::declval<_Nodeptr>() == std::declval<_Nodeptr>())) {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators incompatible", this->_Get_cont() == rhs._Get_cont()));
#endif
                return rhs._node == this->_node;
            }

            [[nodiscard]] constexpr bool operator<=>(const _Self& rhs) const noexcept(noexcept(std::declval<_Nodeptr>() <=> std::declval<_Nodeptr>())) {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators incompatible", this->_Get_cont() == rhs._Get_cont()));
#endif
                return rhs._node <=> this->_node;
            }

            constexpr _Self& operator++() {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot increase", _Scary_val::range_verify(CAST2SCARY(CAST2SCARY(this->_Get_cont())), this->_node, 1)));
#endif
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot decrease", _Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, -1)));
#endif
                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot move backwards", _Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, off)));
#endif
                static_cast<_Base&>(*this) += off;
                return *this;
            }

            constexpr _Self operator+(const difference_type off) const { return _Self(*this) += off; }

            [[nodiscard]] constexpr friend _Self operator+(const difference_type off, const _Self& rhs) { return rhs + off; }

            constexpr _Self& operator-=(const difference_type off) {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot move forwards", _Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, -off)));
#endif
                static_cast<_Base&>(*this) -= off;
                return *this;
            }

            [[nodiscard]] constexpr _Self operator-(const difference_type off) const { return _Self(*this) -= off; }

            [[nodiscard]] constexpr difference_type operator-(const _Self& iter) const {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators are not compatible", this->_Get_cont() == iter._Get_cont()));
#endif
                return this->_node - iter._node;
            }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const { return *(*this + off); }
        };

        template <class _Scary_val>
        class rand_iter : public rand_citer<_Scary_val> {
            using _Base    = rand_citer<_Scary_val>;
            using _Self    = rand_iter<_Scary_val>;
            using _Nodeptr = typename _Scary_val::_Nodeptr;

        public:
            using value_type        = typename _Scary_val::value_type;
            using pointer           = get_pointer_t<_Scary_val>;
            using reference         = get_reference_t<_Scary_val>;
            using difference_type   = get_difference_t<_Scary_val>;
            using iterator_category = std::random_access_iterator_tag;

            constexpr rand_iter() = default;
            constexpr rand_iter(_Nodeptr node, const container_val_base* cont) : _Base(node, cont) {}

            constexpr decltype(auto) operator*() const noexcept {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));
#endif
                return (reference) * static_cast<const _Base&>(*this);
            }
            constexpr decltype(auto) operator->() const noexcept {
#if !defined(_NO_XSTL_SAFETY_VERIFT_) || !defined(_NO_XSTL_ITER_SAFETY_VERIFT_)
                assert(("iterators haven't binded to any containers", this->_Get_cont()));
                assert(("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));
#endif
                return (get_deref_t<pointer, _Scary_val>)static_cast<const _Base&>(*this).operator->();
            }

            constexpr _Self& operator++() {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }
            constexpr _Self& operator--() {
                --static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator--(int) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) {
                static_cast<_Base&>(*this) += off;
                return *this;
            }

            constexpr _Self operator+(const difference_type off) const { return _Self(*this) += off; }

            [[nodiscard]] constexpr friend _Self operator+(const difference_type off, const _Self& rhs) { return rhs + off; }

            constexpr _Self& operator-=(const difference_type off) { return *this += -off; }

            [[nodiscard]] constexpr _Self operator-(const difference_type off) const { return _Self(*this) -= off; }

            [[nodiscard]] constexpr difference_type operator-(const _Self& iter) const { return static_cast<const _Base&>(*this) - static_cast<const _Base&>(iter); }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const { return *(*this + off); }
        };

        /**
         *	@class reverse_iterator
         *	@brief an iterator adaptor that reverses the direction of a given iterator.
         *	@attention It's designed for containers whose rbegin() is undecrable, which is different from std::reverse_iterator
         */
        template <class _Iter>
        class reverse_iterator {
        public:
            using iterator_category = typename std::iterator_traits<_Iter>::iterator_category;
            using value_type        = typename std::iterator_traits<_Iter>::value_type;
            using difference_type   = typename std::iterator_traits<_Iter>::difference_type;
            using pointer           = typename std::iterator_traits<_Iter>::pointer;
            using reference         = typename std::iterator_traits<_Iter>::reference;

            constexpr reverse_iterator() = default;
            constexpr explicit reverse_iterator(_Iter rhs) noexcept(std::is_nothrow_move_constructible_v<_Iter>) : _iter(std::move(rhs)) {}
            template <class _Other>
            requires(!std::is_same_v<_Other, _Iter>)
                && std::is_convertible_v<const _Other&, _Iter> constexpr reverse_iterator(const reverse_iterator<_Other>& rhs) noexcept(std::is_nothrow_constructible_v<_Iter, const _Other&>)
                : _iter(rhs._iter) {}

            template <class _Other>
            requires(!std::is_same_v<_Other, _Iter>)
                && std::is_convertible_v<const _Other&, _Iter>&& std::is_assignable_v<_Iter&, const _Other&> constexpr reverse_iterator& operator=(const reverse_iterator<_Other>& rhs) {
                _iter = rhs._iter;
                return *this;
            }

            [[nodiscard]] constexpr _Iter base() const { return _iter; }

            [[nodiscard]] constexpr reference operator*() const { return *_iter; }

            [[nodiscard]] constexpr auto operator->() const requires(std::is_pointer_v<_Iter> || requires(const _Iter i) { i.operator->(); }) {
                if constexpr (std::is_pointer_v<_Iter>)
                    return _iter;
                else
                    return _iter.operator->();
            }

            constexpr reverse_iterator& operator++() {
                --_iter;
                return *this;
            }

            constexpr reverse_iterator operator++(int) {
                reverse_iterator _tmp = *this;
                --_iter;
                return _tmp;
            }

            constexpr reverse_iterator& operator--() {
                ++_iter;
                return *this;
            }

            constexpr reverse_iterator operator--(int) {
                reverse_iterator _tmp = *this;
                ++_iter;
                return _tmp;
            }

            [[nodiscard]] constexpr reverse_iterator operator+(const difference_type off) const { return reverse_iterator(_iter - off); }

            [[nodiscard]] constexpr friend reverse_iterator operator+(const difference_type off, const reverse_iterator& rhs) { return rhs + off; }

            constexpr reverse_iterator& operator+=(const difference_type off) {
                _iter -= off;
                return *this;
            }

            [[nodiscard]] constexpr reverse_iterator operator-(const difference_type off) const { return reverse_iterator(_iter + off); }

            constexpr reverse_iterator& operator-=(const difference_type off) {
                _iter += off;
                return *this;
            }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const { return _iter[static_cast<difference_type>(-off - 1)]; }

        private:
            _Iter _iter{};
        };

        template <class _Iter1, class _Iter2>
        [[nodiscard]] constexpr bool operator==(const reverse_iterator<_Iter1>& lhs, const reverse_iterator<_Iter2>& rhs) {
            return lhs.base() == rhs.base();
        }

        template <class _Iter1, class _Iter2>
        [[nodiscard]] constexpr bool operator!=(const reverse_iterator<_Iter1>& lhs, const reverse_iterator<_Iter2>& rhs) {
            return !(lhs == rhs);
        }
    }  // namespace xstl_iterator

    // propagate on container swap
    template <class _Alloc>
    void alloc_pocs(_Alloc& left, _Alloc& right) noexcept(std::allocator_traits<_Alloc>::propagate_on_container_swap::value) {
        using std::swap;
        if constexpr (std::allocator_traits<_Alloc>::propagate_on_container_swap::value)
            swap(left, right);
        else
            assert(("containers incompatible for swap", left != right));
    }

    // propagate on container copy assignment
    template <class _Alloc>
    void alloc_pocca(_Alloc& left, const _Alloc& right) noexcept {
        if constexpr (std::allocator_traits<_Alloc>::propagate_on_container_copy_assignment::value)
            left = right;
    }

    // propagate on container move assignment
    template <class _Alloc>
    void alloc_pocma(_Alloc& left, _Alloc& right) noexcept {
        if constexpr (std::allocator_traits<_Alloc>::propagate_on_container_move_assignment::value)
            left = std::move(right);
    }
}  // namespace xstl
#endif