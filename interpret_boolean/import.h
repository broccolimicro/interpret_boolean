/*
 * import.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include <common/standard.h>

#include <boolean/variable.h>
#include <boolean/cube.h>
#include <boolean/cover.h>

#include <parse_boolean/variable_name.h>
#include <parse_boolean/assignment.h>
#include <parse_boolean/guard.h>

#ifndef interpret_boolean_import_h
#define interpret_boolean_import_h

vector<boolean::instance> import_instances(const parse_boolean::member_name &syntax, tokenizer *tokens);
vector<boolean::variable> import_variables(const parse_boolean::variable_name &syntax, tokenizer *tokens);

vector<int> define_variables(const parse_boolean::variable_name &syntax, boolean::variable_set &variables, tokenizer *tokens, bool define = false, bool squash_errors = false);

boolean::cover import_cover(const parse_boolean::assignment &syntax, boolean::variable_set &variables, tokenizer *tokens, bool auto_define = false);
boolean::cover import_cover(const parse_boolean::guard &syntax, boolean::variable_set &variables, tokenizer *tokens, bool auto_define = false);

boolean::cube import_cube(const parse_boolean::assignment &syntax, boolean::variable_set &variables, tokenizer *tokens, bool auto_define = false);
boolean::cube import_cube(const parse_boolean::guard &syntax, boolean::variable_set &variables, tokenizer *tokens, bool auto_define = false);

#endif
