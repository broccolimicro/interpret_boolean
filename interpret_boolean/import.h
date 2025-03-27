#pragma once

#include <common/standard.h>

#include <boolean/cube.h>
#include <boolean/cover.h>
#include <boolean/unsigned_int.h>

#include <parse_expression/composition.h>
#include <parse_expression/expression.h>

#include "interface.h"

namespace boolean {

int import_net(const parse_ucs::variable_name &syntax, boolean::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);

boolean::cube import_cube(const parse_expression::assignment &syntax, boolean::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::assignment &syntax, boolean::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::composition &syntax, boolean::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::composition &syntax, boolean::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::expression &syntax, boolean::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::expression &syntax, boolean::Netlist nets, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::unsigned_int import_unsigned_int(const parse_expression::expression &syntax, map<string, boolean::unsigned_int> &variables, int default_id, tokenizer *tokens);

}
