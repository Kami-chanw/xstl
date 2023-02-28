#pragma once
#ifndef _SMALL_VECTOR_HPP_
#define _SMALL_VECTOR_HPP_

#include "allocator.hpp"
#include "compressed_tuple.hpp"
#include "iter_adapter.hpp"
#include <scoped_allocator>

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

        template <class _Albyte, class _Alty>
        class _Merge_alloc : _Alty, _Albyte {
            using _Alty_traits                       = std::allocator_traits<_Alty>;
            using _Albyte_traits                     = std::allocator_traits<_Albyte>;
            using _Ty                                = typename _Alty_traits::value_type;
            static constexpr bool has_byte_allocator = !std::is_void_v<typename _Albyte_traits::value_type>;

        public:
            using alty_type   = _Alty;
            using albyte_type = _Albyte;

            using value_type         = typename _Albyte_traits::value_type;
            using pointer            = typename _Albyte_traits::pointer;
            using const_pointer      = typename _Albyte_traits::const_pointer;
            using void_pointer       = typename _Albyte_traits::void_pointer;
            using const_void_pointer = typename _Albyte_traits::const_void_pointer;

            using size_type       = typename _Albyte_traits::size_type;
            using difference_type = typename _Albyte_traits::difference_type;

            template <class _Other>
            struct rebind {
                using other = _Merge_alloc<_Albyte, typename _Alty_traits::template rebind_alloc<_Other>>;
            };

            using propagate_on_container_copy_assignment =
                std::disjunction<typename _Alty_traits::propagate_on_container_copy_assignment,
                                 typename _Albyte_traits::propagate_on_container_copy_assignment>;

            using propagate_on_container_move_assignment =
                std::disjunction<typename _Alty_traits::propagate_on_container_move_assignment,
                                 typename _Albyte_traits::propagate_on_container_move_assignment>;

            using propagate_on_container_swap = std::disjunction<typename _Alty_traits::propagate_on_container_swap,
                                                                 typename _Albyte_traits::propagate_on_container_swap>;

            using is_always_equal =
                std::conjunction<typename _Alty_traits::is_always_equal, typename _Albyte_traits::is_always_equal>;

            _Merge_alloc() = default;
            _Merge_alloc(const _Alty& alty) : _Alty(alty), _Albyte() {}
            _Merge_alloc(const _Albyte& albyte) : _Alty(), _Albyte(albyte) {}
            _Merge_alloc(const _Merge_alloc&) noexcept = default;

            _Merge_alloc(_Merge_alloc&&) noexcept = default;

            _Merge_alloc& operator=(const _Merge_alloc&) = default;
            _Merge_alloc& operator=(_Merge_alloc&&)      = default;

            XSTL_NODISCARD albyte_type& byte_allocator() noexcept { return static_cast<_Albyte&>(*this); }

            XSTL_NODISCARD const albyte_type& byte_allocator() const noexcept { return static_cast<const _Albyte&>(*this); }

            XSTL_NODISCARD alty_type& value_allocator() noexcept { return static_cast<_Alty&>(*this); }

            XSTL_NODISCARD const alty_type& value_allocator() const noexcept { return static_cast<const _Alty&>(*this); }

            XSTL_NODISCARD pointer allocate(size_type n) {
                if constexpr (has_byte_allocator)
                    return _Albyte_traits::allocate(byte_allocator(), n);
                else
                    return reinterpret_cast<pointer>(_Alty_traits::allocate(value_allocator(),
                                                  std::floor(static_cast<double>(n / sizeof(_Ty))));
            }

            void deallocate(pointer ptr, size_type n) {
                if constexpr (has_byte_allocator)
                    _Albyte_traits::deallocate(byte_allocator(), ptr, n);
                else
                    _Alty_traits::deallocate(value_allocator(), reinterpret_cast<_Ty*>(ptr),
                                             std::floor(static_cast<double>(n) / sizeof(_Ty)));
            }

            XSTL_NODISCARD size_type max_size() const { return _Albyte_traits::max_size(byte_allocator()); }

            template <class _Ty, class... _Args>
            void construct(_Ty* ptr, _Args&&... args) {
                _Alty_traits::construct(value_allocator(), ptr, std::forward<_Args>(args)...);
            }

            template <class _Ty>
            void destroy(_Ty* ptr) {
                _Alty_traits::destroy(value_allocator(), ptr);
            }
        };

        /**
         * Heap policy is used to determine where data is placed
         * There are two choices:
         * 1. default: if size < Capacity, place on the stack, otherwise, transfer to heap.
         * 2. NoHeap: avoid the heap entirely. (Throws std::length_error if you would've spilled out of the in-place allocation.
         */

        struct _Heap_policy {};

        /**
         * Due to the allocation of heapified capaicty, we have to use allocator<std::byte> instead of allocator<value_type>
         * There are three choices:
         * 1. default: use allocator<value_type>::construct/destroy but use rebind_allocator<std::byte> to allocate.
         * 2. NoByteAllocator: use allocator<value_type>::allocate/construct/destroy, but sizeof(capaicty) == sizeof(value_type)
         * or capaicty will never be heapified (it depends on the size between value_type and size_type).
         * 3. ByteAllocator<_Alloc>: use allocator<value_type>::construct/destroy and use another byte allocator to allocator.
         * allocator_policy allows you determine which policy to use
         */
        struct _Allocator_policy {};
    }  // namespace

    struct NoHeap : _Heap_policy {};
    template <class _Alloc>
    struct ByteAllocator : _Allocator_policy {
        static_assert(is_allocator<_Alloc>::value
                          && is_any_of_v<typename std::allocator_traits<_Alloc>::value_type, std::byte, unsigned char>,
                      "_Alloc must be an allocator whose value_type is std::byte or unsigned char");
        using type = _Alloc;
    };
    struct NoByteAllocator : _Allocator_policy {
        using type = std::allocator<void>;
    };

    namespace {
        template <class... _Args>
        struct _Select_heap_policy {
            template <class _Ty>
            struct _Is_heap_policy : std::is_convertible<_Ty, _Heap_policy> {
                using type = _Ty;
            };
            static constexpr bool use_heap = !std::is_same_v<select_type_t<_Is_heap_policy<_Args>..., void>, NoHeap>;
        };

        template <class _Default, class... _Args>
        struct _Select_allocator {
        private:
            template <class _Ty>
            struct _Is_alloc_policy : std::is_convertible<_Ty, _Allocator_policy> {
                using type = typename _Ty::type;
            };

        public:
            using type = select_type_t<is_allocator<_Args>..., _Default>;
            using byte_allocator =
                select_type_t<_Is_alloc_policy<_Args>..., std::allocator_traits<type>::template rebind_alloc<std::byte>>;

            static constexpr bool has_byte_allocator =
                !std::is_void_v<typename std::allocator_traits<byte_allocator>::value_type>;
        };

        inline void* _Shift_pointer(void* p, size_t n) noexcept { return static_cast<char*>(p) + n; }
        inline void* _Unshift_pointer(void* p, size_t n) noexcept { return static_cast<char*>(p) - n; }

        template <class _Val_types, class _Vec_base>
        struct _Vector_val : public container_val_base {
            using _Self           = _Vector_val<_Val_types, _Vec_base>;
            using _Nodeptr        = typename _Val_types::_Nodeptr;
            using allocator_type  = typename _Val_types::allocator_type;
            using value_type      = typename _Val_types::value_type;
            using size_type       = typename _Val_types::size_type;
            using difference_type = typename _Val_types::difference_type;
            using pointer         = typename _Val_types::pointer;
            using const_pointer   = typename _Val_types::const_pointer;
            using reference       = value_type&;
            using const_reference = const value_type&;

            // requirements for iterator adapters
            template <class _This, class _Other>
            struct is_forced_castable : std::false_type {};
            template <class _Other_base>
            struct is_forced_castable<_Self, _Vector_val<_Val_types, _Other_base>> : std::true_type {};

            static reference extract(_Nodeptr ptr) noexcept { return *ptr; }
            static bool      dereferable(const _Self* self, _Nodeptr ptr) noexcept { return range_verify(self, ptr, 0); }
            static bool      range_verify(const _Self* self, _Nodeptr ptr, difference_type diff) noexcept {
                _Nodeptr _dest = ptr + diff;
                if (self->is_data_inline())
                    return self->_vec.buffer() <= _dest && _dest < self->_vec.buffer() + _Vec_base::max_inline;
                else {
                    pointer _data = _Shift_pointer(self->_vec.heap(), heapify_capacity_size);
                    return _data <= _dest && _dest < _data + self->_size.size();
                }
            }

            // public member funciton
            size_type capacity() const noexcept { return _vec.capacity(); }
            void      set_capacity(size_type c) noexcept { _vec.set_capacity(c); }

            bool is_data_inline() const noexcept { return !_size.is_extern(); }
            void set_data_inline(bool is_inline) noexcept { _size.set_extern(!is_inline); }

            constexpr size_type max_size() const noexcept { return _Actual_size_type::max_size(); }

            size_type size() const noexcept { _size.size(); }
            void      set_size(size_type sz) noexcept { _size.set_size(sz); }

            size_type internal_size() const noexcept { return _size.internal_size(); }
            void      set_internal_size(size_type sz) noexcept { _size.set_internal_size(sz); }

            pointer       buffer() noexcept { return _vec.buffer(); }
            const_pointer buffer() const noexcept { return _vec.buffer(); }
            pointer       heap() noexcept { return _vec.heap(); }
            void          set_heap(pointer ptr) noexcept { _vec.set_heap(ptr); }
            const_pointer heap() const noexcept { return _vec.heap(); }
            template <class _Alloc, class _Iter, class _Dest>
            static _Iter copy_to_uninitialize(_Alloc& alloc, _Iter first, _Iter last, _Dest dest) {
                if constexpr (uses_default_construct<allocator_type, pointer, const_reference>::value)
                    dest = std::uninitialized_copy(first, last, dest);
                else {
                    const _Dest _old_dest = first;
                    auto        _guard    = scoped_guard([&] { destroy(alloc, _old_dest, dest); });
                    for (; first != last; ++first, ++dest)
                        construct(alloc, dest++, *first);
                    _guard.dismiss();
                }
            }

            template <class _Alloc, class _Iter, class... _Args>
            static void construct(_Alloc& alloc, _Iter it, _Args&&... args) {
                using _Alloc_traits = std::allocator_traits<_Alloc>;
                if constexpr (std::conjunction_v<std::is_nothrow_constructible<value_type, _Args...>,
                                                 uses_default_construct<allocator_type, pointer, _Args...>>)
                    construct_in_place(*it, std::forward<_Args>(args)...);
                else {
#if XSTL_HAS_CXX20
                    std::uninitialized_construct_using_allocator(std::to_address(it), alloc, std::forward<_Args>(args)...);
#else
                    _Alloc_traits::construct(alloc, std::addressof(*it), std::forward<_Args>(args)...);

#endif
                }
            }

            template <class _Alloc, class _Iter, class... _Args>
            static void construct_range(_Alloc& alloc, _Iter first, _Iter last, _Args&&... args) {
                if (first == last)
                    return;
                const pointer _old_first = first;
                auto          _guard     = scoped_guard([&] {
                    if (first != last)
                        destroy(alloc, _old_first, first);
                });
                if constexpr (sizeof...(args) == 0) {
                    // value construct [first, last)
                    if constexpr (std::conjunction_v<uses_default_construct<allocator_type, pointer>,
                                                     std::is_nothrow_constructible<value_type>>) {
                        std::uninitialized_value_construct(first, last);
                        first = last;
                    }
                    else {
                        do {
                            construct(alloc, first);
                        } while (++first != last);
                    }
                }
                else if constexpr (sizeof...(args) == 1) {
                    // fill [first, last) by a lref
                    static_assert(std::is_same_v<_Args..., const_reference>);
                    if constexpr (std::conjunction_v<uses_default_construct<allocator_type, pointer, _Args...>,
                                                     std::is_nothrow_constructible<value_type, _Args...>>)
                        first = std::uninitialized_fill(first, last, args...);
                    else {
                        static_assert(std::is_same_v<_Args..., const_reference>,
                                      "This is only allowed if _Args is the same as const_reference");
                        do {
                            construct(alloc, first, std::forward<_Args>(args)...);
                        } while (++first != last);
                    }
                }
                else if constexpr (sizeof...(args) == 3) {
                    // construct [first, last) from [src_first, src_last)
                    // e.g. construct_range(al, first, last, copy_op_tag, src_first, src_last)
                    // this is a weird function signature. the normal case would be to construct [src_first, src_last)
                    //  from [first, last) instead of [first, last) from [src_first, src_last). for higher abstraction, make a
                    //  compromise.
                    // 'last' is unused, but for unity of the interface, keep it
                    auto [_, _first, _last] = std::forward_as_tuple(std::forward<_Args>(args)...);

                    using _Strategy   = args_element_t<0, _Args...>;
                    using _Iter_val_t = typename std::iterator_traits<args_element_t<1, _Args...>>::value_type;
                    static_assert(is_any_of_v<_Strategy, copy_op_tag, move_op_tag>, "tag should be copy_op_tag or move_op_tag");
                    if constexpr (std::is_same_v<_Strategy, move_op_tag>) {
                        if constexpr (std::is_nothrow_move_constructible_v<value_type>
                                      || !std::is_copy_constructible_v<value_type>) {
                            if constexpr (uses_default_construct<allocator_type, pointer, value_type&&>::value)
                                first = std::uninitialized_move(_first, _last, first);
                            else {  // still might throw
                                for (; _first != _last; ++first, ++_first)
                                    construct(alloc, first, std::move(*_first));
                            }
                            _guard.dismiss();
                            return;
                        }
                    }
                    if constexpr (std::conjunction_v<uses_default_construct<allocator_type, pointer, _Iter_val_t>,
                                                     std::is_nothrow_constructible<value_type, _Iter_val_t>>)
                        first = std::uninitialized_copy(_first, _last, first);
                    else {
                        for (; _first != _last; ++first, ++_first)
                            construct(alloc, first, *_first);
                    }
                }
                else {
                    static_assert(always_false<_Args...>, "Should be unreachable");
                }
                _guard.dismiss();
            }

            template <class _Alloc>
            void free_all(_Alloc& alloc) {
                if (!is_data_inline() && heap())
                    alloc.deallocate(_Unshift_pointer(heap(), heapify_capacity_size),
                                     _vec.capacity() * sizeof(value_type) + heapify_capacity_size);
            }

            template <class _Alloc, class _Iter>
            static void destroy(_Alloc& alloc, _Iter first, _Iter last) {
                using _Alloc_traits = std::allocator_traits<_Alloc>;
                if constexpr (!std::conjunction_v<std::is_trivially_destructible<value_type>,
                                                  uses_default_destroy<_Alloc, pointer>>) {
                    for (; first != last; ++first)
                        _Alloc_traits::destroy(alloc, std::addressof(*first));
                }
            }

            template <class _Alloc>
            void swap_with_inline(_Alloc& alloc, _Self& right) {
                using _Alloc_traits = std::allocator_traits<_Alloc>;
                if (!is_data_inline()) {
                    right._Swap_inline_with_extern(alloc, *this);
                    return;
                }
                _Self *_small = this, _large = &right;
                if (_size.size() > right._size.size())
                    std::swap(_small, _large);

                const size_type _sz     = _small->_size.size();
                pointer         _sfirst = _small->_vec.buffer(), _lfirst = _large->_vec.buffer();
                const pointer   _llast = _lfirst + _large.size(), _slast = _sfirst + _sz, _old_lfirst = _lfirst;

                {
                    auto _guard = scoped_guard([&] {
                        destroy(alloc, _sfirst, _slast);
                        destroy(alloc, _lfirst, _llast);
                        _small->set_internal_size(_lfirst - _old_lfirst);
                        _large->set_internal_size(_lfirst - _old_lfirst);
                    });
#if XSTL_HAS_CXX20
                    if constexpr (std::is_nothrow_swappable_v<value_type>)
                        std::swap_ranges(_sfirst, _sfirst + _sz, _lfirst);
                    else
#endif
                        for (; _sfirst != _lfirst; ++_sfirst, ++_lfirst)
                            std::iter_swap(_sfirst, _lfirst);
                    _guard.dismiss();
                }

                if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
                    std::uninitialized_move(_lfirst, _llast, _sfirst);
                    destroy(alloc, _lfirst, _llast);
                    _small->_size.swap(_large->_size);
                }
                else {
                    _large->set_internal_size(_lfirst - _old_lfirst);
                    auto _guard = scoped_guard([&] {
                        destroy(alloc, _lfirst, _llast);
                        _small->set_internal_size(_lfirst - _old_lfirst);
                    });
                    for (; _lfirst != _llast; ++_sfirst, ++_lfirst) {
                        construct(alloc, _sfirst, std::move(*_lfirst)));
                        _Alloc_traits::destroy(alloc, _lfirst);
                    }
                    _small->set_internal_size(_lfirst - _old_lfirst);
                    _guard.dismiss();
                }
            }

            template <class _Alloc>
            void swap_with_extern(_Alloc& alloc, _Self& right) {
                if (is_data_inline())
                    _Swap_inline_with_extern(alloc, right);
                else {
                    _vec.swap_extern(right._vec);
                    _size.swap(right._size);
                }
            }

        private:  // private functions
            template <class _Alloc>
            void _Swap_inline_with_extern(_Alloc& alloc, _Self& right) {
                using _Alloc_traits = std::allocator_traits<_Alloc>;
                // swap this.heap with right.heap (no throw)
                _vec.swap_extern(right._vec);
                _size.swap(right._size);

                pointer       _first = _vec.buffer(), _rfirst = right.buffer();
                const pointer _last = _first + _size.size(), _old_first = _first;

                // move this.inline to right.inline
                if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
                    std::uninitialized_move(_first, _last, _rfirst);
                    destroy(alloc, _first, _last);
                }
                else {
                    auto _guard = scoped_guard([&] {
                        destroy(alloc, _first, _last);
                        right.set_size(_first - _old_first);
                    });
                    for (; _first != _last; ++_first, ++_rfirst) {
                        construct(alloc, _rfirst, std::move(*_first)));
                        _Alloc_traits::destroy(alloc, _first);
                    }
                    _guard.dismiss();
                }
            }

        private:  // data structure
            static size_t constexpr heapify_capacity_size =
                has_inline_capacity ? 0 : sizeof(xstl_aligned_storage_t<sizeof(size_type), alignof(value_type)>);

            template <bool _Inline_capacity>
            struct heap_ptr {
                // heap[-heapify_capacity_size] contains capacity
                size_type capacity() const noexcept {
                    return _heap ? *static_cast<size_type*>(_Unshift_pointer(_heap, heapify_capacity_size)) : 0;
                }
                void set_capacity(size_type c) noexcept {
                    *static_cast<size_type*>(_Unshift_pointer(_heap, heapify_capacity_size)) = c;
                }

                value_type* _heap = nullptr;
            };

            template <>
            struct heap_ptr<true> {
                size_type capacity() const noexcept { return _capacity; }
                void      set_capacity(size_type c) noexcept { _capacity = c; }

                value_type* _heap = nullptr;
                size_type   _capacity{ 0 };
            };

            using inline_storage_t =
                std::conditional_t<sizeof(value_type) * _Vec_base::max_inline != 0,
                                   aligned_storage_for_t<value_type[_Vec_base::max_inline ? _Vec_base::max_inline : 1u]>, char>;

            static bool constexpr has_inline_capacity =
                !_Vec_base::always_use_heap && sizeof(heap_ptr<true>) < sizeof(inline_storage_t)
                && (_Vec_base::has_byte_allocator || sizeof(value_type) <= sizeof(size_type));

            union _Data {
                explicit _Data() = default;

                pointer       buffer() noexcept { return reinterpret_cast<pointer>(&_storage); }
                const_pointer buffer() const noexcept { return const_cast<_Data*>(this)->buffer(); }
                pointer       heap() noexcept { return _pdata._heap; }
                void          set_heap(pointer ptr) noexcept { _pdata._heap = ptr; }
                const_pointer heap() const noexcept { return _pdata._heap; }

                size_type capacity() const noexcept { return _pdata.capacity(); }
                void      set_capacity(size_type c) noexcept { _pdata.set_capacity(c); }

                void swap_extern(_Data& right) noexcept {
                    std::swap(_pdata._heap, right._pdata._heap);
                    if constexpr (has_inline_capacity)
                        std::swap(_pdata._capacity, right._pdata._capacity);
                }

            private:
                heap_ptr<has_inline_capacity> _pdata;
                inline_storage_t              _storage;
            };

            struct _Actual_size_type {
                static constexpr size_type max_size() noexcept { return size_type(~extern_mask); }

                size_type size() const noexcept {
                    if constexpr (_Vec_base::always_use_heap)
                        return _val;
                    else
                        return _val & ~extern_mask;
                }

                void set_size(size_type sz) noexcept {
                    XSTL_EXPECT(sz <= max_size(), "the size of small_vector exceeds max_size()");
                    if constexpr (_Vec_base::always_use_heap)
                        _val = sz;
                    else
                        _val = (extern_mask & _val) | size_type(sz);
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

                void swap(_Actual_size_type& right) noexcept { std::swap(_val, right._val); }

            private:
                // We reserve two most significant bits of _val.
                static constexpr size_type extern_mask = _Vec_base::should_use_heap
                                                             ? size_type(1) << (sizeof(size_type) * 8 - 1)
                                                             : 0;  // 1000... If this bit is set to true, it means that the
                                                                   // current data is stored externally (i.e. on the heap)

                size_type _val{ 0 };
            };

            _Data             _vec;
            _Actual_size_type _size;
        };

        // deal with policies
        template <class _Tp, size_t _Capacity, class... _Policies>
        class _Small_vector_base {

        protected:
            using _Alloc_policy = _Select_allocator<std::allocator<_Tp>, _Policies...>;

            static constexpr bool        use_heap           = _Select_heap_policy<_Policies...>::use_heap;
            static constexpr bool        always_use_heap    = _Capacity == 0;
            static constexpr bool        should_use_heap    = use_heap || always_use_heap;
            static constexpr bool        has_byte_allocator = _Alloc_policy::has_byte_allocator;
            static constexpr std::size_t max_inline{ always_use_heap ? 0 : (std::max)(sizeof(_Tp*) / sizeof(_Tp), _Capacity) };

            using allocator_type = typename _Alloc_policy::type;

            using size_type =
                _Select_size_type<std::conditional_t<should_use_heap, _Smallest_size_t<_Capacity>, size_t>, _Policies...>::type;

            using _Allocator    = _Merge_alloc<typename _Alloc_policy::byte_allocator, allocator_type>;
            using _Alloc_traits = std::allocator_traits<_Allocator>;
        };
    }  // namespace

    template <class _Tp, size_t _Capacity, class... _Policies>
    class small_vector : private _Small_vector_base<_Tp, _Capacity, _Policies...> {
        using _Base         = _Small_vector_base<_Tp, _Capacity, _Policies...>;
        using _Self         = small_vector<_Tp, _Capacity, _Policies...>;
        using _Allocator    = typename _Base::_Allocator;
        using _Alloc_traits = typename _Base::_Alloc_traits;

    public:
        using allocator_type = typename _Base::allocator_type;
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
            _Construct_n(right.size(), right._Unchecked_begin(), right._Unchecked_end());
        }

        small_vector(small_vector&& right) noexcept(std::is_nothrow_move_constructible_v<value_type>);

        small_vector(std::initializer_list<value_type> il, const allocator_type& alloc = allocator_type())
            : _tpl(std::ignore, alloc) {
            _Construct_n(il.size(), il.begin(), il.end());
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
            if constexpr (is_forward_iterator_v<_Iter>) {
                const auto _len = static_cast<size_t>(std::distance(first, last));
                if constexpr (sizeof(_len) > sizeof(size_type)) {
                    if (_len > (std::numeric_limits<size_type>::max)()) {
                        throw std::length_error("size is too long for size_type");
                    }
                }
                _Construct_n(static_cast<size_type>(_len), std::move(first), std::move(last));
            }
            else {
                auto _guard = scoped_guard([&] {
                    _Scary_val::destroy(_Getal(), _Unchecked_begin(), _Unchecked_end());
                    _Get_val().free_all(_Getal());
                });
                for (; first != last; ++first)
                    emplace_back(*first);
                _guard.dismiss();
            }
        }

        template <class _Tp, size_t _Capacity, class... _Policies>
        XSTL_NODISCARD friend bool operator==(const small_vector<_Tp, _Capacity, _Policies...>& left,
                                              const small_vector<_Tp, _Capacity, _Policies...>& right) {
            return left.size() == right.size()
                   && std::equal(left._Unchecked_begin(), left._Unchecked_end(), right._Unchecked_begin());
        }

