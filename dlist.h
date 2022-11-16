#ifndef _DLIST_H_
#define _DLIST_H_

#include <stdexcept>
#include <algorithm>
#include <functional>
#include <deque>
#include <iterator>

namespace Algs {
		template <typename _Tp>
	struct _dlist_node {
		_Tp _value;
		_dlist_node *_prev;
		_dlist_node *_next;

		_dlist_node() :_next(nullptr) {}
		explicit _dlist_node(const _Tp & value, _dlist_node * prev = nullptr, _dlist_node *next = nullptr)
			: _value(value), _prev(prev), _next(next) { }

	}; // template <typename _Tp> struct _dlist_node;
	
	template <class _Tp, class _Ptr, class _Ref>
	struct _dlist_iter_base {
	    typedef size_t size_type;
	    typedef _Tp value_type;
		typedef _Tp* pointer;
		typedef const _Tp* const_pointer;
		typedef _Tp& reference;
		typedef const _Tp& const_reference;
		typedef ptrdiff_t difference_type;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef _dlist_node<_Tp> node;
		typedef node* node_ptr;

	    explicit _dlist_iter_base() : _node(nullptr) { }
		explicit _dlist_iter_base(node_ptr _Node) : _node(_Node) { }
		explicit _dlist_iter_base(const_reference _Value) : _node(new node(_Value)) { }
		
		inline void _incr() {
		    if (_node == nullptr)
				throw std::out_of_range("Out of range");
			_node = _node->_next;
		}
		
		inline void _decr() {
		    if (_node == nullptr)
				throw std::out_of_range("Out of range");
			_node = _node->_prev;
		}
		reference operator*() const { return (*_node)._value; }
		pointer operator->() const { return &(operator*()); }
		bool operator==(_dlist_iter_base &rhs) const { return rhs._node == _node; }
		bool operator!=(_dlist_iter_base &rhs) const {
			return !(rhs._node == _node);
		}
		difference_type operator-(_dlist_iter_base it) {
			difference_type diff;
			for (diff = 0; it._node && _node != it._node; ++diff, ++it)
			if (it._node == nullptr && _node)
				return ~diff + 1;
			return diff;
		}

		operator node_ptr() {
			return _node;
		}

		node_ptr _node;
	}; // template <class _Tp, class _Ptr, class _Ref> struct _dlist_iter_base;
	
	template <class _Tp, class _Ptr, class _Ref>
	struct _dlist_riter;

	template <class _Tp, class _Ptr, class _Ref>
	struct _dlist_iter :public _dlist_iter_base <_Tp, _Ptr, _Ref> {
		typedef _dlist_iter <_Tp, _Tp*, _Tp&>   iterator;
		typedef _dlist_iter <_Tp, _Ptr, _Ref >  _Self;
		typedef _dlist_iter_base<_Tp, _Ptr, _Ref> _Base;
		explicit _dlist_iter() : _Base() { }
		explicit _dlist_iter(typename _Base::node_ptr _Node) : _Base(_Node) { }
		explicit _dlist_iter(typename _Base::const_reference _Value) : _Base(_Value) { }
		_dlist_iter(_dlist_riter<_Tp, _Ptr, _Ref> i) : _Base::_node(i._node) { }

		_Self &operator++() {
		    _Base::_incr();
			return *this;
		}

		_Self operator++(int) {
			iterator _Tmp = *this;
			++*this;
			return _Tmp;
		}

		_Self &operator--() {
			_Base::_decr();
			return *this;
		}

		_Self operator--(int) {
			iterator _Tmp = *this;
			--*this;
			return _Tmp;
		}
	}; // template <class _Tp, class _Ptr, class _Ref> struct _dlist_iter;
	
	
	template <class _Tp, class _Ptr, class _Ref>
	struct _dlist_riter : public _dlist_iter_base<_Tp, _Ptr, _Ref> {
		typedef _dlist_riter <_Tp, _Tp*, _Tp&>   reverse_iterator;
		typedef _dlist_riter <_Tp, _Ptr, _Ref >  _Self;
		typedef _dlist_iter_base<_Tp, _Ptr, _Ref> _Base;
		
