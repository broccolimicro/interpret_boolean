#include "export.h"

namespace boolean {

parse_expression::assignment export_assignment(int uid, int value, ucs::ConstNetlist nets) {
	parse_expression::assignment result;
	result.valid = true;

	result.names.push_back(ucs::Net(nets.netAt(uid)));
	if (value == 0)
		result.operation = "-";
	else if (value == 1)
		result.operation = "+";
	else if (value == -1)
		result.operation = "~";

	return result;
}

parse_expression::composition export_composition(boolean::cube c, ucs::ConstNetlist nets) {
	static const int level = parse_expression::composition::get_level(",");
	parse_expression::composition result;
	result.valid = true;

	result.level = level;

	for (int uid = 0; uid < c.size()*16; uid++) {
		int val = c.get(uid);
		if (val != 2) {
			result.literals.push_back(export_assignment(uid, val, nets));
		}
	}

	return result;
}

parse_expression::composition export_composition(boolean::cover c, ucs::ConstNetlist nets) {
	static const int level = parse_expression::composition::get_level(":");
	parse_expression::composition result;
	result.valid = true;

	result.level = level;

	for (int i = 0; i < (int)c.cubes.size(); i++) {
		result.compositions.push_back(export_composition(c.cubes[i], nets));
	}

	return result;
}

parse_expression::expression export_expression(int uid, int value, ucs::ConstNetlist nets) {
	static const int level = parse_expression::expression::get_level("~");

	parse_expression::expression result;
	result.valid = true;

	result.level = level;

	result.arguments.push_back(parse_ucs::variable_name(nets.netAt(uid)));
	if (value == 0)
		result.operations.push_back("~");
	else if (value == -1)
		result.operations.push_back("?");

	return result;
}

parse_expression::expression export_expression(boolean::cube c, ucs::ConstNetlist nets)
{
	static const int level = parse_expression::expression::get_level("&");
	parse_expression::expression result;
	result.valid = true;

	result.level = level;

	for (int uid = 0; uid < c.size()*16; uid++) {
		int val = c.get(uid);
		if (val != 2) {
			result.arguments.push_back(parse_expression::argument(export_expression(uid, val, nets)));
		}
	}

	if (result.arguments.size() == 0)
		result.arguments.push_back(parse_expression::argument("1"));

	for (int i = 1; i < (int)result.arguments.size(); i++)
		result.operations.push_back("&");

	return result;
}

parse_expression::expression export_expression(boolean::cover c, ucs::ConstNetlist nets)
{
	static const int level = parse_expression::expression::get_level("|");
	parse_expression::expression result;
	result.valid = true;

	result.level = level;

	for (int i = 0; i < (int)c.cubes.size(); i++)
		result.arguments.push_back(parse_expression::argument(export_expression(c.cubes[i], nets)));

	if (c.cubes.size() == 0)
		result.arguments.push_back(parse_expression::argument("0"));

	for (int i = 1; i < (int)result.arguments.size(); i++)
		result.operations.push_back("|");

	return result;
}

parse_expression::expression export_expression_xfactor(boolean::cover c, ucs::ConstNetlist nets, int level)
{
	static const int andlevel = parse_expression::expression::get_level("&");
	static const int orlevel = parse_expression::expression::get_level("|");
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
		if (level == andlevel)
		{
			c = nc;
			result.level = orlevel;
		}

		for (int i = 0; i < (int)c.cubes.size(); i++)
			result.arguments.push_back(parse_expression::argument(export_expression(c.cubes[i], nets)));

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
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(c_left, nets, level)));
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(c_right, nets, level)));
			result.operations.push_back(result.precedence[result.level].symbols[0]);
		}
		else if (nc_weight < c_weight)
		{
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(nc_left, nets, 1-level)));
			result.arguments.push_back(parse_expression::argument(export_expression_xfactor(nc_right, nets, 1-level)));
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

parse_expression::expression export_expression_hfactor(boolean::cover c, ucs::ConstNetlist nets)
{
	static const int andlevel = parse_expression::expression::get_level("&");
	static const int orlevel = parse_expression::expression::get_level("|");

	parse_expression::expression result;
	result.valid = true;
	result.level = orlevel;

	if (c.is_null())
		result.arguments.push_back(parse_expression::argument("0"));
	else if (c.is_tautology())
		result.arguments.push_back(parse_expression::argument("1"));
	else if (c.cubes.size() == 1)
		result.arguments.push_back(parse_expression::argument(export_expression(c.cubes[0], nets)));
	else
	{
		boolean::cube common = c.supercube();
		if (common == 1)
		{
			boolean::cover c_left, c_right;
			c.partition(c_left, c_right);

			result.arguments.push_back(parse_expression::argument(export_expression_hfactor(c_left, nets)));
			result.arguments.push_back(parse_expression::argument(export_expression_hfactor(c_right, nets)));
			result.operations.push_back("|");
		}
		else
		{
			c.cofactor(common);

			result.arguments.push_back(parse_expression::argument(export_expression(common, nets)));
			result.arguments.push_back(parse_expression::argument(export_expression_hfactor(c, nets)));
			result.operations.push_back("&");
			result.level = andlevel;
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

}
