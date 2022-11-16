/*
 *	Copyright(c) 2022 Kamichanw.All rights reserved.
 *   @file slist.hpp
 *   @brief The singly linked list library contains 2 types of singly linked lists:
 *	1. slist (singly linked list)
 *	2. slist (loop singly linked list)
 *   @author Kami-chan e-mail: 865710157@qq.com
 */

#ifndef _SLIST_HPP_
#define _SLIST_HPP_

#include "compressed_tuple.hpp"
#include "config.hpp"
#include <algorithm>
#undef CAST
#undef VALUE
#define CAST(NODE) static_cast<_Slist_node<value_type>*>(NODE)
#define VALUE(NODE) CAST(NODE)->_value

namespace xstl {
    namespace {
        /**
         *	@class _Slist_node_base
         *   @brief the node base of slist.
         */
        struct _Slist_node_base {
            _Slist_node_base(_Slist_node_base* next = nullptr) : _next(next) {}
            _Slist_node_base* _next;
        };

        /**
         *	@class _Slist_node
         *   @brief the node of slist.
         */
        template <class _Tp>
        struct _Slist_node : _Slist_node_base {
            using _Self      = _Slist_node_base;
            using value_type = _Tp;

            _Slist_node() = default;
            explicit _Slist_node(const _Tp& value, _Self* next = nullptr) : _value(value), _Self(_next) {}

            _Tp _value;
        };

        template <class _Val_types>
        struct _Scary_slist : public container_val_base {
            using _Self           = _Scary_slist<_Val_types>;
            using _Nodeptr        = typename _Val_types::_Nodeptr;
            using value_type      = typename _Val_types::value_type;
            using size_type       = typename _Val_types::size_type;
            using difference_type = typename _Val_types::difference_type;
            using pointer         = typename _Val_types::pointer;
            using const_pointer   = typename _Val_types::const_pointer;
            using reference       = value_type&;
            using const_reference = const value_type&;

            _Scary_slist() : _head(_head._next) {}

            static value_type& extract(_Slist_node_base* node) noexcept { return VALUE(node); }

            static bool dereferable(const _Self* slist, _Slist_node_base* node) noexcept { return node != std::addressof(slist->_head); }

            static bool incrable(const _Self* slist, _Slist_node_base* node) noexcept { return node->_next != std::addressof(slist->_head); }

            void swap(_Self& x) {
                if (this != std::addressof(x))
                    std ::swap(_head._next, x._head._next);
            }

            _Slist_node_base _head;
        };
    }  // namespace

    /**
     *	@class _Slist_base
     *   @brief singly linked list interfaces.
     */
    template <class _Tp, class _Alloc>
    class slist {
        using _Self        = slist<_Tp, _Alloc>;
        using _Altp_traits = std::allocator_traits<_Alloc>;
        using link_type    = _Slist_node_base*;
        using node         = _Slist_node_base;

        struct _Nodes_creator {
            explicit _Nodes_creator(_Self& ref_list) : _ref_list(ref_list) {}

            template <class _Iter>
            void create_from_range(_Iter first, _Iter last) {
                _ref_list._Check_max_size(_ref_list._size);
                _first = _ref_list._Create_node(*first);
                for (_last = _first; ++first != last; _last = _last->_next) {
                    _ref_list._Check_max_size(_ref_list._size);
                    _last->_next = _ref_list._Create_node(*first);
                }
            }

            void create_n(typename _Altp_traits::size_type n, const _Tp& value) {
                _ref_list._Check_max_size(_ref_list._size + n);
                _first = _ref_list._Create_node(value);
                for (_last = _first; --n; _last = _last->_next)
                    _last->_next = _ref_list._Create_node(value);
            }

            ~_Nodes_creator() {
                if (_last == nullptr)
                    _ref_list._Destroy(_first, _last);
            }

            _Self&    _ref_list;
            link_type _first = nullptr;
            link_type _last  = nullptr;
        };

    public:
        using allocator_type  = typename _Self::allocator_type;
        using value_type      = _Tp;
        using size_type       = typename _Altp_traits::size_type;
        using reference       = _Tp&;
        using const_reference = const _Tp&;
        using difference_type = typename _Altp_traits::difference_type;
        using pointer         = typename _Altp_traits::pointer;
        using const_pointer   = typename _Altp_traits::const_pointer;
        using _Scary_val      = _Scary_slist<xstl_iterator::scary_iter_types<value_type, size_type, difference_type, pointer, const_pointer, reference, const_reference, link_type>>;
        using const_iterator  = xstl_iterator::unchecked_fwd_citer<_Scary_val>;
        using iterator        = xstl_iterator::unchecked_fwd_iter<_Scary_val>;

