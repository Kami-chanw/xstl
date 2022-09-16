#include <algorithm>
#include <iostream>
using namespace std;
#define N 15
int main() {
    int a[N], b[N], c[2 * N];
    generate_n(a, 15, [] { return rand() % 100; });
    sort(a, a + N);
    transform(a, a + N, b, [](auto v) { return v * 1.5; });
    merge(a, a + N, b, b + N, c);
    copy(c, c + N * 2, ostream_iterator<int>(cout, " "));
}
//#include <xstring>
//#include "bs_tree.hpp"
//#include "bitstring.hpp"
//#include "utility.hpp"
//#include "huffman.hpp"
//using namespace xstl;
//using namespace std;
//template <class T>
//void print(const T& val) {
//	for (int i = 0; i < TYPE_BIT(T); ++i)
//		cout << (get_bits(val, i, i + 1) != 0);
//	cout << endl;
//}
//
//int main() {
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