		explicit _dlist_riter() : _Base() { }
		explicit _dlist_riter(typename _Base::node_ptr _Node) : _Base(_Node) { }
		explicit _dlist_riter(typename _Base::const_reference _Value) : _Base(_Value) { }
		explicit
			_dlist_riter(_dlist_iter<_Tp, _Ptr, _Ref> i) : _Base::_node(i._node) { }

		_Self &operator++() {
			_Base::_decr();
			return *this;
		}

		_Self operator++(int) {
			reverse_iterator _Tmp = *this;
			++*this;
			return _Tmp;
		}

		_Self &operator--() {
			_Base::_incr();
			return *this;
		}

		_Self operator--(int) {
			reverse_iterator _Tmp = *this;
			--*this;
			return _Tmp;
		}
	}; // template <class _Tp, class _Ptr, class _Ref> struct _dlist_riter;

	template <typename _Tp>
	void destroy(_dlist_node<_Tp>* _Head, _dlist_node<_Tp>* _Tail = nullptr) {
		_dlist_node<_Tp>* _Tmp;
		while (_Head != _Tail) {
			_Tmp = _Head;
			_Head = _Head->_next;
			delete _Tmp;
		}
		_Tmp = nullptr;
	}

	template <class _Tp>
	class dlist
	{
	public:
		typedef _dlist_iter <_Tp, _Tp*, _Tp&> iterator;
		typedef _dlist_iter <_Tp, const _Tp*, const _Tp&> const_iterator;
		typedef _dlist_riter <_Tp, _Tp*, _Tp&> reverse_iterator;
		typedef _dlist_iter <_Tp, const _Tp*, const _Tp&> const_reverse_iterator;
		typedef typename iterator::node node;
		typedef _Tp value_type;
		typedef ptrdiff_t difference_type;
		typedef size_t size_type;
		typedef typename iterator::node_ptr node_ptr;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;

		inline reference   front() { return _head._next->_value; }
		inline const_reference front()  const { return _head._next->_value; }
		inline reference     back() { return _tail->_value; }
		inline const_reference back()   const { return _tail->_value; }
		inline iterator       begin() { return iterator(_head._next); }
		inline iterator       end() { return iterator(nullptr); }
		inline const_iterator begin()  const { return const_iterator(_head._next); }
		inline const_iterator end()    const { return const_iterator(nullptr); }
		inline const_iterator cbegin() const { return const_iterator(_head._next); }
		inline const_iterator cend()   const { return const_iterator(nullptr); }
		inline reverse_iterator rbegin() { return reverse_iterator(_tail); }
		inline const_reverse_iterator rbegin() const { return const_reverse_iterator(_tail); }
		inline const_reverse_iterator crbegin() const { return const_reverse_iterator(_tail); }
		inline reverse_iterator rend() { return reverse_iterator(nullptr); }
		inline const_reverse_iterator rend() const { return const_reverse_iterator(nullptr); }
		inline const_reverse_iterator crend() const { return const_reverse_iterator(nullptr); }

		dlist() : _tail(&_head), _ptr(nullptr) { }

		explicit dlist(const_reference _Value)
			:_tail(&_head), _ptr(nullptr) { push_front(_Value); }

		dlist(const dlist &x) :_tail(&_head), _ptr(nullptr) {
			insert(begin(), x.begin(), x.end());
		}

		template <typename _Ty> dlist(_Ty _First, _Ty _Last) : _tail(&_head), _ptr(nullptr) {
			insert(begin(), _First, _Last);
		}

		dlist(int n, const_reference _Value) : _tail(&_head), _ptr(nullptr) {
			insert(begin(), n, _Value);
		}

		dlist(dlist &&x) : _tail(&_head), _ptr(nullptr) {
			*this = x;
		}

		dlist(std::initializer_list<_Tp> l) : _tail(&_head), _ptr(nullptr) {
			insert(begin(), l);
		}

