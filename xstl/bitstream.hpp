#ifndef _BITSTREAM_HPP_
#define _BITSTREAM_HPP_
#include "bitstring.hpp"
#include <bit>  //for little/big endian
#include <ios>
#include <limits>
#include <memory>
#include <optional>
#pragma warning(push)
#pragma warning(disable : 4244 4250 4554 6011)
#define BYTE_BIT TYPE_BIT(bios::byte)

namespace xstl {
    /**
     *	@class bios_base
     *	@brief The class bios_base is a multipurpose class that serves as the base class for all bits I/O stream classes
     */
    class bios_base {
    public:
        using failure    = std::ios::failure;
        using seekdir    = int;
        using openmode   = int;
        using biostate   = int;
        using bfmtflags  = int;
        using bitsize    = long long;
        using streamsize = long long;
        using byte       = std::uint8_t;
        static_assert(std::is_unsigned_v<byte>, "type 'byte' should be unsigned type");

        bios_base(bios_base&) = delete;
        bios_base& operator=(const bios_base&) = delete;

        bfmtflags         flags() const { return _fmtfl; }
        virtual bfmtflags flags(bfmtflags flags) {  // allow derived class do external operations when flag changed
            const bfmtflags _oldflags = _fmtfl;
            _fmtfl                    = flags;
            return _oldflags;
        }

        virtual bfmtflags setf(bfmtflags flags) {  // allow derived class do external operations when flag changed
            const bfmtflags _oldflags = _fmtfl;
            _fmtfl |= flags;
            return _oldflags;
        }
        bfmtflags setf(bfmtflags flags, bfmtflags mask) { return setf((_fmtfl & ~mask) | (flags & mask)); }

        virtual void unsetf(bfmtflags flags) { _fmtfl &= ~flags; }  // allow derived class do external operations when flag changed

        XSTL_NODISCARD bool     good() const { return rdstate() == goodbit; }
        XSTL_NODISCARD bool     eof() const { return rdstate() & eofbit; }
        XSTL_NODISCARD bool     fail() const { return rdstate() & (badbit | failbit); }
        XSTL_NODISCARD bool     bad() const { return rdstate() & badbit; }
        XSTL_NODISCARD biostate rdstate() const { return _state; }

        void setstate(biostate state) { clear(rdstate() | state); }
        void clear(biostate state = goodbit);

        XSTL_NODISCARD biostate exceptions() const { return _exception; }
        void                   exceptions(biostate except) {
            _exception = except;
            clear(rdstate());
        }

        void swap(bios_base& rhs) { std::swap(_fmtfl, rhs._fmtfl); }

        virtual ~bios_base() noexcept {}

    private:
        bfmtflags _fmtfl     = 0;
        biostate  _exception = goodbit;
        biostate  _state     = goodbit;

    protected:
        bios_base() = default;

    public:
        // bitstream format flags
        static constexpr bfmtflags littleendian = 0x0000;
        static constexpr bfmtflags bigendian    = 0x0001;

        // biostate
        static constexpr biostate goodbit = 0x0000;
        static constexpr biostate eofbit  = 0x0001;
        static constexpr biostate failbit = 0x0002;
        static constexpr biostate badbit  = 0x0004;

        // openmode
        static constexpr openmode in  = 0x0001;
        static constexpr openmode out = 0x0002;
        static constexpr openmode ate = 0x0004;
        static constexpr openmode app = 0x0008;

        // seekdir
        static constexpr seekdir beg = 0x0001;
        static constexpr seekdir cur = 0x0002;
        static constexpr seekdir end = 0x0004;
    };

    constexpr bios_base::bfmtflags bios_base::bigendian;
    constexpr bios_base::bfmtflags bios_base::littleendian;

    constexpr bios_base::biostate bios_base::goodbit;
    constexpr bios_base::biostate bios_base::badbit;
    constexpr bios_base::biostate bios_base::eofbit;
    constexpr bios_base::biostate bios_base::failbit;

    constexpr bios_base::openmode bios_base::in;
    constexpr bios_base::openmode bios_base::out;
    constexpr bios_base::openmode bios_base::ate;
    constexpr bios_base::openmode bios_base::app;

    constexpr bios_base::seekdir bios_base::beg;
    constexpr bios_base::seekdir bios_base::cur;
    constexpr bios_base::seekdir bios_base::end;

    void bios_base::clear(biostate state) {
        _state = state;
        if (const auto _excode = _state & _exception; _excode) {
            const char* _msg;
            if (_excode & badbit)
                _msg = "bios_base::badbit set";
            else if (_excode & failbit)
                _msg = "bios_base::failbit set";
            else
                _msg = "bios_base::eofbit set";
            throw failure(_msg);
        }
    }

    using bios = bios_base;

    template <class _Iter>
    concept standard_layout_iterator = std::is_standard_layout_v<std::iter_value_t<_Iter>> && !std::is_same_v<std::iter_value_t<_Iter>, bool>;
    template <class _Tp>
    concept standard_layout_type = std::is_standard_layout_v<_Tp> && !std::is_same_v<_Tp, bool>;
    template <class _Iter>
    concept arithmetic_iterator = std::is_arithmetic_v<std::iter_value_t<_Iter>> && !std::is_same_v<std::iter_value_t<_Iter>, bool>;
    template <class _Tp>
    concept arithmetic_type = std::is_arithmetic_v<_Tp> && !std::is_same_v<_Tp, bool>;
    template <class _Iter>
    concept bool_iterator = std::same_as < std::iter_value_t<_Iter>,
    bool > ;
    template <class _Tp>
    concept byte_type = character_type<_Tp> && sizeof(_Tp) == sizeof(bios::byte);
    template <class _Iter>
    concept char_input_iterator = character_type<std::iter_value_t<_Iter>> && std::input_iterator<_Iter>;
    template <class _Iter>
    concept char_output_iterator = character_type<std::iter_value_t<_Iter>> &&(
        std::output_iterator<
            _Iter,
            char> || std::output_iterator<_Iter, signed char> || std::output_iterator<_Iter, unsigned char> || std::output_iterator<_Iter, wchar_t> || std::output_iterator<_Iter, char8_t> || std::output_iterator<_Iter, char16_t> || std::output_iterator<_Iter, char32_t>);

    template <class _Iter>
    concept byte_input_iterator = char_input_iterator<_Iter> && sizeof(std::iter_value_t<_Iter>) == sizeof(bios::byte);
    template <class _Iter>
    concept byte_output_iterator = char_output_iterator<_Iter> && sizeof(std::iter_value_t<_Iter>) == sizeof(bios::byte);

    /**
     *	@class basic_bitbuf
     *	@brief The class basic_bitbuf controls input and output to a bit sequence.
     */
    template <class _Alloc>
    class basic_bitbuf {
        using _Self = basic_bitbuf<_Alloc>;
        // using _Bstr     = basic_bitstring<default_block_type, typename std::allocator_traits<_Alloc>::template rebind_alloc<default_block_type>>;
        // using _Bstr_ref = basic_bitstring_ref<default_block_type, typename std::allocator_traits<_Alloc>::template rebind_alloc<default_block_type>>;
        template <class>
        friend class basic_ibitstream;
        enum { ALLOCATED = 0x0001, WRITABLE = 0x0002, READABLE = 0x0004, APPEND = 0x0008, ATEND = 0x0010 };

    public:
        static_assert(sizeof(typename std::allocator_traits<_Alloc>::value_type) == 1, "bitbuf requires allocators for char types");

        using allocator_type = _Alloc;
        using bitsize        = typename bios::bitsize;
        using pos_type       = std::pair<std::streamsize, bitsize>;
        using off_type       = bitsize;
        using char_type      = char;
        using byte           = typename bios::byte;
        using int_type       = int;

        basic_bitbuf(bios::openmode mode = bios::in | bios::out) : _state(_Getstate(mode)) {}
        template <byte_type _Byte>
        basic_bitbuf(_Byte* newbuf, pos_type usedpos, std::streamsize bufsz, bios::openmode mode = bios::in | bios::out)
            : _buf(reinterpret_cast<byte*>(newbuf)), _bufsz(bufsz), _usedbits(usedpos.first * BYTE_BIT + usedpos.second), _state(_Getstate(mode)) {
            _countp = _bufsz * BYTE_BIT - _usedbits;
            if (_state & (ATEND | APPEND) & bios::out) {
                _nextp = usedpos.first;
                _offp  = usedpos.second;
            }
            else
                _countg = _usedbits;
        }
        basic_bitbuf(const _Self&) = delete;
        basic_bitbuf(_Self&& x) { this->swap(std::move(x)); }

        /**
         *	@brief removal or replacement of the controlled character sequence (the buffer) with a user-provided array
         *	@param newbuf pointer to the first char in the user-provided buffer
         *	@param usedpos the position of new buffer that has used
         *	@param bufsz : the size of new buffer
         */
        template <byte_type _Byte>
        _Self* setbuf(_Byte* newbuf, pos_type usedpos, std::streamsize bufsz);

        /**
         *	@brief repositions the stream pointer, if possible, to the position that corresponds to exactly off characters from beginning, end, or current position of the stream
         *	@param off : relative position to set the position indicator to
         *	@param dir : defines base position to apply the relative offset to
         *	@param mode :	defines which of the input and/or output sequences to affect
         */
        pos_type seekoff(off_type off, bios::seekdir dir, bios::openmode mode = bios::in | bios::out);

        /**
         *	@brief sets the position indicator of the input and/or output sequence to an absolute position.
         *	@param pos : absolute position to set the position indicator to
         *	@param mode : defines which of the input and/or output sequences to affect
         */
        pos_type seekpos(pos_type pos, bios::openmode mode = bios::in | bios::out);

        /**
         *	@brief get the number of available bits in buffer
         *	@return the number of bits available for non-blocking read
         */
        bitsize in_avail();

        /**
         *	@brief advances the input sequence by ignore bits and reads count bits.
         * 	@param ignore : the number of bits which is due to skip
         *	@param count : the number of bits to read.
         *	@return the bits after ignored bits, converted to int_type on success.
         */
        int_type snextb(bitsize ignore, bitsize count = 1);

        /**
         *	@brief advances the input sequence by ignore bits and reads count bits to dst.
         *	@param ignore : the number of bits which is due to skip
         * 	@param dst : pointer to the beginning of a char array which will store the result.
         *	@param count : the number of bits to read.
         *	@return the number of bits successfully read after ignored bits
         */
        template <byte_output_iterator _Iter>
        bitsize snextn(bitsize ignore, _Iter dst, bitsize count);

