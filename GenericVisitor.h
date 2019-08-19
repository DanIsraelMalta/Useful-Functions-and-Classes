/**
* A generic visitor
*
* Dan Israel Malta
**/
#include <utility>

// forward deceleration of a variadic visitor interface
template<typename ... Types> class Visitor;

// one argument visitor interface
template<typename T> class Visitor<T> {
	public:
		virtual void visit(T& visitable) = 0;
};

// variadic visitor interface
template<typename T, typename ... TList> class Visitor<T, TList ...> : public Visitor<TList ...> {
	public:
		using Visitor<TList ...>::visit;
		virtual void visit(T& visitable) = 0;
};

// variadic visit-able interface 
template<typename ... TList> class Visitable {
	public:
		virtual void accept(Visitor<TList ...> & visitor) = 0;
};

/**
* \brief variadic visit-able implementation
* 
* @param {Derived, in} visit-able object
* @param {...,     in} list of objects which are visit-able
**/
template<typename Derived, typename ... TList> class VisitableImpl : public Visitable<TList ...> {
	public:
		virtual void accept(Visitor<TList ...> & visitor) {
			visitor.visit(static_cast<Derived &>(*this));
		}
};

// forward deceleration of a generic variadic visitor
template<typename ... TList> class GenericVisitor;

/**
* \brief generic visitor on one visit-able object
* 
* @param {U, in} visitor object
* @param {T, in} visit-able object
**/
template<typename U, typename T> class GenericVisitor<U, T> {
	protected:
		U u;

	public:
		template<typename ... ParamList> GenericVisitor(ParamList && ... plist) : u(std::forward<ParamList>(plist) ...) {}

		virtual void visit(T & t) {
			u.visit(t);
		}
};

/**
* \brief variadic generic visitor
*
* @param {U,   in} visitor object
* @param {..., in} visit-able objects
**/
template<typename U, typename T, typename ... TList> class GenericVisitor<U, T, TList ...> : public GenericVisitor<U, TList ...> {
public:
	template<typename ... ParamList> GenericVisitor(ParamList && ... plist) : GenericVisitor<U, TList ...>(std::forward<ParamList>(plist) ...) {}

	using GenericVisitor<U, TList ...>::visit;
	using GenericVisitor<U, TList ...>::u;

	virtual void visit(T & t) {
		u.visit(t);
	}
};

// ---------------------
// --- example usage ---
// ---------------------

#include "GenericVisitor.h"
#include <iostream>

// an 'expression' interface
class Expression {
public:
	virtual std::string name() = 0;
};

// forward deceleration of 'constant' and 'variable' objects
template<typename T> class Constant;
class Variable;

// 'constant' object
template<typename T> class Constant : public Expression, public VisitableImpl<Constant<T>, Constant<T>, Variable> {
	public:
		virtual std::string name() { return "Constant"; }
};

// 'variable' object
class Variable : public Expression, public VisitableImpl<Variable, Constant<double>, Variable> {
	public:
		virtual std::string name() { return "Variable"; }
};

// a visitor for 'constant' and 'variable' objects
class ExpressionVisitor {
	public:
		ExpressionVisitor() { std::cout << "ExpressionVisitor was created.\n"; }
		template<typename T> void visit(Constant<T> c) { std::cout << "visited a 'Constant' object.\n"; }
		void visit(Variable c) { std::cout << "visited a 'Variable' object.\n"; }
};


int main(int argc, char * argv[]) {

	Variable var;
	Constant<double> con;

	GenericVisitor<ExpressionVisitor, Constant<double>, Variable> v;

	v.visit(var);
	v.visit(con);

	return 0;

}
