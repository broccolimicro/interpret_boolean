/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"

// parse_boolean::variable_name

int import_variable(const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define, bool squash_errors)
{
	boolean::variable v;
	for (int i = 0; i < (int)syntax.names.size(); i++)
		v.name.push_back(boolean::instance(syntax.names[i], vector<int>()));

	int result;
	if (define)
	{
		result = variables.define(v);

		if (result == -1 && !squash_errors)
			error("", "variable '" + v.to_string() + "' previously defined here", __FILE__, __LINE__);
		else if (result == -1 && squash_errors)
			result = variables.find(v);
	}
	else
	{
		result = variables.find(v);

		if (result == -1 && !squash_errors)
			error("", "variable '" + v.to_string() + "' not yet defined", __FILE__, __LINE__);
		else if (result == -1 && squash_errors)
			result = variables.define(v);
	}

	return result;
}

boolean::cube import_cube(const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define, bool squash_errors)
{
	return boolean::cube(import_variable(syntax, variables, define, squash_errors), 1);
}

boolean::cover import_cover(const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define, bool squash_errors)
{
	return boolean::cover(import_variable(syntax, variables, define, squash_errors), 1);
}

// parse_boolean::constant

boolean::cover import_cover(const parse_boolean::constant &syntax)
{
	if (syntax.value == "0")
		return boolean::cover(0);
	else if (syntax.value == "1")
		return boolean::cover(1);
	else
	{
		error("", "invalid constant value '" + syntax.value + "'", __FILE__, __LINE__);
		return boolean::cube();
	}
}

boolean::cube import_cube(const parse_boolean::constant &syntax)
{
	if (syntax.value == "0")
		return boolean::cube(0);
	else if (syntax.value == "1")
		return boolean::cube(1);
	else
	{
		error("", "invalid constant value '" + syntax.value + "'", __FILE__, __LINE__);
		return boolean::cube();
	}
}

// parse_boolean::assignment

boolean::cube import_cube(const parse_boolean::assignment &syntax, boolean::variable_set &variables, bool auto_define)
{
	int uid = import_variable(syntax.variable, variables, auto_define, auto_define);
	if (uid != -1)
		return boolean::cube(uid, syntax.value);
	else
		return boolean::cube();
}

boolean::cover import_cover(const parse_boolean::assignment &syntax, boolean::variable_set &variables, bool auto_define)
{
	return boolean::cover(import_cube(syntax, variables, auto_define));
}

// parse_boolean::guard

boolean::cover import_cover(const parse_boolean::guard &syntax, boolean::variable_set &variables, bool auto_define)
{
	boolean::cover result;
	if (syntax.operands.size() > 0)
	{
		if (syntax.operands[0]->is_a<parse_boolean::guard>())
			result = import_cover(*(parse_boolean::guard*)syntax.operands[0], variables, auto_define);
		else if (syntax.operands[0]->is_a<parse_boolean::variable_name>())
			result = import_cover(*(parse_boolean::variable_name*)syntax.operands[0], variables, auto_define);
		else if (syntax.operands[0]->is_a<parse_boolean::constant>())
			result = import_cover(*(parse_boolean::constant*)syntax.operands[0]);
	}

	for (int i = 0; i < (int)syntax.functions.size(); i++)
	{
		if (syntax.functions[i] == "~")
			result = ~result;
		else if (syntax.functions[i] == "&")
		{
			if (syntax.operands[i+1]->is_a<parse_boolean::guard>())
				result &= import_cover(*(parse_boolean::guard*)syntax.operands[i+1], variables, auto_define);
			else if (syntax.operands[i+1]->is_a<parse_boolean::variable_name>())
				result &= import_variable(*(parse_boolean::variable_name*)syntax.operands[i+1], variables, auto_define);
			else if (syntax.operands[i+1]->is_a<parse_boolean::constant>())
				result &= import_cover(*(parse_boolean::constant*)syntax.operands[i+1]);
		}
		else if (syntax.functions[i] == "|")
		{
			if (syntax.operands[i+1]->is_a<parse_boolean::guard>())
				result |= import_cover(*(parse_boolean::guard*)syntax.operands[i+1], variables, auto_define);
			else if (syntax.operands[i+1]->is_a<parse_boolean::variable_name>())
				result |= import_variable(*(parse_boolean::variable_name*)syntax.operands[i+1], variables, auto_define);
			else if (syntax.operands[i+1]->is_a<parse_boolean::constant>())
				result |= import_cover(*(parse_boolean::constant*)syntax.operands[i+1]);
		}
	}
	return result;
}
