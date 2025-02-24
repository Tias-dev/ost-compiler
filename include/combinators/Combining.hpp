#ifndef COMBINING_COMBINATORS_HPP_
#define COMBINING_COMBINATORS_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"
#include <functional>
#include <utility>

namespace combinator {
	template <typename ...Args>
	class Lambda : public ICombinator<Args...> {
			std::function<ptr_res<Args...>(ICharStream&)> parseF_;
		public:
			Lambda(std::function<ptr_res<Args...>(ICharStream&)> parseF) : parseF_(parseF) {};
			ptr_res<Args...> parse(ICharStream & stream) override {
				return parseF_(stream);
			}
		};

	/**
	 * @brief Container for multiple results
	 * @tparam Parent -- parent result from which data tuple is stored
	 * @tparam ...ResultsT -- Stored results which will be reverting in given order
	 */
	template <typename Parent, typename ...ResultsT>
	class CombinedResult : public Parent {
		std::tuple<std::shared_ptr<ResultsT>...> results_;
		public:
			CombinedResult(Parent parent, std::tuple<std::shared_ptr<ResultsT>...> results)
				: Parent(parent), results_(results) {}

		void revert(ICharStream & stream) override {
			if(this->status() != ResultStatus::Success) 
				return;
		
			[&stream]<size_t ...Idx>(std::index_sequence<Idx...>, std::tuple<std::shared_ptr<ResultsT>...> & results) {
				(std::get<Idx>(results)->revert(stream),...);
			}(std::make_index_sequence<sizeof...(ResultsT)>{}, this->results_);
		}
	};
} // namespace combinator

#endif // !COMBINING_COMBINATORS_HPP_
