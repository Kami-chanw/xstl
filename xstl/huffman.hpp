#ifndef _HUFFMAN_HPP_
#define _HUFFMAN_HPP_
#include "bitstream.hpp"
#include "compressed_tuple.hpp"
#include "config.hpp"
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#define IS_LEAF(NODE) (!((NODE)->_left && (NODE)->_right))

#define ELEM_NUM_TYPE uint8_t
#define MAX_ELEM_NUM std::numeric_limits<ELEM_NUM_TYPE>::max()

#define _UL_CODE_ 1
#define _ULL_CODE_ 2
#define _CODE_TYPE_ _ULL_CODE_
#if _CODE_TYPE_ == _UL_CODE_
#define CODE_TYPE unsigned long
#define MAX_CODE_LENGTH (sizeof(unsigned long) * CHAR_BIT)
#define TO_BIN(STR) std::stoul(STR, nullptr, 2)
#elif _CODE_TYPE_ == _ULL_CODE_
#define CODE_TYPE unsigned long long
#define MAX_CODE_LENGTH (sizeof(unsigned long long) * CHAR_BIT)
#define TO_BIN(STR) std::stoull(STR, nullptr, 2)
#endif

namespace xstl {
    namespace {
        /**
         *	@class _Huff_node
         *   @brief the node of huff.
         */
        struct _Huff_node {
            using value_type = uint8_t;

            _Huff_node(const uint8_t& value, int weight, _Huff_node* left = nullptr, _Huff_node* right = nullptr) : _value(value), _weight(weight), _left(left), _right(right) {}

            int         _weight;
            uint8_t     _value;
            _Huff_node* _left;
            _Huff_node* _right;
        };

        template <class _Alnode>
        void destroy_node(_Alnode& alloc, _Huff_node* node) noexcept {
            static_assert(std::is_same_v<typename _Alnode::value_type, _Huff_node>, "Allocator's value_type is not consist with node");
            std::allocator_traits<_Alnode>::destroy(alloc, std::addressof(node->_value));
            std::allocator_traits<_Alnode>::deallocate(alloc, node, 1);
        }

        template <class _Alnode>
        void _Destroy(_Alnode& alloc, _Huff_node* node) {
            if (node) {
                _Destroy(alloc, node->_left);
                _Destroy(alloc, node->_right);
                destroy_node(alloc, node);
            }
        }

        template <class _Alnode>
        inline _Huff_node* _Create_node(_Alnode& alloc, uint8_t value, int weight, _Huff_node* left = nullptr, _Huff_node* right = nullptr) {
            _Huff_node*     _node = alloc.allocate(1);
            exception_guard _guard(_node, [](_Huff_node* node) { destroy_node(node); });
            std::allocator_traits<_Alnode>::construct(alloc, std::addressof(_node->_value), value);
            _node->_left   = left;
            _node->_right  = right;
            _node->_weight = weight;
            return _guard.release();
        }

    }  // namespace

    /**
     *	@class huff_encoder
     *  @brief compress data by Huffman code
     */
    template <class _Alloc = _DEFAULT_ALLOC(uint8_t)>
    class huff_encoder {
        using _Self      = huff_encoder<_Alloc>;
        using _PairAlloc = typename std::allocator_traits<_Alloc>::template rebind_alloc<std::pair<const uint8_t, std::string>>;
        using _HuffMap   = std::unordered_map<uint8_t, std::string, std::hash<uint8_t>, std::equal_to<uint8_t>, _PairAlloc>;
        using _Bstream   = basic_bitstream<typename ::std::allocator_traits<_Alloc>::template rebind_alloc<bios::byte>>;
        using _Alnode    = typename std::allocator_traits<_Alloc>::template rebind_alloc<_Huff_node>;
        using node       = _Huff_node;
        using link_type  = _Huff_node*;

    public:
        using const_iterator  = typename _HuffMap::const_iterator;
        using iterator        = typename _HuffMap::iterator;
        using value_type      = typename _HuffMap::value_type;
        using difference_type = typename _HuffMap::difference_type;
        using size_type       = typename _HuffMap::size_type;
        using reference       = typename _HuffMap::reference;
        using const_reference = typename _HuffMap::const_reference;
        using pointer         = typename _HuffMap::pointer;
        using const_pointer   = typename _HuffMap::const_pointer;

        iterator       begin() { return _code_map.begin(); }
        iterator       end() { return _code_map.end(); }
        const_iterator begin() const { return _code_map.begin(); }
        const_iterator end() const { return _code_map.end(); }
        const_iterator cbegin() const { return _code_map.cbegin(); }
        const_iterator cend() const { return _code_map.cend(); }

