#include "export.h"

namespace boolean {

parse_expression::expression export_field(string str) {
	static const auto op = parse_expression::expression::precedence.find(parse_expression::operation_set::MODIFIER, "", "[", ":", "]");

	parse_expression::expression result;
	if (op.level < 0 or op.index < 0) {
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

	result.arguments.push_back(parse_expression::argument::literalOf(name));
	while (open != string::npos and open < str.size()) {
		open += 1;
		size_t close = str.find(']', open);
		result.arguments.push_back(parse_expression::argument::constantOf(str.substr(open, close-open)));
		open = close+1;
	}

	return result;
}

parse_expression::expression export_member(string str) {
	static const auto op = parse_expression::expression::precedence.find(parse_expression::operation_set::MODIFIER, "", ".", "", "");
	
	parse_expression::expression result;
	if (op.level < 0 or op.index < 0) {
		return result;
	}
 
	result.valid = true;
	result.level = op.level;

	if (not str.empty()) {
		size_t prev = 0u;
		size_t dot = str.find('.', prev);
		while (dot != string::npos and dot < str.size()) {
			result.arguments.push_back(export_field(str.substr(prev, dot-prev)));
			result.operators.push_back(op.index);
			prev = dot+1;
			dot = str.find('.', prev);
		}
		result.arguments.push_back(export_field(str.substr(prev)));
	}

	return result;
}

parse_expression::expression export_net(string str) {
	static const auto op = parse_expression::expression::precedence.find(parse_expression::operation_set::MODIFIER, "", "'", "", "");

	parse_expression::expression result;
	if (op.level < 0 or op.index < 0) {
		return result;
	}
 
	result.valid = true;
	result.level = op.level;

	size_t tic = str.rfind('\'');
	if (tic != string::npos) {
		string region = str.substr(tic+1);
		str = str.substr(0, tic);
		result.operators.push_back(op.index);
		result.arguments.push_back(export_member(str));
		result.arguments.push_back(parse_expression::argument::constantOf(region));
	} else {
		result.arguments.push_back(export_member(str));
	}

	return result;
}


parse_expression::expression export_net(int uid, ucs::ConstNetlist nets) {
	string name = nets.netAt(uid);
	if (name.empty()) {
		return parse_expression::expression();
	}

	return export_net(name);
}

parse_expression::assignment export_assignment(int uid, int value, ucs::ConstNetlist nets) {
	parse_expression::assignment result;
	result.valid = true;

	result.lvalue.push_back(export_net(uid, nets));
	if (value == 0)
		result.operation = "-";
	else if (value == 1)
		result.operation = "+";
	else if (value == -1)
		result.operation = "~";

	return result;
}

parse_expression::composition export_composition(boolean::cube c, ucs::ConstNetlist nets) {
	parse_expression::composition::init();
	static const int level = parse_expression::composition::get_level(",");

	parse_expression::composition result;
	result.valid = true;

	result.level = level;

	for (int uid = 0; uid < c.size()*16; uid++) {
		int val = c.get(uid);
		if (val != 2) {
			result.literals.push_back(export_assignment(uid, val, nets));
		}
	}

	return result;
}

parse_expression::composition export_composition(boolean::cover c, ucs::ConstNetlist nets) {
	parse_expression::composition::init();
	static const int level = parse_expression::composition::get_level(":");

	parse_expression::composition result;
	result.valid = true;

	result.level = level;

	for (int i = 0; i < (int)c.cubes.size(); i++) {
		result.compositions.push_back(export_composition(c.cubes[i], nets));
	}

	return result;
}

parse_expression::expression export_expression(int uid, int value, ucs::ConstNetlist nets) {
	static const auto NOT = parse_expression::expression::precedence.find(parse_expression::operation_set::UNARY, "~", "", "", "");
	static const auto INV = parse_expression::expression::precedence.find(parse_expression::operation_set::UNARY, "?", "", "", "");

	parse_expression::expression result;
	result.valid = true;

	result.level = NOT.level;

	result.arguments.push_back(parse_expression::argument(export_net(uid, nets)));
	if (value == 0) {
		result.level = NOT.level;
		result.operators.push_back(NOT.index);
	} else if (value == -1) {
		result.level = INV.level;
		result.operators.push_back(INV.index);
	}

	return result;
}

parse_expression::expression export_expression(boolean::cube c, ucs::ConstNetlist nets) {
	static const auto AND = parse_expression::expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "&", "");
	parse_expression::expression result;
	result.valid = true;

	result.level = AND.level;

	for (int uid = 0; uid < c.size()*16; uid++) {
		int val = c.get(uid);
		if (val != 2) {
			result.arguments.push_back(parse_expression::argument(export_expression(uid, val, nets)));
		}
	}

	if (result.arguments.size() == 0) {
		result.arguments.push_back(parse_expression::argument::constantOf("1"));
	}

	for (int i = 1; i < (int)result.arguments.size(); i++) {
		result.operators.push_back(AND.index);
	}

	return result;
}

parse_expression::expression export_expression(boolean::cover c, ucs::ConstNetlist nets) {
	static const auto OR = parse_expression::expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "|", "");
	parse_expression::expression result;
	result.valid = true;

