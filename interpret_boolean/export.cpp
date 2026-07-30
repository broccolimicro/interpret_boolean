#include "export.h"

namespace boolean {

string export_value(int v) {
	if (v == 2) {
		return "unknown";
	} else if (v == 1) {
		return "vdd";
	} else if (v == 0) {
		return "gnd";
	} else if (v == -1) {
		return "unstable";
	}
	return "undef";
}

parse_expression::expression::argument ExpressionExporter::export_constant(int value) const {
	internal("", "constant export not defined", __FILE__, __LINE__);
	return {-1, nullptr};
}

parse_expression::expression::argument ExpressionExporter::export_literal(size_t index) const {
	internal("", "literal export not defined", __FILE__, __LINE__);
	return {-1, nullptr};
}

parse_expression::expression::argument ExpressionExporter::export_term(size_t uid, int value) const {
	const parse_expression::precedence_set &order = precedence();

	if (value == 0) {
		auto notOp = export_operator(NOT);
		if (notOp.empty()) {
			internal("", "not operator not defined", __FILE__, __LINE__);
			return export_literal(uid);
		}

		auto notIdx = order.find(-1, notOp);

		parse_expression::expression result;
		result.valid = true;
		result.level = notIdx.level;
		result.type = order.type(notIdx.level);
		result.arguments.push_back(export_literal(uid));
		result.operators.push_back(notOp);
		return {-1, std::shared_ptr<parse::syntax>(result.clone())};
	} else if (value == -1) {
		auto invOp = export_operator(INTERFERE);
		if (invOp.empty()) {
			internal("", "inv operator not defined", __FILE__, __LINE__);
			return export_literal(uid);
		}

		auto invIdx = order.find(-1, invOp);

		parse_expression::expression result;
		result.valid = true;
		result.level = invIdx.level;
		result.type = order.type(invIdx.level);
		result.arguments.push_back(export_literal(uid));
		result.operators.push_back(invOp);
		return {-1, std::shared_ptr<parse::syntax>(result.clone())};
	}

	return export_literal(uid);
}

parse_expression::expression ExpressionExporter::export_expression(boolean::cube expr) const {
	const parse_expression::precedence_set &order = precedence();

	parse_expression::expression result;
	result.valid = true;

	auto andOp = export_operator(AND);
	if (andOp.empty()) {
		internal("", "and operator not defined", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	auto andIdx = order.find(-1, andOp);

	result.level = andIdx.level;
	result.type = order.type(result.level);

	for (int uid = 0; uid < expr.size()*16; uid++) {
		int val = expr.get(uid);
		if (val < 2) {
			result.arguments.push_back(export_term(uid, val));
		}
	}

	if (result.arguments.size() == 0) {
		result.arguments.push_back(export_constant(1));
	}

	for (int i = 1; i < (int)result.arguments.size(); i++) {
		result.operators.push_back(andOp);
	}

	return result;
}

parse_expression::expression ExpressionExporter::export_expression(boolean::cover expr) const {
	const parse_expression::precedence_set &order = precedence();

	parse_expression::expression result;
	result.valid = true;

	auto orOp = export_operator(OR);
	if (orOp.empty()) {
		internal("", "or operator not defined", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	auto orIdx = order.find(-1, orOp);

	result.level = orIdx.level;
	result.type = order.type(result.level);

	for (int i = 0; i < (int)expr.cubes.size(); i++) {
		result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression(expr.cubes[i]).clone())});
	}

	if (expr.cubes.size() == 0) {
		result.arguments.push_back(export_constant(0));
	}

	for (int i = 1; i < (int)result.arguments.size(); i++) {
		result.operators.push_back(orOp);
	}

	return result;
}

parse_expression::expression ExpressionExporter::export_expression_xfactor(boolean::cover c, int op) const {
	const parse_expression::precedence_set &order = precedence();

	auto andOp = export_operator(AND);
	if (andOp.empty()) {
		internal("", "and operator not defined", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	auto andIdx = order.find(-1, andOp);
	int andType = order.type(andIdx.level);

	auto orOp = export_operator(OR);
	if (orOp.empty()) {
		internal("", "or operator not defined", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	auto orIdx = order.find(-1, orOp);
	int orType = order.type(orIdx.level);

	if (op < AND) {
		op = OR;
	}

	parse_expression::expression result;
	result.level = orIdx.level;
	result.type = orType;
	if (op == AND) {
		result.level = andIdx.level;
		result.type = andType;
	}
	result.valid = true;

	boolean::cover nc = ~c;

	if (c.cubes.size() == 0) {
		result.arguments.push_back(export_constant(0));
	} else if (nc.cubes.size() == 0) {
		result.arguments.push_back(export_constant(1));
	} else if (c.cubes.size() == 1 or nc.cubes.size() == 1) {
		if (op == AND) {
			c = nc;
			op = OR;
			result.level = orIdx.level;
			result.type = orType;
		}

		for (int i = 0; i < (int)c.cubes.size(); i++) {
			result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression(c.cubes[i]).clone())});
		}
		for (int i = 1; i < (int)result.arguments.size(); i++) {
			result.operators.push_back(op == AND ? andOp : orOp);
		}
	} else {
		boolean::cover c_left, c_right, nc_left, nc_right;
		float c_weight, nc_weight;

		c_weight = c.partition(c_left, c_right);
		nc_weight = nc.partition(nc_left, nc_right);

		if (c_weight <= nc_weight) {
			result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression_xfactor(c_left, op).clone())});
			result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression_xfactor(c_right, op).clone())});
			result.operators.push_back(op == AND ? andOp : orOp);
		} else if (nc_weight < c_weight) {
			op = (op == AND ? OR : AND);
			if (op == OR) {
				result.level = orIdx.level;
				result.type = orType;
			} else if (op == AND) {
				result.level = andIdx.level;
				result.type = andType;
			}
			result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression_xfactor(nc_left, op).clone())});
			result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression_xfactor(nc_right, op).clone())});
			result.operators.push_back(op == AND ? andOp : orOp);
		}
	}

	for (int i = (int)result.arguments.size()-1; i >= 0; i--) {
		if (result.arguments[i].type < 0 and result.arguments[i].ptr->valid) {
			const parse_expression::expression &sub = result.arguments[i].ptr->get<parse_expression::expression>();
			if (sub.level == result.level or (sub.arguments.size() == 1u and sub.operators.size() == 0u)) {
				result.arguments.insert(result.arguments.begin() + i+1, sub.arguments.begin(), sub.arguments.end());
				result.operators.insert(result.operators.begin() + i, sub.operators.begin(), sub.operators.end());
				result.arguments.erase(result.arguments.begin() + i);
			}
		}
	}

	return result;
}

