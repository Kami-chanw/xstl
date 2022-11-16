/*
*	Copyright(c) 2020 Shen Xian.All rights reserved.
*   @file dlist.hpp
*   @brief The doubly linked list library contains 2 types of doubly linked lists:
*	1. dlist (doubly linked list)
*	2. ldlist (loop doubly linked list)
*   @author Kami-chan e-mail: 865710157@qq.com
*/

#ifndef _DLIST_HPP_
#define _DLIST_HPP_

#include <algorithm>
#include "utility.hpp"
#include "config.hpp"
#undef CAST
#undef VALUE
#define CAST(NODE) static_cast<_Dlist_node<value_type>*>(NODE)
#define VALUE(NODE) CAST(NODE)->_value

namespace xstl {
	namespace {
		/**
		*	@class _Dlist_node_base
		*   @brief the node base of dlist.
		*/
		struct _Dlist_node_base {
			_Dlist_node_base(_Dlist_node_base* prev = nullptr, _Dlist_node_base* next = nullptr) : _prev(prev), _next(next) { }
			_Dlist_node_base* _prev;
			_Dlist_node_base* _next;
		};

		/**
		*	@class _Dlist_node
		*   @brief the node of dlist.
		*/
		template <class _Tp>
		struct _Dlist_node : _Dlist_node_base {
			using _Base = _Dlist_node_base;
			using value_type = _Tp;

			_Dlist_node() = default;
			explicit _Dlist_node(const _Tp& value, _Base* prev = nullptr, _Base* next = nullptr) : _value(value), _Base(prev, _next) { }

			_Tp _value;
		};

		/**
		*	@class _Dlist_pack
		*   @brief some functions for dlist's and ldlist's iterators.
		*/
		template <class _Cont>
		struct _Dlist_pack {
			using container_type = _Cont;
			using value_type = typename _Cont::value_type;
			using link_type = typename _Cont::link_type;

			static void incr(_Dlist_node_base*& node) noexcept {
				if (node)
					node = node->_next;
			}

			static void decr(_Dlist_node_base*& node) noexcept {
				if (node)
					node = node->_prev;
			}

			static value_type& extract(_Dlist_node_base* node) noexcept {
				return VALUE(node);
			}
		};

		/**
		*	@class _Dlist_base
		*   @brief doubly linked list interfaces.
		*/
		template <class _Tp, class _Alloc, class _CRTP>
		class _Dlist_base : protected container_base<_Dlist_node<_Tp>, _Alloc, _Dlist_pack<_Dlist_base<_Tp, _Alloc, _CRTP>>> {
			using _Self = _Dlist_base<_Tp, _Alloc, _CRTP>;
			using _Base = container_base<_Dlist_node<_Tp>, _Alloc, _Dlist_pack<_Self>>;
			using _Altp_traits = typename _Base::_Altp_traits;
			using _Base::_Getal;
		public:
			using link_type = _Dlist_node_base*;
		protected:
			using node = _Dlist_node_base;
			struct _Nodes_creator {
				explicit _Nodes_creator(_Self& ref_list) :_ref_list(ref_list) { }

				template <class _Iter>
				void create_from_range(_Iter first, _Iter last) {
					_ref_list._Check_max_size(_ref_list._size);
					_first = _ref_list._Create_node(*first);
					for (_last = _first; ++first != last; _last = _last->_next) {
						_ref_list._Check_max_size(_ref_list._size);
						_last->_next = _ref_list._Create_node(*first);
						_last->_next->_prev = _last;
					}
				}

				template <class _Tp>
				void create_n(typename _Altp_traits::size_type n, const _Tp& value) {
					_ref_list._Check_max_size(_ref_list._size + n);
					_first = _ref_list._Create_node(value);
					for (_last = _first; --n; _last = _last->_next) {
						_last->_next = _ref_list._Create_node(value);
						_last->_next->_prev = _last;
					}
				}

				~_Nodes_creator() {
					if (_last == nullptr)
						_ref_list._Destroy(_first, _last);
				}

				_Self& _ref_list;
				link_type _first = nullptr;
				link_type _last = nullptr;
			};

		public:
			using allocator_type	=		typename _Base::allocator_type;
			using value_type		=		_Tp;
			using reference			=		_Tp&;
			using const_reference	=		const _Tp&;
			using pointer			=		typename _Altp_traits::pointer;
			using const_pointer		=		typename _Altp_traits::const_pointer;
			using const_iterator	=		xstl_iterator::unchecked_bid_citer<_Dlist_node_base*,  _Dlist_pack<_Self>>;
			using iterator			=		xstl_iterator::unchecked_bid_iter<_Dlist_node_base*, _Dlist_pack<_Self>>;
			using size_type			=		typename _Altp_traits::size_type;
			using difference_type	=		typename _Altp_traits::difference_type;