		inline void insert(iterator _Pos, const_reference _Value);
		inline void insert(iterator _Pos, node_ptr _Node);
		inline void insert(iterator _Pos, iterator _It);
		inline void insert(iterator _Pos, int n, const_reference _Value);
		inline void insert(iterator _Pos, std::initializer_list<_Tp> l);
		template <class _Ty>
		inline void insert(iterator _Pos, _Ty _First, _Ty _Last);

		void splice(iterator _Pos, dlist &x);
		void splice(iterator _Pos, dlist &x, iterator i);
		void splice(iterator _Pos, dlist &x, iterator _First, iterator _Last);

		template <class _Pr2>
		void push_if(const_reference _Value, _Pr2 _Pred);
		void push_front(const_reference _Value);
		void push_back(const_reference _Value);

		void pop(const_reference _Value);
		template <class _Pr1>
		void pop_if(_Pr1 _Pred);
		void pop_front();
		void pop_front(int n);
		void pop_back();
		void pop_back(int n);

		inline void erase(iterator i);
		inline void erase(iterator _First, iterator _Last);

		void remove(const_reference _Value);

		template <class _Pr1>
		void remove_if(_Pr1 _Pred);

		template <class _Cmp>
		void sort(_Cmp cmp);

		void merge(dlist &x);
		void swap(dlist &x);
		void unique();
		void reverse();
		iterator find(const_reference _Value) const;
		difference_type size();

		void clear();
		bool empty() const;

		~dlist();

		dlist<_Tp> &operator=(const dlist &rhs);
		dlist<_Tp> &operator=(dlist &&rhs);
	private:
		node _head;
		node_ptr _ptr;
		node_ptr _tail;
		
		inline void __make_link(iterator, iterator, iterator);
		inline void __make_link(iterator, node_ptr, node_ptr);
		inline iterator __break_link(dlist&, iterator, iterator);
		inline node_ptr __break_link(dlist&, node_ptr, node_ptr);
	};

	template <typename _Tp>
	void dlist<_Tp>::__make_link(iterator _Prev, iterator _First, iterator _Last) {
	    __make_link(_Prev, _First._node, _Last._node);
	}

	template <typename _Tp>
	void dlist<_Tp>::__make_link(iterator _Prev, node_ptr _First, node_ptr _Last) {
	    _ptr = _Prev._node ? _Prev._node->_prev : _tail;
	    _Last->_next = _Prev;
	    if (_Prev._node)
	        _Prev._node->_prev = _Last;
		_ptr->_next = _First;
		_First->_prev = _ptr;
		if (_Prev._node == nullptr)
		    _tail = _Last;
	}

	template <typename _Tp>
	typename dlist<_Tp>::iterator dlist<_Tp>::__break_link(dlist &x, iterator _First, iterator _Last) {
		return iterator(__break_link(x, _First._node, _Last._node));
	}
	
	template <typename _Tp>
	typename dlist<_Tp>::node_ptr dlist<_Tp>::__break_link(dlist &x, node_ptr _First, node_ptr _Last) {
	    _ptr = _Last ? _Last->_prev : x._tail;
		_First->_prev->_next = _Last;
		(_Last ? _Last->_prev : x._tail) = _First->_prev;
		return _ptr;
	}
	
	template <typename _Tp>
	void dlist<_Tp>::insert(iterator _Pos, const_reference _Value) {
		insert(_Pos, new node(_Value));
	}

	template <typename _Tp>
	void dlist<_Tp>::insert(iterator _Pos, iterator _It) {
		insert(_Pos, new node(*_It));
	}

	template <typename _Tp>
	void dlist<_Tp>::insert(iterator _Pos, node_ptr _Node) {
		__make_link(_Pos, _Node, _Node);
	}

	template <typename _Tp>
	void dlist<_Tp>::insert(iterator _Pos, int n, const_reference _Value) {
		if (n <= 0)
			return;
		node_ptr _New = new node(_Value);
		for (_ptr = _New; --n; _New = _New->_next)
			_New->_next = new node(_Value, _New);
		__make_link(_Pos, _ptr, _New);
	}

	template <typename _Tp>
	void dlist<_Tp>::insert(iterator _Pos, std::initializer_list<_Tp> l) {
		insert(_Pos, l.begin(), l.end());
	}

