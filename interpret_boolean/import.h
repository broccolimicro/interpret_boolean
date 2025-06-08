#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <boolean/cube.h>
#include <boolean/cover.h>
#include <boolean/unsigned_int.h>

#include <parse_expression/composition.h>
#include <parse_expression/expression.h>

namespace boolean {

// Instead of expanding while I interpret, I should probably expand as a
// separate step. Therefore, this interpreter code should only accept parse
// trees without arithmetic operations or more complex types. That
// transformation should happen outside of this code.

string import_constant(const parse_expression::expression &syntax, tokenizer *tokens);
string import_constant(const parse_expression::argument &syntax, tokenizer *tokens);
string import_net_name(const parse_expression::argument &syntax, tokenizer *tokens);
string import_net_name(const parse_expression::expression &syntax, tokenizer *tokens);

int import_net(string syntax, ucs::Netlist nets, tokenizer *tokens, bool auto_define);
int import_net(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);

boolean::cube import_cube(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::unsigned_int import_unsigned_int(const parse_expression::expression &syntax, map<string, boolean::unsigned_int> &variables, int default_id, tokenizer *tokens);

}
