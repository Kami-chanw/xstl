#ifndef _SLIST_H_
#define _SLIST_H_

#include <stdexcept>
#include <algorithm>
#include <functional>
#include <deque>
#include <stack>
#include <iterator>

namespace Algs {
	template <typename _Tp>
	struct _slist_node {
		_Tp _value;
		_slist_node *_next;

		_slist_node() :_next(nullptr) { }
		explicit _slist_node(const _Tp & value, _slist_node * next = nullptr)
			: _value(value), _next(next) { }
	}; // template <typename _Tp> struct _slist_node;

	template <class _Tp, class _Ptr, class _Ref>
	struct _slist_iter {
		typedef _slist_iter <_Tp, _Tp*, _Tp&> iterator;
		typedef _slist_iter <_Tp, _Ptr, _Ref > _Self;
		typedef size_t size_type;
		typedef _Tp value_type;
		typedef _Ptr pointer;
		typedef const _Ptr const_pointer;
		typedef _Ref reference;
		typedef const _Ref const_reference;
		typedef ptrdiff_t difference_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef _slist_node<_Tp> node;
		typedef node* node_ptr;

		explicit _slist_iter() : _node(nullptr) { }
		explicit _slist_iter(node_ptr _Node) : _node(_Node) { }
		explicit _slist_iter(const _Tp &_Value) : _node(new node(_Value)) { }

		reference operator*() const { return (*_node)._value; }
		pointer operator->() const { return &(operator*()); }
		bool operator==(_Self &rhs) const { return rhs._node == _node; }
		bool operator!=(_Self &rhs) const {
			return !(rhs._node == _node);
		}

		_Self &operator++() {
			if (_node == nullptr)
				throw std::out_of_range("Out of range");
			_node = _node->_next;
			return *this;
		}

		_Self operator++(int) {
			iterator _Tmp(*this);
			++*this;
			return _Tmp;
		}

		difference_type operator - (iterator it) {
			difference_type diff;
			for (diff = 0; it && _node != it; ++diff, ++it);
			if (it == nullptr && _node)
				return ~diff + 1;
			return diff;
		}

		operator node_ptr() {
			return _node;
		}

		node_ptr _node;
	}; // template <class _Tp, class _Ptr, class _Ref> struct _slist_iter;

	template <typename _Tp>
	void destroy(_slist_node<_Tp>* _Head, _slist_node<_Tp>* _Tail = nullptr) {
		_slist_node<_Tp>* _Tmp;
		while (_Head != _Tail) {
			_Tmp = _Head;
			_Head = _Head->_next;
			delete _Tmp;
		}
		_Tmp = nullptr;
	}

	template <typename _Tp>
	class slist {
	public:
		typedef _slist_iter <_Tp, _Tp*, _Tp&> iterator;
		typedef _slist_iter <_Tp, const _Tp*, const _Tp&> const_iterator;
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

		slist() : _tail(nullptr), _ptr(nullptr) { }

		explicit slist(const_reference _Value)
			: _tail(_head), _ptr(_head) { _head._next = new node(_Value); }

		slist(const slist &x) : _tail(nullptr), _ptr(nullptr) {
			insert(begin(), x.begin(), x.end());
		}

		template <typename _Ty> slist(_Ty _First, _Ty _Last) : _tail(nullptr), _ptr(nullptr) {
			insert(begin(), _First, _Last);
		}

		slist(int n, const_reference _Value) : _tail(nullptr), _ptr(nullptr) {
			insert(begin(), n, _Value);
		}

		slist(slist &&x) : _tail(nullptr), _ptr(nullptr) {
			*this = x;
		}
		
		slist(std::initializer_list<_Tp> l) :_tail(nullptr), _ptr(nullptr) {
		    insert(begin(), l);
		}

		void insert(iterator _Pos, const_reference _Value);
		void insert(iterator _Pos, node_ptr _Node);
		void insert(iterator _Pos, iterator _It);
		void insert(iterator _Pos, std::initializer_list<_Tp> l);
		void insert(iterator _Pos, int n, const_reference _Value);
		template <class _Ty>
		void insert(iterator _Pos, _Ty _First, _Ty _Last);