	template <typename _Tp>
	template <typename _Ty>
	void dlist<_Tp>::insert(iterator _Pos, _Ty _First, _Ty _Last) {
		if (_First == _Last)
			return;
		_Ty i = _First;
		node_ptr _New = new node(*i);
		for (_ptr = _New; ++i != _Last; _New = _New->_next)
			_New->_next = new node(*i, _New);
		__make_link(_Pos, _ptr, _New);
	}

	template <typename _Tp>
	void dlist<_Tp>::splice(iterator _Pos, dlist &x) {
		splice(_Pos, x, x.begin(), x.end());
	}

	template <typename _Tp>
	void dlist<_Tp>::splice(iterator _Pos, dlist &x, iterator i) {
		if (i)
			splice(_Pos, x, i++, i);
	}

	template <typename _Tp>
	void dlist<_Tp>::splice(iterator _Pos, dlist &x, iterator _First, iterator _Last) {
		if (_First != _Last)
			__make_link(_Pos, _First, __break_link(x, _First, _Last));
	}

	template <typename _Tp>
	template <class _Pr2>
	void dlist<_Tp>::push_if(const_reference _Value, _Pr2 _Pred) {
		for (_ptr = &_head; _ptr->_next && _Pred(_Value, _ptr->_next->_value); _ptr = _ptr->_next);
		insert(iterator(_ptr), new node(_Value));
	}

	template <typename _Tp>
	void dlist<_Tp>::push_front(const_reference _Value) {
		insert(begin(), new node(_Value));
	}

	template <typename _Tp>
	void dlist<_Tp>::push_back(const_reference _Value) {
		insert(end(), new node(_Value));
	}
	
	template <typename _Tp>
	void dlist<_Tp>::pop(const_reference _Value) {
	    pop_if([&](const_reference value)->bool{ return value == _Value; });
	}
	
	template <typename _Tp>
	template <class _Pr1>
	void dlist<_Tp>::pop_if(_Pr1 _Pred) {
		if (empty())
			return;
		iterator _It(begin());
		for (; _It && !_Pred(*_It); ++_It);
		erase(_It);
	}

	template <typename _Tp>
	void dlist<_Tp>::pop_front() {
		erase(begin());
	}

	template <typename _Tp>
	void dlist<_Tp>::pop_front(int n) {
		while (--n)
			pop_front();
	}

	template <typename _Tp>
	void dlist<_Tp>::pop_back() {
		erase(iterator(_tail));
	}

	template <typename _Tp>
	void dlist<_Tp>::pop_back(int n) {
		while (--n)
			pop_back();
	}

	template <typename _Tp>
	void dlist<_Tp>::erase(iterator i) {
		if (i)
			erase(i++, i);
	}

	template <typename _Tp>
	void dlist<_Tp>::erase(iterator _First, iterator _Last) {
		if (_First == _Last || empty())
			return;
		__break_link(*this, _First, _Last);
		destroy(_First._node, _Last._node);
	}

	template <typename _Tp>
	void dlist<_Tp>::remove(const_reference _Value) {
		remove_if([&](node_ptr _Node)->bool {return _Node->_value == _Value; });
	}

	template <typename _Tp>
	template <typename _Pr1>
	void dlist<_Tp>::remove_if(_Pr1 _Pred) {
		if (empty())
			return;
		node_ptr *_Link;
		for (_Link = &_head._next; *_Link;) {
			_ptr = *_Link;
			if (_Pred((*_Link))->_value) {
				*_Link = _ptr->_next;
				delete _ptr;
			}
			else
				_Link = &_ptr->_next;
		}
	}

	template <typename _Tp>
	template <class _Cmp>
	void dlist<_Tp>::sort(_Cmp cmp) {
		std::deque<_Tp> _Tmp;
		for (auto &i : *this)
			_Tmp.push_back(i);
		std::sort(_Tmp.begin(), _Tmp.end(), cmp);
		for (auto &i : *this) {
			i = _Tmp.front();
			_Tmp.pop_front();
		}
	}

