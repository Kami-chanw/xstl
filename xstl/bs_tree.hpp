/*
 *   Copyright (c) 2022 Kamichanw. All rights reserved.
 *   @file allocator.hpp
 *   @brief The tree library contains 5 types of binary search trees :
 *	1. bs_tree (Binary Search Tree)
 *	2. avl_tree (AVL Tree)
 *	3. treap_tree (Treap Tree)
 *	4. splay_tree (Splay Tree)
 *	5. rb_tree (Red Black Tree)
 *   @author Kami-chan e-mail: 865710157@qq.com
 */
#pragma once
#ifndef _BS_TREE_HPP_
#define _BS_TREE_HPP_

#include "allocator.hpp"
#include "compressed_tuple.hpp"
#include "iter_adapter.hpp"
#include <algorithm>
#ifndef __cpp_explicit_this_parameter
#include <functional>
#endif
#include <iosfwd>
#include <queue>
#include <random>

#undef KFN
#define KFN(NODE) _Traits::kfn((NODE)->_value)

namespace xstl {
    // namespace {
    /**
     *	@enum color
     *	@brief for red black tree(rb_tree)
     */
    enum { BLACK, RED };

    /**
     *	@class _Tree_node
     *   @brief the node of bs_tree.
     */
    template <class _Tp>
    struct _Tree_node {
        using _Node    = _Tree_node<_Tp>;
        using _Nodeptr = _Node*;

        int _prop = 0;

        bool     _is_nil = true;
        _Nodeptr _left{ nullptr };
        _Nodeptr _right{ nullptr };
        _Nodeptr _parent{ nullptr };
        _Tp      _value{};

        template <class _Alnode>
        inline static _Nodeptr create_root(_Alnode& alloc) {
            static_assert(std::is_same_v<typename _Alnode::value_type, _Node>, "Allocator's value_type is not consist with node");
            _Nodeptr _node = alloc.allocate(1);
            init_node(_node, _node, _node, _node, BLACK, true);
            return _node;
        }

        template <class _Alnode, class... _Args>
        inline static _Nodeptr create_node(_Alnode& alloc, _Nodeptr root, _Args&&... args) {
            static_assert(std::is_same_v<typename _Alnode::value_type, _Node>, "Allocator's value_type is not consist with node");
            _Nodeptr _node = alloc.allocate(1);
            std::allocator_traits<_Alnode>::construct(alloc, std::addressof(_node->_value), std::forward<_Args>(args)...);
            init_node(_node, root, root, root, BLACK, false);
            return _node;
        }

        inline bool is_real_root() const noexcept { return this == _parent->_parent; }
        inline bool is_left() const noexcept { return this == _parent->_left; }
        inline bool is_right() const noexcept { return this == _parent->_right; }

        inline static void assign_node(_Nodeptr node, _Nodeptr left, _Nodeptr right, _Nodeptr parent, const int attr,
                                       bool is_nil) {
            node->_left   = left;
            node->_right  = right;
            node->_parent = parent;
            node->_prop   = attr;
            node->_is_nil = is_nil;
        }

        inline static void init_node(_Nodeptr node, _Nodeptr left, _Nodeptr right, _Nodeptr parent, const int attr, bool is_nil) {
            construct_in_place(node->_left, left);
            construct_in_place(node->_right, right);
            construct_in_place(node->_parent, parent);
            node->_prop   = attr;
            node->_is_nil = is_nil;
        }

        template <class _Alnode>
        void static destroy_node(_Alnode& alloc, _Nodeptr node) noexcept {
            static_assert(std::is_same_v<typename _Alnode::value_type, _Node>, "Allocator's value_type is not consist with node");
            std::allocator_traits<_Alnode>::destroy(alloc, std::addressof(node->_value));
            std::allocator_traits<_Alnode>::deallocate(alloc, node, 1);
        }

        inline static _Nodeptr leftmost(_Nodeptr node) noexcept {
            while (!node->_left->_is_nil)
                node = node->_left;
            return node;
        }

        inline static _Nodeptr rightmost(_Nodeptr node) noexcept {
            while (!node->_right->_is_nil)
                node = node->_right;
            return node;
        }

        inline static _Nodeptr find_inorder_predecessor(_Nodeptr node) noexcept {
            if (!node->_left->_is_nil)
                return rightmost(node->_left);
            _Nodeptr _parent{};
            while (!(_parent = node->_parent)->_is_nil && node->is_left())
                node = _parent;
            if (!node->_is_nil)
                node = _parent;
            return node;
        }

        inline static _Nodeptr find_inorder_successor(_Nodeptr node) noexcept {
            if (!node->_right->_is_nil)
                return leftmost(node->_right);
            while (!node->_parent->_is_nil && node->is_right())
                node = node->_parent;
            return node->_parent;
        }
    };

    template <class _Alnode>
    struct _Tree_temp_node {  // for exception safety
        using _Alnode_traits = std::allocator_traits<_Alnode>;
        using _Nodeptr       = typename _Alnode_traits::pointer;
        using _Node          = typename std::remove_pointer_t<_Nodeptr>;

        explicit _Tree_temp_node(_Alnode& alloc) : _alnode(alloc), _node(alloc.allocate(1)) {
            _Node::init_node(_node, _node, _node, _node, BLACK, true);
        }

        template <class... _Args>
        _Tree_temp_node(_Alnode& alloc, _Nodeptr root, _Args&&... values) : _Tree_temp_node(alloc) {
            _Alnode_traits::construct(_alnode, std::addressof(_node->_value), std::forward<_Args>(values)...);
            _Node::init_node(_node, root, root, root, RED, false);
        }

        _Tree_temp_node(const _Tree_temp_node&)            = delete;
        _Tree_temp_node& operator=(const _Tree_temp_node&) = delete;

        XSTL_NODISCARD _Nodeptr release() noexcept { return std::exchange(_node, nullptr); }

        ~_Tree_temp_node() {
            if (_node)
                _Node::destroy_node(_alnode, _node);
        }

        _Nodeptr _node = nullptr;
        _Alnode& _alnode;
    };

    /**
     *	@class _Tree_val
     *   @brief for scary iterator
     */
    template <class _Val_types>
    struct _Tree_val : public container_val_base {
        using _Self           = _Tree_val<_Val_types>;
        using _Nodeptr        = typename _Val_types::_Nodeptr;
        using _Node           = typename _Val_types::_Node;
        using value_type      = typename _Val_types::value_type;
        using size_type       = typename _Val_types::size_type;
        using difference_type = typename _Val_types::difference_type;
        using pointer         = typename _Val_types::pointer;
        using const_pointer   = typename _Val_types::const_pointer;
        using reference       = value_type&;
        using const_reference = const value_type&;

#ifdef PREORDER_ITERATOR
        static void incr(_Nodeptr& node) noexcept {
            if (!node->_left->_is_nil)
                node = node->_left;
            else if (!node->_right->_is_nil)
                node = node->_right;
            else {
                _Nodeptr _suc = node->_parent;
                while (node != _suc->_parent) {
                    if (node == _suc->_left && !_suc->_right->_is_nil) {
                        node = _suc->_right;
                        return;
                    }
                    node = std::exchange(_suc, _suc->_parent);
                }
                node = _suc;
            }
        }

        static void _Decr(_Nodeptr& node) noexcept {
            if (node->_is_nil) {
                node = node->_right;
                while (!node->_left->_is_nil || !node->_right->_is_nil) {
                    node = _Node::rightmost(node);
                    if (!node->_left->_is_nil)
                        node = node->_left;
                }
            }
            else {
                _Nodeptr _pre = node->_parent;
                if (node->is_right() && !_pre->_left->_is_nil && !node->is_real_root()) {
                    _pre = _pre->_left;
                    while (!_pre->_right->_is_nil || !_pre->_left->_is_nil) {
                        _pre = _Node::rightmost(_pre);
                        if (!_pre->_left->_is_nil)
                            _pre = _pre->_left;
                    }
                }
                node = _pre;
            }
        }
#elif defined POSTORDER_ITERATOR
        static void incr(_Nodeptr& node) noexcept {
            _Nodeptr _suc = node->_parent;
            if (node->is_left() && !_suc->_right->_is_nil && !node->is_real_root()) {
                _suc = _suc->_right;
                while (!_suc->_left->_is_nil || !_suc->_right->_is_nil) {
                    _suc = _Node::leftmost(_suc);
                    if (!_suc->_right->_is_nil)
                        _suc = _suc->_right;
                }
            }
            node = _suc;
        }

        static void _Decr(_Nodeptr& node) noexcept {
            if (node->_is_nil)
                node = node->_parent;
            else if (!node->_right->_is_nil)
                node = node->_right;
            else if (!node->_left->_is_nil)
                node = node->_left;
            else {
                for (_Nodeptr _pre = node->_parent;; _pre = _pre->_parent) {
                    if (node == _pre->_right && !_pre->_left->_is_nil) {
                        node = _pre->_left;
                        return;
                    }
                    node = _pre;
                }
            }
        }
#elif defined LEVEL_ORDER_ITERATOR
        static _Nodeptr _Fwd_explore(_Nodeptr node, size_type height) noexcept {
            if (height == 0 || node->_is_nil)
                return node;
            _Nodeptr _res = _Fwd_explore(node->_left, height - 1);
            if (!_res->_is_nil)
                return _res;
            return _Fwd_explore(node->_right, height - 1);
        }

        static _Nodeptr _Incr(_Nodeptr node, size_type height = 0) noexcept {
            if (node->is_real_root())
                return _Fwd_explore(node, height + 1);
            if (node->is_left()) {
                _Nodeptr _res = _Fwd_explore(node->_parent->_right, height);
                if (!_res->_is_nil)
                    return _res;
            }
            return _Incr(node->_parent, height + 1);
        }

        static void incr(_Nodeptr& node) noexcept { node = _Incr(node); }

        static _Nodeptr _Backwd_explore(_Nodeptr node, int height) noexcept {
            if (height == 0 || node->_is_nil)
                return node;
            _Nodeptr _res = _Backwd_explore(node->_right, height - 1);
            if (!_res->_is_nil)
                return _res;
            return _Backwd_explore(node->_left, height - 1);
        }

        static _Nodeptr _Decr(_Nodeptr node, int height) noexcept {
            if (node->is_real_root())
                return _Backwd_explore(node, height - 1);
            if (node->is_right()) {
                _Nodeptr _res = _Backwd_explore(node->_parent->_left, height);
                if (!_res->_is_nil)
                    return _res;
            }
            return _Decr(node->_parent, height + 1);
        }

        static void _Decr(_Nodeptr& node) noexcept { node = _Decr(node, 0); }
#else  // INORDER_ITERATOR

        static void incr(_Nodeptr& node) noexcept { node = _Node::find_inorder_successor(node); }

        static void _Decr(_Nodeptr& node) noexcept {
            node = node->_is_nil ? node->_right : _Node::find_inorder_predecessor(node);
        }
#endif

        static void decr(_Nodeptr& node) noexcept {
            auto _oldnode = node;

            _Decr(node);
            XSTL_EXPECT(_oldnode != node, "iterators cannot decrease");
        }
        static value_type& extract(_Nodeptr node) noexcept { return node->_value; }

        static bool dereferable(const _Self* tree, _Nodeptr node) noexcept { return node != tree->_root; }

        static bool increasable(const _Self*, _Nodeptr node) noexcept { return !node->_is_nil; }

        static bool decreasable(const _Self* tree, _Nodeptr node) noexcept {
            return true;  // always true because verification should be after decreasing
        }

        void swap(_Self& x) {
            // no checking, because this swap is always be called in library
            using std::swap;
            swap(_root, x._root);
        }

        void init() noexcept { _Node::assign_node(_root, _root, _root, _root, BLACK, true); }

        _Nodeptr _root{};
    };
    //}  // namespace
    /**
     *	@class bs_tree
     *   @brief binary search tree.
     */
    template <class _Traits, template <class, class> class... _MixIn>
    class _Bs_tree : public _MixIn<_Traits, _Bs_tree<_Traits, _MixIn...>>... {
        using _Self                     = _Bs_tree<_Traits, _MixIn...>;
        using _Scary_val                = typename _Traits::_Scary_val;
        using _Node                     = typename _Traits::_Scary_val::_Node;
        using _Nodeptr                  = typename _Traits::_Nodeptr;
        using _Unchecked_const_iterator = iter_adapter::unchecked_bid_citer<_Scary_val>;
        using _Altp_traits              = typename _Traits::_Altp_traits;
        using _Alnode_type              = typename _Altp_traits::template rebind_alloc<_Node>;
        using _Alnode_traits            = std::allocator_traits<_Alnode_type>;

