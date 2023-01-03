#pragma once
#ifndef _XSTL_ITER_
#define _XSTL_ITER_
#include "config.hpp"

namespace xstl {
    namespace xstl_iterator {
        template <class _Value_type, class _Size_type, class _Difference_type, class _Pointer, class _Const_pointer,
                  class _Reference, class _Const_reference, class _Nodeptr_type>
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

        template <class _Iter>
        using iterator_category_t = typename std::iterator_traits<_Iter>::iterator_category;

        template <class _Iter, class _Cat, class = void>
        struct is_iterator : std::false_type {};

        template <class _Iter, class _Cat>
        struct is_iterator<_Iter, _Cat, std::void_t<iterator_category_t<_Iter>>>
            : std::bool_constant<std::is_convertible_v<iterator_category_t<_Iter>, _Cat>> {};

        template <class _Iter, class _Cat>
        inline constexpr bool is_iterator_v = is_iterator<_Iter, _Cat>::value;
        template <class _Iter>
        inline constexpr bool is_input_iterator_v = is_iterator_v<_Iter, std::input_iterator_tag>;

        template <class _Scary_val>
        struct iter_base {
            using _Nodeptr = typename _Scary_val::_Nodeptr;
            static_assert("node pointer should be default constructible", std::is_default_constructible_v<_Nodeptr>);

            constexpr iter_base() = default;
            constexpr iter_base(_Nodeptr node, const container_val_base* cont) noexcept : _node(node), _pcont(cont) {}

            [[nodiscard]] constexpr _Nodeptr        base() const noexcept { return _node; }
            [[nodiscard]] const container_val_base* _Get_cont() const noexcept { return _pcont; }

        protected:
            const container_val_base* _pcont = nullptr;
            _Nodeptr                  _node{};
        };

