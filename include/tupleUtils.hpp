#ifndef TUPLE_UTILS_HPP_
#define TUPLE_UTILS_HPP_

#include <utility>

namespace impl {
	template <typename Head, typename ...Args2>
	std::pair<std::tuple<Head>, std::tuple<Args2...>>
	split(std::tuple<Head, Args2...> & tuple) {

	}
} // namespace impl
template <typename ...Args1, typename ...Args2>
std::pair<std::tuple<Args1...>, std::tuple<Args2...>>
split(std::tuple<Args1..., Args2...> & source, std::tuple) {

}

#endif // !TUPLE_UTILS_HPP_
