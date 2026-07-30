#pragma once

#include <vector>
#include <parse_expression/import.h>
#include <parse_expression/expression.h>
#include <parse_expression/precedence.h>
#include <common/net.h>

#include "expression.h"
#include <boolean/cover.h>

namespace test {

struct ExpressionImporter : parse_expression::LValuedImporter<boolean::cover, std::string> {
	ucs::Netlist symbols;
	vector<int> region;
	bool autoDefine;

	ExpressionImporter(ucs::Netlist symbols, int region = 0, bool autoDefine = false);
	~ExpressionImporter();

	boolean::cover L_to_T(std::string lval, tokenizer *tokens) const override;
	std::string T_to_L(boolean::cover expr, tokenizer *tokens) const override;

	bool is_lvalue(const parse_expression::expression &syntax) const override;
	
	std::string import_term(const parse_expression::expression::argument &syntax, tokenizer *tokens) const override;

	void push_properties(parse_expression::operation op, const vector<parse_expression::expression::argument> &args, tokenizer *tokens) override;
	void pop_properties(parse_expression::operation op) override;

	std::string import_modifier(parse_expression::operation op, vector<std::string> args, tokenizer *tokens) const override;

	boolean::cover import_unary(parse_expression::operation op, boolean::cover expr, tokenizer *tokens) const override;
	boolean::cover import_binary(parse_expression::operation op, boolean::cover left, boolean::cover right, tokenizer *tokens) const override;
	boolean::cover import_modifier(parse_expression::operation op, vector<boolean::cover> args, tokenizer *tokens) const override;
};

boolean::cover import_expression(const parse_expression::expression &syntax, ucs::Netlist nets, tokenizer *tokens, int region = 0, bool auto_define = false);

struct CompositionImporter : parse_expression::Importer<boolean::cover> {
	ucs::Netlist symbols;
	vector<int> region;
	bool autoDefine;

	CompositionImporter(ucs::Netlist symbols, int region = 0, bool autoDefine = false);
	~CompositionImporter();

	boolean::cube import_assignment(const assignment &syntax, tokenizer *tokens) const;
	boolean::cover import_term(const parse_expression::expression::argument &syntax, tokenizer *tokens) const override;
	void push_properties(parse_expression::operation op, const vector<parse_expression::expression::argument> &args, tokenizer *tokens) override;
	void pop_properties(parse_expression::operation op) override;
	boolean::cover import_binary(parse_expression::operation op, boolean::cover left, boolean::cover right, tokenizer *tokens) const override;
	boolean::cover import_modifier(parse_expression::operation op, vector<boolean::cover> args, tokenizer *tokens) const override;
};

boolean::cube import_assignment(const assignment &syntax, ucs::Netlist nets, tokenizer *tokens, int region = 0, bool auto_define = false);
boolean::cover import_composition(const parse_expression::expression &syntax, ucs::Netlist nets, tokenizer *tokens, int region = 0, bool auto_define = false);

}
