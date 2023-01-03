#pragma once
#include "config.hpp"
#include <array>
#include <cassert>      // for assertion diagnostics
#include <cstddef>      // for size_t
#include <cstdint>      // for fixed-width integer types
#include <functional>   // for less and equal_to
#include <iterator>     // for reverse_iterator and iterator traits
#include <limits>       // for numeric_limits
#include <stdexcept>    // for length_error
#include <type_traits>  // for aligned_storage and all meta-functions

namespace xstl {
    namespace {
        template <size_t N>
        using _Smallest_size_t = std::conditional_t<
            (N < std::numeric_limits<uint8_t>::max()), uint8_t,
            std::conditional_t<
                (N < std::numeric_limits<uint16_t>::max()), uint16_t,
                std::conditional_t<(N < std::numeric_limits<uint32_t>::max()), uint32_t,
                                   std::conditional_t<(N < std::numeric_limits<uint64_t>::max()), uint64_t, size_t>>>>;

        template <std::ranges::random_access_range Rng, class Index>
        constexpr decltype(auto) index(Rng&& rng, Index&& i) noexcept {
            XSTL_EXPECT(static_cast<std::ptrdiff_t>(i) < (std::end(rng) - std::begin(rng)));
            return std::begin(std::forward<Rng>(rng))[std::forward<Index>(i)];
        }
        template <class _Traits>
        struct _Zero_sized {
            using size_type       = typename _Traits::size_type;
            using value_type      = typename _Traits::_Tp;
            using difference_type = std::ptrdiff_t;
            using pointer         = value_type*;
            using const_pointer   = value_type const*;

            static constexpr pointer   data() noexcept { return nullptr; }
            static constexpr size_type size() noexcept { return 0; }
            static constexpr size_type capacity() noexcept { return 0; }
            static constexpr bool      empty() noexcept { return true; }
            static constexpr bool      full() noexcept { return true; }

            template <class... _Args>
            static constexpr void emplace_back(_Args&&...) noexcept {
                XSTL_EXPECT(false && "tried to emplace_back on empty storage");
            }
            static constexpr void pop_back() noexcept { XSTL_EXPECT(("tried to pop_back on empty storage", false)); }

            static constexpr void unsafe_set_size(size_t new_size) noexcept {
                XSTL_EXPECT(new_size == 0
                            && "tried to change size of empty storage to "
                               "non-zero value");
            }

            template <class _Iter>
            static constexpr void unsafe_destroy(_Iter, _Iter) noexcept {}
            static constexpr void unsafe_destroy_all() noexcept {}

            template <class _Ty>
            constexpr _Zero_sized(std::initializer_list<_Ty> l) noexcept {
                XSTL_EXPECT(l.size() == 0
                            && "tried to construct empty from a "
                               "non-empty initializer list");
            }
        };

        /// Storage for _Trivial types.
        template <class _Tp, class _SizeType, _SizeType _Capacity, class = std::enable_if_t<std::is_trivial_v<_Tp>>>
        struct _Trivial {
            using size_type       = _SizeType;
            using value_type      = _Tp;
            using difference_type = ptrdiff_t;
            using pointer         = _Tp*;
            using const_pointer   = _Tp const*;

        private:
            using data_t = std::conditional_t<!std::is_const_v<_Tp>, std::array<_Tp, _Capacity>,
                                              const std::array<std::remove_const_t<_Tp>, _Capacity>>;
            alignas(alignof(_Tp)) data_t _data{};

            size_type _size = 0;

        public:
            constexpr const_pointer    data() const noexcept { return _data.data(); }
            constexpr pointer          data() noexcept { return _data.data(); }
            constexpr size_type        size() const noexcept { return _size; }
            static constexpr size_type capacity() noexcept { return _Capacity; }
            constexpr bool             empty() const noexcept { return size() == size_type{ 0 }; }
            constexpr bool             full() const noexcept { return size() == _Capacity; }

            template <class... Args>
            constexpr void emplace_back(Args&&... args) noexcept {
                static_cast(!full(), "tried to emplace_back on full storage!");
                index(_data, size()) = _Tp(forward<Args>(args)...);
                unsafe_set_size(size() + 1);
            }

            constexpr void pop_back() noexcept {
                static_cast(!empty(), "tried to pop_back from empty storage!");
                unsafe_set_size(size() - 1);
            }

