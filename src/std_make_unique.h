
#include <memory>

// TODO: drop this helper when C++14 std_make_unique is available

template <typename T, typename... Args>
std::unique_ptr<T> std_make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