        /**
         *	@brief constructs an empty slist.
         *   @param alloc : allocator to use for all memory allocations of this slist
         */
        slist() = default;
        explicit slist(allocator_type& alloc) : _Self(alloc) {}
        /**
         *   @brief constructs the slist with the copy of the contents of other.
         *   @param x : other instance of slists(pass by lvalue).
         *   @param alloc : allocator to use for all memory allocations of this slist
         */
        slist(const slist& x) : _Self(x) {}
        slist(const slist& x, const allocator_type& alloc) : _Self(x, alloc) {}
        slist(slist&& x) noexcept : _Self(std::move(x)) {}
        slist(slist&& x, const allocator_type& alloc) noexcept(_Self::_Alnode_traits::is_always_equal::value) : _Self(std::move(x), alloc) {}
        /**
         *   @brief constructs the slist with the contents of the range [first, last)
         *   @param first : the beginning of range to insert
         *   @param last : the end of range to insert
         *   @param alloc : allocator to use for all memory allocations of this slist
         */
        template <std::input_iterator _Iter>
        slist(_Iter first, _Iter last, const allocator_type& alloc = allocator_type()) : _Self(first, last, alloc) {}
        /**
         *   @brief constructs the slist with n elements each with value value_type()
         *   @param n : number of elements to insert.
         *   @param alloc : allocator to use for all memory allocations of this slist
         */
        explicit slist(size_type n, const allocator_type& alloc = allocator_type()) : _Self(n, alloc) {}
        slist(size_type n, const_reference value, const allocator_type& alloc = allocator_type()) : _Self(n, value, alloc) {}
        /**
         *   @brief constructs the slist with the contents of the initializer list init.
         *   @param l : initializer list to insert the values from
         *   @param alloc : allocator to use for all memory allocations of this slist
         */
        slist(std::initializer_list<_Tp> l, const allocator_type& alloc = allocator_type()) : _Self(l.begin(), l.end(), alloc) {}

        ~slist() = default;

        slist& operator=(const slist& rhs) = default;
        slist& operator=(slist&& rhs) = default;

        explicit slist(allocator_type& alloc) : _Self(alloc), _head(_head._next), _tail(std::addressof(_head)) {}
        slist(const _Self& x) : _head(_head._next), _tail(std::addressof(_head)) {
            _Unchecked_insert(begin(), x.begin(), x.end());
            alloc_pocca(_Getal(), _Self::_Alnode_traits::select_on_container_copy_construction(x._Getal()));
        }
        slist(const _Self& x, const allocator_type& alloc) : _Self(alloc), _head(_head._next), _tail(std::addressof(_head)) { _Unchecked_insert(begin(), x.begin(), x.end()); }
        slist(_Self&& x) noexcept : _head(std::exchange(x._head._next, _head._next)), _tail(std::exchange(x._tail, std::addressof(x._head))), _size(std::exchange(x._size, size_type{ 0 })) {
            alloc_pocma(_Getal(), x._Getal());
        }
        slist(_Self&& x, const allocator_type& alloc) noexcept(_Self::_Alnode_traits::is_always_equal::value)
            : _Self(alloc), _head(std::exchange(x._head._next, _head._next)), _tail(std::exchange(x._tail, std::addressof(x._head))), _size(std::exchange(x._size, size_type{ 0 })) {}
        template <std::input_iterator _Iter>
        slist(_Iter first, _Iter last, const allocator_type& alloc = allocator_type()) : _Self(alloc), _head(_head._next), _tail(std::addressof(_head)) {
            _Unchecked_insert(cbegin(), first, last);
        }
        explicit slist(size_type n, const allocator_type& alloc = allocator_type()) : _Self(alloc), _head(_head._next), _tail(std::addressof(_head)) { _Unchecked_insert(begin(), n, _Tp{}); }
        slist(size_type n, const_reference value, const allocator_type& alloc = allocator_type()) : _Self(alloc), _head(_head._next), _tail(std::addressof(_head)) {
            _Unchecked_insert(begin(), n, value);
        }

        /**
         *   @return a reference to the first element in the slist.
         */
        [[nodiscard]] reference front() noexcept {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("front() called on empty list", _size >= 0));
#endif
            return VALUE(_head._next);
        }
        [[nodiscard]] const_reference front() const noexcept {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("front() called on empty list", _size >= 0));
#endif
            return VALUE(_head._next);
        }
        /**
         *   @return a reference to the last element in the slist.
         */
        [[nodiscard]] reference back() noexcept {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("back() called on empty list", _size >= 0));
