#pragma once
#ifndef _SMALL_VECTOR_HPP_
#define _SMALL_VECTOR_HPP_

#include "allocator.hpp"
#include "compressed_tuple.hpp"
#include "iter_adapter.hpp"

namespace xstl {
    namespace {
        template <size_t N>
        using _Smallest_size_t = std::conditional_t<
            (N < std::numeric_limits<uint8_t>::max()), uint8_t,
            std::conditional_t<
                (N < std::numeric_limits<uint16_t>::max()), uint16_t,
                std::conditional_t<(N < std::numeric_limits<uint32_t>::max()), uint32_t,
                                   std::conditional_t<(N < std::numeric_limits<uint64_t>::max()), uint64_t, size_t>>>>;

        template <class _Default, class... _Args>
        struct _Select_size_type {
            template <class _Ty>
            struct _Is_integral_type : std::is_integral<_Ty> {
                using type = _Ty;
            };
            using type = select_type_t<_Is_integral_type<_Args>..., _Default>;
        };

        template <class _Default, class... _Args>
        struct _Select_allocator {
            template <class _Ty, class = void>
            struct _Is_allocator : std::false_type {};

            template <class _Ty>
            struct _Is_allocator<
                _Ty, std::void_t<typename _Ty::value_type, decltype(std::declval<_Ty&>().deallocate(
                                                               std::declval<_Ty&>().allocate(size_t{ 1 }), size_t{ 1 }))>>
                : std::true_type {
                using type = _Ty;
            };

            using type = select_type_t<_Is_allocator<_Args>..., _Default>;
        };

        struct _Heap_policy {};
    }  // namespace

    struct UseHeap : _Heap_policy {};
    struct AlwaysUseHeap : _Heap_policy {};

    namespace {
        template <class... _Args>
        struct _Select_heap_policy {
            template <class _Ty>
            struct _Is_heap_policy : std::is_convertible<_Ty, _Heap_policy> {
                using type = _Ty;
            };
            static constexpr bool use_heap        = std::is_same_v<select_type_t<_Is_heap_policy<_Args>..., void>, UseHeap>;
            static constexpr bool always_use_heap = std::is_same_v<select_type_t<_Is_heap_policy<_Args>..., void>, AlwaysUseHeap>;
        };

        inline void* _Shift_pointer(void* p, size_t n) noexcept { return static_cast<char*>(p) + n; }
        inline void* _Unshift_pointer(void* p, size_t n) noexcept { return static_cast<char*>(p) - n; }

        template <class _Val_types, class _Vec_base>
        struct _Vector_val : public container_val_base {
            using _Self           = _Vector_val<_Val_types, _Vec_base>;
            using _Nodeptr        = typename _Val_types::_Nodeptr;
            using _Node           = typename _Val_types::_Node;
            using value_type      = typename _Val_types::value_type;
            using size_type       = typename _Val_types::size_type;
            using difference_type = typename _Val_types::difference_type;
            using pointer         = typename _Val_types::pointer;
            using const_pointer   = typename _Val_types::const_pointer;
            using reference       = value_type&;
            using const_reference = const value_type&;

            size_type capacity() const noexcept { return _vec.capacity(); }
            void      set_capacity(size_type c) noexcept { _vec.set_capacity(c); }

            bool is_data_inline() const noexcept { return !_size.is_extern(); }
            void set_data_inline(bool is_inline) noexcept { _size.set_extern(!is_inline); }
            bool is_capacity_inline() const noexcept { return !_size.is_heapified_capacity(); }
            void set_capacity_inline(bool is_inline) noexcept { _size.set_heapified_capacity(!is_inline); }

            constexpr size_type max_size() const noexcept { return _Actual_size_type::max_size(); }

            size_type size() const noexcept { _size.size(); }
            void      set_size(size_type sz) noexcept { _size.set_size(sz); }

            size_type internal_size() const noexcept { return _size.internal_size(); }
            void      set_internal_size(size_type sz) noexcept { _size.set_internal_size(sz); }

            pointer       buffer() noexcept { return _vec.buffer(); }
            const_pointer buffer() const noexcept { return _vec.buffer(); }
            pointer       heap() noexcept { return _vec.heap(); }
            const_pointer heap() const noexcept { return _vec.heap(); }

            template <class _Alloc>
            void free_all(_Alloc& alloc) {
                if (!is_data_inline() && heap())
                    alloc.deallocate(_Unshift_pointer(heap(), heapify_capacity_size),
                                     _vec.capacity() * sizeof(value_type) + heapify_capacity_size);
            }

            template <class _Alloc>
            void free_heap(pointer first, size_type n, _Alloc& alloc) {
                if (!is_data_inline() && heap())
                    alloc.deallocate(first, n * sizeof(value_type));
            }

            template <class _Iter, class _Alloc>
            void destroy(_Iter first, _Iter last, _Alloc& alloc) {
                if constexpr (!std::conjunction_v<std::is_trivially_destructible<value_type>,
                                                  uses_default_destroy<_Alloc, pointer>>) {
                    for (; first != last; ++first)
                        std::allocator_traits<_Alloc>::destroy(alloc, std::addressof(*first));
                }
            }