			_Dlist_base(const allocator_type& alloc) : _Base(alloc), _head(_Nil(), _Nil()) { }
			_Dlist_base(const _Dlist_base& x) : _head(_Nil(), _Nil()) {
				_Dertp().tail() = ::std::addressof(_head);
				insert(begin(), x.begin(), x._Dertp().end());
				alloc_pocca(_Getal(), _Base::_Alnode_traits::select_on_container_copy_construction(x._Getal()));
			}
			_Dlist_base(const _Dlist_base& x, const allocator_type& alloc) : _Base(alloc), _head(_Nil(), _Nil()) {
				_Dertp().tail() = ::std::addressof(_head);
				insert(begin(), x.begin(), x._Dertp().end());
				alloc_pocca(_Getal(), _Base::_Alnode_traits::select_on_container_copy_construction(x._Getal()));
			}
			_Dlist_base(_Dlist_base&& x) noexcept : _head(_Nil(), ::std::exchange(x._head._next, x._Nil())),
				_size(::std::exchange(x._size, size_type{ 0 })) {
				_Dertp().tail() = ::std::exchange(x._Dertp().tail(), ::std::addressof(x._head));
				alloc_pocma(_Getal(), x._Getal());
			}
			_Dlist_base(_Dlist_base&& x, const allocator_type& alloc) noexcept(_Base::_Alnode_traits::is_always_equal::value) : _Base(alloc),
				_head(_Nil(), ::std::exchange(x._head._next, _Nil())),
				_size(::std::exchange(x._size, size_type{ 0 })) {
				_Dertp().tail() = ::std::exchange(x._Dertp().tail(), ::std::addressof(x._head));
			}
			template <typename _Iter>
			_Dlist_base(_Iter first, _Iter last, const allocator_type& alloc = allocator_type()) : _Base(alloc), _head(_Nil(), _Nil()) {
				_Dertp().tail() = ::std::addressof(_head);
				insert(begin(), first, last);
			}
			explicit _Dlist_base(size_type n, const allocator_type& alloc = allocator_type()) : _Base(alloc), _head(_Nil(), _Nil()) {
				_Dertp().tail() = ::std::addressof(_head);
				insert(begin(), n, _Tp());
			}
			_Dlist_base(size_type n, const_reference value, const allocator_type& alloc = allocator_type()) : _Base(alloc), _head(_Nil(), _Nil()) {
				_Dertp().tail() = ::std::addressof(_head);
				insert(begin(), n, value);
			}

			/**
			*   @return a reference to the first element in the ldlist.
			*/
			[[nodiscard]] reference front() noexcept {
#if !defined(_NO_DLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
				assert(("front() called on empty list", _size >= 0));
#endif
				return VALUE(_head._next);
			}
			[[nodiscard]] const_reference front() const noexcept {
#if !defined(_NO_DLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
				assert(("front() called on empty list", _size >= 0));
#endif
				return VALUE(_head._next);
			}

			[[nodiscard]] iterator begin() noexcept { return iterator(_head._next, this); }
			[[nodiscard]] const_iterator begin() const noexcept { return const_iterator(_head._next, this); }
			[[nodiscard]] const_iterator cbegin() const noexcept { return const_iterator(_head._next, this); }
			[[nodiscard]] iterator end() noexcept { return iterator(_Nil(), this); }
			[[nodiscard]] const_iterator end() const noexcept { return const_iterator(_Nil()); }
			[[nodiscard]] const_iterator cend() const noexcept { return const_iterator(_Nil()); }

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
				_Check_max_size(_size);
				link_type _node = _Create_node(value);
				return iterator(_Dertp().make_link(position.base(), _node, _node), this);
			}
			iterator insert(const_iterator position, value_type&& value) {
				_Check_max_size(_size);
				link_type _node = _Create_node(::std::move(value));
				return iterator(_Dertp().make_link(position.base(), _node, _node), this);
			}
			/**
			*   @brief inserts elements from initializer list l before position.
			*   @param position : iterator before which the content will be inserted
			*   @param l : initializer list to insert the values from
			*/
			void insert(const_iterator position, ::std::initializer_list<_Tp> l) {
				insert(position, l.begin(), l.end());
			}
			/**
			*   @brief initializes the list with elements each with value
			*   @param n : number of elements to insert.
			*   @param value : element value to insert
			*/
			void insert(const_iterator position, size_type n, const_reference value) {
				if (n <= 0)
					return;
				_Nodes_creator _creator(*this);
				_creator.create_n(n, value);
				_Dertp().make_link(position.base(), _creator._first, _creator._last);
			}
			/**
			*   @brief inserts elements from range [first, last) before pos.
			*   @param position : iterator before which the content will be inserted
			*   @param first : the beginning of range of elements to insert
			*	@param _out: the end of range of elements to insert
			*   @return an iterator which points to the first new node
			*/
			template <::std::input_iterator _Iter>
			iterator insert(const_iterator position, _Iter first, _Iter last) {
				if (first == last)
					return iterator(position.base(), this);
				_Nodes_creator _creator(*this);
				_creator.create_from_range(first, last);
				return iterator(_Dertp().make_link(position.base(), _creator._first, _creator._last), this);
			}

			/**
			*   @brief inserts a new element to the beginning of the dlist.
			*   @param values : arguments to forward to the constructor of the element
			*/
			template <class ..._Args>
			void emplace_front(_Args&& ... values) {
				emplace(cbegin(), ::std::forward<_Args>(values)...);
			}
			/**
			*   @brief inserts a new element to the end of the dlist.
			*   @param values : arguments to forward to the constructor of the element
			*/
			template <class ..._Args>
			void emplace_back(_Args&& ... values) {
				emplace(_Dertp().cend(), ::std::forward<_Args>(values)...);
			}
			/**
			*	@brief inserts a new element into the dlist directly before position
			*	@param position : iterator before which the new element will be constructed
			*	@param values : arguments to forward to the constructor of the element
			*/
			template <class ..._Args>
			void emplace(const_iterator position, _Args&& ... values) {
				_Check_max_size(_size);
				link_type _node = _Create_node(::std::forward<_Args>(values)...);
				_Dertp().make_link(position.base(), _node, _node);
			}