	template <typename _Tp>
	void dlist<_Tp>::reverse() {
		using std::swap;
		node_ptr _Last(_tail);
		_ptr = _head._next;
		for (; _ptr != _Last && _ptr->_next != _Last; _ptr = _ptr->_next, _Last = _Last->_prev)
			swap(_ptr->_value, _Last->_value);
	}

	template <typename _Tp>
	void dlist<_Tp>::merge(dlist &x) {
		if (empty())
			*this = std::move(x);
		iterator _It(begin());
		while (!x.empty() && _It)
			if (*_It > x._head._next->_value)
				splice(_It, x, x.begin());
			else
				++_It;
		if (!x.empty())
			splice(end(), x);
	}

	template <typename _Tp>
	void dlist<_Tp>::swap(dlist<_Tp> &x) {
		using std::swap;
		swap(_head._next, x._head._next);
		swap(_tail, x._tail);
	}

	template <typename _Tp>
	void dlist<_Tp>::unique() {
		iterator _It(begin()), _Next(_It);
		while (++_Next) {
			if (*_It == *_Next)
				erase(_Next);
			else
				_It = _Next;
			_Next = _It;
		}
	}

	template <typename _Tp>
	typename dlist<_Tp>::iterator dlist<_Tp>::find(const_reference _Value) const {
		iterator _It(begin());
		for (; _It && *_It != _Value; ++_It);
		return _It;
	}

	template <typename _Tp>
	typename dlist<_Tp>::difference_type dlist<_Tp>::size() {
		return end() - begin();
	}

	template <typename _Tp>
	dlist<_Tp>::~dlist() {
		clear();
	}

	template <typename _Tp>
	dlist<_Tp> &dlist<_Tp>::operator=(const dlist<_Tp> &rhs)
	{
		if (this == &rhs)
		    return *this;
		iterator _It_1(begin()), _It_2(rhs.begin());
		for (; _It_1 && _It_2; ++_It_1, ++_It_2)
		    *_It_1 = *_It_2;
		if (_It_1)
		    destroy(_It_1._node);
		else
		    insert(end(), _It_2, rhs.end());
		return *this;
	}

	template <typename _Tp>
	dlist<_Tp> &dlist<_Tp>::operator=(dlist<_Tp> &&rhs) {
		destroy(_head._next);
		_head = rhs._head;
		_tail = rhs._tail;
		rhs._tail = nullptr;
		return *this;
	}

	template <typename _Tp>
	bool dlist<_Tp>::empty() const {
		return _head._next == nullptr;
	}

	template <typename _Tp>
	void dlist<_Tp>::clear() {
		destroy(_head._next);
		_tail = _ptr = nullptr;
	}

	template <typename _Tp>
	void swap(dlist<_Tp> &lhs, dlist<_Tp> &rhs)
	{
		lhs.swap(rhs);
	}

	template <class _Tp>
	class ldlist
	{
	public:
		typedef _dlist_iter <_Tp, _Tp*, _Tp&> iterator;
		typedef _dlist_iter <_Tp, const _Tp*, const _Tp&> const_iterator;
		typedef _dlist_riter <_Tp, _Tp*, _Tp&> reverse_iterator;
		typedef _dlist_iter <_Tp, const _Tp*, const _Tp&> const_reverse_iterator;
		typedef typename iterator::node node;
		typedef _Tp value_type;
		typedef ptrdiff_t difference_type;
		typedef size_t size_type;
		typedef typename iterator::node_ptr node_ptr;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;

		inline reference   front() { return _start._next->_value; }
		inline const_reference front()  const { return _start._next->_value(); }
		inline reference     back() { return _start._prev->_value; }
		inline const_reference back()   const { return _start._prev->_value; }
		inline iterator       begin() { return iterator(_start._next); }
		inline iterator       end() { return iterator(&_start); }
		inline const_iterator begin()  const { return const_iterator(_start._next); }
		inline const_iterator end()    const { return const_iterator(&_start); }
		inline const_iterator cbegin() const { return const_iterator(_start._next); }
		inline const_iterator cend()   const { return const_iterator(&_start); }
		inline reverse_iterator rbegin() { return reverse_iterator(_start._prev); }
		inline const_reverse_iterator rbegin() const { return const_reverse_iterator(_start.prev); }
		inline const_reverse_iterator crbegin() const { return const_reverse_iterator(_start.prev); }
		inline reverse_iterator rend() { return reverse_iterator(&_start); }
		inline const_reverse_iterator rend() const { return const_reverse_iterator(&_start); }
		inline const_reverse_iterator crend() const { return const_reverse_iterator(&_start); }