		void splice(iterator _Pos, slist &x);
		void splice(iterator _Pos, slist &x, iterator i);
		inline void splice(iterator _Pos, slist &x, iterator _First, iterator _Last);

		template <class _Pr2>
		void push_if(const_reference _Value, _Pr2 _Pred);
		void push_front(const_reference _Value);

		void push_back(const_reference _Value);

		void pop(const_reference _Value);
		template <class _Pr1> void pop_if(_Pr1 _Pred);
		void pop_front();
		void pop_front(int n);
		void pop_back();
		void pop_back(int n);

		void erase(iterator _Pos);
		inline void erase(iterator _First, iterator _Last);

		void unique();

		void remove(const_reference _Value);

		template <class _Pr1>
		void remove_if(_Pr1 _Pred);

		template <class _Cmp>
		void sort(_Cmp cmp);

		void merge(slist &x);
		void swap(slist &x);
		void reverse();
		iterator find(const_reference _Value) const;
		inline iterator previous(iterator _It);
		difference_type size();

		void clear();
		bool empty() const;
		~slist();

		slist<_Tp> &operator=(const slist &rhs);
		slist<_Tp> &operator=(slist &&rhs);
	private:
		node _head;
		node_ptr _tail;
		node_ptr _ptr;
		
		inline void __make_link(iterator, iterator, iterator);
		inline void __make_link(node_ptr, node_ptr, node_ptr);
		inline iterator __break_link(slist&, iterator, iterator);
		inline node_ptr __break_link(slist&, node_ptr, node_ptr);
	}; // template <typename _Tp> class slist;
	
	template <typename _Tp>
	void slist<_Tp>::__make_link(iterator _Prev, iterator _First, iterator _Last) {
	    __make_link(_Prev._node, _First._node, _Last._node);
	}
	
	template <typename _Tp>
	void slist<_Tp>::__make_link(node_ptr _Prev, node_ptr _First, node_ptr _Last) {
	    if (_tail == _Prev || _head._next == nullptr)
		    _tail = _Last;
	    _Last->_next = _Prev->_next;
	    _Prev->_next = _First;
	}
	
	template <typename _Tp>
	typename slist<_Tp>::iterator slist<_Tp>::__break_link(slist &x, iterator _First, iterator _Last) {
	    return iterator(__break_link(x, _First._node, _Last._node));
	}
	
	template <typename _Tp>
	typename slist<_Tp>::node_ptr slist<_Tp>::__break_link(slist &x, node_ptr _First, node_ptr _Last) {
	    node_ptr _Prev(nullptr);
		for (_ptr = x._head._next; _ptr && _ptr->_next != _Last; _ptr = _ptr->_next)
			if (_ptr->_next == _First)
				_Prev = _ptr;
		_Prev->_next = _Last;
		_ptr->_next = nullptr;
		return _ptr;
	}

	template <typename _Tp>
	void slist<_Tp>::insert(iterator _Pos, const_reference _Value) {
		insert(_Pos, new node(_Value));
	}

	template <typename _Tp>
	void slist<_Tp>::insert(iterator _Pos, node_ptr _Node) {
	    __make_link(previous(_Pos), _Node, _Node);
	}

	template <typename _Tp>
	void slist<_Tp>::insert(iterator _Pos, iterator _It) {
		insert(_Pos, new node(*_It));
	}
	
	template <typename _Tp>
	void slist<_Tp>::insert(iterator _Pos, std::initializer_list<_Tp> l) {
		insert(_Pos, l.begin(), l.end());
	}
	
	template <typename _Tp>
	void slist<_Tp>::insert(iterator _Pos, int n, const_reference _Value) {
		node_ptr _New = new node(_Value);
		--n;
		for (_ptr = _New; --n; _New = _New->_next)
			_New->_next = new node(_Value);
		__make_link(previous(_Pos), _ptr, _New);
	}
	