			/**
			*	@brief appends the given element value to the end of the dlist.
			*	@param value : 	the value of the element to append
			*/
			void push_front(const_reference value) { insert(cbegin(), value); }
			void push_front(value_type&& value) { insert(cbegin(), ::std::move(value)); }
			/**
			*	@brief prepends the given element value to the beginning of the dlist.
			*	@param value : 	the value of the element to append
			*/
			void push_back(const_reference value) { insert(_Dertp().cend(), value); }
			void push_back(value_type&& value) { insert(_Dertp().cend(), ::std::move(value)); }

			/**
			*	@brief removes the first element of the dlist.
			*/
			void pop_front() noexcept { erase(cbegin()); }

			/**
			*	@brief removes the last element of the ldlist.
			*/
			void pop_back() noexcept { erase(const_iterator(_Dertp().tail(), this)); }

			/**
			*	@brief removes specified elements from the dlist
			*	@param position : iterator to the element to remove
			* 	@return iterator following the last removed element.
			*/
			iterator erase(const_iterator position) noexcept {
				return position.base() != _Nil() ? erase(position, ::std::next(position)) : _Dertp().end();
			}
			/**
			*	@brief removes the elements in the range [first, last).
			*	@param first : the beginning of range to erase the elements from
			*	@param last : the end of range to erase the elements from
			* 	@return iterator following the last removed element.
			*/
			iterator erase(const_iterator first, const_iterator last) noexcept {
				if (_Dertp().empty())
					return _Dertp().end();
				_Dertp().break_link(first.base(), last.base());
				_Destroy(first.base(), last.base());
				return iterator(last.base(), this);
			}

			/**
			*	@brief transfers all elements from other into *this. The elements are inserted before the element pointed to by position. The _CRTP other becomes empty after the operation.
			*	@param position : element before which the content will be inserted
			*	@param x : another list to transfer the content from
			*/
			void splice(const_iterator position, _CRTP& x) {
				if constexpr (!_Base::_Alnode_traits::is_always_equal::value)
					assert(("list allocators incompatible for splice", _Getal() != x._Getal()));
				if (!x.empty() && this != ::std::addressof(x)) {
					_Splice(position.base(), x, x._Nil());
					_size += ::std::exchange(x._size, size_type{ 0 });
				}
			}
			void splice(const_iterator position, _CRTP&& x) { splice(position, x); }
			/**
			*	@brief transfers the element pointed to by it from other into *this. The element is inserted before the element pointed to by position.
			*	@param position : element before which the content will be inserted
			*	@param x : another list to transfer the content from
			*	@param iter : the element to transfer from other to *this
			*/
			void splice(const_iterator position, _CRTP& x, const_iterator iter) {
				splice(position, x, iter, ::std::next(iter));
			}
			void splice(const_iterator position, _CRTP&& x, const_iterator iter) {
				splice(position, x, iter, ::std::next(iter));
			}
			/**
			*	@brief transfers the elements in the range [first, last) from other into *this. The elements are inserted before the element pointed to by position.
			*	@param position : element before which the content will be inserted
			*	@param x : another list to transfer the content from
			*	@param first : the beginning of range of elements to transfer from other to *this
			*	@param end : the end of range of elements to transfer from other to *this
			*/
			void splice(const_iterator position, _CRTP& x, const_iterator first, const_iterator last) {
				if constexpr (!_Base::_Alnode_traits::is_always_equal::value)
					assert(("list allocators incompatible for splice", _Getal() != x._Getal()));
				if (first != last && this != ::std::addressof(x)) {
					const size_type _count = ::std::distance(first, last);
					_Dertp().make_link(position.base(), first.base(), x.break_link(first.base(), last.base()));
					_size += _count;
					x._size -= _count;
				}
			}
			void splice(const_iterator position, _CRTP&& x, const_iterator first, const_iterator last) {
				splice(position, x, first, last);
			}

			/**
			*	@brief replaces the contents with the elements from the initializer list l.
			*	@param l : initializer list to copy the values from
			*/
			void assign(::std::initializer_list<_Tp> l) { 
				assign(l.cbegin(), l.cend()); 
			}
			/**
			*	@brief replaces the contents with count copies of value value
			*	@param n : the new size of the ldlist
			*	@param value : the value to initialize elements of the ldlist with
			*/
			void assign(size_type n, const_reference value) {
				link_type _curr = _head._next;
				for (int i = 0; i < n; ++i, _curr = _curr->_next) {
					if (_curr == _Nil()) {
						insert(_Dertp().cend(), n - i, value);
						return;
					}
					VALUE(_curr) = value;
				}
				_curr->_prev->_next = _Nil();
				_Dertp().tail() = _curr->_prev;
				_Destroy(_curr, _Nil());
			}
			/**
			*	@brief replaces the contents with copies of those in the range [first, last).
			*	@param first : the beginning of range to copy the elements from
			*	@param last : the end of range to copy the elements from
			*/
			template <::std::input_iterator _Iter>
			void assign(_Iter first, _Iter last) {
				if (first == last)
					clear();
				else {
					link_type _curr = _head._next;
					do {
						if (_curr == _Nil()) {
							insert(_Dertp().cend(), first, last);
							return;
						}
						VALUE(_curr) = *first;
						_curr = _curr->_next;
					} while (++first != last);
					_curr->_prev->_next = _Nil();
					_Dertp().tail() = _curr->_prev;
					_Destroy(_curr, _Nil());
				}
			}