		ldlist() : _ptr(nullptr) {
			_start._next = _start._prev = &_start;
		}

		explicit ldlist(const_reference _Value) : _ptr(_start) {
			_start._next = _start._prev = &_start;
			push_back(_Value);
		}

		ldlist(const ldlist &x) :_ptr(nullptr) {
			_start._next = _start._prev = &_start;
			insert(begin(), x.begin(), x.end());
		}

		template <typename _Ty> ldlist(_Ty _First, _Ty _Last) : _ptr(nullptr) {
		    _start._next = _start._prev = &_start;
			insert(begin(), _First, _Last);
		}

		ldlist(int n, const_reference _Value) : _ptr(nullptr) {
			_start._next = _start._prev = &_start;
			insert(begin(), n, _Value);
		}

		ldlist(std::initializer_list<_Tp> l) : _ptr(nullptr) {
			_start._next = _start._prev = &_start;
			insert(begin(), l);
		}

		ldlist(ldlist &&x) : _ptr(nullptr) {
			_start._next = _start._prev = &_start;
			*this = x;
		}

		inline void insert(iterator _Pos, const_reference _Value);
		inline void insert(iterator _Pos, node_ptr _Node);
		inline void insert(iterator _Pos, iterator it);
		inline void insert(iterator _Pos, int n, const_reference _Value);
		inline void insert(iterator _Pos, std::initializer_list<_Tp> l);
		template <class _Ty>
		inline void insert(iterator _Pos, _Ty _First, _Ty _Last);

		void splice(iterator _Pos, ldlist &x);
		void splice(iterator _Pos, ldlist &x, iterator i);
		void splice(iterator _Pos, ldlist &x, iterator _First, iterator _Last);

		template <class _Pr2>
		void push_if(const_reference _Value, _Pr2 _Pred);
		void push_front(const_reference _Value);
		void push_back(const_reference _Value);

		void pop(const_reference _Value);
		template <class _Pr1>
		void pop_if(_Pr1 _Pred);
		void pop_front();
		void pop_front(int n);
		void pop_back();
		void pop_back(int n);

		inline void erase(iterator i);
		inline void erase(iterator _First, iterator _Last);

		void remove(const_reference _Value);

		template <class _Pr1>
		void remove_if(_Pr1 _Pred);

		template <class _Cmp>
		void sort(_Cmp cmp);

		void merge(ldlist &x);
		void swap(ldlist &x);
		void unique();
		void reverse();
		iterator find(const_reference _Value) const;
		difference_type size() const;

		void clear();
		bool empty() const;

		~ldlist();

		ldlist<_Tp> &operator=(const ldlist &rhs);
		ldlist<_Tp> &operator=(ldlist &&rhs);
	private:
		node _start;
		node_ptr _ptr;

		inline void __make_link(iterator, iterator, iterator);
		inline void __make_link(iterator, node_ptr, node_ptr);
		inline iterator __break_link(iterator, iterator);
		inline node_ptr __break_link(node_ptr, node_ptr);
		
	};

	template <typename _Tp>
	void ldlist<_Tp>::__make_link(iterator _Prev, iterator _First, iterator _Last) {
		__make_link(_Prev, _First._node, _Last._node);
	}

	template <typename _Tp>
	void ldlist<_Tp>::__make_link(iterator _Prev, node_ptr _First, node_ptr _Last) {
	    _ptr = _Prev._node->_prev;
		_Last->_next = _ptr->_next;
		_Prev._node->_prev = _Last;
		_ptr->_next = _First;
		_First->_prev = _ptr;
	}
	
