#include "helpers.h"
#include <parse_expression/precedence.h>

using parse_expression::precedence_set;
using parse_expression::operation_set;

void setup_expressions() {
	if (expression::precedence.empty()) {
		precedence_set result;
		result.push(operation_set::BINARY);
		result.push_back("", "", "|", "");

		result.push(operation_set::BINARY);
		result.push_back("", "", "&", "");

		result.push(operation_set::UNARY);
		result.push_back("~", "", "", "");
		result.push_back("?", "", "", "");

		result.push(operation_set::MODIFIER);
		result.push_back("", "'", "", "");

		result.push(operation_set::MODIFIER);
		result.push_back("", "(", ",", ")");
		result.push_back("", ".", "", "");
		result.push_back("", "[", ":", "]");

		result.push(operation_set::MODIFIER);
		result.push_back("", "::", "", "");
		
		expression::register_precedence(result);
		assignment::lvalueLevel = result.size()-3;
	}
}
