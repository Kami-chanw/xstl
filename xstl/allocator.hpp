/*
 *   Copyright (c) 2022 Kamichanw. All rights reserved.
 *   @file allocator.hpp
 *   @brief The allocator library contains 3 types of allocator:
 *	1. malloc_alloc
 *	2. defualt_alloc
 *	3. unique_alloc
 *   @author Shen Xian e-mail: 865710157@qq.com
 *   @version 2.0
 */

#ifndef _ALLOCATORS_HPP_
#define _ALLOCATORS_HPP_

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <new>
#include <type_traits>

#define ALIGN_SZ 8 /* the size of a memory chunk */
#define LIST_SZ 16 /* the size of free_list */
#define UNIQUE_INST(_Tp) (sizeof(_Tp) + ALIGN_SZ - 1) & ~(ALIGN_SZ - 1)

/**
 *	@brief appoints underlying allocator
 */
#ifdef STD_ALLOC
#define DEFAULT_ALLOC(_Tp) std::allocator<_Tp>
#elif defined(MALLOC_ALLOC)
#define DEFAULT_ALLOC(_Tp) xstl::alloc_wrapper<_Tp, xstl::malloc_alloc<0>>
#elif defined(UNIQUE_ALLOC)
#define DEFAULT_ALLOC(_Tp) xstl::alloc_wrapper<_Tp, xstl::unique_alloc<UNIQUE_INST(_Tp)>>
#else
#define DEFAULT_ALLOC(_Tp) xstl::alloc_wrapper<_Tp, xstl::defualt_alloc<0>>
#endif

/**
 *	@brief confirm whether needs to use muti-threads
 */
#if !defined(NO_THREADS) || defined(USE_THREADS)
#define USE_THREADS true
#else
#define USE_THREADS false
#endif

namespace xstl {
    /**
     *	@class allocator_base
     *	@brief basic defination for allocators
     */
    class allocator_base {
    public:
        union block_t {
            block_t* _next;
            char     _data[1];
        };

        /**
         *	@brief the size of every memory block in the free list.
         */
        enum : size_t { ALIGN = ALIGN_SZ };
        using block_ptr = block_t*;

        static inline constexpr size_t round_up(size_t n, size_t mask = ALIGN) { return (n + mask - 1) & ~(mask - 1); }
    };

    /**
     *	@class malloc_alloc
     *	@brief allocates memory by malloc in c
     */
    template <int _Inst>
    class malloc_alloc : private allocator_base {
    public:
        static void* allocate(size_t n);
        static void  deallocate(void* ptr, size_t);
        static void* reallocate(void* ptr, size_t oldsz, size_t newsz);
        static void (*set_malloc_handler(void (*new_handler)()))();

    private:
        static void (*_malloc_alloc_oom_handler)();
        static void* _Oom_malloc(size_t);          // out of memory
        static void* _Oom_realloc(void*, size_t);  // out of memory
    };

    template <int _Inst>
    void (*malloc_alloc<_Inst>::_malloc_alloc_oom_handler)() = 0;

    template <int _Inst>
    void* malloc_alloc<_Inst>::allocate(size_t n) {
        void* _res = malloc(n);
        if (_res == nullptr)
            _res = _Oom_malloc(n);
        return _res;
    }

    template <int _Inst>
    void malloc_alloc<_Inst>::deallocate(void* ptr, size_t) {
        free(ptr);
    }

    template <int _Inst>
    void* malloc_alloc<_Inst>::reallocate(void* ptr, size_t, size_t newsz) {
        void* _res = realloc(ptr, newsz);
        if (_res == nullptr)
            _res = _Oom_realloc(ptr, newsz);
        return _res;
    }

    template <int _Inst>
    void (*malloc_alloc<_Inst>::set_malloc_handler(void (*new_handler)()))() {
        void (*_old)()            = _malloc_alloc_oom_handler;
        _malloc_alloc_oom_handler = new_handler;
        return _old;
    }

    template <int _Inst>
    void* malloc_alloc<_Inst>::_Oom_malloc(size_t n) {
        void* _res;
        void (*_handler)();
        while (true) {
            _handler = _malloc_alloc_oom_handler;
            if (_handler == nullptr)
                throw std::bad_alloc();
            (*_handler)();
            _res = malloc(n);
            if (_res)
                return _res;
        }
    }

