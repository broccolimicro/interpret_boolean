/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"

// parse_boolean::variable_name
vector<boolean::instance> import_instances(const parse_boolean::member_name &syntax, tokenizer *tokens)
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

vector<boolean::variable> import_variables(const parse_boolean::variable_name &syntax, tokenizer *tokens)
{
	vector<boolean::variable> result;
	result.push_back(boolean::variable());
	for (int i = 0; i < (int)syntax.names.size(); i++)
	{
		vector<boolean::instance> instances = import_instances(syntax.names[i], tokens);

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

vector<int> define_variables(const parse_boolean::variable_name &syntax, boolean::variable_set &variables, tokenizer *tokens, bool define, bool squash_errors)
{
	vector<boolean::variable> v = import_variables(syntax, tokens);
	vector<int> result;
	for (int i = 0; i < (int)v.size(); i++)
	{
		if (define)
		{
			result.push_back(variables.define(v[i]));
			if (result.back() == -1 && !squash_errors)
			{
				if (tokens != NULL)
				{
					tokens->load(&syntax);
					tokens->error("redefinition of variable '" + v[i].to_string() + "'", __FILE__, __LINE__);
					if (tokens->load(syntax.to_string()))
						tokens->note("previously defined here", __FILE__, __LINE__);
				}
				else
					error("", "redefinition of variable '" + v[i].to_string() + "'", __FILE__, __LINE__);
			}
			else if (result.back() == -1 && squash_errors)
				result.back() = variables.find(v[i]);
			else if (tokens != NULL)
				tokens->save(syntax.to_string(), &syntax);
		}
		else
		{
			result.push_back(variables.find(v[i]));

			if (result.back() == -1 && !squash_errors)
			{
				if (tokens != NULL)
				{
					tokens->load(&syntax);
					tokens->error("variable '" + v[i].to_string() + "' not yet defined", __FILE__, __LINE__);
				}
				else
					error("", "variable '" + v[i].to_string() + "' not yet defined", __FILE__, __LINE__);
			}
			else if (result.back() == -1 && squash_errors)
			{
				result.back() = variables.define(v[i]);
				if (tokens != NULL)
					tokens->save(syntax.to_string(), &syntax);
			}
		}
	}
	return result;
}

boolean::cover import_cover(const parse_boolean::assignment &syntax, boolean::variable_set &variables, tokenizer *tokens, bool auto_define)
{
	if (syntax.operation != parse_boolean::assignment::CHOICE && syntax.operation != parse_boolean::assignment::PARALLEL)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		return boolean::cover(0);
	}

	boolean::cover result(1-syntax.operation);

	for (int i = 0; i < (int)syntax.literals.size(); i++)
	{
		if (syntax.literals[i].first.valid)
		{
			vector<int> v = define_variables(syntax.literals[i].first, variables, tokens, auto_define, auto_define);
			for (int j = 0; j < (int)v.size(); j++)
				if (v[j] >= 0)
				{
					if (syntax.operation == parse_boolean::assignment::CHOICE)
						result |= boolean::cube(v[j], 1-syntax.literals[i].second);
					else if (syntax.operation == parse_boolean::assignment::PARALLEL)
						result &= boolean::cube(v[j], 1-syntax.literals[i].second);
				}
		}
	}

	for (int i = 0; i < (int)syntax.assignments.size(); i++)
	{
		if (syntax.assignments[i].valid)
		{
			if (syntax.operation == parse_boolean::assignment::CHOICE)
				result |= import_cover(syntax.assignments[i], variables, tokens, auto_define);
			else if (syntax.operation == parse_boolean::assignment::PARALLEL)
				result &= import_cover(syntax.assignments[i], variables, tokens, auto_define);
		}
	}

	return result;
}

boolean::cube import_cube(const parse_boolean::assignment &syntax, boolean::variable_set &variables, tokenizer *tokens, bool auto_define)
{
	if (syntax.operation != parse_boolean::assignment::CHOICE && syntax.operation != parse_boolean::assignment::PARALLEL)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		return boolean::cube(0);
	}

	boolean::cover result(1-syntax.operation);

	for (int i = 0; i < (int)syntax.literals.size(); i++)
	{
		if (syntax.literals[i].first.valid)
		{
			vector<int> v = define_variables(syntax.literals[i].first, variables, tokens, auto_define, auto_define);
			for (int j = 0; j < (int)v.size(); j++)
				if (v[j] >= 0)
				{
					if (syntax.operation == parse_boolean::assignment::CHOICE)
						result |= boolean::cube(v[j], 1-syntax.literals[i].second);
					else if (syntax.operation == parse_boolean::assignment::PARALLEL)
						result &= boolean::cube(v[j], 1-syntax.literals[i].second);
				}
		}
	}

	for (int i = 0; i < (int)syntax.assignments.size(); i++)
	{
		if (syntax.assignments[i].valid)
		{
			if (syntax.operation == parse_boolean::assignment::CHOICE)
				result |= import_cube(syntax.assignments[i], variables, tokens, auto_define);
			else if (syntax.operation == parse_boolean::assignment::PARALLEL)
				result &= import_cube(syntax.assignments[i], variables, tokens, auto_define);
		}
	}

	if (result.cubes.size() > 1)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("illegal disjunction", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "illegal disjunction", __FILE__, __LINE__);
		return result[0];
	}
	else if (result.cubes.size() == 1)
		return result[0];
	else
		return boolean::cube(0);
}

