#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <boolean/cube.h>
#include <boolean/cover.h>

#include <parse_expression/composition.h>
#include <parse_expression/expression.h>

#include <parse_ucs/variable_name.h>

namespace boolean {

parse_expression::expression export_field(string str);
parse_expression::expression export_member(string str);
parse_expression::expression export_net(string str);
parse_expression::expression export_net(int uid, ucs::ConstNetlist nets);

parse_expression::assignment export_assignment(int uid, int value, ucs::ConstNetlist nets);

parse_expression::composition export_composition(boolean::cube c, ucs::ConstNetlist nets);
parse_expression::composition export_composition(boolean::cover c, ucs::ConstNetlist nets);

parse_expression::expression export_expression(int uid, int value, ucs::ConstNetlist nets);
parse_expression::expression export_expression(boolean::cube c, ucs::ConstNetlist nets);
parse_expression::expression export_expression(boolean::cover c, ucs::ConstNetlist nets);

parse_expression::expression export_expression_xfactor(boolean::cover c, ucs::ConstNetlist nets, int level = -1);
parse_expression::expression export_expression_hfactor(boolean::cover c, ucs::ConstNetlist nets);

}