    template <int _Inst>
    void* malloc_alloc<_Inst>::_Oom_realloc(void* ptr, size_t newsz) {
        void* _res;
        void (*_handler)();
        while (true) {
            _handler = _malloc_alloc_oom_handler;
            if (_handler == nullptr)
                throw std::bad_alloc();
            (*_handler)();
            _res = realloc(ptr, newsz);
            if (_res)
                return _res;
        }
    }

    /**
     *	@class defualt_alloc
     *	@brief allocates small memory by memory pool
     */
    template <int _Inst, bool _Threads = USE_THREADS>
    class defualt_alloc : private allocator_base {
        using par_alloc = malloc_alloc<_Inst>;
        using _Base     = allocator_base;
        using _Base::ALIGN;
        using _Base::block_ptr;
        using _Base::round_up;
        enum : size_t { MAX_SZ = LIST_SZ * ALIGN };

    public:
        static void* allocate(size_t n);
        static void  deallocate(void* ptr, size_t n);
        static void* reallocate(void* ptr, size_t oldsz, size_t newsz);

    private:
        static char*         _Getchunk(size_t);
        inline static size_t _Fit_idx(size_t);
        class list_t {
        public:
            list_t() { memset(_list, 0, sizeof(block_ptr) * LIST_SZ); }
            block_ptr  operator[](size_t idx) { return _list[idx]; }
            block_ptr* operator+(size_t off) { return _list + off; }
            block_ptr  _list[LIST_SZ];
        };

        static char*  _pool_start;
        static char*  _pool_end;
        static size_t _pool_sz;
        static list_t _free_list;

#if defined(USE_THREADS)
        static std::mutex _mutex;
#endif
    };

    template <int _Inst, bool _Threads>
    typename defualt_alloc<_Inst, _Threads>::list_t defualt_alloc<_Inst, _Threads>::_free_list;

    template <int _Inst, bool _Threads>
    char* defualt_alloc<_Inst, _Threads>::_pool_start = nullptr;

    template <int _Inst, bool _Threads>
    char* defualt_alloc<_Inst, _Threads>::_pool_end = nullptr;

    template <int _Inst, bool _Threads>
    size_t defualt_alloc<_Inst, _Threads>::_pool_sz = 0;
#if defined(USE_THREADS)
    template <int _Inst, bool _Threads>
    std::mutex defualt_alloc<_Inst, _Threads>::_mutex;
#endif

    template <int _Inst, bool _Threads>
    void* defualt_alloc<_Inst, _Threads>::allocate(size_t n) {
        if (n > MAX_SZ)
            return par_alloc::allocate(n);
#if defined(USE_THREADS)
        if (_Threads)
            std::lock_guard garud(_mutex);
#endif
        typename _Base::block_ptr *_free_block_ptr = _free_list + _Fit_idx(n), _res = *_free_block_ptr;
        if (_res == nullptr)                           // if there is no node in free list
            _res = (block_ptr)_Getchunk(round_up(n));  // get a chunk of memory
        else
            *_free_block_ptr = _res->_next;
        return _res;
    }

    template <int _Inst, bool _Threads>
    void defualt_alloc<_Inst, _Threads>::deallocate(void* ptr, size_t n) {
        if (n > MAX_SZ) {
            par_alloc::deallocate(ptr, n);
            return;
        }
        if (ptr == nullptr)
            return;
#if defined(USE_THREADS)
        if (_Threads)
            std::lock_guard garud(_mutex);
#endif
        typename _Base::block_ptr* _free_block_ptr = _free_list + _Fit_idx(n);
        ((typename _Base::block_ptr)ptr)->_next    = *_free_block_ptr;
        *_free_block_ptr                           = (typename _Base::block_ptr)ptr;  // relink to free list
    }

    template <int _Inst, bool _Threads>
    void* defualt_alloc<_Inst, _Threads>::reallocate(void* ptr, size_t oldsz, size_t newsz) {
        if (newsz > MAX_SZ && oldsz > MAX_SZ)
            return par_alloc::reallocate(ptr, oldsz, newsz);
        if (round_up(oldsz) == round_up(newsz))
            return ptr;
        void* _res = allocate(newsz);
        memcpy(_res, ptr, newsz > oldsz ? oldsz : newsz);
        deallocate(ptr, oldsz);
        return _res;
    }

