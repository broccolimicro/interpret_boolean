/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"

boolean::cube import_cube(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.operation == "+")
	{
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		return boolean::cube(v[0], 1);
	}
	else if (syntax.operation == "-")
	{
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		return boolean::cube(v[0], 0);
	}
	else if (syntax.operation == "~")
	{
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		return boolean::cube(v[0], -1);
	}
	else if (syntax.operation == ":=")
	{
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		boolean::cover temp = import_cover(syntax.expressions[0], variables, default_id, tokens, auto_define);
		if (temp.is_tautology())
			return boolean::cube(v[0], 1);
		else if (temp.is_null())
			return boolean::cube(v[0], 0);
		else
		{
			if (tokens != NULL)
			{
				tokens->load(&syntax.expressions[0]);
				tokens->error("unsupported operation", __FILE__, __LINE__);
			}
			else
				error(syntax.expressions[0].to_string(), "unsupported operation", __FILE__, __LINE__);
			return boolean::cube();
		}
	}
	else
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("unsupported operation", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "unsupported operation", __FILE__, __LINE__);
		return boolean::cube();
	}
}

boolean::cover import_cover(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	return boolean::cover(import_cube(syntax, variables, default_id, tokens, auto_define));
}

boolean::cube import_cube(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	if (syntax.level >= (int)syntax.precedence.size())
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		return boolean::cube(1);
	}
	else if (syntax.literals.size() == 0 && syntax.compositions.size() == 0)
		return boolean::cube(1);
	else if (syntax.precedence[syntax.level] == ":" && syntax.literals.size() + syntax.compositions.size() > 1)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("illegal disjunction", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "illegal disjunction", __FILE__, __LINE__);
		return boolean::cube();
	}

	boolean::cube result;

	for (int i = 0; i < (int)syntax.literals.size(); i++)
		if (syntax.literals[i].valid)
			result &= import_cube(syntax.literals[i], variables, default_id, tokens, auto_define);

	for (int i = 0; i < (int)syntax.compositions.size(); i++)
		if (syntax.compositions[i].valid)
			result &= import_cube(syntax.compositions[i], variables, default_id, tokens, auto_define);

	return result;
}

boolean::cover import_cover(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	if (syntax.level >= (int)syntax.precedence.size())
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
	else if (syntax.literals.size() == 0 && syntax.compositions.size() == 0)
		return boolean::cover(1);

	boolean::cover result(syntax.level);

	for (int i = 0; i < (int)syntax.literals.size(); i++)
	{
		if (syntax.literals[i].valid)
		{
			if (syntax.precedence[syntax.level] == ":")
				result |= import_cube(syntax.literals[i], variables, default_id, tokens, auto_define);
			else if (syntax.precedence[syntax.level] == ",")
				result &= import_cube(syntax.literals[i], variables, default_id, tokens, auto_define);
		}
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++)
	{
		if (syntax.compositions[i].valid)
		{
			if (syntax.precedence[syntax.level] == ":")
				result |= import_cover(syntax.compositions[i], variables, default_id, tokens, auto_define);
			else if (syntax.precedence[syntax.level] == ",")
				result &= import_cover(syntax.compositions[i], variables, default_id, tokens, auto_define);
		}
	}

	return result;
}

boolean::cube import_cube(const parse_expression::expression &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	if (syntax.level >= (int)syntax.precedence.size())
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

	boolean::cover result;

	for (int i = 0; i < (int)syntax.arguments.size(); i++)
	{
		// interpret the operands
		boolean::cover sub;
		if (syntax.arguments[i].sub.valid)
			sub = import_cube(syntax.arguments[i].sub, variables, default_id, tokens, auto_define);
		else if (syntax.arguments[i].literal.valid)
		{
			boolean::cube literal;
			vector<int> v = define_variables(syntax.arguments[i].literal, variables, default_id, tokens, auto_define, auto_define);
			for (int j = 0; j < (int)v.size(); j++)
				if (v[j] >= 0)
					literal &= boolean::cube(v[j], 1);
			sub = boolean::cover(literal);
		}
		else if (syntax.arguments[i].constant == "0")
			sub = boolean::cover(0);
		else if (syntax.arguments[i].constant == "1")
			sub = boolean::cover(1);

		// interpret the operators
		bool err = false;
		if (i == 0)
		{
			if (syntax.precedence[syntax.level].type == parse_expression::operation_set::left_unary)
				for (int j = (int)syntax.operations.size()-1; j >= 0; j--)
				{
					if (syntax.operations[j] == "~")
					{
						sub = ~sub;
						if (sub.cubes.size() > 1)
							err = true;
					}
					else
						err = true;
				}
			else if (i == 0 && syntax.precedence[syntax.level].type == parse_expression::operation_set::right_unary)
				for (int j = 0; j < (int)syntax.operations.size(); j++)
					err = true;

			result = sub;
		}
		else if (syntax.operations[i-1] == "&")
			result &= sub;
		else
			err = true;

		if (err)
		{
			if (tokens != NULL)
			{
				tokens->load(&syntax);
				tokens->error("unsupported operation", __FILE__, __LINE__);
			}
			else
				error(syntax.to_string(), "unsupported operation", __FILE__, __LINE__);
		}
	}

	if (result.cubes.size() == 0)
		return boolean::cube(0);
	else
		return result.cubes[0];
}

