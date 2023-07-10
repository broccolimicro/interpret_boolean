/*
 * import.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#pragma once

#include <common/standard.h>

#include <boolean/cube.h>
#include <boolean/cover.h>
#include <boolean/signed_int.h>

#include <parse_expression/composition.h>
#include <parse_expression/expression.h>
#include <ucs/variable.h>
#include <interpret_ucs/import.h>

boolean::cube import_cube(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::expression &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::expression &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::signed_int import_signed_int(const parse_expression::expression &syntax, map<string, boolean::signed_int> &variables, int default_id, tokenizer *tokens);