    public:
        friend struct _Tree_accessor;
        using allocator_type = typename _Traits::allocator_type;
        using value_type     = typename _Traits::value_type;
        static_assert(std::is_same_v<typename allocator_type::value_type, value_type>,
                      MISMATCH_ALLOCATOR_MESSAGE("[some]_map/set", "value_type"));
        using key_type        = typename _Traits::key_type;
        using key_compare     = typename _Traits::key_compare;
        using value_compare   = typename _Traits::value_compare;
        using size_type       = typename _Traits::size_type;
        using difference_type = typename _Traits::difference_type;
        using pointer         = typename _Traits::pointer;
        using const_pointer   = typename _Traits::const_pointer;
        using reference       = value_type&;
        using const_reference = const value_type&;

        using const_iterator = typename _Traits::const_iterator;
        using iterator       = typename _Traits::iterator;
#ifndef LEVEL_ORDER_ITERATOR
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
#endif
        using node_type = typename _Traits::node_type;

        static constexpr bool _Multi = _Traits::_Multi;

        struct insert_return_type {
            iterator  position;
            bool      inserted;
            node_type node;
        };

    public:
        XSTL_NODISCARD iterator       begin() noexcept { return _Make_iter(cbegin().base()); }
        XSTL_NODISCARD const_iterator begin() const noexcept { return cbegin(); }
        XSTL_NODISCARD const_iterator cbegin() const noexcept {
            _Nodeptr _root = _Get_root();
#if defined(PREORDER_ITERATOR) || defined(LEVEL_ORDER_ITERATOR)
            return _Make_citer(_root->_parent);
#elif defined POSTORDER_ITERATOR
            _Nodeptr _curr = _root->_left != _root->_parent ? _root->_left : _root->_right;
            while (!_curr->_left->_is_nil || !_curr->_right->_is_nil) {
                _curr = _Node::leftmost(_curr);
                if (!_curr->_right->_is_nil)
                    _curr = _curr->_right;
            }
            return _Make_citer(_curr);
#else
            return _Make_citer(_root->_left);
#endif
        }
        XSTL_NODISCARD iterator       end() noexcept { return _Make_iter(_Get_root()); }
        XSTL_NODISCARD const_iterator end() const noexcept { return cend(); }
        XSTL_NODISCARD const_iterator cend() const noexcept { return _Make_citer(const_cast<_Nodeptr>(_Get_root())); }

#ifndef LEVEL_ORDER_ITERATOR
        XSTL_NODISCARD reverse_iterator       rbegin() noexcept { return reverse_iterator(_Make_iter(crbegin().base().base())); }
        XSTL_NODISCARD const_reverse_iterator rbegin() const noexcept { return crbegin(); }

        XSTL_NODISCARD const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

        XSTL_NODISCARD reverse_iterator       rend() noexcept { return reverse_iterator(_Make_iter(crend().base().base())); }
        XSTL_NODISCARD const_reverse_iterator rend() const noexcept { return crend(); }

        XSTL_NODISCARD const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }
#endif
        /**
         *	@brief constructs an empty bs_tree.
         *	@param cmpr : comparison function object to use for all comparisons of keys
         *   @param alloc : allocator to use for all memory allocations of this tree
         */
        _Bs_tree() { _Init(); }

        explicit _Bs_tree(const key_compare& cmpr) : _tpl(cmpr, std::ignore, std::ignore) { _Init(); }

        _Bs_tree(const key_compare& cmpr, const allocator_type& alloc) : _tpl(cmpr, alloc, std::ignore) { _Init(); }

        /**
         *   @brief constructs the bs_tree with the copy of the contents of other.
         *   @param tree : other instance of bs_trees(pass by lvalue).
         * 	@param cmpr : comparison function object to use for all comparisons of keys
         *   @param alloc : allocator to use for all memory allocations of this tree
         */
        _Bs_tree(const _Bs_tree& other) : _Bs_tree(other, other._Getal()) {}

        _Bs_tree(const _Bs_tree& other, const allocator_type& alloc) : _tpl(other.key_comp(), alloc, std::ignore) {
            _Init();
            scoped_guard _guard([&] { _Node::destroy_node(_Getal(), _Get_root()); });
            _Copy<copy_op_tag>(other);
            _guard.dismiss();
        }

        _Bs_tree(_Bs_tree&& other) : _tpl(other.key_cmpr(), other._Getal(), std::ignore) {
            _Init();
            _Swap_excluding_cmpr(other);
        }

        _Bs_tree(_Bs_tree&& other, const allocator_type& alloc) : _tpl(other.key_comp(), alloc, std::ignore) {
            _Init();
            if constexpr (!_Alnode_traits::is_always_equal::value) {
                if (_Getal() != other._Getal()) {
                    scoped_guard _guard([&] { _Node::destroy_node(_Getal(), _Get_root()); });
                    _Copy<move_op_tag>(other);
                    _guard.dismiss();
                    return;
                }
            }
            _Swap_excluding_cmpr(other);
        }

        /**
         *   @brief constructs the bs_tree with the contents of the range [first, last)
         *   @param first : the beginning of range to insert
         *   @param last : the end of range to insert
         * 	@param cmpr : comparison function object to use for all comparisons of keys
         *   @param alloc : allocator to use for all memory allocations of this tree
         */
        template <class _Iter, XSTL_REQUIRES_(is_input_iterator_v<_Iter>)>
        _Bs_tree(_Iter first, _Iter last) : _Bs_tree() {
            insert(first, last);
        }

        template <class _Iter, XSTL_REQUIRES_(is_input_iterator_v<_Iter>)>
        _Bs_tree(_Iter first, _Iter last, const key_compare& cmpr) : _Bs_tree(cmpr) {
            insert(first, last);
        }
        template <class _Iter, XSTL_REQUIRES_(is_input_iterator_v<_Iter>)>
        _Bs_tree(_Iter first, _Iter last, const allocator_type& alloc) : _Bs_tree(alloc) {
            insert(first, last);
        }

        template <class _Iter, XSTL_REQUIRES_(is_input_iterator_v<_Iter>)>
        _Bs_tree(_Iter first, _Iter last, const key_compare& cmpr, const allocator_type& alloc) : _Bs_tree(cmpr, alloc) {
            insert(first, last);
        }

        /**
         *   @brief constructs the bs_tree with the contents of the initializer list init.
         *   @param l : initializer list to insert the values from
         * 	@param cmpr : comparison function object to use for all comparisons of keys
         *   @param alloc : allocator to use for all memory allocations of this tree
         */
        _Bs_tree(std::initializer_list<value_type> l) : _Bs_tree(l.begin(), l.end()) {}
        _Bs_tree(std::initializer_list<value_type> l, const key_compare& cmpr) : _Bs_tree(l.begin(), l.end(), cmpr) {}
        _Bs_tree(std::initializer_list<value_type> l, const allocator_type& alloc) : _Bs_tree(l.begin(), l.end(), alloc) {}
        _Bs_tree(std::initializer_list<value_type> l, const key_compare& cmpr, const allocator_type& alloc)
            : _Bs_tree(l.begin(), l.end(), cmpr, alloc) {}

        /**
         *   @return the allocator associated with the container.
         */
        XSTL_NODISCARD allocator_type get_allocator() const noexcept { return static_cast<allocator_type>(_Getal()); }

        /**
         *   @brief insert a key into the tree.
         *   @param key : key of the element to insert.
         *	@return a pair consisting of an iterator to the inserted element
         */
        template <bool _IsMulti = _Multi, std::enable_if_t<_IsMulti, int> = 0>
        iterator insert(const_reference value) {
            return _Emplace(value).first;
        }
        template <bool _IsMulti = _Multi, std::enable_if_t<_IsMulti, int> = 0>
        iterator insert(value_type&& value) {
            return _Emplace(std::move(value)).first;
        }

        /**
         *   @brief insert a key into the tree.the key is unique.
         *   @param key : key of the element to insert.
         *	@return a pair consisting of an iterator to the inserted element
         */
        template <bool _IsMulti = _Multi, std::enable_if_t<!_IsMulti, int> = 0>
        std::pair<iterator, bool> insert(const_reference value) {
            return _Emplace(value);
        }
        template <bool _IsMulti = _Multi, std::enable_if_t<!_IsMulti, int> = 0>
        std::pair<iterator, bool> insert(value_type&& value) {
            return _Emplace(std::move(value));
        }

        /**
         *   @brief insert a key into the tree.
         *	@param position : a hint to insert.(it may not insert at position)
         *   @param value : key of the element to insert.
         *	@return an iterator which points to new node
         */
        iterator insert(const_iterator position, const_reference value) {
            XSTL_EXPECT(std::addressof(_Get_val()) == CAST2SCARY(position._Get_cont()), "tree iterator insert outside range");

            return _Emplace_hint(position.base(), value);
        }
        iterator insert(const_iterator position, value_type&& value) {
            XSTL_EXPECT(std::addressof(_Get_val()) == CAST2SCARY(position._Get_cont()), "tree iterator insert outside range");

            return _Emplace_hint(position.base(), std::move(value));
        }

        /**
         *   @brief inserts elements from initializer list l
         *   @param l : initializer list to insert the values from
         */
        void insert(std::initializer_list<value_type> l) { insert(l.begin(), l.end()); }

        /**
         *   @brief inserts elements from range [first, last)
         *   @param first : the beginning of range of elements to insert
         *	@param last : the end of range of elements to insert
         */
        template <class _Iter, XSTL_REQUIRES_(is_input_iterator_v<_Iter>)>
        void insert(_Iter first, _Iter last) {
            for (; first != last; ++first)
                _Emplace(*first);
        }

        /**
         *	@brief If nh is an empty node handle, does nothing. Otherwise, inserts the element owned by nh into the container.
         *	@param nh : a compatible node handle
         *	@return an insert_return_type with the members initialized.
         */
        auto insert(node_type&& nh);
        /**
        *	@brief If nh is an empty node handle, does nothing. Otherwise, inserts the element owned by nh into the container.
        The element is inserted as close as possible to the position just prior to hint
        *	@param nh : a compatible node handle
        *	@return end iterator if nh was empty, iterator pointing to the inserted element if insertion took place,
        and iterator pointing to an element with a key equivalent to nh.key() if it failed
        */
        iterator insert(const_iterator hint, node_type&& nh);

        /**
         *   @brief inserts a new element into the container by constructing it in - place with the given values
         *   @param values : arguments to forward to the constructor of the element
         *	@return a pair consisting of an iterator to the inserted element
         */
        template <class... _Args>
        std::pair<iterator, bool> emplace(_Args&&... values) {
            return _Emplace(std::forward<_Args>(values)...);
        }
        /**
         *   @brief inserts a new element to the container as close as possible to the position just before hint
         *   @param values : arguments to forward to the constructor of the element
         *	@return an iterator which points to new node.
         */
        template <class... _Args>
        iterator emplace_hint(const_iterator position, _Args&&... values) {
            XSTL_EXPECT(std::addressof(_Get_val()) == CAST2SCARY(position._Get_cont()), "tree iterator insert outside range");

            return _Emplace_hint(position.base(), std::forward<_Args>(values)...);
        }

        /**
         *   @brief return an iterator pointing to the first element that is not less than key.
         *   @param key : key to compare the elements to.
         *	@return iterator pointing to the first element that is not less than key.
         */
        XSTL_NODISCARD iterator       lower_bound(const key_type& key);
        XSTL_NODISCARD const_iterator lower_bound(const key_type& key) const;
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD iterator lower_bound(const _Key& key);
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD const_iterator lower_bound(const _Key& key) const;
        /**
         *   @brief return an iterator pointing to the first element that is greater than key.
         *   @param key : key to compare the elements to.
         *	@return iterator pointing to the first element that is greater than key.
         */
        XSTL_NODISCARD iterator       upper_bound(const key_type& key);
        XSTL_NODISCARD const_iterator upper_bound(const key_type& key) const;
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD iterator upper_bound(const _Key& key);
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD const_iterator upper_bound(const _Key& key) const;

        /**
         *   @brief returns a range containing all elements with the given key in the tree.
         *   @param key : key to compare the elements to.
         *	@return std::pair containing a pair of iterators defining the wanted range:
         *	the first pointing to the first element that is not less than key
         *	and the second pointing to the first element greater than key.
         */
        XSTL_NODISCARD std::pair<iterator, iterator> equal_range(const key_type& key);
        XSTL_NODISCARD std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const;
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD std::pair<iterator, iterator> equal_range(const _Key& key);
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD std::pair<const_iterator, const_iterator> equal_range(const _Key& key) const;

        /**
         *	@brief Returns the number of elements with key that compares equivalent to the specified argument
         *	@param key : key of the elements to count.
         *	@return Number of elements with key that compares equivalent to key
         */
        XSTL_NODISCARD size_type count(const key_type& key) const;
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD size_type count(const _Key& key) const;

        /**
         *	@brief Checks if there is an element with key equivalent to key in container.
         *	@param key : key of the elements to search for.
         *	@return true if there is such an element, otherwise false.
         */
        XSTL_NODISCARD bool contains(const key_type& key) const;
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD bool contains(const _Key& key) const;

