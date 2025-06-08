#include <gtest/gtest.h>
#include <parse/default/line_comment.h>
#include <parse/default/block_comment.h>
#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>
#include <sstream>
#include <string>

#include <boolean/cube.h>
#include <boolean/cover.h>
#include <interpret_boolean/import.h>
#include <interpret_boolean/export.h>
#include "mock_netlist.h"

using namespace std;
using namespace parse_expression;

//==============================================================================
// Round-Trip Import/Export Tests
//==============================================================================

TEST(Expression, CubeBasic) {
	// Test round-trip for simple variable: a
	string test_code = "a & b & ~c & ~d & e";

	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	parse_expression::expression::register_syntax(tokens);
	parse_expression::composition::register_syntax(tokens);
	parse_expression::assignment::register_syntax(tokens);
	tokens.insert("CubeBasic", test_code);

	MockNetlist nets;
	
	// Import
	expression input_expr(tokens);
	boolean::cube dut = boolean::import_cube(input_expr, nets, 0, &tokens, true);
	
	// Export
	expression output_expr = boolean::export_expression(dut, nets);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(output_expr.valid);
	EXPECT_EQ(output_expr.to_string(), "a&b&~c&~d&e");
}

TEST(Expression, CoverBasic) {
	// Test round-trip for simple variable: a
	string test_code = "a & b & ~c & ~d & e | a & ~b & ~c | ~d & ~e";

	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	parse_expression::expression::register_syntax(tokens);
	parse_expression::composition::register_syntax(tokens);
	parse_expression::assignment::register_syntax(tokens);
	tokens.insert("CoverBasic", test_code);

	MockNetlist nets;
	
	// Import
	expression input_expr(tokens);
	boolean::cover dut = boolean::import_cover(input_expr, nets, 0, &tokens, true);
	
	// Export
	expression output_expr = boolean::export_expression(dut, nets);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(output_expr.valid);
	EXPECT_EQ(output_expr.to_string(), "a&b&~c&~d&e|a&~b&~c|~d&~e");
}