        template <class _Scary_val>
        struct unchecked_iter_base {
            using _Nodeptr = typename _Scary_val::_Nodeptr;
            static_assert("node pointer should be default constructible", std::is_default_constructible_v<_Nodeptr>);

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
            constexpr unchecked_fwd_citer(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr reference operator*() const noexcept(noexcept(_Scary_val::extract(_Nodeptr{}))) {
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept(noexcept(operator*())) {
                return std::addressof(_Scary_val::extract(this->_node));
            }
            [[nodiscard]] constexpr bool operator==(const _Self& rhs) const noexcept(noexcept(_Nodeptr{} == _Nodeptr{})) {
                return rhs._node == this->_node;
            }
            [[nodiscard]] constexpr bool operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) {
                return !(*this == rhs);
            }

            constexpr _Self& operator++() noexcept(noexcept(_Scary_val::incr(_Nodeptr{}))) {
                _Scary_val::incr(this->_node);
                return *this;
            }

            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
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
            constexpr unchecked_fwd_iter(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr reference operator*() const noexcept(noexcept(_Scary_val::extract(_Nodeptr{}))) {
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept(noexcept(operator*())) {
                return std::addressof(_Scary_val::extract(this->_node));
            }

            constexpr _Self& operator++() noexcept(noexcept(++static_cast<_Base&>(*this))) {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
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
            constexpr fwd_citer(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr typename _Base::reference operator*() const
                noexcept(noexcept(_Scary_val::extract(_Nodeptr{})) && noexcept(_Scary_val::dereferable(CAST2SCARY(nullptr),
                                                                                                       _Nodeptr{}))) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(
                    ("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));

                return _Scary_val::extract(this->_node);
            }

            constexpr get_deref_t<typename _Base::pointer, _Scary_val> operator->() const noexcept(noexcept(operator*())) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(
                    ("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));

                return std::addressof(_Scary_val::extract(this->_node));
            }

            [[nodiscard]] constexpr bool operator==(const _Self& rhs) const noexcept(noexcept(_Nodeptr{} == _Nodeptr{})) {
                XSTL_EXPECT(("iterators incompatible", this->_Get_cont() == rhs._Get_cont()));
                return rhs._node == this->_node;
            }
            [[nodiscard]] constexpr bool operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) {
                return !(*this == rhs);
            }

            constexpr _Self& operator++() noexcept(
                noexcept(++static_cast<_Base&>(*this)) && noexcept(_Scary_val::incrable(CAST2SCARY(nullptr), _Nodeptr{}))) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(("iterators cannot increase", _Scary_val::incrable(CAST2SCARY(this->_Get_cont()), this->_node)));

                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
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
            constexpr fwd_iter(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr auto operator*() const noexcept(noexcept(_Base::operator*())) -> reference {
                return *static_cast<const _Base&>(*this);
            }
            constexpr auto operator->() const noexcept(noexcept(_Base::operator->())) -> get_deref_t<pointer, _Scary_val> {
                return static_cast<const _Base&>(*this).operator->();
            }

            constexpr _Self& operator++() noexcept(noexcept(++static_cast<_Base&>(*this))) {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
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
            constexpr unchecked_bid_citer(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr reference operator*() const noexcept(noexcept(_Scary_val::extract(_Nodeptr{}))) {
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept(noexcept(operator*())) {
                return std::addressof(_Scary_val::extract(this->_node));
            }
            [[nodiscard]] constexpr bool operator==(const _Self& rhs) const noexcept(noexcept(_Nodeptr{} == _Nodeptr{})) {
                return rhs._node == this->_node;
            }
            [[nodiscard]] constexpr bool operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) {
                return !(*this == rhs);
            }

            constexpr _Self& operator++() noexcept(noexcept(_Scary_val::incr(_Nodeptr{}))) {
                _Scary_val::incr(this->_node);
                return *this;
            }

            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() noexcept(noexcept(_Scary_val::decr(_Nodeptr{}))) {
                _Scary_val::decr(this->_node);
                return *this;
            }

            constexpr _Self operator--(int) noexcept(noexcept(--*this)) {
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
            constexpr unchecked_bid_iter(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr reference operator*() const noexcept(noexcept(_Scary_val::extract(_Nodeptr{}))) {
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept(noexcept(operator*())) {
                return std::addressof(_Scary_val::extract(this->_node));
            }

            constexpr _Self& operator++() noexcept(noexcept(++static_cast<_Base&>(*this))) {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() noexcept(noexcept(--static_cast<_Base&>(*this))) {
                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) noexcept(noexcept(--*this)) {
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
            constexpr bid_citer(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr typename _Base::reference operator*() const
                noexcept(noexcept(_Scary_val::extract(_Nodeptr{})) && noexcept(_Scary_val::dereferable(CAST2SCARY(nullptr),
                                                                                                       _Nodeptr{}))) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(
                    ("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));

                return _Scary_val::extract(this->_node);
            }

            constexpr get_deref_t<typename _Base::pointer, _Scary_val> operator->() const noexcept(noexcept(operator*())) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(
                    ("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));

                return std::addressof(_Scary_val::extract(this->_node));
            }

            [[nodiscard]] constexpr bool operator==(const _Self& rhs) const noexcept(noexcept(_Nodeptr{} == _Nodeptr{})) {
                XSTL_EXPECT(("iterators incompatible", this->_Get_cont() == rhs._Get_cont()));
                return rhs._node == this->_node;
            }
            [[nodiscard]] constexpr bool operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) {
                return !(*this == rhs);
            }

            constexpr _Self& operator++() noexcept(
                noexcept(++static_cast<_Base&>(*this)) && noexcept(_Scary_val::incrable(CAST2SCARY(nullptr), _Nodeptr{}))) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(("iterators cannot increase", _Scary_val::incrable(CAST2SCARY(this->_Get_cont()), this->_node)));

                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() noexcept(
                noexcept(--static_cast<_Base&>(*this)) && noexcept(_Scary_val::decrable(CAST2SCARY(nullptr), _Nodeptr{}))) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(
                    ("iterators cannot decrease", _Scary_val::decrable(CAST2SCARY(this->_Get_cont())), this->_node)));

                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) noexcept(noexcept(--*this)) {
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
            bid_iter(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr auto operator*() const noexcept(noexcept(_Base::operator*())) -> reference {
                return *static_cast<const _Base&>(*this);
            }
            constexpr auto operator->() const noexcept(noexcept(_Base::operator->())) -> get_deref_t<pointer, _Scary_val> {
                return static_cast<const _Base&>(*this).operator->();
            }

            constexpr _Self& operator++() noexcept(noexcept(++static_cast<_Base&>(*this))) {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() noexcept(noexcept(--static_cast<_Base&>(*this))) {
                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) noexcept(noexcept(--*this)) {
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
            constexpr unchecked_rand_citer(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr reference operator*() const noexcept(noexcept(_Scary_val::extract(_Nodeptr{}))) {
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<pointer, _Scary_val> operator->() const noexcept(noexcept(operator*())) {
                return std::addressof(_Scary_val::extract(this->_node));
            }

            [[nodiscard]] constexpr bool operator==(const _Self& rhs) const noexcept(noexcept(_Nodeptr{} == _Nodeptr{})) {
                return rhs._node == this->_node;
            }
#ifdef __cpp_lib_three_way_comparison
            constexpr std::compare_three_way_result_t<_Nodeptr> operator<=>(const _Self& rhs) const
                noexcept(noexcept(this->_node <=> rhs._node)) {
                return this->_node <=> rhs._node;
            }
#else

            [[nodiscard]] constexpr bool operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) {
                return !(*this == rhs);
            }

            [[nodiscard]] constexpr bool operator<(const _Self& rhs) const noexcept(noexcept(this->_node < rhs._node)) {
                return this->_node < rhs._node;
            }

            [[nodiscard]] constexpr bool operator>=(const _Self& rhs) const noexcept(noexcept(*this < rhs)) {
                return !(*this < rhs);
            }

            [[nodiscard]] constexpr bool operator>(const _Self& rhs) const noexcept(noexcept(*this < rhs)) { return rhs < *this; }

            [[nodiscard]] constexpr bool operator<=(const _Self& rhs) const noexcept(noexcept(*this < rhs)) {
                return !(rhs < *this);
            }
#endif

            constexpr _Self& operator++() noexcept(noexcept(_Scary_val::fwd(_Nodeptr{}, difference_type{ 1 }))) {
                _Scary_val::fwd(this->_node, difference_type{ 1 });
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }
            constexpr _Self& operator--() noexcept(noexcept(_Scary_val::fwd(_Nodeptr{}, difference_type{ -1 }))) {
                _Scary_val::fwd(this->_node, difference_type{ -1 });
                return *this;
            }
            constexpr _Self operator--(int) noexcept(noexcept(--*this)) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) noexcept(noexcept(_Scary_val::fwd(_Nodeptr{}, off))) {
                _Scary_val::fwd(this->_node, off);
                return *this;
            }

            [[nodiscard]] constexpr _Self operator+(const difference_type off) const noexcept(noexcept(_Self(*this) += off)) {
                return _Self(*this) += off;
            }

            [[nodiscard]] constexpr friend _Self operator+(const difference_type off,
                                                           const _Self&          rhs) noexcept(noexcept(rhs + off)) {
                return rhs + off;
            }

            constexpr _Self& operator-=(const difference_type off) noexcept(noexcept(*this += -off)) { return *this += -off; }

            [[nodiscard]] constexpr _Self operator-(const difference_type off) const noexcept(noexcept(_Self(*this) -= off)) {
                return _Self(*this) -= off;
            }

            [[nodiscard]] constexpr difference_type operator-(const _Self& iter) const
                noexcept(noexcept(this->_node - iter._node)) {
                return this->_node - iter._node;
            }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const noexcept(noexcept(*(*this + off))) {
                return *(*this + off);
            }
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
            constexpr unchecked_rand_iter(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr auto operator*() const noexcept(noexcept(_Base::operator*())) -> reference {
                return *static_cast<const _Base&>(*this);
            }
            constexpr auto operator->() const noexcept(noexcept(_Base::operator->())) -> get_deref_t<pointer, _Scary_val> {
                return static_cast<const _Base&>(*this).operator->();
            }

            constexpr _Self& operator++() noexcept(noexcept(++static_cast<_Base&>(*this))) {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() noexcept(noexcept(--static_cast<_Base&>(*this))) {
                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) noexcept(noexcept(--*this)) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) noexcept(noexcept(static_cast<_Base&>(*this) += off)) {
                static_cast<_Base&>(*this) += off;
                return *this;
            }

            [[nodiscard]] constexpr _Self operator+(const difference_type off) const noexcept(noexcept(_Self(*this) += off)) {
                return _Self(*this) += off;
            }

            [[nodiscard]] constexpr friend _Self operator+(const difference_type off,
                                                           const _Self&          rhs) noexcept(noexcept(rhs + off)) {
                return rhs + off;
            }

            constexpr _Self& operator-=(const difference_type off) noexcept(noexcept(*this += -off)) { return *this += -off; }

            [[nodiscard]] constexpr _Self operator-(const difference_type off) const noexcept(noexcept(_Self(*this) -= off)) {
                return _Self(*this) -= off;
            }

            [[nodiscard]] constexpr difference_type operator-(const _Self& iter) const
                noexcept(noexcept(this->_node - iter._node)) {
                return this->_node - iter._node;
            }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const noexcept(noexcept(*(*this + off))) {
                return *(*this + off);
            }
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
            constexpr rand_citer(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr typename _Base::reference operator*() const
                noexcept(noexcept(_Scary_val::extract(_Nodeptr{})) && noexcept(_Scary_val::dereferable(CAST2SCARY(nullptr),
                                                                                                       _Nodeptr{}))) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(
                    ("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));
                return _Scary_val::extract(this->_node);
            }
            constexpr get_deref_t<typename _Base::pointer, _Scary_val> operator->() const noexcept() {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(
                    ("iterators cannot dereference", _Scary_val::dereferable(CAST2SCARY(this->_Get_cont()), this->_node)));
                return std::addressof(_Scary_val::extract(this->_node));
            }

            [[nodiscard]] constexpr bool operator==(const _Self& rhs) const noexcept(noexcept(_Nodeptr{} == _Nodeptr{})) {
                XSTL_EXPECT(("iterators incompatible", this->_Get_cont() == rhs._Get_cont()));
                return rhs._node == this->_node;
            }
#ifdef __cpp_lib_three_way_comparison
            [[nodiscard]] constexpr std::compare_three_way_result_t<_Nodeptr> operator<=>(const _Self& rhs) const
                noexcept(noexcept(rhs._node <=> this->_node)) {
                XSTL_EXPECT(("iterators incompatible", this->_Get_cont() == rhs._Get_cont()));
                return rhs._node <=> this->_node;
            }
#else
            [[nodiscard]] constexpr bool operator!=(const _Self& rhs) const noexcept(noexcept(*this == rhs)) {
                return !(*this == rhs);
            }

            [[nodiscard]] constexpr bool operator<(const _Self& rhs) const noexcept(noexcept(_Nodeptr{} < _Nodeptr{})) {
                XSTL_EXPECT(("iterators incompatible", this->_Get_cont() == rhs._Get_cont()));
                return this->_node < rhs._node;
            }

            [[nodiscard]] constexpr bool operator>=(const _Self& rhs) const noexcept(noexcept(*this < rhs)) {
                return !(*this < rhs);
            }

            [[nodiscard]] constexpr bool operator>(const _Self& rhs) const noexcept(noexcept(*this < rhs)) { return rhs < *this; }

            [[nodiscard]] constexpr bool operator<=(const _Self& rhs) const noexcept(noexcept(*this < rhs)) {
                return !(rhs < *this);
            }
#endif

            constexpr _Self& operator++() noexcept(noexcept(++static_cast<_Base&>(*this)) && noexcept(
                _Scary_val::range_verify(CAST2SCARY(nullptr), _Nodeptr{}, difference_type{ 1 }))) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(("iterators cannot increase",
                             _Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, difference_type{ 1 })));
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }

            constexpr _Self& operator--() noexcept(noexcept(++static_cast<_Base&>(*this)) && noexcept(
                _Scary_val::range_verify(CAST2SCARY(nullptr), _Nodeptr{}, difference_type{ -1 }))) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(
                    ("iterators cannot decrease", _Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, -1)));
                --static_cast<_Base&>(*this);
                return *this;
            }

            constexpr _Self operator--(int) noexcept(noexcept(--*this)) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) noexcept(noexcept(static_cast<_Base&>(*this) += off)) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(("iterators cannot move backwards",
                             _Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, off)));

                static_cast<_Base&>(*this) += off;
                return *this;
            }

            [[nodiscard]] constexpr _Self operator+(const difference_type off) const noexcept(noexcept(_Self(*this) += off)) {
                return _Self(*this) += off;
            }

            [[nodiscard]] constexpr friend _Self operator+(const difference_type off,
                                                           const _Self&          rhs) noexcept(noexcept(rhs + off)) {
                return rhs + off;
            }

            constexpr _Self& operator-=(const difference_type off) noexcept(noexcept(static_cast<_Base&>(*this) += off)) {
                XSTL_EXPECT(("iterators haven't binded to any containers", this->_Get_cont()));
                XSTL_EXPECT(("iterators cannot move forwards",
                             _Scary_val::range_verify(CAST2SCARY(this->_Get_cont()), this->_node, -off)));

                static_cast<_Base&>(*this) -= off;
                return *this;
            }

            [[nodiscard]] constexpr _Self operator-(const difference_type off) const noexcept(noexcept(_Self(*this) -= off)) {
                return _Self(*this) -= off;
            }

            [[nodiscard]] constexpr difference_type operator-(const _Self& iter) const
                noexcept(noexcept(this->_node - iter._node)) {
                XSTL_EXPECT(("iterators are not compatible", this->_Get_cont() == iter._Get_cont()));
                return this->_node - iter._node;
            }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const noexcept(noexcept(*(*this + off))) {
                return *(*this + off);
            }
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
            constexpr rand_iter(_Nodeptr node, const container_val_base* cont) noexcept : _Base(node, cont) {}

            constexpr auto operator*() const noexcept(noexcept(*static_cast<const _Base&>(*this))) -> reference {
                return *static_cast<const _Base&>(*this);
            }
            constexpr auto operator->() const noexcept(noexcept(static_cast<const _Base&>(*this).operator->()))
                -> get_deref_t<pointer, _Scary_val> {
                return static_cast<const _Base&>(*this).operator->();
            }

            constexpr _Self& operator++() noexcept(noexcept(++static_cast<_Base&>(*this))) {
                ++static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator++(int) noexcept(noexcept(++*this)) {
                _Self _tmp = *this;
                ++*this;
                return _tmp;
            }
            constexpr _Self& operator--() noexcept(noexcept(--static_cast<_Base&>(*this))) {
                --static_cast<_Base&>(*this);
                return *this;
            }
            constexpr _Self operator--(int) noexcept(noexcept(--*this)) {
                _Self _tmp = *this;
                --*this;
                return _tmp;
            }

            constexpr _Self& operator+=(const difference_type off) noexcept(noexcept(static_cast<_Base&>(*this) += off)) {
                static_cast<_Base&>(*this) += off;
                return *this;
            }

            [[nodiscard]] constexpr _Self operator+(const difference_type off) const noexcept(noexcept(_Self(*this) += off)) {
                return _Self(*this) += off;
            }

            [[nodiscard]] constexpr friend _Self operator+(const difference_type off,
                                                           const _Self&          rhs) noexcept(noexcept(rhs + off)) {
                return rhs + off;
            }

            constexpr _Self& operator-=(const difference_type off) noexcept(noexcept(*this += -off)) { return *this += -off; }

            [[nodiscard]] constexpr _Self operator-(const difference_type off) const noexcept(noexcept(_Self(*this) -= off)) {
                return _Self(*this) -= off;
            }

            [[nodiscard]] constexpr difference_type operator-(const _Self& iter) const
                noexcept(noexcept(static_cast<const _Base&>(*this) - static_cast<const _Base&>(iter))) {
                return static_cast<const _Base&>(*this) - static_cast<const _Base&>(iter);
            }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const noexcept(noexcept(*(*this + off))) {
                return *(*this + off);
            }
        };

        /**
         *	@class reverse_iterator
         *	@brief an iterator adaptor that reverses the direction of a given iterator.
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

            [[nodiscard]] constexpr _Iter base() const noexcept(std::is_nothrow_copy_constructible_v<_Iter>) { return _iter; }

            [[nodiscard]] constexpr reference operator*() const
                noexcept(std::is_nothrow_copy_constructible_v<_Iter>&& noexcept(*(std::declval<_Iter&>()))) {
                return *_iter;
            }

            [[nodiscard]] constexpr auto operator->() const
                noexcept(std::is_nothrow_copy_constructible_v<_Iter>&& noexcept(std::declval<_Iter&>().operator->())
                         && _Has_nothrow_operator_arrow<_Iter&, pointer>::value)
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

            constexpr reverse_iterator operator++(int) noexcept(noexcept(++*this)) {
                reverse_iterator _tmp = *this;
                --_iter;
                return _tmp;
            }

            constexpr reverse_iterator& operator--() noexcept(++_iter) {
                ++_iter;
                return *this;
            }

            constexpr reverse_iterator operator--(int) noexcept(noexcept(--*this)) {
                reverse_iterator _tmp = *this;
                ++_iter;
                return _tmp;
            }

            [[nodiscard]] constexpr reverse_iterator operator+(const difference_type off) const
                noexcept(noexcept(reverse_iterator(_iter - off))) {
                return reverse_iterator(_iter - off);
            }

            [[nodiscard]] constexpr friend reverse_iterator operator+(const difference_type   off,
                                                                      const reverse_iterator& rhs) noexcept(noexcept(rhs + off)) {
                return rhs + off;
            }

            constexpr reverse_iterator& operator+=(const difference_type off) noexcept(noexcept(_iter -= off)) {
                _iter -= off;
                return *this;
            }

            [[nodiscard]] constexpr reverse_iterator operator-(const difference_type off) const
                noexcept(noexcept(reverse_iterator(_iter + off))) {
                return reverse_iterator(_iter + off);
            }

            constexpr reverse_iterator& operator-=(const difference_type off) noexcept(noexcept(_iter += off)) {
                _iter += off;
                return *this;
            }

            [[nodiscard]] constexpr reference operator[](const difference_type off) const
                noexcept(noexcept(_iter[static_cast<difference_type>(-off - 1)])) {
                return _iter[static_cast<difference_type>(-off - 1)];
            }

            template <class _Iter1, class _Iter2>
            [[nodiscard]] friend constexpr bool
            operator==(const reverse_iterator<_Iter1>& lhs,
                       const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() == rhs.base())))
#ifdef __cpp_lib_concepts
                requires requires {
                             { lhs.base() == rhs.base() } -> std::convertible_to<bool>;
                         }
#endif
            {
                return lhs.base() == rhs.base();
            }

            template <class _Iter1, class _Iter2>
            [[nodiscard]] friend constexpr bool
            operator!=(const reverse_iterator<_Iter1>& lhs,
                       const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() != rhs.base())))
#ifdef __cpp_lib_concepts
                requires requires {
                             { lhs.base() != rhs.base() } -> std::convertible_to<bool>;
                         }
#endif
            {
                return lhs.base() != rhs.base();
            }

            template <class _Iter1, class _Iter2>
            [[nodiscard]] friend constexpr bool
            operator<(const reverse_iterator<_Iter1>& lhs,
                      const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() > rhs.base())))
#ifdef __cpp_lib_concepts
                requires requires {
                             { lhs.base() > rhs.base() } -> std::convertible_to<bool>;
                         }
#endif
            {
                return lhs.base() > rhs.base();
            }

            template <class _Iter1, class _Iter2>
            [[nodiscard]] friend constexpr bool
            operator>(const reverse_iterator<_Iter1>& lhs,
                      const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() < rhs.base())))
#ifdef __cpp_lib_concepts
                requires requires {
                             { lhs.base() < rhs.base() } -> std::convertible_to<bool>;
                         }
#endif
            {
                return lhs.base() < rhs.base();
            }

            template <class _Iter1, class _Iter2>
            [[nodiscard]] friend constexpr bool
            operator<=(const reverse_iterator<_Iter1>& lhs,
                       const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() >= rhs.base())))
#ifdef __cpp_lib_concepts
                requires requires {
                             { lhs.base() >= rhs.base() } -> std::convertible_to<bool>;
                         }
#endif
            {
                return lhs.base() >= rhs.base();
            }

            template <class _Iter1, class _Iter2>
            [[nodiscard]] friend constexpr bool
            operator>=(const reverse_iterator<_Iter1>& lhs,
                       const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(_Fake_copy_init<bool>(lhs.base() <= rhs.base())))
#ifdef __cpp_lib_concepts
                requires requires {
                             { lhs.base() <= rhs.base() } -> std::convertible_to<bool>;
                         }
#endif
            {
                return lhs.base() <= rhs.base();
            }

        private:
            _Iter _iter{};
        };

#ifdef __cpp_lib_concepts
        template <class _Iter1, std::three_way_comparable_with<_Iter1> _Iter2>
        [[nodiscard]] constexpr std::compare_three_way_result_t<_Iter1, _Iter2>
        operator<=>(const reverse_iterator<_Iter1>& lhs,
                    const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(rhs.base() <=> lhs.base())) {
            return rhs.base() <=> lhs.base();
        }
#endif

        template <class _Iter1, class _Iter2>
        [[nodiscard]] constexpr auto operator-(const reverse_iterator<_Iter1>& lhs,
                                               const reverse_iterator<_Iter2>& rhs) noexcept(noexcept(rhs.base() - lhs.base()))
            -> decltype(rhs.base() - lhs.base()) {
            return rhs.base() - lhs.base();
        }

        template <class _Iter>
        [[nodiscard]] constexpr reverse_iterator<_Iter>
        operator+(typename reverse_iterator<_Iter>::difference_type off,
                  const reverse_iterator<_Iter>&                    rhs) noexcept(noexcept(rhs + off)) {
            return rhs + off;
        }

        template <class _Iter>
        [[nodiscard]] constexpr reverse_iterator<_Iter>
        make_reverse_iterator(_Iter _Iter) noexcept(std::is_nothrow_move_constructible_v<_Iter>) {
            return reverse_iterator<_Iter>(std::move(_Iter));
        }
    }  // namespace xstl_iterator
}  // namespace xstl
#endif  // !_XSTL_ITER_
