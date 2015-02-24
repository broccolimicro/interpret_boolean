/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"

// parse_boolean::variable_name
vector<boolean::instance> import_instances(tokenizer &tokens, const parse_boolean::member_name &syntax)
{
	vector<boolean::instance> result;
	result.push_back(boolean::instance());
	result.back().name = syntax.name;
	for (int i = 0; i < (int)syntax.slices.size(); i++)
	{
		int lb = atoi(syntax.slices[i].lower.c_str());
		int ub = atoi(syntax.slices[i].upper.c_str());

		int count = (int)result.size()-1;
		for (int j = count; j >= 0; j--)
			for (int k = lb; k < ub; k++)
			{
				result.push_back(result[j]);
				result.back().slice.push_back(k);
			}

		for (int j = count; j >= 0; j--)
			result[j].slice.push_back(ub);
	}
	return result;
}

vector<boolean::variable> import_variables(tokenizer &tokens, const parse_boolean::variable_name &syntax)
{
	vector<boolean::variable> result;
	result.push_back(boolean::variable());
	for (int i = 0; i < (int)syntax.names.size(); i++)
	{
		vector<boolean::instance> instances = import_instances(tokens, syntax.names[i]);

		int count = (int)result.size()-1;
		for (int j = count; j >= 0; j--)
			for (int k = 0; k < (int)instances.size()-1; k++)
			{
				result.push_back(result[j]);
				result.back().name.push_back(instances[k]);
			}

		for (int j = count; j >= 0; j--)
			result[j].name.push_back(instances.back());
	}

	return result;
}

vector<int> define_variables(tokenizer &tokens, const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define, bool squash_errors)
{
	vector<boolean::variable> v = import_variables(tokens, syntax);
	vector<int> result;
	for (int i = 0; i < (int)v.size(); i++)
	{
		if (define)
		{
			result.push_back(variables.define(v[i]));
			if (result.back() == -1 && !squash_errors)
			{
				tokens.load(&syntax);
				tokens.error("redefinition of variable '" + v[i].to_string() + "'", __FILE__, __LINE__);
				if (tokens.load(syntax.to_string()))
					tokens.note("previously defined here", __FILE__, __LINE__);
			}
			else if (result.back() == -1 && squash_errors)
				result.back() = variables.find(v[i]);
			else
				tokens.save(syntax.to_string(), &syntax);
		}
		else
		{
			result.push_back(variables.find(v[i]));

			if (result.back() == -1 && !squash_errors)
			{
				tokens.load(&syntax);
				tokens.error("variable '" + v[i].to_string() + "' not yet defined", __FILE__, __LINE__);
			}
			else if (result.back() == -1 && squash_errors)
			{
				result.back() = variables.define(v[i]);
				tokens.save(syntax.to_string(), &syntax);
			}
		}
	}
	return result;
}


boolean::cover import_cover(tokenizer &tokens, const parse_boolean::assignment &syntax, boolean::variable_set &variables, bool auto_define)
{
	if (syntax.expression != NULL && syntax.expression->valid)
		return import_cover(tokens, *syntax.expression, variables, auto_define);
	else if (syntax.variable.valid)
	{
		vector<int> v = define_variables(tokens, syntax.variable, variables, auto_define, auto_define);
		boolean::cover result(1);
		for (int i = 0; i < (int)v.size(); i++)
			if (v[i] >= 0)
				result &= boolean::cover(v[i], syntax.value);
		return result;
	}
	else
		return boolean::cover(0);
}

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::internal_parallel &syntax, boolean::variable_set &variables, bool auto_define)
{
	boolean::cover result(1);
	for (int i = 0; i < (int)syntax.branches.size(); i++)
		if (syntax.branches[i].valid)
			result &= import_cover(tokens, syntax.branches[i], variables, auto_define);
	return result;
}

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::internal_choice &syntax, boolean::variable_set &variables, bool auto_define)
{
	boolean::cover result(0);
	for (int i = 0; i < (int)syntax.branches.size(); i++)
		if (syntax.branches[i].valid)
			result |= import_cover(tokens, syntax.branches[i], variables, auto_define);
	return result;
}

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::complement &syntax, boolean::variable_set &variables, bool auto_define)
{
	boolean::cover result;
	if (syntax.expression != NULL && syntax.expression->valid)
		result = import_cover(tokens, *syntax.expression, variables, auto_define);
	else if (syntax.variable.valid)
	{
		vector<int> v = define_variables(tokens, syntax.variable, variables, auto_define, auto_define);
		boolean::cover result(1);
		for (int i = 0; i < (int)v.size(); i++)
			if (v[i] > 0)
				result &= boolean::cover(v[i], 1);
		return result;
	}
	else if (syntax.value == "1")
		result = boolean::cover(1);
	else
		result = boolean::cover(0);

	if (syntax.invert)
		result = ~result;

	return result;
}

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::conjunction &syntax, boolean::variable_set &variables, bool auto_define)
{
	boolean::cover result(1);
	for (int i = 0; i < (int)syntax.branches.size(); i++)
		if (syntax.branches[i].valid)
			result &= import_cover(tokens, syntax.branches[i], variables, auto_define);
	return result;
}

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::disjunction &syntax, boolean::variable_set &variables, bool auto_define)
{
	boolean::cover result(0);
	for (int i = 0; i < (int)syntax.branches.size(); i++)
		if (syntax.branches[i].valid)
			result |= import_cover(tokens, syntax.branches[i], variables, auto_define);
	return result;
}
