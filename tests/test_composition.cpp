#include <gtest/gtest.h>

#include <string>

#include <boolean/cube.h>
#include <boolean/cover.h>

#include <common/mock_netlist.h>

#include <parse/default/line_comment.h>
#include <parse/default/block_comment.h>
#include "expression.h"
#include "import_expr.h"
#include "export_expr.h"

using namespace std;
using namespace parse_expression;

TEST(Composition, CubeBasic) {
	string test_code = "a+, b+ , c- , d- , e+";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("CubeBasic", test_code);

	MockNetlist nets;
	
	// Import
	test::composition input_comp(tokens);
	boolean::cover dut = test::import_composition(input_comp, nets, &tokens, 0, true);
	
	// Export
	test::composition output_expr = test::export_composition(dut, nets);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(output_expr.valid);
	EXPECT_EQ(output_expr.to_string(), "a+,b+,c-,d-,e+");
}

TEST(Composition, CubeDisjunction) {
	string test_code = "a+, b+ , c- , d- , e+ : a-,b-";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("CubeMultiple", test_code);

	MockNetlist nets;
	
	// Import
	test::composition input_comp(tokens);
	boolean::cover dut = test::import_composition(input_comp, nets, &tokens, 0, true);
	
	// Export
	test::composition output_expr = test::export_composition(dut, nets);
	
	EXPECT_FALSE(tokens.is_clean());
	EXPECT_TRUE(output_expr.valid);
	EXPECT_EQ(output_expr.to_string(), "skip");
}


TEST(Composition, CoverBasic) {
	string test_code = "a+,b+,c-, d-, e+ : a+, b- , c- : d- ,e-";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("CoverBasic", test_code);

	MockNetlist nets;
	
	// Import
	test::composition input_expr(tokens);
	boolean::cover dut = test::import_composition(input_expr, nets, &tokens, 0, true);
	
	// Export
	test::composition output_expr = test::export_composition(dut, nets);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(output_expr.valid);
	EXPECT_EQ(output_expr.to_string(), "a+,b+,c-,d-,e+:a+,b-,c-:d-,e-");
}

TEST(Composition, CoverSingleVariable) {
	string test_code = "a+";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("CoverSingleVariable", test_code);

	MockNetlist nets;
	
	// Import
	test::composition input_expr(tokens);
	boolean::cover dut = test::import_composition(input_expr, nets, &tokens, 0, true);
	
	// Export
	test::composition output_expr = test::export_composition(dut, nets);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(output_expr.valid);
	EXPECT_EQ(output_expr.to_string(), "a+");
}

TEST(Composition, CoverSingleInterference) {
	string test_code = "a~";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("CoverSingleInterference", test_code);

	MockNetlist nets;
	
	// Import
	test::composition input_expr(tokens);
	boolean::cover dut = test::import_composition(input_expr, nets, &tokens, 0, true);

	// Export
	test::composition output_expr = test::export_composition(dut, nets);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(output_expr.valid);
	EXPECT_EQ(output_expr.to_string(), "a~");
}

TEST(Composition, CoverBasicInterference) {
	string test_code = "a+,b+,c~, d-, e+ : a+, b- , c- : d- ,e-";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("CoverBasicInterference", test_code);

	MockNetlist nets;
	
	// Import
	test::composition input_expr(tokens);
	boolean::cover dut = test::import_composition(input_expr, nets, &tokens, 0, true);
	
	// Export
	test::composition output_expr = test::export_composition(dut, nets);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(output_expr.valid);
	EXPECT_EQ(output_expr.to_string(), "a+,b+,c~,d-,e+:a+,b-,c-:d-,e-");
}

TEST(Composition, CoverCompoundInterference) {
	string test_code = "(a+,b+,c~, d-, e+ : a+, b- , c- : d- ,e-),x+";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("CoverCompoundInterference", test_code);

	MockNetlist nets;
	
	// Import
	test::composition input_expr(tokens);
	boolean::cover dut = test::import_composition(input_expr, nets, &tokens, 0, true);
	
	// Export
	test::composition output_expr = test::export_composition(dut, nets);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(output_expr.valid);
	EXPECT_EQ(output_expr.to_string(), "x+,a+,b+,c~,d-,e+:x+,a+,b-,c-:x+,d-,e-");
}

