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

#include <parse_boolean/internal_choice.h>
#include <parse_boolean/internal_parallel.h>
#include <parse_boolean/assignment.h>

#include <parse_boolean/disjunction.h>
#include <parse_boolean/conjunction.h>
#include <parse_boolean/complement.h>

#ifndef interpret_boolean_import_h
#define interpret_boolean_import_h

vector<boolean::instance> import_instances(tokenizer &tokens, const parse_boolean::member_name &syntax);
vector<boolean::variable> import_variables(tokenizer &tokens, const parse_boolean::variable_name &syntax);

vector<int> define_variables(vector<boolean::variable> v, boolean::variable_set &variables, bool define = false, bool squash_errors = false);

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::assignment &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cover import_cover(tokenizer &tokens, const parse_boolean::internal_parallel &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cover import_cover(tokenizer &tokens, const parse_boolean::internal_choice &syntax, boolean::variable_set &variables, bool auto_define = false);

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::complement &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cover import_cover(tokenizer &tokens, const parse_boolean::conjunction &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cover import_cover(tokenizer &tokens, const parse_boolean::disjunction &syntax, boolean::variable_set &variables, bool auto_define = false);

boolean::cube import_cube(tokenizer &tokens, const parse_boolean::assignment &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cube import_cube(tokenizer &tokens, const parse_boolean::internal_parallel &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cube import_cube(tokenizer &tokens, const parse_boolean::internal_choice &syntax, boolean::variable_set &variables, bool auto_define = false);

boolean::cube import_cube(tokenizer &tokens, const parse_boolean::complement &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cube import_cube(tokenizer &tokens, const parse_boolean::conjunction &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cube import_cube(tokenizer &tokens, const parse_boolean::disjunction &syntax, boolean::variable_set &variables, bool auto_define = false);

#endif
