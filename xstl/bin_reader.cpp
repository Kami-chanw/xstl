#define _LEVEL_ORDER_ITERATOR_
#include "bs_tree.hpp"
#include <algorithm>
#include <format>
#include <iostream>
#include <list>
#include <ranges>
#include <set>
#include <xtree>
using namespace std;
namespace rng = ranges;

int main() {
    xstl::bs_set<int> b{ 5, 3, 6, 2, 4, 8, 7 };
    b.display(cout);

    for (auto i : b)
        cout << i << " ";
    cout << endl;
    for (auto i : views::reverse(b))
        cout << i << " ";
}
// #include <xstring>
// #include "bs_tree.hpp"
// #include "bitstring.hpp"
// #include "utility.hpp"
// #include "huffman.hpp"
//  using namespace xstl;
//  using namespace std;
//  template <class T>
//  void print(const T& val) {
//	for (int i = 0; i < TYPE_BIT(T); ++i)
//		cout << (get_bits(val, i, i + 1) != 0);
//	cout << endl;
// }
//
//  int main() {
//     bs_set<int> b{ 1, 2, 3 }, b2;
//     b.swap(b2);
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
//         /* cout << "1001"_bstr[ 0, 2_ri ];
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
// }