        /*
         *	@brief unlinks the node that contains the element pointed to by position and returns a node handle that owns it
         *	@param position : a valid iterator into this container
         *	@return a node handle that owns the extracted element
         */
        node_type extract(const_iterator position);
        /*
         *	@brief unlinks the node that contains the element pointed to by position and returns a node handle that owns it
         *	@param x : a key to identify the node to be extracted
         *	@return a node handle that owns the extracted element, or empty node handle in case the element is not found in
         *overload
         */
        node_type extract(const key_type& x);

        /*
        *	@brief Attempts to extract each element in source and insert it into *this using the comparison object of *this.
        If there is an element in *this with key equivalent to the key of an element from source, then that element is not
        extracted from source. *	@param x : compatible container to transfer the nodes from
        */
        template <class _Other_traits>
        void merge(_Bs_tree<_Other_traits, _MixIn...>& x);
        template <class _Other_traits>
        void merge(_Bs_tree<_Other_traits, _MixIn...>&& x) {
            merge(x);
        }

        /**
         *   @brief removes the element at position.
         *   @param position : iterator to the element to remove
         *	@return iterator following the last removed element.
         */
        iterator erase(const_iterator position) noexcept;
        /**
         *   @brief removes specified elements from the container.
         *   @param key : key of the elements to remove
         *	@return number of elements removed.
         */
        size_type erase(const key_type& value) noexcept(noexcept(_Equal_range(value))) {
            auto      _res = _Equal_range(value);
            size_type n    = 0;

            for (; _res.first != _res.second; ++n) {
                _Nodeptr _suc = _Node::find_inorder_successor(_res.first);
                erase(_Make_citer(_res.first));
                _res.first = _suc;
            }

            return n;
        }

        /**
         *   @brief removes the elements in the range [first, last).
         *   @param first : the beginning of range of elements to insert
         *	@param last : the end of range of elements to insert
         *	@return iterator following the last removed element.
         */
        iterator erase(const_iterator first, const_iterator last) noexcept;

        /**
         *   @brief display the whole tree
         */
        template <class _Elem, class _ElemTraits>
        void display(std::basic_ostream<_Elem, _ElemTraits>& out) const;

        /**
         *   @brief finds an element with key equivalent to key.
         *	@param key : key of the element to search for
         *	@return iterator to an element with key equivalent to key
         */
        XSTL_NODISCARD iterator       find(const key_type& key);
        XSTL_NODISCARD const_iterator find(const key_type& key) const;
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD iterator find(const _Key& value);
        template <class _Key, class _Cmpr = key_compare, class = typename _Cmpr::is_transparent>
        XSTL_NODISCARD const_iterator find(const _Key& value) const;

        /**
         *	@brief get the height of tree.
         */
        XSTL_NODISCARD size_type height() const noexcept;
        /**
         *	@brief get the width of tree.
         */
        XSTL_NODISCARD size_type width() const;
        /**
         *	@return returns the number of elements in the bs_tree
         */
        XSTL_NODISCARD size_type size() const noexcept { return _size; }
        /**
         *	@return returns the maximum number of elements the bs_tree is able to hold due to system or library implementation
         *limitations
         */
        XSTL_NODISCARD size_type max_size() const noexcept {
            return std::min<size_type>((std::numeric_limits<difference_type>::max)(), _Alnode_traits::max_size(_Getal()));
        }

        /**
         *	@brief checks if the container has no elements
         *	@return true if the container is empty, false otherwise
         */
        XSTL_NODISCARD bool empty() const noexcept;

        void swap(_Bs_tree& tree) noexcept(std::is_nothrow_swappable_v<key_compare>);

        /*
         *	@brief returns the function object that compares the keys, which is a copy of this container's constructor argument
         *comp
         *	@return the key comparison function object
         */
        XSTL_NODISCARD key_compare key_comp() const { return key_compare(); }
        /*
         *	@brief returns a function object that compares objects of type
         *	@return the key comparison function object
         */
        XSTL_NODISCARD value_compare value_comp() const { return value_compare(); }

        /**
         *	@brief removes all elements from the bs_tree.
         */
        void clear() noexcept;

        ~_Bs_tree() {
            clear();
            _Node::destroy_node(_Getal(), _Get_root());
        }

        template <class _Traits, template <class, class> class... _MixIn>
        XSTL_NODISCARD friend bool operator==(const _Bs_tree<_Traits, _MixIn...>& lhs, const _Bs_tree<_Traits, _MixIn...>& rhs) {
            using _Nodeptr = typename _Traits::_Nodeptr;
            using _Node    = typename _Traits::_Node;
            if (lhs.size() != rhs.size())
                return false;
            _Nodeptr _curr1 = lhs._Get_root()->_left, _curr2 = rhs._Get_root()->_left;
            for (;;) {
                if (_curr1->_is_nil)
                    return _curr2->_is_nil;
                if (_curr2->_is_nil)
                    return false;
                if (!std::equal_to<>{}(_curr1->_value, _curr2->_value))
                    return false;

                _curr1 = _Node::find_inorder_successor(_curr1);
                _curr2 = _Node::find_inorder_successor(_curr2);
            }
        }

#ifdef __cpp_lib_concepts
        template <class _Traits, template <class, class> class... _MixIn>
        XSTL_NODISCARD friend synth_three_way_result<typename _Traits::value_type>
        operator<=>(const _Bs_tree<_Traits, _MixIn...>& lhs, const _Bs_tree<_Traits, _MixIn...>& rhs) {
            using _Nodeptr  = typename _Traits::_Nodeptr;
            using _Node     = typename _Traits::_Node;
            _Nodeptr _curr1 = lhs._Get_root()->_left, _curr2 = rhs._Get_root()->_left;
            auto     _cmpr = synth_three_way{};
            for (;;) {
                if (_curr1->_is_nil)
                    return _curr2->_is_nil ? std::strong_ordering::equal : std::strong_ordering::less;
                if (_curr2->_is_nil)
                    return std::strong_ordering::greater;
                if (const auto _res = _cmpr(_curr1->_value, _curr2->_value); _res != 0)
                    return _res;

                _curr1 = _Node::find_inorder_successor(_curr1);
                _curr2 = _Node::find_inorder_successor(_curr2);
            }
        }
#else
        template <class _Traits, template <class, class> class... _MixIn>
        XSTL_NODISCARD friend bool operator!=(const _Bs_tree<_Traits, _MixIn...>& lhs, const _Bs_tree<_Traits, _MixIn...>& rhs) {
            return !(lhs == rhs);
        }

        template <class _Traits, template <class, class> class... _MixIn>
        XSTL_NODISCARD friend bool operator<(const _Bs_tree<_Traits, _MixIn...>& lhs, const _Bs_tree<_Traits, _MixIn...>& rhs) {
            using _Nodeptr  = typename _Traits::_Nodeptr;
            using _Node     = typename _Traits::_Node;
            _Nodeptr _curr1 = lhs._Get_root()->_left, _curr2 = rhs._Get_root()->_left;
            while (!_curr1->_is_nil && !_curr2->_is_nil) {
                if (std::less<>{}(_curr1->_value, _curr2->_value))
                    return true;
                else if (std::less<>{}(_curr2->_value, _curr1->_value))
                    return false;
                _curr1 = _Node::find_inorder_successor(_curr1);
                _curr2 = _Node::find_inorder_successor(_curr2);
            }

            return _curr1->_is_nil && !_curr2->_is_nil;
        }

        template <class _Traits, template <class, class> class... _MixIn>
        XSTL_NODISCARD friend bool operator>(const _Bs_tree<_Traits, _MixIn...>& lhs, const _Bs_tree<_Traits, _MixIn...>& rhs) {
            return rhs < lhs;
        }

        template <class _Traits, template <class, class> class... _MixIn>
        XSTL_NODISCARD friend bool operator<=(const _Bs_tree<_Traits, _MixIn...>& lhs, const _Bs_tree<_Traits, _MixIn...>& rhs) {
            return !(rhs < lhs);
        }

        template <class _Traits, template <class, class> class... _MixIn>
        XSTL_NODISCARD friend bool operator>=(const _Bs_tree<_Traits, _MixIn...>& lhs, const _Bs_tree<_Traits, _MixIn...>& rhs) {
            return !(lhs < rhs);
        }
#endif

        _Bs_tree& operator=(const _Bs_tree&);
        _Bs_tree&
        operator=(_Bs_tree&&) noexcept(_Alnode_traits::is_always_equal::value&& std::is_nothrow_move_assignable_v<key_compare>);

    private:
        enum class _Inspos { LEFT, RIGHT };

        struct _Inspack {
            _Nodeptr _parent;
            _Inspos  _pos;
        };

        struct _Find_result {
            _Inspack _pack;
            _Nodeptr _curr;
        };

        struct _Find_hint_result {
            _Inspack _pack;
            bool     _insertable;
        };

        void _Destroy(_Nodeptr node) noexcept {
            while (!node->_is_nil) {
                _Destroy(node->_right);
                _Node::destroy_node(_Getal(), std::exchange(node, node->_left));
            }
        }
        void _Init() { _Get_val()._root = _Node::create_root(_Getal()); }
        template <class _Key>
        _Find_result _Lower_bound(const _Key& value) const;
        template <class _Key>
        _Find_result _Upper_bound(const _Key& value) const;
        template <class _Key>
        std::pair<_Nodeptr, _Nodeptr> _Equal_range(const _Key& value) const noexcept(
            is_nothrow_comparable_v<key_compare, value_type, _Key>&& is_nothrow_comparable_v<key_compare, _Key, value_type>);

        inline _Nodeptr _Insert_at(const _Inspack&, _Nodeptr);
        template <class... _Args>
        std::pair<iterator, bool> _Emplace(_Args&&...);
        template <class _Key>
        _Find_hint_result _Find_hint(const _Nodeptr, const _Key&);
        template <class... _Args>
        iterator _Emplace_hint(_Nodeptr, _Args&&...);
        template <class _Tag>
        void _Copy(const _Self&);
        template <class _Tag>
        _Nodeptr    _Copy_nodes(_Nodeptr, _Nodeptr);
        _Nodeptr    _Erase(_Nodeptr);
        inline void _Check_max_size(const char* msg = "map/set too long") const {
            if (max_size() == _size)
                throw std::length_error(msg);
        }
        void _Swap_excluding_cmpr(_Self& other) {
            using std::swap;
            _Get_val().swap(other._Get_val());
            swap(_size, _size);
        }

        inline iterator       _Make_iter(_Nodeptr node) const noexcept { return iterator(node, std::addressof(_Get_val())); }
        inline const_iterator _Make_citer(_Nodeptr node) const noexcept {
            return const_iterator(node, std::addressof(_Get_val()));
        }

        inline _Unchecked_const_iterator _Make_unchecked_citer(_Nodeptr node) const noexcept {
            return _Unchecked_const_iterator(node, std::addressof(_Get_val()));
        }

        inline key_compare&        _Get_cmpr() noexcept { return std::get<0>(_tpl); }
        inline const key_compare&  _Get_cmpr() const noexcept { return std::get<0>(_tpl); }
        inline _Alnode_type&       _Getal() noexcept { return std::get<1>(_tpl); }
        inline const _Alnode_type& _Getal() const noexcept { return std::get<1>(_tpl); }
        inline _Scary_val&         _Get_val() noexcept { return std::get<2>(_tpl); }
        inline const _Scary_val&   _Get_val() const noexcept { return std::get<2>(_tpl); }
        inline _Nodeptr            _Get_root() noexcept { return std::get<2>(_tpl)._root; }
        inline const _Nodeptr      _Get_root() const noexcept { return std::get<2>(_tpl)._root; }

