#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <boolean/cube.h>
#include <boolean/cover.h>

#include <parse_expression/expression.h>
#include <parse_expression/precedence.h>
#include <parse_expression/assignment.h>

namespace boolean {

string export_value(int v);

struct ExpressionExporter {
	enum OpType {
		NOT = 0,
		INTERFERE = 1,
		AND = 2,
		OR = 3,
	};

	// override these
	virtual parse_expression::operation export_operator(int func) const = 0;
	virtual const parse_expression::precedence_set &precedence() const = 0;

	virtual parse_expression::expression::argument export_constant(int value) const;
	virtual parse_expression::expression::argument export_literal(size_t index) const;
	virtual parse_expression::expression::argument export_term(size_t index, int value) const;

	// these don't need to be overridden
	virtual parse_expression::expression export_expression(boolean::cube expr) const;
	virtual parse_expression::expression export_expression(boolean::cover expr) const;
	virtual parse_expression::expression export_expression_xfactor(boolean::cover expr, int op = OR) const;
	virtual parse_expression::expression export_expression_hfactor(boolean::cover expr) const;
};



/*template <typename expression>
expression export_field(string str) {
	static const auto op = expression::precedence.find(parse_expression::operation_set::MODIFIER, "", "[", ":", "]");

	expression result;
	if (op.level < 0 or op.index < 0) {
		if (expression::precedence.empty()) {
			error("", "operator precedence not defined", __FILE__, __LINE__);
		}
		result.valid = true;
		result.arguments.push_back(expression::argument::literalOf(str));
		return result;
	}
 
	result.valid = true;
	result.level = op.level;

	string name = str;
	
	size_t open = name.find('[');
	if (open != string::npos) {
		name = str.substr(0u, open);
		result.operators.push_back(op.index);
	}

	result.arguments.push_back(expression::argument::literalOf(name));
	while (open != string::npos and open < str.size()) {
		open += 1;
		size_t close = str.find(']', open);
		result.arguments.push_back(expression::argument::constantOf(str.substr(open, close-open)));
		open = close+1;
	}

	return result;
}

template <typename expression>
expression export_member(string str) {
	static const auto op = expression::precedence.find(parse_expression::operation_set::MODIFIER, "", ".", "", "");
	
	expression result;
	if (op.level < 0 or op.index < 0) {
		if (expression::precedence.empty()) {
			error("", "operator precedence not defined", __FILE__, __LINE__);
		}
		result.valid = true;
		result.arguments.push_back(export_field<expression>(str));
		return result;
	}
 
	result.valid = true;
	result.level = op.level;

	if (not str.empty()) {
		size_t prev = 0u;
		size_t dot = str.find('.', prev);
		while (dot != string::npos and dot < str.size()) {
			result.arguments.push_back(export_field<expression>(str.substr(prev, dot-prev)));
			result.operators.push_back(op.index);
			prev = dot+1;
			dot = str.find('.', prev);
		}
		result.arguments.push_back(export_field<expression>(str.substr(prev)));
	}

	return result;
}

template <typename expression>
expression export_net(string str) {
	static const auto op = expression::precedence.find(parse_expression::operation_set::MODIFIER, "", "'", "", "");

	expression result;
	if (op.level < 0 or op.index < 0) {
		if (expression::precedence.empty()) {
			error("", "operator precedence not defined", __FILE__, __LINE__);
		}
		result.valid = true;
		result.arguments.push_back(export_member<expression>(str));
		return result;
	}
 
	result.valid = true;
	result.level = op.level;

	size_t tic = str.rfind('\'');
	if (tic != string::npos) {
		string region = str.substr(tic+1);
		str = str.substr(0, tic);
		result.operators.push_back(op.index);
		result.arguments.push_back(export_member<expression>(str));
		result.arguments.push_back(expression::argument::constantOf(region));
	} else {
		result.arguments.push_back(export_member<expression>(str));
	}

	return result;
}


template <typename expression>
expression export_net(int uid, ucs::ConstNetlist nets) {
	string name = nets.netAt(uid);
	if (name.empty()) {
		return expression();
	}

	return export_net<expression>(name);
}

template <typename assignment>
assignment export_assignment(int uid, int value, ucs::ConstNetlist nets) {
	assignment result;
	result.valid = true;

	result.lvalue.push_back(export_net<typename assignment::expression>(uid, nets));
	if (value == 0)
		result.operation = "-";
	else if (value == 1)
		result.operation = "+";
	else if (value == -1)
		result.operation = "~";

	return result;
}

template <typename composition>
composition export_composition(boolean::cube c, ucs::ConstNetlist nets) {
	composition::init();
	static const int level = composition::get_level(",");

	composition result;
	result.valid = true;

	result.level = level;

	for (int uid = 0; uid < c.size()*16; uid++) {
		int val = c.get(uid);
		if (val != 2) {
			result.literals.push_back(export_assignment<typename composition::assignment>(uid, val, nets));
		}
	}

	return result;
}

template <typename composition>
composition export_composition(boolean::cover c, ucs::ConstNetlist nets) {
	composition::init();
	static const int level = composition::get_level(":");

	composition result;
	result.valid = true;

	result.level = level;

	for (int i = 0; i < (int)c.cubes.size(); i++) {
		result.compositions.push_back(export_composition<composition>(c.cubes[i], nets));
	}

	return result;
}



*/

}