			/**
			*	@brief removes all consecutive duplicate elements from the dlist.
			*	@param pred : binary predicate which returns ​true if the elements should be treated as equal.
			*/
			size_type unique() { return unique(::std::equal_to<_Tp>()); }
			template <class _Pred>
			size_type unique(_Pred pred) {
				const size_type _oldsz = _size;
				if (_size > 1) {
					for (link_type _curr = _head._next; _curr->_next != _Nil();)
						if (pred(VALUE(_curr), (VALUE(_curr->_next)))) {
							link_type _prev = _Dertp().break_link(_curr, _curr->_next)->_prev;
							_Destroy(_curr, _curr->_next);
							_curr = _prev;
						}
						else
							_curr = _curr->_next;
				}
				return _oldsz - _size;
			}

			/**
			*	@brief removes all elements that are equal to value
			*	@param value : value of the elements to remove
			*/
			size_type remove(const_reference value) {
				return remove_if([&](const_reference _val) {return _val == value; });
			}
			/**
			*	@brief removes all elements for which predicate pred returns true.
			*	@param pred unary predicate which returns ​true if the element should be removed.
			*	@return the numbers of elements removed.
			*/
			template <class _Pred>
			size_type remove_if(_Pred pred) {
				const size_type _oldsz = _size;
				if (!empty()) {
					for (link_type _curr = _head._next; _curr->_next != _Nil();)
						if (pred(VALUE(_curr))) {
							link_type _next = _Dertp().break_link(_curr, _curr->_next)->_next;
							_Destroy(_curr, _curr->_next);
							_curr = _next;
						}
						else
							_curr = _curr->_next;
				}
				return _oldsz - _size;
			}

			/**
			*	@brief sorts the elements in ascending order. The first version uses operator< to compare the elements. The second version uses the given comparison function comp
			*	@param pred comparison function object.
			*/
			void sort() { sort(::std::less<_Tp>()); }
			template <class _Pred>
			void sort(_Pred cmpr) { _Sort(_head._next, _size, cmpr); }

			/**
			*	@brief merges two sorted lists into one. the lists should be sorted into ascending order. this version uses operator< to compare the elements
			*	@param x : another list to merge
			*/
			void merge(_CRTP& x) { merge(x, ::std::less<_Tp>()); }
			void merge(_CRTP&& x) { merge(x, ::std::less<_Tp>()); }
			/**
			*	@brief merges two sorted lists into one. The lists should be sorted into ascending order. this version uses the given comparison function comp.
			*	@param x : another list to merge
			*	@param pred : comparison function object
			*/
			template <class _Pred>
			void merge(_CRTP&& x, _Pred pred) { merge(x, pred); }
			template <class _Pred>
			void merge(_CRTP& x, _Pred pred) {
				if (x.empty() || this == ::std::addressof(x))
					return;
				if constexpr (!_Base::_Alnode_traits::is_always_equal::value)
					assert(("list allocators incompatible for merge", _Getal() != x._Getal()));
				link_type _curr1 = _head._next;
				while (_curr1 != _Nil() && x._head._next != x._Nil()) {
					if (pred(x.front(), VALUE(_curr1))) {
						link_type _curr2 = x._head._next;
						do {
							_curr2 = _curr2->_next;
						} while (_curr2 != x._Nil() && pred(VALUE(_curr2), VALUE(_curr1)));
						_Splice(_curr1, x, _curr2);
					}
					_curr1 = _curr1->_next;
				}
				if (x._head._next != x._Nil())
					_Splice(_curr1, x, x._Nil());
				_size += ::std::exchange(x._size, size_type{ 0 });
			}

			/**
			*	@brief removes all elements from the dlist.
			*/
			void clear() noexcept {
				_Destroy(_head._next, _Nil());
				_head._next = _Nil();
				_Dertp().tail() = ::std::addressof(_head);
			}

			/**
			*	@brief exchanges the contents of the dlist with those of other.
			*	@param x : container to exchange the contents with
			*/
			void swap(_Self& x) noexcept {
				if (this != ::std::addressof(x)) {
					using ::std::swap;
					swap(_head._next->_prev, x._head._next->_prev);
					swap(_head, x._head);
					swap(_size, x._size);
					swap(_Dertp().tail(), x._Dertp().tail());
					alloc_pocs(_Getal(), x._Getal());
				}
			}

