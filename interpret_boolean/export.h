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
#include <parse_boolean/internal_parallel.h>
#include <parse_boolean/internal_choice.h>

#include <parse_boolean/complement.h>
#include <parse_boolean/conjunction.h>
#include <parse_boolean/disjunction.h>

#ifndef interpret_boolean_export_h
#define interpret_boolean_export_h

parse_boolean::slice export_slice(int lb, int ub);
parse_boolean::member_name export_member_name(boolean::instance instance);
parse_boolean::variable_name export_variable_name(boolean::variable variable);
parse_boolean::variable_name export_variable_name(int variable, boolean::variable_set &variables);
vector<parse_boolean::variable_name> export_variable_names(boolean::variable_set &variables);

parse_boolean::assignment export_assignment(int variable, bool value, boolean::variable_set &variables);
parse_boolean::internal_parallel export_internal_parallel(boolean::cube c, boolean::variable_set &variables);
parse_boolean::internal_choice export_internal_choice(boolean::cover c, boolean::variable_set &variables);

parse_boolean::complement export_complement(bool value, boolean::variable_set &variables);
parse_boolean::complement export_complement(int variable, bool value, boolean::variable_set &variables);
parse_boolean::conjunction export_conjunction(boolean::cube c, boolean::variable_set &variables);
parse_boolean::disjunction export_disjunction(boolean::cover c, boolean::variable_set &variables);

#endif
