#pragma once

#include <common/standard.h>
#include <common/interface.h>

#include <boolean/cube.h>
#include <boolean/cover.h>

#include <parse_expression/composition.h>
#include <parse_expression/expression.h>

#include <parse_ucs/variable_name.h>

#include "interface.h"

namespace boolean {

parse_ucs::variable_name export_net(int uid, ConstNetlist nets);
parse_expression::assignment export_assignment(int uid, int value, ConstNetlist nets);

parse_expression::composition export_composition(boolean::cube c, ConstNetlist nets);
parse_expression::composition export_composition(boolean::cover c, ConstNetlist nets);

parse_expression::expression export_expression(int uid, int value, ConstNetlist nets);
parse_expression::expression export_expression(boolean::cube c, ConstNetlist nets);
parse_expression::expression export_expression(boolean::cover c, ConstNetlist nets);

parse_expression::expression export_expression_xfactor(boolean::cover c, ConstNetlist nets, int level = 0);
parse_expression::expression export_expression_hfactor(boolean::cover c, ConstNetlist nets);

}