			/**
			*	@brief resizes the dlist to contain count elements.
			*	@param n : new size of the dlist
			*/
			void resize(size_type n) {
				resize(n, _Tp());
			}
			void resize(size_type n, const_reference value) {
				if (n == 0)
					clear();
				else if (n != _size) {
					link_type _curr = _head._next;
					for (; n > 1 && _curr != _Nil(); --n, _curr = _curr->_next);
					if (n == 1 && _curr != _Nil()) {
						_Destroy(_curr->_next, _Nil());
						_curr->_next = _Nil();
						_Dertp().tail() = _curr;
					}
					else {
						_Nodes_creator _creator(*this);
						_creator.create_n(n, value);
						_Dertp().make_link(_curr, _creator._first, _creator._last);
					}
				}
			}

			/**
			*	@return returns the number of elements in the ldlist
			*/
			[[nodiscard]] size_type size() const noexcept { return _size; }
			/**
			*	@return returns the maximum number of elements the ldlist is able to hold due to system or library implementation limitations
			*/
			[[nodiscard]] size_type max_size() const noexcept {
				return (::std::min)(static_cast<size_type>((::std::numeric_limits<difference_type>::max)()), _Base::_Alnode_traits::max_size(_Getal()));
			}
			[[nodiscard]] bool empty() const noexcept { return _size == 0; }

			[[nodiscard]] bool operator==(const _CRTP& rhs) const {
				return _size == rhs._size && ::std::equal(begin(), _Dertp().end(), rhs.begin());
			}
			[[nodiscard]] ::std::strong_ordering operator<=>(const _CRTP& rhs) const {
				if (_size != rhs._size)
					return _size <=> rhs._size;
				for (link_type _curr1 = _head._next, _curr2 = rhs._head._next; _curr1 != _Nil() && _curr2 != rhs.nil(); _curr1 = _curr1->_next, _curr2 = _curr2->_next)
					if (::std::strong_ordering _res = VALUE(_curr1) <=> VALUE(_curr2); _res != ::std::strong_ordering::equivalent)
						return _res;
				return ::std::strong_ordering::equivalent;
			}

			_Self& operator=(const _Self& rhs) {
				if (this != ::std::addressof(rhs)) {
					link_type _curr1 = _head._next, _curr2 = rhs._head._next;
					for (; _curr1 != _Nil() && _curr2 != rhs._Nil(); _curr1 = _curr1->_next, _curr2 = _curr2->_next)
						VALUE(_curr1) = VALUE(_curr2);
					if (_curr1 != _Nil()) {
						_Dertp().tail() = _curr1->_prev;
						_curr1->_prev->_next = _Nil();
						_Destroy(_curr1, _Nil());
					}
					else
						insert(_Dertp().cend(), const_iterator(_curr2, this), rhs._Dertp().cend());
					alloc_pocca(_Getal(), rhs._Getal());
				}
				return *this;
			}

			_Self& operator=(_Self&& rhs) {
				if (this != ::std::addressof(rhs)) {
					clear();
					_head._next = rhs._head._next;
					rhs._head._next->_prev = ::std::addressof(_head);
					_Dertp().tail() = ::std::exchange(rhs._Dertp().tail(), rhs._Nil());
					rhs._head._next = rhs._Nil();
					_size = ::std::exchange(rhs._size, size_type{ 0 });
					alloc_pocma(_Getal(), rhs._Getal());
				}
				return *this;
			}

			[[nodiscard]] _CRTP operator+(const _CRTP& rhs) { return _CRTP(_Dertp()) += rhs; }
			[[nodiscard]] _CRTP operator+(_CRTP&& rhs) { return _CRTP(_Dertp()) += ::std::move(rhs); }
			_CRTP& operator+=(const _CRTP& rhs) {
				insert(_Dertp().cend(), rhs.cbegin(), rhs.cend());
				return _Dertp();
			}
			_CRTP& operator+=(_CRTP&& rhs) {
				splice(_Dertp().cend(), rhs);
				return _Dertp();
			}
			xstl_iterator::back_insert_iterator<_CRTP> operator+=(const_reference value) {
				push_back(value);
				return{ _Dertp() };
			}
		protected:
			

			node _head;
			size_type _size = 0;

		private:
			inline void _Check_max_size(size_type newsz) {
				if (newsz >= max_size())
					throw ::std::length_error("doubly linked list too long");
			}
			inline _CRTP& _Dertp() noexcept { return static_cast<_CRTP&>(*this); }
			inline const _CRTP& _Dertp() const noexcept { return static_cast<const _CRTP&>(*this); }
			inline link_type _Nil() noexcept { return _Dertp().nil(); }
			inline const node* _Nil() const noexcept { return _Dertp().nil(); }

			template <class... _Args>
			[[nodiscard]] inline _Dlist_node<_Tp>* _Create_node(_Args&& ... values) {
				_Dlist_node<_Tp>* _node = _Base::get_node();
				static auto _releaser = [this](_Dlist_node<_Tp>* node) { this->put_node(node); };
				_Exception_guard _guard(_node, ::std::addressof(_releaser));
				_Base::_Alnode_traits::construct(_Base::_Getal(), ::std::addressof(_node->_value), ::std::forward<_Args>(values)...);
				_node->_prev = _node->_next = _Nil(), _size++;
				return _guard.release();
			}

