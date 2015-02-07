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
#include <parse_boolean/constant.h>
#include <parse_boolean/assignment.h>
#include <parse_boolean/guard.h>

#ifndef interpret_boolean_export_h
#define interpret_boolean_export_h

parse_boolean::variable_name export_variable(int variable, boolean::variable_set &variables);
vector<parse_boolean::variable_name> export_variables(boolean::variable_set &variables);
vector<parse_boolean::variable_name> export_variables(boolean::cube c, boolean::variable_set &variables);
vector<parse_boolean::variable_name> export_variables(boolean::cover c, boolean::variable_set &variables);
parse_boolean::constant export_constant(boolean::cover c);
parse_boolean::constant export_constant(boolean::cube c);
vector<parse_boolean::assignment> export_assignments(boolean::cube c, boolean::variable_set &variables);
vector<vector<parse_boolean::assignment> > export_assignments(boolean::cover c, boolean::variable_set &variables);
parse_boolean::guard export_guard(int variable, bool value, boolean::variable_set &variables);
parse_boolean::guard export_guard(boolean::cube c, boolean::variable_set &variables);
parse_boolean::guard export_guard(boolean::cover c, boolean::variable_set &variables);

#endif
