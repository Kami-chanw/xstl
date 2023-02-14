#ifndef _BITSTRING_HPP_
#define _BITSTRING_HPP_
#include "utility.hpp"
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#define TYPE_BIT(TYPE) (sizeof(TYPE) << 3)

namespace xstl {
    using default_block_type = unsigned char;

    // operate [first, last)
    template <class _Tp>
    inline constexpr _Tp get_bits(const _Tp& value, size_t first, size_t last) {
        return ((_Tp{ 1 } << last - first) - _Tp{ 1 }) << (TYPE_BIT(_Tp) - last) & value;
    }

    template <class _Tp>
    inline constexpr void clear_bits(_Tp& value, size_t first, size_t last) {
        value &= ~(((_Tp{ 1 } << last - first) - _Tp{ 1 }) << TYPE_BIT(_Tp) - last);
    }

    template <class _Tp>
    inline constexpr void flip_bits(_Tp& value, size_t first, size_t last) {
        value ^= ((_Tp{ 1 } << last - first) - _Tp{ 1 }) << TYPE_BIT(_Tp) - last;
    }

    template <class _Tp, class _Ty>
    inline constexpr void set_bits(_Tp& dst, size_t beg, const _Ty& src, size_t first, size_t last) {
        clear_bits(dst, beg, beg + last - first);
        std::make_unsigned_t<_Ty> _tarbits = get_bits(src, first, last);
        if constexpr (sizeof(_Ty) > sizeof(_Tp))
            dst |= static_cast<_Tp>((beg < first ? _tarbits << first - beg : _tarbits >> beg - first) >> TYPE_BIT(_Ty) - TYPE_BIT(_Tp));
        else
            dst |= static_cast<_Tp>(beg < first ? _tarbits << first - beg : _tarbits >> beg - first);
    }

    // operate index
    template <class _Tp>
    inline constexpr bool get_bit(const _Tp& value, size_t index) {
        return ((_Tp{ 1 } << TYPE_BIT(_Tp) - index - 1) & value) != 0;
    }

    template <class _Tp>
    inline constexpr void clear_bit(_Tp& value, size_t index) {
        value &= ~(_Tp{ 1 } << TYPE_BIT(_Tp) - index - 1);
    }

    template <class _Tp>
    inline constexpr void flip_bit(_Tp& value, uint32_t index) {
        value ^= _Tp{ 1 } << TYPE_BIT(_Tp) - index - 1;
    }

    template <class _Tp>
    inline constexpr void set_bit(_Tp& value, uint32_t index, bool val = true) {
        if (get_bit(value, index) ^ val)
            flip_bit(value, index);
    }



    template <class _Block = typename _Bstr_accessor::byte, class _Index>
    inline constexpr _Index to_block_idx(_Index index) {
        return index / static_cast<_Index>(TYPE_BIT(_Block));
    }

    template <class _Block = typename _Bstr_accessor::byte, class _Index>
    inline constexpr _Index to_bit_idx(_Index index) {
        if constexpr (std::is_unsigned_v<_Index>)
            return index & TYPE_BIT(_Block) - 1;
        else
            return index >= _Index{ 0 } ? (index & TYPE_BIT(_Block) - 1) : -static_cast<_Index>(-index % TYPE_BIT(_Block));
    }

    template <class _Block = _Bstr_accessor::byte, class _Index>
    inline constexpr _Index to_fit_size(_Index index) {
        return to_block_idx<_Block>(index) + static_cast<bool>(to_bit_idx<_Block>(index));
    }

    template <std::unsigned_integral, class, class>
    class bitstring_base;
    template <std::unsigned_integral, class>
    class basic_bitstring;
    template <std::unsigned_integral, class>
    class basic_bitstring_ref;

    struct _Bstr_accessor {
        using byte = std::uint8_t;

        template <std::unsigned_integral _Block, class _Alloc, class _Derived>
        inline static auto /* size_type */ get_begin(const bitstring_base<_Block, _Alloc, _Derived>& bstr) noexcept {
            return static_cast<const _Derived&>(bstr)._Get_begin();
        }

        template <std::unsigned_integral _Block, class _Alloc, class _Derived>
        inline static auto& /* buf_type& */ get_vec(bitstring_base<_Block, _Alloc, _Derived>& bstr) noexcept {
            return static_cast<_Derived&>(bstr)._Get_vec();
        }

        template <std::unsigned_integral _Block, class _Alloc, class _Derived>
        inline static const auto& /* const buf_type& */ get_vec(const bitstring_base<_Block, _Alloc, _Derived>& bstr) noexcept {
            return static_cast<const _Derived&>(bstr)._Get_vec();
        }

        template <std::unsigned_integral _Block, class _Alloc, class _Derived>
        inline static auto /* block_type */ get_block(const bitstring_base<_Block, _Alloc, _Derived>& bstr, const typename bitstring_base<_Block, _Alloc, _Derived>::size_type index,
                                                      const typename bitstring_base<_Block, _Alloc, _Derived>::size_type maxsz) noexcept {
            return static_cast<const _Derived&>(bstr).get_block(index, maxsz);
        }

        template <std::unsigned_integral _Block, class _Alloc, class _Derived>
        inline static auto /*reference*/ make_reference(bitstring_base<_Block, _Alloc, _Derived>& bstr, const typename bitstring_base<_Block, _Alloc, _Derived>::size_type index) noexcept {
            return typename bitstring_base<_Block, _Alloc, _Derived>::reference(bstr, index);
        }

        template <std::unsigned_integral _Block, class _Alloc, class _Derived>
        inline static auto /*const_reference*/ make_const_reference(const bitstring_base<_Block, _Alloc, _Derived>&                    bstr,
                                                                    const typename bitstring_base<_Block, _Alloc, _Derived>::size_type index) noexcept {
            return typename bitstring_base<_Block, _Alloc, _Derived>::const_reference(bstr, index);
        }
    };

    template <class _Bstr>
    struct _Bstr_pack {
        using container_type      = _Bstr;
        using value_type          = bool;
        using pointer             = typename _Bstr::pointer;
        using reference           = typename _Bstr::reference;
        using const_pointer       = typename _Bstr::const_pointer;
        using const_reference     = typename _Bstr::const_reference;
        using mem_access_type     = reference*;
        using difference_type     = typename _Bstr::difference_type;
        using variant_convertible = int;

        template <class _Ref>
        constexpr static bool dereferable(const _Bstr* cont, const _Ref& ref) {
            return range_verify(cont, ref, 0);
        }

        template <class _Ref>
        constexpr static bool range_verify(const _Bstr* cont, const _Ref& ref, const difference_type off) {
            return ref._Get_index() + off <= _Bstr_accessor::get_begin(*cont) + cont->size() && _Bstr_accessor::get_begin(*cont) <= ref._Get_index() + off;
        }

        template <class _Ref>
        constexpr static _Ref& extract(_Ref& ref) noexcept {
            return ref;
        }

        template <class _Ref>
        static void fwd(_Ref& ref, difference_type off) noexcept {
            ref._Get_index() += off;
        }
    };

    struct range_index {
        range_index(size_t first, size_t last = static_cast<size_t>(-1)) : _first(first), _last(last) {}
        range_index operator,(range_index idx) {
#if !defined(_NO_BSTRING_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(idx._last == static_cast<size_t>(-1) && _first <= idx._first);
#endif
            return { _first, idx._first };
        }

        friend range_index operator,(unsigned long long idx, range_index rng_idx) {
#if !defined(_NO_BSTRING_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(rng_idx._last == static_cast<size_t>(-1) && idx <= rng_idx._first);
#endif
            return { static_cast<size_t>(idx), rng_idx._first };
        }

