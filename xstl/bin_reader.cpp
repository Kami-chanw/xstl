#include "bs_tree.hpp"
#include <algorithm>
#include <iostream>
#include <list>
#include <ranges>
using namespace std;
namespace rng = ranges;
//clang-format off
template <class _Se, class _It>
concept s_for = copyable<_Se> && default_initializable<_Se> && input_or_output_iterator<_It> && _Weakly_equality_comparable_with<_Se, _It>;
template <class _Ty>
concept _Has_member = requires(_Ty __t) {
    { __t.end() } -> s_for<rng::iterator_t<_Ty>>;
};
template <_Has_member Rng>
void f(Rng) {}
int  main() {
    list s{ 1, 2, 3 };
    auto t = s.begin();
    s.pop_front();
    *t;
}
//#include <xstring>
//#include "bs_tree.hpp"
//#include "bitstring.hpp"
//#include "utility.hpp"
//#include "huffman.hpp"
// using namespace xstl;
// using namespace std;
// template <class T>
// void print(const T& val) {
//	for (int i = 0; i < TYPE_BIT(T); ++i)
//		cout << (get_bits(val, i, i + 1) != 0);
//	cout << endl;
//}
//
// int main() {
//    bs_set<int> b{ 1, 2, 3 }, b2;
//    b.swap(b2);
//	{
//		/*LARGE_INTEGER t1, t2, tc, tr{};
//		for (int i = 0; i < 100000; ++i) {
//
//			QueryPerformanceCounter(&t1);
//			bs >>= 21;
//			QueryPerformanceCounter(&t2);
//			tr.QuadPart += t2.QuadPart - t1.QuadPart;
//
//		}
//		cout << tr.QuadPart;*/
//        /* cout << "1001"_bstr[ 0, 2_ri ];
//
//		basic_bitstring<unsigned char> bs("1001011"), bs2("10001");
//		//bs[0, 6_ri].assign(bs[0,3_ri]);
//		bs[0,3_ri].swap(bs2[0,2_ri]);
//		//bs[2, 3_ri].resize(4);
//		int j = 0;
//		cout << endl;
//		//bs[0, 1_ri].insert(0, 5, 0);
//		for (auto i = bs.cbegin(); i != bs.cend(); ++i, ++j) {
//			cout << *i;
//			if (j == 7) {
//				cout << " ";
//				j = -1;
//			}
//		}*/
//
//	}
//	_CrtDumpMemoryLeaks();
//}