            void copy_inline_data(const _Self& right) {
                // Copy the entire inline storage, instead of just size() values, to make
                // the loop fixed-size and unrollable.
                std::copy(right._vec.buffer(), right._vec.buffer() + _Vec_base::max_inline, _vec.buffer());
                _size.set_size(right._size.size());
            }

            void swap_with_inline(const _Self& right) noexcept {}
            void swap_with_extern(const _Self& right) {
                _vec.swap_extern(right._vec);
                _size.swap(right._size);
            }

        private:
            static size_t constexpr heapify_capacity_size =
                has_inline_capacity ? 0 : sizeof(xstl_aligned_storage_t<sizeof(size_type), alignof(value_type)>);

            template <bool _Inline_capacity>
            struct heap_ptr {
                // heap[-heapify_capacity_size] contains capacity
                value_type* _heap = nullptr;

                size_type capacity() const noexcept {
                    return _heap ? *static_cast<size_type*>(_Unshift_pointer(_heap, heapify_capacity_size)) : 0;
                }
                void set_capacity(size_type c) noexcept {
                    *static_cast<size_type*>(_Unshift_pointer(_heap, heapify_capacity_size)) = c;
                }

                void swap(heap_ptr& right) noexcept { std::swap(_heap, right._heap); }
            };

            template <>
            struct heap_ptr<true> {
                size_type capacity() const noexcept { return _capacity; }
                void      set_capacity(size_type c) noexcept { _capacity = c; }

                void swap(heap_ptr<true>& right) noexcept {
                    std::swap(_heap, right._heap);
                    std::swap(_capacity, right._capacity);
                }

            private:
                value_type* _heap = nullptr;
                size_type   _capacity{ 0 };
            };

            using inline_storage_t =
                std::conditional_t<sizeof(value_type) * _Vec_base::max_inline != 0,
                                   aligned_storage_for_t<value_type[_Vec_base::max_inline ? _Vec_base::max_inline : 1u]>, char>;

            static bool constexpr has_inline_capacity =
                !_Vec_base::always_use_heap && sizeof(heap_ptr<true>) < sizeof(inline_storage_t);
            // If the values are trivially copyable and the storage is small enough, copy
            // it entirely. Limit is half of a cache line, to minimize probability of
            // introducing a cache miss.
            static constexpr bool is_trivially_inline_copyable =
#ifdef __cpp_lib_hardware_interference_size
                sizeof(inline_storage_t) <= std::hardware_constructive_interference_size / 2 &&
#endif
                std::is_trivially_copyable_v<value_type>;

            union _Data {
                explicit _Data() = default;

                pointer       buffer() noexcept { return reinterpret_cast<pointer>(&_storage); }
                const_pointer buffer() const noexcept { return const_cast<_Data*>(this)->buffer(); }
                pointer       heap() noexcept { return _pdata._heap; }
                const_pointer heap() const noexcept { return _pdata._heap; }

                size_type capacity() const noexcept { return _pdata.capacity(); }
                void      set_capacity(size_type c) noexcept { _pdata.set_capacity(c); }

                void swap_inline(_Data& right) noexcept {}
                void swap_extern(_Data& right) noexcept { _pdata.swap(right._pdata); }

            private:
                heap_ptr<has_inline_capacity> _pdata;
                inline_storage_t              _storage;
            };

            struct _Actual_size_type {
                static constexpr size_type max_size() noexcept { return size_type(~clear_mask); }

                size_type size() const noexcept {
                    if constexpr (_Vec_base::always_use_heap)
                        return _val;
                    else
                        return _val & ~clear_mask;
                }

                void set_size(size_type sz) noexcept {
                    XSTL_EXPECT(sz <= max_size(), "the size of small_vector exceeds max_size()");
                    if constexpr (_Vec_base::always_use_heap)
                        _val = sz;
                    else
                        _val = (clear_mask & _val) | size_type(sz);
                }

                size_type internal_size() const noexcept { return _val; }
                void      set_internal_size(size_type sz) noexcept { _val = sz; }

                bool is_extern() const noexcept {
                    if constexpr (_Vec_base::always_use_heap)
                        return true;
                    else
                        return extern_mask & _val;
                }

                void set_extern(bool b) noexcept {
                    if constexpr (!_Vec_base::always_use_heap) {
                        if (b)
                            _val |= extern_mask;
                        else
                            _val &= ~extern_mask;
                    }
                }

                bool is_heapified_capacity() const noexcept {
                    if constexpr (_Vec_base::always_use_heap)
                        return true;
                    else
                        return capacity_mask & _val;
                }

                void set_heapified_capacity(bool b) noexcept {
                    if constexpr (!_Vec_base::always_use_heap) {
                        if (b)
                            _val |= capacity_mask;
                        else
                            _val &= ~capacity_mask;
                    }
                }

                void swap(_Actual_size_type& right) noexcept { std::swap(_val, right._val); }