    template <int _Inst, bool _Threads>
    size_t defualt_alloc<_Inst, _Threads>::_Fit_idx(size_t n) {
        return (n + ALIGN - 1) / ALIGN - 1;
    }

    template <int _Inst, bool _Threads>
    char* defualt_alloc<_Inst, _Threads>::_Getchunk(size_t _size) {
        char*  _res;
        size_t _left_sz = _pool_end - _pool_start;
        if (_left_sz >= _size) {  // if pool still has enough free memory, adjusts the size and return
            _res = _pool_start;
            _pool_start += _size;
            return _res;
        }
        if (_left_sz) {  // if pool isn't enough, put the left memory into free list
            block_ptr* _free_block_ptr                      = _free_list + _Fit_idx(_left_sz);
            reinterpret_cast<block_ptr>(_pool_start)->_next = *_free_block_ptr;
            *_free_block_ptr                                = reinterpret_cast<block_ptr>(_pool_start);
        }
        size_t _total_sz = _size * 40 + round_up(_pool_sz >> 4);
        _pool_start      = reinterpret_cast<char*>(malloc(_total_sz));
        if (_pool_start == NULL) {  // if memory allocation failed, check the free list
            block_ptr* _free_block_ptr;
            block_ptr  _node;
            for (size_t i = _size; i <= MAX_SZ; i += ALIGN) {  // travals the whole free list
                _free_block_ptr = _free_list + _Fit_idx(i);
                _node           = *_free_block_ptr;
                if (_node) {
                    *_free_block_ptr = _node->_next;
                    _pool_start      = (char*)_node;
                    _pool_end        = _pool_start + i;
                    return _Getchunk(_size);  // in order to maintain the other information
                }
            }
            _pool_end   = nullptr;
            _pool_start = (char*)malloc_alloc<_Inst>::allocate(_total_sz);
        }
        _pool_sz += _total_sz;
        _pool_end = _pool_start + _total_sz;
        return _Getchunk(_size);
    }

    /**
     *	@class defualt_alloc
     *	@brief allocator for only one class
     */
    template <int _Inst, bool _Threads = USE_THREADS>
    class unique_alloc : private allocator_base {
        using par_alloc = malloc_alloc<_Inst>;
        using _Base     = allocator_base;
        using _Base::block_ptr;
        using _Base::round_up;

    public:
        static void* allocate(size_t n);
        static void  deallocate(void* ptr, size_t n);
        static void* reallocate(void* ptr, size_t oldsz, size_t newsz);

    private:
        inline static char* _Make_list(size_t);
        static block_ptr    _free_list_header;

        static int _times;
#if defined(USE_THREADS)
        static std::mutex _mutex;
#endif
    };

    template <int _Inst, bool _Threads>
    typename unique_alloc<_Inst, _Threads>::block_ptr unique_alloc<_Inst, _Threads>::_free_list_header = nullptr;
#if defined(USE_THREADS)
    template <int _Inst, bool _Threads>
    std::mutex unique_alloc<_Inst, _Threads>::_mutex;
#endif

    template <int _Inst, bool _Threads>
    int unique_alloc<_Inst, _Threads>::_times = 2;

    template <int _Inst, bool _Threads>
    char* unique_alloc<_Inst, _Threads>::_Make_list(size_t n) {
        int    _nobjs    = 20 * (_times *= 1.8);
        size_t _total_sz = n * _nobjs;
        char*  _chunk    = reinterpret_cast<char*>(malloc(_total_sz));
        if (_chunk == NULL)
            _chunk = reinterpret_cast<char*>(par_alloc::allocate(_total_sz));
        block_ptr _curr;
        _free_list_header = _curr = reinterpret_cast<block_ptr>(_chunk + n);
        while (--_nobjs)
            _curr = _curr->_next = reinterpret_cast<block_ptr>(_curr + n);
        _curr->_next = nullptr;
        return _chunk;
    }

    template <int _Inst, bool _Threads>
    void* unique_alloc<_Inst, _Threads>::allocate(size_t n) {
#if defined(USE_THREADS)
        if (_Threads)
            std::lock_guard garud(_mutex);
#endif
        block_ptr _res = _free_list_header;
        if (_res == nullptr)
            _res = (block_ptr)_Make_list(round_up(n));
        else
            _free_list_header = _res->_next;
        return _res;
    }