#ifdef __cpp_lib_three_way_comparison
        template <class _Tp, size_t _Capacity, class... _Policies>
        XSTL_NODISCARD friend synth_three_way_result<_Tp> operator<=>(const small_vector<_Tp, _Capacity, _Policies...>& left,
                                                                      const small_vector<_Tp, _Capacity, _Policies...>& right) {
            return std::lexicographical_compare_three_way(left._Unchecked_begin(), left._Unchecked_end(),
                                                          right._Unchecked_begin(), right._Unchecked_end(), synth_three_way{});
        }
#else
        template <class _Tp, size_t _Capacity, class... _Policies>
        XSTL_NODISCARD friend bool operator!=(const small_vector<_Tp, _Capacity, _Policies...>& left,
                                              const small_vector<_Tp, _Capacity, _Policies...>& right) {
            return !(right == left);
        }

        template <class _Tp, size_t _Capacity, class... _Policies>
        XSTL_NODISCARD friend bool operator<(const small_vector<_Tp, _Capacity, _Policies...>& left,
                                             const small_vector<_Tp, _Capacity, _Policies...>& right) {
            return std::lexicographical_compare(left._Unchecked_begin(), left._Unchecked_end(), right._Unchecked_begin(),
                                                right._Unchecked_end());
        }

        template <class _Tp, size_t _Capacity, class... _Policies>
        XSTL_NODISCARD friend bool operator>(const small_vector<_Tp, _Capacity, _Policies...>& left,
                                             const small_vector<_Tp, _Capacity, _Policies...>& right) {
            return right < left;
        }

        template <class _Tp, size_t _Capacity, class... _Policies>
        XSTL_NODISCARD friend bool operator<=(const small_vector<_Tp, _Capacity, _Policies...>& left,
                                              const small_vector<_Tp, _Capacity, _Policies...>& right) {
            return !(right < left);
        }

        template <class _Tp, size_t _Capacity, class... _Policies>
        XSTL_NODISCARD friend bool operator>=(const small_vector<_Tp, _Capacity, _Policies...>& left,
                                              const small_vector<_Tp, _Capacity, _Policies...>& right) {
            return !(left < right);
        }