        size_t _first;
        size_t _last;
    };

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    class bitstring_base {
        using _Self = bitstring_base<_Block, _Alloc, _Derived>;

    public:
        friend struct _Bstr_accessor;
        using byte         = _Bstr_accessor::byte;
        using _Altp_traits = std::allocator_traits<_Alloc>;
        using _Bstr        = basic_bitstring<_Block, _Alloc>;
        using _Bstr_ref    = basic_bitstring_ref<_Block, _Alloc>;
        using block_type   = _Block;
        static_assert(std::is_same_v<typename _Altp_traits::value_type, block_type>, "bitstring_base<Block, Alloc> requires that Alloc's value_type match Block");
        using allocator_type  = _Alloc;
        using size_type       = typename _Altp_traits::size_type;
        using difference_type = typename _Altp_traits::difference_type;
        using value_type      = bool;
        class reference;
        class const_reference {
            friend struct _Bstr_accessor;

        public:
            XSTL_NODISCARD bool operator==(const const_reference& rhs) const noexcept { return std::addressof(_bstr) == std::addressof(rhs._bstr) && _index == rhs._index; }  // for iterators
            XSTL_NODISCARD bool operator!=(const const_reference& rhs) const noexcept { return !(*this == rhs); }

            operator bool() const { return unchecked_get(_bstr, _index); }

            bool operator~() const { return static_cast<bool>(*this) == 0; }

            difference_type operator-(const const_reference& cref) const { return _index - cref._index; }

            explicit   operator reference() { return _Bstr_accessor::make_reference(_bstr, _index); }
            _Self&     _Get_bstr() const noexcept { return const_cast<_Self&>(_bstr); }
            size_type& _Get_index() const noexcept { return _index; }

        protected:
            const_reference(const _Self& bstr, const size_type index) : _bstr(const_cast<_Self&>(bstr)), _index(index) {}

            _Self&            _bstr;
            mutable size_type _index;
        };

        class reference : public const_reference {
            friend struct _Bstr_accessor;
            using _Base = const_reference;
            using _Base::_bstr;
            using _Base::_index;

            reference(_Self& bstr, const size_type index) : _Base(bstr, index) {}

        public:
            reference& flip() { return *this = ~*this; }

            void swap(reference& ref) {
                if (*this ^ ref)
                    flip(), ref.flip();
            }

            friend void swap(reference& x, reference& y) { x.swap(y); }

            reference& operator=(bool x) {
                set_bit(_Bstr_accessor::get_vec(_bstr)[to_block_idx<block_type>(_index)], to_bit_idx<block_type>(_index), x);
                return *this;
            }
            reference& operator=(const reference& rhs) {
                set_bit(_Bstr_accessor::get_vec(_bstr)[to_block_idx<block_type>(_index)], to_bit_idx<block_type>(_index), static_cast<bool>(rhs));
                return *this;
            }

            reference& operator|=(bool x) {
                if (x && (_Bstr_accessor::get_vec(_bstr)[to_block_idx<block_type>(_index)] |= (block_type{ 1 } << TYPE_BIT(block_type) - to_bit_idx<block_type>(_index) - 1)))
                    ;
                return *this;
            }
            reference& operator&=(bool x) {
                if (!x)
                    clear_bit(_Bstr_accessor::get_vec(_bstr)[to_block_idx<block_type>(_index)], to_bit_idx<block_type>(_index));
                return *this;
            }
            reference& operator^=(bool x) {
                if (x && (_Bstr_accessor::get_vec(_bstr)[to_block_idx<block_type>(_index)] ^= (block_type{ 1 } << TYPE_BIT(block_type) - to_bit_idx<block_type>(_index) - 1)))
                    ;
                return *this;
            }
            reference& operator-=(bool x) {
                if (x)
                    clear_bit(_Bstr_accessor::get_vec(_bstr)[to_block_idx<block_type>(_index)], to_bit_idx<block_type>(_index));
                return *this;
            }
        };

        using iterator               = iter_adapter::rand_iter<reference, _Bstr_pack<_Derived>>;
        using xstl_const_iterator         = iter_adapter::rand_citer<const_reference, _Bstr_pack<_Derived>>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<xstl_const_iterator>;
        using pointer                = iterator;
        using const_pointer          = xstl_const_iterator;
        using buf_type               = std::vector<block_type, _Alloc>;
        static_assert(std::contiguous_iterator<typename buf_type::iterator>, "bitstring requires contiguous memory containers");

        XSTL_NODISCARD constexpr iterator               begin() noexcept { return iterator(_Bstr_accessor::make_reference(*this, abspos(0)), _Derptr()); }
        XSTL_NODISCARD constexpr iterator               end() noexcept { return iterator(_Bstr_accessor::make_reference(*this, abspos(_size)), _Derptr()); }
        XSTL_NODISCARD constexpr xstl_const_iterator         begin() const noexcept { return xstl_const_iterator(_Bstr_accessor::make_const_reference(*this, abspos(0)), _Derptr()); }
        XSTL_NODISCARD constexpr xstl_const_iterator         end() const noexcept { return xstl_const_iterator(_Bstr_accessor::make_const_reference(*this, abspos(_size)), _Derptr()); }
        XSTL_NODISCARD constexpr xstl_const_iterator         cbegin() const noexcept { return xstl_const_iterator(_Bstr_accessor::make_const_reference(*this, abspos(0)), _Derptr()); }
        XSTL_NODISCARD constexpr xstl_const_iterator         cend() const noexcept { return xstl_const_iterator(_Bstr_accessor::make_const_reference(*this, abspos(_size)), _Derptr()); }
        XSTL_NODISCARD constexpr reverse_iterator       rbegin() noexcept { return reverse_iterator(end()); }
        XSTL_NODISCARD constexpr reverse_iterator       rend() noexcept { return reverse_iterator(begin()); }
        XSTL_NODISCARD constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(cend()); }
        XSTL_NODISCARD constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(cbegin()); }
        XSTL_NODISCARD constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
        XSTL_NODISCARD constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    protected:
        bitstring_base() = default;
        bitstring_base(const size_type sz) : _size(sz) {}

    public:
        allocator_type& get_allocator() const noexcept { return _Bstr_accessor::get_vec(*this).get_allocator(); }

        reference       front() { return (*this)[0]; }
        const_reference front() const { return (*this)[0]; }
        reference       back() { return (*this)[_size - 1]; }
        const_reference back() const { return (*this)[_size - 1]; }

        _Bstr substr(size_type index = 0, size_type n = npos) const {
            check_npos(*this, index, n);
            return _Bstr((*this)[range_index(index, index + n)]);
        }

        _Self& set(size_type index, size_type n, bool value /* = true */);
        _Self& set(size_type index, bool value = true);
        _Self& set();
        _Self& reset(size_type index, size_type n) { return set(index, n, false); }
        _Self& reset(size_type index) { return set(index, false); }
        _Self& reset() { return range_operation(abspos(0), _size, reset_partial, reset_full); }
        _Self& flip(size_type index, size_type n);
        _Self& flip(size_type index);
        _Self& flip();

        _Self& left_shift(size_type n);
        _Self& right_shift(size_type n);
        template <class _Other>
        _Self& or_with(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index = 0);
        template <class _Other>
        _Self& xor_with(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index = 0);
        template <class _Other>
        _Self& and_with(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index = 0);
        template <class _Other>
        _Self& subtract_with(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index = 0);

        XSTL_NODISCARD bool      test(size_type index) const;
        XSTL_NODISCARD bool      test_set(size_type index, bool value = true);
        XSTL_NODISCARD bool      all() const;
        XSTL_NODISCARD bool      any() const;
        XSTL_NODISCARD bool      none() const { return !all(); }
        XSTL_NODISCARD size_type count() const noexcept;

        XSTL_NODISCARD size_type size() const noexcept { return _size; }
        XSTL_NODISCARD bool      empty() const noexcept { return _size == 0; }

        XSTL_NODISCARD reference       at(size_type index);
        XSTL_NODISCARD reference       operator[](size_type index);
        XSTL_NODISCARD const_reference at(size_type index) const;
        XSTL_NODISCARD const_reference operator[](size_type index) const;
        XSTL_NODISCARD _Bstr_ref       at(size_t first, size_t last) { return const_cast<_Self*>(this)->_Derptr()->at(range_index(first, last)); }
        XSTL_NODISCARD const _Bstr_ref at(size_t first, size_t last) const { return const_cast<_Self*>(this)->at(first, last); }
        XSTL_NODISCARD const _Bstr_ref at(range_index index) const { return const_cast<_Self*>(this)->_Derptr()->at(index); }
        XSTL_NODISCARD const _Bstr_ref operator[](range_index index) const { return (*const_cast<_Self*>(this)->_Derptr())[index]; }

        template <class _String = std::string>
        XSTL_NODISCARD _String to_string(typename _String::value_type elem0 = static_cast<typename _String::value_type>('0'),
                                        typename _String::value_type elem1 = static_cast<typename _String::value_type>('1')) const;
        template <class _String = std::string>
        void to_string(_String& str, typename _String::value_type elem0 = static_cast<typename _String::value_type>('0'),
                       typename _String::value_type elem1 = static_cast<typename _String::value_type>('1')) const;
        template <class _Tp>
        XSTL_NODISCARD _Tp to_type() const;
        template <class _Tp>
        void to_type(_Tp& value) const;
        template <class _String = std::string>
        XSTL_NODISCARD _String to_string(size_type index, size_type n = npos, typename _String::value_type elem0 = static_cast<typename _String::value_type>('0'),
                                        typename _String::value_type elem1 = static_cast<typename _String::value_type>('1')) const;
        template <class _String = std::string>
        void to_string(_String& str, size_type index, size_type n = npos, typename _String::value_type elem0 = static_cast<typename _String::value_type>('0'),
                       typename _String::value_type elem1 = static_cast<typename _String::value_type>('1')) const;
        template <class _Tp>
        XSTL_NODISCARD _Tp to_type(size_type index, size_type n = npos) const;
        template <class _Tp>
        void to_type(_Tp& value, size_type index, size_type n = npos) const;

        template <class _Tp, class _Ext, class _Fn>
        size_type _Find(const _Tp& str, size_type count, _Ext getter, _Fn is_0, _Fn is_1) {}
        template <class _Other>
        size_type find(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index = 0) const noexcept(is_nothrow_comparable_v<std::equal_to<>, block_type>) {
            return 0; /*_Find(bstr, bstr.size(),
                 [&](size_type index) { return str[index]; },
                 [](const _Elem elem) { return elem == elem0; },
                 [](const _Elem elem) { return elem == elem1; });*/
        }
        template <character_type _Elem>
        size_type find(const _Elem* str, size_type index = 0, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) const noexcept {
            return _Find(
                str, std::char_traits<_Elem>::length(str), [&](size_type index) { return str[index]; }, [](const _Elem elem) { return elem == elem0; }, [](const _Elem elem) { return elem == elem1; });
        }

        void assign(size_type n, bool value) {
            _Derptr()->resize(n);
            unchecked_set(0, n, value);
        }
        template <class _Iter>
        void assign(_Iter binary_first, _Iter binary_last) {
            size_type _index = set_from_range(0, binary_first, binary_last, [&](size_type i) { return i < _size; });
            if (_index == _size)
                _Derptr()->insert(cend(), binary_first, binary_last);
            else
                _Derptr()->erase(_index, _size - _index);
        }
        template <character_type _Elem>
        void assign(const _Elem* str, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) {
            assign(str, std::char_traits<_Elem>::length(str), elem0, elem1);
        }
        template <character_type _Elem>
        void assign(const _Elem* str, size_type str_count, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) {
            size_type i = set_from_c_str(0, str, str_count, elem0, elem1, [&](size_type i) { return i < _size && !std::char_traits<_Elem>::eq(str[i], static_cast<_Elem>('\0')); });
            if (i == _size)
                _Derptr()->insert(i, str + i, str_count - i);
            else
                _Derptr()->erase(i, _size - i);
        }

        template <class _Tp>
        void assign_type(const _Tp& value, size_type n) {
            _Derptr()->resize(n);
            set_from_type(0, value, n);
        }

        // bitset operations
        XSTL_NODISCARD _Bstr operator~() const { return _Bstr(*this).flip(); }
        template <class _Other>
        XSTL_NODISCARD _Bstr operator&(const bitstring_base<_Block, _Alloc, _Other>& rhs) const {
            return _Bstr(*this) &= rhs;
        }
        template <class _Other>
        XSTL_NODISCARD _Bstr operator^(const bitstring_base<_Block, _Alloc, _Other>& rhs) const {
            return _Bstr(*this) ^= rhs;
        }
        template <class _Other>
        XSTL_NODISCARD _Bstr operator|(const bitstring_base<_Block, _Alloc, _Other>& rhs) const {
            return _Bstr(*this) |= rhs;
        }
        template <class _Other>
        XSTL_NODISCARD _Bstr operator-(const bitstring_base<_Block, _Alloc, _Other>& rhs) const {
            return _Bstr(*this) -= rhs;
        }
        XSTL_NODISCARD _Bstr operator<<(size_type n) const { return _Bstr(*this) <<= n; }
        XSTL_NODISCARD _Bstr operator>>(size_type n) const { return _Bstr(*this) >>= n; }

        template <class _Other>
        _Self& operator&=(const bitstring_base<_Block, _Alloc, _Other>& rhs) {
            return and_with(rhs);
        }
        template <class _Other>
        _Self& operator^=(const bitstring_base<_Block, _Alloc, _Other>& rhs) {
            return or_with(rhs);
        }
        template <class _Other>
        _Self& operator|=(const bitstring_base<_Block, _Alloc, _Other>& rhs) {
            return xor_with(rhs);
        }
        template <class _Other>
        _Self& operator-=(const bitstring_base<_Block, _Alloc, _Other>& rhs) {
            return subtract_with(rhs);
        }
        _Self& operator<<=(size_type n) { return left_shift(n); }
        _Self& operator>>=(size_type n) { return right_shift(n); }

        // string operations
        template <class _Other>
        XSTL_NODISCARD _Bstr operator+(const bitstring_base<_Block, _Alloc, _Other>& rhs) const {
            return _Bstr(*this) += rhs;
        }
        template <character_type _Elem>
        XSTL_NODISCARD _Bstr operator+(const _Elem* str) const {
            return _Bstr(*this) += str;
        }

        XSTL_NODISCARD bool                 operator==(const _Self& rhs) const;
        XSTL_NODISCARD std::strong_ordering operator<=>(const _Self& rhs) const;

        template <class _Elem, class _Traits>
        friend std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& os, const _Self& bstr) {
            using _Ostream = std::basic_ostream<_Elem, _Traits>;
            const typename _Ostream::sentry _ok(os);
            if (!_ok)
                os.setstate(_Ostream::badbit);
            else {
                size_type _padding = os.width() > 0 || static_cast<size_type>(os.width()) <= bstr.size() ? 0 : static_cast<size_type>(os.width()) - bstr.size();
                try {
                    if ((os.flags() & _Ostream::adjustfield) != _Ostream::left) {
                        for (; _padding > 0; --_padding)
                            if (_Traits::eq_int_type(_Traits::eof(), os.rdbuf()->sputc(os.fill())))
                                return os.setstate(_Ostream::badbit), os;
                    }
                    for (size_type i = 0; i < bstr.size(); ++i)
                        if (_Traits::eq_int_type(_Traits::eof(), os.rdbuf()->sputc(_Self::unchecked_get(bstr, i) ? static_cast<_Elem>('1') : static_cast<_Elem>('0'))))
                            return os.setstate(_Ostream::badbit), os;
                    for (; _padding > 0; --_padding)
                        if (_Traits::eq_int_type(_Traits::eof(), os.rdbuf()->sputc(os.fill())))
                            return os.setstate(_Ostream::badbit), os;
                    os.width(0);
                } catch (...) {
                    os.setstate(_Ostream::badbit);
                    throw;
                }
            }
            return os;
        }

        ~bitstring_base() noexcept {}

        static constexpr auto npos{ static_cast<size_type>(-1) };

    protected:
        void uninit_left_shift(size_type index, size_type n, size_type maxsz);   // assume n < maxsz
        void uninit_right_shift(size_type index, size_type n, size_type maxsz);  // assume n < maxsz

        inline static void set_partial(block_type& value, size_type first, size_type last) noexcept { set_bits(value, first, (std::numeric_limits<block_type>::max)(), 0, last - first); }
        inline static void set_full(block_type& value) noexcept { value = (std::numeric_limits<block_type>::max)(); }
        inline static void reset_partial(block_type& value, size_type first, size_type last) noexcept { clear_bits(value, first, last); }
        inline static void reset_full(block_type& value) noexcept { value = block_type{ 0 }; }
        inline static void flip_full(block_type& value) noexcept { value = ~value; }
        inline size_type   abspos(size_type index) const noexcept { return _Bstr_accessor::get_begin(*this) + index; }

        inline _Self& unchecked_set(size_type index, size_type n, bool value) noexcept {
            return value ? range_operation(abspos(index), n, set_partial, set_full) : range_operation(abspos(index), n, reset_partial, reset_full);
        }

        inline _Self& unchecked_set(size_type index, bool value) noexcept {
            set_bit(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(abspos(index))], to_bit_idx<block_type>(abspos(index)), value);
            return *this;
        }

        template <class _Other>
        inline static bool unchecked_get(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index) noexcept {
            return get_bit(_Bstr_accessor::get_vec(bstr)[to_block_idx<block_type>(index)], to_bit_idx<block_type>(index));
        }

        template <class _PartOp, class _FullOp>
        inline _Self& range_operation(size_type index, size_type n, _PartOp partial_operation, _FullOp full_operation) noexcept {
            const size_type _bit_idx = to_bit_idx<block_type>(index);
            block_type*     _node    = std::addressof(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(index)]);
            if (_bit_idx) {
                if (_bit_idx + n <= TYPE_BIT(block_type)) {
                    partial_operation(*_node, _bit_idx, _bit_idx + n);
                    return *this;
                }
                partial_operation(*_node, _bit_idx, TYPE_BIT(block_type));
                ++_node, n -= TYPE_BIT(block_type) - _bit_idx;
            }
            for (const block_type* _pend = _node + to_block_idx<block_type>(n); _node < _pend; full_operation(*_node++))
                ;
            if (to_bit_idx<block_type>(n))
                partial_operation(*_node, 0, to_bit_idx<block_type>(n));
            return *this;
        }

        template <class _Other, class _Fn>
        inline _Self& range_operation /*_with*/ (size_type index, const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type bstr_index, size_type bstr_count, _Fn operation) {
            block_type*     _node   = std::addressof(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(index)]);
            const size_type _maxcnt = bstr_index + bstr_count;
            for (const size_type _bit_idx = to_bit_idx<block_type>(index);; bstr_index += TYPE_BIT(block_type)) {
                const block_type _curr = _Bstr_accessor::get_block(bstr, bstr_index, _maxcnt);
                const size_type  _left = _maxcnt - bstr_index;
                if (_left >= TYPE_BIT(block_type)) {
                    operation(*_node, _bit_idx, _curr, 0, TYPE_BIT(block_type) - _bit_idx);
                    operation(*++_node, 0, _curr, TYPE_BIT(block_type) - _bit_idx, TYPE_BIT(block_type));
                }
                else {
                    if (_left <= TYPE_BIT(block_type) - _bit_idx)
                        operation(*_node, _bit_idx, _curr, 0, _left);
                    else {
                        operation(*_node, _bit_idx, _curr, 0, TYPE_BIT(block_type) - _bit_idx);
                        operation(*++_node, 0, _curr, TYPE_BIT(block_type) - _bit_idx, _left);
                    }
                    break;
                }
            }
            return *this;
        }

        template <class _Iter, class _Pred>
        size_type set_from_range(size_type beg, _Iter& first, _Iter last, _Pred pred) {
            for (; first != last && pred(beg); ++first) {
                if (*first == 1)
                    unchecked_set(beg++, true);
                else {
                    if (*first != 0)
                        throw std::invalid_argument("invaild bitstring element");
                    unchecked_set(beg++, false);
                }
            }
            return beg;
        }

        template <class _Iter>
        void set_from_range(size_type beg, _Iter first, _Iter last) {
            for (; first != last; ++first) {
                if (*first == 1)
                    unchecked_set(beg++, true);
                else {
                    if (*first != 0)
                        throw std::invalid_argument("invaild bitstring element");
                    unchecked_set(beg++, false);
                }
            }
        }

        template <class _Elem, class _Pred>
        size_type set_from_c_str(size_type beg, const _Elem* str, size_type str_count, _Elem elem0, _Elem elem1, _Pred pred) {
            size_type i = 0;
            while (pred(i)) {
                if (std::char_traits<_Elem>::eq(str[i], elem1))
                    unchecked_set(beg + i++, true);
                else if (std::char_traits<_Elem>::eq(str[i], elem0))
                    unchecked_set(beg + i++, false);
                else
                    throw std::invalid_argument("invaild bitstring char");
            }
            return i;
        }

        template <class _Tp>
        void set_from_type(size_type index, const _Tp& value, size_type n) {
            const size_type _block_idx = to_block_idx<block_type>(abspos(index)), _bit_idx = to_bit_idx<block_type>(abspos(index)), _oldn = n;
            block_type*     _node = std::addressof(_Bstr_accessor::get_vec(*this)[_block_idx]);
            size_type       i     = 0;
            for (; n >= TYPE_BIT(block_type); n -= TYPE_BIT(block_type), ++i) {
                set_bits(*_node, _bit_idx, value, i * TYPE_BIT(block_type), (i + 1) * TYPE_BIT(block_type) - _bit_idx);
                set_bits(*++_node, 0, value, (i + 1) * TYPE_BIT(block_type) - _bit_idx, (i + 1) * TYPE_BIT(block_type));
            }
            if (TYPE_BIT(block_type) - _bit_idx >= n)
                set_bits(*_node, _bit_idx, value, i * TYPE_BIT(block_type), _oldn);
            else {
                set_bits(*_node, _bit_idx, value, i * TYPE_BIT(block_type), (i + 1) * TYPE_BIT(block_type) - _bit_idx);
                set_bits(*++_node, 0, value, (i + 1) * TYPE_BIT(block_type) - _bit_idx, _oldn);
            }
        }

        inline block_type get_block(const size_type index, const size_type maxsz) const noexcept {
            const size_type _block_idx = to_block_idx<block_type>(index), _bit_idx = to_bit_idx<block_type>(index);
            return TYPE_BIT(block_type) - _bit_idx >= maxsz - index
                       ? _Bstr_accessor::get_vec(*this)[_block_idx] << _bit_idx
                       : _Bstr_accessor::get_vec(*this)[_block_idx] << _bit_idx | _Bstr_accessor::get_vec(*this)[_block_idx + 1] >> TYPE_BIT(block_type) - _bit_idx;
        }

        inline block_type get_block(const block_type* ptr, const size_type index, const size_type maxsz) const noexcept {
            const size_type _bit_idx = to_bit_idx<block_type>(index);
            return TYPE_BIT(block_type) - _bit_idx >= maxsz - index ? *ptr << _bit_idx : *ptr << _bit_idx | ptr[1] >> TYPE_BIT(block_type) - _bit_idx;
        }

        inline void check_index_exclusive(const size_type index, const char* msg = "invalid bitstring position") const {  // checks whether index is in the bounds of[0, size())
            if (index >= _size)
                throw std::out_of_range(msg);
        }

        inline void check_index(const size_type index, const char* msg = "invalid bitstring position") const {  // checks whether index is in the bounds of[0, size()]
            if (index > _size)
                throw std::out_of_range(msg);
        }

        template <class _Other>
        static void check_npos(const bitstring_base<_Block, _Alloc, _Other>& bstr, const size_type index, size_type& n, const char* msg = "invalid bitstring position") {
            if (index >= bstr.size())
                throw std::out_of_range(msg);
            if (n == bitstring_base<_Block, _Alloc, _Other>::npos)
                n = bstr.size() - index;
        }

        template <class _Other>
        void xswap(bitstring_base<_Block, _Alloc, _Other>& bstr) {
            static auto _exchange = [&] {
                *this ^= bstr;
                bstr ^= *this;
                *this ^= bstr;
            };
            std::make_signed_t<size_type> _dist = bstr.size() - _size;
            if (_dist < 0) {
                static_cast<_Other&>(bstr).resize(_size);
                _exchange();
                _Derptr()->resize(_size + _dist);
            }
            else if (_dist > 0) {
                _Derptr()->resize(bstr.size());
                _exchange();
                static_cast<_Other&>(bstr).resize(_size + _dist);
            }
            else
                _exchange();
        }

        size_type _size = 0;

    private:
        _Derived*       _Derptr() noexcept { return static_cast<_Derived*>(this); }
        const _Derived* _Derptr() const noexcept { return static_cast<const _Derived*>(this); }
    };

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    void bitstring_base<_Block, _Alloc, _Derived>::uninit_left_shift(size_type index, size_type n, size_type maxsz) {
        using signed_size_type      = std::make_signed_t<size_type>;
        const size_type  _beg_block = to_block_idx<block_type>(index), _beg_bit = to_bit_idx<block_type>(index);
        const size_type  _width = to_block_idx<block_type>(n + _beg_bit), _shift_bit = to_bit_idx<block_type>(n);
        const size_type  _first_need = TYPE_BIT(block_type) - _beg_bit, _start_bit = to_bit_idx<block_type>(_beg_bit + _shift_bit);
        block_type*      _node = std::addressof(_Bstr_accessor::get_vec(*this)[_beg_block]);
        signed_size_type _left = maxsz - n - _first_need;  // left bits after shifting the first block
        if (_left <= 0)
            set_bits<block_type, block_type>(*_node, _beg_bit, (_node[_width] << _start_bit) | (_node[_width + 1] >> TYPE_BIT(block_type) - _start_bit), 0, maxsz - n);
        else {
            set_bits<block_type, block_type>(*_node, _beg_bit, (_node[_width] << _start_bit) | (_node[_width + 1] >> TYPE_BIT(block_type) - _start_bit), 0, _first_need);
            if (_beg_bit > _start_bit) {                                                                                    //_ptr[_width] hasn't used up yet
                for (++_node; std::cmp_greater_equal(_left, TYPE_BIT(block_type)); _left -= TYPE_BIT(block_type), ++_node)  // suppress C6259
                    *_node = (_node[_width - 1] << TYPE_BIT(block_type) - _start_bit + _first_need) | (_node[_width] >> _start_bit - _first_need);
                set_bits<block_type, block_type>(*_node, 0, (_node[_width - 1] << TYPE_BIT(block_type) - _start_bit + _first_need) | (_node[_width] >> _start_bit - _first_need), 0, _left);
            }
            else {
                for (++_node; std::cmp_greater_equal(_left, TYPE_BIT(block_type)); _left -= TYPE_BIT(block_type), ++_node)
                    *_node = (_node[_width] << _shift_bit) | (_node[_width + 1] >> TYPE_BIT(block_type) - _shift_bit);
                set_bits<block_type, block_type>(*_node, 0, (_node[_width] << _shift_bit) | (_node[_width + 1] >> TYPE_BIT(block_type) - _shift_bit), 0, _left);
            }
        }
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    void bitstring_base<_Block, _Alloc, _Derived>::uninit_right_shift(size_type index, size_type n, size_type maxsz) {
        using signed_size_type     = std::make_signed_t<size_type>;
        const size_type _beg_bit   = to_bit_idx<block_type>(index);
        size_type       _end_block = to_block_idx<block_type>(index + maxsz), _end_bit = to_bit_idx<block_type>(_beg_bit + maxsz);
        if (_end_bit == 0)
            _end_bit = TYPE_BIT(block_type), --_end_block;
        const signed_size_type _width     = std::floor(1.0 * static_cast<signed_size_type>(_end_bit - n) / static_cast<signed_size_type>(TYPE_BIT(block_type)));
        const size_type        _start_bit = to_bit_idx<block_type>(index + maxsz - n), _shift_bit = to_bit_idx<block_type>(n);
        block_type*            _node = std::addressof(_Bstr_accessor::get_vec(*this)[_end_block]);
        signed_size_type       _left = maxsz - n - _end_bit;  // left bits after shifting the first block
        if (_left <= 0)
            set_bits<block_type, block_type>(*_node, _end_bit - maxsz + n, (_node[_width] >> TYPE_BIT(block_type) - _start_bit) | (_node[_width - 1] << _start_bit), TYPE_BIT(block_type) - maxsz + n,
                                             TYPE_BIT(block_type));
        else {
            set_bits<block_type, block_type>(*_node, 0, (_node[_width] >> TYPE_BIT(block_type) - _start_bit) | (_node[_width - 1] << _start_bit), TYPE_BIT(block_type) - _end_bit,
                                             TYPE_BIT(block_type));
            if (_start_bit > _end_bit) {  //_ptr[_width] hasn't used up yet
                for (--_node; std::cmp_greater_equal(_left, TYPE_BIT(block_type)); _left -= TYPE_BIT(block_type), --_node)
                    *_node = (_node[_width + 1] >> TYPE_BIT(block_type) - _start_bit + _end_bit) | (_node[_width] << _start_bit - _end_bit);
                set_bits<block_type, block_type>(*_node, TYPE_BIT(block_type) - _left, (_node[_width + 1] >> TYPE_BIT(block_type) - _start_bit + _end_bit) | (_node[_width] << _start_bit - _end_bit),
                                                 TYPE_BIT(block_type) - _left, TYPE_BIT(block_type));
            }
            else {
                for (--_node; std::cmp_greater_equal(_left, TYPE_BIT(block_type)); _left -= TYPE_BIT(block_type), --_node)
                    *_node = (_node[_width] >> _shift_bit) | (_node[_width - 1] << TYPE_BIT(block_type) - _shift_bit);
                set_bits<block_type, block_type>(*_node, TYPE_BIT(block_type) - _left, (_node[_width] >> _shift_bit) | (_node[_width - 1] << TYPE_BIT(block_type) - _shift_bit),
                                                 TYPE_BIT(block_type) - _left, TYPE_BIT(block_type));
            }
        }
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::set(size_type index, size_type n, bool value) {
        check_npos(*this, index, n);
        return unchecked_set(index, n, value);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::set(size_type index, bool value) {
        check_index_exclusive(index);
        return unchecked_set(index, value);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::set() {
        return range_operation(abspos(0), _size, set_partial, set_full);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::flip(size_type index, size_type n) {
        check_npos(*this, index, n);
        return range_operation(abspos(0) + index, n, flip_bits, flip_full);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::flip(size_type index) {
        check_index_exclusive(index);
        flip_bit(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(abspos(index))], to_bit_idx<block_type>(abspos(index)));
        return *this;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::flip() {
        return range_operation(abspos(0), _size, flip_bits, flip_full);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::left_shift(size_type n) {
        if (n >= _size)
            return reset();
        uninit_left_shift(abspos(0), n, _size);
        range_operation(abspos(_size - n), n, reset_partial, reset_full);
        return *this;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::right_shift(size_type n) {
        if (n >= _size)
            return reset();
        uninit_right_shift(abspos(0), n, _size);
        range_operation(abspos(0), n, reset_partial, reset_full);
        return *this;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _Other>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::or_with(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index) {
        check_index(bstr.size() + index, "the size of the bitstring used as parameter is too large");
        return range_operation(abspos(index), bstr, _Bstr_accessor::get_begin(bstr), bstr.size(), [](block_type& dst, size_type beg, const block_type& src, size_type first, size_type last) {
            block_type _tarbits = get_bits(src, first, last);
            dst |= beg < first ? _tarbits << (first - beg) : _tarbits >> (beg - first);
        });
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _Other>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::and_with(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index) {
        check_index(bstr.size() + index, "the size of the bitstring used as parameter is too large");
        return range_operation(abspos(index), bstr, _Bstr_accessor::get_begin(bstr), bstr.size(), [](block_type& dst, size_type beg, const block_type& src, size_type first, size_type last) {
            block_type _tarbits = get_bits(src, first, last);
            dst &= (std::numeric_limits<block_type>::max)() & (beg < first ? _tarbits << first - beg : _tarbits >> beg - first);
        });
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _Other>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::xor_with(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index) {
        check_index(bstr.size() + index, "the size of the bitstring used as parameter is too large");
        return range_operation(abspos(index), bstr, _Bstr_accessor::get_begin(bstr), bstr.size(), [](block_type& dst, size_type beg, const block_type& src, size_type first, size_type last) {
            block_type _tarbits = get_bits(src, first, last);
            dst ^= beg < first ? _tarbits << (first - beg) : _tarbits >> (beg - first);
        });
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _Other>
    bitstring_base<_Block, _Alloc, _Derived>& bitstring_base<_Block, _Alloc, _Derived>::subtract_with(const bitstring_base<_Block, _Alloc, _Other>& bstr, size_type index) {
        check_index(bstr.size() + index, "the size of the bitstring used as parameter is too large");
        return range_operation(abspos(index), bstr, _Bstr_accessor::get_begin(bstr), bstr.size(), [](block_type& dst, size_type beg, const block_type& src, size_type first, size_type last) {
            block_type _tarbits = get_bits(src, first, last);
            dst &= ~(beg < first ? _tarbits << first - beg : _tarbits >> beg - first);
        });
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    typename bitstring_base<_Block, _Alloc, _Derived>::reference bitstring_base<_Block, _Alloc, _Derived>::at(size_type index) {
        check_index_exclusive(index);
        return _Bstr_accessor::make_reference(*this, abspos(index));
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    typename bitstring_base<_Block, _Alloc, _Derived>::reference bitstring_base<_Block, _Alloc, _Derived>::operator[](size_type index) {
#if !defined(_NO_BSTRING_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
        assert(("invalid bitstring subscript", index < _size));
#endif
        return _Bstr_accessor::make_reference(*this, abspos(index));
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    typename bitstring_base<_Block, _Alloc, _Derived>::const_reference bitstring_base<_Block, _Alloc, _Derived>::at(size_type index) const {
        check_index_exclusive(index);
        return _Bstr_accessor::make_const_reference(*this, abspos(index));
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    typename bitstring_base<_Block, _Alloc, _Derived>::const_reference bitstring_base<_Block, _Alloc, _Derived>::operator[](size_type index) const {
#if !defined(_NO_BSTRING_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
        assert(("invalid bitstring subscript", index < _size));
#endif
        return _Bstr_accessor::make_const_reference(*this, abspos(index));
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _String>
    _String bitstring_base<_Block, _Alloc, _Derived>::to_string(typename _String::value_type elem0, typename _String::value_type elem1) const {
        return this->to_string(0, _size, elem0, elem1);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _String>
    void bitstring_base<_Block, _Alloc, _Derived>::to_string(_String& str, typename _String::value_type elem0, typename _String::value_type elem1) const {
        return this->to_string(str, 0, _size, elem0, elem1);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _String>
    _String bitstring_base<_Block, _Alloc, _Derived>::to_string(size_type index, size_type n, typename _String::value_type elem0, typename _String::value_type elem1) const {
        _String _str{};
        this->to_string(_str, index, n, elem0, elem1);
        return _str;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _String>
    void bitstring_base<_Block, _Alloc, _Derived>::to_string(_String& str, size_type index, size_type n, typename _String::value_type elem0, typename _String::value_type elem1) const {
        check_npos(*this, index, n);
        const block_type* _node = std::addressof(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(abspos(index))]);
        for (size_type _bit_idx = to_bit_idx<block_type>(abspos(index)); n--; _bit_idx = _bit_idx == 7 ? ++_node, 0 : _bit_idx + 1)
            str.push_back(get_bit(*_node, _bit_idx) ? elem1 : elem0);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _Tp>
    _Tp bitstring_base<_Block, _Alloc, _Derived>::to_type() const {
        return to_type(abspos(0), _size);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _Tp>
    void bitstring_base<_Block, _Alloc, _Derived>::to_type(_Tp& value) const {
        return to_type(value, abspos(0), _size);
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _Tp>
    _Tp bitstring_base<_Block, _Alloc, _Derived>::to_type(size_type index, size_type n) const {
        _Tp _value{};
        to_type(_value, index, n);
        return _value;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    template <class _Tp>
    void bitstring_base<_Block, _Alloc, _Derived>::to_type(_Tp& value, size_type index, size_type n) const {
#if !defined(_NO_BSTRING_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
        assert(index + n <= _size);
#endif
        if (sizeof(_Tp) * CHAR_BIT < n)
            throw std::overflow_error("overflow, argument 'n' > T-bits.");
        const size_type _zero_block = sizeof(_Tp) - to_block_idx(n);
        //_bit_idx + n <= block_bits
        if constexpr (std::endian::native == std::endian::big) {
            const size_type _bit_idx = to_bit_idx<block_type>(index);
            value                    = get_bits(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(index)], _bit_idx, _bit_idx + n) >> sizeof(_Tp) * CHAR_BIT - _bit_idx - n;
        }
        else {
        }
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bool bitstring_base<_Block, _Alloc, _Derived>::test(size_type index) const {
        check_index_exclusive(index);
        return unchecked_get(*this, abspos(index));
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bool bitstring_base<_Block, _Alloc, _Derived>::test_set(size_type index, bool value) {
        check_index_exclusive(index);
        const bool _oldbit = unchecked_get(*this, abspos(index));
        if (_oldbit ^ value)
            flip_bit(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(abspos(index))], to_bit_idx<block_type>(abspos(index)));
        return _oldbit;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bool bitstring_base<_Block, _Alloc, _Derived>::all() const {
        const size_type   _beg_bit = to_bit_idx<block_type>(abspos(0));
        size_type         n        = _size;
        const block_type* _node    = std::addressof(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(abspos(0))]);
        if (_beg_bit) {
            if (_size + _beg_bit <= TYPE_BIT(block_type))
                return get_bits(*_node, _beg_bit, _beg_bit + _size) == get_bits((std::numeric_limits<block_type>::max)(), _beg_bit, _beg_bit + _size);
            if (*_node << _beg_bit != (std::numeric_limits<block_type>::max)() << _beg_bit)
                return false;
            ++_node, n -= TYPE_BIT(block_type) - _beg_bit;
        }
        for (const block_type* _pend = _node + to_block_idx<block_type>(n); ++_node < _pend;)
            if (*_node != (std::numeric_limits<block_type>::max)())
                return false;
        return to_bit_idx<block_type>(n) ? (*_node >> TYPE_BIT(block_type) - to_bit_idx<block_type>(n) != (std::numeric_limits<block_type>::max)() >> TYPE_BIT(block_type) - to_bit_idx<block_type>(n))
                                         : true;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bool bitstring_base<_Block, _Alloc, _Derived>::any() const {
        const size_type   _beg_bit = to_bit_idx<block_type>(abspos(0));
        size_type         n        = _size;
        const block_type* _node    = std::addressof(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(abspos(0))]);
        if (_beg_bit) {
            if (_size + _beg_bit <= TYPE_BIT(block_type))
                return get_bits(*_node, _beg_bit, _beg_bit + _size) > 0;
            if (*_node << _beg_bit > 0)
                return true;
            ++_node, n -= TYPE_BIT(block_type) - _beg_bit;
        }
        for (const block_type* _pend = _node + to_block_idx<block_type>(n); ++_node < _pend;)
            if (*_node > 0)
                return true;
        return to_bit_idx<block_type>(n) ? *_node >> TYPE_BIT(block_type) - to_bit_idx<block_type>(n) > 0 : false;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    typename bitstring_base<_Block, _Alloc, _Derived>::size_type bitstring_base<_Block, _Alloc, _Derived>::count() const noexcept {
        static const char* const _bits_table = "\0\1\1\2\1\2\2\3\1\2\2\3\2\3\3\4"
                                               "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
                                               "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
                                               "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                                               "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
                                               "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                                               "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                                               "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
                                               "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
                                               "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                                               "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                                               "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
                                               "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                                               "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
                                               "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
                                               "\4\5\5\6\5\6\6\7\5\6\6\7\6\7\7\b";
        size_type                _count      = 0;
        constexpr auto           _count_full = [&](block_type value) {
            for (size_type i = 0; i < sizeof(block_type); ++i)
                _count += _bits_table[*(reinterpret_cast<byte*>(std::addressof(value)) + i)];
        };
        constexpr auto _count_partial = [&](block_type value, size_type first, size_type last) { _count_full(get_bits(value, first, last)); };
        const_cast<_Self&>(*this).range_operation(abspos(0), _size, _count_partial, _count_full);  // XD
        return _count;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    bool bitstring_base<_Block, _Alloc, _Derived>::operator==(const _Self& rhs) const {
        if (_size != rhs._size)
            return false;
        const block_type* _node    = std::addressof(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(abspos(0))]);
        const size_type   _bit_idx = to_bit_idx<block_type>(abspos(0));
        for (size_type _beg = _Bstr_accessor::get_begin(rhs); _beg < _Bstr_accessor::get_begin(rhs) + rhs.size(); _beg += TYPE_BIT(block_type))
            if (get_block(_node, _bit_idx, _size) != rhs.get_block(_beg, rhs._size))
                return false;
        return true;
    }

    template <std::unsigned_integral _Block, class _Alloc, class _Derived>
    std::strong_ordering bitstring_base<_Block, _Alloc, _Derived>::operator<=>(const _Self& rhs) const {
        if (_size != rhs._size)
            return _size <=> rhs._size;
        const block_type* _node    = std::addressof(_Bstr_accessor::get_vec(*this)[to_block_idx<block_type>(abspos(0))]);
        const size_type   _bit_idx = to_bit_idx<block_type>(abspos(0));
        for (size_type _beg = _Bstr_accessor::get_begin(rhs); _beg < _Bstr_accessor::get_begin(rhs) + rhs.size(); _beg += TYPE_BIT(block_type))
            if (std::strong_ordering _res = get_block(_node, _bit_idx, _size) <=> rhs.get_block(_beg, rhs._size); _res != std::strong_ordering::equivalent)
                return _res;
        return std::strong_ordering::equivalent;
    }

    template <std::unsigned_integral _Block, class _Alloc = std::allocator<_Block>>
    class basic_bitstring : public bitstring_base<_Block, _Alloc, basic_bitstring<_Block, _Alloc>> {
        using _Self     = basic_bitstring<_Block, _Alloc>;
        using _Base     = bitstring_base<_Block, _Alloc, _Self>;
        using _Bstr_ref = basic_bitstring_ref<_Block, _Alloc>;
        using _Base::_size;
        using _Base::check_index;
        using _Base::check_npos;
        using _Base::get_block;
        using _Base::range_operation;
        using _Base::unchecked_set;

    public:
        friend struct _Bstr_accessor;
        using _Base::npos;
        using byte           = typename _Base::byte;
        using size_type      = typename _Base::size_type;
        using buf_type       = typename _Base::buf_type;
        using block_type     = typename _Base::block_type;
        using reference      = typename _Base::reference;
        using allocator_type = typename _Base::allocator_type;
        using iterator       = typename _Base::iterator;
        using xstl_const_iterator = typename _Base::xstl_const_iterator;

        /**
         *	@brief Constructs empty bitstring. If no allocator is supplied, allocator is obtained from a default-constructed instance.
         *	@param alloc : allocator to use for all memory allocations of this bitstring
         */
        basic_bitstring() noexcept(noexcept(allocator_type())) : basic_bitstring(allocator_type()) {}
        explicit basic_bitstring(const allocator_type& alloc) noexcept : _vec(alloc) {}
        /**
         *	@brief Constructs the bitstring with n value.
         *	@param n : the size of the bitstring
         *	@param value : value to initialize the bitstring with
         *	@param alloc : allocator to use for all memory allocations of this bitstring
         */
        basic_bitstring(size_type n, bool value, const allocator_type& alloc = allocator_type())
            : _vec(to_fit_size<block_type>(n), value ? (std::numeric_limits<block_type>::max)() : block_type{ 0 }, alloc) {}
        /**
         *	@brief Constructs the bitstring with the contents of the range [first, last).
         *	@param first, last : range to copy the bits from
         *	@param alloc : allocator to use for all memory allocations of this bitstring
         */
        template <class _Iter>
        basic_bitstring(_Iter binary_first, _Iter binary_last, const allocator_type& alloc = allocator_type()) : _vec(alloc) {
            append(binary_first, binary_last);
        }
        /**
         *	@brief Constructs the bitstring with the contents initialized with a copy of the null-terminated character string pointed to by str
         *	@param str : pointer to an array of characters to use as source to initialize the string with
         *	@param elem0 : alternate character for unset bits in str
         * 	@param elem1 : alternate character for set bits in str
         *	@param alloc : allocator to use for all memory allocations of this bitstring
         */
        template <character_type _Elem>
        basic_bitstring(const _Elem* str, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1'), const allocator_type& alloc = allocator_type()) : _vec(alloc) {
            append(str, elem0, elem1);
        }
        /**
         *	@brief Constructs the bitstring with the contents initialized with a copy of the null-terminated character string pointed to by str
         *	@param str : pointer to an array of characters to use as source to initialize the string with
         *	@param str_count : size of the resulting bitstring
         *	@param elem0 : alternate character for unset bits in str
         * 	@param elem1 : alternate character for set bits in str
         *	@param alloc : allocator to use for all memory allocations of this bitstring
         */
        template <character_type _Elem>
        basic_bitstring(const _Elem* str, const size_type str_count, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1'), const allocator_type& alloc = allocator_type())
            : _vec(alloc) {
            append(str, str_count, elem0, elem1);
        }

        /**
         *	@brief Constructs the string with a substring [bstr_index, bstr_index+bstr_count) of other bstr
         *	@param bstr : another string to use as source to initialize the string with
         *	@param bstr_index : position of the first bit to include
         *	@param bstr_count : the size of substring
         *	@param alloc : allocator to use for all memory allocations of this bitstring
         */
        template <class _Other>
        basic_bitstring(const bitstring_base<_Block, _Alloc, _Other>& bstr, typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_index = 0,
                        typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_count = npos, const allocator_type& alloc = allocator_type())
            : _vec(alloc) {
            append(bstr, bstr_index, bstr_count);
        }
        basic_bitstring(const _Self& bstr) : _Base(bstr._size), _vec(bstr._vec) {}
        basic_bitstring(const _Self& bstr, const allocator_type& alloc = allocator_type()) : _Base(bstr._size), _vec(bstr._vec, alloc) {}
        /**
         *	@brief Constructs the string with the contents of other using move semantics
         *	@param bstr : another string to use as source to initialize the string with
         *	@param alloc : allocator to use for all memory allocations of this bitstring
         */
        basic_bitstring(_Self&& bstr) : _vec(std::move(bstr._vec)), _Base(std::exchange(bstr._size, size_type{ 0 })) {}
        basic_bitstring(_Self&& bstr, const allocator_type& alloc = allocator_type()) : _vec(std::move(bstr._vec), alloc), _Base(std::exchange(bstr._size, size_type{ 0 })) {}

        XSTL_NODISCARD size_type max_size() const noexcept { return _vec.max_size() * TYPE_BIT(block_type); }
        XSTL_NODISCARD size_type capacity() const noexcept { return _vec.size() * TYPE_BIT(block_type) - _size; }
        void                    shrink_to_fit() {
            _vec.resize(to_fit_size<block_type>(_size));
            _vec.shrink_to_fit();
        }

        template <class _Check = std::true_type>
        _Self& insert(size_type index, size_type n, bool value);
        template <character_type _Elem, class _Check = std::true_type>
        _Self& insert(size_type index, const _Elem* str, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) {
            return insert<_Elem, _Check>(index, str, std::char_traits<_Elem>::length(str), elem0, elem1);
        }
        template <character_type _Elem, class _Check = std::true_type>
        _Self& insert(size_type index, const _Elem* str, size_type str_count, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1'));
        template <class _Other, class _Check = std::true_type>
        _Self& insert(size_type index, const bitstring_base<_Block, _Alloc, _Other>& bstr, typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_index = 0,
                      typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_count = npos);

        iterator insert(xstl_const_iterator position, bool value) { return insert(position, 1, value); }
        iterator insert(xstl_const_iterator position, size_type n, bool value) {
            return iterator(_Bstr_accessor::make_reference(insert<std::false_type>(position->_Get_index(), n, value), position->_Get_index()), this);
        }
        template <class _Iter>
        iterator insert(xstl_const_iterator position, _Iter binary_first, _Iter binary_last);

        template <class _Tp, class _Check = std::true_type>
        iterator insert_type(size_type index, const _Tp& value, size_type n = TYPE_BIT(_Tp));
        template <class _Tp>
        iterator insert_type(xstl_const_iterator position, const _Tp& value, size_type n = TYPE_BIT(_Tp)) {
            return insert_type<_Tp, std::false_type>(position->_Get_index(), value, n);
        }

        template <class _Check = std::true_type>
        _Self&   erase(size_type index = 0, size_type n = npos);
        iterator erase(xstl_const_iterator position) { return erase(position, position + 1); }
        iterator erase(xstl_const_iterator first, xstl_const_iterator last);

        void resize(size_type new_size, bool value = false);
        void clear() {
            _vec.clear();
            _size = 0;
        }
        void push_back(bool value);
        void pop_back();

        void append(size_type n, bool value);
        template <class _Iter>
        void append(_Iter binary_first, _Iter binary_last);
        template <character_type _Elem>
        void append(const _Elem* str, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) {
            _Append_from_c_str(str, elem0, elem1, [&](size_type i) { return !std::char_traits<_Elem>::eq(str[i], static_cast<_Elem>('\0')); });
        }
        template <character_type _Elem>
        void append(const _Elem* str, size_type str_count, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) {
            _Append_from_c_str(str, elem0, elem1, [&](size_type i) { return i < str_count; });  // assert fail if str_count > the real size
        }
        template <class _Other>
        void append(const bitstring_base<_Block, _Alloc, _Other>& bstr, typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_index = 0,
                    typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_count = npos);
        template <class _Tp>
        void append_type(const _Tp& value, size_type n);

        using _Base::assign;
        template <class _Other>
        void assign(const bitstring_base<_Block, _Alloc, _Other>& bstr, typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_index = 0,
                    typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_count = npos) {
            check_npos(bstr, bstr_index, bstr_count);
            if (std::addressof(_vec) == std::addressof(_Bstr_accessor::get_vec(bstr))) {
                if (_size != bstr_count - bstr_index)
                    _Base::uninit_left_shift(0, _Bstr_accessor::get_begin(bstr) + bstr_index, _size), _size = bstr_count;
            }
            else
                _size = 0, append(bstr, bstr_index, bstr_count);
        }
        void assign(_Self&& bstr) noexcept {
            if (this != std::addressof(bstr)) {
                _vec  = std::move(bstr._vec);
                _size = std::exchange(bstr._size, size_type{ 0 });
            }
        }

        using _Base::at;
        using _Base::           operator[];
        XSTL_NODISCARD _Bstr_ref at(range_index index);
        XSTL_NODISCARD _Bstr_ref operator[](range_index index);

        void swap(_Self& x);
        void swap(_Bstr_ref& x);
        void swap(_Bstr_ref&& x) { this->swap(x); }

        _Self& operator=(const _Bstr_ref& rhs) {
            assign(rhs);
            return *this;
        }
        _Self& operator=(const _Self& rhs);
        _Self& operator=(_Self&& rhs) noexcept {
            assign(std::move(rhs));
            return *this;
        }

        // string operations
        template <class _Other>
        _Self& operator+=(const bitstring_base<_Block, _Alloc, _Other>& rhs);
        template <character_type _Elem>
        _Self& operator+=(const _Elem* str);

        template <class _Elem, class _Traits>
        friend std::basic_istream<_Elem, _Traits>& operator>>(std::basic_istream<_Elem, _Traits>& is, _Self& bstr) {
            using _Istream = std::basic_istream<_Elem, _Traits>;
            using _Ctype   = typename std::ctype<_Elem>;
            const typename _Istream::sentry _ok(is);
            bool                            _changed = false;
            if (_ok) {
                const _Ctype& _facet = std::use_facet<_Ctype>(is.getloc());
                bstr._size           = 0;  // erase all data
                try {
                    size_type _width = is.width() > 0 && static_cast<size_type>(is.width()) < bstr.max_size() ? static_cast<size_type>(is.width()) : bstr.max_size();
                    for (typename _Traits::int_type _curr = is.rdbuf()->sgetc(); _width; --_width, _curr = is.rdbuf()->snextc()) {
                        if (_Traits::eq_int_type(_Traits::eof(), _curr)) {
                            is.setstate(_Istream::eofbit);
                            break;
                        }
                        else if (_facet.is(_Ctype::space, _Traits::to_char_type(_curr)))
                            break;
                        else
                            bstr.push_back(_Traits::eq(_curr, '1') ? 1 : 0), _changed = true;
                    }
                } catch (...) {
                    is.setstate(_Istream::badbit);
                    throw;
                }
            }
            is.width(0);
            if (!_changed)
                is.setstate(_Istream::failbit);
            return is;
        }

        operator basic_bitstring_ref<_Block, _Alloc>() { return basic_bitstring_ref<_Block, _Alloc>(*this, 0, _size); }

    private:
        void _Left_move(size_type index, size_type n);
        void _Right_move(size_type index, size_type n);

        template <class _Elem, class _Pred>
        void _Append_from_c_str(const _Elem* str, _Elem elem0, _Elem elem1, _Pred pred) {
            for (size_type i = 0; pred(i); ++i) {
                if (std::char_traits<_Elem>::eq(str[i], elem1))
                    push_back(true);
                else if (std::char_traits<_Elem>::eq(str[i], elem0))
                    push_back(false);
                else
                    throw std::invalid_argument("invaild bitstring char");
            }
        }

        template <class _Iter>
        iterator _Insert_from_range(size_type index, _Iter first, _Iter last, std::forward_iterator_tag) {
            if (first != last) {
                _Right_move(index, std::distance(first, last));
                _Base::set_from_range(index, first, last);
            }
            return iterator(_Bstr_accessor::make_reference(*this, index), this);
        }

        template <class _Iter>
        iterator _Insert_from_range(size_type index, _Iter first, _Iter last, std::input_iterator_tag) {
            _Self _tmp(first, last);
            _Right_move(index, _tmp.size());
            range_operation(index, _tmp, 0, _tmp.size(), set_bits<block_type, block_type>);
            return iterator(_Bstr_accessor::make_reference(*this, index), this);
        }

        inline size_t          _Get_begin() const noexcept { return 0; }
        inline buf_type&       _Get_vec() noexcept { return _vec; }
        inline const buf_type& _Get_vec() const noexcept { return _vec; }

        typename _Base::buf_type _vec{};
    };

    template <std::unsigned_integral _Block, class _Alloc>
    void basic_bitstring<_Block, _Alloc>::_Left_move(size_type index, size_type n) {
        _Base::uninit_left_shift(index, n, _size);
        _size -= n;
    }

    template <std::unsigned_integral _Block, class _Alloc>
    void basic_bitstring<_Block, _Alloc>::_Right_move(size_type index, size_type n) {
        const size_type _newsz_block = to_fit_size<block_type>(_size + n);
        if (_newsz_block > _vec.size())
            _vec.resize(_newsz_block);
        _size += n;
        _Base::uninit_right_shift(index, n, _size - index);
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <class _Check>
    basic_bitstring<_Block, _Alloc>& basic_bitstring<_Block, _Alloc>::insert(size_type index, size_type n, bool value) {
        if (index == _size)
            append(n, value);
        else {
            if constexpr (_Check::value)
                check_index(index);
            _Right_move(index, n);
            value ? range_operation(index, n, _Base::set_partial, _Base::set_full) : range_operation(index, n, _Base::reset_partial, _Base::reset_full);
        }
        return *this;
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <character_type _Elem, class _Check>
    basic_bitstring<_Block, _Alloc>& basic_bitstring<_Block, _Alloc>::insert(size_type index, const _Elem* str, size_type str_count, _Elem elem0, _Elem elem1) {
        if (index == _size)
            append(str, str_count, elem0, elem1);
        else {
            if constexpr (_Check::value)
                check_index(index);
            _Right_move(index, str_count);
            _Base::set_from_c_str(index, str, str_count, elem0, elem1, [=](size_type i) { return i < str_count; });
        }
        return *this;
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <class _Other, class _Check>
    basic_bitstring<_Block, _Alloc>& basic_bitstring<_Block, _Alloc>::insert(size_type index, const bitstring_base<_Block, _Alloc, _Other>& bstr,
                                                                             typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_index,
                                                                             typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_count) {
        if constexpr (_Check::value)
            check_index(index);
        if (index == _size)
            append(bstr, bstr_index, bstr_count);
        else {
            check_npos(bstr, bstr_index, bstr_count);
            bstr_index += _Bstr_accessor::get_begin(bstr);
            _Right_move(index, bstr_count);
            if (std::addressof(_vec) == std::addressof(_Bstr_accessor::get_vec(bstr))) {
                if (bstr_index < index) {                   // source position before insertion position
                    if (bstr_index + bstr_count > index) {  // two intervals have intersection
                        range_operation(index, bstr, bstr_index, index - bstr_index, set_bits<block_type, block_type>);
                        range_operation(index * 2 - bstr_index, bstr, index + bstr_count, bstr_index + bstr_count - index, set_bits<block_type, block_type>);
                        return *this;
                    }
                }
                else
                    bstr_index += bstr_count;
            }
            range_operation(index, bstr, bstr_index, bstr_count, set_bits<block_type, block_type>);
        }
        return *this;
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <class _Iter>
    typename basic_bitstring<_Block, _Alloc>::iterator basic_bitstring<_Block, _Alloc>::insert(xstl_const_iterator position, _Iter binary_first, _Iter binary_last) {
        if (position == _Base::cend()) {
            append(binary_first, binary_last);
            return iterator(_Bstr_accessor::make_reference(*this, position->_Get_index()), this);
        }
        return _Insert_from_range(position->_Get_index(), binary_first, binary_last, typename std::iterator_traits<_Iter>::iterator_category{});
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <class _Tp, class _Check>
    typename basic_bitstring<_Block, _Alloc>::iterator basic_bitstring<_Block, _Alloc>::insert_type(size_type index, const _Tp& value, size_type n) {
        if constexpr (_Check::value)
            check_index(index);
        if (n > TYPE_BIT(_Tp))
            n = TYPE_BIT(_Tp);
        _Right_move(index, n);
        _Base::set_from_type(index, value, n);
        return iterator(_Bstr_accessor::make_reference(*this, index), this);
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <class _Check>
    basic_bitstring<_Block, _Alloc>& basic_bitstring<_Block, _Alloc>::erase(size_type index, size_type n) {
        if constexpr (_Check::value)
            check_index(index);
        return index + n >= _size ? (_size = index, *this) : (_Left_move(index, n), *this);
    }

    template <std::unsigned_integral _Block, class _Alloc>
    typename basic_bitstring<_Block, _Alloc>::iterator basic_bitstring<_Block, _Alloc>::erase(xstl_const_iterator first, xstl_const_iterator last) {
        const size_type _first = first->_Get_index(), _last = last->_Get_index();
        return iterator(_Bstr_accessor::make_reference(erase<std::false_type>(_first, _last - _first), _last), this);
    }

    template <std::unsigned_integral _Block, class _Alloc>
    void basic_bitstring<_Block, _Alloc>::resize(size_type new_size, bool value) {
        if (new_size > _size)
            append(new_size - _size, value);
        else if (new_size < _size) {  // shrink
            _vec.resize(to_fit_size<block_type>(new_size));
            _size = new_size;
        }
    }

    template <std::unsigned_integral _Block, class _Alloc>
    void basic_bitstring<_Block, _Alloc>::push_back(bool value) {
        if (to_bit_idx<block_type>(_size) == 0 && _vec.size() == to_block_idx<block_type>(_size))
            _vec.push_back(block_type{ 0 });
        unchecked_set(_size++, value);
    }

    template <std::unsigned_integral _Block, class _Alloc>
    void basic_bitstring<_Block, _Alloc>::pop_back() {
#if !defined(_NO_BSTRING_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
        assert(("bitstring empty before pop", _size != 0));
#endif
        clear_bit(_vec[to_block_idx<block_type>(_size)], to_bit_idx<block_type>(_size));
        --_size;
    }

    template <std::unsigned_integral _Block, class _Alloc>
    void basic_bitstring<_Block, _Alloc>::append(size_type n, bool value) {
        const size_type _newsz = _size + n, _fitsz_block = to_fit_size<block_type>(_newsz);
        if (_fitsz_block > _vec.size()) {  // reallocate
            if (value) {
                _vec.resize(_fitsz_block, (std::numeric_limits<block_type>::max)());  // may throw
                range_operation(_size, _vec.size() * TYPE_BIT(block_type) - _size, _Base::set_partial, _Base::set_full);
            }
            else {
                _vec.resize(_fitsz_block, block_type{ 0 });
                range_operation(_size, _vec.size() * TYPE_BIT(block_type) - _size, _Base::reset_partial, _Base::reset_full);
            }
        }
        else
            range_operation(_size, _newsz - _size, value ? _Base::set_partial : _Base::reset_partial, value ? _Base::set_full : _Base::reset_full);
        _size = _newsz;
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <class _Tp>
    void basic_bitstring<_Block, _Alloc>::append_type(const _Tp& value, size_type n) {
        if (n > TYPE_BIT(_Tp))
            n = TYPE_BIT(_Tp);
        const size_type _newsz_block = to_fit_size<block_type>(_size + n);
        if (_newsz_block > _vec.size())
            _vec.resize(_newsz_block);
        _Base::set_from_type(_size, value, n);
        _size += n;
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <class _Iter>
    void basic_bitstring<_Block, _Alloc>::append(_Iter binary_first, _Iter binary_last) {
        for (; binary_first != binary_last; ++binary_first) {
            if (*binary_first == 1)
                push_back(true);
            else {
                if (*binary_first != 0)
                    throw std::invalid_argument("invaild bitstring element");
                push_back(false);
            }
        }
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <class _Other>
    void basic_bitstring<_Block, _Alloc>::append(const bitstring_base<_Block, _Alloc, _Other>& bstr, typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_index,
                                                 typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_count) {
        check_npos(bstr, bstr_index, bstr_count);
        const size_type _newsz = _size + bstr_count, _fitsz_block = to_fit_size<block_type>(_newsz);
        if (_fitsz_block > _vec.size())
            _vec.resize(_fitsz_block);
        range_operation(std::exchange(_size, _newsz), bstr, bstr_index + _Bstr_accessor::get_begin(bstr), bstr_count, set_bits<block_type, block_type>);
    }

    template <std::unsigned_integral _Block, class _Alloc>
    basic_bitstring_ref<_Block, _Alloc> basic_bitstring<_Block, _Alloc>::at(range_index index) {
        _Base::check_index(index._first), _Base::check_index_exclusive(index._last);
        return basic_bitstring_ref<_Block, _Alloc>(*this, index._first, index._last);
    }

    template <std::unsigned_integral _Block, class _Alloc>
    basic_bitstring_ref<_Block, _Alloc> basic_bitstring<_Block, _Alloc>::operator[](range_index index) {
#if !defined(_NO_BSTRING_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
        assert(("invalid bitstring subscript", index._first < _size && index._last <= _size));
#endif
        return basic_bitstring_ref<_Block, _Alloc>(*this, index._first, index._last);
    }

    template <std::unsigned_integral _Block, class _Alloc>
    void basic_bitstring<_Block, _Alloc>::swap(_Self& x) {
        if (this != std::addressof(x)) {
            _vec.swap(x._vec);
            std::swap(_size, x._size);
        }
    }

    template <std::unsigned_integral _Block, class _Alloc>
    void basic_bitstring<_Block, _Alloc>::swap(_Bstr_ref& x) {
        if (std::addressof(_vec) != std::addressof(_Bstr_accessor::get_vec(x)))
            _Base::xswap(x);
    }

    template <std::unsigned_integral _Block, class _Alloc>
    basic_bitstring<_Block, _Alloc>& basic_bitstring<_Block, _Alloc>::operator=(const _Self& rhs) {
        if (this != std::addressof(rhs)) {
            _vec  = rhs._vec;
            _size = rhs._size;
        }
        return *this;
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <class _Other>
    basic_bitstring<_Block, _Alloc>& basic_bitstring<_Block, _Alloc>::operator+=(const bitstring_base<_Block, _Alloc, _Other>& rhs) {
        append(rhs);
        return *this;
    }

    template <std::unsigned_integral _Block, class _Alloc>
    template <character_type _Elem>
    basic_bitstring<_Block, _Alloc>& basic_bitstring<_Block, _Alloc>::operator+=(const _Elem* str) {
        append(str);
        return *this;
    }

    template <std::unsigned_integral _Block, class _Alloc = std::allocator<_Block>>
    class basic_bitstring_ref : public bitstring_base<_Block, _Alloc, basic_bitstring_ref<_Block, _Alloc>> {
        using _Self = basic_bitstring_ref<_Block, _Alloc>;
        using _Base = bitstring_base<_Block, _Alloc, _Self>;
        using _Bstr = typename _Base::_Bstr;
        using _Base::_size;
        using _Base::check_index;
        using _Base::unchecked_set;

    public:
        friend struct _Bstr_accessor;
        using _Base::npos;
        using size_type      = typename _Base::size_type;
        using buf_type       = typename _Base::buf_type;
        using block_type     = typename _Base::block_type;
        using iterator       = typename _Base::iterator;
        using xstl_const_iterator = typename _Base::xstl_const_iterator;
        using reference      = typename _Base::reference;

        basic_bitstring_ref(_Bstr& bstr, size_t first, size_t last) : _bstr(bstr), _beg(first), _Base(last - first) {}
        basic_bitstring_ref(const _Self& x) : _bstr(x._bstr), _beg(x._beg), _Base(x._size) {}

        _Self& insert(size_type index, size_type n, bool value) {
            check_index(index);
            _Size_fix_operation([&] { _bstr.insert<std::false_type>(_beg + index, n, value); });
            return *this;
        }
        template <character_type _Elem>
        _Self& insert(size_type index, const _Elem* str, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) {
            return insert(index, str, std::char_traits<_Elem>::length(str), elem0, elem1);
        }
        template <character_type _Elem>
        _Self& insert(size_type index, const _Elem* str, size_type str_count, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) {
            check_index(index);
            _Size_fix_operation([&] { _bstr.insert<_Elem, std::false_type>(_beg + index, str, str_count, elem0, elem1); });
            return *this;
        }
        template <class _Other>
        _Self& insert(size_type index, const bitstring_base<_Block, _Alloc, _Other>& bstr, typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_index = 0,
                      typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_count = npos) {
            check_index(index);
            _Size_fix_operation([&] { _bstr.insert<_Other, std::false_type>(_beg + index, bstr, bstr_index, bstr_count); });
            return *this;
        }

        iterator insert(xstl_const_iterator position, bool value) { return insert(position, 1, value); }
        iterator insert(xstl_const_iterator position, size_type n, bool value) {
            _bstr.insert<std::false_type>(position->_Get_index(), n, value);
            _size += n;
            return iterator(_Bstr_accessor::make_reference(*this, position->_Get_index()), this);
        }
        template <class _Iter>
        iterator insert(xstl_const_iterator position, _Iter binary_first, _Iter binary_last) {
            _Size_fix_operation([&] { _bstr.insert(_bstr.cbegin() + position->_Get_index(), binary_first, binary_last); });
            return iterator(_Bstr_accessor::make_reference(*this, position->_Get_index()), this);
        }

        template <class _Tp>
        iterator insert_type(size_type index, const _Tp& value, size_type n = TYPE_BIT(_Tp)) {
            check_index(index);
            _Size_fix_operation([&] { _bstr.insert_type<_Tp, std::false_type>(_beg + index, value, n); });
            return iterator(_Bstr_accessor::make_reference(*this, index), this);
        }
        template <class _Tp>
        iterator insert_type(xstl_const_iterator position, const _Tp& value, size_type n = TYPE_BIT(_Tp)) {
            _Size_fix_operation([&] { _bstr.insert_type<_Tp, std::false_type>(position->_Get_index(), value, n); });
            return iterator(_Bstr_accessor::make_reference(*this, position->_Get_index()), this);
        }

        _Self& erase(size_type index = 0, size_type n = npos) {
            _Base::check_npos(*this, index, n);
            _Size_fix_operation([&] { _bstr.erase<std::false_type>(_beg + index, _size - index); });
            return *this;
        }
        iterator erase(xstl_const_iterator position) { return erase(position, position + 1); }
        iterator erase(xstl_const_iterator first, xstl_const_iterator last) {
            _Size_fix_operation([&] { _bstr.erase(_bstr.cbegin() + first->_Get_index(), _bstr.cbegin() + last->_Get_index()); });
            return iterator(_Bstr_accessor::make_reference(*this, first->_Get_index()), this);
        }

        void resize(size_type new_size, bool value = false) {
            if (new_size > _size)
                append(new_size - _size, value);
            else if (new_size < _size)
                _Size_fix_operation([&] { _bstr.erase<std::false_type>(_beg + new_size, _size - new_size); });
        }
        void clear() { erase(); }
        void push_back(bool value) { append(1, value); }
        void pop_back() {
#if !defined(_NO_BSTRING_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("invalid to pop_back empty bitstring", _size != 0));
#endif
            _Size_fix_operation([&] { _bstr.erase<std::false_type>(_beg + _size, 1); });
        }

        void append(size_type n, bool value) {
            _Size_fix_operation([&] { _bstr.insert<std::false_type>(_beg + _size, n, value); });
        }
        template <class _Iter>
        void append(_Iter binary_first, _Iter binary_last) {
            insert(this->cend(), binary_first, binary_last);
        }
        template <character_type _Elem>
        void append(const _Elem* str, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) {
            append(str, std::char_traits<_Elem>::length(str), elem0, elem1);
        }
        template <character_type _Elem>
        void append(const _Elem* str, size_type str_count, _Elem elem0 = static_cast<_Elem>('0'), _Elem elem1 = static_cast<_Elem>('1')) {
            _Size_fix_operation([&] { _bstr.insert<_Elem, std::false_type>(_beg + _size, str, str_count, elem0, elem1); });
        }
        template <class _Other>
        void append(const bitstring_base<_Block, _Alloc, _Other>& bstr, typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_index = 0,
                    typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_count = _Base::npos) {
            _Size_fix_operation([&] { _bstr.insert<_Other, std::false_type>(_beg + _size, bstr, bstr_index, bstr_count); });
        }
        template <class _Tp>
        void append_type(const _Tp& value, size_type n) {
            _Size_fix_operation([&] { _bstr.insert_type<_Tp, std::false_type>(_beg + _size, value, n); });
        }

        using _Base::assign;
        template <class _Other>
        void assign(const bitstring_base<_Block, _Alloc, _Other>& bstr, typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_index = 0,
                    typename bitstring_base<_Block, _Alloc, _Other>::size_type bstr_count = npos) {
            _Base::check_npos(bstr, bstr_index, bstr_count);
            const std::make_signed_t<size_type> _dist = bstr_count - _size;
            resize(bstr_count);
            bstr_index += _Bstr_accessor::get_begin(bstr);
            if (std::addressof(_Bstr_accessor::get_vec(_bstr)) == std::addressof(_Bstr_accessor::get_vec(bstr))) {
                if (bstr_index < _beg) {                   // source position before the beginning of the splice
                    if (bstr_index + bstr_count > _beg) {  // two intervals have intersection
                        _Bstr _tmp(_bstr, bstr_index, bstr_count);
                        _Base::range_operation(_beg, _tmp, 0, bstr_count, set_bits<block_type, block_type>);
                        return;
                    }
                }
                else
                    bstr_index += _dist;
            }
            _Base::range_operation(_beg, bstr, bstr_index, bstr_count, set_bits<block_type, block_type>);
        }

        using _Base::at;
        using _Base::       operator[];
        XSTL_NODISCARD _Self at(range_index index) {
            check_index(index._first), _Base::check_index_exclusive(index._last);
            return basic_bitstring_ref<_Block, _Alloc>(_bstr, _beg + index._first, _beg + index._last);
        }
        XSTL_NODISCARD _Self operator[](range_index index) {
#if !defined(_NO_BSTRING_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFATRY_VERIFY_)
            assert(("invalid bitstring subscript", index._first < _size && index._last <= _size));
#endif
            return basic_bitstring_ref<_Block, _Alloc>(_bstr, _beg + index._first, _beg + index._last);
        }

        template <class _Other>
        void swap(bitstring_base<_Block, _Alloc, _Other>& bstr) {
            if (std::addressof(_Bstr_accessor::get_vec(_bstr)) == std::addressof(_Bstr_accessor::get_vec(bstr)))
                if (_beg + _size > _Bstr_accessor::get_begin(bstr) || _Bstr_accessor::get_begin(bstr) + bstr.size() > _beg)
                    return;
            _Base::xswap(bstr);
        }

        template <class _Other>
        void swap(bitstring_base<_Block, _Alloc, _Other>&& bstr) {
            this->swap(bstr);
        }

        _Self& operator=(const _Self& rhs) {
            assign(rhs);
            return *this;
        }
        _Self& operator=(const _Bstr& rhs) {
            assign(rhs);
            return *this;
        }

        ~basic_bitstring_ref() {
            // DO NOTHING
        }

    private:
        inline size_t          _Get_begin() const noexcept { return _beg; }
        inline buf_type&       _Get_vec() noexcept { return _Bstr_accessor::get_vec(_bstr); }
        inline const buf_type& _Get_vec() const noexcept { return _Bstr_accessor::get_vec(_bstr); }

        template <class _Fn>
        void _Size_fix_operation(_Fn operation) {
            const size_type _oldsz = _bstr.size();
            operation();
            _size += static_cast<std::make_signed_t<size_type>>(_bstr.size() - _oldsz);
        }

        _Bstr&       _bstr;
        const size_t _beg;  // be consistent with range_index
    };

    using bitstring = basic_bitstring<default_block_type, _DEFAULT_ALLOC(default_block_type)>;

    range_index operator""_ri(unsigned long long index) { return range_index(static_cast<size_t>(index)); }

    bitstring operator""_bstr(const char* str, size_t) { return bitstring(str); }

}  // namespace xstl

namespace std {
    template <unsigned_integral _Block, class _Alloc>
    struct pointer_traits<
        xstl::iter_adapter::rand_iter<typename xstl::bitstring_base<_Block, _Alloc, xstl::basic_bitstring<_Block, _Alloc>>::reference, xstl::_Bstr_pack<xstl::basic_bitstring<_Block, _Alloc>>>> {
        using _Bstr_base      = xstl::bitstring_base<_Block, _Alloc, xstl::basic_bitstring<_Block, _Alloc>>;
        using pointer         = xstl::iter_adapter::rand_iter<typename _Bstr_base::reference, xstl::_Bstr_pack<xstl::basic_bitstring<_Block, _Alloc>>>;
        using element_type    = typename _Bstr_base::reference;
        using difference_type = typename _Bstr_base::difference_type;

        XSTL_NODISCARD static constexpr pointer pointer_to(element_type& value) noexcept { return pointer(value); }
    };

    template <unsigned_integral _Block, class _Alloc>
    struct pointer_traits<xstl::iter_adapter::rand_citer<typename xstl::bitstring_base<_Block, _Alloc, xstl::basic_bitstring<_Block, _Alloc>>::const_reference,
                                                          xstl::_Bstr_pack<xstl::basic_bitstring<_Block, _Alloc>>>> {
        using _Bstr_base      = xstl::bitstring_base<_Block, _Alloc, xstl::basic_bitstring<_Block, _Alloc>>;
        using pointer         = xstl::iter_adapter::rand_citer<typename _Bstr_base::const_reference, xstl::_Bstr_pack<xstl::basic_bitstring<_Block, _Alloc>>>;
        using element_type    = typename _Bstr_base::const_reference;
        using difference_type = typename _Bstr_base::difference_type;

        XSTL_NODISCARD static constexpr pointer pointer_to(element_type& value) noexcept { return pointer(value); }
    };

    template <unsigned_integral _Block, class _Alloc>
    struct pointer_traits<xstl::iter_adapter::rand_iter<typename xstl::bitstring_base<_Block, _Alloc, xstl::basic_bitstring_ref<_Block, _Alloc>>::reference,
                                                         xstl::_Bstr_pack<xstl::basic_bitstring_ref<_Block, _Alloc>>>> {
        using _Bstr_base      = xstl::bitstring_base<_Block, _Alloc, xstl::basic_bitstring_ref<_Block, _Alloc>>;
        using pointer         = xstl::iter_adapter::rand_iter<typename _Bstr_base::const_reference, xstl::_Bstr_pack<xstl::basic_bitstring_ref<_Block, _Alloc>>>;
        using element_type    = typename _Bstr_base::reference;
        using difference_type = typename _Bstr_base::difference_type;

        XSTL_NODISCARD static constexpr pointer pointer_to(element_type& value) noexcept { return pointer(value); }
    };

    template <unsigned_integral _Block, class _Alloc>
    struct pointer_traits<xstl::iter_adapter::rand_citer<typename xstl::bitstring_base<_Block, _Alloc, xstl::basic_bitstring_ref<_Block, _Alloc>>::reference,
                                                          xstl::_Bstr_pack<xstl::basic_bitstring_ref<_Block, _Alloc>>>> {
        using _Bstr_base      = xstl::bitstring_base<_Block, _Alloc, xstl::basic_bitstring_ref<_Block, _Alloc>>;
        using pointer         = xstl::iter_adapter::rand_citer<typename _Bstr_base::const_reference, xstl::_Bstr_pack<xstl::basic_bitstring_ref<_Block, _Alloc>>>;
        using element_type    = typename _Bstr_base::const_reference;
        using difference_type = typename _Bstr_base::difference_type;

        XSTL_NODISCARD static constexpr pointer pointer_to(element_type& value) noexcept { return pointer(value); }
    };

    template <unsigned_integral _Block, class _Alloc, class _Derived>
    void swap(xstl::bitstring_base<_Block, _Alloc, _Derived>& x, xstl::bitstring_base<_Block, _Alloc, _Derived>& y) {
        x.swap(y);
    }
}  // namespace std
#endif