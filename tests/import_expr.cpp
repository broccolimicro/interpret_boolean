#include "import_expr.h"
#include "expression.h"
#include <interpret_boolean/import_default.h>

namespace test {

ExpressionImporter::ExpressionImporter(ucs::Netlist symbols, int region, bool autoDefine) : symbols(symbols) {
	this->region.push_back(region);
	this->autoDefine = autoDefine;
}

ExpressionImporter::~ExpressionImporter() {
}

boolean::cover ExpressionImporter::L_to_T(std::string lval, tokenizer *tokens) const {
	if (lval == "vdd") {
		return boolean::cover(1);
	} else if (lval == "gnd") {
		return boolean::cover();
	}
	int uid = boolean::import_net(lval, symbols, tokens, autoDefine);
	if (uid < 0) {
		return boolean::cover();
	}
	return boolean::cover(uid, 1);
}

std::string ExpressionImporter::T_to_L(boolean::cover expr, tokenizer *tokens) const {
	internal("", "sub expressions in variabe names not supported", __FILE__, __LINE__);
	return "gnd";
}

bool ExpressionImporter::is_lvalue(const parse_expression::expression &syntax) const {
	return syntax.level >= expression_config::cfg->lvalueLevel;
}

std::string ExpressionImporter::import_term(const parse_expression::expression::argument &syntax, tokenizer *tokens) const {
	if (syntax.type < 0 or syntax.type >= (int)expression_config::cfg->literals.size() or not syntax.ptr) {
		return "gnd";
	}

	std::string type = expression_config::cfg->literals[syntax.type].first;

	if (type == "constant") {
		std::string value = syntax.ptr->get<constant>().value;
		if (value == "vdd" or value == "gnd") {
			return value;
		}
		error("", "unrecognized constant value, expected 'vdd' or 'gnd'", __FILE__, __LINE__);
		return "gnd";
	} else if (type == "literal") {
		std::string name = syntax.ptr->get<literal>().name;
		if (region.back() != 0) {
			name += "'" + std::to_string(region.back());
		}
		return name;
	}
	internal("", "unsupported literal type '" + type + "'", __FILE__, __LINE__);
	return "gnd";
}

void ExpressionImporter::push_properties(parse_expression::operation op, const vector<parse_expression::expression::argument> &args, tokenizer *tokens) {
	if (op.is("", "'", "", "")) { // Region
		int value = -1;
		if (args.size() == 2u) {
			std::string str = args[1].ptr->to_string("");
			value = atoi(str.c_str());
		} else {
			error("", "operator ''' expects 2 arguments, found '" + ::to_string(args.size()) + "'", __FILE__, __LINE__);
		}
		this->region.push_back(value);
	}
}

void ExpressionImporter::pop_properties(parse_expression::operation op) {
	if (op.is("", "'", "", "")) { // Region
		region.pop_back();
	}
}

std::string ExpressionImporter::import_modifier(parse_expression::operation op, vector<std::string> args, tokenizer *tokens) const {
	if (op.is("", "'", "", "")) { // Region
		// only affects properties
		return args[0];
	} else if (op.is("", ".", "", "")) { // Member
		std::string result = args[0];
		for (int i = 1; i < (int)args.size(); i++) {
			result += "." + args[i];
		}
		return result;
	} else if (op.is("", "[", ":", "]")) {
		std::string result = args[0];
		if (args.size() > 1u) {
			result += "[" + args[1];
			for (int i = 2; i < (int)args.size(); i++) {
				result += ":" + args[i];
			}
			result += "]";
		}
		return result;
	}
	internal("", "sub expressions in variabe names not supported", __FILE__, __LINE__);
	return "gnd";
}

boolean::cover ExpressionImporter::import_unary(parse_expression::operation op, boolean::cover expr, tokenizer *tokens) const {
	if (op.is("~", "", "", "")) {
		return ~expr;
	}
	internal("", "unrecognized operation", __FILE__, __LINE__);
	return expr;
}

boolean::cover ExpressionImporter::import_binary(parse_expression::operation op, boolean::cover left, boolean::cover right, tokenizer *tokens) const {
	if (op.is("", "", "|", "")) {
		return left | right;
	} else if (op.is("", "", "&", "")) {
		return left & right;
	} else if (op.is("", "", "^", "")) {
		return left ^ right;
	}
	internal("", "unrecognized operation", __FILE__, __LINE__);
	return left;
}

boolean::cover ExpressionImporter::import_modifier(parse_expression::operation op, vector<boolean::cover> args, tokenizer *tokens) const {
	if (op.is("", "'", "", "")) { // Region
		// only affects properties
		return args[0];
	}
	internal("", "unrecognized operation", __FILE__, __LINE__);
	return boolean::cover();
}

boolean::cover import_expression(const parse_expression::expression &syntax, ucs::Netlist nets, tokenizer *tokens, int region, bool auto_define) {
	return ExpressionImporter(nets, region, auto_define).import_expression(syntax, tokens);
}





CompositionImporter::CompositionImporter(ucs::Netlist symbols, int region, bool autoDefine) : symbols(symbols) {
	this->region.push_back(region);
	this->autoDefine = autoDefine;
}

CompositionImporter::~CompositionImporter() {
}

boolean::cube CompositionImporter::import_assignment(const assignment &syntax, tokenizer *tokens) const {
	ExpressionImporter in(symbols, region.back(), autoDefine);

	if (syntax.operation.empty() or syntax.left.size() != 1u) {
		error("", "malformed assignment", __FILE__, __LINE__);
		return boolean::cube();
	}

	std::string lval = in.import_lvalue(syntax.left[0], tokens);
	int uid = boolean::import_net(lval, symbols, tokens, autoDefine);
	if (uid < 0) {
		return boolean::cube();
	}

 	if (syntax.operation == "+") {
		return boolean::cube(uid, 1);
	} else if (syntax.operation == "-") {
		return boolean::cube(uid, 0);
	} else if (syntax.operation == "=") {
		std::string rval = in.import_lvalue(syntax.right, tokens);
		if (rval == "vdd") {
			return boolean::cube(uid, 1);
		} else if (rval == "gnd") {
			return boolean::cube(uid, 0);
		}
		internal("", "unsupported constant type", __FILE__, __LINE__);
		return boolean::cube();
	}
	internal("", "unsupported assignment operation", __FILE__, __LINE__);
	return boolean::cube();
}

boolean::cover CompositionImporter::import_term(const parse_expression::expression::argument &syntax, tokenizer *tokens) const {
	if (syntax.type < 0 or syntax.type >= (int)expression_config::cfg->literals.size() or not syntax.ptr) {
		return boolean::cover();
	}

	return boolean::cover(import_assignment(syntax.ptr->get<assignment>(), tokens));
}

void CompositionImporter::push_properties(parse_expression::operation op, const vector<parse_expression::expression::argument> &args, tokenizer *tokens) {
	if (op.is("", "'", "", "")) { // Region
		int value = -1;
		if (args.size() == 2u) {
			std::string str = args[1].ptr->to_string("");
			value = atoi(str.c_str());
		} else {
			error("", "operator ''' expects 2 arguments, found '" + ::to_string(args.size()) + "'", __FILE__, __LINE__);
		}
		this->region.push_back(value);
	}
}

void CompositionImporter::pop_properties(parse_expression::operation op) {
	if (op.is("", "'", "", "")) { // Region
		region.pop_back();
	}
}

boolean::cover CompositionImporter::import_modifier(parse_expression::operation op, vector<boolean::cover> args, tokenizer *tokens) const {
	if (op.is("", "'", "", "")) {
		return args[0];
	}
	return parse_expression::Importer<boolean::cover>::import_modifier(op, args, tokens);
}

boolean::cover CompositionImporter::import_binary(parse_expression::operation op, boolean::cover left, boolean::cover right, tokenizer *tokens) const {
	if (op.is("", "", ":", "")) {
		return left | right;
	} else if (op.is("", "", ",", "")) {
		return left & right;
	}
	internal("", "unrecognized operation", __FILE__, __LINE__);
	return left;
}

boolean::cube import_assignment(const assignment &syntax, ucs::Netlist nets, tokenizer *tokens, int region, bool auto_define) {
	return CompositionImporter(nets, region, auto_define).import_assignment(syntax, tokens);
}

boolean::cover import_composition(const parse_expression::expression &syntax, ucs::Netlist nets, tokenizer *tokens, int region, bool auto_define) {
	return CompositionImporter(nets, region, auto_define).import_expression(syntax, tokens);
}

}