	template <typename _Tp>
	template <class _Ty>
	void slist<_Tp>::insert(iterator _Pos, _Ty _First, _Ty _Last) {
	    if (_First == _Last)
	       return;
		_Ty i = _First;
		node_ptr _New = new node(*i);
		for (_ptr = _New; ++i != _Last; _New = _New->_next)
			_New->_next = new node(*i);
		__make_link(previous(_Pos), _ptr, _New);
	}

	template <typename _Tp>
	void slist<_Tp>::splice(iterator _Pos, slist &x)
	{
	    splice(_Pos, x, x.begin(), x.end());
	}

	template <typename _Tp>
	void slist<_Tp>::splice(iterator _Pos, slist &x, iterator i)
	{
	    splice(_Pos, x, i++, i);
	}

	template <typename _Tp>
	void slist<_Tp>::splice(iterator _Pos, slist &x, iterator _First, iterator _Last)
	{
		if (x._head._next && _First != _Last && this != &x)
		    __make_link(previous(_Pos), _First, __break_link(x, _First, _Last));
	}

	template <typename _Tp>
	template <class _Pr2>
	void slist <_Tp>::push_if(const_reference _Value, _Pr2 _Pred) {
	    node_ptr _Prev;
		for (_ptr = _head._next; _ptr && _Pred(_ptr->_value, _Value); _ptr = _ptr->_next)
		    _Prev = _ptr;
		if (_ptr == nullptr)
		    return;
		_ptr = new node(_Value);
		__make_link(_Prev, _ptr, _ptr);
	}

	template <typename _Tp>
	void slist <_Tp>::push_front(const_reference _Value) {
		insert(begin(), _Value);
	}

	template <typename _Tp>
	void slist <_Tp>::push_back(const_reference _Value) {
		insert(end(), _Value);
	}

	template <typename _Tp>
	void slist <_Tp>::pop(const_reference _Value) {
	    pop_if([&](const_reference value)->bool{ return value == _Value; });
	}
	
	template <typename _Tp>
	template <class _Pr1>
	void slist <_Tp>::pop_if(_Pr1 _Pred) {
	    if (empty())
			return;
		node_ptr *_Link;
		for (_Link = &_head._next; (_ptr = *_Link) && !_Pred(_ptr->_value); _Link = &_ptr->_next);
		if (_ptr) {
			*_Link = _ptr->_next;
			delete _ptr;
		}
	}
	
	template <typename _Tp>
	void slist <_Tp>::pop_front() {
		erase(begin());
	}
	
	template <typename _Tp>
	void slist <_Tp>::pop_front(int n) {
		while (--n)
			pop_front();
	}
	
	template <typename _Tp>
	void slist <_Tp>::pop_back() {
		erase(iterator(_tail));
	}
	
	template <typename _Tp>
	void slist <_Tp>::pop_back(int n) {
		while (--n)
			pop_back();
	}

	template <typename _Tp>
	void slist<_Tp>::erase(iterator i) {
		if (i == nullptr)
			return;
		erase(i++, i);
	}

	template <typename _Tp>
	void slist<_Tp>::erase(iterator _First, iterator _Last) {
	    if (empty())
	        return;
		__break_link(*this, _First, _Last);
		destroy(_First._node);
	}

	template <typename _Tp>
	void slist<_Tp>::unique() {
		node_ptr *_Link;
		for (_Link = &_head._next; (_ptr = *_Link) && _ptr->_next;)
		if (_ptr->_next->_value == _ptr->_value) {
			*_Link = _ptr->_next;
			delete _ptr;
		}
		else
		    _Link = &_ptr->_next;
	}

	template <typename _Tp>
	void slist <_Tp>::remove(const_reference _Value) {
		remove_if([&](node_ptr _Node)->bool {return _Node->_value == _Value; });
	}

	template <typename _Tp>
	template <class _Pr1>
	void slist <_Tp>::remove_if(_Pr1 _Pred) {
		if (empty())
			return;
		node_ptr *_Link = &_head._next;
		while (_ptr = *_Link)
			if (_Pred((*_Link))) {
				*_Link = _ptr->_next;
				delete _ptr;
			}
			else
				_Link = &_ptr->_next;
	}