			[[nodiscard]] inline _Dlist_node<_Tp>* _Create_node(const _Tp& value, link_type _prev = nullptr, link_type _next = nullptr) {
				_Dlist_node<_Tp>* _node = _Base::get_node();
				static auto _releaser = [this](_Dlist_node<_Tp>* node) { this->put_node(node); };
				_Exception_guard _guard(_node, ::std::addressof(_releaser));
				_Base::_Alnode_traits::construct(_Base::_Getal(), ::std::addressof(_node->_value), value);
				_node->_prev = _prev, _node->_next = _next, _size++;
				return _guard.release();
			}

			void _Destroy(link_type first, link_type last = nullptr) noexcept {
				for (; first != last; _size--)
					_Base::destroy_node(CAST(::std::exchange(first, first->_next)));
			}

			template <class _Pred>
			link_type _Sort(link_type& first, size_type n, _Pred cmpr) {
				switch (n) {
				case 0:
					return first;
				case 1:
					return first->_next;
				default:
					link_type _mid = _Sort(first, n >> 1, cmpr);
					const link_type _last = _Sort(_mid, n - n / 2, cmpr);
					//merge [first, mid) and [mid, last)
					link_type _curr1 = first;
					while (_curr1 != _mid && _mid != _last) {
						if (cmpr(VALUE(_mid), VALUE(_curr1))) {
							if (_curr1 == first)
								first = _mid;
							link_type _curr2 = _mid;
							do {
								_curr2 = _curr2->_next;
							} while (_curr2 != _last && cmpr(VALUE(_curr2), VALUE(_curr1)));
							_Splice(_curr1, _mid, _curr2);
							_mid = _curr2;
						}
						_curr1 = _curr1->_next;
					}
					return _last;
				}
			}

