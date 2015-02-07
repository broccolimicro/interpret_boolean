/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export.h"

parse_boolean::variable_name export_variable(int variable, boolean::variable_set &variables)
{
	parse_boolean::variable_name result;

	result.valid = true;
	for (int j = 0; j < (int)variables.variables[variable].name.size(); j++)
		result.names.push_back(variables.variables[variable].name[j].name);

	return result;
}

vector<parse_boolean::variable_name> export_variables(boolean::variable_set &variables)
{
	vector<parse_boolean::variable_name> result;

	for (int i = 0; i < (int)variables.variables.size(); i++)
		result.push_back(export_variable(i, variables));

	return result;
}

vector<parse_boolean::variable_name> export_variables(boolean::cube c, boolean::variable_set &variables)
{
	vector<parse_boolean::variable_name> result;
	vector<int> indices = c.vars();

	for (int i = 0; i < (int)indices.size(); i++)
		result.push_back(export_variable(indices[i], variables));

	return result;
}

vector<parse_boolean::variable_name> export_variables(boolean::cover c, boolean::variable_set &variables)
{
	vector<parse_boolean::variable_name> result;
	vector<int> indices = c.vars();

	for (int i = 0; i < (int)indices.size(); i++)
		result.push_back(export_variable(indices[i], variables));

	return result;
}

parse_boolean::constant export_constant(boolean::cover c)
{
	parse_boolean::constant result;
	result.valid = true;
	if (c == 0)
		result.valid = "0";
	else if (c == 1)
		result.valid = "1";
	else
	{
		error("", "exporting non-constant valued boolean::cover to parse_boolean::constant", __FILE__, __LINE__);
		result.valid = false;
	}
	return result;
}

parse_boolean::constant export_constant(boolean::cube c)
{
	parse_boolean::constant result;
	result.valid = true;
	if (c == 0)
		result.valid = "0";
	else if (c == 1)
		result.valid = "1";
	else
	{
		error("", "exporting non-constant valued boolean::cube to parse_boolean::constant", __FILE__, __LINE__);
		result.valid = false;
	}
	return result;
}

vector<parse_boolean::assignment> export_assignments(boolean::cube c, boolean::variable_set &variables)
{
	vector<parse_boolean::assignment> result;
	vector<int> indices = c.vars();

	for (int i = 0; i < (int)indices.size(); i++)
	{
		parse_boolean::assignment temp;
		temp.valid = true;

		temp.value = c.get(indices[i]);
		temp.variable = export_variable(indices[i], variables);
		result.push_back(temp);
	}

	return result;
}

vector<vector<parse_boolean::assignment> > export_assignments(boolean::cover c, boolean::variable_set &variables)
{
	vector<vector<parse_boolean::assignment> > result;
	for (int i = 0; i < (int)c.cubes.size(); i++)
		result.push_back(export_assignments(c.cubes[i], variables));
	return result;
}

parse_boolean::guard export_guard(int variable, bool value, boolean::variable_set &variables)
{
	parse_boolean::guard result;
	result.valid = true;
	result.level = 2;

	result.operands.push_back(new parse_boolean::variable_name(export_variable(variable, variables)));
	if (!value)
		result.functions.push_back("~");

	return result;
}

parse_boolean::guard export_guard(boolean::cube c, boolean::variable_set &variables)
{
	parse_boolean::guard result;
	result.valid = true;
	result.level = 1;

	if (c == 0 || c == 1)
		result.operands.push_back(new parse_boolean::constant(export_constant(c)));
	else
	{
		vector<int> indices = c.vars();

		for (int i = 0; i < (int)indices.size(); i++)
		{
			if (i != 0)
				result.functions.push_back("&");
			result.operands.push_back(new parse_boolean::guard(export_guard(indices[i], c.get(indices[i]), variables)));
		}
	}

	return result;
}

parse_boolean::guard export_guard(boolean::cover c, boolean::variable_set &variables)
{
	parse_boolean::guard result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)c.cubes.size(); i++)
	{
		if (i != 0)
			result.functions.push_back("|");
		result.operands.push_back(new parse_boolean::guard(export_guard(c.cubes[i], variables)));
	}

	return result;
}