            private:
                // We reserve two most significant bits of _val.
                static constexpr size_type extern_mask = _Vec_base::should_use_heap
                                                             ? size_type(1) << (sizeof(size_type) * 8 - 1)
                                                             : 0;  // 1000... If this bit is set to true, it means that the
                                                                   // current data is stored externally (i.e. on the heap)
                static constexpr size_type capacity_mask = _Vec_base::should_use_heap
                                                               ? size_type(1) << (sizeof(size_type) * 8 - 2)
                                                               : 0;  // 0100... If this bit is set to true, it means that the
                                                                     // current capacity is stored externally (i.e. on the heap)
                static constexpr size_type clear_mask = extern_mask | capacity_mask;  // 1100...

                size_type _val{ 0 };
            };

            _Data             _vec;
            _Actual_size_type _size;
        };

        // deal with policies
        template <class _Tp, size_t _Capacity, class... _Policies>
        class _Small_vector_base {
        public:
            static constexpr bool use_heap        = _Select_heap_policy<_Policies...>::use_heap;
            static constexpr bool always_use_heap = _Select_heap_policy<_Policies...>::always_use_heap;
            static constexpr bool should_use_heap = use_heap || always_use_heap;

            static constexpr std::size_t max_inline{ _Capacity == 0 ? 0 : (std::max)(sizeof(_Tp*) / sizeof(_Tp), _Capacity) };

            using allocator_type = _Select_allocator<std::allocator<_Tp>, _Policies...>::type;
            using size_type =
                _Select_size_type<std::conditional_t<should_use_heap, _Smallest_size_t<_Capacity>, size_t>, _Policies...>::type;
        };
    }  // namespace

    template <class _Tp, size_t _Capacity, class... _Policies>
    class small_vector : private _Small_vector_base<_Tp, _Capacity, _Policies...> {
        using _Base = _Small_vector_base<_Tp, _Capacity, _Policies...>;
        using _Self = small_vector<_Tp, _Capacity, _Policies...>;

    public:
        using allocator_type = typename _Base::allocator_type;
        using _Alloc_traits  = std::allocator_traits<allocator_type>;
        static_assert(std::is_same_v<typename allocator_type::value_type, _Tp>, "small_vector<T, Capacity, ...>", "T");
        using value_type                = _Tp;
        using difference_type           = std::ptrdiff_t;
        using reference                 = value_type&;
        using const_reference           = const value_type&;
        using pointer                   = value_type*;
        using const_pointer             = const value_type*;
        using size_type                 = typename _Base::size_type;
        using _Scary_val                = _Vector_val<iter_adapter::scary_iter_types<pointer, value_type, size_type>, _Base>;
        using iterator                  = iter_adapter::rand_iter<_Scary_val>;
        using const_iterator            = iter_adapter::rand_citer<_Scary_val>;
        using _Unchecked_const_iterator = iter_adapter::unchecked_rand_citer<_Scary_val>;
        using _Unchecked_iterator       = iter_adapter::unchecked_rand_iter<_Scary_val>;
        using reverse_iterator          = std::reverse_iterator<iterator>;
        using const_reverse_iterator    = std::reverse_iterator<const_iterator>;

        small_vector() noexcept(std::is_nothrow_default_constructible_v<allocator_type>) {}

        explicit small_vector(const allocator_type& alloc) noexcept : _tpl(std::ignore, alloc) {}

        small_vector(const small_vector& right)
            : _tpl(std::ignore, _Alloc_traits::select_on_container_copy_construction(right._Getal())) {
            if constexpr (_Scary_val::is_trivially_inline_copyable) {
                if (right._Get_val().is_data_inline()) {
                    _Get_val().copy_inline_data(right._Get_val());
                    return;
                }
            }
            auto _sz = right.size();
            _Allocate_for(_sz);
            {
                auto _guard = scoped_guard([&] { _Get_val().free_all(_Getal()); });
                std::uninitialized_copy(right._Unchecked_begin(), right._Unchecked_begin() + static_cast<difference_type>(_sz),
                                        _Unchecked_begin());
                _guard.dismiss();
            }
            _Get_val()->set_size(_sz);
        }

        small_vector(small_vector&& right) noexcept(std::is_nothrow_move_constructible_v<value_type>)
            : _tpl(std::ignore, std::move(right._Getal())) {
            if (right._Get_val().is_data_inline()) {
                if (_Scary_val::is_trivially_inline_copyable) {
                    _Get_val().copy_inline_data(right._Get_val());
                    right._Get_val().set_internal_size(0);
                }
                else {
                    std::uninitialized_copy(std::make_move_iterator(right._Unchecked_begin()),
                                            std::make_move_iterator(right._Unchecked_end()), _Unchecked_begin());
                    _Get_val()->set_size(right.size());
                    right.clear();
                }
            }
            else
                _Get_val().swap_with_extern(right._Get_val());
        }

        small_vector(std::initializer_list<value_type> il, const allocator_type& alloc = allocator_type())
            : small_vector(il.begin(), il.end(), alloc) {}

