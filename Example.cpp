#include "MathParser.hpp"

int main() {
	MathParser mp = MathParser();
	auto state = mp.setup();
	
	auto res = mp.RunLine("a = 1-3*cos(2*pi)", state);
	
	if (res.worked) {
		std::cout << res.value << "\n";  // -2
	}else{
		std::cout << res.error_msg << "\n";
	}
	
	res = mp.RunLine("2*a", state);
	
	if (res.worked) {
		std::cout << res.value << "\n";  // -4
	}else{
		std::cout << res.error_msg << "\n";
	}
}