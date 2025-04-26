#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <boolean/cube.h>
#include <boolean/cover.h>
#include <boolean/unsigned_int.h>

#include <parse_expression/composition.h>
#include <parse_expression/expression.h>

namespace boolean {

int import_net(const parse_ucs::variable_name &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);

boolean::cube import_cube(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::unsigned_int import_unsigned_int(const parse_expression::expression &syntax, map<string, boolean::unsigned_int> &variables, int default_id, tokenizer *tokens);

}