        template <class... _Args>
        void _Construct_n(size_type n, _Args&&... args) {
            if (n == 0)
                return;
            _Scary_val& _val = _Get_val();
            auto&       _al  = _Getal();
            _Allocate_for(n);
            pointer _first = data(), _last = _first;
            auto    _guard = scoped_guard([&] {
                _val.destroy(_first, _last, _al);
                _val.free_all();
            });
            if constexpr (sizeof...(args) == 0) {
                if constexpr (std::is_scalar_v<value_type> && !std::is_volatile_v<value_type>)
                    memset(_first, 0, n * sizeof(value_type));
                else {
                    do {
                        _Alloc_traits::construct(_al, _last);
                        _last++;
                    } while (--n != 0)
                }
            }
            else if constexpr (sizeof...(args) == 1) {
                static_assert(std::is_same_v<_Args..., const_reference>);
                do {
                    _Alloc_traits::construct(_al, _last, args...);
                    _last++;
                } while (--n != 0);
            }
            else if constexpr (sizeof...(args) == 2) {
                _My_data._Mylast = _Uninitialized_copy(_STD forward<argsty>(args)..., _My_data._Myfirst, _Al);
            }
            else {
                static_assert(always_false<_Args...>, "Should be unreachable");
            }
            _val.set_internal_size(n);
            _guard.dismiss();
        }

        explicit small_vector(size_type n, const allocator_type& alloc = allocator_type()) : _tpl(std::ignore, alloc) {
            _Construct_n(n);
        }

        small_vector(size_type n, const_reference value, const allocator_type& alloc = allocator_type())
            : _tpl(std::ignore, alloc) {
            _Construct_n(n, value);
        }

        template <class _Iter, XSTL_REQUIRES_(is_input_iterator_v<_Iter>)>
        explicit small_vector(_Iter first, _Iter last, const allocator_type& alloc = allocator_type())
            : _tpl(std::ignore, alloc) {
            // Forward using std::is_arithmetic to get to the proper
            // implementation; this disambiguates between the iterators and
            // (size_t, value_type) meaning for this constructor.
            constructImpl(arg1, arg2, std::is_arithmetic<_Iter>());
        }

        ~small_vector() {
            for (auto& t : *this) {
                (&t)->~value_type();
            }
            _Get_val().free_all(_Getal());
        }

        small_vector& operator=(small_vector const& o) {
            if (FOLLY_LIKELY(this != &o)) {
                if (is_trivially_inline_copyable && !this->is_extern() && !o.is_extern()) {
                    copyInlineTrivial<Value>(o);
                }
                else if (o.size() < capacity()) {
                    const size_t oSize = o.size();
                    detail::partiallyUninitializedCopy(o.begin(), oSize, begin(), size());
                    this->set_size(oSize);
                }
                else {
                    assign(o.begin(), o.end());
                }
            }
            return *this;
        }

        small_vector& operator=(small_vector&& o) noexcept(std::is_nothrow_move_constructible<Value>::value) {
            if (FOLLY_LIKELY(this != &o)) {
                // If either is external, reduce to the default-constructed case for this,
                // since there is nothing that we can move in-place.
                if (this->is_extern() || o.is_extern()) {
                    reset();
                }

                if (!o.is_extern()) {
                    if (is_trivially_inline_copyable) {
                        copyInlineTrivial<Value>(o);
                        o.resetSizePolicy();
                    }
                    else {
                        const size_t oSize = o.size();
                        detail::partiallyUninitializedCopy(std::make_move_iterator(o.u.buffer()), oSize, this->u.buffer(),
                                                           size());
                        this->set_size(oSize);
                        o.clear();
                    }
                }
                else {
                    this->u._pdata._heap = o.u._pdata._heap;
                    o.u._pdata._heap     = nullptr;
                    // this was already reset above, so it's empty and internal.
                    this->swapSizePolicy(o);
                    if (kHasInlineCapacity) {
                        this->u.set_capacity(o.u.capacity());
                    }
                }
            }
            return *this;
        }

        bool operator==(small_vector const& o) const { return size() == o.size() && std::equal(begin(), end(), o.begin()); }

        bool operator<(small_vector const& o) const { return std::lexicographical_compare(begin(), end(), o.begin(), o.end()); }

        XSTL_NODISCARD constexpr size_type max_size() const noexcept {
            if constexpr (_Base::should_use_heap)
                return (std::min)(_Get_val().max_size(),
                                  (std::min)(static_cast<size_type>((std::numeric_limits<difference_type>::max)()),
                                             _Alloc_traits::max_size(_Getal())));
            else
                return static_cast<size_type>(_Base::max_inline);
        }

        allocator_type get_allocator() const noexcept { return _Getal(); }

        size_type size() const noexcept { return _Get_val().size(); }
        bool      empty() const noexcept { return !size(); }

