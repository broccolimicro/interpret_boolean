/*
 * export.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include <common/standard.h>

#include <boolean/cube.h>
#include <boolean/cover.h>

#include <parse_expression/composition.h>
#include <parse_expression/expression.h>

#include <interpret_ucs/export.h>

#ifndef interpret_boolean_export_h
#define interpret_boolean_export_h

parse_expression::assignment export_assignment(int uid, int value, const ucs::variable_set &variables);

parse_expression::composition export_composition(boolean::cube c, const ucs::variable_set &variables);
parse_expression::composition export_composition(boolean::cover c, const ucs::variable_set &variables);

parse_expression::expression export_expression(int uid, int value, const ucs::variable_set &variables);
parse_expression::expression export_expression(boolean::cube c, const ucs::variable_set &variables);
parse_expression::expression export_expression(boolean::cover c, const ucs::variable_set &variables);

parse_expression::expression export_expression_xfactor(boolean::cover c, const ucs::variable_set &variables, int level = 0);
parse_expression::expression export_expression_hfactor(boolean::cover c, const ucs::variable_set &variables);

#endif