        /**
         *	@brief skips count bits of one character in input sequence.
         *	@param value : character to skip
         *	@param count : the number of bits to match.
         *	@return the number of bits successfully skip
         */
        template <byte_type _Byte>
        bitsize sskipb(_Byte value, bitsize count);

        /**
         *	@brief reads count bits and advances the input sequence by count bits.
         *	@param count : the number of bits to read.
         *	@return the value of bits successfully extracted, converted to int_type on success.
         */
        int_type sbumpb(bitsize count = 1);

        /**
         *	@brief reads count bits and advances the input sequence by count bits.
         * 	@param dst : pointer to the beginning of a char array which will store the result.
         *	@param count : the number of bits to read.
         *	@return the number of bits successfully extracted.
         */
        template <byte_output_iterator _Iter>
        bitsize sbumpn(_Iter dst, bitsize count);

        /**
         *	@brief reads count bits from the input sequence.
         *	@param count : the number of bits to read.
         *	@return the value of  bits successfully read, converted to int_type on success.
         */
        int_type sgetb(bitsize count = 1);

        /**
         *	@brief reads count bits from the input sequence and stores them into a character array pointed to by dst
         *	@param dst : pointer to the beginning of a char array which will store the result.
         *	@param count : the number of bits to read.
         *	@return the number of bits successfully read
         */
        template <byte_output_iterator _Iter>
        bitsize sgetn(_Iter dst, bitsize count);

        /**
         *	@brief read count bits to the other bitbuf.
         *	@param dst : another bitbuf which will store the result.
         *	@param count : the number of bits to read.
         *	@return the number of bits successfully read.
         */
        bitsize sgetbuf(_Self& dst, bitsize count);

        /**
         *	@brief writes count bits of one character to the output sequence.
         *	@param value : character to write
         *	@param count : the number of bits to write.
         *	@return the written value of bits, converted to int_type on success.
         */
        template <byte_type _Byte>
        int_type sputb(_Byte value, bitsize count = BYTE_BIT);

        /**
         *	@brief writes count bits to the output sequence from the character array whose first value is pointed to by src
         *	@param src : pointer to the beginning of a char array where data from.
         *	@param count : the number of bits to write.
         *	@return the number of bits successfully written.
         */
        template <byte_input_iterator _Iter>
        bitsize sputn(_Iter src, bitsize count);

        /**
         *	@brief puts the count bits of character value back to the input stream so the next extracted character will be value
         * 	@param value : character to write
         *	@param count : the number of bits to write.
         *	@return the bits of character which are putback successfully, converted to int_type.
         */
        template <byte_type _Byte>
        int_type sputbackb(_Byte value, bitsize count = 1);

        /**
         *	@brief decrements the next pointer
         *	@param count : the number of bits to backforward.
         *	@return the bits which are backforwad successfully, converted to int_type.
         */
        int_type sungetb(bitsize count = 1);

        /**
         *	@brief decrements the next pointer
         *	@param count : the number of bits to backforward.
         *	@return the maximum of bits to backforward.
         */
        bitsize sungetn(bitsize count);

        /**
         *	@brief replaces or obtains a copy of the associated with the internal basic_bitstring
         *   @return a basic_bitstring object holding the replacement character sequence
         */
        /* XSTL_NODISCARD _Bstr str() const& {
            _Bstr _bstr(_alloc);
            auto& _vec = _Bstr_accessor::get_vec(_bstr);
            return _bstr;
        }*/

        void swap(_Self& rhs) noexcept;

        _Self& operator=(const _Self&) = delete;
        _Self& operator                =(_Self&& rhs) {
            this->swap(std::move(rhs));
            return *this;
        }

        ~basic_bitbuf() {
            if (_state & ALLOCATED)
                _alloc.deallocate((byte* const)_buf, _bufsz);
        }

    private:
        static int _Getstate(bios::openmode mode) {
            int _curstate = 0;
            if (mode & bios::in)
                _curstate |= READABLE;
            if (mode & bios::out)
                _curstate |= WRITABLE;
            if (mode & bios::app)
                _curstate |= APPEND;
            if (mode & bios::ate)
                _curstate |= ATEND;
            return _curstate;
        }

        inline allocator_type& get_allocator() { return _alloc; }  // for bitstream
        inline bitsize         rewind_avail() {                    // for rewind operation
            if (std::cmp_less(_usedbits, _nextp * BYTE_BIT + _offp))
                _usedbits = _nextp * BYTE_BIT + _offp;
            return _usedbits - _countg;
        }

        int_type overflow(byte value, bitsize count);
        int_type underflow() { return EOF; }
        int_type uflow() { return EOF; }
        int_type pbackfail() { return EOF; }

        inline void gbump(const bitsize count); /* fix up get pointer */
        inline void pbump(const bitsize count); /* fix up put pointer */

        enum { MINSIZE = 16 };  // min of the first allocation

        bitsize _offp = 0;  // offset for the current block
        bitsize _offg = 0;  // offset for the current block

        bitsize _countp = 0;  //_countp == _bufsz * BYTE_BIT - (_nextp * BYTE_BIT + _offp)
        bitsize _countg = 0;  //_countg == _usedbits - (_nextg * BYTE_BIT + _offg)

        bitsize _usedbits = 0;

        std::streamsize _nextp = 0;
        std::streamsize _nextg = 0;
        std::streamsize _bufsz = 0;

        byte*          _buf   = nullptr;
        int            _state = 0;
        allocator_type _alloc{};
    };