    template <int _Inst, bool _Threads>
    void unique_alloc<_Inst, _Threads>::deallocate(void* ptr, size_t n) {
        if (ptr == nullptr)
            return;
#if defined(USE_THREADS)
        if (_Threads)
            std::lock_guard garud(_mutex);
#endif
        ((block_ptr)ptr)->_next = _free_list_header;
        _free_list_header       = (block_ptr)ptr;
    }

    template <int _Inst, bool _Threads>
    void* unique_alloc<_Inst, _Threads>::reallocate(void* ptr, size_t oldsz, size_t newsz) {
        if (round_up(oldsz) == round_up(newsz))
            return ptr;
        void* _res = allocate(newsz);
        memcpy(_res, ptr, newsz > oldsz ? oldsz : newsz);
        deallocate(ptr, oldsz);
        return _res;
    }

#ifdef MALLOC_ALLOC
    template <int _Inst>
    using alloc = malloc_alloc<_Inst>;
    template <int _Inst>
    using single_thread_alloc = malloc_alloc<_Inst>;
    template <int _Inst>
    using multi_thread_alloc = malloc_alloc<_Inst>;
#elif defined UNIQUE_ALLOC
    template <class _Tp>
    using alloc = unique_alloc<(sizeof(_Tp) + 7) & ~7, USE_THREADS>;

    template <class _Tp>
    using single_thread_alloc = defualt_alloc<(sizeof(_Tp) + 7) & ~7, false>;

    template <class _Tp>
    using multi_thread_alloc = defualt_alloc<(sizeof(_Tp) + 7) & ~7, true>;
#else
    template <class _Tp>
    using alloc = defualt_alloc<0, USE_THREADS>;

    template <class _Tp>
    using single_thread_alloc = defualt_alloc<0, false>;

    template <class _Tp>
    using multi_thread_alloc = defualt_alloc<0, true>;
#endif

    /**
     *	@class alloc_wrapper
     *	@brief makes static underlying allocator instantiable and allocate by sizeof(_Tp)
     */
    template <class _Tp, class _Alloc>
    class alloc_wrapper {
    public:
        static_assert(!std::is_const_v<_Tp>, "The C++ Standard forbids containers of const elements "
                                             "because allocator<const T> is ill-formed.");

        using size_type                              = size_t;
        using difference_type                        = ptrdiff_t;
        using value_type                             = _Tp;
        using propagate_on_container_copy_assignment = std::false_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap            = std::false_type;
        using is_always_equal                        = std::true_type;

        alloc_wrapper() = default;
        alloc_wrapper(const alloc_wrapper&) noexcept {}
        template <class _OtherTp>
        alloc_wrapper(const alloc_wrapper<_OtherTp, _Alloc>&) noexcept {}
        ~alloc_wrapper() noexcept {}

        _Tp* allocate(size_type n, const void* = nullptr) {
            return n != 0 ? static_cast<_Tp*>(_Alloc::allocate(n * sizeof(_Tp))) : nullptr;
        }
        void deallocate(_Tp* ptr, size_type n) { _Alloc::deallocate(ptr, n * sizeof(_Tp)); }
    };

    template <class _Alloc>
    class alloc_wrapper<void, _Alloc> {
        using value_type      = void;
        using size_type       = size_t;
        using difference_type = ptrdiff_t;

        using propagate_on_container_copy_assignment = std::false_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap            = std::false_type;
        using is_always_equal                        = std::true_type;
    };

    template <class _Tp, bool _Threads = USE_THREADS>
    using malloc_allocator = alloc_wrapper<_Tp, malloc_alloc<0>>;
    template <class _Tp, bool _Threads = USE_THREADS>
    using unique_allocator = alloc_wrapper<_Tp, unique_alloc<(sizeof(_Tp) + 7) & ~7, _Threads>>;
    template <class _Tp, bool _Threads = USE_THREADS>
    using default_allocator = alloc_wrapper<_Tp, defualt_alloc<0, _Threads>>;

    template <class _Tp, class _Alloc>
    inline bool operator==(const alloc_wrapper<_Tp, _Alloc>& lhs, const alloc_wrapper<_Tp, _Alloc>& rhs) {
        return true;
    }

