#ifndef COMBINING_COMBINATORS_HPP_
#define COMBINING_COMBINATORS_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"
#include <functional>
#include <type_traits>
#include <utility>

namespace combinator {
	template <typename ...Args>
	class Lambda : public ICombinator<Args...> {
			std::function<ptr_res<Args...>(ICharStream&)> parseF_;
		public:
			Lambda(std::function<ptr_res<Args...>(ICharStream&)> parseF) : parseF_(parseF) {};
			Result<Args...> parse(ICharStream & stream) override {
				return parseF_(stream);
			}
		};

	template <typename T, typename ...Args>
		concept ResultT = std::is_base_of_v<Result<Args...>, T>;

	/**
	 * @brief Consume all parsers and return value from ParentN'th parser
	 *
	 * @tparam ParentN
	 * @tparam ...ResultsT
	 */
	template <ResultT Parent, ResultT ...ResultsT>
	class CombinedResult : public Parent {
		std::tuple<ResultsT...> results_;
		public:
			CombinedResult(Parent parent, std::tuple<ResultsT...> results)
				: Parent(parent), results_(results) {}
		void revert(ICharStream & stream) override {
			if(this->status_ != ResultStatus::Success) 
				return;
		
			[&stream]<size_t ...Idx>(std::index_sequence<Idx...>, std::tuple<ResultsT...> & results) {
				(std::get<Idx>(results).revert(stream),...);
			}(std::make_index_sequence<sizeof...(ResultsT)>{}, this->results);
		}
	};
} // namespace combinator

#endif // !COMBINING_COMBINATORS_HPP_