            constexpr void unsafe_set_size(size_t new_size) noexcept {
                static_cast(new_size <= _Capacity, "new_size out-of-bounds [0, _Capacity]");
                _size = size_type(new_size);
            }

            template <class _Iter>
            constexpr void unsafe_destroy(_Iter, _Iter) noexcept {}

            static constexpr void unsafe_destroy_all() noexcept {}

        private:
            template <class _Ty>
            static constexpr std::array<std::remove_const_t<_Tp>, _Capacity>
            unsafe_recast_init_list(std::initializer_list<_Ty>& l) noexcept {
                XSTL_EXPECT(l.size() <= capacity()
                            && "trying to construct storage from an "
                               "initializer_list "
                               "whose size exceeds the storage capacity");
                std::array<std::remove_const_t<_Tp>, _Capacity> d_{};
                for (size_t i = 0, e = l.size(); i < e; ++i) {
                    index(d_, i) = index(l, i);
                }
                return d_;
            }

        public:
            template <class _Ty>
            constexpr trivial(std::initializer_list<_Ty> l) noexcept : _data(unsafe_recast_init_list(l)) {
                unsafe_set_size(static_cast<size_type>(l.size()));
            }
        };

        template <class _Tp, size_t _Capacity>
        struct _Non_trivial {
            static_assert(_Capacity != size_t{ 0 }, "Capacity must be greater than zero!");

            using size_type       = _Smallest_size_t<_Capacity>;
            using value_type      = _Tp;
            using difference_type = ptrdiff_t;
            using pointer         = _Tp*;
            using const_pointer   = _Tp const*;

            template <std::size_t Len, std::size_t Align>
            struct aligned_storage {
                struct type {
                    alignas(Align) std::byte data[Len];
                };
            };

        private:
            size_type _size = 0;

            using aligned_storage_t = aligned_storage<sizeof(std::remove_const_t<_Tp>), alignof(std::remove_const_t<_Tp>)>::type;
            using data_t            = std::conditional_t<!std::is_const<_Tp>, aligned_storage_t, const aligned_storage_t>;
            alignas(alignof(_Tp)) data_t _data[_Capacity]{};

        public:
            const_pointer              data() const noexcept { return reinterpret_cast<const_pointer>(_data); }
            pointer                    data() noexcept { return reinterpret_cast<pointer>(_data); }
            const_pointer              end() const noexcept { return data() + size(); }
            pointer                    end() noexcept { return data() + size(); }
            constexpr size_type        size() const noexcept { return _size; }
            static constexpr size_type capacity() noexcept { return _Capacity; }
            constexpr bool             empty() const noexcept { return size() == size_type{ 0 }; }
            constexpr bool             full() const noexcept { return size() == _Capacity; }

            template <class... Args>
            void emplace_back(Args&&... args) noexcept(noexcept(new(end()) _Tp(std::forward<Args>(args)...))) {
                XSTL_EXPECT(("tried to emplace_back on full storage", !full()));
                new (end()) _Tp(std::forward<Args>(args)...);
                unsafe_set_size(size() + 1);
            }

            void pop_back() noexcept(std::is_nothrow_destructible_v<_Tp>) {
                XSTL_EXPECT(("tried to pop_back from empty storage!", !empty()));
                std::destroy_at(std::launder(reinterpret_cast<_Tp*>(end() - 1)));
                unsafe_set_size(size() - 1);
            }

            constexpr void unsafe_set_size(size_t new_size) noexcept {
                XSTL_EXPECT(("new_size out-of-bounds [0)_Capacity)", new_size <= _Capacity));
                _size = size_type(new_size);
            }

            template <class _Iter>
            void unsafe_destroy(_Iter first, _Iter last) noexcept(std::is_nothrow_destructible_v<_Tp>) {
                XSTL_EXPECT(("first is out-of-bounds", first >= data() && first <= end()));
                XSTL_EXPECT(("last is out-of-bounds", last >= data() && last <= end()));
                for (; first != last; ++first) {
                    first->~_Tp();
                }
            }

            void unsafe_destroy_all() noexcept(std::is_nothrow_destructible_v<_Tp>) { unsafe_destroy(data(), end()); }