	template <typename _Tp>
	typename ldlist<_Tp>::iterator ldlist<_Tp>::__break_link(iterator _First, iterator _Last) {
		return iterator(__break_link(_First._node, _Last._node));
	}
	
	template <typename _Tp>
	typename ldlist<_Tp>::node_ptr ldlist<_Tp>::__break_link(node_ptr _First, node_ptr _Last) {
	    _ptr = _Last->_prev;
	    _First->_prev->_next = _Last;
	    _Last->_prev = _First->_prev;
	    return _ptr;
	}
	
	template <typename _Tp>
	void ldlist<_Tp>::insert(iterator _Pos, const_reference _Value) {
		insert(_Pos, new node(_Value));
	}

	template <typename _Tp>
	void ldlist<_Tp>::insert(iterator _Pos, iterator _It) {
		insert(_Pos, new node(*_It));
	}

	template <typename _Tp>
	void ldlist<_Tp>::insert(iterator _Pos, node_ptr _Node) {
	    __make_link(_Pos, _Node, _Node);
	}

	template <typename _Tp>
	void ldlist<_Tp>::insert(iterator _Pos, int n, const_reference _Value) {
		if (n <= 0)
			return;
		node_ptr _New = new node(_Value, _Pos._node);
		--n;
		for (_ptr = _New; --n; _New = _New->_next)
			_New->_next = new node(_Value, _New);
		__make_link(_Pos, _ptr, _New);
	}

	template <typename _Tp>
	void ldlist<_Tp>::insert(iterator _Pos, std::initializer_list<_Tp> l) {
		insert(_Pos, l.begin(), l.end());
	}

	template <typename _Tp>
	template <class _Ty>
	void ldlist<_Tp>::insert(iterator _Pos, _Ty _First, _Ty _Last) {
		if (_First == _Last)
			return;
		_Ty i = _First;
		node_ptr _New = new node(*i, _Pos._node);
		for (_ptr = _New; ++i != _Last; _New = _New->_next)
			_New->_next = new node(*i, _New);
		__make_link(_Pos, _ptr, _New);
	}

	template <typename _Tp>
	void ldlist<_Tp>::splice(iterator _Pos, ldlist &x) {
		splice(_Pos, x, x.begin(), x.end());
	}

	template <typename _Tp>
	void ldlist<_Tp>::splice(iterator _Pos, ldlist &x, iterator i) {
		splice(_Pos, x, i++, i);
	}

	template <typename _Tp>
	void ldlist<_Tp>::splice(iterator _Pos, ldlist &x, iterator _First, iterator _Last) {
		if (_First != _Last)
			__make_link(_Pos, _First, __break_link(_First, _Last));
	}

	template <typename _Tp>
	template <class _Pr2>
	void ldlist<_Tp>::push_if(const_reference _Value, _Pr2 _Pred) {
	    iterator _It(begin());
		for (; _It._node != &_start && _Pred(*_It, _Value); ++_It);
		insert(_It, _Value);
	}

	template <typename _Tp>
	void ldlist<_Tp>::push_front(const_reference _Value) {
		insert(begin(), _Value);
	}

	template <typename _Tp>
	void ldlist<_Tp>::push_back(const_reference _Value) {
		insert(end(), _Value);
	}

	template <typename _Tp>
	void ldlist<_Tp>::pop(const_reference _Value) {
		pop_if([&](const_reference value)->bool{ return value == _Value; });
	}
	
	template <typename _Tp>
	template <class _Pr1>
	void ldlist<_Tp>::pop_if(_Pr1 _Pred) {
		iterator _It(begin());
		for (; _It._node != &_start && !_Pred(*_It); ++_It);
		erase(_It);
	}
	
	template <typename _Tp>
	void ldlist<_Tp>::pop_front() {
		erase(begin());
	}

	template <typename _Tp>
	void ldlist<_Tp>::pop_front(int n) {
		while (--n)
			pop_front();
	}

	template <typename _Tp>
	void ldlist<_Tp>::pop_back() {
		erase(iterator(_start._prev));
	}

	template <typename _Tp>
	void ldlist<_Tp>::pop_back(int n) {
		while (--n)
			pop_back();
	}

