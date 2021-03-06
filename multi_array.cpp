//https://codereview.stackexchange.com/questions/94148/generic-multidimensional-array-in-c

#include <iostream>
using namespace std;
#include <memory>
using std::cout;
using std::endl;

template<typename T, int dimension = 1>
class Array{
private:
	std::unique_ptr<T[]> pointer;
	int bounds[dimension];
	int constants[dimension];
	int realSize{1};
public:
	Array() = default;
	template<typename... ints>
	Array(ints... ds){
		(realSize*=...*= ds);
		auto d{0};
		([&](const auto& x) { bounds[d] = x; d++; }(ds), ...);

		constants[dimension - 1] = 1;
		for (auto i{ dimension - 2 }; i >= 0; --i)
			constants[i] = bounds[i + 1] * constants[i + 1];

		pointer = std::unique_ptr<T[]>(new T[realSize]);
	}
	template<typename... ints>
	T& operator()(ints... js) const
	{		
		auto idx{ 0 };
		auto d{ 0 };
		([&](const auto& ji) { idx+=constants[d]*ji; d++; }(js), ...);		
		return pointer[idx];
	}
	template<typename... ints>
	T& operator()(ints... js) 
	{
		auto idx{0};
		auto d{ 0 };
		([&](const auto& ji) { idx += constants[d] * ji; d++; }(js), ...);
		return pointer[idx];
	}
};
int main() {
	Array<double,3> A(2, 3, 4);
	A(0, 1, 2) = 37;
	std::cout << A(0, 1, 2) << std::endl;
	Array<double, 2> B( 3, 4);

	auto k{ 0 };
	for (auto i{ 0 }; i != 3; i++)
		for (auto j{ 0 }; j != 4; j++)
			B(i, j) = k++;
	for (auto i{ 0 }; i != 3; i++) {
		for (auto j{ 0 }; j != 4; j++)
			std::cout << B(i, j) << " ";
		std::cout << std::endl;
	}
	return 0;	
}