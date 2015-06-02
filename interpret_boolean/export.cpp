/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export.h"
#include "import.h"

parse_boolean::slice export_slice(int lb, int ub)
{
	parse_boolean::slice result;
	result.valid = true;
	result.lower = ::to_string(lb);
	result.upper = ::to_string(ub);
	return result;
}

parse_boolean::member_name export_member_name(boolean::instance instance)
{
	parse_boolean::member_name result;
	result.valid = true;
	result.name = instance.name;
	for (int i = 0; i < (int)instance.slice.size(); i++)
		result.slices.push_back(export_slice(instance.slice[i], instance.slice[i]));
	return result;
}

parse_boolean::variable_name export_variable_name(boolean::variable variable)
{
	parse_boolean::variable_name result;
	result.valid = true;
	for (int i = 0; i < (int)variable.name.size(); i++)
		result.names.push_back(export_member_name(variable.name[i]));
	return result;
}

parse_boolean::variable_name export_variable_name(int variable, const boolean::variable_set &variables)
{
	parse_boolean::variable_name result;
	if (variable >= 0 && variable < (int)variables.variables.size())
		result = export_variable_name(variables.variables[variable]);
	return result;
}

vector<parse_boolean::variable_name> export_variable_names(const boolean::variable_set &variables)
{
	vector<parse_boolean::variable_name> result;

	for (int i = 0; i < (int)variables.variables.size(); i++)
		result.push_back(export_variable_name(i, variables));

	return result;
}

parse_boolean::assignment export_assignment(boolean::cube c, const boolean::variable_set &variables)
{
	parse_boolean::assignment result;
	result.operation = parse_boolean::assignment::PARALLEL;
	result.valid = true;

	for (int i = 0; i < (int)variables.variables.size(); i++)
	{
		int value = c.get(i);
		if (value == 0 || value == 1)
			result.literals.push_back(pair<parse_boolean::variable_name, bool>(export_variable_name(i, variables), 1-c.get(i)));
		else if (value == -1)
		{
			result.literals.push_back(pair<parse_boolean::variable_name, bool>(export_variable_name(i, variables), 1));
			result.literals.push_back(pair<parse_boolean::variable_name, bool>(export_variable_name(i, variables), 0));
		}
	}

	return result;
}

parse_boolean::assignment export_assignment(boolean::cover c, const boolean::variable_set &variables)
{
	parse_boolean::assignment result;
	result.operation = parse_boolean::assignment::CHOICE;
	result.valid = true;

	for (int i = 0; i < (int)c.cubes.size(); i++)
		result.assignments.push_back(export_assignment(c.cubes[i], variables));

	return result;
}

parse_boolean::guard export_guard(boolean::cube c, const boolean::variable_set &variables)
{
	parse_boolean::guard result;
	result.operation = parse_boolean::guard::AND;
	result.valid = true;
	for (int i = 0; i < (int)variables.variables.size(); i++)
	{
		int value = c.get(i);
		if (value == 0 || value == 1)
			result.literals.push_back(pair<parse_boolean::variable_name, bool>(export_variable_name(i, variables), 1-c.get(i)));
		else if (value == -1)
		{
			result.literals.push_back(pair<parse_boolean::variable_name, bool>(export_variable_name(i, variables), 1));
			result.literals.push_back(pair<parse_boolean::variable_name, bool>(export_variable_name(i, variables), 0));
		}
	}

	if (result.literals.size() == 0)
		result.constants.push_back("1");

	return result;
}

parse_boolean::guard export_guard_xfactor(boolean::cover c, const boolean::variable_set &variables, int op)
{
	parse_boolean::guard result;
	result.operation = op;
	result.valid = true;

	boolean::cover nc = ~c;

	if (c.cubes.size() == 0)
		result.constants.push_back("0");
	else if (nc.cubes.size() == 0)
		result.constants.push_back("1");
	else if (c.cubes.size() == 1 || nc.cubes.size() == 1)
	{
		if (op == parse_boolean::guard::AND)
		{
			c = nc;
			result.operation = 1-op;
		}

		for (int i = 0; i < (int)c.cubes.size(); i++)
			result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard(c.cubes[i], variables), false));
	}
	else
	{
		boolean::cover c_left, c_right, nc_left, nc_right;
		float c_weight, nc_weight;

		c_weight = c.partition(c_left, c_right);
		nc_weight = nc.partition(nc_left, nc_right);

		if (c_weight <= nc_weight)
		{
			result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard_xfactor(c_left, variables, op), false));
			result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard_xfactor(c_right, variables, op), false));
		}
		else if (nc_weight < c_weight)
		{
			result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard_xfactor(nc_left, variables, 1-op), false));
			result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard_xfactor(nc_right, variables, 1-op), false));
			result.operation = 1-op;
		}
	}

	for (int i = 0; i < (int)result.guards.size(); )
	{
		if (!result.guards[i].second && (result.guards[i].first.operation == result.operation || (result.guards[i].first.guards.size() + result.guards[i].first.literals.size() + result.guards[i].first.constants.size() == 1)))
		{
			result.guards.insert(result.guards.end(), result.guards[i].first.guards.begin(), result.guards[i].first.guards.end());
			result.literals.insert(result.literals.end(), result.guards[i].first.literals.begin(), result.guards[i].first.literals.end());
			result.constants.insert(result.constants.end(), result.guards[i].first.constants.begin(), result.guards[i].first.constants.end());
			result.guards.erase(result.guards.begin() + i);
		}
		else
			i++;
	}

	return result;
}

parse_boolean::guard export_guard_hfactor(boolean::cover c, const boolean::variable_set &variables)
{
	parse_boolean::guard result;
	result.valid = true;
	result.operation = parse_boolean::guard::OR;

	if (c.is_null())
		result.constants.push_back("0");
	else if (c.is_tautology())
		result.constants.push_back("1");
	else if (c.cubes.size() == 1)
		result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard(c.cubes[0], variables), false));
	else
	{
		boolean::cube common = c.supercube();
		if (common == 1)
		{
			boolean::cover c_left, c_right;
			c.partition(c_left, c_right);

			result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard_hfactor(c_left, variables), false));
			result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard_hfactor(c_right, variables), false));
		}
		else
		{
			c.cofactor(common);

			result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard(common, variables), false));
			result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard_hfactor(c, variables), false));
			result.operation = parse_boolean::guard::AND;
		}
	}

	for (int i = 0; i < (int)result.guards.size(); )
	{
		if (!result.guards[i].second && (result.guards[i].first.operation == result.operation || (result.guards[i].first.guards.size() + result.guards[i].first.literals.size() + result.guards[i].first.constants.size() == 1)))
		{
			result.guards.insert(result.guards.end(), result.guards[i].first.guards.begin(), result.guards[i].first.guards.end());
			result.literals.insert(result.literals.end(), result.guards[i].first.literals.begin(), result.guards[i].first.literals.end());
			result.constants.insert(result.constants.end(), result.guards[i].first.constants.begin(), result.guards[i].first.constants.end());
			result.guards.erase(result.guards.begin() + i);
		}
		else
			i++;
	}

	return result;
}

parse_boolean::guard export_guard(boolean::cover c, const boolean::variable_set &variables)
{
	parse_boolean::guard result;
	result.operation = parse_boolean::guard::OR;
	result.valid = true;

	for (int i = 0; i < (int)c.cubes.size(); i++)
		result.guards.push_back(pair<parse_boolean::guard, bool>(export_guard(c.cubes[i], variables), false));

	if (c.cubes.size() == 0)
	{
		result.constants.push_back("0");
		return result;
	}

	return result;
}