parse_expression::expression ExpressionExporter::export_expression_hfactor(boolean::cover c) const {
	const parse_expression::precedence_set &order = precedence();

	auto andOp = export_operator(AND);
	if (andOp.empty()) {
		internal("", "and operator not defined", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	auto andIdx = order.find(-1, andOp);
	int andType = order.type(andIdx.level);

	auto orOp = export_operator(OR);
	if (orOp.empty()) {
		internal("", "or operator not defined", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	auto orIdx = order.find(-1, orOp);
	int orType = order.type(orIdx.level);

	parse_expression::expression result;
	result.valid = true;
	result.level = orIdx.level;
	result.type = orType;

	if (c.is_null()) {
		result.arguments.push_back(export_constant(0));
	} else if (c.is_tautology()) {
		result.arguments.push_back(export_constant(1));
	} else if (c.cubes.size() == 1) {
		result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression(c.cubes[0]).clone())});
	} else {
		boolean::cube common = c.supercube();
		if (common == 1) {
			boolean::cover c_left, c_right;
			c.partition(c_left, c_right);

			result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression_hfactor(c_left).clone())});
			result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression_hfactor(c_right).clone())});
			result.operators.push_back(orOp);
		} else {
			c.cofactor(common);

			result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression(common).clone())});
			result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression_hfactor(c).clone())});
			result.operators.push_back(andOp);
			result.level = andIdx.level;
			result.type = andType;
		}
	}

	for (int i = (int)result.arguments.size()-1; i >= 0; i--) {
		if (result.arguments[i].type < 0 and result.arguments[i].ptr->valid) {
			const parse_expression::expression &sub = result.arguments[i].ptr->get<parse_expression::expression>();
			if (sub.level == result.level or (sub.arguments.size() == 1u and sub.operators.size() == 0u)) {
				result.arguments.insert(result.arguments.begin() + i+1, sub.arguments.begin(), sub.arguments.end());
				result.operators.insert(result.operators.begin() + i, sub.operators.begin(), sub.operators.end());
				result.arguments.erase(result.arguments.begin() + i);
			}
		}
	}

	return result;
}

}