boolean::cover import_cover(const parse_boolean::guard &syntax, boolean::variable_set &variables, tokenizer *tokens, bool auto_define)
{
	if (syntax.operation != parse_boolean::guard::OR && syntax.operation != parse_boolean::guard::AND)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		return boolean::cover(0);
	}

	boolean::cover result(1-syntax.operation);

	for (int i = 0; i < (int)syntax.constants.size(); i++)
	{
		if (syntax.constants[i] == "0" && syntax.operation == parse_boolean::guard::AND)
			return boolean::cover(0);
		else if (syntax.constants[i] == "1" && syntax.operation == parse_boolean::guard::OR)
			return boolean::cover(1);
	}

	for (int i = 0; i < (int)syntax.literals.size(); i++)
	{
		if (syntax.literals[i].first.valid)
		{
			vector<int> v = define_variables(syntax.literals[i].first, variables, tokens, auto_define, auto_define);
			for (int j = 0; j < (int)v.size(); j++)
				if (v[j] >= 0)
				{
					if (syntax.operation == parse_boolean::guard::OR)
						result |= boolean::cube(v[j], 1-syntax.literals[i].second);
					else if (syntax.operation == parse_boolean::guard::AND)
						result &= boolean::cube(v[j], 1-syntax.literals[i].second);
				}
		}
	}

	for (int i = 0; i < (int)syntax.guards.size(); i++)
	{
		if (syntax.guards[i].first.valid)
		{
			boolean::cover sub(import_cover(syntax.guards[i].first, variables, tokens, auto_define));
			if (syntax.guards[i].second)
				sub = ~sub;

			if (syntax.operation == parse_boolean::guard::OR)
				result |= sub;
			else if (syntax.operation == parse_boolean::guard::AND)
				result &= sub;
		}
	}

	return result;
}

boolean::cube import_cube(const parse_boolean::guard &syntax, boolean::variable_set &variables, tokenizer *tokens, bool auto_define)
{
	if (syntax.operation != parse_boolean::guard::OR && syntax.operation != parse_boolean::guard::AND)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		return boolean::cube(0);
	}

	boolean::cover result(1-syntax.operation);
	for (int i = 0; i < (int)syntax.constants.size(); i++)
	{
		if (syntax.constants[i] == "0" && syntax.operation == parse_boolean::guard::AND)
			return boolean::cube(0);
		else if (syntax.constants[i] == "1" && syntax.operation == parse_boolean::guard::OR)
			return boolean::cube(1);
	}

	for (int i = 0; i < (int)syntax.literals.size(); i++)
	{
		if (syntax.literals[i].first.valid)
		{
			vector<int> v = define_variables(syntax.literals[i].first, variables, tokens, auto_define, auto_define);
			for (int j = 0; j < (int)v.size(); j++)
				if (v[j] >= 0)
				{
					if (syntax.operation == parse_boolean::guard::OR)
						result |= boolean::cube(v[j], 1-syntax.literals[i].second);
					else if (syntax.operation == parse_boolean::guard::AND)
						result &= boolean::cube(v[j], 1-syntax.literals[i].second);
				}
		}
	}

	for (int i = 0; i < (int)syntax.guards.size(); i++)
	{
		if (syntax.guards[i].first.valid)
		{
			boolean::cover sub(import_cube(syntax.guards[i].first, variables, tokens, auto_define));
			if (syntax.guards[i].second)
				sub = ~sub;

			if (syntax.operation == parse_boolean::guard::OR)
				result |= sub;
			else if (syntax.operation == parse_boolean::guard::AND)
				result &= sub;
		}
	}

	if (result.cubes.size() > 1)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("illegal disjunction", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "illegal disjunction", __FILE__, __LINE__);
		return result[0];
	}
	else if (result.cubes.size() == 1)
		return result[0];
	else
		return boolean::cube(0);
}