	result.level = OR.level;

	for (int i = 0; i < (int)c.cubes.size(); i++) {
		result.arguments.push_back(parse_expression::argument(export_expression(c.cubes[i], nets)));
	}

	if (c.cubes.size() == 0) {
		result.arguments.push_back(parse_expression::argument::constantOf("0"));
	}

	for (int i = 1; i < (int)result.arguments.size(); i++) {
		result.operators.push_back(OR.index);
	}

	return result;
}

parse_expression::expression export_expression_xfactor(boolean::cover c, ucs::ConstNetlist nets, int level) {
	static const auto AND = parse_expression::expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "&", "");
	static const auto OR = parse_expression::expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "|", "");

	parse_expression::expression result;
	result.level = level < 0 ? OR.level : level;
	result.valid = true;

	boolean::cover nc = ~c;

	if (c.cubes.size() == 0) {
		result.arguments.push_back(parse_expression::argument::constantOf("0"));
	} else if (nc.cubes.size() == 0) {
		result.arguments.push_back(parse_expression::argument::constantOf("1"));
	} else if (c.cubes.size() == 1 || nc.cubes.size() == 1) {
		if (level == AND.level) {
			c = nc;
			result.level = OR.level;
		}

		for (int i = 0; i < (int)c.cubes.size(); i++) {
			result.arguments.push_back(parse_expression::argument(export_expression(c.cubes[i], nets)));
		}
		for (int i = 1; i < (int)result.arguments.size(); i++) {
			result.operators.push_back(result.level == AND.level ? AND.index : OR.index);
		}
	} else {
		boolean::cover c_left, c_right, nc_left, nc_right;
		float c_weight, nc_weight;

		c_weight = c.partition(c_left, c_right);
		nc_weight = nc.partition(nc_left, nc_right);

		if (c_weight <= nc_weight) {
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(c_left, nets, result.level)));
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(c_right, nets, result.level)));
			result.operators.push_back(result.level == AND.level ? AND.index : OR.index);
		} else if (nc_weight < c_weight) {
			result.level = result.level == AND.level ? OR.level : AND.level;
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(nc_left, nets, result.level)));
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(nc_right, nets, result.level)));
			result.operators.push_back(result.level == AND.level ? AND.index : OR.index);
		}
	}

	for (int i = 0; i < (int)result.arguments.size(); ) {
		if (result.arguments[i].sub.valid && (result.arguments[i].sub.level == result.level || (result.arguments[i].sub.arguments.size() == 1 && result.arguments[i].sub.operators.size() == 0))) {
			result.arguments.insert(result.arguments.begin() + i+1, result.arguments[i].sub.arguments.begin(), result.arguments[i].sub.arguments.end());
			result.operators.insert(result.operators.begin() + i, result.arguments[i].sub.operators.begin(), result.arguments[i].sub.operators.end());
			result.arguments.erase(result.arguments.begin() + i);
		} else {
			i++;
		}
	}

	return result;
}

parse_expression::expression export_expression_hfactor(boolean::cover c, ucs::ConstNetlist nets) {
	static const auto AND = parse_expression::expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "&", "");
	static const auto OR = parse_expression::expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "|", "");

	parse_expression::expression result;
	result.valid = true;
	result.level = OR.level;

	if (c.is_null()) {
		result.arguments.push_back(parse_expression::argument::constantOf("0"));
	} else if (c.is_tautology()) {
		result.arguments.push_back(parse_expression::argument::constantOf("1"));
	} else if (c.cubes.size() == 1) {
		result.arguments.push_back(parse_expression::argument(export_expression(c.cubes[0], nets)));
	} else {
		boolean::cube common = c.supercube();
		if (common == 1) {
			boolean::cover c_left, c_right;
			c.partition(c_left, c_right);

			result.arguments.push_back(parse_expression::argument(export_expression_hfactor(c_left, nets)));
			result.arguments.push_back(parse_expression::argument(export_expression_hfactor(c_right, nets)));
			result.operators.push_back(OR.index);
		} else {
			c.cofactor(common);

			result.arguments.push_back(parse_expression::argument(export_expression(common, nets)));
			result.arguments.push_back(parse_expression::argument(export_expression_hfactor(c, nets)));
			result.operators.push_back(AND.index);
			result.level = AND.level;
		}
	}

	for (int i = 0; i < (int)result.arguments.size(); ) {
		if (result.arguments[i].sub.valid && (result.arguments[i].sub.level == result.level || (result.arguments[i].sub.arguments.size() == 1 && result.arguments[i].sub.operators.size() == 0))) {
			result.arguments.insert(result.arguments.begin() + i+1, result.arguments[i].sub.arguments.begin(), result.arguments[i].sub.arguments.end());
			result.operators.insert(result.operators.begin() + i, result.arguments[i].sub.operators.begin(), result.arguments[i].sub.operators.end());
			result.arguments.erase(result.arguments.begin() + i);
		} else {
			i++;
		}
	}

	return result;
}

}