        iterator       begin() { return data(); }
        iterator       end() { return data() + size(); }
        const_iterator begin() const { return data(); }
        const_iterator end() const { return data() + size(); }
        const_iterator cbegin() const { return begin(); }
        const_iterator cend() const { return end(); }

        reverse_iterator rbegin() { return reverse_iterator(end()); }
        reverse_iterator rend() { return reverse_iterator(begin()); }

        const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

        const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

        const_reverse_iterator crbegin() const { return rbegin(); }
        const_reverse_iterator crend() const { return rend(); }

        /*
         * Usually one of the simplest functions in a Container-like class
         * but a bit more complex here.  We have to handle all combinations
         * of in-place vs. heap between this and o.
         */
        void swap(small_vector& right) noexcept(
            std::is_nothrow_move_constructible_v<value_type>&& is_nothrow_swappable_v<value_type>) {
            using std::swap;  // Allow ADL on swap for our value_type.
            _Scary_val& _val = _Get_val();
            if (!_val.is_data_inline() && !right._Get_val().is_data_inline())
                _val.swap_with_extern(right._Get_val());
            else if (!this->is_extern() && !right.is_extern()) {
                auto& oldSmall = size() < right.size() ? *this : right;
                auto& oldLarge = size() < right.size() ? right : *this;

                for (size_type i = 0; i < oldSmall.size(); ++i) {
                    swap(oldSmall[i], oldLarge[i]);
                }

                size_type       i  = oldSmall.size();
                const size_type ci = i;
                {
                    auto _guard = scoped_guard([&] {
                        oldSmall.set_size(i);
                        for (; i < oldLarge.size(); ++i) {
                            oldLarge[i].~value_type();
                        }
                        oldLarge.set_size(ci);
                    });
                    for (; i < oldLarge.size(); ++i) {
                        auto addr = oldSmall.begin() + i;
                        new (addr) value_type(std::move(oldLarge[i]));
                        oldLarge[i].~value_type();
                    }
                    _guard.dismiss();
                }
                oldSmall.set_size(i);
                oldLarge.set_size(ci);
                return;
            }

            // is_extern != right.is_extern()
            auto& oldExtern = right.is_extern() ? right : *this;
            auto& oldIntern = right.is_extern() ? *this : right;

            auto oldExternCapacity = oldExtern.capacity();
            auto oldExternHeap     = oldExtern.u._pdata._heap;

            auto      buff = oldExtern.u.buffer();
            size_type i    = 0;
            {
                auto _guard = scoped_guard([&] {
                    for (size_type kill = 0; kill < i; ++kill) {
                        buff[kill].~value_type();
                    }
                    for (; i < oldIntern.size(); ++i) {
                        oldIntern[i].~value_type();
                    }
                    oldIntern.resetSizePolicy();
                    oldExtern.u._pdata._heap = oldExternHeap;
                    oldExtern.set_capacity(oldExternCapacity);
                });
                for (; i < oldIntern.size(); ++i) {
                    new (&buff[i]) value_type(std::move(oldIntern[i]));
                    oldIntern[i].~value_type();
                }
                _guard.dismiss();
            }
            oldIntern.u._pdata._heap = oldExternHeap;
            this->swapSizePolicy(right);
            oldIntern.set_capacity(oldExternCapacity);
        }

        void resize(size_type sz) {
            if (sz <= size()) {
                downsize(sz);
                return;
            }
            auto extra = sz - size();
            _Allocate_for(sz);
            detail::populateMemForward(begin() + size(), extra, [&](void* p) { new (p) value_type(); });
            this->incrementSize(extra);
        }

        void resize(size_type sz, value_type const& v) {
            if (sz < size()) {
                erase(begin() + sz, end());
                return;
            }
            auto extra = sz - size();
            _Allocate_for(sz);
            detail::populateMemForward(begin() + size(), extra, [&](void* p) { new (p) value_type(v); });
            this->incrementSize(extra);
        }

        pointer data() noexcept { return _Get_val().is_data_inline() ? _Get_val().buffer() : _Get_val().heap(); }

        const_pointer data() const noexcept { return const_cast<_Self*>(this)->data(); }

        template <class... Args>
        iterator emplace(const_iterator p, Args&&... args) {
            if (p == cend()) {
                emplace_back(std::forward<Args>(args)...);
                return end() - 1;
            }

            /*
             * We implement emplace at places other than at the back with a
             * temporary for exception safety reasons.  It is possible to
             * avoid having to do this, but it becomes hard to maintain the
             * basic exception safety guarantee (unless you respond to a copy
             * constructor throwing by clearing the whole vector).
             *
             * The reason for this is that otherwise you have to destruct an
             * element before constructing this one in its place---if the
             * constructor throws, you either need a nothrow default
             * constructor or a nothrow copy/move to get something back in the
             * "gap", and the vector requirements don't guarantee we have any
             * of these.  Clearing the whole vector is a legal response in
             * this situation, but it seems like this implementation is easy
             * enough and probably better.
             */
            return insert(p, value_type(std::forward<Args>(args)...));
        }

        void reserve(size_type sz) { _Allocate_for(sz); }