#endif
            return VALUE(_tail);
        }
        [[nodiscard]] const_reference back() const noexcept {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("back() called on empty list", _size >= 0));
#endif
            return VALUE(_tail);
        }

        [[nodiscard]] iterator       begin() noexcept { return _Make_iter(_head._next); }
        [[nodiscard]] const_iterator begin() const noexcept { return _Make_const_iter(_head._next); }
        [[nodiscard]] const_iterator cbegin() const noexcept { return _Make_const_iter(_head._next); }
        [[nodiscard]] iterator       end() noexcept { return _Make_iter(_head._next); }
        [[nodiscard]] const_iterator end() const noexcept { return _Make_const_iter(const_cast<link_type>(_head._next)); }
        [[nodiscard]] const_iterator cend() const noexcept { return _Make_const_iter(const_cast<link_type>(_head._next)); }

        /**
         *   @return the allocator associated with the container.
         */
        [[nodiscard]] allocator_type get_allocator() const noexcept { return static_cast<_Alloc>(_Getal()); }

        /**
         *   @brief insert an item into the list at position.
         *   @param position : iterator before which the content will be inserted
         *   @param value : value of the element to insert.
         *   @return an iterator which points to new node
         */
        iterator insert(const_iterator position, const_reference value) {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list insert iterator outside range", this == position._Get_cont()));
#endif
            _Check_max_size(_size);
            link_type _node = _Create_node(value);
            return _Make_iter(_Make_link(position.base(), _node, _node));
        }
        iterator insert(const_iterator position, value_type&& value) {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list insert iterator outside range", this == position._Get_cont()));
#endif
            _Check_max_size(_size);
            link_type _node = _Create_node(std::move(value));
            return _Make_iter(_Make_link(position.base(), _node, _node));
        }
        /**
         *   @brief inserts elements from initializer list l before position.
         *   @param position : iterator before which the content will be inserted
         *   @param l : initializer list to insert the values from
         */
        void insert(const_iterator position, std::initializer_list<_Tp> l) { insert(position, l.begin(), l.end()); }
        /**
         *   @brief initializes the list with elements each with value
         *   @param n : number of elements to insert.
         *   @param value : element value to insert
         */
        void insert(const_iterator position, size_type n, const_reference value) {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list insert iterator outside range", this == position._Get_cont()));
#endif
            _Unchecked_insert(position, n, value);
        }
        /**
         *   @brief inserts elements from range [first, last) before pos.
         *   @param position : iterator before which the content will be inserted
         *   @param first : the beginning of range of elements to insert
         *	@param last : the end of range of elements to insert
         *   @return an iterator which points to the first new node
         */
        template <std::input_iterator _Iter>
        iterator insert(const_iterator position, _Iter first, _Iter last) {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list insert iterator outside range", this == position._Get_cont()));
#endif
            return _Make_iter(_Unchecked_insert(position, first, last));
        }

        /**
         *   @brief inserts a new element to the beginning of the slist.
         *   @param values : arguments to forward to the constructor of the element
         */
        template <class... _Args>
        void emplace_front(_Args&&... values) {
            _Check_max_size(_size);
            link_type _node = _Create_node(_head._next, std::forward<_Args>(values)...);
            if (_head._next == _head._next)
                _tail = _node;
            _head._next = _node;
        }
        /**
         *   @brief inserts a new element to the end of the slist.
         *   @param values : arguments to forward to the constructor of the element
         */
        template <class... _Args>
        void emplace_back(_Args&&... values) {
            _Check_max_size(_size);
            _tail = _tail->_next = _Create_node(_head._next, std::forward<_Args>(values)...);
        }
        /**
         *	@brief inserts a new element into the slist directly before position
         *	@param position : iterator before which the new element will be constructed
         *	@param values : arguments to forward to the constructor of the element
         */
        template <class... _Args>
        void emplace(const_iterator position, _Args&&... values) {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list insert iterator outside range", this == position._Get_cont()));
#endif
            if (position.base() == _head._next)
                emplace_front(std::forward<_Args>(values)...);
            else if (position.base() == std::addressof(_head))
                emplace_back(std::forward<_Args>(values)...);
            else {
                _Check_max_size(_size);
                link_type _node = _Create_node(_head._next, std::forward<_Args>(values)...);
                _Make_link(position.base(), _node, _node);
            }
        }

        /**
         *	@brief appends the given element value to the end of the slist.
         *	@param value : the value of the element to append
         */
        void push_front(const_reference value) { emplace_front(value); }
        void push_front(value_type&& value) { emplace_front(std::move(value)); }
        /**
         *	@brief prepends the given element value to the beginning of the slist.
         *	@param value : the value of the element to append
         */
        void push_back(const_reference value) { emplace_back(value); }
        void push_back(value_type&& value) { emplace_back(std::move(value)); }

        /**
         *	@brief removes the first element of the slist.
         */
        void pop_front() noexcept {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list empty before pop", _size != 0));
#endif
            _Unchecked_erase(_head._next);
        }
        /**
         *	@brief removes the last element of the slist.
         */
        void pop_back() noexcept {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list empty before pop", _size != 0));
#endif
            _Unchecked_erase(_tail);
        }

        /**
         *	@brief removes specified elements from the slist.
         *	@param position : iterator to the element to remove.
         *	@return iterator following the last removed element.
         */
        iterator erase(const_iterator position) noexcept {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list erase iterator outside range", this == position._Get_cont()));
#endif
            return _Make_iter(position.base() != _head._next ? _Unchecked_erase(position.base()) : _head._next);
        }
        /**
         *	@brief removes the elements in the range [first, last).
         *	@param first : the beginning of range to erase the elements from
         *	@param last : the end of range to erase the elements from
         * 	@return iterator following the last removed element.
         */
        iterator erase(const_iterator first, const_iterator last) noexcept {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list erase iterator outside range", this == first._Get_cont() && this == last._Get_cont()));
#endif
            return first.base() == _head._next || first == last ? _Make_iter(_head._next) : (_Destroy(first, _Break_link(first, last)), last);
        }

        /**
         *	@brief removes all consecutive duplicate elements from the slist.
         *	@param pred : binary predicate which returns ​true if the elements should be treated as equal.
         * 	@return the numbers of elements removed.
         */
        size_type unique() { return unique(std::equal_to<>()); }
        template <class _Pred>
        size_type unique(_Pred pred) {
            const size_type _oldsz = _size;
            if (_size > 1) {
                link_type *_prev = std::addressof(_head._next), _curr;
                while ((_curr = *_prev)->_next != _head._next)
                    if (pred(VALUE(_curr), VALUE(_curr->_next))) {
                        *_prev = _curr->_next;
                        _Destroy(_curr, *_prev);
                    }
                    else
                        _prev = std::addressof(_curr->_next);
            }
            return _oldsz - _size;
        }

        /**
         *	@brief removes all elements that are equal to value
         *	@param value : value of the elements to remove
         * 	@return the numbers of elements removed.
         */
        size_type remove(const_reference value) {
            return remove_if([&](const_reference _value) { return _value == value; });
        }
        /**
         *	@brief removes all elements for which predicate pred returns true
         *	@param pred : unary predicate which returns ​true if the element should be removed.
         * 	@return the numbers of elements removed.
         */
        template <class _Pred>
        size_type remove_if(_Pred pred) {
            const size_type _oldsz = _size;
            if (!empty()) {
                for (link_type _curr1 = std::addressof(_head); _curr1->_next != _head._next; _curr1 = _curr1->_next) {
                    link_type _curr2 = _curr1->_next;
                    for (; _curr2 != _head._next && pred(VALUE(_curr2)); _curr2 = _curr2->_next)
                        ;
                    if (_curr2 != _curr1->_next) {
                        _Destroy(_curr1->_next, _curr2);
                        _curr1->_next = _curr2;
                        if (_curr2 == _head._next)
                            _tail = _curr1;
                    }
                }
            }
            return _oldsz - _size;
        }
        /**
         *	@brief sorts the elements in ascending order. The first version uses operator< to compare the elements. The second version uses the given comparison function comp
         *	@param cmpr : comparison function object.
         */
        void sort() { sort(std::less<_Tp>()); }
        template <class _Pred>
        void sort(_Pred cmpr) {
            link_type _mid = _Sort2(std::addressof(_head), cmpr);
            for (size_type bound = 2; _mid->_next != _head._next && bound; bound <<= 1) {
                const link_type _last = _Sort(_mid, bound, cmpr);
                _mid                  = _Merge_same(std::addressof(_head), _mid, _last, cmpr);
            }
            _tail = _mid;
        }

        /**
         *	@brief transfers all elements from other into *this. The elements are inserted before the element pointed to by position. The _Self other becomes empty after the operation.
         *	@param position : element before which the content will be inserted
         *	@param x : another list to transfer the content from
         */
        void splice(const_iterator position, _Self& x) {
#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("list erase iterator outside range", this == position._Get_cont()));
#endif
            if constexpr (!_Self::_Alnode_traits::is_always_equal::value)
                assert(("list allocators incompatible for splice", _Getal() == x._Getal()));
            if (!x.empty() && this != std::addressof(x)) {
                link_type _prev;
                for (_prev = std::addressof(_head); _prev->_next != position.base(); _prev = _prev->_next)
                    ;
                _prev->_next   = std::exchange(x._head._next, x._head._next);
                x._tail->_next = position.base();
                if (position.base() == _head._next)
                    _tail = x._tail;
                x._tail = std::addressof(x._head);
                _size += std::exchange(x._size, size_type{ 0 });
            }
        }
        void splice(const_iterator position, _Self&& x) { splice(position, x); }
        /**
         *	@brief transfers the element pointed to by it from other into *this. The element is inserted before the element pointed to by position.
         *	@param position : element before which the content will be inserted
         *	@param x : another list to transfer the content from
         *	@param iter : the element to transferfrom other to *this
         */
        void splice(const_iterator position, _Self& x, const_iterator iter) { splice(position, x, iter, std::next(iter)); }
        void splice(const_iterator position, _Self&& x, const_iterator iter) { splice(position, x, iter, std::next(iter)); }
        /**
         *	@brief transfers the elements in the range [first, last) from other into *this. The elements are inserted before the element pointed to by position.
         *	@param position : element before which the content will be inserted
         *	@param x : another list to transfer the content from
         *	@param first : the beginning of range of elements to transferfrom other to *this
         *	@param last : the end of range of elements to transferfrom other to *this
         */
        void splice(const_iterator position, _Self&& x, const_iterator first, const_iterator last) {
            if constexpr (!_Self::_Alnode_traits::is_always_equal::value)
                assert(("list allocators incompatible for splice", _Getal() == x._Getal()));
            if (first != last && this != std::addressof(x)) {
                link_type _prev = std::addressof(x._head);
                for (; _prev->_next != first.base(); _prev = _prev->_next)
                    ;
                _prev->_next           = last.base();
                link_type _before_last = first.base();
                if (last == _head._next)
                    x._tail = _prev;
                size_type _count = 1;
                for (; _before_last->_next != last.base(); ++_count, _before_last = _before_last->_next)
                    ;
                _size += _count;
                x._size -= _count;
                _Make_link(position.base(), first.base(), _before_last);
            }
        }
        void splice(const_iterator position, _Self&& x, const_iterator first, const_iterator last) { splice(position, x, first, last); }

        /**
         *	@brief replaces the contents with count copies of value value
         *	@param n : the new size of the slist
         *	@param value : the value to initialize elements of the slist with
         */
        void assign(size_type n, const_reference value) {
            link_type _curr = std::addressof(_head), *_prev = std::addressof(_head._next);
            for (size_t i = 0; i < n; ++i, _prev = std::addressof(_curr->_next)) {
                if ((_curr = *_prev) == _head._next) {
                    _Unchecked_insert(cend(), n - i, value);
                    return;
                }
                VALUE(_curr) = value;
            }
            _tail = _curr;
            _Destroy(_curr->_next, _head._next);
            _curr->_next = _head._next;
        }
        /**
         *	@brief replaces the contents with copies of those in the range [first, last).
         *	@param first : the beginning of range to copy the elements from
         *	@param last : the end of range to copy the elements from
         */
        template <std::input_iterator _Iter>
        void assign(_Iter first, _Iter last) {
            if (first == last)
                clear();
            else {
                link_type _curr, *_prev = std::addressof(_head._next);
                do {
                    if ((_curr = *_prev) == _head._next) {
                        _Unchecked_insert(cend(), first, last);
                        return;
                    }
                    VALUE(_curr) = *first;
                    _prev        = std::addressof(_curr->_next);
                } while (++first != last);
                _tail = _curr;
                _Destroy(_curr->_next, _head._next);
                _curr->_next = _head._next;
            }
        }
        /**
         *	@brief replaces the contents with the elements from the initializer list l.
         *	@param l : initializer list to copy the values from
         */
        void assign(std::initializer_list<_Tp> l) { assign(l.begin(), l.end()); }

        /**
         *	@brief merges two sorted lists into one. the lists should be sorted into ascending order. this version uses operator< to compare the elements
         *	@param x : another list to merge
         */
        void merge(_Self& x) { merge(x, std::less<_Tp>()); }
        void merge(_Self&& x) { merge(x, std::less<_Tp>()); }
        /**
         *	@brief merges two sorted lists into one. The lists should be sorted into ascending order. this version uses the given comparison function comp.
         *	@param x : another list to merge
         *	@param pred : comparison function object
         */
        template <class _Pred>
        void merge(_Self& x, _Pred pred) {
            if (x.empty() || this == std::addressof(x))
                return;
            if constexpr (!_Self::_Alnode_traits::is_always_equal::value)
                assert(("list allocators incompatible for merge", _Getal() == x._Getal()));
            link_type _curr1    = std::addressof(_head), _curr2;
            auto      _transfer = [&]() {
                link_type _tmp = _curr1->_next;
                _curr1->_next  = x._head._next;
                x._head._next  = _curr2->_next;
                _curr2->_next  = _tmp;
            };
            while (_curr1->_next != _head._next && x._head._next != _head._next) {
                for (_curr2 = &x._head; _curr2->_next != _head._next && pred(VALUE(_curr2->_next), VALUE(_curr1->_next)); _curr2 = _curr2->_next)
                    ;
                if (_curr2 != &x._head)
                    _transfer();
                _curr1 = _curr1->_next;
            }
            if (x._head._next != _head._next) {
                for (_curr2 = &x._head; _curr2->_next != _head._next; _curr2 = _curr2->_next)
                    ;
                _transfer();
                _tail = x._tail;
            }
            x._head._next = _head._next;
            x._tail       = &x._head;
            _size += std::exchange(x._size, size_type{ 0 });
        }
        template <class _Pred>
        void merge(_Self&& x, _Pred pred) {
            merge(x, pred);
        }
        /**
         *	@brief exchanges the contents of the list with those of other.
         *	@param x : list to exchange the contents with
         */
        void swap(_Self& x) noexcept {
            if (this == std::addressof(x))
                return;
            using std::swap;
            swap(_head._next, x._head._next);
            swap(_tail, x._tail);
            swap(_size, x._size);
            alloc_pocs(_Getal(), x._Getal());
        }
        /**
         *	@brief reverses the order of the elements in the list.
         */
        void reverse() noexcept {
            if (_size <= 1)
                return;
            link_type _curr = _head._next, _prev = _head._next;
            _tail = _head._next;
            while (true) {
                const link_type _next = _curr->_next;
                _curr->_next          = _prev;
                if (_next == _head._next) {
                    _head._next = _curr;
                    return;
                }
                _prev = _curr;
                _curr = _next;
            }
        }

        /**
         *	@brief resizes the list to contain count elements.
         *	@param n : new size of the list
         */
        void resize(size_type n) { resize(n, _Tp()); }
        void resize(size_type n, const_reference value) {
            if (n == 0)
                clear();
            else if (n != _size) {
                link_type _curr = _head._next;
                for (; n > 1 && _curr != _head._next; --n, _curr = _curr->_next)
                    ;
                if (n == 1 && _curr != _head._next) {
                    _Destroy(_curr->_next, _head._next);
                    _curr->_next = _head._next;
                    _tail        = _curr;
                }
                else {
                    _Nodes_creator _creator(*this);
                    _creator.create_n(n, value);
                    _Make_link(_curr, _creator._first, _creator._last);
                }
            }
        }

        /**
         *	@return returns the number of elements in the slist
         */
        [[nodiscard]] size_type size() const noexcept { return _size; }
        /**
         *	@return returns the maximum number of elements the slist is able to hold due to system or library implementation limitations
         */
        [[nodiscard]] size_type max_size() const noexcept { return (std::min)(static_cast<size_type>((std::numeric_limits<difference_type>::max)()), _Self::_Alnode_traits::max_size(_Getal())); }

        /**
         *	@brief removes all elements from the slist.
         */
        void clear() noexcept {
            _Destroy(_head._next, _head._next);
            _head._next = _head._next;
            _tail       = std::addressof(_head);
        }

        [[nodiscard]] bool empty() const noexcept { return _size == 0; }

        [[nodiscard]] bool operator==(const _Self& rhs) const { return _size == rhs._size && std::equal(cbegin(), cend(), rhs.cbegin()); }

        [[nodiscard]] std::strong_ordering operator<=>(const _Self& rhs) const {
            if (_size != rhs._size)
                return _size <=> rhs._size;
            for (link_type _curr1 = _head._next, _curr2 = rhs._head._next; _curr1 != _head._next && _curr2 != rhs._head._next; _curr1 = _curr1->_next, _curr2 = _curr2->_next)
                if (std::strong_ordering _res = VALUE(_curr1) <=> VALUE(_curr2); _res != std::strong_ordering::equivalent)
                    return _res;
            return std::strong_ordering::equivalent;
        }

        [[nodiscard]] _Self operator+(const _Self& rhs) { return _Self(*this) += rhs; }
        [[nodiscard]] _Self operator+(_Self&& rhs) { return _Self(*this) += std::move(rhs); }

        _Self& operator+=(const _Self& rhs) {
            _Unchecked_insert(cend(), rhs.cbegin(), rhs.cend());
            return *this;
        }
        _Self& operator+=(_Self&& rhs) {
            splice(cend(), rhs);
            return *this;
        }

        _Self& operator=(const _Self& rhs) {
            if (this == std::addressof(rhs))
                return *this;
            link_type _curr1 = _head._next, _curr2 = rhs._head._next, _prev = std::addressof(_head);
            for (; _curr1 != _head._next && _curr2 != rhs._head._next; _curr1 = _curr1->_next, _curr2 = _curr2->_next) {
                _prev         = _curr1;
                VALUE(_curr1) = VALUE(_curr2);
            }
            if (_curr1) {
                _Destroy(_curr1, _head._next);
                _prev->_next = _head._next;
                _tail        = _prev;
            }
            else
                _Unchecked_insert(cend(), const_iterator(_curr2), rhs.cend());
            alloc_pocca(_Getal(), rhs._Getal());
            return *this;
        }
        _Self& operator=(_Self&& rhs) noexcept {
            if (this == std::addressof(rhs))
                return *this;
            clear();
            _head._next = std::exchange(rhs._head._next, _head._next);
            _tail       = std::exchange(rhs._tail, std::addressof(rhs._head));
            _size       = std::exchange(rhs._size, size_type{ 0 });
            alloc_pocma(_Getal(), rhs._Getal());
            return *this;
        }
        _Self& operator=(std::initializer_list<_Tp> l) {
            assign(l.begin(), l.end());
            return *this;
        }

        ~slist() { clear(); }

    private:
        inline void _Check_max_size(size_type newsz) {
            if (newsz >= max_size())
                throw std::length_error("singly linked list too long");
        }

