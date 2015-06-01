/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export.h"

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

parse_boolean::assignment export_assignment(int variable, bool value, const boolean::variable_set &variables)
{
	parse_boolean::assignment result;
	result.valid = true;
	result.variable = export_variable_name(variable, variables);
	result.value = value;
	return result;
}

parse_boolean::internal_parallel export_internal_parallel(boolean::cube c, const boolean::variable_set &variables)
{
	parse_boolean::internal_parallel result;
	result.valid = true;
	for (int i = 0; i < (int)variables.variables.size(); i++)
	{
		int value = c.get(i);
		if (value == 0 || value == 1)
			result.branches.push_back(export_assignment(i, c.get(i), variables));
		else if (value == -1)
		{
			result.branches.push_back(export_assignment(i, 0, variables));
			result.branches.push_back(export_assignment(i, 1, variables));
		}
	}
	return result;
}

parse_boolean::internal_choice export_internal_choice(boolean::cover c, const boolean::variable_set &variables)
{
	parse_boolean::internal_choice result;
	result.valid = true;
	for (int i = 0; i < (int)c.cubes.size(); i++)
		result.branches.push_back(export_internal_parallel(c.cubes[i], variables));
	return result;
}

parse_boolean::complement export_complement(bool value, const boolean::variable_set &variables)
{
	parse_boolean::complement result;
	result.valid = true;
	result.value = ::to_string(value);
	return result;
}

parse_boolean::complement export_complement(int variable, bool value, const boolean::variable_set &variables)
{
	parse_boolean::complement result;
	result.valid = true;
	result.variable = export_variable_name(variable, variables);
	result.invert = !value;
	return result;
}

parse_boolean::conjunction export_conjunction(boolean::cube c, const boolean::variable_set &variables)
{
	parse_boolean::conjunction result;
	result.valid = true;
	for (int i = 0; i < (int)variables.variables.size(); i++)
	{
		int value = c.get(i);
		if (value == 0 || value == 1)
			result.branches.push_back(export_complement(i, c.get(i), variables));
		else if (value == -1)
		{
			result.branches.push_back(export_complement(i, 0, variables));
			result.branches.push_back(export_complement(i, 1, variables));
		}
	}
	return result;
}

parse_boolean::disjunction export_disjunction(boolean::cover c, const boolean::variable_set &variables)
{
	parse_boolean::disjunction result;
	result.valid = true;
	for (int i = 0; i < (int)c.cubes.size(); i++)
		result.branches.push_back(export_conjunction(c.cubes[i], variables));
	return result;
}

parse_boolean::complement export_complement(boolean::factor f, const boolean::variable_set &variables)
{
	parse_boolean::complement result;
	result.valid = true;
	if (f.subfactors.size() == 0)
	{
		/*if (f.op == boolean::factor::AND)
		{
			f.terms = ~f.terms;
			f.op = 1-f.op;
		}*/

		result.expression = new parse_boolean::disjunction();
		*result.expression = export_disjunction(f.terms, variables);
	}
	else
	{
		result.expression = new parse_boolean::disjunction();
		*result.expression = export_disjunction(f, variables);
	}
	return result;
}

parse_boolean::conjunction export_conjunction(boolean::factor f, const boolean::variable_set &variables)
{
	parse_boolean::conjunction result;
	result.valid = true;
	if (f.op == boolean::factor::OR || f.subfactors.size() == 0)
		result.branches.push_back(export_complement(f, variables));
	else if (f.subfactors.size() > 0)
	{
		parse_boolean::complement tmp;
		tmp.valid = true;
		tmp.expression = new parse_boolean::disjunction();
		*tmp.expression = export_disjunction(f.terms, variables);
		result.branches.push_back(tmp);

		for (int i = 0; i < (int)f.subfactors.size(); i++)
		{
			if (f.subfactors[i].op == boolean::factor::OR)
				result.branches.push_back(export_complement(f.subfactors[i], variables));
			else
			{
				parse_boolean::conjunction temp = export_conjunction(f.subfactors[i], variables);
				result.branches.insert(result.branches.end(), temp.branches.begin(), temp.branches.end());
			}
		}
	}

	return result;
}

parse_boolean::disjunction export_disjunction(boolean::factor f, const boolean::variable_set &variables)
{
	parse_boolean::disjunction result;
	result.valid = true;
	if (f.op == boolean::factor::AND)
		result.branches.push_back(export_conjunction(f, variables));
	else if (f.subfactors.size() > 0)
	{
		for (int i = 0; i < (int)f.terms.cubes.size(); i++)
			result.branches.push_back(export_conjunction(f.terms.cubes[i], variables));

		for (int i = 0; i < (int)f.subfactors.size(); i++)
		{
			if (f.subfactors[i].op == boolean::factor::AND)
				result.branches.push_back(export_conjunction(f.subfactors[i], variables));
			else
			{
				parse_boolean::disjunction temp = export_disjunction(f.subfactors[i], variables);
				result.branches.insert(result.branches.end(), temp.branches.begin(), temp.branches.end());
			}
		}
	}
	else
	{
		parse_boolean::disjunction temp = export_disjunction(f.terms, variables);
		result.branches.insert(result.branches.end(), temp.branches.begin(), temp.branches.end());
	}

	return result;
}