			inline void _Splice(link_type pos, _CRTP& x, link_type last) noexcept {
				const link_type _oldhead = x._head._next;
				_Dertp().make_link(pos, _oldhead, x.break_link(x._head._next, last));
			}
			inline void _Splice(link_type pos, link_type first, link_type last) noexcept {
				const link_type _oldfirst = first;
				_Dertp().make_link(pos, _oldfirst, _Dertp().break_link(first, last));
			}
		};
	}

	/**
	*	@class dlist
	*   @brief doubly linked list.
	*/
	template <class _Tp, class _Alloc = _DEFAULT_ALLOC(_Tp)>
	class dlist : public _Dlist_base<_Tp, _Alloc, dlist<_Tp, _Alloc>> {
		using _Base = _Dlist_base<_Tp, _Alloc, dlist<_Tp, _Alloc>>;
		using _Base::_head;
		using _Base::_size;
		using typename _Base::node;
		using typename _Base::link_type;
	public:
		friend class _Dlist_base<_Tp, _Alloc, dlist<_Tp, _Alloc>>;
		using const_iterator			=		typename _Base::const_iterator;
		using iterator					=		typename _Base::iterator;
		using const_reverse_iterator	=		xstl_iterator::reverse_iterator<const_iterator>;
		using reverse_iterator			=		xstl_iterator::reverse_iterator<iterator>;
		using value_type				=		_Tp;
		using difference_type			=		typename _Base::difference_type;
		using size_type					=		typename _Base::size_type;
		using reference					=		_Tp&;
		using const_reference			=		const _Tp&;
		using pointer					=		typename _Base::pointer;
		using const_pointer				=		typename _Base::const_pointer;
		using allocator_type			=		typename _Base::allocator_type;

		/**
		*   @return a reference to the last element in the dlist.
		*/
		[[nodiscard]] reference back() noexcept { 
#if !defined(_NO_DLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
			assert(("back() called on empty list", _size >= 0));
#endif
			return VALUE(_tail); 
		}
		[[nodiscard]] const_reference back() const noexcept { 
#if !defined(_NO_DLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
			assert(("back() called on empty list", _size >= 0));
#endif
			return VALUE(_tail); 
		}

		using _Base::begin;
		[[nodiscard]] reverse_iterator rbegin() noexcept { return reverse_iterator(iterator(_tail, this)); }
		[[nodiscard]] reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
		[[nodiscard]] const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(const_iterator(_tail, this)); }
		[[nodiscard]] const_reverse_iterator rend() const noexcept { return const_reverse_iterator(end()); }
		[[nodiscard]] const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(const_iterator(_tail, this)); }
		[[nodiscard]] const_reverse_iterator crend() const noexcept { return const_reverse_iterator(end()); }

		/**
		*	@brief constructs an empty dlist.
		*	@param alloc : allocator to use for all memory allocations of this dlist
		*/
		dlist() { }
		explicit dlist(allocator_type& alloc) : _Base(alloc) { }
		/**
		*   @brief constructs the dlist with the copy of the contents of other.
		*   @param x : other instance of dlists(pass by lvalue).
		*   @param alloc : allocator to use for all memory allocations of this dlist
		*/
		dlist(const dlist& x) : _Base(x) { }
		dlist(const dlist& x, const allocator_type& alloc) : _Base(x, alloc) { }
		dlist(dlist&& x) noexcept : _Base(::std::move(x)){ }
		dlist(dlist&& x, const allocator_type& alloc) noexcept(_Base::_Alnode_traits::is_always_equal::value) : _Base(::std::move(x), alloc) { }
		/**
		*   @brief constructs the dlist with the contents of the range [first, last)
		*   @param first : the beginning of range to insert
		*   @param last : the end of range to insert
		*   @param alloc : allocator to use for all memory allocations of this dlist
		*/
		template <typename _Iter>
		dlist(_Iter first, _Iter last, const allocator_type& alloc = allocator_type()) : _Base(first, last, alloc) { }
		/**
		*   @brief constructs the dlist with n elements each with value value_type()
		*   @param n : number of elements to insert.
		*   @param alloc : allocator to use for all memory allocations of this dlist
		*/
		explicit dlist(size_type n, const allocator_type& alloc = allocator_type()) : _Base(n, alloc) { }
		dlist(size_type n, const_reference value, const allocator_type& alloc = allocator_type()) : _Base(n, value, alloc) { }
		/**
		*   @brief constructs the dlist with the contents of the initializer list init.
		*   @param l : initializer list to insert the values from
		*   @param alloc : allocator to use for all memory allocations of this dlist
		*/
		dlist(::std::initializer_list<_Tp> l, const allocator_type& alloc = allocator_type()) : _Base(l.begin(), l.end(), alloc) { }

		/**
		*	@brief reverses the order of the elements in the dlist.
		*/
		void reverse() noexcept;
		
		~dlist() { _Base::clear(); }

		dlist& operator=(const dlist& rhs) {
			static_cast<_Base&>(*this) = rhs;
			return *this;
		}
		dlist& operator=(dlist&& rhs) noexcept {
			static_cast<_Base&>(*this) = ::std::move(rhs);
			return *this;
		}
		dlist& operator=(::std::initializer_list<_Tp> l) {
			_Base::assign(l.begin(), l.end());
			return *this;
		}
	private:
		link_type _tail; //will be initialized in base class

		inline link_type nil() const noexcept { return nullptr; }
		inline link_type& tail() noexcept { return _tail; }
		inline const link_type tail() const noexcept { return _tail; }

		inline link_type make_link(link_type, link_type, link_type) noexcept;
		inline link_type break_link(link_type, link_type) noexcept;
	};

	template <class _Tp, class _Alloc>
	typename dlist<_Tp, _Alloc>::link_type dlist<_Tp, _Alloc>::make_link(link_type pos, link_type first, link_type last) noexcept {
		link_type& _prev = pos ? pos->_prev : _tail;
		first->_prev = _prev;
		_prev->_next = first;
		last->_next = pos;
		_prev = last;
		return first;
	}

	template <class _Tp, class _Alloc>
	typename dlist<_Tp, _Alloc>::link_type dlist<_Tp, _Alloc>::break_link(link_type first, link_type last) noexcept {
		link_type& _prev = last ? last->_prev : _tail, _old_prev = _prev;
		_prev = first->_prev;
		first->_prev->_next = last;
		return _old_prev;
	}

	template <class _Tp, class _Alloc>
	void dlist<_Tp, _Alloc>::reverse() noexcept {
		if (_size <= 1)
			return;
		link_type _tmp = _head._next, _node;
		while (_tmp != (_node = _tail))
			make_link(_tmp, _node, break_link(_tail, nil()));
	}

	/**
	*	@class ldlist
	*	@brief loop doubly linked list
	*/
	template <class _Tp, class _Alloc = _DEFAULT_ALLOC(_Tp)>
	class ldlist : public _Dlist_base<_Tp, _Alloc, ldlist<_Tp, _Alloc>> {
		using _Base = _Dlist_base<_Tp, _Alloc, ldlist<_Tp, _Alloc>>;
		using _Base::_size;
		using _Base::_head;
		using typename _Base::node;
		using typename _Base::link_type;
	public:
		friend class _Dlist_base<_Tp, _Alloc, ldlist<_Tp, _Alloc>>;
		using const_iterator			=		typename _Base::const_iterator;
		using iterator					=		typename _Base::iterator;
		using const_reverse_iterator	=		::std::reverse_iterator<const_iterator>;
		using reverse_iterator			=		::std::reverse_iterator<iterator>;
		using value_type				=		_Tp;
		using difference_type			=		typename _Base::difference_type;
		using size_type					=		typename _Base::size_type;
		using reference					=		_Tp&;
		using const_reference			=		const _Tp&;
		using pointer					=		typename _Base::pointer;
		using const_pointer				=		typename _Base::const_pointer;
		using allocator_type			=		typename _Base::allocator_type;

		/**
		*   @return a reference to the last element in the ldlist.
		*/
		[[nodiscard]] reference back() noexcept {
#if !defined(_NO_DLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
			assert(("back() called on empty list", _size >= 0));
#endif
			return VALUE(_head._prev); 
		}
		[[nodiscard]] const_reference back() const noexcept { 
#if !defined(_NO_DLIST_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
			assert(("back() called on empty list", _size >= 0));
#endif
			return VALUE(_head._prev); 
		}

		using _Base::begin;
		[[nodiscard]] reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
		[[nodiscard]] reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
		[[nodiscard]] const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
		[[nodiscard]] const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
		[[nodiscard]] const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
		[[nodiscard]] const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

		/**
		*	@brief constructs an empty ldlist.
		*   @param alloc : allocator to use for all memory allocations of this ldlist
		*/
		ldlist() = default;
		explicit ldlist(allocator_type& alloc) : _Base(alloc) { }
		/**
		*   @brief constructs the ldlist with the copy of the contents of other.
		*   @param x : other instance of ldlists(pass by lvalue).
		*   @param alloc : allocator to use for all memory allocations of this ldlist
		*/
		ldlist(const ldlist& x) : _Base(x) { }
		ldlist(const ldlist& x, const allocator_type& alloc) : _Base(x, alloc) { }
		ldlist(ldlist&& x) noexcept : _Base(::std::move(x)) { }
		ldlist(ldlist&& x, const allocator_type& alloc) noexcept(_Base::_Alnode_traits::is_always_equal::value) : _Base(::std::move(x), alloc) { }
		/**
		*   @brief constructs the ldlist with the contents of the range [first, last)
		*   @param first : the beginning of range to insert
		*   @param last : the end of range to insert
		*   @param alloc : allocator to use for all memory allocations of this ldlist
		*/
		template <typename _Iter>
		ldlist(_Iter first, _Iter last, const allocator_type& alloc = allocator_type()) : _Base(first, last, alloc) { }
		/**
		*   @brief constructs the ldlist with n elements each with value value_type()
		*   @param n : number of elements to insert.
		*   @param alloc : allocator to use for all memory allocations of this ldlist
		*/
		explicit ldlist(size_type n, const allocator_type& alloc = allocator_type()) : _Base(n, alloc) { }
		ldlist(size_type n, const_reference value, const allocator_type& alloc = allocator_type()) : _Base(n, value, alloc) { }
		/**
		*   @brief constructs the ldlist with the contents of the initializer list init.
		*   @param l : initializer list to insert the values from
		*   @param alloc : allocator to use for all memory allocations of this ldlist
		*/
		ldlist(::std::initializer_list<_Tp> l, const allocator_type& alloc = allocator_type()) : _Base(l.begin(), l.end(), alloc) { }
		/**
		*	@brief reverses the order of the elements in the ldlist.
		*/
		void reverse() noexcept;

		~ldlist() { _Base::clear(); }

		ldlist& operator=(const ldlist& rhs) = default;
		ldlist& operator=(ldlist&& rhs) = default;
		ldlist& operator=(::std::initializer_list<_Tp> l) {
			_Base::assign(l.begin(), l.end());
			return *this;
		}

	private:
		inline link_type nil() noexcept { return ::std::addressof(_head); }
		inline const node* nil() const noexcept { return ::std::addressof(_head); }
		inline link_type& tail() noexcept { return _head._prev; }
		inline const link_type& tail() const noexcept { return _head._prev; }

		inline link_type make_link(link_type, link_type, link_type) noexcept;
		inline link_type break_link(link_type, link_type) noexcept;
	};

	template <class _Tp, class _Alloc>
	typename ldlist<_Tp, _Alloc>::link_type ldlist<_Tp, _Alloc>::make_link(link_type pos, link_type first, link_type last) noexcept {
		first->_prev = pos->_prev;
		pos->_prev->_next = first;
		last->_next = pos;
		pos->_prev = last;
		return first;
	}

	template <class _Tp, class _Alloc>
	typename ldlist<_Tp, _Alloc>::link_type ldlist<_Tp, _Alloc>::break_link(link_type first, link_type last) noexcept {
		link_type _old_prev = last->_prev;
		first->_prev->_next = last;
		last->_prev = first->_prev;
		return _old_prev;
	}

	template <class _Tp, class _Alloc>
	void ldlist<_Tp, _Alloc>::reverse() noexcept {
		if (_size <= 1)
			return;
		link_type _tmp = ::std::addressof(_head);
		do {
			::std::swap(_tmp->_next, _tmp->_prev);
			_tmp = _tmp->_prev;
		} while (_tmp != ::std::addressof(_head));
	}