	template <typename _Tp>
	void ldlist<_Tp>::erase(iterator _Pos) {
		erase(_Pos++, _Pos);
	}

	template <typename _Tp>
	void ldlist<_Tp>::erase(iterator _First, iterator _Last) {
		if (!empty())
		    destroy(_First._node, __break_link(_First, _Last)._node->_next);
	}

	template <typename _Tp>
	void ldlist<_Tp>::remove(const_reference _Value) {
		remove_if([&](node_ptr _Node)->bool { return _Node->_value == _Value; });
	}

	template <typename _Tp>
	template <class _Pr1>
	void ldlist<_Tp>::remove_if(_Pr1 _Pred) {
		iterator _It(begin()), _Next(_It);
		for (;++_Next != _start;_It = _Next)
			if (_Pred(*_It))
				erase(_It);
	}

	template <typename _Tp>
	void ldlist<_Tp>::unique() {
		iterator _It(begin()), _Next(_It);
		for (;++_Next != _start; _It = _Next)
			if (_It == _Next)
				erase(_It);
	}

	template <typename _Tp>
	template <class _Cmp>
	void ldlist<_Tp>::sort(_Cmp cmp) {
		std::deque<_Tp> _Tmp;
		for (auto &i : *this)
			_Tmp.push_back(i);
		std::sort(_Tmp.begin(), _Tmp.end(), cmp);
		for (auto &i : *this) {
			i = _Tmp.front();
			_Tmp.pop_front();
		}
	}

	template <typename _Tp>
	void ldlist<_Tp>::reverse() {
		using std::swap;
		iterator _It_1(end());
		reverse_iterator _It_2(rbegin());
		for (; _It_1 != _It_2 && ++_It_1 != _It_2; ++_It_2)
			swap(*_It_1, *_It_2);
	}

	template <typename _Tp>
	void ldlist<_Tp>::merge(ldlist &x) {
		if (empty())
			*this = std::move(x);
		iterator _It(begin());
		while (!x.empty() && _It != &_start)
			if (*_It > x._start._next->_value)
				splice(_It, x, x.begin());
			else
				++_It;
		if (!x.empty())
			splice(end(), x);
	}

	template <typename _Tp>
	void ldlist<_Tp>::swap(ldlist &x) {
		using std::swap;
		swap(_start._prev->_next, x._start._prev->_next);
		swap(_start._next, x._start._next);
	}

	template <typename _Tp>
	typename ldlist<_Tp>::iterator ldlist<_Tp>::find(const_reference _Value) const {
		iterator _It(begin());
		for (; _It != _start && *_It != _Value; ++_It);
		return _It;
	}

	template <typename _Tp>
	typename ldlist<_Tp>::difference_type ldlist<_Tp>::size() const {
		difference_type diff = 0;
		iterator _It(begin());
		for (; _It != &_start; ++_It)
			++diff;
		return diff;
	}

	template <typename _Tp>
	void ldlist<_Tp>::clear() {
		erase(begin(), end());
	}

	template <typename _Tp>
	bool ldlist<_Tp>::empty() const {
		return _start._next == &_start;
	}

	template <typename _Tp>
	ldlist<_Tp>::~ldlist() {
		clear();
	}

	template <typename _Tp>
	ldlist<_Tp>& ldlist<_Tp>::operator =(const ldlist &rhs) {
		if (this == &rhs)
		    return *this;
		iterator _It_1(begin()), _It_2(rhs.begin());
		for (; _It_1 != &_start && _It_2 != &rhs._start; ++_It_1, ++_It_2)
		    *_It_1 = *_It_2;
		if (_It_1 != &_start)
		    erase(_It_1, end());
		else
		    insert(end(), _It_2, rhs.end());
		return *this;
	}

	template <typename _Tp>
	ldlist<_Tp>& ldlist<_Tp>::operator =(ldlist &&rhs) {
		clear();
		splice(end(), rhs);
		return *this;
	}
	template <typename _Tp>
	void swap(ldlist<_Tp> &lhs, ldlist<_Tp> &rhs)
	{
		lhs.swap(rhs);
	}
}
#endif