        size_type capacity() const noexcept { return _Get_val().is_data_inline() ? _Base::max_inline : _Get_val().capacity(); }

        void shrink_to_fit() {
            if (!_Get_val().is_data_inline())
                small_vector(begin(), end()).swap(*this);
        }

        template <class... Args>
        reference emplace_back(Args&&... args) {
            auto _isize = this->getInternalSize();
            if (_isize < max_inline) {
                new (u.buffer() + _isize) value_type(std::forward<Args>(args)...);
                this->incrementSize(1);
                return *(u.buffer() + _isize);
            }
            if (!BaseType::kShouldUseHeap) {
                throw_exception<std::length_error>("max_size exceeded in small_vector");
            }
            auto _size     = size();
            auto _capacity = capacity();
            if (_capacity == _size) {
                // Any of args may be references into the vector.
                // When we are reallocating, we have to be careful to construct the new
                // element before modifying the data in the old buffer.
                _Allocate_for(
                    _size + 1, [&](void* p) { new (p) value_type(std::forward<Args>(args)...); }, _size);
            }
            else {
                // We know the vector is stored in the heap.
                new (u.heap() + _size) value_type(std::forward<Args>(args)...);
            }
            this->incrementSize(1);
            return *(u.heap() + _size);
        }

        void push_back(value_type&& t) { emplace_back(std::move(t)); }

        void push_back(const_reference t) { emplace_back(t); }

        void pop_back() {
            // ideally this would be implemented in terms of erase(end() - 1) to reuse
            // the higher-level abstraction, but neither Clang or GCC are able to
            // optimize it away. if you change this, please verify (with disassembly)
            // that the generated code on -O3 (and ideally -O2) stays short
            downsize(size() - 1);
        }

        iterator insert(const_iterator constp, value_type&& t) {
            iterator p = unconst(constp);
            if (p == end()) {
                push_back(std::move(t));
                return end() - 1;
            }

            auto offset = p - begin();
            auto _size  = size();
            if (capacity() == _size) {
                _Allocate_for(
                    _size + 1, [&t](void* ptr) { new (ptr) value_type(std::move(t)); }, offset);
                this->incrementSize(1);
            }
            else {
                detail::moveObjectsRightAndCreate(data() + offset, data() + _size, data() + _size + 1,
                                                  [&]() mutable -> value_type&& { return std::move(t); });
                this->incrementSize(1);
            }
            return begin() + offset;
        }

        iterator insert(const_iterator p, value_type const& t) {
            // Make a copy and forward to the rvalue value_type&& overload
            // above.
            return insert(p, value_type(t));
        }

        iterator insert(const_iterator pos, size_type n, value_type const& val) {
            auto offset = pos - begin();
            auto _size  = size();
            _Allocate_for(_size + n);
            detail::moveObjectsRightAndCreate(data() + offset, data() + _size, data() + _size + n,
                                              [&]() mutable -> value_type const& { return val; });
            this->incrementSize(n);
            return begin() + offset;
        }

        template <class Arg>
        iterator insert(const_iterator p, Arg arg1, Arg arg2) {
            // Forward using std::is_arithmetic to get to the proper
            // implementation; this disambiguates between the iterators and
            // (size_t, value_type) meaning for this function.
            return insertImpl(unconst(p), arg1, arg2, std::is_arithmetic<Arg>());
        }

        iterator insert(const_iterator p, std::initializer_list<value_type> il) { return insert(p, il.begin(), il.end()); }

        iterator erase(const_iterator q) {
            // ideally this would be implemented in terms of erase(q, q + 1) to reuse
            // the higher-level abstraction, but neither Clang or GCC are able to
            // optimize it away. if you change this, please verify (with disassembly)
            // that the generated code on -O3 (and ideally -O2) stays short
            std::move(unconst(q) + 1, end(), unconst(q));
            downsize(size() - 1);
            return unconst(q);
        }

        iterator erase(const_iterator q1, const_iterator q2) {
            if (q1 == q2) {
                return unconst(q1);
            }
            std::move(unconst(q2), end(), unconst(q1));
            downsize(size() - std::distance(q1, q2));
            return unconst(q1);
        }

        void clear() {
            // ideally this would be implemented in terms of erase(begin(), end()) to
            // reuse the higher-level abstraction, but neither Clang or GCC are able to
            // optimize it away. if you change this, please verify (with disassembly)
            // that the generated code on -O3 (and ideally -O2) stays short
            downsize(0);
        }

        template <class _Iter, XSTL_REQUIRES_(is_input_iterator_v<_Iter>)>
        void assign(_Iter first, _Iter last) {
            clear();
            insert(begin(), first, last);
        }

        void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }

        void assign(size_type n, const value_type& t) {
            clear();
            insert(end(), n, t);
        }

