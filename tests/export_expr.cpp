#include "export_expr.h"
#include "expression.h"

namespace test {

ExpressionExporter::ExpressionExporter(ucs::ConstNetlist nets) : nets(nets) {
}

ExpressionExporter::~ExpressionExporter() {
}

parse_expression::operation ExpressionExporter::export_operator(int func) const {
	using operation = parse_expression::operation;

	switch (func) {
	case NOT: return operation("~", "", "", "");
	case INTERFERE: return operation("?", "", "", "");
	case AND: return operation("", "", "&", "");
	case OR: return operation("", "", "|", "");
	}
	return operation();
}

const parse_expression::precedence_set &ExpressionExporter::precedence() const {
	return test::expression_config::cfg->order;
}

parse_expression::expression::argument ExpressionExporter::export_constant(int value) const {
	constant result;
	result.value = boolean::export_value(value);
	return {0, std::shared_ptr<parse::syntax>(result.clone())};
}

parse_expression::expression::argument ExpressionExporter::export_literal(size_t index) const {
	literal result;
	result.name = nets.netAt(index);
	return {1, std::shared_ptr<parse::syntax>(result.clone())};
}

parse_expression::expression export_expression(boolean::cube expr, ucs::ConstNetlist nets) {
	return ExpressionExporter(nets).export_expression(expr);
}

parse_expression::expression export_expression(boolean::cover expr, ucs::ConstNetlist nets) {
	return ExpressionExporter(nets).export_expression(expr);
}

parse_expression::expression export_expression_xfactor(boolean::cover expr, ucs::ConstNetlist nets) {
	return ExpressionExporter(nets).export_expression_xfactor(expr);
}

parse_expression::expression export_expression_hfactor(boolean::cover expr, ucs::ConstNetlist nets) {
	return ExpressionExporter(nets).export_expression_hfactor(expr);
}

CompositionExporter::CompositionExporter(ucs::ConstNetlist nets) : nets(nets) {
}

CompositionExporter::~CompositionExporter() {
}

parse_expression::operation CompositionExporter::export_operator(int func) const {
	using operation = parse_expression::operation;

	switch (func) {
	case AND: return operation("", "", ",", "");
	case OR: return operation("", "", ":", "");
	}
	return operation();
}

const parse_expression::precedence_set &CompositionExporter::precedence() const {
	return test::expression_config::cfg->order;
}

parse_expression::expression::argument CompositionExporter::export_constant(int value) const {
	constant result;
	result.value = boolean::export_value(value);
	return {0, std::shared_ptr<parse::syntax>(result.clone())};
}

parse_expression::expression::argument CompositionExporter::export_literal(size_t index) const {
	literal result;
	result.name = nets.netAt(index);
	return {1, std::shared_ptr<parse::syntax>(result.clone())};
}

assignment CompositionExporter::export_assignment(size_t index, int value) const {
	assignment result;
	result.valid = true;
	if (value >= 2) {
		return result;
	}

	parse_expression::expression lvalue;
	lvalue.valid = true;
	lvalue.level = 0;
	lvalue.type = test::expression_config::cfg->order.type(0);
	lvalue.arguments.push_back(export_literal(index));
	result.left.push_back(lvalue);

	if (value == 0) {
		result.operation = "-";
	} else if (value == 1) {
		result.operation = "+";
	} else {
		result.operation = "~";
	}

	return result;
}

parse_expression::expression::argument CompositionExporter::export_term(size_t index, int value) const {
	return {1, std::shared_ptr<parse::syntax>(export_assignment(index, value).clone())};
}

parse_expression::expression export_composition(boolean::cube expr, ucs::ConstNetlist nets) {
	return CompositionExporter(nets).export_expression(expr);
}

parse_expression::expression export_composition(boolean::cover expr, ucs::ConstNetlist nets) {
	return CompositionExporter(nets).export_expression(expr);
}

parse_expression::expression export_composition_xfactor(boolean::cover expr, ucs::ConstNetlist nets) {
	return CompositionExporter(nets).export_expression_xfactor(expr);
}

parse_expression::expression export_composition_hfactor(boolean::cover expr, ucs::ConstNetlist nets) {
	return CompositionExporter(nets).export_expression_hfactor(expr);
}


}