    template <class _Alloc>
    template <byte_type _Byte>
    basic_bitbuf<_Alloc>* basic_bitbuf<_Alloc>::setbuf(_Byte* newbuf, pos_type usedpos, std::streamsize newsz) {
        if (_state & ALLOCATED) {
            _alloc.deallocate(_buf, _bufsz);
            _state &= ~ALLOCATED;
        }
        _buf      = reinterpret_cast<byte*>(newbuf);
        _bufsz    = newsz;
        _usedbits = usedpos.first * BYTE_BIT + usedpos.second;
        _nextg = _offg = _nextp = _offp = 0;
        _countp                         = _bufsz - _usedbits;
        _countg                         = _usedbits;
        return this;
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::pos_type basic_bitbuf<_Alloc>::seekoff(off_type off, bios::seekdir dir, bios::openmode mode) {
        bitsize _position = 0;
        if (_usedbits < _nextp * BYTE_BIT + _offp)
            _usedbits = _nextp * BYTE_BIT + _offp;
        switch (dir) {
        case bios::beg:
            break;
        case bios::end:
            _position = _usedbits;
            break;
        case bios::cur: {
            constexpr auto _both = bios::in | bios::out;
            if ((mode & _both) != _both) {
                if (mode & bios::in)
                    _position = _nextg * BYTE_BIT + _offg;
                else if (mode & bios::out)
                    _position = _nextp * BYTE_BIT + _offp;
                else
                    return pos_type(-1, 0);
                break;
            }
        }
        default:
            return pos_type(-1, 0);
        }

        _position += off;
        if (_position >= 0 && _position <= _usedbits) {
            if (mode & bios::in) {
                gbump(_position - _nextg * BYTE_BIT - _offg);
                return pos_type(_nextg, _offg);
            }
            if (mode & bios::out) {
                pbump(_position - _nextp * BYTE_BIT - _offp);
                return pos_type(_nextp, _offp);
            }
        }

        return pos_type(-1, 0);
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::pos_type basic_bitbuf<_Alloc>::seekpos(pos_type pos, bios::openmode mode) {
        if (((mode & bios::in) && pos.first * BYTE_BIT + pos.second > _countg) || ((mode & bios::out) && pos.first * BYTE_BIT + pos.second > _countp))
            return std::make_pair(-1, 0);
        if (_usedbits < _nextp * BYTE_BIT + _offp)
            _usedbits = _nextp * BYTE_BIT + _offp;
        if (mode & bios::in) {
            _nextg  = pos.first + to_block_idx(pos.second);
            _offg   = to_bit_idx(pos.second);
            _countg = _usedbits - (_nextg * BYTE_BIT + _offg);
        }
        if (mode & bios::out) {
            _nextp  = pos.first + to_block_idx(pos.second);
            _offp   = to_bit_idx(pos.second);
            _countp = (_bufsz - _nextp) * BYTE_BIT - _offp;
        }

        return pos;
    }

    template <class _Alloc>
    void basic_bitbuf<_Alloc>::gbump(const bitsize count) {
        _offg += to_bit_idx(count);
        _countg -= count;
        if (std::cmp_greater_equal(_offg, BYTE_BIT))
            _offg -= BYTE_BIT, _nextg++;
        else if (_offg < 0)
            _offg += BYTE_BIT, _nextg--;
        _nextg += to_block_idx(count);
    }

    template <class _Alloc>
    void basic_bitbuf<_Alloc>::pbump(const bitsize count) {
        _offp += to_bit_idx(count);
        _countp -= count;
        if (std::cmp_greater_equal(_offp, BYTE_BIT))
            _offp -= BYTE_BIT, _nextp++;
        else if (_offp < 0)
            _offp += BYTE_BIT, _nextp--;
        _nextp += to_block_idx(count);
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::bitsize basic_bitbuf<_Alloc>::in_avail() {
        return _countg;
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::int_type basic_bitbuf<_Alloc>::sgetb(bitsize count) {
        const auto     _old_countg = _countg;
        const int_type _res        = sbumpb(count);
        gbump(_countg - _old_countg);
        return _res;
    }

    template <class _Alloc>
    template <byte_output_iterator _Iter>
    typename basic_bitbuf<_Alloc>::bitsize basic_bitbuf<_Alloc>::sgetn(_Iter dst, bitsize count) {
        const auto    _old_countg = _countg;
        const bitsize _num        = sbumpn(dst, count);
        gbump(_countg - _old_countg);
        return _num;
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::bitsize basic_bitbuf<_Alloc>::sgetbuf(_Self& src, bitsize count) {
        if (!(_state & READABLE) || _countg == 0)
            return 0;
        bitsize _maxcnt = (std::min)(count, _countg);
        src.sputb(_buf[_nextg] >> _offg, (std::min)(_maxcnt, BYTE_BIT - _offg));
        if (_maxcnt - BYTE_BIT + _offg > 0)
            src.sputn(_buf + _nextg + 1, _maxcnt - BYTE_BIT + _offg);
        gbump(_maxcnt);
        return _maxcnt;
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::int_type basic_bitbuf<_Alloc>::sbumpb(bitsize count) {
        if (!(_state & READABLE))
            return EOF;
#if !defined(_NO_BSTREAM_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETRY_VERIFY_)
        assert(count <= BYTE_BIT && "basic_bitbuf<Alloc>::sbumpb : reads too much bits");
#endif
        if (_countg == 0)
            return underflow();
        byte _res = 0;
        if (_countg < count)
            count = _countg;
        if (std::cmp_greater_equal(BYTE_BIT - _offg, count))
            set_bits(_res, 0, _buf[_nextg], _offg, _offg + count);
        else
            _res = (get_bits(_buf[_nextg], _offg, BYTE_BIT) << _offg) | (get_bits(_buf[_nextg + 1], 0, count - BYTE_BIT + _offg) >> BYTE_BIT - _offg);
        gbump(count);
        return static_cast<int_type>(_res);
    }

    template <class _Alloc>
    template <byte_output_iterator _Iter>
    typename basic_bitbuf<_Alloc>::bitsize basic_bitbuf<_Alloc>::sbumpn(_Iter dst, bitsize count) {
        if (!(_state & READABLE))
            return 0;
        const bitsize _old_countg = _countg;
        bitsize       _offset     = 0;  // for current data block
        while (count > 0 && _countg > 0) {
            bitsize _maxcnt = std::min<bitsize>(std::min<bitsize>(BYTE_BIT - _offset, count), BYTE_BIT - _offg);
            set_bits(*dst, _offset, _buf[_nextg], _offg, _offg + _maxcnt);
            gbump(_maxcnt);
            _offset += _maxcnt;
            if (_offset == BYTE_BIT)
                _offset = 0, ++dst;
            count -= _maxcnt;
        }
        return _old_countg - _countg;
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::int_type basic_bitbuf<_Alloc>::snextb(bitsize ignore, bitsize count) {
        if (!(_state & READABLE))
            return EOF;
#if !defined(_NO_BSTREAM_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETRY_VERIFY_)
        assert(("basic_bitbuf<Alloc>::snextb : reads too much bits", count <= BYTE_BIT));
#endif
        if (_countg < ignore) {
            gbump(_countg);
            return underflow();
        }
        gbump(ignore);
        return sgetb(count);
    }

    template <class _Alloc>
    template <byte_output_iterator _Iter>
    typename basic_bitbuf<_Alloc>::bitsize basic_bitbuf<_Alloc>::snextn(bitsize ignore, _Iter dst, bitsize count) {
        if (!(_state & READABLE))
            return 0;
        if (_countg < ignore) {
            gbump(_countg);
            return underflow();
        }
        gbump(ignore);
        return sgetn(dst, count);
    }

    template <class _Alloc>
    template <byte_type _Byte>
    typename basic_bitbuf<_Alloc>::bitsize basic_bitbuf<_Alloc>::sskipb(_Byte value, bitsize count) {
        if (!(_state & READABLE))
            return 0;
#if !defined(_NO_BSTREAM_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETRY_VERIFY_)
        assert(("basic_bitbuf<Alloc>::sskipb : skip too much bits", count <= BYTE_BIT));
#endif
        bitsize    _skipcnt = 0;
        const byte _subval  = get_bits(value, BYTE_BIT - count, BYTE_BIT) << BYTE_BIT - count;
        for (; _countg > 0; _skipcnt += count) {
            if (BYTE_BIT - _offg >= count) {
                if (get_bits(_buf[_nextg], _offg, _offg + count) << _offg != _subval)
                    break;
            }
            else {
                if (((get_bits(_buf[_nextg], _offg, BYTE_BIT) << _offg) | (get_bits(_buf[_nextg + 1], 0, count - BYTE_BIT + _offg) >> BYTE_BIT - _offg)) != _subval)
                    break;
            }
            gbump(count);
        }
        return _skipcnt;
    }

    template <class _Alloc>
    template <byte_input_iterator _Iter>
    typename basic_bitbuf<_Alloc>::bitsize basic_bitbuf<_Alloc>::sputn(_Iter src, bitsize count) {
        if (!(_state & WRITABLE))
            return 0;
        bitsize _putcnt = 0;
        bitsize _offset = 0;  // for current data block
        while (count > 0) {
            size_t _maxcnt = (std::min)(count, _countp);
            if (_maxcnt > 0) {
                _putcnt += _maxcnt;
                for (bitsize i = _maxcnt; std::cmp_greater_equal(i, BYTE_BIT - _offp); i -= BYTE_BIT, ++src) {
                    set_bits(_buf[_nextp], _offp, *src, 0, BYTE_BIT - _offp);
                    set_bits(_buf[++_nextp], 0, *src, BYTE_BIT - _offp, BYTE_BIT);
                }
                _countp -= static_cast<bitsize>(_maxcnt) - to_bit_idx(_maxcnt);
                set_bits(_buf[_nextp], _offp, *src, 0, to_bit_idx(_maxcnt));
                _offset = BYTE_BIT - _offp;
                count -= _maxcnt;
                pbump(to_bit_idx(_maxcnt));
            }
            else {  // overflow
                bitsize _fitcnt = std::min<bitsize>(BYTE_BIT - _offset, count);
                if (overflow(static_cast<byte>(*src) << _offset, _fitcnt) == EOF)
                    break;
                _putcnt += _fitcnt;
                count -= _fitcnt;
                src++;
            }
        }
        _countg += _putcnt;
        return _putcnt;
    }

    template <class _Alloc>
    template <byte_type _Byte>
    typename basic_bitbuf<_Alloc>::int_type basic_bitbuf<_Alloc>::sputb(_Byte value, bitsize count) {
        if (!(_state & WRITABLE))
            return EOF;
#if !defined(_NO_BSTREAM_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETRY_VERIFY_)
        assert(("basic_bitbuf<Alloc>::sputb : puts too much bits", count <= BYTE_BIT));
#endif
        const bitsize _old_count = count;
        while (count > 0) {
            bitsize _size = count;
            if (_countp > 0) {
                _size = std::min<bitsize>(BYTE_BIT - _offp, count);
                set_bits(_buf[_nextp], _offp, value, 0, _size);
                value <<= _size;
                pbump(_size);
            }
            else
                overflow(static_cast<byte>(value), count);
            count -= _size;
        }
        _countg += _old_count - count;
        return get_bits(value, count, _old_count);
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::int_type basic_bitbuf<_Alloc>::sungetb(bitsize count) {
        if (!(_state & READABLE))
            return EOF;
#if !defined(_NO_BSTREAM_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETRY_VERIFY_)
        assert(("basic_bitbuf<Alloc>::sungetb : backwards too much bits", count <= BYTE_BIT));
#endif
        byte _res = 0;
        while (count > 0) {
            bitsize _maxsz = (std::min)(_offg, count);
            if (_maxsz == 0) {
                if (_nextg == 0)
                    return pbackfail();
                --_nextg, _offg = BYTE_BIT, _maxsz = count;
            }
            set_bits(_res, count - _maxsz, _buf[_nextg], _offg - _maxsz, _offg);
            count -= _maxsz;
            gbump(-_maxsz);
        }
        return _res;
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::bitsize basic_bitbuf<_Alloc>::sungetn(bitsize count) {
        if (!(_state & READABLE))
            return 0;
        bitsize _maxcnt = (std::min)(rewind_avail(), count);
        gbump(-_maxcnt);
        return _maxcnt;
    }

    template <class _Alloc>
    template <byte_type _Byte>
    typename basic_bitbuf<_Alloc>::int_type basic_bitbuf<_Alloc>::sputbackb(_Byte value, bitsize count) {
        if (!(_state & READABLE))
            return EOF;
#if !defined(_NO_BSTREAM_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETRY_VERIFY_)
        assert(("basic_bitbuf<Alloc>::sputbackb : puts back too much bits", count <= BYTE_BIT));
#endif
        const auto _old_count = count;
        while (count > 0) {
            bitsize _size = (std::min)(_offg, count);
            if (_size == 0) {
                if (_nextg == 0)
                    return pbackfail();
                --_nextg, _offg = BYTE_BIT, _size = count;
            }
            set_bits(_buf[_nextg], _offg - _size, value, count - _size, count);
            count -= _size;
            gbump(-_size);
        }
        return get_bits(value, count, _old_count);
    }

    template <class _Alloc>
    typename basic_bitbuf<_Alloc>::int_type basic_bitbuf<_Alloc>::overflow(byte value, bitsize count) {
        std::streamsize _oldsz = _nextp + to_block_idx(_offp + _countp), _newsz;
        if (_oldsz < MINSIZE)
            _newsz = MINSIZE;
        else if (_oldsz < (std::numeric_limits<int>::max)() / 2)
            _newsz = _oldsz << 1;
        else if (_oldsz < (std::numeric_limits<int>::max)())
            _newsz = (std::numeric_limits<int>::max)();
        else
            return EOF;
        byte* _newbuf = _alloc.allocate(_newsz);
        std::char_traits<byte>::copy(_newbuf, _buf, _oldsz);
        if (_state & ALLOCATED)
            _alloc.deallocate(_buf, _oldsz);
        _buf         = _newbuf;
        _bufsz       = _newsz;
        _offp        = to_bit_idx(count);
        _countp      = (_newsz - _oldsz) * BYTE_BIT - count;
        _buf[_nextp] = get_bits(value, 0, count);
        if (count == BYTE_BIT)
            _nextp++;
        _state |= ALLOCATED;
        return static_cast<int_type>(value);
    }

    template <class _Alloc>
    void basic_bitbuf<_Alloc>::swap(_Self& x) noexcept {
        using std::swap;
        if (this != addressof(x)) {
            swap(_nextg, x._nextg);
            swap(_nextp, x._nextp);
            swap(_offg, x._offg);
            swap(_offp, x._offp);
            swap(_countg, x._countg);
            swap(_countp, x._countp);
            swap(_usedbits, x._usedbits);
            swap(_bufsz, x._bufsz);
            swap(_buf, x._buf);
            swap(_state, x._state);
            alloc_pocs(_alloc, x._alloc);
        }
    }

    /**
     *	@class basic_bios
     *	@brief The class basic_bios provides facilities for interfacing with objects.
     */
    template <class _Alloc>
    class basic_bios : public bios {
        using _Base = bios;

    public:
        using bios::biostate;
        using buf_type  = basic_bitbuf<_Alloc>;
        using pos_type  = typename buf_type::pos_type;
        using off_type  = typename buf_type::off_type;
        using char_type = typename buf_type::char_type;
        using int_type  = typename buf_type::int_type;
        using byte      = typename buf_type::byte;

        explicit basic_bios(buf_type* pbuf) : _pbuf(pbuf) { init(pbuf); }
        basic_bios(const basic_bios&) = delete;

        buf_type* rdbuf() const { return _pbuf; }
        buf_type* rdbuf(buf_type* sb) {
            buf_type* _oldbuf = _pbuf;
            _pbuf             = sb;
            clear();
            return _oldbuf;
        }

        explicit           operator bool() const { return !fail(); }
        XSTL_NODISCARD bool operator!() const { return fail(); }

        void        swap(basic_bios& other) noexcept;
        basic_bios& operator=(const basic_bios&) = delete;

        virtual ~basic_bios() noexcept {}

    protected:
        basic_bios(_Base::openmode mode) : _buf(mode), _pbuf(&_buf) { init(&_buf); }
        basic_bios(byte* buffer, const std::streamsize bufsz, _Base::openmode mode) : _buf(buffer, { bufsz, 0 }, bufsz, mode), _pbuf(&_buf) { init(&_buf); }
        basic_bios(basic_bios&& x) : _pbuf(&_buf) {
            init(&_buf);
            this->swap(x);
        }

        void init(buf_type* pbuf);

        buf_type _buf;

    private:
        buf_type* _pbuf;
    };

    template <class _Alloc>
    void basic_bios<_Alloc>::init(buf_type* pbuf) {
        if (std::endian::native == std::endian::big)
            setf(bios::bigendian);
        (void)pbuf;
    }

    template <class _Alloc>
    void basic_bios<_Alloc>::swap(basic_bios& rhs) noexcept {
        using std::swap;
        if (this != std::addressof(rhs)) {
            bios::swap(rhs);
            swap(_pbuf, rhs._pbuf);
            _buf.swap(rhs._buf);
        }
    }

    template <arithmetic_type _Tp>
    struct default_ignore {
        default_ignore() = default;
        default_ignore(const _Tp& value) : _value(value), _has_value(true) {}

    private:
        _Tp  _value;
        bool _has_value = false;
    };

    /**
     *	@class basic_ibitstream
     *	@brief The class basic_ibitstream provides support for high level input operations on bit stream.
     */
    template <class _Alloc>
    class basic_ibitstream : virtual public basic_bios<_Alloc> {
        using _Base = basic_bios<_Alloc>;
        using _Self = basic_ibitstream<_Alloc>;
        struct _Default_tag {};
        struct _Ignore_proxy {
            char _value[sizeof(long double)]{};
            char _size = 0;  //[0, sizeof(long double)]
        };

    public:
        using buf_type  = typename _Base::buf_type;
        using off_type  = typename _Base::off_type;
        using pos_type  = typename _Base::pos_type;
        using bitsize   = typename _Base::bitsize;
        using char_type = typename _Base::char_type;
        using int_type  = typename _Base::int_type;
        using byte      = typename _Base::byte;

        class sentry {
        public:
            template <class _Ignore = _Default_tag>
            explicit sentry(_Self& ibs, const _Ignore& ignore = _Ignore()) : _ibs(ibs) {
                if (!_ibs.good())
                    _ibs.setstate(bios::failbit);
                else {
                    if constexpr (!std::is_same_v<_Ignore, _Default_tag>) {
                        constexpr bitsize _ignbits = TYPE_BIT(_Ignore);
                        char              _curr[sizeof(_Ignore)]{};
                        for (bitsize _curcnt; ibs.rdbuf()->in_avail() >= _ignbits;) {
                            _curcnt = ibs._Gettp(reinterpret_cast<_Ignore&>(_curr));
                            if (reinterpret_cast<_Ignore&>(_curr) != ignore) {
                                _ibs.rdbuf()->gbump(-_curcnt);  // friend using
                                break;
                            }
                        }
                    }
                    if (ibs._ign._size != 0) {
                        const bitsize _ignbits = static_cast<bitsize>(ibs._ign._size) * BYTE_BIT;
                        auto          _curr    = std::allocate_shared<byte[]>(ibs.rdbuf()->get_allocator(), ibs._ign._size);
                        for (bitsize _curcnt; ibs.rdbuf()->in_avail() >= _ignbits;) {
                            _curcnt = ibs._Gettp(*_curr.get(), _ignbits);
                            if (memcmp(_curr.get(), ibs._ign._value, ibs._ign._size)) {
                                _ibs.rdbuf()->gbump(-_curcnt);  // friend using
                                break;
                            }
                        }
                    }
                    _ok = true;
                }
            }
            explicit operator bool() const { return _ok; }

            sentry(const sentry&) = delete;
            sentry& operator=(const sentry&) = delete;

        private:
            bool   _ok = false;
            _Self& _ibs;
        };

        /**
         *	@brief Constructs new underlying bits device with the default(in) open mode
         */
        basic_ibitstream() : basic_ibitstream(bios::in) {}
        /**
         *	@brief Constructs new underlying bits device. The underlying basic_bitbuf object is constructed as basic_bitbuf<Allocator>(mode | bios_base::in)
         *	@param mode : specifies stream open mode.
         */
        explicit basic_ibitstream(typename _Base::openmode mode) : _Base(mode | bios::in) {}
        /**
         *	@brief Constructs the basic_ibitstream object, assigning initial values to the base class. The value of gcount() is initialized to zero.
         *	@param pbuf : pointer to basic_bitbuf to use as underlying device
         */
        explicit basic_ibitstream(buf_type* pbuf) : _Base(pbuf) {}
        /**
         *	@brief Constructs the basic_ibitstream object with external buffer.
         *	@param buffer : a successive memory to initialize bitbuf.
         *	@param bufsz : the size of buffer.
         *	@param mode : specifies stream open mode.
         */
        template <standard_layout_type _Buffer>
        basic_ibitstream(const _Buffer& buffer, const std::streamsize bufsz, typename _Base::openmode mode = bios::in)
            : _Base(reinterpret_cast<byte*>(const_cast<_Buffer*>(std::addressof(buffer))), bufsz, mode | bios::in) {}

        basic_ibitstream(const basic_ibitstream&) = delete;
        basic_ibitstream(basic_ibitstream&& x) : _Base(bios::in) { this->swap(std::move(x)); }

        /**
         *	@brief Reads a bit and returns it.
         *	@return the extracted bit wrapped by optional<bool>
         */
        std::optional<bool> get();
        /**
         *	@brief Reads count bits to T value and returns them.
         *	@param count : the number of bits to extract
         *	@param ignore : skip value before read
         *	@return the extracted T value wrapped by optional<T>
         */
        template <arithmetic_type _Tp, class _Ignore = _Default_tag>
        std::optional<_Tp> get(bitsize count = TYPE_BIT(_Tp), const _Ignore& ignore = _Ignore());

        /**
         *	@brief Reads a bit and stores it.
         *	@param dst : reference to the bit to write the result to.
         *	@return *this
         *	@exception will throw std::bad_optional_access if fails to read
         */
        _Self& get(bool& dst);
        /**
         *	@brief Reads count bits to T value and stores them.
         *	@param dst : reference to the value to write the result to.
         *	@param count : the number of bits to extract
         *	@param ignore : skip value before read
         *	@return *this
         *	@exception will throw std::bad_optional_access if fails to read
         */
        template <arithmetic_type _Tp, class _Ignore = _Default_tag>
        _Self& get(_Tp& dst, bitsize count = TYPE_BIT(_Tp), const _Ignore& ignore = _Ignore());
        /**
         *	@brief Reads count bits to a bool sequence until delimiter is found
         *	@param dst : the first iterator of bool sequence to store the bits
         *	@param count : the number of bits to read
         *   @param delim : delimiting value to stop the extraction at. It is not extracted and not stored.
         *	@param ignore : skip value before read
         *	@return *this
         */
        template <bool_iterator _Iter, class _Delim, class _Ignore = _Default_tag>
        _Self& get(_Iter dst, bitsize count, const _Delim& delim, const _Ignore& ignore = _Ignore());  // bm needed
        /**
         *	@brief Reads count bits to a T sequence until delimiter is found
         *	@param dst : the first iterator of bool sequence to store the bits
         *	@param num : the number of bits of T to read.
         *	@param delim : delimiting value to stop the extraction at. It is not extracted and not stored. '\n' as default
         * 	@param each_count : the number of bits that each T value to extract
         *	@param ignore : skip value before read
         *	@return *this
         */
        template <arithmetic_iterator _Iter, class _Delim, class _Ignore = _Default_tag>
        _Self& get(_Iter dst, const std::streamsize num, const _Delim& delim, size_t each_count = TYPE_BIT(std::iter_value_t<_Iter>), const _Ignore& ignore = _Ignore());
        template <arithmetic_iterator _Iter, class _Elem = char, class _Ignore = _Default_tag>
        _Self& get(_Iter dst, const std::streamsize num, size_t each_count = TYPE_BIT(std::iter_value_t<_Iter>), const _Ignore& ignore = _Ignore());
        /**
         *	@brief reads available bits and inserts them to the given basic_bitbuf object
         *	@param buf : bit buffer to read the content to
         *	@param count : the number of bits to extract
         *	@return *this
         */
        _Self& get(buf_type& buf, bitsize count);
        /**
         *	@brief reads bits and inserts them to the output sequence controlled by the given basic_bitbuf object until delim is found.
         *	@param buf : bit buffer to read the content to
         *	@param count : the number of bits to extract
         *	@param delim : delimiting value to stop the extraction at. It is not extracted and not stored.
         *	@return *this
         */
        template <class _Delim>
        _Self& get(buf_type& buf, bitsize count, const _Delim& delim);  // bm needed

        /**
         *	@brief Reads a bit without extracting and returns it.
         *	@return the read bit wrapped by optional<bool>
         */
        std::optional<bool> peek();
        /**
         *	@brief Reads count bits to T value without extracting and returns it.
         *	@param count : the number of bits to extract
         *	@return the read bits wrapped by optional<T>
         */
        template <arithmetic_type _Tp>
        std::optional<_Tp> peek(bitsize count = TYPE_BIT(_Tp));

        /**
         *	@brief extracts count bits to a bool sequence
         *	@param dst : the first iterator of bool sequence to store the bits
         *	@param count : the number of bits to extract
         *	@param ignore : skip value before read
         *	@return *this
         */
        template <bool_iterator _Iter, class _Ignore = _Default_tag>
        _Self& read(_Iter dst, const bitsize count, const _Ignore& ignore = _Ignore());
        /**
         *	@brief extracts num * sizeof(T) bits to a T sequence
         *	@param dst : the first iterator of T sequence to store the bits
         *	@param num : the number of T to read
         *	@param each_count : the number of bits that each T value to extract
         *	@param ignore : skip value before read
         *	@return *this
         */
        template <arithmetic_iterator _Iter, class _Ignore = _Default_tag>
        _Self& read(_Iter dst, const std::streamsize num, size_t each_count = TYPE_BIT(std::iter_value_t<_Iter>), const _Ignore& ignore = _Ignore());
        /**
         *	@brief extracts up to count bits to a bool sequence
         *	@param dst : the first iterator of bool sequence to store the bits
         *	@param count : the maximum number of bits to extract
         *	@param ignore : skip value before read
         *	@return the number of bits actually extracted.
         */
        template <bool_iterator _Iter, class _Ignore = _Default_tag>
        bitsize readsome(_Iter dst, const bitsize count, const _Ignore& ignore = _Ignore());
        /**
         *	@brief extracts up to num * sizeof(T) bits to a T sequence
         *	@param dst : the first iterator of T sequence to store the bits
         *	@param num : the maximum number of T to extract
         *	@param each_count : the number of bits that each T value to extract
         *	@param ignore : skip value before read
         *	@return the number of T actually extracted.
         */
        template <arithmetic_iterator _Iter, class _Ignore = _Default_tag>
        std::streamsize readsome(_Iter dst, const std::streamsize num, size_t each_count = TYPE_BIT(std::iter_value_t<_Iter>), const _Ignore& ignore = _Ignore());

        /**
         *	@brief Extracts bits from stream until end of line or the specified delimiter.
         *	@param dst : the first iterator of T sequence to store the bits
         *	@param num : the number of T to extract
         *	@param delim : delimiting value to stop the extraction at. It is extracted and not stored.
         *	@param each_count : the number of bits that each T value to extract
         *	@param ignore : skip value before read
         *	@return *this
         */
        template <arithmetic_iterator _Iter, class _Elem = char, class _Ignore = _Default_tag>
        _Self& getline(_Iter dst, const std::streamsize num, size_t each_count = TYPE_BIT(std::iter_value_t<_Iter>), const _Ignore& ignore = _Ignore());
        template <arithmetic_iterator _Iter, class _Delim, class _Ignore = _Default_tag>
        _Self& getline(_Iter dst, const std::streamsize num, const _Delim& delim, size_t each_count = TYPE_BIT(std::iter_value_t<_Iter>), const _Ignore& ignore = _Ignore());

        /**
         *	@brief Makes the most recently extracted count bits available again
         *	@param count : the number of bits want to rewind
         *	@return *this
         */
        _Self& unget(bitsize count);
        /**
         *	@brief Puts a bit back to the input stream so the next extracted bit will be value
         *	@param value : bit to put into input stream
         *	@return *this
         */
        _Self& putback(const bool value);
        /**
         *	@brief Puts bits of value back to the input stream so the next extracted bit will be value
         *	@param value : bits to put into input stream
         *	@return *this
         */
        template <arithmetic_type _Tp>
        _Self& putback(const _Tp& value, bitsize count);

        /**
         *	@brief Extracts and discards bits from the input stream until and including delim
         *	@param count : the number of bits to extract
         *	@param delim : delimiting value to stop the extraction at. It is also extracted
         *	@return *this
         */
        template <class _Delim>
        _Self& ignore(bitsize count, const _Delim& delim) {
            _bitcount = 0;
            const sentry _ok(*this);
            if (!_ok)
                return *this;
            size_t _tmpsz  = to_fit_size(count + TYPE_BIT(_Delim));
            auto   _tmpbuf = std::allocate_shared<byte[]>(this->rdbuf()->get_allocator(), _tmpsz);
            this->rdbuf()->sgetn(_tmpbuf, _tmpsz);
            return *this;
        }

        /**
         *	@brief Returns the number of bits extracted by the last unformatted input operation
         *	@return the number of bits extracted by the last unformatted input operation
         */
        bitsize gcount() const { return _bitcount; }

        /**
         *	@brief Returns the input position indicator of the current associated bitbuf object
         *	@return current input position indicator on success, pos_type(-1, 0) if a failure occurs.
         */
        pos_type tellg();

        /**
         *	@brief Sets the input position indicator of the current associated bitbuf object.
         *	@param pos : absolute position to set the input position indicator to.
         *	@return *this
         */
        _Self& seekg(pos_type pos);
        /**
         *	@brief Sets the input position indicator of the current associated bitbuf object.
         *	@param off : relative position(positive or negative) to set the input position indicator to.
         * 	@param dir : defineds base position to apply the relative offset to.
         *	@return *this
         */
        _Self& seekg(off_type off, bios::seekdir dir);

        bios::bfmtflags flags(bios::bfmtflags flags) override {
            if ((bios::flags() & bios::bigendian) ^ (flags & bios::bigendian))
                std::reverse(_ign._value, _ign._value + _ign._size);
            return bios::flags(flags);
        }
        bios::bfmtflags setf(bios::bfmtflags flags) override {
            if ((bios::flags() & bios::bigendian) ^ (flags & bios::bigendian))
                std::reverse(_ign._value, _ign._value + _ign._size);
            return bios::setf(flags);
        }
        void unsetf(bios::bfmtflags flags) override {
            if ((bios::flags() & bios::bigendian) ^ (flags & bios::bigendian))
                std::reverse(_ign._value, _ign._value + _ign._size);
            return bios::unsetf(flags);
        }

        void swap(_Self& rhs) noexcept;

        _Self& operator=(const basic_ibitstream&) = delete;
        _Self& operator                           =(basic_ibitstream&& rhs) {
            swap(std::move(rhs));
            return *this;
        }

        _Self& set_default_ignore();
        template <arithmetic_type _Tp>
        _Self& set_default_ignore(const _Tp& value);

        _Self& operator>>(_Self& (*pf)(_Self&));
        _Self& operator>>(basic_bios<_Alloc>& (*pf)(basic_bios<_Alloc>&));
        _Self& operator>>(bool& value) { return get(value); }
        template <arithmetic_type _Tp>
        _Self& operator>>(_Tp& value) {
            return get(value, TYPE_BIT(_Tp));
        }
        _Self& operator>>(buf_type& dst);
        template <arithmetic_type _Tp>
        _Self& operator>>(default_ignore<_Tp>&& value);
        /*template <class _Elem, class _Traits>
        _Self& operator>>(std::basic_ostream<_Elem, _Traits>& os) {

        }*/

        virtual ~basic_ibitstream() noexcept {}

    private:
        template <class _Tp>
        bitsize _Gettp(_Tp& res, bitsize count = TYPE_BIT(_Tp));  // count <= TYPE_BIT(_Tp)
        template <arithmetic_iterator _Iter, class _Delim, class _Ignore>
        _Self& _Getline(_Iter dst, const std::streamsize num, const _Delim& delim, size_t each_count, const _Ignore& ignore, bool is_getline);  // true for getline and false for get

        bitsize       _bitcount = 0;
        _Ignore_proxy _ign;
    };

    template <class _Alloc>
    template <class _Tp>
    typename basic_ibitstream<_Alloc>::bitsize basic_ibitstream<_Alloc>::_Gettp(_Tp& res, bitsize count) {
        const bitsize _old_avail = _Base::rdbuf()->in_avail();
        if (_Base::flags() & _Base::bigendian) {
            byte* _node;
            for (_node = reinterpret_cast<byte*>(std::addressof(res)) + sizeof(_Tp) - 1; std::cmp_greater(count, BYTE_BIT); count -= BYTE_BIT)
                *_node-- = _Base::rdbuf()->sbumpb(BYTE_BIT);
            *_node = _Base::rdbuf()->sbumpb(count);
            return _old_avail - _Base::rdbuf()->in_avail();
        }
        else  // little endian
            return _Base::rdbuf()->sbumpn(reinterpret_cast<byte*>(std::addressof(res)), count);
    }

    template <class _Alloc>
    template <arithmetic_iterator _Iter, class _Delim, class _Ignore>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::_Getline(_Iter dst, const std::streamsize num, const _Delim& delim, size_t each_count, const _Ignore& ignore, bool is_getline) {
        using val_t = std::iter_value_t<_Iter>;
        _bitcount   = 0;
        const sentry _ok(*this, ignore);
        if (!_ok)
            return *this;
        if (each_count > TYPE_BIT(val_t)) {
            _Base::setstate(bios::failbit);
            each_count = TYPE_BIT(val_t);
        }
        constexpr bitsize _delimbits = TYPE_BIT(_Delim);
        const size_t      _nbyte     = to_block_idx(static_cast<bios::bitsize>(each_count) + BYTE_BIT - 1);
        auto              _curres    = std::allocate_shared<byte[]>(_Base::rdbuf()->get_allocator(), (std::max)(sizeof(_Delim), _nbyte));
        for (; (_bitcount < num * each_count || (is_getline ? (_Base::setstate(bios::failbit), 0) : 0))
               && (_Base::rdbuf()->in_avail() >= each_count || (_Base::setstate(bios::eofbit | bios::failbit), 0));
             ++dst, _bitcount += each_count) {
            if constexpr (is_any_of_v<std::iter_value_t<_Iter>, char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t>) {
                if (_bitcount >= (num - 1) * each_count) {
                    std::char_traits<val_t>::assign(*dst, val_t());  // if is char types, add terminating null character
                    break;
                }
            }
            if (_Base::rdbuf()->in_avail() >= _delimbits) {
                _Gettp(*_curres.get(), _delimbits);
                if (reinterpret_cast<_Delim&>(*_curres.get()) == delim) {  // got a delimiter, quit
                    if (is_getline)
                        _bitcount += _delimbits;  // release delimiter
                    else
                        _Base::rdbuf()->gbump(-_delimbits);  // friend using, if is get(), don't release
                    break;
                }
                else {
                    if (_delimbits >= each_count) {
                        if (to_bit_idx(each_count))
                            reinterpret_cast<std::make_unsigned_t<byte>&>(_curres[_Base::flags() & _Base::bigendian ? sizeof(_Delim) - _nbyte : _nbyte - 1]) >>= BYTE_BIT - to_bit_idx(each_count);
                        _Base::rdbuf()->gbump(each_count - _delimbits);  // friend using
                    }
                    else {  // each_count > delimbits
                        if (_Base::flags() & _Base::bigendian) {
                            byte*   _node  = _curres.get() + _nbyte - sizeof(_Delim) - 1;
                            bitsize _count = each_count;
                            for (; std::cmp_greater_equal(_count, BYTE_BIT); _count -= BYTE_BIT)
                                *_node-- = _Base::rdbuf()->sbumpb(BYTE_BIT);
                            if (_count > 0)
                                *_node = _Base::rdbuf()->sbumpb(_count);
                        }
                        else  // little endian
                            _Base::rdbuf()->sbumpn(_curres.get() + sizeof(_Delim), each_count - _delimbits);
                    }
                    if ((_Base::flags() & _Base::bigendian) ^ (std::endian::native == std::endian::big))
                        std::reverse(_curres.get(), _curres.get() + (std::max)(sizeof(_Delim), _nbyte));
                    *dst = reinterpret_cast<val_t&>(*_curres.get());
                }
            }
            else
                _Gettp(*dst, each_count);
        }
        return *this;
    }

    template <class _Alloc>
    std::optional<bool> basic_ibitstream<_Alloc>::get() {
        _bitcount = 0;
        const sentry _ok(*this);
        if (!_ok)
            return {};
        int_type _res = _Base::rdbuf()->sbumpb(1);
        if (_res == -1) {
            _Base::setstate(bios::failbit | bios::eofbit);
            return {};
        }
        return static_cast<bool>(_res);
    }

    template <class _Alloc>
    template <arithmetic_type _Tp, class _Ignore>
    std::optional<_Tp> basic_ibitstream<_Alloc>::get(bitsize count, const _Ignore& ignore) {
        _bitcount = 0;
        const sentry _ok(*this, ignore);
        if (!_ok || count <= 0 || (_Base::rdbuf()->in_avail() == 0 && (_Base::setstate(bios::eofbit | bios::failbit), 1)))
            return {};
        if (count > TYPE_BIT(_Tp)) {
            _Base::setstate(bios::failbit);
            count = TYPE_BIT(_Tp);
        }
        _Tp _res{};
        _bitcount = _Gettp(_res, count);
        if (_bitcount < count)
            _Base::setstate(bios::eofbit | bios::failbit);
        return _res;
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::get(bool& dst) {
        dst = std::move(get().value());
        return *this;
    }

    template <class _Alloc>
    template <arithmetic_type _Tp, class _Ignore>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::get(_Tp& dst, bitsize count, const _Ignore& ignore) {
        dst = std::move(get<_Tp>(count, ignore).value());
        return *this;
    }

    // template <class _Alloc>
    // template <bool_iterator _Iter, class _Delim, class _Ignore>
    // basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::get(_Iter dst, bitsize count, const _Delim& delim, const _Ignore& ignore) {
    //	_bitcount = 0;
    //	const sentry _ok(*this, ignore);
    //	//not finish
    //	return *this;
    // }

    template <class _Alloc>
    template <arithmetic_iterator _Iter, class _Delim, class _Ignore>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::get(_Iter dst, const std::streamsize num, const _Delim& delim, size_t each_count, const _Ignore& ignore) {
        return _Getline(dst, num, delim, each_count, ignore, false);
    }

    template <class _Alloc>
    template <arithmetic_iterator _Iter, class _Elem, class _Ignore>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::get(_Iter dst, const std::streamsize num, size_t each_count, const _Ignore& ignore) {
        return get(dst, num, std::use_facet<std::ctype<_Elem>>(std::locale()).widen('\n'), each_count, ignore);
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::get(buf_type& buf, bitsize count) {
        _bitcount = 0;
        const sentry _ok(*this);
        if (!_ok || count <= 0)
            return *this;
        _bitcount = _Base::rdbuf()->sgetbuf(buf, count);
        if (_bitcount < count)
            _Base::setstate(bios::eofbit | bios::failbit);
        return *this;
    }

    /*template <class _Alloc>
    template <class _Delim>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::get(buf_type&, bitsize count, const _Delim& delim) {
        _bitcount = 0;
        const sentry _ok(*this);
        if (!_ok)
            return *this;

        return *this;
    }*/

    template <class _Alloc>
    template <arithmetic_iterator _Iter, class _Elem, class _Ignore>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::getline(_Iter dst, const std::streamsize num, const size_t each_count, const _Ignore& ignore) {
        return getline(dst, num, std::use_facet<std::ctype<_Elem>>(std::locale()).widen('\n'), each_count, ignore);
    }

    template <class _Alloc>
    template <arithmetic_iterator _Iter, class _Delim, class _Ignore>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::getline(_Iter dst, const std::streamsize num, const _Delim& delim, const size_t each_count, const _Ignore& ignore) {
        return _Getline(dst, num, delim, each_count, ignore, true);
    }

    template <class _Alloc>
    template <bool_iterator _Iter, class _Ignore>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::read(_Iter dst, const bitsize count, const _Ignore& ignore) {
        _bitcount = 0;
        const sentry _ok(*this, ignore);
        if (!_ok)
            return *this;
        while (_bitcount < count) {
            int_type _curres = _Base::rdbuf()->sbumpb(1);
            if (_curres != EOF) {
                *dst++ = static_cast<bool>(_curres);
                ++_bitcount;
            }
            else {
                _Base::setstate(bios::failbit | bios::eofbit);
                break;
            }
        }
        return *this;
    }

    template <class _Alloc>
    template <arithmetic_iterator _Iter, class _Ignore>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::read(_Iter dst, const std::streamsize num, size_t each_count, const _Ignore& ignore) {
        using val_t = std::iter_value_t<_Iter>;
        _bitcount   = 0;
        const sentry _ok(*this, ignore);
        if (_ok) {
            if (each_count > TYPE_BIT(val_t)) {
                _Base::setstate(bios::failbit);
                each_count = TYPE_BIT(val_t);
            }
            for (; _bitcount < num * each_count && (_Base::rdbuf()->in_avail() >= each_count || (_Base::setstate(bios::eofbit | bios::failbit), 0)); ++dst)
                _bitcount += _Gettp(*dst, each_count);
        }
        return *this;
    }

    template <class _Alloc>
    template <bool_iterator _Iter, class _Ignore>
    typename basic_ibitstream<_Alloc>::bitsize basic_ibitstream<_Alloc>::readsome(_Iter dst, const bitsize count, const _Ignore& ignore) {
        _bitcount = 0;
        const sentry _ok(*this, ignore);
        if (_ok) {
            bitsize _maxcnt = count < _Base::rdbuf()->in_avail() ? count : (_Base::setstate(bios::eofbit), _Base::rdbuf()->in_avail());  // the max bits can get in buffer
            for (; _bitcount < _maxcnt; ++_bitcount, ++dst)
                *dst = _Base::rdbuf()->sbumpb(1);
        }
        return _bitcount;
    }

    template <class _Alloc>
    template <arithmetic_iterator _Iter, class _Ignore>
    std::streamsize basic_ibitstream<_Alloc>::readsome(_Iter dst, const std::streamsize num, size_t each_count, const _Ignore& ignore) {
        using val_t = std::iter_value_t<_Iter>;
        _bitcount   = 0;
        const sentry _ok(*this, ignore);
        if (_ok) {
            if (each_count > TYPE_BIT(val_t)) {
                _Base::setstate(bios::failbit);
                each_count = TYPE_BIT(val_t);
            }
            std::streamsize _maxnum =
                num < _Base::rdbuf()->in_avail() / each_count ? num : (_Base::setstate(bios::eofbit), _Base::rdbuf()->in_avail() / each_count);  // the max number can get in buffer
            for (; _bitcount < _maxnum * each_count; ++dst)
                _bitcount += _Gettp(*dst);
        }
        return _bitcount;
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::unget(bitsize count) {
        _bitcount = 0;
        _Base::clear(_Base::rdstate() & ~bios::eofbit);
        const sentry _ok(*this);
        if (_ok && count > 0 && _Base::rdbuf()->sungetn(count) != count)
            _Base::setstate(bios::badbit);
        return *this;
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::putback(const bool value) {
        _bitcount = 0;
        _Base::clear(_Base::rdstate() & ~bios::eofbit);
        const sentry _ok(*this);
        if (_ok && _Base::rdbuf()->sputbackb(value, 1) == EOF)
            _Base::setstate(bios::badbit);
        return *this;
    }

    template <class _Alloc>
    template <arithmetic_type _Tp>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::putback(const _Tp& value, bitsize count) {
        _bitcount = 0;
        _Base::clear(_Base::rdstate() & ~bios::eofbit);
        const sentry _ok(*this);
        if (!_ok || (count > _Base::rdbuf()->rewind_avail() && (_Base::setstate(bios::badbit), 1)))  // friend using
            return *this;
        const byte* _node;
        if (count > TYPE_BIT(_Tp)) {
            _Base::setstate(bios::failbit);
            count = TYPE_BIT(_Tp);
        }
        if (_Base::flags() & bios::bigendian) {
            for (_node = reinterpret_cast<const byte*>(std::addressof(value)); std::cmp_greater_equal(count, BYTE_BIT); count -= BYTE_BIT)
                _Base::rdbuf()->sputbackb(*_node++, BYTE_BIT);
        }
        else {  // little endian
            for (_node = reinterpret_cast<const byte*>(std::addressof(value)) + sizeof(_Tp) - 1; std::cmp_greater_equal(count, BYTE_BIT); count -= BYTE_BIT)
                _Base::rdbuf()->sputbackb(*_node--, BYTE_BIT);
        }
        _Base::rdbuf()->sputbackb(*_node, count);
        return *this;
    }

    template <class _Alloc>
    std::optional<bool> basic_ibitstream<_Alloc>::peek() {
        _bitcount = 0;
        const sentry _ok(*this);
        if (!_ok)
            return {};
        int_type _res = _Base::rdbuf()->sgetb(1);
        if (_res == -1) {
            _Base::setstate(bios::eofbit);
            return {};
        }
        return static_cast<bool>(_res);
    }

    template <class _Alloc>
    template <arithmetic_type _Tp>
    std::optional<_Tp> basic_ibitstream<_Alloc>::peek(bitsize count) {
        _bitcount = 0;
        const sentry _ok(*this);
        if (!_ok || count <= 0)
            return {};
        _Tp _res{};
        if (count > TYPE_BIT(_Tp)) {
            _Base::setstate(bios::failbit);
            count = TYPE_BIT(_Tp);
        }
        const bitsize _curcnt = _Gettp(_res, count);
        if (_curcnt < count)
            _Base::setstate(bios::eofbit);
        _Base::rdbuf()->gbump(-_curcnt);
        return _res;
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::seekg(pos_type pos) {
        _Base::clear(_Base::rdstate() & ~bios::eofbit);
        if (!this->fail() && _Base::rdbuf()->seekpos(pos, bios::in).first == -1)
            _Base::setstate(bios::failbit);
        return *this;
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::seekg(off_type off, bios::seekdir dir) {
        _Base::clear(_Base::rdstate() & ~bios::eofbit);
        if (!this->fail() && _Base::rdbuf()->seekoff(off, dir, bios::in).first == -1)
            _Base::setstate(bios::failbit);
        return *this;
    }

    template <class _Alloc>
    typename basic_ibitstream<_Alloc>::pos_type basic_ibitstream<_Alloc>::tellg() {
        return this->fail() ? pos_type(-1, 0) : _Base::rdbuf()->seekoff(0, bios::cur, bios::in);
    }

    template <class _Alloc>
    void basic_ibitstream<_Alloc>::swap(_Self& rhs) noexcept {
        using std::swap;
        if (this != std::addressof(rhs)) {
            _Base::swap(rhs);
            swap(_ign, rhs._ign);
            swap(_bitcount, rhs._bitcount);
        }
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::set_default_ignore() {
        _ign._size = 0;
        return *this;
    }

    template <class _Alloc>
    template <arithmetic_type _Tp>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::set_default_ignore(const _Tp& value) {
        _ign._size                          = sizeof(_Tp);
        reinterpret_cast<_Tp&>(_ign._value) = value;
        return *this;
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::operator>>(_Self& (*pf)(_Self&)) {
        return pf(*this);
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::operator>>(basic_bios<_Alloc>& (*pf)(basic_bios<_Alloc>&)) {
        pf(*this);
        return *this;
    }

    template <class _Alloc>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::operator>>(buf_type& dst) {
        return get(dst, _Base::rdbuf()->in_avail());
    }

    template <class _Alloc>
    template <arithmetic_type _Tp>
    basic_ibitstream<_Alloc>& basic_ibitstream<_Alloc>::operator>>(default_ignore<_Tp>&& value) {
        return value._has_value ? set_default_ignore(std::move(value._value)) : set_default_ignore();
    }

    /**
     *	@class basic_obitstream
     *	@brief The class basic_obitstream provides support for high level output operations on bit stream.
     */
    template <class _Alloc>
    class basic_obitstream : virtual public basic_bios<_Alloc> {
        using _Base = basic_bios<_Alloc>;
        using _Self = basic_obitstream<_Alloc>;

    public:
        using buf_type  = typename _Base::buf_type;
        using off_type  = typename _Base::off_type;
        using pos_type  = typename _Base::pos_type;
        using bitsize   = typename _Base::bitsize;
        using char_type = typename _Base::char_type;
        using int_type  = typename _Base::int_type;
        using byte      = typename _Base::byte;

        class sentry {
        public:
            explicit sentry(_Self& obs) : _obs(obs) {
                if (_obs.good())
                    _ok = true;
            }

            explicit operator bool() const { return _ok; }

            sentry(const sentry&) = delete;
            sentry& operator=(const sentry&) = delete;

        private:
            bool   _ok = false;
            _Self& _obs;
        };

        /**
         *	@brief Constructs new underlying bits device with the default(out) open mode
         */
        basic_obitstream() : basic_obitstream(bios::out) {}
        /**
         *	@brief Constructs new underlying bits device. The underlying basic_bitbuf object is constructed as basic_bitbuf<Allocator>(mode | bios_base:out
         *	@param mode : specifies stream open mode.
         */
        explicit basic_obitstream(typename _Base::openmode mode) : _Base(mode | bios::out) {}
        /**
         *	@brief Constructs the basic_obitstream object, assigning initial values to the base class.
         *	@param pbuf : pointer to bitsbuf to use as underlying device
         */
        basic_obitstream(buf_type* pbuf) : _Base(pbuf) {}
        /**
         *	@brief Constructs the basic_obitstream object with external buffer.
         *	@param buffer : a successive memory to initialize bitbuf.
         *	@param bufsz : the size of buffer.
         *	@param mode : specifies stream open mode.
         */
        template <standard_layout_type _Buffer>
        basic_obitstream(const _Buffer& buffer, const std::streamsize bufsz, typename _Base::openmode mode = bios::out)
            : _Base(reinterpret_cast<byte*>(const_cast<_Buffer*>(std::addressof(buffer))), bufsz, mode | bios::out) {}

        basic_obitstream(const _Self& x) = delete;
        basic_obitstream(_Self&& x) : _Base(bios::out) { this->swap(std::move(x)); }

        /**
         *	@brief writes the count bits to src
         *	@param src : bits to write
         *	@param count : the number of bits to write
         *	@return *this
         */
        _Self& put(bool src);
        template <arithmetic_type _Tp>
        _Self& put(const _Tp& dst, bitsize count);
        /**
         *	@brief writes the bits to a place whose first value is pointed to by first.
         *	@param first : iterator which points to the first value to write
         *	@param count : the number of bits to write
         *	@return *this
         */
        template <bool_iterator _Iter>
        _Self& write(_Iter first, bitsize count);
        /**
         *	@brief writes the bits to a place whose first value is pointed to by first.
         *	@param first : iterator which points to the first value to write
         *	@param num : the number of iter::value_type to write
         *	@param each_count : the number of bits that each T value to put
         *	@return *this
         */
        template <arithmetic_iterator _Iter>
        _Self& write(_Iter first, std::streamsize num, size_t each_count = TYPE_BIT(std::iter_value_t<_Iter>));

        /**
         *	@brief returns the output position indicator of the current associated bitbuf object
         *	@return current output position indicator on success, pos_type(-1, 0) if a failure occurs.
         */
        pos_type tellp();
        /**
         *	@brief Sets the output position indicator of the current associated bitbuf object.
         *	@param pos : absolute position to set the output position indicator to.
         *	@return *this
         */
        _Self& seekp(pos_type pos);
        /**
         *	@brief Sets the output position indicator of the current associated bitbuf object.
         *	@param off : relative position(positive or negative) to set the output position indicator to.
         * 	@param dir : defineds base position to apply the relative offset to.
         *	@return *this
         */
        _Self& seekp(off_type off, bios::seekdir dir);

        void swap(_Self& x) { _Base::swap(x); }

        _Self& operator=(const _Self&) = delete;
        _Self& operator                =(_Self&& rhs) {
            this->swap(std::move(rhs));
            return *this;
        }

        _Self& operator<<(const bool src) { return put(src); }
        template <arithmetic_type _Tp>
        _Self& operator<<(const _Tp& src) {
            return put(src, TYPE_BIT(_Tp));
        }
        template <character_type _Elem>
        _Self& operator<<(const _Elem* str);
        _Self& operator<<(buf_type& buf);
        _Self& operator<<(_Self& (*pf)(_Self&));
        _Self& operator<<(basic_bios<_Alloc>& (*pf)(basic_bios<_Alloc>&));

        ~basic_obitstream() noexcept {}

    private:
        template <class _Tp>
        void _Puttp(_Tp value, bitsize count);
    };

    template <class _Alloc>
    template <class _Tp>
    void basic_obitstream<_Alloc>::_Puttp(_Tp value, bitsize count) {
        if (_Base::flags() & bios::bigendian) {
            const byte* _node;
            for (_node = reinterpret_cast<const byte*>(std::addressof(value)) + sizeof(_Tp) - 1; std::cmp_greater(count, BYTE_BIT); count -= BYTE_BIT)
                if (_Base::rdbuf()->sputb(*_node--, BYTE_BIT) == EOF) {
                    _Base::setstate(bios::badbit);
                    return;
                }
            if (_Base::rdbuf()->sputb(*_node, count) == EOF)
                _Base::setstate(bios::badbit);
        }
        else {  // little endian
            if (_Base::rdbuf()->sputn(reinterpret_cast<const byte*>(std::addressof(value)), count) == 0)
                _Base::setstate(bios::badbit);
        }
    }

    template <class _Alloc>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::put(bool src) {
        const sentry     _ok(*this);
        const bios::byte _wrap = src;
        if (!_ok || _Base::rdbuf()->sputb(_wrap, 1) == EOF)
            _Base::setstate(bios::badbit);
        return *this;
    }

    template <class _Alloc>
    template <arithmetic_type _Tp>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::put(const _Tp& src, bitsize count) {
        const sentry _ok(*this);
        if (!_ok && (_Base::setstate(bios::badbit), 1))
            return *this;
        if (count > TYPE_BIT(_Tp)) {
            _Base::setstate(bios::failbit);
            count = TYPE_BIT(_Tp);
        }
        _Puttp(src, count);
        return *this;
    }

    template <class _Alloc>
    template <bool_iterator _Iter>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::write(_Iter first, bitsize count) {
        const sentry _ok(*this);
        if (_ok || (_Base::setstate(bios::badbit), 0))
            for (; count > 0 && (_Base::rdbuf()->sputb(*first, 1) != EOF || (_Base::setstate(bios::badbit), 0)); --count, ++first)
                ;
        return *this;
    }

    template <class _Alloc>
    template <arithmetic_iterator _Iter>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::write(_Iter first, std::streamsize num, size_t each_count) {
        using val_t = std::iter_value_t<_Iter>;
        const sentry _ok(*this);
        if (_ok || (_Base::setstate(bios::badbit), 0)) {
            if (each_count > TYPE_BIT(val_t)) {
                _Base::setstate(bios::failbit);
                each_count = TYPE_BIT(val_t);
            }
            for (; num-- > 0; ++first)
                _Puttp(*first, each_count);
        }
        return *this;
    }

    template <class _Alloc>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::seekp(pos_type pos) {
        _Base::clear(_Base::rdstate() & ~bios::eofbit);
        if (!this->fail() && _Base::rdbuf()->seekpos(pos, bios::out).first == -1)
            _Base::setstate(bios::failbit);
        return *this;
    }

    template <class _Alloc>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::seekp(off_type off, bios::seekdir dir) {
        _Base::clear(_Base::rdstate() & ~bios::eofbit);
        if (!this->fail() && _Base::rdbuf()->seekoff(off, dir, bios::out).first == -1)
            _Base::setstate(bios::failbit);
        return *this;
    }

    template <class _Alloc>
    typename basic_obitstream<_Alloc>::pos_type basic_obitstream<_Alloc>::tellp() {
        return this->fail() ? pos_type(-1, 0) : _Base::rdbuf()->seekoff(0, bios::cur, bios::out);
    }

    template <class _Alloc>
    template <character_type _Elem>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::operator<<(const _Elem* str) {
        const sentry _ok(*this);
        if (!_ok && (_Base::setstate(bios::badbit), 1))
            return *this;
        for (int i = 0; str[i] != static_cast<_Elem>('\0'); ++i)
            put(str[i], TYPE_BIT(_Elem));
        return *this;
    }

    template <class _Alloc>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::operator<<(buf_type& buf) {
        return buf.sgetbuf(this->rdbuf());
    }

    template <class _Alloc>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::operator<<(_Self& (*pf)(_Self&)) {
        return pf(*this);
    }

    template <class _Alloc>
    basic_obitstream<_Alloc>& basic_obitstream<_Alloc>::operator<<(basic_bios<_Alloc>& (*pf)(basic_bios<_Alloc>&)) {
        pf(*this);
        return *this;
    }

    /**
     *	@class basic_bitstream
     *	@brief The class basic_bitstream provides support for high level output/input operations on bit stream.
     */
    template <class _Alloc>
    class basic_bitstream : public basic_ibitstream<_Alloc>, public basic_obitstream<_Alloc> {
        using _Ibitstream = basic_ibitstream<_Alloc>;
        using _Obitstream = basic_obitstream<_Alloc>;
        using _Basic_bios = basic_bios<_Alloc>;

    public:
        using buf_type  = typename _Ibitstream::buf_type;
        using char_type = typename _Ibitstream::char_type;
        using int_type  = typename _Ibitstream::int_type;
        using pos_type  = typename _Ibitstream::pos_type;
        using off_type  = typename _Ibitstream::off_type;
        using byte      = typename _Ibitstream::byte;

        /**
         *	@brief Constructs new underlying bits device with the default(in/out) open mode
         */
        basic_bitstream() : basic_bitstream(bios::in | bios::out) {}
        /**
         *	@brief Constructs new underlying bits device. The underlying basic_bitbuf object is constructed as basic_bitbuf<Allocator>(mode | bios::in | bios::out)
         *	@param mode : specifies stream open mode.
         */
        explicit basic_bitstream(typename bios::openmode mode) : _Basic_bios(mode | bios::in | bios::out) {}
        /**
         *	@brief Constructs the basic_bitstream object, assigning initial values to the base class. The value of gcount() is initialized to zero.
         *	@param pbuf : pointer to bitsbuf to use as underlying device
         */
        explicit basic_bitstream(buf_type* pbuf) : _Basic_bios(pbuf) {}
        /**
         *	@brief Constructs the basic_bitstream object with external buffer.
         *	@param buffer : a successive memory to initialize bitbuf.
         *	@param bufsz : the size of buffer.
         *	@param mode : specifies stream open mode.
         */
        template <standard_layout_type _Buffer>
        basic_bitstream(_Buffer& buffer, const std::streamsize bufsz = sizeof(_Buffer), typename bios::openmode mode = bios::in | bios::out)
            : _Basic_bios(reinterpret_cast<byte*>(std::addressof(buffer)), bufsz, mode | bios::in | bios::out) {}

        basic_bitstream& operator=(basic_bitstream&& rhs) {
            this->swap(rhs);
            return *this;
        }

        void swap(basic_bitstream& rhs) { _Ibitstream::swap(rhs); }

        basic_bitstream(const basic_bitstream&) = delete;
        basic_bitstream& operator=(const basic_bitstream&) = delete;

        virtual ~basic_bitstream() noexcept {}
    };

    /**
     *	@class ibitstream_iterator
     *	@brief ibitstream_iterator is a single-pass input iterator that reads successive objects of type T
     *	from the basic_ibitsream object for which it was constructed, by calling the appropriate operator>>
     */
    template <class _Tp, class _Alloc = _DEFAULT_ALLOC(bios::byte)>
    class ibitstream_iterator {
    public:
        using streambuf_type = basic_bitbuf<_Alloc>;
        using istream_type   = basic_ibitstream<_Alloc>;

        using iterator_Category = std::input_iterator_tag;
        using value_type        = _Tp;
        using difference_type   = typename istream_type::bitsize;
        using pointer           = const value_type*;
        using reference         = const value_type&;

        static_assert(std::conjunction_v<std::is_default_constructible<_Tp>, std::is_copy_constructible<_Tp>, std::is_copy_assignable<_Tp>>,
                      "ibitstream_iterator<T> requires T to be default constructible, copy constructible, and copy assignable. ");

        constexpr ibitstream_iterator() {}
        ibitstream_iterator(istream_type& ibs) : _pibs(std::addressof(ibs)) { _Getval(); }

        XSTL_NODISCARD const _Tp& operator*() const {
#if !defined(_NO_ITERATOR_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETY_VERIFY_)
            assert(("ibitstreambuf_iterator is not dereferenceable", _pibs));
#endif
            return _value;
        }

        XSTL_NODISCARD const _Tp* operator->() const { return std::addressof(operator*()); }

        ibitstream_iterator& operator++() {
            _Getval();
            return *this;
        }

        ibitstream_iterator operator++(int) {
            ibitstream_iterator _tmp = *this;
            _Getval();
            return _tmp;
        }

        bool _Equal(const ibitstream_iterator& rhs) const { return _pibs == rhs._pibs; }

    private:
        void _Getval() {
#if !defined(_NO_ITERATOR_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETY_VERIFY_)
            assert(("ibitstreambuf_iterator is not incrementable", _pibs));
#endif
            auto&& _res = _pibs->get<_Tp>();
            if (_res.has_value())
                _value = std::move(_res.value());
            else
                _pibs = nullptr;
        }

        istream_type* _pibs = nullptr;
        _Tp           _value{};
    };

    template <class _Tp, class _Alloc>
    XSTL_NODISCARD bool operator==(const ibitstream_iterator<_Tp, _Alloc>& lhs, const ibitstream_iterator<_Tp, _Alloc>& rhs) {
        return lhs._Equal(rhs);
    }

    template <class _Tp, class _Alloc>
    XSTL_NODISCARD bool operator!=(const ibitstream_iterator<_Tp, _Alloc>& lhs, const ibitstream_iterator<_Tp, _Alloc>& rhs) {
        return !(lhs == rhs);
    }

    /**
     *	@class obitstream_iterator
     *	@brief obitstream_iterator is a single-pass input iterator that reads successive objects of type T
     *	from the basic_obitsream object for which it was constructed, by calling the appropriate operator<<
     */
    template <class _Tp, class _Delim = void, class _Alloc = _DEFAULT_ALLOC(bios::byte)>
    class obitstream_iterator {
    public:
        using iterator_Category = std::output_iterator_tag;
        using value_type        = void;
        using difference_type   = void;
        using pointer           = void;
        using reference         = void;

        using ostream_type = basic_obitstream<_Alloc>;

        obitstream_iterator(ostream_type& obs, const _Delim* const delim = nullptr) : _pdelim(delim), _pobs(std::addressof(obs)) {}

        obitstream_iterator& operator=(const _Tp& value) {
            *_pobs << value;
            if (_pdelim)
                *_pobs << _pdelim;
            return *this;
        }

        XSTL_NODISCARD obitstream_iterator& operator*() { return *this; }

        obitstream_iterator& operator++() { return *this; }

        obitstream_iterator& operator++(int) { return *this; }

    private:
        const _Delim* _pdelim;
        ostream_type* _pobs;
    };

    template <class _Alloc>
    basic_bios<_Alloc>& big_endian(basic_bios<_Alloc>& bs) {
        bs.setf(bios::bigendian);
        return bs;
    }

    template <class _Alloc>
    basic_bios<_Alloc>& little_endian(basic_bios<_Alloc>& bs) {
        bs.unsetf(bios::bigendian);
        return bs;
    }

    template <class _Alloc>
    basic_bios<_Alloc>& native_endian(basic_bios<_Alloc>& bs) {
        if (std::endian::native == std::endian::big)
            bs.setf(bios::bigendian);
        else
            bs.unsetf(bios::bigendian);
        return bs;
    }

    using bitbuf     = basic_bitbuf<_DEFAULT_ALLOC(bios::byte)>;
    using obitstream = basic_obitstream<_DEFAULT_ALLOC(bios::byte)>;
    using ibitstream = basic_ibitstream<_DEFAULT_ALLOC(bios::byte)>;
    using bitstream  = basic_bitstream<_DEFAULT_ALLOC(bios::byte)>;
}  // namespace xstl

namespace std {
    void swap(xstl::bios& x, xstl::bios& y) noexcept { x.swap(y); }

    template <class _Alloc>
    void swap(xstl::basic_bitbuf<_Alloc>& x, xstl::basic_bitbuf<_Alloc>& y) noexcept {
        x.swap(y);
    }

    template <class _Alloc>
    void swap(xstl::basic_ibitstream<_Alloc>& x, xstl::basic_ibitstream<_Alloc>& y) noexcept {
        x.swap(y);
    }

    template <class _Alloc>
    void swap(xstl::basic_obitstream<_Alloc>& x, xstl::basic_obitstream<_Alloc>& y) noexcept {
        x.swap(y);
    }

    template <class _Alloc>
    void swap(xstl::basic_bitstream<_Alloc>& x, xstl::basic_bitstream<_Alloc>& y) noexcept {
        x.swap(y);
    }
}  // namespace std
#pragma warning(pop)
#endif