        XSTL_NODISCARD reference front() noexcept {
            XSTL_EXPECT(!empty(), "front() called on empty small_vector");
            return *begin();
        }
        XSTL_NODISCARD reference back() noexcept {
            XSTL_EXPECT(!empty(), "back() called on empty small_vector");
            return *(end() - 1);
        }
        XSTL_NODISCARD const_reference front() const noexcept {
            XSTL_EXPECT(!empty(), "front() called on empty small_vector");
            return *begin();
        }
        XSTL_NODISCARD const_reference back() const noexcept {
            XSTL_EXPECT(!empty(), "back() called on empty small_vector");
            return *(end() - 1);
        }

        XSTL_NODISCARD reference operator[](size_type i) noexcept {
            XSTL_EXPECT(i < size(), "small_vector subscript out of range");
            return *(data() + i);
        }

        XSTL_NODISCARD const_reference operator[](size_type i) const noexcept {
            XSTL_EXPECT(i < size(), "small_vector subscript out of range");
            return *(data() + i);
        }

        XSTL_NODISCARD reference at(size_type i) {
            if (i >= size())
                throw std::out_of_range("index out of range");
            return *(data() + i);
        }

        XSTL_NODISCARD const_reference at(size_type i) const {
            if (i >= size())
                throw std::out_of_range("index out of range");
            return *(data() + i);
        }

    private:
        static iterator unconst(const_iterator it) { return const_cast<iterator>(it); }

        void downsize(size_type sz) {
            assert(sz <= size());
            for (auto it = (begin() + sz), e = end(); it != e; ++it) {
                it->~value_type();
            }
            this->set_size(sz);
        }

        void reset() {
            clear();
            _Get_val().free_all(_Getal());
            this->resetSizePolicy();
        }

        // The std::false_type argument is part of disambiguating the
        // iterator insert functions from integral types (see insert().)
        template <class It>
        iterator insertImpl(iterator pos, It first, It last, std::false_type) {
            if (first == last) {
                return pos;
            }
            using categ  = typename std::iterator_traits<It>::iterator_category;
            using it_ref = typename std::iterator_traits<It>::reference;
            if (std::is_same<categ, std::input_iterator_tag>::value) {
                auto offset = pos - begin();
                while (first != last) {
                    pos = insert(pos, *first++);
                    ++pos;
                }
                return begin() + offset;
            }

            auto const distance = std::distance(first, last);
            auto const offset   = pos - begin();
            auto       _size    = size();
            assert(distance >= 0);
            assert(offset >= 0);
            _Allocate_for(_size + distance);
            detail::moveObjectsRightAndCreate(data() + offset, data() + _size, data() + _size + distance,
                                              [&, in = last]() mutable -> it_ref { return *--in; });
            this->incrementSize(distance);
            return begin() + offset;
        }

        iterator insertImpl(iterator pos, size_type n, const value_type& val, std::true_type) {
            // The true_type means this should call the size_t,value_type
            // overload.  (See insert().)
            return insert(pos, n, val);
        }

        // The std::false_type argument came from std::is_arithmetic as part
        // of disambiguating an overload (see the comment in the
        // constructor).
        template <class It>
        void constructImpl(It first, It last, std::false_type) {
            typedef typename std::iterator_traits<It>::iterator_category categ;
            if (std::is_same<categ, std::input_iterator_tag>::value) {
                // With iterators that only allow a single pass, we can't really
                // do anything sane here.
                while (first != last) {
                    emplace_back(*first++);
                }
                return;
            }
            size_type distance = std::distance(first, last);
            if (distance <= max_inline) {
                this->incrementSize(distance);
                detail::populateMemForward(u.buffer(), distance, [&](void* p) { new (p) value_type(*first++); });
                return;
            }
            _Allocate_for(distance);
            this->incrementSize(distance);
            {
                auto _guard = scoped_guard([&] { _Get_val().free_all(_Getal()); });
                detail::populateMemForward(u.heap(), distance, [&](void* p) { new (p) value_type(*first++); });
                _guard.dismiss();
            }
        }

        template <typename InitFunc>
        void doConstruct(size_type n, InitFunc&& func) {
            _Allocate_for(n);
            assert(size() == 0);
            this->incrementSize(n);
            {
                auto _guard = scoped_guard([&] { _Get_val().free_all(_Getal()); });
                detail::populateMemForward(data(), n, std::forward<InitFunc>(func));
                _guard.dismiss();
            }
        }

        // The true_type means we should forward to the size_t,value_type
        // overload.
        void constructImpl(size_type n, value_type const& val, std::true_type) {
            doConstruct(n, [&](void* p) { new (p) value_type(val); });
        }

        /*
         * Compute the size after growth.
         */
        size_type computeNewSize() const { return std::min((3 * capacity()) / 2 + 1, max_size()); }

        void _Allocate_for(size_type newSize) {
            if (newSize <= capacity()) {
                return;
            }
            makeSizeInternal(
                newSize, false, [](void*) { assume_unreachable(); }, 0);
        }

        template <typename EmplaceFunc>
        void _Allocate_for(size_type newSize, EmplaceFunc&& emplaceFunc, size_type pos) {
            assert(size() == capacity());
            makeSizeInternal(newSize, true, std::forward<EmplaceFunc>(emplaceFunc), pos);
        }