#if !defined(_NO_SLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
        inline iterator       _Make_iter(link_type node) const noexcept { return iterator(node, const_cast<_Self*>(this)); }
        inline const_iterator _Make_const_iter(link_type node) const noexcept { return const_iterator(node, const_cast<_Self*>(this)); }
#else
        inline iterator       _Make_iter(link_type node) const noexcept { return iterator(node); }
        inline const_iterator _Make_const_iter(link_type node) const noexcept { return const_iterator(node); }
#endif

        template <class... _Args>
        [[nodiscard]] inline _Slist_node<_Tp>* _Create_node(link_type next, _Args&&... values) {
            _Slist_node<_Tp>* _node     = _Self::get_node();
            static auto       _releaser = [this](_Slist_node<_Tp>* node) { this->put_node(node); };
            _Exception_guard  _guard(_node, std::addressof(_releaser));
            _Self::_Alnode_traits::construct(_Self::_Getal(), std::addressof(_node->_value), std::forward<_Args>(values)...);
            _node->_next = next, _size++;
            return _guard.release();
        }

        [[nodiscard]] inline _Slist_node<_Tp>* _Create_node(const _Tp& value, link_type _next = nullptr) {
            _Slist_node<_Tp>* _node     = _Self::get_node();
            static auto       _releaser = [this](_Slist_node<_Tp>* node) { this->put_node(node); };
            _Exception_guard  _guard(_node, std::addressof(_releaser));
            _Self::_Alnode_traits::construct(_Self::_Getal(), std::addressof(_node->_value), value);
            _node->_next = _next, _size++;
            return _guard.release();
        }

        void _Unchecked_insert(const_iterator position, size_type n, const_reference value) {
            if (n <= 0)
                return;
            _Nodes_creator _creator(*this);
            _creator.create_n(n, value);
            _Make_link(position.base(), _creator._first, _creator._last);
        }

        template <std::input_iterator _Iter>
        link_type _Unchecked_insert(const_iterator position, _Iter first, _Iter last) {
            if (first == last)
                return position.base();
            _Nodes_creator _creator(*this);
            _creator.create_from_range(first, last);
            return _Make_link(position.base(), _creator._first, _creator._last);
        }

        link_type _Unchecked_erase(link_type position) noexcept {
            const link_type _suc = position->_next;
            _Destroy(position, _Break_link(position, _suc));
            return _suc;
        }

        void _Destroy(link_type first, link_type last) noexcept {
            for (; first != last; --_size)
                _Self::destroy_node(CAST(std::exchange(first, first->_next)));
        }

        link_type _Break_link(link_type first, link_type last) noexcept {
            link_type _before_first = std::addressof(_head);
            for (; _before_first->_next != first; _before_first = _before_first->_next)
                ;
            _before_first->_next = last;
            if (last == _head._next)
                _tail = _before_first;
            return last;
        }

        link_type _Make_link(link_type pos, link_type first, link_type last) noexcept {
            link_type *_prev, _curr = _head._next;
            for (_prev = std::addressof(_head._next); (_curr = *_prev) != pos; _prev = std::addressof(_curr->_next))
                ;
            last->_next = _curr;
            *_prev      = first;
            return first;
        }

        template <class _Pred>
        link_type _Sort(link_type first, size_type bound, _Pred cmpr) {
            if (bound <= 2)
                return _Sort2(first, cmpr);
            const link_type _mid = _Sort(first, bound >> 1, cmpr);
            if (_mid->_next == _head._next)
                return _mid;
            const link_type _last = _Sort(_mid, bound >> 1, cmpr);
            return _Merge_same(first, _mid, _last, cmpr);
        }

        template <class _Pred>
        link_type _Sort2(const link_type curr, _Pred cmpr) {
            const link_type _next1 = curr->_next;
            if (_next1 == _head._next)
                return curr;
            link_type _next2 = _next1->_next;
            if (_next2 == _head._next || !cmpr(VALUE(_next2), VALUE(_next1)))
                return _next1;
            _next1->_next = _next2->_next;
            curr->_next   = _next2;
            _next2->_next = _next1;
            return _next2;
        }

        template <class _Pred>
        link_type _Merge_same(link_type first, const link_type mid, const link_type last, _Pred cmpr) {
            // merge (first, mid] and (mid, last)
            link_type   _curr1    = first, _curr2;
            static auto _transfer = [&]() {
                link_type _tmp = _curr1->_next;
                _curr1->_next  = mid->_next;
                mid->_next     = _curr2->_next;
                _curr2->_next  = _tmp;
            };
            while (_curr1 != mid && mid != last) {
                for (_curr2 = mid; _curr2 != last && cmpr(VALUE(_curr2->_next), VALUE(_curr1->_next)); _curr2 = _curr2->_next)
                    ;
                if (_curr2 != mid)
                    _transfer();
                _curr1 = _curr1->_next;
            }
            if (_curr1 != mid) {
                for (_curr2 = mid; _curr2->_next != last; _curr2 = _curr2->_next)
                    ;
                _transfer();
                return mid;
            }
            return last;
        }

        size_type _size = 0;
    };

