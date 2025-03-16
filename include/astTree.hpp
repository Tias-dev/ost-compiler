#ifndef AST_TREE_HPP_
#define AST_TREE_HPP_

#include "Lexem.hpp"
#include "keywords.hpp"
#include "lexemType.hpp"
#include "operators.hpp"
#include <cassert>
#include <iostream>
#include <iterator>
#include <list>

class AstTree {
	struct Node {
		Lexem * lexem = nullptr;
		std::list<Node*> childs = {};
	};


	Node * root_;
	Node * processMt(std::list<Lexem*> &lexems) {
		Lexem * front = lexems.front();
		assert(("MT state must begin with keyword MT", front->type() == LexemType::Keyword && front->keyword() == KeywordType::MT));
		lexems.pop_front(), front = lexems.front();
		assert(("Expected Id after MT", front->type() == LexemType::Id));

		Node * node = new Node{.lexem = front};
		while(lexems.size() > 2) {
			auto second = std::begin(lexems), first = second++;
			if((*first)->keyword() == KeywordType::END && (*second)->Id().second == node->lexem->Id().second) {
				lexems.pop_front();
				lexems.pop_front();
				if(!lexems.empty() && lexems.front()->op() == OperatorType::Terminator) 
					lexems.pop_front();
				break;
			}

			node->childs.push_back(fetchExpression(lexems));
		}
		return node;
	}
	Node * processBegin(std::list<Lexem*> &lexems);
	Node * processAlphabet(std::list<Lexem*> &lexems);
	Node * processDo(std::list<Lexem*> &lexems);
	Node * processIf(std::list<Lexem*> &lexems);
	Node * processKeyword(std::list<Lexem*> &lexems) {

	}
	Node * processOperator(std::list<Lexem*> &lexems);
	Node * fetchExpression(std::list<Lexem*> &lexems) {
		Node * node = new Node;
		Lexem * first = lexems.front();
		switch (first->type()) {
			case LexemType::Id:
				node->lexem = first;
				lexems.pop_front();
				return node;
				break;
			case LexemType::Keyword:
				return processKeyword(lexems);
			case LexemType::Operator:
				return processOperator(lexems);
			default:
				std::cerr << "None type lexem found!" << std::endl;
				return nullptr;
		}
	}
	public:
	AstTree(std::list<Lexem *>& lexems) {
		root_ = new Node;
		while(!lexems.empty()) {
			root_->childs.push_back(fetchExpression(lexems));
		}
	};
};

#endif // !AST_TREE_HPP_