        /*
         * Ensure we have a large enough memory region to be size `newSize'.
         * Will move/copy elements if we are spilling to heap_ or needed to
         * allocate a new region, but if resized in place doesn't initialize
         * anything in the new region.  In any case doesn't change size().
         * Supports insertion of new element during reallocation by given
         * pointer to new element and position of new element.
         * NOTE: If reallocation is not needed, insert must be false,
         * because we only know how to emplace elements into new memory.
         */
        template <typename EmplaceFunc>
        void makeSizeInternal(size_type newSize, bool insert, EmplaceFunc&& emplaceFunc, size_type pos) {
            if (newSize > max_size()) {
                throw std::length_error("max_size exceeded in small_vector");
            }
            assert(this->kShouldUseHeap);
            // This branch isn't needed for correctness, but allows the optimizer to
            // skip generating code for the rest of this function in in-situ-only
            // small_vectors.
            if (!this->kShouldUseHeap) {
                return;
            }

            newSize = std::max(newSize, computeNewSize());

            const auto needBytes = newSize * sizeof(value_type);
            // If the capacity isn't explicitly stored inline, but the heap
            // allocation is grown to over some threshold, we should store
            // a capacity at the front of the heap allocation.
            const bool   heapifyCapacity           = !kHasInlineCapacity && needBytes >= kHeapifyCapacityThreshold;
            const size_t allocationExtraBytes      = heapifyCapacity ? kHeapifyCapacitySize : 0;
            const size_t goodAllocationSizeBytes   = goodMallocSize(needBytes + allocationExtraBytes);
            const size_t goodAllocationNewCapacity = (goodAllocationSizeBytes - allocationExtraBytes) / sizeof(value_type);
            const size_t newCapacity               = std::min(goodAllocationNewCapacity, max_size());
            // Make sure that the allocation request has a size computable from the
            // capacity, instead of using goodAllocationSizeBytes, so that we can do
            // sized deallocation. If goodMallocSize() gives us extra bytes that are not
            // a multiple of the value size we cannot use them anyway.
            const size_t sizeBytes = newCapacity * sizeof(value_type) + allocationExtraBytes;
            void*        newh      = checkedMalloc(sizeBytes);
            value_type*  newp =
                static_cast<value_type*>(heapifyCapacity ? detail::shiftPointer(newh, kHeapifyCapacitySize) : newh);

            {
                auto _guard = scoped_guard([&] {  //
                    sizedFree(newh, sizeBytes);
                });
                if (insert) {
                    // move and insert the new element
                    this->moveToUninitializedEmplace(begin(), end(), newp, pos, std::forward<EmplaceFunc>(emplaceFunc));
                }
                else {
                    // move without inserting new element
                    if (data()) {
                        this->moveToUninitialized(begin(), end(), newp);
                    }
                }
                _guard.dismiss();
            }
            _Scary_val& _val = _Get_val();
            auto&       _al  = _Getal();
            _val.destroy() _val.free_all(_al);

            // Store shifted pointer if capacity is heapified
            u.pdata_.heap_ = newp;
            this->set_heapified_capacity(heapifyCapacity);
            this->set_extern(true);
            this->set_capacity(newCapacity);
        }

        /*
         * This will set the capacity field, stored inline in the storage_ field
         * if there is sufficient room to store it.
         */
        void set_capacity(size_type newCapacity) {
            assert(this->is_extern());
            if (hasCapacity()) {
                assert(newCapacity < std::numeric_limits<InternalSizeType>::max());
                u.set_capacity(newCapacity);
            }
        }

    private:
        _Scary_val&           _Get_val() noexcept { return std::get<0>(_tpl); }
        const _Scary_val&     _Get_val() const noexcept { return std::get<0>(_tpl); }
        allocator_type&       _Getal() noexcept { return std::get<1>(_tpl); }
        const allocator_type& _Getal() const noexcept { return std::get<1>(_tpl); }

        iterator _Make_iter(pointer ptr) const noexcept {
            return iterator(ptr, std::addressof(_Get_val());
        }
        const_iterator _Make_citer(pointer ptr) const noexcept {
            return const_iterator(ptr, std::addressof(_Get_val());
        }
        _Unchecked_const_iterator _Make_unchecked_citer(pointer ptr) const noexcept { return; }

        _Unchecked_iterator       _Unchecked_begin() noexcept { return _Unchecked_iterator(data(), std::addressof(_Get_val())); }
        _Unchecked_iterator       _Unchecked_end() noexcept { return _Unchecked_begin() + static_cast<difference_type>(size()); }
        _Unchecked_const_iterator _Unchecked_begin() const noexcept {
            return _Unchecked_const_iterator(data(), std::addressof(_Get_val()));
        }
        _Unchecked_const_iterator _Unchecked_end() const noexcept {
            return _Unchecked_begin() + static_cast<difference_type>(size());
        }

        compressed_tuple<_Scary_val, allocator_type> _tpl;
    };

}  // namespace xstl
#endif