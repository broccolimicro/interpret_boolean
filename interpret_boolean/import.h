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
#include <parse_boolean/constant.h>
#include <parse_boolean/assignment.h>
#include <parse_boolean/guard.h>

#ifndef interpret_boolean_import_h
#define interpret_boolean_import_h

int import_variable(const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define = false, bool squash_errors = false);
boolean::cube import_cube(const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define = false, bool squash_errors = false);
boolean::cover import_cover(const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define = false, bool squash_errors = false);
boolean::cover import_cover(const parse_boolean::constant &syntax);
boolean::cube import_cube(const parse_boolean::constant &syntax);
boolean::cube import_cube(const parse_boolean::assignment &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cover import_cover(const parse_boolean::assignment &syntax, boolean::variable_set &variables, bool auto_define = false);
boolean::cover import_cover(const parse_boolean::guard &syntax, boolean::variable_set &variables, bool auto_define = false);

#endif