        huff_encoder() = default;
        /**
         *   @brief read data from [first, last) and create Huffman tree
         *   @param first : the beginning of range of elements to read
         *   @param last : the end of range of elements to read
         */
        template <class _Iter>
        huff_encoder(_Iter first, _Iter last) {
            read_data(first, last);
        }
        huff_encoder(const _Self& x) = delete;
        huff_encoder(_Self&& x) noexcept : _tpl(std::move(x._tpl)), _code_map(std::move(x._code_map)), _datanum(x._datanum) {}

        /**
         *   @brief read data from [first, last) and create Huffman tree
         *   @param first : the beginning of range of elements to read
         *   @param last : the end of range of elements to read
         */
        template <class _Iter>
        bool read_data(_Iter first, _Iter last);
        /**
         *   @brief compress data by Huffman tree which is created before via read_data(). first and last should be the same as before
         *   @param first : the beginning of range of elements to compress
         *   @param last : the end of range of elements to compress
         *	@param dst : the standard ostream to store the result.
         */
        template <class _Iter, class _Elem, class _ElemTraits>
        bool compress(_Iter first, _Iter last, std::basic_ostream<_Elem, _ElemTraits>& dst);

        ~huff_encoder() noexcept {}

        _Self& operator=(_Self&& rhs) {
            if (this != std::addressof(rhs)) {
                _tpl      = std::move(rhs._tpl);
                _code_map = std::move(rhs._code_map);
                _datanum  = rhs._datanum;
            }
            return *this;
        }
        _Self& operator=(const _Self& rhs) = delete;

    private:
        template <class _Container>
        void           _Create_tree(const _Container& counter);
        void           _Encode(link_type node, std::string& code);
        void           _Serialize_tree(_Bstream& bs, link_type node, std::string& tree_info);
        auto           _Get_root() { return std::get<0>(_tpl); }
        const auto     _Get_root() const { return std::get<0>(_tpl); }
        _Alnode&       _Getal() { return std::get<1>(_tpl); }
        const _Alnode& _Getal() const { return std::get<1>(_tpl); }

        _HuffMap                             _code_map;
        size_type                            _datanum = 0;
        compressed_tuple<link_type, _Alnode> _tpl;
    };

    template <class _Alloc>
    template <class _Iter>
    bool huff_encoder<_Alloc>::read_data(_Iter first, _Iter last) {
        static_assert(sizeof(std::iter_value_t<_Iter>) == 1, "huff encoder requires sizeof(iter::value_type) == 1");
        if (first == last)
            return false;
        std::unordered_map<uint8_t, size_type, std::hash<uint8_t>, std::equal_to<uint8_t>, typename std::allocator_traits<_Alloc>::template rebind_alloc<std::pair<const uint8_t, size_type>>> _counter;
        for (; first != last; ++first, ++_datanum)
            _counter[*first]++;
        _Create_tree(_counter);
        std::string _code;
        _Encode(_Get_root(), _code);
        return true;
    }

    template <class _Alloc>
    template <class _Container>
    void huff_encoder<_Alloc>::_Create_tree(const _Container& counter) {
        std::multimap<size_type, link_type, std::less<const size_type>, typename std::allocator_traits<_Alloc>::template rebind_alloc<std::pair<const size_type, link_type>>> _forest;
        for (const auto& _cur_pair : counter)
            _forest.emplace(_cur_pair.second, _Create_node(_Getal(), 0, _cur_pair.first, _cur_pair.second, nullptr, nullptr));
        while (_forest.size() > 1) {
            link_type _left  = _forest.extract(_forest.cbegin()).mapped();
            link_type _right = _forest.extract(_forest.cbegin()).mapped();
            if (_left->_weight > _right->_weight)
                std::swap(_left, _right);
            link_type _tmp = _Create_node(_Getal(), 0, _left->_weight + _right->_weight, _left, _right);
            _forest.emplace_hint(_forest.cbegin(), _tmp->_weight, _tmp);
        }
        _Get_root() = _forest.extract(_forest.cbegin()).mapped();
    }

    template <class _Alloc>
    void huff_encoder<_Alloc>::_Encode(link_type node, std::string& code) {
        if (IS_LEAF(node))
            _code_map.emplace(node, code);
        else {
            code += '0';
            _Encode(node->_left, code);
            code += '1';
            _Encode(node->_right, code);
        }
        if (!code.empty())
            code.pop_back();
    }

