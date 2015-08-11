/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export.h"
#include "import.h"

parse_expression::assignment export_assignment(int uid, int value, const ucs::variable_set &variables)
{
	parse_expression::assignment result;
	result.valid = true;

	result.names.push_back(export_variable_name(uid, variables));
	if (value == 0)
		result.operation = "-";
	else if (value == 1)
		result.operation = "+";
	else if (value == -1)
		result.operation = "~";

	return result;
}

parse_expression::composition export_composition(boolean::cube c, const ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;

	result.level = 1;//parse_expression::composition::get_level(",");

	for (int i = 0; i < (int)variables.nodes.size(); i++)
		if (c.get(i) != 2)
			result.literals.push_back(export_assignment(i, c.get(i), variables));

	return result;
}

parse_expression::composition export_composition(boolean::cover c, const ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;

	result.level = 0;//parse_expression::composition::get_level(":");

	for (int i = 0; i < (int)c.cubes.size(); i++)
		result.compositions.push_back(export_composition(c.cubes[i], variables));

	return result;
}

parse_expression::expression export_expression(int uid, int value, const ucs::variable_set &variables)
{
	parse_expression::expression result;
	result.valid = true;

	result.level = 8;//parse_expression::expression::get_level("~");

	result.arguments.push_back(parse_expression::argument(export_variable_name(uid, variables)));
	if (value == 0)
		result.operations.push_back("~");
	else if (value == -1)
		result.operations.push_back("?");

	return result;
}

parse_expression::expression export_expression(boolean::cube c, const ucs::variable_set &variables)
{
	parse_expression::expression result;
	result.valid = true;

	result.level = 1;//parse_expression::expression::get_level("&");

	for (int i = 0; i < (int)variables.nodes.size(); i++)
		if (c.get(i) != 2)
			result.arguments.push_back(parse_expression::argument(export_expression(i, c.get(i), variables)));

	if (result.arguments.size() == 0)
		result.arguments.push_back(parse_expression::argument("1"));

	for (int i = 1; i < (int)result.arguments.size(); i++)
		result.operations.push_back("&");

	return result;
}

parse_expression::expression export_expression(boolean::cover c, const ucs::variable_set &variables)
{
	parse_expression::expression result;
	result.valid = true;

	result.level = 0;//parse_expression::expression::get_level("|");

	for (int i = 0; i < (int)c.cubes.size(); i++)
		result.arguments.push_back(parse_expression::argument(export_expression(c.cubes[i], variables)));

	if (c.cubes.size() == 0)
		result.arguments.push_back(parse_expression::argument("0"));

	for (int i = 1; i < (int)result.arguments.size(); i++)
		result.operations.push_back("|");

	return result;
}

parse_expression::expression export_expression_xfactor(boolean::cover c, const ucs::variable_set &variables, int level)
{
	parse_expression::expression result;
	result.level = level;
	result.valid = true;

	boolean::cover nc = ~c;

	if (c.cubes.size() == 0)
		result.arguments.push_back(parse_expression::argument("0"));
	else if (nc.cubes.size() == 0)
		result.arguments.push_back(parse_expression::argument("1"));
	else if (c.cubes.size() == 1 || nc.cubes.size() == 1)
	{
		if (level == 1)//parse_expression::expression::level_has(level, "&"))
		{
			c = nc;
			result.level = 0;//parse_expression::expression::get_level("|");
		}

		for (int i = 0; i < (int)c.cubes.size(); i++)
			result.arguments.push_back(parse_expression::argument(export_expression(c.cubes[i], variables)));

		for (int i = 1; i < (int)result.arguments.size(); i++)
			result.operations.push_back(result.precedence[result.level].symbols[0]);
	}
	else
	{
		boolean::cover c_left, c_right, nc_left, nc_right;
		float c_weight, nc_weight;

		c_weight = c.partition(c_left, c_right);
		nc_weight = nc.partition(nc_left, nc_right);

		if (c_weight <= nc_weight)
		{
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(c_left, variables, level)));
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(c_right, variables, level)));
			result.operations.push_back(result.precedence[result.level].symbols[0]);
		}
		else if (nc_weight < c_weight)
		{
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(nc_left, variables, 1-level)));
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(nc_right, variables, 1-level)));
			result.level = 1-level;
			result.operations.push_back(result.precedence[result.level].symbols[0]);
		}
	}

	for (int i = 0; i < (int)result.arguments.size(); )
	{
		if (result.arguments[i].sub.valid && (result.arguments[i].sub.level == result.level || (result.arguments[i].sub.arguments.size() == 1 && result.arguments[i].sub.operations.size() == 0)))
		{
			result.arguments.insert(result.arguments.begin() + i+1, result.arguments[i].sub.arguments.begin(), result.arguments[i].sub.arguments.end());
			result.operations.insert(result.operations.begin() + i, result.arguments[i].sub.operations.begin(), result.arguments[i].sub.operations.end());
			result.arguments.erase(result.arguments.begin() + i);
		}
		else
			i++;
	}

	return result;
}

parse_expression::expression export_expression_hfactor(boolean::cover c, const ucs::variable_set &variables)
{
	parse_expression::expression result;
	result.valid = true;
	result.level = 0;//parse_expression::expression::get_level("|");

	if (c.is_null())
		result.arguments.push_back(parse_expression::argument("0"));
	else if (c.is_tautology())
		result.arguments.push_back(parse_expression::argument("1"));
	else if (c.cubes.size() == 1)
		result.arguments.push_back(parse_expression::argument(export_expression(c.cubes[0], variables)));
	else
	{
		boolean::cube common = c.supercube();
		if (common == 1)
		{
			boolean::cover c_left, c_right;
			c.partition(c_left, c_right);

			result.arguments.push_back(parse_expression::argument(export_expression_hfactor(c_left, variables)));
			result.arguments.push_back(parse_expression::argument(export_expression_hfactor(c_right, variables)));
			result.operations.push_back("|");
		}
		else
		{
			c.cofactor(common);

			result.arguments.push_back(parse_expression::argument(export_expression(common, variables)));
			result.arguments.push_back(parse_expression::argument(export_expression_hfactor(c, variables)));
			result.operations.push_back("&");
			result.level = 1;//parse_expression::expression::get_level("&");
		}
	}

	for (int i = 0; i < (int)result.arguments.size(); )
	{
		if (result.arguments[i].sub.valid && (result.arguments[i].sub.level == result.level || (result.arguments[i].sub.arguments.size() == 1 && result.arguments[i].sub.operations.size() == 0)))
		{
			result.arguments.insert(result.arguments.begin() + i+1, result.arguments[i].sub.arguments.begin(), result.arguments[i].sub.arguments.end());
			result.operations.insert(result.operations.begin() + i, result.arguments[i].sub.operations.begin(), result.arguments[i].sub.operations.end());
			result.arguments.erase(result.arguments.begin() + i);
		}
		else
			i++;
	}

	return result;
}