#endif

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

        void swap(small_vector& right) noexcept(
            std::is_nothrow_move_constructible_v<value_type>&& std::is_nothrow_swappable_v<value_type>) {
            if XSTL_UNLIKELY (this == std::addressof(right))
                return;
            _Scary_val &_val = _Get_val(), _rval = right._Get_val();
            if (_rval.is_data_inline())
                _val.swap_with_inline(_rval, _Getal());
            else
                _val.swap_with_extern(_rval, _Getal());
        }

        void resize(size_type sz) { _Resize(sz); }

        void resize(size_type sz, const_reference value) { _Resize(sz, value); }

        pointer data() noexcept {
            _Scary_val& _val = _Get_val();
            if constexpr (_Base::always_use_heap)
                return _val.buffer();
            else
                return _val.is_data_inline() ? _val.buffer() : _val.heap();
        }

        const_pointer data() const noexcept { return const_cast<_Self*>(this)->data(); }

        template <class... _Args>
        iterator emplace(const_iterator pos, _Args&&... args) {
            _Scary_val& _val = _Get_val();
            auto&       _al  = _Getal();
            XSTL_EXPECT(pos._Get_cont() == std::addressof(_val) && _Scary_val::dereferable(std::addressof(_val), pos.base()),
                        "small_vector insert iterator outside range");
            if (pos == cend()) {  // at back, provide strong guarantee
                emplace_back(std::forward<_Args>(args)...);
                return end() - 1;
            }
            const difference_type       _off = pos - begin();
            value_proxy<allocator_type> _obj(_al, std::forward<_Args>(args)...);
            // after constructing _Obj, provide basic guarantee
            _Insert_n(pos, 1, std::move(_obj.get_value()));
            return begin() + _off;
        }

        void reserve(size_type sz) {
            if (sz > capacity())
                _Expand_to<_Allocate_strategy::Exactly>(sz);
        }

        size_type capacity() const noexcept { return _Get_val().is_data_inline() ? _Base::max_inline : _Get_val().capacity(); }

        void shrink_to_fit() {
            _Scary_val&     _val = _Get_val();
            auto&           _al  = _Getal();
            const size_type _sz  = _val.size();
            if (_val.is_data_inline())
                return;
            if (_sz < _Base::max_inline) {
                _Scary_val::construct_range(_al, _vec.buffer(), _vec.buffer() + _sz, move_op_tag{}, _vec.heap(),
                                            _vec.heap() + _sz);
                _val.free_all(_al);
                _val.set_data_inline(true);
            }
            else {
                _Expand_to<_Allocate_strategy::Exactly>(_sz);
            }
        }

        template <class... _Args>
        reference emplace_back(_Args&&... args) {
            _Scary_val&     _val = _Get_val();
            auto&           _al  = _Getal();
            const size_type _sz  = _val.size();
            pointer         _pos = data() + _sz;

            if (_sz < _val.capacity()) {
                _val.construct(_al, _pos, std::forward<_Args>(args)...);
                _val.set_size(_sz + 1);
                return *_pos;
            }
            if constexpr (!_Base::should_use_heap)
                throw std::length_error("max_size exceeded in small_vector");
            _Expand_to<_Allocate_strategy::EmplaceAtBack>(_sz + 1, std::forward<_Args>(args)...);
            return *(_val.heap() + _sz);
        }

        void push_back(value_type&& value) { emplace_back(std::move(value)); }

        void push_back(const_reference value) { emplace_back(value); }

        void pop_back() {
            XSTL_EXPECT(!empty(), "vector empty before pop");
            // ideally this would be implemented in terms of erase(end() - 1) to reuse
            // the higher-level abstraction, but neither Clang or GCC are able to
            // optimize it away. if you change this, please verify (with disassembly)
            // that the generated code on -O3 (and ideally -O2) stays short
            _Downsize(size() - 1);
        }

        iterator insert(const_iterator pos, value_type&& value) { return emplace(pos, std::move(value)); }

        iterator insert(const_iterator pos, const_reference value) { return emplace(pos, value); }

        iterator insert(const_iterator pos, size_type n, const_reference value) {
            _Scary_val& _val = _Get_val();
            auto&       _al  = _Getal();
            XSTL_EXPECT(pos._Get_cont() == std::addressof(_val) && _Scary_val::dereferable(std::addressof(_val), pos.base()),
                        "small_vector insert iterator outside range");
            if (pos == cend()) {  // at back, provide strong guarantee
                emplace_back(value);
                return end() - 1;
            }
            const difference_type _off = pos - begin();
            _Insert_n(pos, n, value);
            return _Make_iter(data() + _off)
        }

        template <class _Iter, XSTL_REQUIRES_(is_input_iterator_v<_Iter>)>
        iterator insert(const_iterator pos, _Iter first, _Iter last) {
            _Scary_val& _val = _Get_val();
            auto&       _al  = _Getal();
            XSTL_EXPECT(pos._Get_cont() == std::addressof(_val) && _Scary_val::dereferable(std::addressof(_val), pos.base()),
                        "small_vector insert iterator outside range");
            if constexpr (is_forward_iterator_v<_Iter>) {
                _Insert_n(pos, std::distance(first, last), first, last);
            }
            else {
            }
        }

        iterator insert(const_iterator pos, std::initializer_list<value_type> il) { return insert(pos, il.begin(), il.end()); }

        iterator erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<value_type>) {
            XSTL_EXPECT(pos._Get_cont() == std::addressof(_Get_val())
                            && _Scary_val::dereferable(std::addressof(_Get_val()), pos.base()),
                        "small_vector erase iterator outside range");

            // ideally this would be implemented in terms of erase(pos, pos + 1) to reuse
            // the higher-level abstraction, but neither Clang or GCC are able to
            // optimize it away. if you change this, please verify (with disassembly)
            // that the generated code on -O3 (and ideally -O2) stays short
            std::move<_Unchecked_const_iterator>(pos + 1, _Unchecked_end(), pos);
            _Downsize(size() - 1);
            return _Make_iter(pos.base());
        }

        iterator erase(const_iterator first, const_iterator last) noexcept(std::is_nothrow_move_assignable_v<value_type>) {
            XSTL_EXPECT(first._Get_cont() == std::addressof(_Get_val()) && last._Get_cont() == std::addressof(_Get_val())
                            && first <= last && _Scary_val::dereferable(std::addressof(_Get_val()), first.base())
                            && _Scary_val::dereferable(std::addressof(_Get_val()), last.base()),
                        "small_vector erase iterator outside range");
            if (first != last) {
                std::move<_Unchecked_const_iterator>(last, _Unchecked_end(), first);
                _Downsize(size() - std::distance(first, last));
            }
            return _Make_iter(first.base());
        }

        void clear() {
            // ideally this would be implemented in terms of erase(begin(), end()) to
            // reuse the higher-level abstraction, but neither Clang or GCC are able to
            // optimize it away. if you change this, please verify (with disassembly)
            // that the generated code on -O3 (and ideally -O2) stays short
            _Downsize(0);
        }

        template <class _Iter, XSTL_REQUIRES_(is_input_iterator_v<_Iter>)>
        void assign(_Iter first, _Iter last) {
            clear();
            insert(begin(), first, last);
        }

        void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }

        void assign(size_type n, const_reference t) {
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

        ~small_vector() {
            _Scary_val::destroy(_Getal(), _Unchecked_begin(), _Unchecked_end());
            _Get_val().free_all(_Getal());
        }

    private:
        static iterator unconst(const_iterator it) { return const_cast<iterator>(it); }

        void _Downsize(size_type sz) {
            XSTL_EXPECT(sz <= size(), "unexpected size for _Downsize");
            _Scary_val::destroy(_Getal(), _Unchecked_begin() + sz, _Unchecked_end());
            _Get_val().set_size(sz);
        }

        enum _Allocate_strategy { Construct, NoConstruct, EmplaceAtBack, Exactly };

        template <_Allocate_strategy _Strat>
        std::pair<void*, pointer> _Allocate_for(size_type& newsz) {
            if (newsz > max_size())
                throw std::length_error("max_size exceeded in small_vector");

            XSTL_EXPECT(_Base::should_use_heap, "allocation on heap is not allowed when should_use_heap == false");
            if constexpr (_Strat != _Allocate_strategy::Exactly) {
                newsz = (std::max)(newsz, (std::min)((capacity() + capacity() / 2, max_size())) * sizeof(value_type)
                                              + _Scary_val::heapify_capacity_size);
            }

            auto* _ptr = _Getal().allocate(newsz);
            return { _ptr, _Shift_pointer(_ptr, _Scary_val::heapify_capacity_size) };
        }

        template <class... _Args>
        void _Insert_n(const_iterator pos, const size_type n, _Args&&... args) {  // pos shouldn't be end, checking forwardly
            const size_type       _sz = size(), _old_newsz = n + _sz;
            _Scary_val&           _val  = _Get_val();
            const difference_type _off  = pos - begin();
            const pointer         _pold = data(), _old_last = _pold + _sz, _oldpos = _pold + _off;
            auto&                 _al = _Getal();
            if (_old_newsz >= capacity()) {
                size_type _newsz      = _old_newsz;
                auto [_ptr, _pnew]    = _Allocate_for<_Allocate_strategy::NoConstruct>(_newsz);
                const pointer _newpos = _pnew + _off;
                auto          _guard  = [&] { _Alloc_traits::deallocate(_al, _ptr, _newsz); };
                // construct new values
                if constexpr (sizeof...(args) == 1) {
                    if constexpr (std::is_same_v<const_reference, _Args...>)
                        _Scary_val::construct_range(_al, _newpos, _newpos + n, std::forward<_Args>(args)...);
                    else
                        _Scary_val::construct(_al, _newpos, std::forward<_Args>(args)...);
                }
                else if constexpr (sizeof...(args) == 2) {
                    static_assert(is_input_iterator_v<args_element_t<0, _Args...>>,
                                  "should be iteraor when sizeof...(args) == 2");
                    _Scary_val::construct_range(_al, _newpos, _newpos + n, copy_op_tag{}, std::forward<_Args>(args)...);
                }
                else {
                    static_assert(always_false<_Args...>, "should be unreachable");
                }
                // move old data
                _Scary_val::construct_range(_al, _pnew, _newpos, move_op_tag{}, _pold, _oldpos);
                _Scary_val::construct_range(_al, _newpos + n, _pnew + _old_newsz, move_op_tag{}, _oldpos, _old_last);
                _guard.dismiss();
                _Scary_val::destroy(_al, _pold, _old_last);
                _val.free_all(_al);
                _val.set_heap(_ptr);
                _val.set_capacity(_newsz);
                _val.set_data_inline(false);
            }
            else {
                if constexpr (sizeof...(args) == 1) {
                    if constexpr (std::is_same_v<const_reference, _Args...>) {
                        const value_proxy<allocator_type> _tmp_storage(
                            _al, std::forward<_Args>(args)...);  // handle aliasing. It handles the case where const _Ty& _Val
                                                                 // refers to
                        // an element within the vector itself
                        const_reference _tmp           = _tmp_storage.get_value();
                        const size_type _rest_elements = end() - pos;
                        if (n > _rest_elements) {  // new stuff spills off end
                            _Scary_val::construct_range(_al, _old_last, _old_last + n - _rest_elements, _tmp);
                            _Scary_val::construct_range(_al, _old_last + n - _rest_elements, _pold + _old_newsz, move_op_tag{},
                                                        _oldpos, _old_last);
                            std::fill(_oldpos, _old_last, _tmp);
                        }
                        else {  // new stuff can all be assigned
                            _Scary_val::construct_range(_al, _old_last, _old_last + n, move_op_tag{}, _old_last - n, _old_last);
                            std::move_backward(_oldpos, _old_last - n, _old_last);
                            std::fill(_oldpos, _oldpos + n, _tmp);
                        }
                    }
                    else {
                        _Scary_val::construct(_al, _old_last, std::move(*(_old_last - 1)));
                        std::move_backward(_oldpos, _old_last - 1, _old_last);
                        *_oldpos = std::forward<_Args>(args)...;
                    }
                }
                else if constexpr (sizeof...(args) == 2) {
                    auto [_first, _last]           = std::forward_as_tuple(std::forward<_Args>(args)...);
                    const size_type _rest_elements = end() - pos;
                    if (n > _rest_elements) {  // new stuff spills off end
                        _Scary_val::construct_range(_al, _old_last, _old_last + n - _rest_elements, _tmp);
                        _Scary_val::construct_range(_al, _old_last + n - _rest_elements, _pold + _old_newsz, move_op_tag{},
                                                    _oldpos, _old_last);
                        std::fill(_oldpos, _old_last, _tmp);
                    }
                    else {  // new stuff can all be assigned
                        _Scary_val::construct_range(_al, _old_last, _old_last + n, move_op_tag{}, _old_last - n, _old_last);
                        std::move_backward(_oldpos, _old_last - n, _old_last);
                        std::fill(_oldpos, _oldpos + n, _tmp);
                    }
                }
            }
            _val.set_size(_old_newsz);
        }

        template <_Allocate_strategy _Strat, class... _Args>
        void _Expand_to(size_type newsz, _Args&&... args) {
            const size_type _old_newsz = newsz, _sz = size();
            _Scary_val&     _val = _Get_val();
            auto&           _al  = _Getal();
            auto [_ptr, _pnew]   = _Allocate_for<_Strat>(newsz);
            const pointer _pold  = data();
            auto          _guard = [&] { _Alloc_traits::deallocate(_al, _ptr, newsz); };
            // construct new values (if need)
            if constexpr (_Strat == _Allocate_strategy::Construct)
                _Scary_val::construct_range(_al, _pnew + _sz, _pnew + _old_newsz, std::forward<_Args>(args)...);
            else if constexpr (_Strat == _Allocate_strategy::EmplaceAtBack)
                _Scary_val::construct(_al, _pnew + _sz, std::forward<_Args>(args)...);
            // move old data
            _Scary_val::construct_range(_al, _pnew, _pnew + _sz, move_op_tag{}, _pold, _pold + _sz);
            _guard.dismiss();
            _Scary_val::destroy(_al, _pold, _pold + _sz);
            _val.free_all(_al);
            _val.set_heap(_ptr);
            _val.set_capacity(newsz);
            _val.set_data_inline(false);
            _val.set_size(_old_newsz);
        }

        template <class... _Args>
        void _Construct_n(size_type n, _Args&&... args) {
            if (n == 0)
                return;
            auto& _al = _Getal();
            _Expand_to<_Allocate_strategy::NoConstruct>(n);
            pointer _first = data();
            if constexpr (sizeof...(args) <= 1)
                _Scary_val::construct_range(_al, _first, _first + n, std::forward<_Args>(args)...);
            else if constexpr (sizeof...(args) == 2)
                _Scary_val::construct_range(_al, _first, _first + n, copy_op_tag{}, std::forward<_Args>(args)...);
            else {
                static_assert(always_false<_Args...>, "Should be unreachable");
            }
            _Get_val().set_size(n);
        }

        template <class... _Args>
        void _Resize(size_type newsz, _Args&&... args) {
            if (newsz < size())
                _Downsize(newsz);
            else if (newsz > size()) {
                if (newsz > capacity()) {
                    _Expand_to<_Allocate_strategy::Construct>(newsz, std::forward<_Args>(args)...);
                }
                else {
                    _Scary_val::construct_range(_Getal(), _Unchecked_end(), _Unchecked_begin() + newsz,
                                                std::forward<_Args>(args)...);
                    _Get_val().set_size(newsz);
                }
            }
        }

        _Scary_val&       _Get_val() noexcept { return std::get<0>(_tpl); }
        const _Scary_val& _Get_val() const noexcept { return std::get<0>(_tpl); }
        _Allocator&       _Getal() noexcept { return std::get<1>(_tpl); }
        const _Allocator& _Getal() const noexcept { return std::get<1>(_tpl); }

        iterator _Make_iter(pointer ptr) noexcept {
            return iterator(ptr, std::addressof(_Get_val());
        }
        const_iterator _Make_citer(pointer ptr) const noexcept {
            return const_iterator(ptr, std::addressof(_Get_val());
        }
        _Unchecked_iterator _Make_uiter(pointer ptr) const noexcept {
            return _Unchecked_iterator(ptr, std::addressof(_Get_val()));
        }
        _Unchecked_const_iterator _Make_uciter(pointer ptr) const noexcept {
            return _Unchecked_const_iterator(ptr, std::addressof(_Get_val()));
        }

        _Unchecked_iterator       _Unchecked_begin() noexcept { return _Make_uiter(data()); }
        _Unchecked_iterator       _Unchecked_end() noexcept { return _Make_uiter(data() + static_cast<difference_type>(size())); }
        _Unchecked_const_iterator _Unchecked_begin() const noexcept { return _Make_uciter(data()); }
        _Unchecked_const_iterator _Unchecked_end() const noexcept {
            return _Make_uciter(data() + static_cast<difference_type>(size()));
        }

        compressed_tuple<_Scary_val, _Allocator> _tpl;
    };

    template <class _Tp, size_t _Capacity, class... _Policies>
    small_vector<_Tp, _Capacity, _Policies...>::small_vector(small_vector&& right) noexcept(
        std::is_nothrow_move_constructible_v<value_type>)
        : _tpl(std::ignore, std::move(right._Getal())) {
        _Scary_val &_val = _Get_val(), &_rval = right._Get_val();
        if (_rval.is_data_inline()) {
            _Scary_val::constuct_range(_Getal(), _val.buffer(), _val.buffer() + right.size(), move_op_tag{}, _rval.buffer(),
                                       _rval.buffer() + _rval.size());
            right.clear();
        }
        else
            _val.swap_with_extern(_rval);
    }

}  // namespace xstl
#endif