    template <class _Alloc>
    void huff_encoder<_Alloc>::_Serialize_tree(_Bstream& bs, link_type node, std::string& tree_info) {
        if (node == nullptr)
            return;
        if (IS_LEAF(node))
            bs << node;
        else {
            tree_info += IS_LEAF(node->_left) ? '0' : '1';
            tree_info += IS_LEAF(node->_right) ? '0' : '1';
            _Serialize_tree(bs, node->_left, tree_info);
            _Serialize_tree(bs, node->_right, tree_info);
        }
    }

    template <class _Alloc>
    template <class _Iter, class _Elem, class _ElemTraits>
    bool huff_encoder<_Alloc>::compress(_Iter first, _Iter last, std::basic_ostream<_Elem, _ElemTraits>& dst) {
        static_assert(sizeof(std::iter_value_t<_Iter>) == 1, "huff encoder requires sizeof(iter::value_type) == 1");
        if (!dst || !_Get_root())
            return false;
#if !defined(_NO_HUFF_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETY_VERIFY_)
        assert(_code_map.size() <= MAX_ELEM_NUM);
#endif
        _Bstream _bs{};
        _bs.setf(xstl::bios::bigendian);
        _bs << "xstl" << static_cast<ELEM_NUM_TYPE>(_code_map.size());
        std::string _tree_info;
        _Serialize_tree(_bs, _Get_root(), _tree_info);
        _bs << _datanum;
        size_type i = 0;
        for (; i < _tree_info.size() / MAX_CODE_LENGTH; ++i)
            _bs.put(TO_BIN(_tree_info.substr(i * MAX_CODE_LENGTH, MAX_CODE_LENGTH)), MAX_CODE_LENGTH);
        _tree_info.erase(0, i * MAX_CODE_LENGTH);
        if (_tree_info.size())
            _bs.put(TO_BIN(_tree_info) << (MAX_CODE_LENGTH - _tree_info.size()), _tree_info.size());
        for (; first != last; ++first) {
            std::string& _code = _code_map[*first];
#if !defined(_NO_HUFF_SAFETY_VERIFY_) && !defined(_NO_XSTL_SAFETY_VERIFY_)
            assert(_code.size() <= MAX_CODE_LENGTH);
#endif
            _bs.put(TO_BIN(_code) << (MAX_CODE_LENGTH - _code.size()), _code.size());
        }
        while (true) {
            auto res = _bs.get<std::uint8_t>();
            if (res.has_value())
                dst << res.value();
            else
                break;
        }
        return true;
    }

    /**
     *	@class huff_decoder
     *   @brief decompress data by Huffman code
     */
    template <class _Alloc = _DEFAULT_ALLOC(uint8_t)>
    class huff_decoder {
        using _Self     = huff_decoder<_Alloc>;
        using _Bstream  = basic_bitstream<typename ::std::allocator_traits<_Alloc>::template rebind_alloc<bios::byte>>;
        using _Alnode    = typename std::allocator_traits<_Alloc>::template rebind_alloc<_Huff_node>;
        using node      = _Huff_node;
        using link_type = _Huff_node*;

    public:
    private:
        enum class _Cat { NODE, LEAF };
        struct _Atbase {
            _Atbase(_Cat cat) : _cat(cat) {}
            virtual ~_Atbase() {}
            _Cat _cat;
        };

        struct _Atnode : public _Atbase {
            _Atnode() : _Atbase(_Cat::NODE) {}
            std::unique_ptr<_Atbase> _child[16];
        };
        struct _Atleaf : public _Atbase {
            _Atleaf(uint8_t * ptr, size_t size) : _Atbase(_Cat::LEAF), _node(ptr), _len(size) {}
            uint8_t*  _node;
            size_t _len;
        };

    public:
        huff_decoder()               = default;
        huff_decoder(const _Self& x) = delete;
        huff_decoder(_Self&& x) noexcept : _tpl(std::move(x._tpl)), _dict(std::move(x._dict)) {}

        /**
         *   @brief decompress data from [fisrt, last).
         *   @param first : the beginning of range of elements to compress
         *   @param last : the end of range of elements to compress
         * 	@param dst : the standard ostream to store the result.
         */
        template <class _Iter, class _Elem, class _ElemTraits>
        bool decompress(_Iter first, _Iter last, std::basic_ostream<_Elem, _ElemTraits>& dst);

        _Self& operator=(_Self&& rhs) {
            if (this != std::addressof(rhs)) {
                _tpl  = std::move(rhs._tpl);
                _dict = std::move(rhs._dict);
            }
            return *this;
        }
        _Self& operator=(const _Self& x) = delete;
        ~huff_decoder() noexcept {}

