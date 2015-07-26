/*
 * import.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include <common/standard.h>

#include <boolean/cube.h>
#include <boolean/cover.h>

#include <parse_expression/composition.h>
#include <parse_expression/expression.h>
#include <ucs/variable.h>
#include <interpret_ucs/import.h>

#ifndef interpret_boolean_import_h
#define interpret_boolean_import_h

boolean::cube import_cube(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_expression::expression &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_expression::expression &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define = false);

#endif
