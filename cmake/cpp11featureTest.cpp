#include <string>
#include <cctype>

namespace n {
    class C {
        static_assert(true, "");
        
        C(const C&) = delete;
        C& operator=(const C&) = delete;
        C(C&&) = delete;
        C& operator=(C&&) = delete;
    };

    enum class E {
        v
    };
	
	auto v = E::v;
}

using CC = n::C;

int main(){
	auto toLower = [](std::string str) { for (char& c : str) { c = std::tolower(c); } return str; };
}