boolean::cover import_cover(const parse_expression::expression &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	if (syntax.level >= (int)syntax.precedence.size())
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

	boolean::cover result;

	for (int i = 0; i < (int)syntax.arguments.size(); i++)
	{
		// interpret the operands
		boolean::cover sub;
		if (syntax.arguments[i].sub.valid)
			sub = import_cover(syntax.arguments[i].sub, variables, default_id, tokens, auto_define);
		else if (syntax.arguments[i].literal.valid)
		{
			boolean::cube literal;
			vector<int> v = define_variables(syntax.arguments[i].literal, variables, default_id, tokens, auto_define, auto_define);
			for (int j = 0; j < (int)v.size(); j++)
				if (v[j] >= 0)
					literal &= boolean::cube(v[j], 1);
			sub = boolean::cover(literal);
		}
		else if (syntax.arguments[i].constant == "0")
			sub = boolean::cover(0);
		else if (syntax.arguments[i].constant == "1")
			sub = boolean::cover(1);

		// interpret the operators
		bool err = false;
		if (i == 0)
		{
			if (syntax.precedence[syntax.level].type == parse_expression::operation_set::left_unary)
				for (int j = (int)syntax.operations.size()-1; j >= 0; j--)
				{
					if (syntax.operations[j] == "~")
						sub = ~sub;
					else
						err = true;
				}
			else if (syntax.precedence[syntax.level].type == parse_expression::operation_set::right_unary)
				for (int j = 0; j < (int)syntax.operations.size(); j++)
					err = true;

			result = sub;
		}
		else if (syntax.operations[i-1] == "|")
			result |= sub;
		else if (syntax.operations[i-1] == "&")
			result &= sub;
		else
			err = true;

		if (err)
		{
			if (tokens != NULL)
			{
				tokens->load(&syntax);
				tokens->error("unsupported operation", __FILE__, __LINE__);
			}
			else
				error(syntax.to_string(), "unsupported operation", __FILE__, __LINE__);
		}
	}

	return result;
}

boolean::unsigned_int import_unsigned_int(const parse_expression::expression &syntax, map<string, boolean::unsigned_int> &variables, int default_id, tokenizer *tokens)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	if (syntax.level >= (int)syntax.precedence.size())
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		return boolean::unsigned_int(0);
	}

	boolean::unsigned_int result;

	for (int i = 0; i < (int)syntax.arguments.size(); i++)
	{
		// interpret the operands
		boolean::unsigned_int sub;
		if (syntax.arguments[i].sub.valid) {
			sub = import_unsigned_int(syntax.arguments[i].sub, variables, default_id, tokens);
		} else if (syntax.arguments[i].literal.valid) {
			map<string, boolean::unsigned_int>::iterator id = variables.find(syntax.arguments[i].literal.to_string());
			if (id != variables.end()) {
				sub = id->second;
			} else {
				if (tokens != NULL)
				{
					tokens->load(&syntax);
					tokens->error("reference to undefined variable", __FILE__, __LINE__);
				}
				else
					error(syntax.to_string(), "reference to undefined variable", __FILE__, __LINE__);
			}
		} else if (syntax.arguments[i].constant.size() > 0) {
			sub = boolean::unsigned_int(atoi(syntax.arguments[i].constant.c_str()));
		}

		// interpret the operators
		bool err = false;
		if (i == 0) {
			if (syntax.precedence[syntax.level].type == parse_expression::operation_set::left_unary) {
				for (int j = (int)syntax.operations.size()-1; j >= 0; j--)
				{
					if (syntax.operations[j] == "~") {
						sub = ~sub;
					} else if (syntax.operations[j] == "+") {
					} else if (syntax.operations[j] == "-") {
						sub = -sub;
					} else {
						err = true;
					}
				}
			} else if (syntax.precedence[syntax.level].type == parse_expression::operation_set::right_unary) {
				for (int j = 0; j < (int)syntax.operations.size(); j++) {
					err = true;
				}
			}

			result = sub;
		} else if (syntax.operations[i-1] == "|") {
			result |= sub;
		} else if (syntax.operations[i-1] == "&") {
			result &= sub;
		} else if (syntax.operations[i-1] == "^") {
			result ^= sub;
		} else if (syntax.operations[i-1] == "==") {
			result = (result == sub);
		} else if (syntax.operations[i-1] == "~=") {
			result = (result != sub);
		} else if (syntax.operations[i-1] == "<") {
			result = (result < sub);
		} else if (syntax.operations[i-1] == ">") {
			result = (result > sub);
		} else if (syntax.operations[i-1] == "<=") {
			result = (result <= sub);
		} else if (syntax.operations[i-1] == ">=") {
			result = (result >= sub);
		//} else if (syntax.operations[i-1] == "<<") {
		//	result <<= sub;
		//} else if (syntax.operations[i-1] == ">>") {
		//	result >>= sub;
		} else if (syntax.operations[i-1] == "+") {
			result += sub;
		} else if (syntax.operations[i-1] == "-") {
			result -= sub;
		} else if (syntax.operations[i-1] == "*") {
			result *= sub;
		} else if (syntax.operations[i-1] == "/") {
			result /= sub;
		//} else if (syntax.operations[i-1] == "%") {
		//	result %= sub;
		} else {
			err = true;
		}

		if (err)
		{
			if (tokens != NULL)
			{
				tokens->load(&syntax);
				tokens->error("unsupported operation", __FILE__, __LINE__);
			}
			else
				error(syntax.to_string(), "unsupported operation", __FILE__, __LINE__);
		}
	}

	return result;
}