#define _SLIST_DEDUCTION_GUIDES(LIST)                                                                               \
    template <class _Tp, class _Alloc>                                                                              \
    LIST(const LIST<_Tp, _Alloc>&, const _Alloc& = _Alloc()) -> LIST<_Tp, _Alloc>;                                  \
    template <class _Tp, class _Alloc>                                                                              \
    LIST(LIST<_Tp, _Alloc>&&, const _Alloc& = _Alloc()) -> LIST<_Tp, _Alloc>;                                       \
    template <class _Iter, class _Alloc>                                                                            \
    LIST(_Iter, _Iter, const _Alloc& = _Alloc()) -> LIST<typename std::iterator_traits<_Iter>::value_type, _Alloc>; \
    template <class _Tp, class _Alloc>                                                                              \
    LIST(size_t, const _Tp&, const _Alloc& = _Alloc()) -> LIST<_Tp, _Alloc>;                                        \
    template <class _Tp, class _Alloc>                                                                              \
    LIST(std::initializer_list<_Tp>, const _Alloc& = _Alloc()) -> LIST<_Tp, _Alloc>;

    _SLIST_DEDUCTION_GUIDES(slist);
    _SLIST_DEDUCTION_GUIDES(slist);
}  // namespace xstl

namespace std {
    template <class _Tp, class _Alloc, class _Self>
    void swap(xstl::_Slist_base<_Tp, _Alloc>& x, xstl::_Slist_base<_Tp, _Alloc>& y) noexcept {
        x.swap(y);
    }
}  // namespace std
#undef CAST
#undef VALUE
#undef _SLIST_DEDUCTION_GUIDES
#endif