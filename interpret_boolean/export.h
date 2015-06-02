/*
 * export.h
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

#ifndef interpret_boolean_export_h
#define interpret_boolean_export_h

parse_boolean::slice export_slice(int lb, int ub);
parse_boolean::member_name export_member_name(boolean::instance instance);
parse_boolean::variable_name export_variable_name(boolean::variable variable);
parse_boolean::variable_name export_variable_name(int variable, const boolean::variable_set &variables);
vector<parse_boolean::variable_name> export_variable_names(const boolean::variable_set &variables);

parse_boolean::assignment export_assignment(boolean::cube c, const boolean::variable_set &variables);
parse_boolean::assignment export_assignment(boolean::cover c, const boolean::variable_set &variables);
parse_boolean::guard export_guard(boolean::cube c, const boolean::variable_set &variables);
parse_boolean::guard export_guard_xfactor(boolean::cover c, const boolean::variable_set &variables, int op = parse_boolean::guard::OR);
parse_boolean::guard export_guard_hfactor(boolean::cover c, const boolean::variable_set &variables);
parse_boolean::guard export_guard(boolean::cover c, const boolean::variable_set &variables);

#endif