    template <class _Tp, class _Alloc>
    inline bool operator!=(const alloc_wrapper<_Tp, _Alloc>& lhs, const alloc_wrapper<_Tp, _Alloc>& rhs) {
        return false;
    }

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

    template <class _Alloc>
    inline constexpr bool alloc_pocca_v = std::allocator_traits<_Alloc>::propagate_on_container_copy_assignment::value
                                          && !std::allocator_traits<_Alloc>::is_always_equal::value;

    enum class pocma_values {
        IsEqual,      // usually allows contents to be stolen (e.g. with swap)
        Propagate,    // usually allows the allocator to be propagated, and then contents stolen
        NoPropagate,  // usually turns moves into copies
    };

    template <class _Alloc>
    inline constexpr pocma_values alloc_pocma_v =
        std::allocator_traits<_Alloc>::is_always_equal::value
            ? pocma_values::IsEqual
            : (std::allocator_traits<_Alloc>::propagate_on_container_move_assignment::value ? pocma_values::Propagate
                                                                                            : pocma_values::NoPropagate);

    template <class _Ty, class... _Args>
    constexpr void construct_in_place(_Ty& obj, _Args&&... args) noexcept(std::is_nothrow_constructible_v<_Ty, _Args...>) {
#if __cplusplus >= 202002L
        if (std::is_constant_evaluated())
            std::construct_at(std::addressof(obj), std::forward<_Args>(args)...);
        else
#endif
            ::new (const_cast<void*>(static_cast<const volatile void*>(std::addressof(obj)))) _Ty(std::forward<_Args>(args)...);
    }

    template <class _Ty>
    constexpr void destroy_in_place(_Ty& obj) noexcept {
        if constexpr (std::is_array_v<_Ty>) {
            if constexpr (!std::is_trivially_destructible_v<typename std::iterator_traits<_Ty>::value_type>) {
                for (auto _first = obj, _last = obj + std::extent_v<_Ty>; _first != _last; ++_first)
                    destroy_in_place(*_first);
            }
        }
        else
            obj.~_Ty();
    }

#if __cplusplus >= 202207L
    template <std::size_t _Len, std::size_t _Align>
    struct xstl_aligned_storage_t {
        alignas(_Align) unsigned char data[_Len];
    };
#else
    template <std::size_t _Len, std::size_t _Align>
    using xstl_aligned_storage_t = std::aligned_storage_t<_Len, _Align>;
#endif

    template <class _Tp>
    using aligned_storage_for_t = xstl_aligned_storage_t<sizeof(_Tp), alignof(_Tp)>;

    template <class _Alloc>
    using is_default_allocator = std::is_same<std::allocator<typename std::allocator_traits<_Alloc>::value_Type>, _Alloc>;

    template <class _Void, class... _Types>
    struct has_no_allocator_construct : true_type {};

    _STL_DISABLE_DEPRECATED_WARNING
    template <class _Alloc, class _Ptr, class... _Args>
    struct has_no_allocator_construct<
        std::void_t<decltype(std::declval<_Alloc&>().construct(std::declval<_Ptr>(), std::declval<_Args>()...))>, _Alloc, _Ptr,
        _Args...> : std::false_type {};
    _STL_RESTORE_DEPRECATED_WARNING

    template <class _Alloc, class _Ptr, class... _Args>
    using uses_default_construct =
        std::disjunction<is_default_allocator<_Alloc>, has_no_allocator_construct<void, _Alloc, _Ptr, _Args...>>;

    template <class _Alloc, class _Ptr, class = void>
    struct has_no_allocator_destroy : true_type {};

    _STL_DISABLE_DEPRECATED_WARNING
    template <class _Alloc, class _Ptr>
    struct has_no_allocator_destroy<_Alloc, _Ptr, void_t<decltype(std::declval<_Alloc&>().destroy(std::declval<_Ptr>()))>>
        : std::false_type {};
    _STL_RESTORE_DEPRECATED_WARNING

    template <class _Alloc, class _Ptr>
    using uses_default_destroy = std::disjunction<is_default_allocator<_Alloc>, has_no_allocator_destroy<_Alloc, _Ptr>>;

    template <class _Alloc, class _Ptr>
    using uses_default_destroy_t = typename uses_default_destroy<_Alloc, _Ptr>::type;
}  // namespace xstl
#endif