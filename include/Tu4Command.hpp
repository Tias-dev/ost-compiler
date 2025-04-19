#ifndef TU_4_COMMAND_HPP_
#define TU_4_COMMAND_HPP_


#include <variant>
namespace tu4 {
enum class MoveDirection {
	LEFT,
	RIGHT
};

template <typename TQ>
class Tu4Command {
	TQ q0_, q_;
	char letterToCheck_;
public:
	Tu4Command(TQ q0, TQ q, char letterToCheck)
		: q0_(q0), q_(q), letterToCheck_(letterToCheck) {}

	const TQ & q0() {return q0_;}
	const TQ & q() {return q_;}
};

template <typename TQ>
class Tu4SetLetter : public Tu4Command<TQ> {
	char letterToSet_;
public:
	Tu4SetLetter(TQ q0, char letterToCheck, char letterToSet, TQ q)
		: Tu4Command<TQ>(q0, q, letterToCheck), letterToSet_(letterToSet) {}
};

template <typename TQ>
class Tu4Move : public Tu4Command<TQ> {
	MoveDirection dir_;
public:
	Tu4Move(TQ q0, char letterToCheck, MoveDirection dir, TQ q)
		: Tu4Command<TQ>(q0, q, letterToCheck), dir_(dir) {}
};

template <typename TQ>
using tu4_union = std::variant<Tu4SetLetter<TQ>, Tu4Command<TQ>>;
} // namespace tu4
	
#endif // !TU_4_COMMAND_HPP_