        compressed_tuple<key_compare, _Alnode_type, _Scary_val> _tpl;
        size_type                                               _size = 0;
    };

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Tag>
    void _Bs_tree<_Traits, _MixIn...>::_Copy(const _Self& other) {
        _Nodeptr _root = _Get_root();
        _root->_parent = _Copy_nodes<_Tag>(other._Get_root()->_parent, _root);
        _size          = other._size;
        if (!_root->_parent->_is_nil) {  // nonempty tree, look for new smallest and largest
            _root->_left  = _Node::leftmost(_root->_parent);
            _root->_right = _Node::rightmost(_root->_parent);
        }
        else {  // empty tree, just tidy head pointers
            _root->_left  = _root;
            _root->_right = _root;
        }
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Tag>
    typename _Bs_tree<_Traits, _MixIn...>::_Nodeptr _Bs_tree<_Traits, _MixIn...>::_Copy_nodes(_Nodeptr src, _Nodeptr dst) {
        _Nodeptr _subroot = _Get_root();
        if (!src->_is_nil) {
            _Nodeptr _node;
            if constexpr (std::is_same_v<_Tag, copy_op_tag>)
                _node = _Node::create_node(_Getal(), _subroot, src->_value);
            else {
                if constexpr (std::is_same_v<key_type, value_type>)  // is set
                    _node = _Node::create_node(_Getal(), _subroot, std::move(src->_value));
                else  // is map
                    _node = _Node::create_node(_Getal(), _subroot, std::move(src->_value->first), std::move(src->_value->second));
            }
            _node->_parent = dst;
            _node->_prop   = src->_prop;
            if (_subroot->_is_nil)
                _subroot = _node;
            try {
                _node->_left  = _Copy_nodes<_Tag>(src->_left, _node);
                _node->_right = _Copy_nodes<_Tag>(src->_right, _node);
            } catch (...) {
                _Destroy(_node);
                throw;
            }
        }
        return _subroot;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::_Nodeptr _Bs_tree<_Traits, _MixIn...>::_Erase(_Nodeptr node) {
        const _Nodeptr _root = _Get_root();
        _Nodeptr       _suc  = nullptr;
        if (_root->_left == node)
            _suc = _root->_left = node->_right->_is_nil ? node->_parent : _Node::leftmost(node->_right);
        if (_root->_right == node)
            _suc = _root->_right = node->_left->_is_nil ? node->_parent : _Node::rightmost(node->_left);
        if (_root->_parent == node)
            _root->_parent = _suc ? _suc : node->_right->_is_nil ? _root : _Node::leftmost(node->_right);
        _Nodeptr _parent = _Traits::extract_node(this, node);
        _Node::destroy_node(_Getal(), node);
        return _parent;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::_Nodeptr _Bs_tree<_Traits, _MixIn...>::_Insert_at(const _Inspack& pack,
                                                                                             _Nodeptr        new_node) {
        _Nodeptr _root    = _Get_root();
        new_node->_parent = pack._parent;
        if (pack._parent == _root)
            _root->_parent = _root->_left = _root->_right = new_node;
        else {
            if (pack._pos == _Inspos::LEFT) {
                pack._parent->_left = new_node;
                if (pack._parent == _root->_left)
                    _root->_left = new_node;
            }
            else {
                pack._parent->_right = new_node;
                if (pack._parent == _root->_right)
                    _root->_right = new_node;
            }
        }
        _Traits::insert_fixup(this, new_node);
        ++_size;
        return new_node;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class... _Args>
    std::pair<typename _Bs_tree<_Traits, _MixIn...>::iterator, bool> _Bs_tree<_Traits, _MixIn...>::_Emplace(_Args&&... values) {
        using _In_place_key_extractor =
            typename _Traits::template _In_place_key_extractor<std::remove_cv_t<std::remove_reference_t<_Args>>...>;

        _Nodeptr     _new_node;
        _Find_result _res;
        if constexpr (_In_place_key_extractor::extractable && !_Multi) {
            const auto& _key = _In_place_key_extractor::extract(values...);
            _res             = _Lower_bound(_key);
            if (!_res._curr->_is_nil && !_Get_cmpr()(_key, KFN(_res._curr)))  // key has existed in the tree
                return { _Make_iter(_res._curr), false };
            _Check_max_size();
            _new_node = _Tree_temp_node<_Alnode_type>(_Getal(), _Get_root(), std::forward<_Args>(values)...).release();
        }
        else {
            _Tree_temp_node<_Alnode_type> _tmp_node(_Getal(), _Get_root(), std::forward<_Args>(values)...);
            const key_type&               _key = _Traits::kfn(_tmp_node._node->_value);
            if constexpr (_Multi)
                _res = _Upper_bound(_key);
            else {
                _res = _Lower_bound(_key);
                if (!_res._curr->_is_nil && !_Get_cmpr()(_key, KFN(_res._curr)))  // key has existed in the tree
                    return { _Make_iter(_res._curr), false };
            }
            _Check_max_size();
            _new_node = _tmp_node.release();
        }
        return { _Make_iter(_Insert_at(_res._pack, _new_node)), true };
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key>
    typename _Bs_tree<_Traits, _MixIn...>::_Find_hint_result _Bs_tree<_Traits, _MixIn...>::_Find_hint(const _Nodeptr hint,
                                                                                                      const _Key&    key) {
        if (empty())
            return { { hint }, true };
        _Nodeptr _curr = hint;
        if (hint->_is_nil)
            _curr = _Get_root()->_right;
        if (_Get_cmpr()(key, KFN(_curr))) {  // if key < curr.key, may insert at position where before hint
            for (_Nodeptr _pre = _Node::find_inorder_predecessor(_curr); !_pre->_is_nil;
                 _pre          = _Node::find_inorder_predecessor(_pre)) {
                if (!_Get_cmpr()(key, KFN(_pre))) {    // if pre.key <= key
                    if (!_Get_cmpr()(KFN(_pre), key))  // if pre.key == key
                        if constexpr (!_Multi)
                            return { { _pre }, false };
                    return { { _pre, _Inspos::RIGHT }, true };
                }
            }
            return { { _Get_root()->_left, _Inspos::LEFT }, true };
        }
        else {  // key >= curr.key
            for (_Nodeptr _suc = nullptr;; _curr = _suc) {
                if (!_Get_cmpr()(KFN(_curr), key)) {  // if key == curr.key
                    if constexpr (_Multi) {
                        do {
                            if (_curr->_right->_is_nil)
                                return { { _curr, _Inspos::RIGHT }, true };
                        } while (!_Get_cmpr()(key, KFN(_curr->_right))
                                 && (_curr = _curr->_right, true));  // travelling while suc->_right.key == key
                        return { { _curr, _Inspos::LEFT }, true };
                    }
                    else
                        return { { _curr }, false };
                }
                // key > curr.key
                _suc = _Node::find_inorder_successor(_curr);
                if (!_suc->_is_nil) {
                    if (_Get_cmpr()(key, KFN(_suc)))  // key < suc.key
                        return { { _suc, _Inspos::LEFT }, true };
                }
                else
                    return { { _Get_root()->_right, _Inspos::RIGHT }, true };
            }
        }
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class... _Args>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::_Emplace_hint(_Nodeptr hint,
                                                                                                _Args&&... values) {
        using _In_place_key_extractor =
            typename _Traits::template _In_place_key_extractor<std::remove_cv_t<std::remove_reference_t<_Args>>...>;
        _Nodeptr          _new_node;
        _Find_hint_result _res;
        if constexpr (_In_place_key_extractor::extractable && !_Multi) {
            _res = _Find_hint(hint, _In_place_key_extractor::extract(values...));
            if (!_res._insertable)
                return _Make_iter(_res._pack._parent);
            _Check_max_size();
            _new_node = _Tree_temp_node<_Alnode_type>(_Getal(), _Get_root(), std::forward<_Args>(values)...).release();
        }
        else {
            _Tree_temp_node<_Alnode_type> _tmp_node(_Getal(), _Get_root(), std::forward<_Args>(values)...);
            _res = _Find_hint(hint, _Traits::kfn(_tmp_node._node->_value));
            if constexpr (!_Multi)
                if (!_res._insertable)
                    return _Make_iter(_res._pack._parent);
            _Check_max_size();
            _new_node = _tmp_node.release();
        }
        return _Make_iter(_Insert_at(_res._pack, _new_node));
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::erase(const_iterator position) noexcept {
        XSTL_EXPECT(std::addressof(_Get_val()) == CAST2SCARY(position._Get_cont()), "tree iterator insert outside range");

        if (position.base()->_is_nil)
            return end();
        _Nodeptr _curr = (position++).base();
        _Traits::erase_fixup(this, _Erase(_curr));
        --_size;
        return _Make_iter(position.base());
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::erase(const_iterator first,
                                                                                        const_iterator last) noexcept {
        XSTL_EXPECT(std::addressof(_Get_val()) == CAST2SCARY(first._Get_cont())
                        && std::addressof(_Get_val()) == CAST2SCARY(last._Get_cont()),
                    "tree iterator insert outside range");

        if (first == begin() && last == end())
            clear();
        else {
            while (first != last)
                erase(first++);
            return _Make_iter(first.base());
        }
        return begin();
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key>
    typename _Bs_tree<_Traits, _MixIn...>::_Find_result _Bs_tree<_Traits, _MixIn...>::_Lower_bound(const _Key& key) const {
        _Nodeptr     _curr = _Get_root()->_parent;
        _Find_result _res{ { _Get_root()->_parent }, const_cast<_Nodeptr>(_Get_root()) };
        while (!_curr->_is_nil) {
            _res._pack._parent = _curr;
            if (!_Get_cmpr()(KFN(_curr), key)) {  // curr.key >= key
                _res._pack._pos = _Inspos::LEFT;
                _res._curr      = _curr;
                _curr           = _curr->_left;
            }
            else {
                _res._pack._pos = _Inspos::RIGHT;
                _curr           = _curr->_right;
            }
        }
        return _res;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    auto _Bs_tree<_Traits, _MixIn...>::insert(node_type&& nh) {
        if (nh.empty()) {
            if constexpr (_Multi)
                return end();
            else
                return insert_return_type{ end(), false, {} };
        }
        const _Nodeptr _new_node = _Tree_accessor::get_ptr(nh);
        if constexpr (_Multi) {
            _Check_max_size();
            return _Make_iter(_Insert_at(_Upper_bound(KFN(_new_node))._pack, _Tree_accessor::release(nh)));
        }
        else {
            const auto _res = _Lower_bound(KFN(_new_node));
            if (!_res._curr->_is_nil && !_Get_cmpr()(KFN(_new_node), KFN(_res._curr)))
                return insert_return_type{ _Make_iter(_res._curr), false, std::move(nh) };
            _Check_max_size();
            return insert_return_type{ _Make_iter(_Insert_at(_res._pack, _Tree_accessor::release(nh))), true, std::move(nh) };
        }
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::insert(const_iterator hint, node_type&& nh) {
        XSTL_EXPECT(std::addressof(_Get_val()) == CAST2SCARY(hint._Get_cont()), "tree iterator insert outside range");

        if (nh.empty())
            return end();
        const _Nodeptr _new_node = _Tree_accessor::get_ptr(nh);
        const auto     _res      = _Find_hint(hint.base(), KFN(_new_node));
        if (_res.insertable == false)
            return _Make_iter(_res._pack._parent);
        _Check_max_size();
        return _Make_iter(_Insert_at(_res._pack, _Tree_accessor::release(nh)));
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::lower_bound(const key_type& key) {
        return _Make_iter(_Lower_bound(key)._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::const_iterator _Bs_tree<_Traits, _MixIn...>::lower_bound(const key_type& key) const {
        return _Make_citer(_Lower_bound(key)._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::lower_bound(const _Key& key) {
        return _Make_iter(_Lower_bound(key)._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    typename _Bs_tree<_Traits, _MixIn...>::const_iterator _Bs_tree<_Traits, _MixIn...>::lower_bound(const _Key& key) const {
        return _Make_citer(_Lower_bound(key)._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key>
    typename _Bs_tree<_Traits, _MixIn...>::_Find_result _Bs_tree<_Traits, _MixIn...>::_Upper_bound(const _Key& key) const {
        _Nodeptr     _curr = _Get_root()->_parent;
        _Find_result _res{ { _Get_root()->_parent }, const_cast<_Nodeptr>(_Get_root()) };
        while (!_curr->_is_nil) {
            _res._pack._parent = _curr;
            if (_Get_cmpr()(key, KFN(_curr))) {  // curr.key > key
                _res._pack._pos = _Inspos::LEFT;
                _res._curr      = _curr;
                _curr           = _curr->_left;
            }
            else {
                _res._pack._pos = _Inspos::RIGHT;
                _curr           = _curr->_right;
            }
        }
        return _res;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::upper_bound(const key_type& key) {
        return _Make_iter(_Upper_bound(key)._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::const_iterator _Bs_tree<_Traits, _MixIn...>::upper_bound(const key_type& key) const {
        return _Make_citer(_Upper_bound(key)._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::upper_bound(const _Key& key) {
        return _Make_iter(_Upper_bound(key)._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    typename _Bs_tree<_Traits, _MixIn...>::const_iterator _Bs_tree<_Traits, _MixIn...>::upper_bound(const _Key& key) const {
        return _Make_citer(_Upper_bound(key)._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key>
    std::pair<typename _Bs_tree<_Traits, _MixIn...>::_Nodeptr, typename _Bs_tree<_Traits, _MixIn...>::_Nodeptr>
    _Bs_tree<_Traits, _MixIn...>::_Equal_range(const _Key& key) const noexcept(
        is_nothrow_comparable_v<key_compare, value_type, _Key>&& is_nothrow_comparable_v<key_compare, _Key, value_type>) {
        auto _first = _Get_root(), _second = _Get_root();
        auto _curr = _Get_root()->_parent;
        while (!_curr->_is_nil)
            if (_Get_cmpr()(KFN(_curr), key))
                _curr = _curr->_right;
            else {
                if (_second->_is_nil && _Get_cmpr()(key, KFN(_curr)))
                    _second = _curr;
                _first = std::exchange(_curr, _curr->_left);
            }
        _curr = _second->_is_nil ? _Get_root()->_parent : _second->_left;
        while (!_curr->_is_nil)
            if (_Get_cmpr()(key, KFN(_curr)))
                _second = std::exchange(_curr, _curr->_left);
            else
                _curr = _curr->_right;
        return { const_cast<_Nodeptr>(_first), const_cast<_Nodeptr>(_second) };
    }

    template <class _Traits, template <class, class> class... _MixIn>
    std::pair<typename _Bs_tree<_Traits, _MixIn...>::iterator, typename _Bs_tree<_Traits, _MixIn...>::iterator>
    _Bs_tree<_Traits, _MixIn...>::equal_range(const key_type& key) {
        const auto _res = _Equal_range(key);
        return { _Make_iter(_res.first), _Make_iter(_res.second) };
    }

    template <class _Traits, template <class, class> class... _MixIn>
    std::pair<typename _Bs_tree<_Traits, _MixIn...>::const_iterator, typename _Bs_tree<_Traits, _MixIn...>::const_iterator>
    _Bs_tree<_Traits, _MixIn...>::equal_range(const key_type& key) const {
        const auto _res = _Equal_range(key);
        return { _Make_citer(_res.first), _Make_citer(_res.second) };
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    std::pair<typename _Bs_tree<_Traits, _MixIn...>::iterator, typename _Bs_tree<_Traits, _MixIn...>::iterator>
    _Bs_tree<_Traits, _MixIn...>::equal_range(const _Key& key) {
        const auto _res = _Equal_range(key);
        return { _Make_iter(_res.first), _Make_iter(_res.second) };
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    std::pair<typename _Bs_tree<_Traits, _MixIn...>::const_iterator, typename _Bs_tree<_Traits, _MixIn...>::const_iterator>
    _Bs_tree<_Traits, _MixIn...>::equal_range(const _Key& key) const {
        const auto _res = _Equal_range(key);
        return { _Make_citer(_res.first), _Make_citer(_res.second) };
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::size_type _Bs_tree<_Traits, _MixIn...>::count(const key_type& key) const {
        if constexpr (_Multi) {
            const auto _res = _Equal_range(key);
            return std::distance(_Make_unchecked_citer(_res.first), _Make_unchecked_citer(_res.second));
        }
        if (const auto _curr = _Lower_bound(key)._curr; !_curr->_is_nil)
            if (!_Get_cmpr()(key, KFN(_curr)))
                return 1;
        return 0;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    typename _Bs_tree<_Traits, _MixIn...>::size_type _Bs_tree<_Traits, _MixIn...>::count(const _Key& key) const {
        const auto _res = _Equal_range(key);
        return std::distance(_Make_unchecked_citer(_res.first), _Make_unchecked_citer(_res.second));
    }

    template <class _Traits, template <class, class> class... _MixIn>
    bool _Bs_tree<_Traits, _MixIn...>::contains(const key_type& key) const {
        return !find(key).base()->_is_nil;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::node_type _Bs_tree<_Traits, _MixIn...>::extract(const_iterator position) {
        XSTL_EXPECT(std::addressof(_Get_val()) == CAST2SCARY(position._Get_cont()), "tree iterator insert outside range");

        _Traits::extract_node(this, position.base());
        return _Tree_accessor::make_handle<node_type>(position.base(), _Getal());
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::node_type _Bs_tree<_Traits, _MixIn...>::extract(const key_type& key) {
        const const_iterator _res = find(key);
        return _res == end() ? node_type{} : extract(_res);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    bool _Bs_tree<_Traits, _MixIn...>::contains(const _Key& key) const {
        return !find(key).base()->_is_nil;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Other_traits>
    void _Bs_tree<_Traits, _MixIn...>::merge(_Bs_tree<_Other_traits, _MixIn...>& x) {
        static_assert(std::is_same_v<_Nodeptr, typename _Bs_tree<_Other_traits, _MixIn...>::_Nodeptr>,
                      "merge() requires an argument with a compatible node type.");
        static_assert(std::is_same_v<allocator_type, typename _Bs_tree<_Other_traits, _MixIn...>::allocator_type>,
                      "merge() requires an argument with the same allocator type.");

        if constexpr (std::is_same_v<_Bs_tree, _Bs_tree<_Other_traits, _MixIn...>>)
            if XSTL_UNLIKELY (this == std::addressof(x))
                return;
        if constexpr (!_Alnode_traits::is_always_equal::value)
            XSTL_EXPECT(_Getal() != x._Getal(), "tree allocators incompatible for merge");

        _Nodeptr _curr = _Tree_accessor::root(std::addressof(x))->_left;
        while (!_curr->_is_nil) {
            const _Nodeptr _node = _curr;
            if (!_curr->_right->_is_nil)  // increase _curr, find in-order successor
                _curr = _Node::leftmost(_curr->_right);
            else {
                while (!_curr->_parent->_is_nil && _curr == _curr->_parent->_right)
                    _curr = _curr->_parent;
                _curr = _curr->_parent;
            }
            _Find_result _res;
            if constexpr (_Multi)
                _res = _Upper_bound(KFN(_node));
            else {
                _res = _Lower_bound(KFN(_node));
                if (!_res._curr->_is_nil && !_Get_cmpr()(KFN(_node), KFN(_res._curr)))
                    continue;
            }
            _Check_max_size();
            _Other_traits::extract_node(std::addressof(x), _node);
            _Insert_at(_res._pack, _node);
        }
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::size_type _Bs_tree<_Traits, _MixIn...>::height() const noexcept {
#ifdef __cpp_explicit_this_parameter
        auto _get_height = [&](this auto self,
#else
        std::function<size_type(_Nodeptr)> _get_height = [&](
#endif
                               _Nodeptr node) {
            return node->_is_nil ? 0
                                 : (std::max)(
#ifdef __cpp_explicit_this_parameter
                                       self(node->_left), self(node->right)
#else
                                       _get_height(node->_left), _get_height(node->_right)
#endif
                                           )
                                       + 1;
        };
        return _get_height(_Get_root()->_parent);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::size_type _Bs_tree<_Traits, _MixIn...>::width() const {
        if (empty())
            return 0;
        std::queue<_Nodeptr> _wid;
        size_type            _w = 1, _curw = _w;
        _wid.push(_Get_root()->_parent);
        while (!_wid.empty()) {
            for (; _curw > 0; --_curw) {
                _Nodeptr _front = _wid.front();
                _wid.pop();
                if (!_front->_left->_is_nil)
                    _wid.push(_front->_left);
                if (!_front->_right->_is_nil)
                    _wid.push(_front->_right);
            }
            _curw = _wid.size();
            if (_curw > _w)
                _w = _curw;
        }
        return _w;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Elem, class _ElemTraits>
    void _Bs_tree<_Traits, _MixIn...>::display(std::basic_ostream<_Elem, _ElemTraits>& out) const {
        constexpr size_type MAX_DEPTH = 128;
        bool                _visited[MAX_DEPTH]{};
#ifdef __cpp_explicit_this_parameter
        auto _display = [&](this auto self,
#else
        std::function<void(_Nodeptr, size_type, bool)> _display = [&](
#endif
                            _Nodeptr node, size_type size, bool position) {
            if (size > MAX_DEPTH)
                return;
            for (size_type i = 1; i < size; i++)
                out << (_visited[i] ? static_cast<const _Elem*>("  │") : static_cast<const _Elem*>("   "));
            if (node->_is_nil)
                out << (position ? static_cast<const _Elem*>("  ├─ ") : static_cast<const _Elem*>("  └─ "))
                    << static_cast<_Elem>('\n');
            else {
                if (node->_parent == _Get_root())
                    out << static_cast<const _Elem*>("└─ ");
                else if (node->is_left())
                    out << static_cast<const _Elem*>("  └─ ");
                else
                    out << static_cast<const _Elem*>("  ├─ ");
                out << node->_value << static_cast<_Elem>('\n');
                if (!node->_left->_is_nil || !node->_right->_is_nil) {
                    _visited[size + 1] = true;
#ifdef __cpp_explicit_this_parameter
                    self(node->_right, size + 1, 1);
                    _visited[size + 1] = false;
                    self(node->_left, size + 1, 0);
#else
                    _display(node->_right, size + 1, 1);
                    _visited[size + 1] = false;
                    _display(node->_left, size + 1, 0);
#endif
                }
            }
        };
        _display(_Get_root()->_parent, 0, 0);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::find(const key_type& key) {
        auto _res = _Lower_bound(key);
        return (_res._curr->_is_nil || _Get_cmpr()(key, KFN(_res._curr)))
                   ? end()
                   : (_Traits::access_fixup(this, _res._curr), _Make_iter(_res._curr));
    }

    template <class _Traits, template <class, class> class... _MixIn>
    typename _Bs_tree<_Traits, _MixIn...>::const_iterator _Bs_tree<_Traits, _MixIn...>::find(const key_type& key) const {
        auto _res = _Lower_bound(key);
        return (_res._curr->_is_nil || _Get_cmpr()(key, KFN(_res._curr))) ? cend() : _Make_citer(_res._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    typename _Bs_tree<_Traits, _MixIn...>::iterator _Bs_tree<_Traits, _MixIn...>::find(const _Key& key) {
        auto _res = _Lower_bound(key);
        return (_res._curr->_is_nil || _Get_cmpr()(key, KFN(_res._curr)))
                   ? end()
                   : (_Traits::access_fixup(this, _res._curr), _Make_iter(_res._curr));
    }

    template <class _Traits, template <class, class> class... _MixIn>
    template <class _Key, class, class>
    typename _Bs_tree<_Traits, _MixIn...>::const_iterator _Bs_tree<_Traits, _MixIn...>::find(const _Key& key) const {
        auto _res = _Lower_bound(key);
        return (_res._curr->_is_nil || _Get_cmpr()(key, KFN(_res._curr))) ? cend() : _Make_citer(_res._curr);
    }

    template <class _Traits, template <class, class> class... _MixIn>
    void _Bs_tree<_Traits, _MixIn...>::swap(_Bs_tree& x) noexcept(std::is_nothrow_swappable_v<key_compare>) {
        if XSTL_LIKELY (this != std::addressof(x)) {
            using std::swap;
            swap(_Get_cmpr(), x._Get_cmpr());
            _Swap_excluding_cmpr(x);
            alloc_pocs(_Getal(), x._Getal());
        }
    }

    template <class _Traits, template <class, class> class... _MixIn>
    void _Bs_tree<_Traits, _MixIn...>::clear() noexcept {
        _Destroy(_Get_root()->_parent);
        _Get_val().init();
        _size = 0;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    bool _Bs_tree<_Traits, _MixIn...>::empty() const noexcept {
        return _Get_root()->_parent->_is_nil;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    _Bs_tree<_Traits, _MixIn...>& _Bs_tree<_Traits, _MixIn...>::operator=(const _Bs_tree& rhs) {
        if XSTL_UNLIKELY (this == std::addressof(rhs))
            return *this;

        auto& _al       = _Getal();
        auto& _other_al = rhs._Getal();
        clear();
        _Get_cmpr() = rhs._Get_cmpr();
        if constexpr (alloc_pocca_v<_Alnode_type>) {
            if (_al != _other_al) {
                const _Nodeptr _new_root = _Node::create_root(_other_al);
                _Node::destroy_node(_al, _Get_root());
                _Get_root() = _new_root;
            }
        }

        alloc_pocca(_al, _other_al);
        _Copy<copy_op_tag>(rhs);

        return *this;
    }

    template <class _Traits, template <class, class> class... _MixIn>
    _Bs_tree<_Traits, _MixIn...>& _Bs_tree<_Traits, _MixIn...>::operator=(_Bs_tree&& rhs) noexcept(
        _Alnode_traits::is_always_equal::value&& std::is_nothrow_move_assignable_v<key_compare>) {
        if XSTL_UNLIKELY (this == std::addressof(rhs))
            return *this;

        auto& _al       = _Getal();
        auto& _other_al = rhs._Getal();

        constexpr auto _pocma_val = alloc_pocma_v<_Alnode_type>;
        clear();
        _Get_cmpr() = rhs._Get_cmpr();
        if constexpr (_pocma_val == pocma_values::Propagate) {
            if (_al != _other_al) {
                const _Nodeptr _new_root = std::exchange(rhs._Get_root(), _Node::create_root(_other_al));
                _Node::destroy_node(_al, _Get_root());
                alloc_pocma(_al, _other_al);
                _Get_root() = _new_root;
                _size       = std::exchange(rhs._size, size_type{ 0 });
                return *this;
            }
        }
        else if constexpr (_pocma_val == pocma_values::NoPropagate) {
            if (_al != _other_al) {
                _Copy<move_op_tag>(rhs);
                return *this;
            }
        }

        alloc_pocma(_al, _other_al);
        _Swap_excluding_cmpr(rhs);

        return *this;
    }

    template <class _Traits, class _Derived>
    class _Map {
    public:
        using allocator_type = typename _Traits::allocator_type;
        using key_compare    = typename _Traits::key_compare;
        using value_type     = typename _Traits::value_type;
        using key_type       = typename _Traits::key_type;
        using const_iterator = typename _Traits::const_iterator;
        using iterator       = typename _Traits::iterator;
        using mapped_type    = typename _Traits::value_type::second_type;

    private:
        using _Nodeptr   = typename _Traits::_Nodeptr;
        using _Scary_val = typename _Traits::_Scary_val;
        using _Alnode_type =
            typename std::allocator_traits<typename _Traits::allocator_type>::template rebind_alloc<_Tree_node<value_type>>;
        using _Alnode_traits = std::allocator_traits<_Alnode_type>;

    public:
        static_assert(!_Traits::_Multi && std::is_const_v<typename value_type::first_type>, "traits is not compatible with map");
        /**
        *	@brief  If a key equivalent to k already exists in the container, does nothing.
        Otherwise, behaves like emplace except that the element is constructed as value_type
        *	@param key : the key used both to look up and to insert if not found
        *	@param mapped_value : arguments to forward to the constructor of the element
        *	@return same as emplace
        */
        template <class... _Mapped>
        std::pair<iterator, bool> try_emplace(const key_type& key, _Mapped&&... mapped_value) {
            auto _res = _Try_emplace(key, std::forward<_Mapped>(mapped_value)...);
            return { _Tree_accessor::make_iter(_Derptr(), _res.first), _res.second };
        }
        template <class... _Mapped>
        std::pair<iterator, bool> try_emplace(key_type&& key, _Mapped&&... mapped_value) {
            const auto _res = _Try_emplace(std::move(key), std::forward<_Mapped>(mapped_value)...);
            return { _Tree_accessor::make_iter(_Derptr(), _res.first), _res.second };
        }

        /**
        *	@brief  If a key equivalent to k already exists in the container, does nothing.
        Otherwise, behaves like emplace_hint except that the element is constructed as value_type
        *	@param hint : iterator to the position before which the new element will be inserted.
        *	@param key : the key used both to look up and to insert if not found
        *	@param mapped_value : arguments to forward to the constructor of the element
        *	@return Same as emplace_hint
        */
        template <class... _Mapped>
        iterator try_emplace(const_iterator hint, const key_type& key, _Mapped&&... mapped_value) {
            XSTL_EXPECT(std::addressof(_Derptr()->_Get_val()) == CAST2SCARY(hint._Get_cont()),
                        "tree iterator insert outside range");

            return _Try_emplace_hint(hint.base(), key, std::forward<_Mapped>(mapped_value)...);
        }
        template <class... _Mapped>
        iterator try_emplace(const_iterator hint, key_type&& key, _Mapped&&... mapped_value) {
            XSTL_EXPECT(std::addressof(_Derptr()->_Get_val()) == CAST2SCARY(hint._Get_cont()),
                        "tree iterator insert outside range");

            return _Try_emplace_hint(hint.base(), std::move(key), std::forward<_Mapped>(mapped_value)...);
        }

        /**
        *	@brief If a key equivalent to key already exists in the container, assigns std::forward<_Mapped>(mapped_value) to the
        mapped_type corresponding to the key. If the key does not exist, inserts the new key as if by insert, constructing it from
        value_type(key, std::forward<_Mapped>(mapped_value))
        *	@param key : the key used both to look up and to insert if not
        found
        *	@param mapped_value : the key to insert or assign *	@param hint : iterator to the position before which the new
        element will be inserted
        *	@return The bool component is true if the insertion took place and false if the assignment
        took place. The iterator component is pointing at the element that was inserted or updated
        */
        template <class _Mapped>
        std::pair<iterator, bool> insert_or_assign(const key_type& key, _Mapped&& mapped_value) {
            return _Insert_or_assign(key, std::forward<_Mapped>(mapped_value));
        }

        template <class _Mapped>
        std::pair<iterator, bool> insert_or_assign(key_type&& key, _Mapped&& mapped_value) {
            return _Insert_or_assign(std::move(key), std::forward<_Mapped>(mapped_value));
        }

        template <class _Mapped>
        iterator insert_or_assign(const_iterator hint, const key_type& key, _Mapped&& mapped_value) {
            XSTL_EXPECT(std::addressof(_Derptr()->_Get_val()) == CAST2SCARY(hint._Get_cont()),
                        "tree iterator insert outside range");

            return _Insert_or_assign_hint(hint.base(), key, std::forward<_Mapped>(mapped_value));
        }

        template <class _Mapped>
        iterator insert_or_assign(const_iterator hint, key_type&& key, _Mapped&& mapped_value) {
            XSTL_EXPECT(std::addressof(_Derptr()->_Get_val()) == CAST2SCARY(hint._Get_cont()),
                        "tree iterator insert outside range");

            return _Insert_or_assign_hint(hint.base(), std::move(key), std::forward<_Mapped>(mapped_value));
        }

        /**
         *	@brief returns a reference to the mapped key of the element with key equivalent to key.
         *	@param key : the key of the element to find
         *	@return reference to the mapped key of the requested element.
         */
        mapped_type& at(const key_type& key) { return const_cast<mapped_type&>(const_cast<const _Map*>(this)->at(key)); }

        const mapped_type& at(const key_type& key) const {
            const auto _res = _Tree_accessor::find_lower_bound(_Derptr(), key);
            if (_res._curr->_is_nil || _Derptr()->key_comp()(key, KFN(_res._curr)))
                throw std::out_of_range("invalid map<K, T> key");
            return _res._curr->_value.second;
        }

        mapped_type& operator[](const key_type& key) { return _Try_emplace(key).first->_value.second; }
        mapped_type& operator[](key_type&& key) { return _Try_emplace(std::move(key)).first->_value.second; }

    private:
        _Derived* _Derptr() const noexcept { return const_cast<_Derived*>(static_cast<const _Derived*>(this)); }

        template <class _Key, class... _Mapped>
        std::pair<_Nodeptr, bool> _Try_emplace(_Key&& key, _Mapped&&... mapped_value) {
            const auto _res  = _Tree_accessor::find_lower_bound(_Derptr(), key);
            auto       _cmpr = _Derptr()->key_comp();
            if (!_res._curr->_is_nil && !_cmpr(key, KFN(_res._curr)))
                return { _res._curr, false };
            _Tree_accessor::check_max_size(_Derptr());
            // clang-format off
            const _Nodeptr _new_node = _Tree_temp_node<_Alnode_type>(_Tree_accessor::get_alnode(_Derptr()), 
                _Tree_accessor::root(_Derptr()), std::piecewise_construct,
                std::forward_as_tuple(std::forward<_Key>(key)), 
                std::forward_as_tuple(std::forward<_Mapped>(mapped_value)...)).release();
            // clang-format on
            return { _Tree_accessor::insert_at(_Derptr(), _res._pack, _new_node), true };
        }

        template <class _Key, class... _Mapped>
        iterator _Try_emplace_hint(_Nodeptr hint, _Key&& key, _Mapped&&... mapped_value) {
            const auto _res = _Tree_accessor::find_hint(_Derptr(), hint, key);
            if (!_res._insertable)
                return _Tree_accessor::make_iter(_Derptr(), _res._pack._parent);
            _Tree_accessor::check_max_size(_Derptr());
            // clang-format off
            const _Nodeptr _new_node = _Tree_temp_node<_Alnode_type>(_Tree_accessor::get_alnode(_Derptr()), 
                _Tree_accessor::root(_Derptr()), std::piecewise_construct,
                std::forward_as_tuple(std::forward<_Key>(key)), 
                std::forward_as_tuple(std::forward<_Mapped>(mapped_value)...)).release();
            // clang-format on
            return _Tree_accessor::make_iter(_Derptr(), _Tree_accessor::insert_at(_Derptr(), _res._pack, _new_node));
        }

        template <class _Key, class _Mapped>
        std::pair<iterator, bool> _Insert_or_assign(_Key&& key, _Mapped&& mapped_value) {
            const auto _res  = _Tree_accessor::find_lower_bound(_Derptr(), key);
            const auto _cmpr = _Derptr()->key_comp();
            if (!_res._curr->_is_nil && !_cmpr(key, KFN(_res._curr))) {
                _res._pack._parent->_value.second = std::forward<_Mapped>(mapped_value);
                return { _Tree_accessor::make_iter(_Derptr(), _res._pack._parent), false };
            }
            _Tree_accessor::check_max_size(_Derptr());
            // clang-format off
            const _Nodeptr _new_node =
                _Tree_temp_node<_Alnode_type>(_Tree_accessor::get_alnode(_Derptr()), 
                    _Tree_accessor::root(_Derptr()), 
                    std::forward<_Key>(key), 
                    std::forward<_Mapped>(mapped_value)).release();
            // clang-format on
            return { _Tree_accessor::make_iter(_Derptr(), _Tree_accessor::insert_at(_Derptr(), _res._pack, _new_node)), true };
        }

        template <class _Key, class _Mapped>
        iterator _Insert_or_assign_hint(_Nodeptr hint, _Key&& key, _Mapped&& mapped_value) {
            const auto _res = _Tree_accessor::find_hint(_Derptr(), hint, key);
            if (_res._insertable) {
                _res._pack._parent->_value.second = std::forward<_Mapped>(mapped_value);
                return _Tree_accessor::make_iter(_Derptr(), _res._pack._parent);
            }
            _Tree_accessor::check_max_size(_Derptr());
            // clang-format off
            const _Nodeptr _new_node =
                _Tree_temp_node<_Alnode_type>(_Tree_accessor::get_alnode(_Derptr()), 
                    _Tree_accessor::root(), 
                    std::forward<_Key>(key), 
                    std::forward<_Mapped>(mapped_value)).release();
            // clang-format on
            return _Tree_accessor::make_iter(_Derptr(), _Tree_accessor::insert_at(_Derptr(), _res._pack, _new_node));
        }

    protected:
        _Map() = default;
    };

    /**
     *	@class _Node_handle
     *	@brief the implementation of node_type
     */
    template <class _Node, class _Alloc, template <class...> class _Base, class... _Types>
    class _Node_handle : public _Base<_Node_handle<_Node, _Alloc, _Base, _Types...>, _Types...> {
        friend struct _Tree_accessor;
        using _Altp_traits   = std::allocator_traits<_Alloc>;
        using _Alnode_type   = typename _Altp_traits::template rebind_alloc<_Node>;
        using _Alnode_traits = std::allocator_traits<_Alnode_type>;
        using _Nodeptr       = typename _Alnode_traits::pointer;

    public:
        using allocator_type = _Alloc;
        using value_type     = typename _Altp_traits::value_type;

        constexpr _Node_handle() noexcept : _alloc_storage{} {}

        _Node_handle(_Node_handle&& x) noexcept : _node(std::exchange(x._node, nullptr)) {
            construct_in_place(_Getal(), std::move(x._Getal()));
            destroy_in_place(x._Getal());
        }

        _Node_handle& operator=(_Node_handle&& rhs) noexcept {
            if (!_node && rhs._node) {
                construct_in_place(_Getal(), std::move(rhs._Getal()));
                destroy_in_place(rhs._Getal());
                _node = std::exchange(rhs._node, nullptr);
            }
            else if (!rhs._node || this == std::addressof(rhs))
                _Clear();
            else {
                _Alnode_type _alnode(_Getal());
                _Node::destroy_node(_alnode, _node);
                alloc_pocma(_Getal(), rhs._Getal());
                destroy_in_place(rhs._Getal());
                _node = std::exchange(rhs._node, nullptr);
            }
            return *this;
        }

        XSTL_NODISCARD allocator_type get_allocator() const noexcept { return _Getal(); }

        explicit operator bool() const noexcept { return _node != nullptr; }

        XSTL_NODISCARD bool empty() const noexcept { return _node == nullptr; }

        void swap(_Node_handle& x) noexcept {
            if (_node) {
                if (x._node)
                    alloc_pocs(_Getal(), x._Getal());
                else {
                    construct_in_place(x._Getal(), std::move(_Getal()));
                    destroy_in_place(_Getal());
                }
            }
            else {
                if (!x._node)
                    return;
                construct_in_place(_Getal(), std::move(x._Getal()));
                destroy_in_place(x._Getal());
            }
            std::swap(_node, x._node);
        }

        ~_Node_handle() noexcept { _Clear(); }

    private:
        _Nodeptr _Release() noexcept {
            destroy_in_place(_Getal());
            return std::exchange(_node, nullptr);
        }

        _Nodeptr      _Getptr() const noexcept { return _node; }
        _Alloc&       _Getal() noexcept { return reinterpret_cast<_Alloc&>(_alloc_storage); }
        const _Alloc& _Getal() const noexcept { return reinterpret_cast<const _Alloc&>(_alloc_storage); }

        void _Clear() noexcept {
            if (_node) {
                _Alnode_type _alnode(_Getal());
                _Altp_traits::destroy(_Getal(), std::addressof(_node->_value));
                _Alnode_traits::deallocate(_alnode, _node, 1);
                destroy_in_place(_Getal());
                _node = nullptr;
            }
        }

        _Node_handle(const _Nodeptr ptr, const _Alloc& alloc) noexcept : _node(ptr) { construct_in_place(_Getal(), alloc); }

        _Nodeptr                      _node{};
        aligned_storage_for_t<_Alloc> _alloc_storage;  // equals to std::optional
    };

    struct _Tree_accessor {
        template <class _Traits, template <class, class> class... _MixIn>
        inline static auto /*_Nodeptr*/ root(_Bs_tree<_Traits, _MixIn...>* tree) noexcept {
            return tree->_Get_root();
        }

        template <class _Traits, template <class, class> class... _MixIn>
        inline static auto /*_Nodeptr*/ insert_at(_Bs_tree<_Traits, _MixIn...>*                          tree,
                                                  const typename _Bs_tree<_Traits, _MixIn...>::_Inspack& pack,
                                                  typename _Bs_tree<_Traits, _MixIn...>::_Nodeptr        new_node) noexcept {
            return tree->_Insert_at(pack, new_node);
        }

        template <class _Key, class _Traits, template <class, class> class... _MixIn>
        inline static auto /*_Find_result*/ find_lower_bound(_Bs_tree<_Traits, _MixIn...>* tree, const _Key& key) noexcept {
            return tree->_Lower_bound(key);
        }

        template <class _Key, class _Traits, template <class, class> class... _MixIn>
        inline static auto /*_Find_result*/ find_upper_bound(_Bs_tree<_Traits, _MixIn...>* tree, const _Key& key) noexcept {
            return tree->_Upper_bound(key);
        }

        template <class _Key, class _Traits, template <class, class> class... _MixIn>
        inline static auto /*_Find_hint_result*/ find_hint(_Bs_tree<_Traits, _MixIn...>*                   tree,
                                                           typename _Bs_tree<_Traits, _MixIn...>::_Nodeptr hint,
                                                           const _Key&                                     key) noexcept {
            return tree->_Find_hint(hint, key);
        }

        template <class _Node, class _Alloc, template <class...> class _Base, class... _Types>
        inline static auto /*_Node_handle::_Nodeptr*/ get_ptr(_Node_handle<_Node, _Alloc, _Base>& nh) {
            return nh._Getptr();
        }

        template <class _NodeHandle, class _Node, class _Alloc>
        inline static auto make_handle(const _Node node, const _Alloc& alloc) {
            return _NodeHandle{ node, alloc };
        }

        template <class _Node, class _Alloc, template <class...> class _Base, class... _Types>
        inline static auto /*_Node_handle::_Nodeptr*/ release(_Node_handle<_Node, _Alloc, _Base>& nh) {
            return nh._Release();
        }

        template <class _Traits, template <class, class> class... _MixIn>
        inline static auto /*iterator*/ make_iter(_Bs_tree<_Traits, _MixIn...>* tree, typename _Traits::_Nodeptr node) noexcept {
            return tree->_Make_iter(node);
        }

        template <class _Traits, template <class, class> class... _MixIn>
        inline static auto& /*_Alnode_type&*/ get_alnode(_Bs_tree<_Traits, _MixIn...>* tree) noexcept {
            return tree->_Getal();
        }

        template <class _Traits, template <class, class> class... _MixIn>
        inline static void check_max_size(_Bs_tree<_Traits, _MixIn...>* tree) {
            tree->_Check_max_size();
        }
    };

    namespace {
        template <class _Tp, class _Compare>
        struct _Set_traits {
            using key_type      = _Tp;
            using value_type    = key_type;
            using key_compare   = _Compare;
            using value_compare = key_compare;

            template <class _Derived>
            struct node_handle_base {
                value_type& value() const noexcept { return static_cast<const _Derived*>(this)->_Getptr()->_value; }
            };

            template <class... _Args>
            struct in_place_key_extract {
                static constexpr bool extractable = false;
            };

            template <>
            struct in_place_key_extract<_Tp> {
                static constexpr bool extractable = true;
                static const _Tp&     extract(const _Tp& value) noexcept { return value; }
            };

            static const _Tp& kfn(const _Tp& value) { return value; }
        };

        template <class _Key, class _Value, class _Compare>
        struct _Map_traits {
            using key_type    = _Key;
            using value_type  = std::pair<const _Key, _Value>;
            using key_compare = _Compare;
            struct value_compare {
                XSTL_NODISCARD bool operator()(const value_type& lhs, const value_type& rhs) const {
                    return key_compare()(lhs.first, rhs.first);
                }
            };

            template <class _Derived>
            struct node_handle_base {
                using mapped_type = _Value;

                key_type&    key() const noexcept { return static_cast<const _Derived*>(this)->_Getptr()->_value.first; }
                mapped_type& mapped() const noexcept { return static_cast<const _Derived*>(this)->_Getptr()->_value.second; }
            };

            template <class... Args>
            struct in_place_key_extract {
                static constexpr bool extractable = false;
            };

            template <>
            struct in_place_key_extract<_Key, _Value> {
                static constexpr bool extractable = true;
                static const _Key&    extract(const _Key& key, const _Value&) noexcept { return key; }
            };

            static const _Key& kfn(const std::pair<const _Key, _Value>& value) { return value.first; }
        };

        template <class _Cate, class _Alloc, bool _Mfl, class _CRTP = void>
        struct bs_traits {
            using key_type        = typename _Cate::key_type;
            using value_type      = typename _Cate::value_type;
            using key_compare     = typename _Cate::key_compare;
            using value_compare   = typename _Cate::value_compare;
            using _Node           = _Tree_node<value_type>;
            using _Nodeptr        = _Node*;
            using allocator_type  = _Alloc;
            using _Altp_traits    = std::allocator_traits<allocator_type>;
            using size_type       = typename _Altp_traits::size_type;
            using difference_type = typename _Altp_traits::difference_type;
            using pointer         = typename _Altp_traits::pointer;
            using const_pointer   = typename _Altp_traits::const_pointer;
            using reference       = value_type&;
            using const_reference = const value_type&;

            using _Scary_val = _Tree_val<iter_adapter::scary_iter_types<_Nodeptr, value_type, size_type, difference_type, pointer,
                                                                        const_pointer, reference, const_reference>>;
            using const_iterator = iter_adapter::bid_citer<_Scary_val>;
            using iterator =
                std::conditional_t<std::is_same_v<key_type, value_type>, const_iterator, iter_adapter::bid_iter<_Scary_val>>;

            using _Traits_category = _Cate;
            using node_type        = _Node_handle<_Tree_node<value_type>, _Alloc, typename _Cate::node_handle_base>;
            template <class... _Args>
            using _In_place_key_extractor = typename _Cate::template in_place_key_extract<_Args...>;

            static constexpr bool _Multi = _Mfl;

            static const auto& kfn(const typename _Cate::value_type& value) { return _Cate::kfn(value); }

            template <class _Traits, template <class, class> class... _MixIn>
            static void insert_fixup(_Bs_tree<_Traits, _MixIn...>* tree, _Nodeptr node) noexcept {}
            template <class _Crtp,
                      class = decltype(&std::declval<_Bs_tree<bs_traits<_Cate, _Alloc, _Mfl, _Crtp>, _MixIn...>>()::insert_fixup),
                      template <class, class> class... _MixIn>
            static void insert_fixup(_Bs_tree<bs_traits<_Cate, _Alloc, _Mfl, _Crtp>, _MixIn...>* tree, _Nodeptr node) {
                static_cast<_Crtp*>(this)->insert_fixup(tree, node);
            }

            template <class _Traits, template <class, class> class... _MixIn>
            static void erase_fixup(_Bs_tree<_Traits, _MixIn...>* tree, _Nodeptr node) noexcept {
                insert_fixup(tree, node);
            }
            template <class _Crtp,
                      class = decltype(&std::declval<_Bs_tree<bs_traits<_Cate, _Alloc, _Mfl, _Crtp>, _MixIn...>>()::erase_fixup),
                      template <class, class> class... _MixIn>
            static void erase_fixup(_Bs_tree<bs_traits<_Cate, _Alloc, _Mfl, _Crtp>, _MixIn...>* tree, _Nodeptr node) {
                static_cast<_Crtp*>(this)->erase_fixup(tree, node);
            }

            template <class _Traits, template <class, class> class... _MixIn>
            static void access_fixup(_Bs_tree<_Traits, _MixIn...>*, _Nodeptr) noexcept {
                // DO NOTHING
            }
            template <class _Crtp,
                      class = decltype(&std::declval<_Bs_tree<bs_traits<_Cate, _Alloc, _Mfl, _Crtp>, _MixIn...>>()::access_fixup),
                      template <class, class> class... _MixIn>
            static void access_fixup(_Bs_tree<bs_traits<_Cate, _Alloc, _Mfl, _Crtp>, _MixIn...>* tree, _Nodeptr node) {
                static_cast<_Crtp*>(this)->access_fixup(tree, node);
            }

            template <class _Traits, template <class, class> class... _MixIn>
            static _Nodeptr extract_node(_Bs_tree<_Traits, _MixIn...>* tree, _Nodeptr node) {
                const _Nodeptr _parent = extract_node_impl(tree, node), _root = _Tree_accessor::root(tree);
                _Node::assign_node(node, _root, _root, _root, BLACK, node->_is_nil);
                return _parent;
            }

        protected:
            template <class _Traits, template <class, class> class... _MixIn>
            inline static _Nodeptr rotate_left(_Bs_tree<_Traits, _MixIn...>* tree, _Nodeptr node) noexcept {
                _Nodeptr _pivot = node->_right;
                node->_right    = _pivot->_left;
                if (!_pivot->_left->_is_nil)
                    _pivot->_left->_parent = node;
                _pivot->_parent = node->_parent;
                if (node->is_real_root())
                    _Tree_accessor::root(tree)->_parent = _pivot;
                else
                    (node->is_left() ? node->_parent->_left : node->_parent->_right) = _pivot;
                _pivot->_left = node;
                node->_parent = _pivot;
                return _pivot;
            }

            template <class _Traits, template <class, class> class... _MixIn>
            inline static _Nodeptr rotate_right(_Bs_tree<_Traits, _MixIn...>* tree, _Nodeptr node) noexcept {
                _Nodeptr _pivot = node->_left;
                node->_left     = _pivot->_right;
                if (!_pivot->_right->_is_nil)
                    _pivot->_right->_parent = node;
                _pivot->_parent = node->_parent;
                if (node->is_real_root())
                    _Tree_accessor::root(tree)->_parent = _pivot;
                else
                    (node->is_left() ? node->_parent->_left : node->_parent->_right) = _pivot;
                _pivot->_right = node;
                node->_parent  = _pivot;
                return _pivot;
            }

            /**
             *	@brief take node from tree.the old position will be occupied by its successor.
             *	@param tree : current tree
             *	@param node : the node need to be taken.
             *	@param suc : the successor of node.
             */
            template <class _Traits, template <class, class> class... _MixIn>
            static void take_node(_Bs_tree<_Traits, _MixIn...>* tree, _Nodeptr node, _Nodeptr suc) noexcept {
                if (node->is_left())
                    node->_parent->_left = suc;
                else if (node->is_right())
                    node->_parent->_right = suc;
                if (!suc->_is_nil)
                    suc->_parent = node->_parent;
            }

            /**
             *	@brief extract node from tree and find its successor to replace the old node
             *	@param tree : current tree
             *	@param node : the node need to be extracted.
             *	@return parent of node.
             */
            template <
                class _Crtp,
                class = decltype(&std::declval<_Bs_tree<bs_traits<_Cate, _Alloc, _Mfl, _Crtp>, _MixIn...>>()::extract_node_impl),
                template <class, class> class... _MixIn>
            static _Nodeptr extract_node_impl(_Bs_tree<bs_traits<_Cate, _Alloc, _Mfl, _Crtp>, _MixIn...>* tree, _Nodeptr node) {
                return static_cast<_Crtp*>(this)->extract_node_impl(tree, node);
            }

            template <class _Traits, template <class, class> class... _MixIn>
            static _Nodeptr extract_node_impl(_Bs_tree<_Traits, _MixIn...>* tree, _Nodeptr node) noexcept {
                if (node->_left->_is_nil)
                    take_node(tree, node, node->_right);
                else if (node->_right->_is_nil)
                    take_node(tree, node, node->_left);
                else {
                    _Nodeptr _suc = _Node::leftmost(node->_right);
                    if (_suc->_parent != node) {
                        take_node(tree, _suc, _suc->_right);
                        _suc->_right          = node->_right;
                        _suc->_right->_parent = _suc;
                    }
                    take_node(tree, node, _suc);
                    _suc->_left          = node->_left;
                    _suc->_left->_parent = _suc;
                }
                return node->_parent;
            }
        };

        template <class _Cate, class _Alloc, bool _Mfl>
        struct avl_traits : public bs_traits<_Cate, _Alloc, _Mfl, avl_traits<_Cate, _Alloc, _Mfl>> {
            using _Self      = avl_traits<_Cate, _Alloc, _Mfl>;
            using _Base      = bs_traits<_Cate, _Alloc, _Mfl, _Self>;
            using _Nodeptr   = typename _Base::_Nodeptr;
            using _Node      = typename _Base::_Node;
            using value_type = typename _Base::value_type;

            using _Base::rotate_left;
            using _Base::rotate_right;

            template <template <class, class> class... _MixIn>
            static void insert_fixup(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                erase_fixup(tree, node);
            }

            template <template <class, class> class... _MixIn>
            static void erase_fixup(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                for (_Nodeptr _curr = node; !_curr->_is_nil; _curr = _curr->_parent) {
                    _Nodeptr _left = _curr->_left, _right = _curr->_right;
                    _curr->_prop = (std::max)(_left->_prop, _right->_prop) + 1;
                    if (_left->_prop - _right->_prop == 2) {
                        if (_left->_left->_prop < _left->_right->_prop)
                            _Rotate_left(tree, _left);
                        _Rotate_right(tree, _curr);
                    }
                    else if (_right->_prop - _left->_prop == 2) {
                        if (_right->_left->_prop > _right->_right->_prop)
                            _Rotate_right(tree, _right);
                        _Rotate_left(tree, _curr);
                    }
                }
            }

        private:
            template <template <class, class> class... _MixIn>
            inline static void _Rotate_left(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) noexcept {
                _Nodeptr _pivot = rotate_left(tree, node);
                node->_prop     = (std::max)(node->_left->_prop, node->_right->_prop) + 1;
                _pivot->_prop   = (std::max)(_pivot->_left->_prop, _pivot->_right->_prop) + 1;
            }

            template <template <class, class> class... _MixIn>
            inline static void _Rotate_right(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) noexcept {
                _Nodeptr _pivot = rotate_right(tree, node);
                node->_prop     = (std::max)(node->_left->_prop, node->_right->_prop) + 1;
                _pivot->_prop   = (std::max)(_pivot->_left->_prop, _pivot->_right->_prop) + 1;
            }
        };

        template <class _Cate, class _Alloc, bool _Mfl>
        struct treap_traits : public bs_traits<_Cate, _Alloc, _Mfl, treap_traits<_Cate, _Alloc, _Mfl>> {
            using _Self    = treap_traits<_Cate, _Alloc, _Mfl>;
            using _Base    = bs_traits<_Cate, _Alloc, _Mfl, _Self>;
            using _Nodeptr = typename _Base::_Nodeptr;
            using _Base::rotate_left;
            using _Base::rotate_right;

            template <template <class, class> class... _MixIn>
            static void insert_fixup(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                node->_prop = _mt();
                erase_fixup(tree, node);
            }

            template <template <class, class> class... _MixIn>
            static void erase_fixup(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                while (!node->is_real_root() && node->_prop < node->_parent->_prop) {
                    if (node->is_right())
                        rotate_left(tree, node->_parent);
                    else
                        rotate_right(tree, node->_parent);
                }
            }

        private:
            inline static std::mt19937 _mt{ std::random_device{}() };
        };

        template <class _Cate, class _Alloc, bool _Mfl>
        struct splay_traits : public bs_traits<_Cate, _Alloc, _Mfl, splay_traits<_Cate, _Alloc, _Mfl>> {
            using _Self    = splay_traits<_Cate, _Alloc, _Mfl>;
            using _Base    = bs_traits<_Cate, _Alloc, _Mfl, _Self>;
            using _Nodeptr = typename _Base::_Nodeptr;
            using _Base::rotate_left;
            using _Base::rotate_right;

            template <template <class, class> class... _MixIn>
            static void insert_fixup(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                while (!node->is_real_root()) {
                    if (node->is_left()) {
                        if (node->_parent->is_left() && !node->_parent->is_real_root())
                            rotate_right(tree, node->_parent);
                        rotate_right(tree, node->_parent);
                    }
                    else {
                        if (node->_parent->is_right() && !node->_parent->is_real_root())
                            rotate_left(tree, node->_parent);
                        rotate_left(tree, node->_parent);
                    }
                }
            }

            template <template <class, class> class... _MixIn>
            static void erase_fixup(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                insert_fixup(tree, node);
            }

            template <template <class, class> class... _MixIn>
            static void access_fixup(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                insert_fixup(tree, node);
            }
        };

        template <class _Cate, class _Alloc, bool _Mfl>
        struct rb_traits : public bs_traits<_Cate, _Alloc, _Mfl, rb_traits<_Cate, _Alloc, _Mfl>> {
            using _Self    = rb_traits<_Cate, _Alloc, _Mfl>;
            using _Base    = bs_traits<_Cate, _Alloc, _Mfl, _Self>;
            using _Node    = typename _Base::_Node;
            using _Nodeptr = typename _Base::_Nodeptr;

            using _Base::rotate_left;
            using _Base::rotate_right;
            using _Base::take_node;

            template <template <class, class> class... _MixIn>
            static void insert_fixup(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                _Nodeptr _uncle;
                while (!node->is_real_root() && node->_parent->_prop == RED) {
                    if (node->_parent == node->_parent->_parent->_left) {
                        _uncle = node->_parent->_parent->_right;
                        if (_uncle->_prop == RED) {
                            _uncle->_prop = node->_parent->_prop = BLACK;
                            node->_parent->_parent->_prop        = RED;

                            node = node->_parent->_parent;
                        }
                        else {
                            if (node->is_right()) {
                                node = node->_parent;
                                rotate_left(tree, node);
                            }
                            node->_parent->_prop          = BLACK;
                            node->_parent->_parent->_prop = RED;
                            rotate_right(tree, node->_parent->_parent);
                        }
                    }
                    else {
                        _uncle = node->_parent->_parent->_left;
                        if (_uncle->_prop == RED) {
                            _uncle->_prop = node->_parent->_prop = BLACK;
                            node->_parent->_parent->_prop        = RED;

                            node = node->_parent->_parent;
                        }
                        else {
                            if (node->is_left()) {
                                node = node->_parent;
                                rotate_right(tree, node);
                            }
                            node->_parent->_prop          = BLACK;
                            node->_parent->_parent->_prop = RED;
                            rotate_left(tree, node->_parent->_parent);
                        }
                    }
                }
                _Tree_accessor::root(tree)->_parent->_prop = BLACK;
            }

            template <template <class, class> class... _MixIn>
            static void erase_fixup(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                _Nodeptr _bro;
                while (!node->is_real_root() && node->_prop == BLACK) {
                    if (node->is_left()) {
                        _bro = node->_parent->_right;
                        if (_bro->_prop == RED) {
                            _bro->_prop          = BLACK;
                            node->_parent->_prop = RED;
                            rotate_left(tree, node->_parent);
                            _bro = node->_parent->_right;
                        }
                        if (_bro->_left->_prop == BLACK && _bro->_right->_prop == BLACK) {
                            _bro->_prop   = RED;
                            node          = node->_parent;
                            node->_parent = node->_parent->_parent;
                        }
                        else {
                            if (_bro->_right->_prop == BLACK) {
                                _bro->_left->_prop = BLACK;
                                _bro->_prop        = RED;
                                rotate_right(tree, _bro);
                                _bro = node->_parent->_right;
                            }
                            _bro->_prop          = node->_parent->_prop;
                            node->_parent->_prop = BLACK;
                            _bro->_right->_prop  = BLACK;
                            rotate_left(tree, node->_parent);
                            break;
                        }
                    }
                    else {
                        _bro = node->_parent->_left;
                        if (_bro->_prop == RED) {
                            _bro->_prop          = BLACK;
                            node->_parent->_prop = RED;
                            rotate_right(tree, node->_parent);
                            _bro = node->_parent->_left;
                        }
                        if (_bro->_right->_prop == BLACK && _bro->_left->_prop == BLACK) {
                            _bro->_prop   = RED;
                            node          = node->_parent;
                            node->_parent = node->_parent->_parent;
                        }
                        else {
                            if (_bro->_left->_prop == BLACK) {
                                _bro->_right->_prop = BLACK;
                                _bro->_prop         = RED;
                                rotate_left(tree, _bro);
                                _bro = node->_parent->_left;
                            }
                            _bro->_prop          = node->_parent->_prop;
                            node->_parent->_prop = BLACK;
                            _bro->_left->_prop   = BLACK;
                            rotate_right(tree, node->_parent);
                            break;
                        }
                    }
                }
                _Tree_accessor::root(tree)->_parent->_prop = BLACK;
            }

            template <template <class, class> class... _MixIn>
            static _Nodeptr extract_node_impl(_Bs_tree<_Self, _MixIn...>* tree, _Nodeptr node) {
                _Nodeptr _fixnode;
                int      _prop = node->_prop;
                if (node->_left->_is_nil) {
                    _fixnode = node->_right;
                    take_node(tree, node, node->_right);
                }
                else if (node->_right->_is_nil) {
                    _fixnode = node->_left;
                    take_node(tree, node, node->_left);
                }
                else {
                    _Nodeptr _suc = _Node::leftmost(node->_right);
                    _prop         = _suc->_prop;
                    _fixnode      = _suc->_right;
                    if (_suc->_parent == node)
                        _fixnode->_parent = _suc;
                    else {
                        take_node(tree, _suc, _suc->_right);
                        _suc->_right          = node->_right;
                        _suc->_right->_parent = _suc;
                    }
                    take_node(tree, node, _suc);
                    _suc->_left          = node->_left;
                    _suc->_left->_parent = _suc;
                    _suc->_prop          = node->_prop;
                }
                return _prop == BLACK ? _fixnode : _Tree_accessor::root(tree)->_parent;
            }
        };
    }  // namespace

#define MAP_VALUE_TYPE std::pair<const _Key, _Value>
#define DEFINE_ASSO_CONTAINER(NAME)                                                                                 \
    template <class _Tp, class _Compare = std::less<>, class _Alloc = DEFAULT_ALLOC(_Tp)>                           \
    using NAME##_set = _Bs_tree<NAME##_traits<_Set_traits<_Tp, _Compare>, _Alloc, false>>;                          \
    template <class _Tp, class _Compare = std::less<>, class _Alloc = DEFAULT_ALLOC(_Tp)>                           \
    using NAME##_multiset = _Bs_tree<NAME##_traits<_Set_traits<_Tp, _Compare>, _Alloc, true>>;                      \
    template <class _Key, class _Value, class _Compare = std::less<>, class _Alloc = DEFAULT_ALLOC(MAP_VALUE_TYPE)> \
    using NAME##_map = _Bs_tree<NAME##_traits<_Map_traits<_Key, _Value, _Compare>, _Alloc, false>, _Map>;           \
    template <class _Key, class _Value, class _Compare = std::less<>, class _Alloc = DEFAULT_ALLOC(MAP_VALUE_TYPE)> \
    using NAME##_multimap = _Bs_tree<NAME##_traits<_Map_traits<_Key, _Value, _Compare>, _Alloc, true>>;

    DEFINE_ASSO_CONTAINER(bs);
    DEFINE_ASSO_CONTAINER(avl);
    DEFINE_ASSO_CONTAINER(treap);
    DEFINE_ASSO_CONTAINER(splay);
    DEFINE_ASSO_CONTAINER(rb);
#undef MAP_VALUE_TYPE
#undef DEFINE_ASSO_CONTAINER

    template <class _Traits, template <class, class> class... _MixIn>
    _Bs_tree(const _Bs_tree<_Traits, _MixIn...>&, const typename _Traits::allocator_type& = typename _Traits::allocator_type())
        -> _Bs_tree<_Traits, _MixIn...>;
    template <class _Traits, template <class, class> class... _MixIn>
    _Bs_tree(_Bs_tree<_Traits, _MixIn...>&&, const typename _Traits::allocator_type& = typename _Traits::allocator_type())
        -> _Bs_tree<_Traits, _MixIn...>;
}  // namespace xstl

namespace std {
    template <class _Traits, template <class, class> class... _MixIn>
    void swap(xstl::_Bs_tree<_Traits, _MixIn...>& lhs, xstl::_Bs_tree<_Traits, _MixIn...>& rhs) {
        lhs.swap(rhs);
    }

    template <class _Node, class _Alloc, template <class> class _Base, class... _Types>
    void swap(xstl::_Node_handle<_Node, _Alloc, _Base, _Types...>& lhs,
              xstl::_Node_handle<_Node, _Alloc, _Base, _Types...>& rhs) {
        lhs.swap(rhs);
    }
}  // namespace std
#endif