            constexpr _Non_trivial()                               = default;
            constexpr _Non_trivial(_Non_trivial const&)            = default;
            constexpr _Non_trivial& operator=(_Non_trivial const&) = default;
            constexpr _Non_trivial(_Non_trivial&&)                 = default;
            constexpr _Non_trivial& operator=(_Non_trivial&&)      = default;
            ~_Non_trivial() noexcept(std::is_nothrow_destructible_v<_Tp>) { unsafe_destroy_all(); }

            template <class _Ty, class = std::enable_if_t<std::is_convertible_v<_Ty, _Tp>>>
            constexpr _Non_trivial(std::
                                       : initializer_list<_Ty>
                                           l) noexcept(noexcept(emplace_back(index(l, 0)))) {
                XSTL_EXPECT(l.size() <= capacity()
                            && "trying to construct storage from an "
                               "initializer_list "
                               "whose size exceeds the storage capacity");
                for (size_t i = 0; i < l.size(); ++i) {
                    emplace_back(index(l, i));
                }
            }
        };

        /// Selects the vector storage.
        template <class _Tp, size_t _Capacity>
        using _Storage = std::conditional_t<
            _Capacity == 0, _Zero_sized<_Tp>,
            std::conditional_t<std::is_trivial_v<_Tp>, _Trivial<_Tp, _Capacity>, _Non_trivial<_Tp, _Capacity>>>;

        template <class _Tp, class... _Args>
        struct _Conditions : _Conditions<std::bool_constant<_Tp::value>, _Tp, _Args...> {};

        template <class _Ty>
        struct _Conditions<_Ty> {
            using type = _Ty;
        };

        template <class _Tp, class _Tp2, class... _Args>
        struct _Conditions<std::false_type, _Tp, _Tp2, _Args...> : _Conditions<std::bool_constant<_Tp2::value>, _Tp2, _Args...> {
        };

        template <class _Tp, class... _Args>
        struct _Conditions<std::true_type, _Tp, _Args...> {
            static_assert(!std::is_void_v<std::void_t<typename _Conditions<_Args...>::type>>, "duplicate arguments");
            using type = typename _Tp::type;
        };

        template <class _Tp, class _Ty>
        struct _Conditions<std::false_type, _Tp, _Ty> {
            using type = _Ty;
        };

        template <class... _Args>
        using _Conditions_t = typename _Conditions<_Args...>::type;

        template <size_t _Capacity, class... _Args>
        struct _Select_size_type {
            template <class>
            struct _Is_size_policy : std::false_type {};

            template <class _SizeType>
            struct _Is_size_policy<size_policy<_SizeType>> : std::bool_constant<std::is_integral_v<_SizeType>> {
                using type = _SizeType;
            };
            using type = _Conditions_t<_Is_size_policy<_Args>..., _Smallest_size_t<_Capacity>>;
        };

        template <class _Tp, class... _Args>
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