	template <typename _Tp>
	template < class _Cmp > 
	void slist <_Tp>::sort(_Cmp cmp) {
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
	void slist<_Tp>::merge(slist &x) {
	    node_ptr _Curr = &_head;
	    while (_Curr->_next && x._head._next)
	        if (x.front() < _Curr->_next->_value) {
	            _ptr = x._head._next;
	            x._head._next = x._head._next->_next;
	            __make_link(_Curr, _ptr, _ptr);
	        }
	        else
	            _Curr = _Curr->_next;
	  if (x._head._next)
	      _Curr->_next = x._head._next;
	  x._head._next = nullptr;
}

	template <typename _Tp> void slist <_Tp>::swap(slist &x) {
		using std::swap;
		swap(_head._next, x._head._next);
		swap(_tail, x._tail);
	}

	template <typename _Tp> void slist <_Tp>::reverse() {
		if (empty())
			return;
		node_ptr _New_head = nullptr;
		while (_ptr = _head._next->_next) {
			_head._next->_next = _New_head;
			_New_head = _head._next;
			_head._next = _ptr;
		}
		_head._next = _New_head;
	}

	template <typename _Tp>
	typename slist<_Tp>::iterator slist<_Tp>::previous(iterator i) {
		if (i == _head._next)
			return iterator(&_head);
		if (i == nullptr)
			return iterator(_tail);
		iterator _It(begin());
		for (; _It && _It._node->_next != i; ++_It);
		return _It;
	}

	template <typename _Tp>
	typename slist<_Tp>::iterator slist<_Tp>::find(const_reference _Value) const {
	    iterator _It(begin());
		for (; _It && *_It != _Value; ++_It);
		return _It;
	}

	template <typename _Tp>
	typename slist <_Tp>::difference_type slist <_Tp>::size() {
		return end() - begin();
	}

	template <typename _Tp>
	void slist<_Tp>::clear() {
		destroy(_head._next);
		_head._next = _tail = _ptr = nullptr;
	}

	template <typename _Tp>
	bool slist<_Tp>::empty() const {
		return _head._next == nullptr;
	}

	template <typename _Tp>
	slist<_Tp> &slist<_Tp>::operator=(const slist<_Tp> &rhs) {
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
	slist<_Tp> &slist<_Tp>::operator=(slist<_Tp> &&rhs) {
		destroy(_head._next);
		_head._next = rhs._head._next;
		_tail = rhs._tail;
		rhs._head._next = rhs._tail = nullptr;
	}
	
	template <typename _Tp>
	slist<_Tp>::~slist() {
		destroy(_head._next);
		_head._next = _tail = _ptr = nullptr;
	}

	template <typename _Tp>
	void swap(slist <_Tp> &x, slist <_Tp> &y) {
		x.swap(y);
	}

	template <typename _Tp>
	class lslist
	{
	public:
		typedef _slist_iter <_Tp, _Tp*, _Tp&> iterator;
		typedef _slist_iter <_Tp, const _Tp*, const _Tp&> const_iterator;
		typedef typename iterator::node node;
		typedef _Tp value_type;
		typedef ptrdiff_t difference_type;
		typedef size_t size_type;
		typedef typename iterator::node_ptr node_ptr;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;

		inline reference      front() { return _start._next->_value; }
		inline const_reference front()  const { return _start._next->_value; }
		inline reference    back() { return _finish->_value; }
		inline const_reference back()   const { return _finish->_value; }
		inline iterator       begin() { return iterator(_start._next); }
		inline iterator       end() { return iterator(&_start); }
		inline const_iterator begin()  const { return const_iterator(_start._next); }
		inline const_iterator end()    const { return const_iterator(&_start); }
		inline const_iterator cbegin() const { return const_iterator(_start._next); }
		inline const_iterator cend()   const { return const_iterator(&_start); }

		lslist() :_finish(&_start), _ptr(nullptr) {
			_start._next = &_start;
		}
		explicit lslist(const_reference _Value) :_finish(&_start), _ptr(nullptr) {
			_start._next = &_start;
			push_front(_Value);
		}

		lslist(const lslist &x) : _finish(&_start), _ptr(nullptr) {
			_start._next = &_start;
			insert(begin(), x.begin(), x.end());
		}

		lslist(lslist &&x) :_finish(nullptr), _ptr(nullptr) {
			_start = x._start;
			_finish = x._finish;
			x._finish = nullptr;
		}

		template <typename _Ty> lslist(_Ty _First, _Ty _Last) : _finish(&_start), _ptr(nullptr) {
			_start._next = &_start;
			insert(begin(), _First, _Last);
		}

		lslist(int n, const_reference _Value) : _finish(&_start), _ptr(nullptr) {
			_start._next = &_start;
			insert(begin(), n, _Value);
		}
		
		lslist(std::initializer_list<_Tp> l) : _finish(&_start), _ptr(nullptr) {
			_start._next = &_start;
			insert(begin(), l);
		}
		
		void insert(iterator _Pos, const_reference _Value);
		void insert(iterator _Pos, node_ptr _Node);
		void insert(iterator _Pos, iterator _It);
		void insert(iterator _Pos, int n, const_reference _Value);
		void insert(iterator _Pos, std::initializer_list<_Tp> l);
		template <class _Ty>
		void insert(iterator _Pos, _Ty _First, _Ty _Last);

		template <class _Ty>
		void insert_after(iterator _Pos, _Ty _First, _Ty _Last);

		void splice(iterator _Pos, lslist &x);
		void splice(iterator _Pos, lslist &x, iterator i);
		void splice(iterator _Pos, lslist &x, iterator _First, iterator _Last);

		template <class _Pr2>
		void push_if(const_reference _Value, _Pr2 _Pred);
		void push_front(const_reference _Value);

		void push_back(const_reference _Value);

		void pop(const_reference _Value);
		template <class _Pr1>  void pop_if(_Pr1 _Pred);
		void pop_front();
		void pop_front(int n);
		void pop_back();
		void pop_back(int n);
		void remove(const_reference _Value);

		template <class _Pr1>
		void remove_if(_Pr1 _Pred);
		void erase(iterator i);
		void erase(iterator _First, iterator _Last);
		
		void unique();
		void merge(lslist &x);
		void reverse();
		void swap(lslist &x);
		template <class _Cmp>
		void sort(_Cmp cmp);
		difference_type size() const;
		void clear();
		iterator find(const_reference _Value) const;
		inline bool empty() const;

		~lslist();
		lslist &operator =(const lslist &rhs);
		lslist &operator =(lslist &&rhs);
	private:
		node _start;
		node_ptr _finish;
		node_ptr _ptr;
		
		inline iterator __previous(iterator) const;
		inline void __make_link(iterator, iterator, iterator);
		inline void __make_link(node_ptr, node_ptr, node_ptr);
		inline iterator __break_link(lslist&, iterator, iterator);
		inline node_ptr __break_link(lslist&, node_ptr, node_ptr);
	};

	template <typename _Tp>
	typename lslist<_Tp>::iterator lslist<_Tp>::__previous(iterator i) const{
		iterator _It(begin());
		for (; _It._node != &_start && _It._node->_next != i; ++_It);
		return _It;
	}
	
	template <typename _Tp>
	void lslist<_Tp>::__make_link(iterator _Prev, iterator _First, iterator _Last) {
	    __make_link(_Prev._node, _First._node, _Last._node);
	}
	
	template <typename _Tp>
	void lslist<_Tp>::__make_link(node_ptr _Prev, node_ptr _First, node_ptr _Last) {
		_Last->_next = _Prev->_next;
		_Prev->_next = _First;
		if (_Prev == _finish)
		    _finish = _Last;
	}
	
	template <typename _Tp>
	typename lslist<_Tp>::iterator lslist<_Tp>::__break_link(lslist &x, iterator _First, iterator _Last) {
	    return iterator(__break_link(x, _First._node, _Last._node));
	}
	
	template <typename _Tp>
	typename lslist<_Tp>::node_ptr lslist<_Tp>::__break_link(lslist &x, node_ptr _First, node_ptr _Last) {
	    node_ptr _Prev(&x._start);
		for (_ptr = x._start._next; _ptr != &x._start && _ptr->_next != _Last; _ptr = _ptr->_next)
		    if (_ptr->_next == _First)
		        _Prev = _ptr;
		_Prev->_next = _Last;
		return _ptr;
	}
	
	template <typename _Tp>
	void lslist<_Tp>::insert(iterator _Pos, const_reference _Value) {
	    insert(_Pos, new node(_Value));
	}

	template <typename _Tp>
	void lslist<_Tp>::insert(iterator _Pos, node_ptr _Node) {
		__make_link(__previous(_Pos), _Node, _Node);
	}

	template <typename _Tp>
	void lslist<_Tp>::insert(iterator _Pos, iterator _It) {
		insert(_Pos, new node(*_It));
	}
	
	template <typename _Tp>
	void lslist<_Tp>::insert(iterator _Pos, std::initializer_list<_Tp> l) {
	    insert(_Pos, l.begin(), l.end());
	}
	
	template <typename _Tp>
	void lslist<_Tp>::insert(iterator _Pos, int n, const_reference _Value) {
		node_ptr _New = new node(_Value);
		--n;
		for (_ptr = _New; --n; _New = _New->_next)
				_New->_next = new node(_Value);
		__make_link(_Pos._node, _ptr, _New);
	}
	
	template <typename _Tp>
	template <typename _Ty>
	void lslist<_Tp>::insert(iterator _Pos, _Ty _First, _Ty _Last) {
	    if (_First == _Last)
	        return;
		_Ty i = _First;
		node_ptr _New = new node(*i);
		for (_ptr = _New; ++i != _Last; _New = _New->_next)
				_New->_next = new node(*i);
		__make_link(_Pos, _ptr, _New);
	}

	template <typename _Tp>
	void lslist<_Tp>::splice(iterator _Pos, lslist &x) {
		splice(_Pos, x, x.begin(), x.end());
	}

	template <typename _Tp>
	void lslist<_Tp>::splice(iterator _Pos, lslist &x, iterator i) {
		splice(_Pos, x, i++, i);
	}

	template <typename _Tp>
	void lslist<_Tp>::splice(iterator _Pos, lslist &x, iterator _First, iterator _Last) {
	    if (_First != _Last)
		    __make_link(_Pos, _First, __break_link(x, _First, _Last));
	}

	template <typename _Tp>
	template <class _Pr2>
	void lslist<_Tp>::push_if(const_reference _Value, _Pr2 _Pred) {
	    node_ptr _Prev(nullptr);
		for (_ptr = _start._next; _ptr != &_start && _Pred(_ptr->_value, _Value); _ptr = _ptr->_next);
			_Prev = _ptr;
		_ptr = new node(_Value);
		__make_link(_Prev, _ptr, _ptr);
	}
	
	template <typename _Tp>
	void lslist<_Tp>::push_front(const_reference _Value) {
		insert(begin(), _Value);
	}

	template <typename _Tp>
	void lslist<_Tp>::push_back(const_reference _Value) {
		insert(end(), _Value);
	}

	template <typename _Tp>
	void lslist<_Tp>::pop(const_reference _Value) {
	    pop_if([&](const_reference value)->bool{ return value == _Value; });
	}
	
	template <typename _Tp>
	template <class _Pr1>
	void lslist <_Tp>::pop_if(_Pr1 _Pred) {
	    if (empty())
			return;
		node_ptr *_Link;
		for (_Link = &_start._next; (_ptr = *_Link) != &_start && !_Pred(_ptr->_value); _Link = &_ptr->_next);
		if (_ptr != &_start) {
			*_Link = _ptr->_next;
			delete _ptr;
		}
	}

	template <typename _Tp>
	void lslist<_Tp>::pop_front() {
		erase(begin());
	}

	template <typename _Tp>
	void lslist<_Tp>::pop_back() {
		erase(iterator(_finish));
	}

	template <typename _Tp>
	typename lslist<_Tp>::iterator lslist<_Tp>::find(const_reference _Value) const {
	    iterator _It(begin());
		for (; *_It != _Value && _It._node != &_start; ++_It);
		return _It;
	}

	template <typename _Tp>
	void lslist<_Tp>::remove(const_reference _Value) {
		remove_if([&](const_reference value)->bool {return value == _Value; });
	}

	template <typename _Tp>
	template <class _Pr1>
	void lslist<_Tp>::remove_if(_Pr1 _Pred) {
		if (empty())
			return;
		node_ptr *_Link;
		for (_Link = &_start._next; (_ptr = *_Link) != &_start;)
			if (_Pred((*_Link)->_value)) {
				*_Link = _ptr->_next;
				delete _ptr;
			}
			else
				_Link = &_ptr->_next;
	}

	template <typename _Tp>
	void lslist<_Tp>::erase(iterator i) {
		if (i._node != &_start)
		    erase(i++, i);
	}

	template <typename _Tp>
	void lslist<_Tp>::erase(iterator _First, iterator _Last) {
		if (_First != _Last)
		    destroy(_First._node, __break_link(*this, _First, _Last)._node->_next);
	}

	template <typename _Tp>
	void lslist<_Tp>::unique() {
		if (empty())
			return;
		node_ptr *_Link;
		for (_Link = &_start._next; (_ptr = *_Link) != _finish;)
			if (_ptr->_value == _ptr->_next->_value) {
			    *_Link = _ptr->_next;
				delete _ptr;
			}
			else
			    _Link = &_ptr->_next;
}

	template <typename _Tp>
	void lslist<_Tp>::swap(lslist &x) {
		using std::swap;
		swap(_finish->_next, x._finish->_next);
		swap(_start._next, x._start._next);
	}
	
	template <typename _Tp>
	template <class _Cmp>
	void lslist<_Tp>::sort(_Cmp cmp) {
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
	void lslist<_Tp>::reverse() {
		std::stack<_Tp> _Tmp;
		for (auto &i : *this)
			_Tmp.push(i);
		for (auto &i : *this) {
			i = _Tmp.top();
			_Tmp.pop();
		}
	}

	template <typename _Tp>
	void lslist<_Tp>::merge(lslist &x) {
		if (x.empty())
			return;
		iterator _It(end());
		 while (!x.empty() && _It._node->_next != &_start)
			if (_It._node->_next->_value > x.front()) {
			    _ptr = x._start._next;
				x._start._next = x._start._next->_next;
				__make_link(_It._node, _ptr, _ptr);
			}
			else
			    ++_It;
		if (!x.empty())
			splice(_It, x);
	}

	template <typename _Tp>
	typename lslist<_Tp>::difference_type lslist<_Tp>::size() const {
		iterator _It(begin());
		difference_type diff = 0;
		for (; _It._node != &_start; ++_It, ++diff);
		return diff;
	}

	template <typename _Tp>
	void lslist<_Tp>::clear() {
		if (empty())
			return;
		erase(begin(), end());
	}

	template <typename _Tp>
	bool lslist<_Tp>::empty() const {
		return _start._next == &_start;
	}

	template <typename _Tp>
	lslist<_Tp> &lslist<_Tp>::operator =(const lslist &rhs) {
		iterator _It = begin();
		for (auto &i : rhs) {
			if (_It == _start)
				push_back(i);
			else {
				*_It = i;
				++_It;
			}
		}
		if (_It != &_start)
		    destroy(_It._node, &_start);
		return *this;
	}

	template <typename _Tp>
	lslist<_Tp> &lslist<_Tp>::operator =(lslist &&rhs) {
		destroy(_start._next, &_start);
		_start = rhs._start;
		_finish = rhs._finish;
		rhs._finish = rhs._start._next = &rhs._start;
	}

	template <typename _Tp>
	lslist<_Tp>::~lslist() {
		destroy(_start._next, &_start);
		_finish = _ptr = nullptr;
	}

	template <typename _Tp>
	void swap(lslist<_Tp> &x, lslist<_Tp> &y) {
		x.swap(y);
	}
}
#endif