#ifndef TUPLE_UTILS_HPP_
#define TUPLE_UTILS_HPP_

#include <cstddef>
#include <tuple>
#include <utility>

namespace impl {
template <size_t From, size_t To> auto createRangeIndexSequence() {
  return []<size_t... Idx>(std::index_sequence<Idx...>) {
    return std::index_sequence<(Idx + From)...>{};
  }(std::make_index_sequence<To - From>{});
}

template <size_t... Idx, typename TupleT>
auto tupleSlice(TupleT &tuple, std::index_sequence<Idx...>) {
  return std::make_tuple(std::get<Idx>(tuple)...);
}

template <typename... Args1, typename... Args2>
void
tupleSplit(std::tuple<Args1..., Args2...> &tuple, std::tuple<Args1...> & dst1, std::tuple<Args2...> & dst2) {
	dst1 = impl::tupleSlice(tuple, impl::createRangeIndexSequence<0, sizeof...(Args1)>());
	dst2 = impl::tupleSlice(tuple, impl::createRangeIndexSequence<sizeof...(Args1), sizeof...(Args1) + sizeof...(Args2)>());
}
} // namespace impl


template <typename... Args1, typename... Args2>
std::pair<std::tuple<Args1...>, std::tuple<Args2...>>
tupleSplit(std::tuple<Args1..., Args2...> &tuple) {
	std::pair<std::tuple<Args1...>, std::tuple<Args2...>> result;
	impl::tupleSplit(tuple, result.first, result.second);
	return result;
}

#endif // !TUPLE_UTILS_HPP_