    private:
        void _Set_node(_Bstream& bs, link_type& node, std::list<uint8_t, _Alloc>& values, bool bit);
        void _Recreate(_Bstream& bs, link_type& node, std::list<uint8_t, _Alloc>& values);
        void _Create_dict(link_type node, CODE_TYPE code, std::uint8_t index);
        auto           _Get_root() { return std::get<0>(_tpl); }
        const auto     _Get_root() const { return std::get<0>(_tpl); }
        _Alnode&       _Getal() { return std::get<1>(_tpl); }
        const _Alnode& _Getal() const { return std::get<1>(_tpl); }

        compressed_tuple<link_type, _Alnode> _tpl;
        _Atnode _dict;
    };

    template <class _Alloc>
    void huff_decoder<_Alloc>::_Recreate(_Bstream& bs, link_type& node, std::list<uint8_t, _Alloc>& values) {
        bool _left_bit = bs.get().value(), _right_bit = bs.get().value();
        _Set_node(bs, node->_left, values, _left_bit);
        _Set_node(bs, node->_right, values, _right_bit);
    }

    template <class _Alloc>
    void huff_decoder<_Alloc>::_Set_node(_Bstream& bs, link_type& node, std::list<uint8_t, _Alloc>& values, bool bit) {
        if (bit) {
            node = _Create_node(_Getal(), 0);
            _Recreate(bs, node, values);
        }
        else {
            node = _Create_node(values.front(), 0);
            values.pop_front();
        }
    }

    template <class _Alloc>
    void huff_decoder<_Alloc>::_Create_dict(link_type node, CODE_TYPE code, std::uint8_t index) {
        if (IS_LEAF(node)) {
            _Atnode* _curr = &_dict;
            uint8_t  _idx;
            for (size_t i = 1; (_idx = code >> (MAX_CODE_LENGTH - (i << 2)) & 0xf), i != static_cast<size_t>(index + 3) >> 2; ++i) {
                if (_curr->_child[_idx] == nullptr)
                    _curr->_child[_idx] = std::make_unique<_Atnode>();
                _curr = static_cast<_Atnode*>(_curr->_child[_idx].get());
            }
            for (bool _lastbit = _idx & 1 << (4 - index & 3); _lastbit == static_cast<bool>(_idx & 1 << (4 - index & 3)); ++_idx)
                _curr->_child[_idx] = std::make_unique<_Atleaf>(std::addressof(node), index);
        }
        else {
            ++index;
            _Create_dict(node->_left, code, index);
            _Create_dict(node->_right, code | static_cast<CODE_TYPE>(1) << (MAX_CODE_LENGTH - index), index);
            --index;
        }
    }

    template <class _Alloc>
    template <class _Iter, class _Elem, class _ElemTraits>
    bool huff_decoder<_Alloc>::decompress(_Iter first, _Iter last, std::basic_ostream<_Elem, _ElemTraits>& dst) {
        static_assert(sizeof(std::iter_value_t<_Iter>) == 1, "huff decoder requires sizeof(iter::value_type) == 1");

        _Bstream _bs{};
        for (size_t i = 0; i < sizeof(int) / sizeof(std::iter_value_t<_Iter>); ++i, ++first)
            _bs << *first;
        if (!dst || _bs.get<int>().value() != *(reinterpret_cast<const int*>("xstl")))
            return false;
        _bs.setf(bios::bigendian);
        while (first != last && (_bs << *first))
            ++first;
        std::list<uint8_t, _Alloc> _values;
        for (ELEM_NUM_TYPE _mapsz = _bs.get<ELEM_NUM_TYPE>().value(); _mapsz > 0; --_mapsz)
            _values.emplace_back(std::move(_bs.get<uint8_t>().value()));
        size_t _datanum = _bs.get<size_t>().value();
        _Get_root()              = _Create_node(_Getal(), 0);
        _Recreate(_bs, _Get_root(), _values);
        _Create_dict(_Get_root(), 0, 0);
        auto _pbuf = _bs.rdbuf();
        for (bios::byte _idx{}; _datanum-- > 0;)
            for (_Atnode* _node = std::addressof(_dict);; _node = static_cast<_Atnode*>(_node->_child[_idx].get())) {
                size_t _oldcount = _pbuf->in_avail();
                _idx                = _pbuf->sbumpb(4) >> 4;
                if (_node->_child[_idx]->_cat == _Cat::LEAF) {
                    dst << *static_cast<_Atleaf*>(_node->_child[_idx].get())->_node;
                    auto _ungetbits = _oldcount - _pbuf->in_avail() - (static_cast<_Atleaf*>(_node->_child[_idx].get())->_len & 3);
                    if (_ungetbits != 4)
                        _pbuf->sungetn(_ungetbits);
                    break;
                }
            }
        return true;
    }
}  // namespace xstl

#undef CAST
#undef VALUE
#undef IS_LEAF
#endif
