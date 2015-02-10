/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"

// parse_boolean::variable_name

int import_variable(tokenizer &tokens, const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define, bool squash_errors)
{
	if (syntax.valid)
	{
		boolean::variable v;
		for (int i = 0; i < (int)syntax.names.size(); i++)
			v.name.push_back(boolean::instance(syntax.names[i], vector<int>()));

		int result;
		if (define)
		{
			result = variables.define(v);
			if (result == -1 && !squash_errors)
			{
				tokens.load(&syntax);
				tokens.error("redefinition of variable '" + v.to_string() + "'", __FILE__, __LINE__);
				if (tokens.load(syntax.to_string()))
					tokens.note("previously defined here", __FILE__, __LINE__);
			}
			else if (result == -1 && squash_errors)
				result = variables.find(v);
			else
				tokens.save(syntax.to_string(), &syntax);
		}
		else
		{
			result = variables.find(v);

			if (result == -1 && !squash_errors)
			{
				tokens.load(&syntax);
				tokens.error("variable '" + v.to_string() + "' not yet defined", __FILE__, __LINE__);
			}
			else if (result == -1 && squash_errors)
			{
				result = variables.define(v);
				tokens.save(syntax.to_string(), &syntax);
			}
		}

		return result;
	}
	else
		return -1;
}

boolean::cube import_cube(tokenizer &tokens, const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define, bool squash_errors)
{
	int v = import_variable(tokens, syntax, variables, define, squash_errors);
	if (v != -1)
		return boolean::cube(v, 1);
	else
		return boolean::cube();
}

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::variable_name &syntax, boolean::variable_set &variables, bool define, bool squash_errors)
{
	int v = import_variable(tokens, syntax, variables, define, squash_errors);
	if (v != -1)
		return boolean::cover(v, 1);
	else
		return boolean::cover();
}

// parse_boolean::constant

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::constant &syntax)
{
	if (syntax.valid)
	{
		if (syntax.value == "0")
			return boolean::cover(0);
		else if (syntax.value == "1")
			return boolean::cover(1);
		else
		{
			tokens.load(&syntax);
			tokens.error("invalid constant value '" + syntax.value + "'", __FILE__, __LINE__);
			return boolean::cube();
		}
	}
	else
		return boolean::cover();
}

boolean::cube import_cube(tokenizer &tokens, const parse_boolean::constant &syntax)
{
	if (syntax.valid)
	{
		if (syntax.value == "0")
			return boolean::cube(0);
		else if (syntax.value == "1")
			return boolean::cube(1);
		else
		{
			tokens.load(&syntax);
			tokens.error("invalid constant value '" + syntax.value + "'", __FILE__, __LINE__);
			return boolean::cube();
		}
	}
	else
		return boolean::cube();
}

// parse_boolean::assignment

boolean::cube import_cube(tokenizer &tokens, const parse_boolean::assignment &syntax, boolean::variable_set &variables, bool auto_define)
{
	int v = -1;
	if (syntax.valid)
		v = import_variable(tokens, syntax.variable, variables, auto_define, auto_define);

	if (v != -1)
		return boolean::cube(v, syntax.value);
	else
		return boolean::cube();
}

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::assignment &syntax, boolean::variable_set &variables, bool auto_define)
{
	int v = -1;
	if (syntax.valid)
		v = import_variable(tokens, syntax.variable, variables, auto_define, auto_define);

	if (v != -1)
		return boolean::cover(v, syntax.value);
	else
		return boolean::cover();
}

// parse_boolean::guard

boolean::cover import_cover(tokenizer &tokens, const parse_boolean::guard &syntax, boolean::variable_set &variables, bool auto_define)
{
	if (syntax.valid)
	{
		boolean::cover result;
		if (syntax.operands.size() > 0)
		{
			if (syntax.operands[0]->is_a<parse_boolean::guard>())
				result = import_cover(tokens, *(parse_boolean::guard*)syntax.operands[0], variables, auto_define);
			else if (syntax.operands[0]->is_a<parse_boolean::variable_name>())
				result = import_cover(tokens, *(parse_boolean::variable_name*)syntax.operands[0], variables, auto_define, auto_define);
			else if (syntax.operands[0]->is_a<parse_boolean::constant>())
				result = import_cover(tokens, *(parse_boolean::constant*)syntax.operands[0]);
		}

		for (int i = 0; i < (int)syntax.functions.size(); i++)
		{
			if (syntax.functions[i] == "~")
				result = ~result;
			else if (syntax.functions[i] == "&")
			{
				if (syntax.operands[i+1]->is_a<parse_boolean::guard>())
					result &= import_cover(tokens, *(parse_boolean::guard*)syntax.operands[i+1], variables, auto_define);
				else if (syntax.operands[i+1]->is_a<parse_boolean::variable_name>())
					result &= import_variable(tokens, *(parse_boolean::variable_name*)syntax.operands[i+1], variables, auto_define, auto_define);
				else if (syntax.operands[i+1]->is_a<parse_boolean::constant>())
					result &= import_cover(tokens, *(parse_boolean::constant*)syntax.operands[i+1]);
			}
			else if (syntax.functions[i] == "|")
			{
				if (syntax.operands[i+1]->is_a<parse_boolean::guard>())
					result |= import_cover(tokens, *(parse_boolean::guard*)syntax.operands[i+1], variables, auto_define);
				else if (syntax.operands[i+1]->is_a<parse_boolean::variable_name>())
					result |= import_variable(tokens, *(parse_boolean::variable_name*)syntax.operands[i+1], variables, auto_define, auto_define);
				else if (syntax.operands[i+1]->is_a<parse_boolean::constant>())
					result |= import_cover(tokens, *(parse_boolean::constant*)syntax.operands[i+1]);
			}
		}
		return result;
	}
	else
		return boolean::cover();
}