#define _DLIST_DEDUCTION_GUIDES(LIST)\
	template <class _Tp, class _Alloc>\
	LIST(const LIST<_Tp, _Alloc>&, const _Alloc & = _Alloc())->LIST <_Tp, _Alloc>;\
	template <class _Tp, class _Alloc>\
	LIST(LIST<_Tp, _Alloc>&&, const _Alloc & = _Alloc())->LIST <_Tp, _Alloc>;\
	template <class _Iter, class _Alloc>\
	LIST(_Iter, _Iter, const _Alloc & = _Alloc())->LIST <typename ::std::iterator_traits<_Iter>::value_type, _Alloc>;\
	template <class _Tp, class _Alloc>\
	LIST(size_t, const _Tp&, const _Alloc & = _Alloc())->LIST <_Tp, _Alloc>;\
	template <class _Tp, class _Alloc>\
	LIST(::std::initializer_list<_Tp>, const _Alloc & = _Alloc())->LIST <_Tp, _Alloc>;

	_DLIST_DEDUCTION_GUIDES(dlist);
	_DLIST_DEDUCTION_GUIDES(ldlist);
}// namespace xstl

namespace std {
	template <class _Tp, class _Alloc>
	void swap(xstl::dlist<_Tp, _Alloc>& x, xstl::dlist<_Tp, _Alloc>& y) noexcept {
		x.swap(y);
	}

	template <class _Tp, class _Alloc>
	void swap(xstl::ldlist<_Tp, _Alloc>& x, xstl::ldlist<_Tp, _Alloc>& y) noexcept {
		x.swap(y);
	}
}// namespace std
#undef _DLIST_DEDUCTION_GUIDES
#endif