            using type = _Conditions_t<_Is_allocator<_Args>..., std::allocator<_Tp>>;
        };
    }  // namespace

    template <class _SizeType>
    struct size_policy {};

    template <class _Iter, size_t Capacity, class... Policies>
    struct small_vector : private _Storage<T, Capacity> {
    private:
        static_assert(is_nothrow_destructible_v<T>, "T must be nothrow destructible");
        using base_t = _Storage<T, Capacity>;
        using self   = small_vector<T, Capacity>;

        using base_t::unsafe_destroy;
        using base_t::unsafe_destroy_all;
        using base_t::unsafe_set_size;

    public:
        using value_type             = typename base_t::value_type;
        using difference_type        = ptrdiff_t;
        using reference              = value_type&;
        using const_reference        = value_type const&;
        using pointer                = typename base_t::pointer;
        using const_pointer          = typename base_t::const_pointer;
        using iterator               = typename base_t::pointer;
        using const_iterator         = typename base_t::const_pointer;
        using size_type              = size_t;
        using reverse_iterator       = ::std::reverse_iterator<iterator>;
        using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

        /// \name Size / capacity
        ///@{
        using base_t::empty;
        using base_t::full;

        /// Number of elements in the vector
        constexpr size_type size() const noexcept { return base_t::size(); }

        /// Maximum number of elements that can be allocated in the vector
        static constexpr size_type capacity() noexcept { return base_t::capacity(); }

        /// Maximum number of elements that can be allocated in the vector
        static constexpr size_type max_size() noexcept { return capacity(); }

        ///@} // Size / capacity

        /// \name Data access
        ///@{

        using base_t::data;

        ///@} // Data access

        /// \name Iterators
        ///@{

        constexpr iterator       begin() noexcept { return data(); }
        constexpr const_iterator begin() const noexcept { return data(); }
        constexpr iterator       end() noexcept { return data() + size(); }
        constexpr const_iterator end() const noexcept { return data() + size(); }

        reverse_iterator       rbegin() noexcept { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
        reverse_iterator       rend() noexcept { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

        constexpr const_iterator cbegin() noexcept { return begin(); }
        constexpr const_iterator cbegin() const noexcept { return begin(); }
        constexpr const_iterator cend() noexcept { return end(); }
        constexpr const_iterator cend() const noexcept { return end(); }

        ///@}  // Iterators

    private:
        /// \name Iterator bound-check utilites
        ///@{

        template <typename It>
        constexpr void assert_iterator_in_range(It it) noexcept {
            static_assert(Pointer<It>);
            XSTL_EXPECT(begin() <= it && "iterator not in range");
            XSTL_EXPECT(it <= end() && "iterator not in range");
        }

        template <typename It0, typename It1>
        constexpr void assert_valid_iterator_pair(It0 first, It1 last) noexcept {
            static_assert(Pointer<It0>);
            static_assert(Pointer<It1>);
            XSTL_EXPECT(first <= last && "invalid iterator pair");
        }

        template <typename It0, typename It1>
        constexpr void assert_iterator_pair_in_range(It0 first, It1 last) noexcept {
            assert_iterator_in_range(first);
            assert_iterator_in_range(last);
            assert_valid_iterator_pair(first, last);
        }

        ///@}
    public:
        /// \name Element access
        ///
        ///@{

        /// Unchecked access to element at index \p pos (UB if index not in
        /// range)
        constexpr reference operator[](size_type pos) noexcept { return index(*this, pos); }

        /// Unchecked access to element at index \p pos (UB if index not in
        /// range)
        constexpr const_reference operator[](size_type pos) const noexcept { return index(*this, pos); }

        /// Checked access to element at index \p pos (throws `out_of_range`
        /// if index not in range)
        constexpr reference at(size_type pos) {
            if (XSTL_UNLIKELY(pos >= size())) {
                throw out_of_range("small_vector::at");
            }
            return index(*this, pos);
        }

        /// Checked access to element at index \p pos (throws `out_of_range`
        /// if index not in range)
        constexpr const_reference at(size_type pos) const {
            if (XSTL_UNLIKELY(pos >= size())) {
                throw out_of_range("small_vector::at");
            }
            return index(*this, pos);
        }

        ///
        constexpr reference       front() noexcept { return index(*this, 0); }
        constexpr const_reference front() const noexcept { return index(*this, 0); }

        constexpr reference back() noexcept {
            XSTL_EXPECT(!empty() && "calling back on an empty vector");
            return index(*this, size() - 1);
        }
        constexpr const_reference back() const noexcept {
            XSTL_EXPECT(!empty() && "calling back on an empty vector");
            return index(*this, size() - 1);
        }

        ///@} // Element access

        /// \name Modifiers
        ///@{

        using base_t::emplace_back;
        using base_t::pop_back;

        /// Clears the vector.
        constexpr void clear() noexcept {
            unsafe_destroy_all();
            unsafe_set_size(0);
        }

        /// Appends \p value at the end of the vector.
        template <typename U, XSTL_REQUIRES_(Constructible<T, U>&& Assignable<reference, U&&>)>
        constexpr void push_back(U&& value) noexcept(noexcept(emplace_back(forward<U>(value)))) {
            XSTL_EXPECT(!full() && "vector is full!");
            emplace_back(forward<U>(value));
        }

        /// Appends a default constructed `T` at the end of the vector.
        XSTL_REQUIRES(Constructible<T, T>&& Assignable<reference, T&&>)
        void push_back() noexcept(noexcept(emplace_back(T{}))) {
            XSTL_EXPECT(!full() && "vector is full!");
            emplace_back(T{});
        }

        template <typename... Args, XSTL_REQUIRES_(Constructible<T, Args...>)>
        constexpr iterator
        emplace(const_iterator position,
                Args&&... args) noexcept(noexcept(move_insert(position, declval<value_type*>(), declval<value_type*>()))) {
            XSTL_EXPECT(!full() && "tried emplace on full small_vector!");
            assert_iterator_in_range(position);
            value_type a(forward<Args>(args)...);
            return move_insert(position, &a, &a + 1);
        }
        XSTL_REQUIRES(CopyConstructible<T>)
        constexpr iterator insert(const_iterator  position,
                                  const_reference x) noexcept(noexcept(insert(position, size_type(1), x))) {
            XSTL_EXPECT(!full() && "tried insert on full small_vector!");
            assert_iterator_in_range(position);
            return insert(position, size_type(1), x);
        }

        XSTL_REQUIRES(MoveConstructible<T>)
        constexpr iterator insert(const_iterator position, value_type&& x) noexcept(noexcept(move_insert(position, &x, &x + 1))) {
            XSTL_EXPECT(!full() && "tried insert on full small_vector!");
            assert_iterator_in_range(position);
            return move_insert(position, &x, &x + 1);
        }

        XSTL_REQUIRES(CopyConstructible<T>)
        constexpr iterator insert(const_iterator position, size_type n, const T& x) noexcept(noexcept(push_back(x))) {
            assert_iterator_in_range(position);
            const auto new_size = size() + n;
            XSTL_EXPECT(new_size <= capacity() && "trying to insert beyond capacity!");
            auto b = end();
            while (n != 0) {
                push_back(x);
                --n;
            }

            auto writable_position = begin() + (position - begin());
            slow_rotate(writable_position, b, end());
            return writable_position;
        }

        template <class InputIt,
                  XSTL_REQUIRES_(InputIterator<InputIt>and Constructible<value_type, iterator_reference_t<InputIt>>)>
        constexpr iterator insert(const_iterator position, InputIt first, InputIt last) noexcept(noexcept(emplace_back(*first))) {
            assert_iterator_in_range(position);
            assert_valid_iterator_pair(first, last);
            if constexpr (random_access_iterator_v<InputIt>) {
                XSTL_EXPECT(size() + static_cast<size_type>(last - first) <= capacity() && "trying to insert beyond capacity!");
            }
            auto b = end();

            // insert at the end and then just rotate:
            // cannot use try in constexpr function
            // try {  // if copy_constructor throws you get basic-guarantee?
            for (; first != last; ++first) {
                emplace_back(*first);
            }
            // } catch (...) {
            //   erase(b, end());
            //   throw;
            // }

            auto writable_position = begin() + (position - begin());
            slow_rotate(writable_position, b, end());
            return writable_position;
        }

        template <class InputIt, XSTL_REQUIRES_(InputIterator<InputIt>)>
        constexpr iterator move_insert(const_iterator position, InputIt first,
                                       InputIt last) noexcept(noexcept(emplace_back(move(*first)))) {
            assert_iterator_in_range(position);
            assert_valid_iterator_pair(first, last);
            if constexpr (random_access_iterator_v<InputIt>) {
                XSTL_EXPECT(size() + static_cast<size_type>(last - first) <= capacity() && "trying to insert beyond capacity!");
            }
            iterator b = end();

            // we insert at the end and then just rotate:
            for (; first != last; ++first) {
                emplace_back(move(*first));
            }
            auto writable_position = begin() + (position - begin());
            slow_rotate<iterator>(writable_position, b, end());
            return writable_position;
        }

        XSTL_REQUIRES(CopyConstructible<T>)
        constexpr iterator insert(const_iterator      position,
                                  initializer_list<T> il) noexcept(noexcept(insert(position, il.begin(), il.end()))) {
            assert_iterator_in_range(position);
            return insert(position, il.begin(), il.end());
        }

        XSTL_REQUIRES(Movable<value_type>)
        constexpr iterator erase(const_iterator position) noexcept {
            assert_iterator_in_range(position);
            return erase(position, position + 1);
        }

        XSTL_REQUIRES(Movable<value_type>)
        constexpr iterator erase(const_iterator first, const_iterator last) noexcept {
            assert_iterator_pair_in_range(first, last);
            iterator p = begin() + (first - begin());
            if (first != last) {
                unsafe_destroy(move(p + (last - first), end(), p), end());
                unsafe_set_size(size() - static_cast<size_type>(last - first));
            }

            return p;
        }

        XSTL_REQUIRES(Assignable<T&, T&&>)
        constexpr void swap(small_vector& other) noexcept(is_nothrow_swappable_v<T>) {
            small_vector tmp = move(other);
            other            = move(*this);
            (*this)          = move(tmp);
        }

        /// Resizes the container to contain \p sz elements. If elements
        /// need to be appended, these are copy-constructed from \p value.
        ///
        XSTL_REQUIRES(CopyConstructible<T>)
        constexpr void resize(size_type sz, T const& value) noexcept(is_nothrow_copy_constructible_v<T>) {
            if (sz == size()) {
                return;
            }
            if (sz > size()) {
                XSTL_EXPECT(sz <= capacity()
                            && "small_vector cannot be resized to "
                               "a size greater than capacity");
                insert(end(), sz - size(), value);
            }
            else {
                erase(end() - (size() - sz), end());
            }
        }

    private:
        XSTL_REQUIRES(MoveConstructible<T> or CopyConstructible<T>)
        constexpr void emplace_n(size_type n) noexcept((MoveConstructible<T> && is_nothrow_move_constructible_v<T>)
                                                       || (CopyConstructible<T> && is_nothrow_copy_constructible_v<T>)) {
            XSTL_EXPECT(n <= capacity()
                        && "small_vector cannot be "
                           "resized to a size greater than "
                           "capacity");
            while (n != size()) {
                emplace_back(T{});
            }
        }

    public:
        /// Resizes the container to contain \p sz elements. If elements
        /// need to be appended, these are move-constructed from `T{}` (or
        /// copy-constructed if `T` is not `MoveConstructible`).
        XSTL_REQUIRES(Movable<value_type>)
        constexpr void resize(size_type sz) noexcept((MoveConstructible<T> && is_nothrow_move_constructible_v<T>)
                                                     || (CopyConstructible<T> && is_nothrow_copy_constructible_v<T>)) {
            if (sz == size()) {
                return;
            }

            if (sz > size()) {
                emplace_n(sz);
            }
            else {
                erase(end() - (size() - sz), end());
            }
        }

        ///@}  // Modifiers

        /// \name Construct/copy/move/destroy
        ///@{

        /// Default constructor.
        constexpr small_vector() = default;

        /// Copy constructor.
        XSTL_REQUIRES(CopyConstructible<value_type>)
        constexpr small_vector(small_vector const& other) noexcept(noexcept(insert(begin(), other.begin(), other.end()))) {
            // nothin to assert: size of other cannot exceed capacity
            // because both vectors have the same type
            insert(begin(), other.begin(), other.end());
        }

        /// Move constructor.
        XSTL_REQUIRES(MoveConstructible<value_type>)
        constexpr small_vector(small_vector&& other) noexcept(noexcept(move_insert(begin(), other.begin(), other.end()))) {
            // nothin to assert: size of other cannot exceed capacity
            // because both vectors have the same type
            move_insert(begin(), other.begin(), other.end());
        }

        /// Copy assignment.
        XSTL_REQUIRES(Assignable<reference, const_reference>)
        constexpr small_vector& operator=(small_vector const& other) noexcept(
            noexcept(clear()) && noexcept(insert(begin(), other.begin(), other.end()))) {
            // nothin to assert: size of other cannot exceed capacity
            // because both vectors have the same type
            clear();
            insert(this->begin(), other.begin(), other.end());
            return *this;
        }

        /// Move assignment.
        XSTL_REQUIRES(Assignable<reference, reference>)
        constexpr small_vector& operator=(small_vector&& other) noexcept(
            noexcept(clear()) and noexcept(move_insert(begin(), other.begin(), other.end()))) {
            // nothin to assert: size of other cannot exceed capacity
            // because both vectors have the same type
            clear();
            move_insert(this->begin(), other.begin(), other.end());
            return *this;
        }

        /// Initializes vector with \p n default-constructed elements.
        XSTL_REQUIRES(CopyConstructible<T> or MoveConstructible<T>)
        explicit constexpr small_vector(size_type n) noexcept(noexcept(emplace_n(n))) {
            XSTL_EXPECT(n <= capacity() && "size exceeds capacity");
            emplace_n(n);
        }

        /// Initializes vector with \p n with \p value.
        XSTL_REQUIRES(CopyConstructible<T>)
        constexpr small_vector(size_type n, T const& value) noexcept(noexcept(insert(begin(), n, value))) {
            XSTL_EXPECT(n <= capacity() && "size exceeds capacity");
            insert(begin(), n, value);
        }

        /// Initialize vector from range [first, last).
        template <class InputIt, XSTL_REQUIRES_(InputIterator<InputIt>)>
        constexpr small_vector(InputIt first, InputIt last) {
            if constexpr (random_access_iterator_v<InputIt>) {
                XSTL_EXPECT(last - first >= 0);
                XSTL_EXPECT(static_cast<size_type>(last - first) <= capacity() && "range size exceeds capacity");
            }
            insert(begin(), first, last);
        }

        template <typename U, XSTL_REQUIRES_(Convertible<U, value_type>)>
        constexpr small_vector(initializer_list<U> il) noexcept(noexcept(base_t(move(il))))
            : base_t(move(il)) {  // assert happens in base_t constructor
        }

        template <class InputIt, XSTL_REQUIRES_(InputIterator<InputIt>)>
        constexpr void assign(InputIt first,
                              InputIt last) noexcept(noexcept(clear()) and noexcept(insert(begin(), first, last))) {
            if constexpr (random_access_iterator_v<InputIt>) {
                XSTL_EXPECT(last - first >= 0);
                XSTL_EXPECT(static_cast<size_type>(last - first) <= capacity() && "range size exceeds capacity");
            }
            clear();
            insert(begin(), first, last);
        }

        XSTL_REQUIRES(CopyConstructible<T>)
        constexpr void assign(size_type n, const T& u) {
            XSTL_EXPECT(n <= capacity() && "size exceeds capacity");
            clear();
            insert(begin(), n, u);
        }
        XSTL_REQUIRES(CopyConstructible<T>)
        constexpr void assign(initializer_list<T> const& il) {
            XSTL_EXPECT(il.size() <= capacity() && "initializer_list size exceeds capacity");
            clear();
            insert(this->begin(), il.begin(), il.end());
        }
        XSTL_REQUIRES(CopyConstructible<T>)
        constexpr void assign(initializer_list<T>&& il) {
            XSTL_EXPECT(il.size() <= capacity() && "initializer_list size exceeds capacity");
            clear();
            insert(this->begin(), il.begin(), il.end());
        }

        ///@}  // Construct/copy/move/destroy/assign
    };

    template <class _Iter, size_t Capacity>
    constexpr bool operator==(small_vector<T, Capacity> const& a, small_vector<T, Capacity> const& b) noexcept {
        return a.size() == b.size() and cmp(a.begin(), a.end(), b.begin(), b.end(), equal_to<>{});
    }

    template <class _Iter, size_t Capacity>
    constexpr bool operator<(small_vector<T, Capacity> const& a, small_vector<T, Capacity> const& b) noexcept {
        return cmp(a.begin(), a.end(), b.begin(), b.end(), less<>{});
    }

    template <class _Iter, size_t Capacity>
    constexpr bool operator!=(small_vector<T, Capacity> const& a, small_vector<T, Capacity> const& b) noexcept {
        return not(a == b);
    }

    template <class _Iter, size_t Capacity>
    constexpr bool operator<=(small_vector<T, Capacity> const& a, small_vector<T, Capacity> const& b) noexcept {
        return cmp(a.begin(), a.end(), b.begin(), b.end(), less_equal<>{});
    }

    template <class _Iter, size_t Capacity>
    constexpr bool operator>(small_vector<T, Capacity> const& a, small_vector<T, Capacity> const& b) noexcept {
        return cmp(a.begin(), a.end(), b.begin(), b.end(), greater<>{});
    }

    template <class _Iter, size_t Capacity>
    constexpr bool operator>=(small_vector<T, Capacity> const& a, small_vector<T, Capacity> const& b) noexcept {
        return cmp(a.begin(), a.end(), b.begin(), b.end(), greater_equal<>{});
    }

}  // namespace xstl