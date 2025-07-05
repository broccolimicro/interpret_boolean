#include "import.h"

namespace boolean {

string import_constant(const parse_expression::expression &syntax, tokenizer *tokens) {
	if (not syntax.valid or syntax.level < 0 or syntax.arguments.empty()) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->internal("invalid expression", __FILE__, __LINE__);
		} else {
			internal("", "invaid expression", __FILE__, __LINE__);
		}
		return "0";
	}

	string result = "";
	if (syntax.operators.empty()) {
		result += import_constant(syntax.arguments[0], tokens);
	} else {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->internal("sub expressions in constants not supported", __FILE__, __LINE__);
		} else {
			internal("", "sub expressions in constants not supported", __FILE__, __LINE__);
		}
		return "0";
	}

	return result;
}

string import_constant(const parse_expression::argument &syntax, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_constant(syntax.sub, tokens);
	} else if (not syntax.literal.empty()) {
		internal(syntax.literal, "expected constant-valued expression", __FILE__, __LINE__);
		return "0";
	}
	return syntax.constant;
}

string import_net_name(const parse_expression::argument &syntax, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_net_name(syntax.sub, tokens);
	} else if (not syntax.literal.empty()) {
		return syntax.literal;
	}
	internal(syntax.constant, "expected instance", __FILE__, __LINE__);
	return "_";
}

string import_net_name(const parse_expression::expression &syntax, tokenizer *tokens) {
	if (not syntax.valid or syntax.level < 0 or syntax.arguments.empty()) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->internal("invalid expression", __FILE__, __LINE__);
		} else {
			internal("", "invaid expression", __FILE__, __LINE__);
		}
		return "_";
	}

	string result = "";
	if (syntax.operators.empty()) {
		result += import_net_name(syntax.arguments[0], tokens);
	} else if (syntax.precedence.isModifier(syntax.level)
		and syntax.precedence.at(syntax.level, syntax.operators[0]).trigger == "'") {
		string cnst = import_constant(syntax.arguments[1], tokens);
		result += import_net_name(syntax.arguments[0], tokens) + syntax.precedence.at(syntax.level, syntax.operators[0]).trigger + cnst;
	} else if (syntax.precedence.isModifier(syntax.level)
		and syntax.precedence.at(syntax.level, syntax.operators[0]).trigger == "[") {
		result += import_net_name(syntax.arguments[0], tokens) + syntax.precedence.at(syntax.level, syntax.operators[0]).trigger;
		for (int i = 1; i < (int)syntax.arguments.size(); i++) {
			if (i != 1) {
				result += syntax.precedence.at(syntax.level, syntax.operators[0]).infix;
			}
			result += import_constant(syntax.arguments[i], tokens);
		}
		result += syntax.precedence.at(syntax.level, syntax.operators[0]).postfix;
	} else if (syntax.precedence.isModifier(syntax.level)
		and syntax.precedence.at(syntax.level, syntax.operators[0]).trigger == ".") {
		for (int i = 0; i < (int)syntax.arguments.size(); i++) {
			if (i != 0) {
				result += syntax.precedence.at(syntax.level, syntax.operators[0]).trigger;
			}
			result += import_net_name(syntax.arguments[i], tokens);
		}
	} else {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->internal("sub expressions in variable names not supported", __FILE__, __LINE__);
		} else {
			internal("", "sub expressions in variabe names not supported", __FILE__, __LINE__);
		}
		return "_";
	}

	return result;
}

int import_net(string syntax, ucs::Netlist nets, tokenizer *tokens, bool auto_define) {
	int uid = nets.netIndex(syntax, auto_define);
	if (uid < 0) {
		if (tokens != nullptr) {
			tokens->error("undefined net '" + syntax + "'", __FILE__, __LINE__);
		} else {
			error("", "undefined net '" + syntax + "'", __FILE__, __LINE__);
		}
	}

	return uid;
}

int import_net(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	string name = import_net_name(syntax, tokens);
	if (default_id != 0) {
		name += "'" + ::to_string(default_id);
	}

	return import_net(name, nets, tokens, auto_define);
}

boolean::cube import_cube(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	if (syntax.operation == "+"
		or syntax.operation == "-"
		or syntax.operation == "~") {
		int uid = import_net(syntax.lvalue[0], nets, default_id, tokens, auto_define);
		if (uid < 0) {
			return boolean::cube();
		}

		int value = -1;
		if (syntax.operation == "+") {
			value = 1;
		} else if (syntax.operation == "-") {
			value = 0;
		}

		return boolean::cube(uid, value);
	} else if (syntax.operation == "=") {
		// TODO(edward.bingham) support batch assignments
		int uid = import_net(syntax.lvalue[0], nets, default_id, tokens, auto_define);
		if (uid < 0) {
			return boolean::cube();
		}

		boolean::cover temp = import_cover(syntax.rvalue, nets, default_id, tokens, auto_define);
		if (temp.is_tautology()) {
			return boolean::cube(uid, 1);
		} else if (temp.is_null()) {
			return boolean::cube(uid, 0);
		} else {
			if (tokens != nullptr) {
				tokens->load(&syntax.rvalue);
				tokens->error("unsupported operation", __FILE__, __LINE__);
			} else {
				error(syntax.rvalue.to_string(), "unsupported operation", __FILE__, __LINE__);
			}
			return boolean::cube();
		}
	}
	if (tokens != nullptr) {
		tokens->load(&syntax);
		tokens->error("unsupported operation", __FILE__, __LINE__);
	} else {
		error(syntax.to_string(), "unsupported operation", __FILE__, __LINE__);
	}
	return boolean::cube();
}

boolean::cover import_cover(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	return boolean::cover(import_cube(syntax, nets, default_id, tokens, auto_define));
}

boolean::cube import_cube(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	if (syntax.region != "") {
		default_id = atoi(syntax.region.c_str());
	}

	if (syntax.level >= (int)syntax.precedence.size()) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		}
		return boolean::cube(1);
	} else if (syntax.literals.empty() and syntax.compositions.empty()) {
		return boolean::cube(1);
	} else if (syntax.precedence[syntax.level] == ":" and syntax.literals.size() + syntax.compositions.size() > 1) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->error("illegal disjunction", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "illegal disjunction", __FILE__, __LINE__);
		}
		return boolean::cube();
	}

	boolean::cube result;

	for (int i = 0; i < (int)syntax.literals.size(); i++) {
		if (syntax.literals[i].valid) {
			result = boolean::parallel(result, import_cube(syntax.literals[i], nets, default_id, tokens, auto_define));
		}
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		if (syntax.compositions[i].valid) {
			result = boolean::parallel(result, import_cube(syntax.compositions[i], nets, default_id, tokens, auto_define));
		}
	}

	return result;
}

boolean::cover import_cover(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	if (syntax.region != "") {
		default_id = atoi(syntax.region.c_str());
	}

	if (syntax.level >= (int)syntax.precedence.size()) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		}
		return boolean::cover(0);
	} else if (syntax.literals.empty() and syntax.compositions.empty()) {
		return boolean::cover(1);
	}

	boolean::cover result(syntax.precedence[syntax.level] == "," ? 1 : 0);

	for (int i = 0; i < (int)syntax.literals.size(); i++) {
		if (syntax.literals[i].valid) {
			boolean::cover sub = import_cube(syntax.literals[i], nets, default_id, tokens, auto_define);
			if (syntax.precedence[syntax.level] == ":") {
				result = boolean::choice(result, sub);
			} else if (syntax.precedence[syntax.level] == ",") {
				result = boolean::parallel(result, sub);
			}
		}
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		if (syntax.compositions[i].valid) {
			boolean::cover sub = import_cover(syntax.compositions[i], nets, default_id, tokens, auto_define);
			if (syntax.precedence[syntax.level] == ":") {
				result = boolean::choice(result, sub);
			} else if (syntax.precedence[syntax.level] == ",") {
				result = boolean::parallel(result, sub);
			}
		}
	}

	return result;
}

boolean::cover import_argument(const parse_expression::expression &syntax, const parse_expression::argument &arg, bool isCover, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	if (arg.sub.valid) {
		if (arg.sub.level >= 13 and not arg.sub.operators.empty()) {
			int uid = import_net(arg.sub, nets, default_id, tokens, auto_define);
			if (uid >= 0) {
				return boolean::cover(boolean::cube(uid, 1));
			}
			return boolean::cover(0);
		} else if (isCover) {
			return import_cover(arg.sub, nets, default_id, tokens, auto_define);
		}
		return import_cube(arg.sub, nets, default_id, tokens, auto_define);
	} else if (not arg.literal.empty()) {
		string name = arg.literal;
		if (default_id != 0) {
			name += "'" + ::to_string(default_id);
		}

		int uid = import_net(name, nets, tokens, auto_define);
		if (uid >= 0) {
			return boolean::cover(boolean::cube(uid, 1));
		}
		return boolean::cover(0);
	} else if (not arg.constant.empty()) {
		if (arg.constant == "0"
			or arg.constant == "false") {
			return boolean::cover(0);
		} else if (arg.constant == "1"
			or arg.constant == "true") {
			return boolean::cover(1);
		} else if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->error("unrecognized constant value", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized constant value", __FILE__, __LINE__);
		}
	}
	return boolean::cover(0);
}

boolean::cube import_cube(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	if (syntax.level >= (int)syntax.precedence.size()) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		}
		return boolean::cube(0);

	}

	boolean::cover result;
	if (syntax.precedence.isModifier(syntax.level) and not syntax.operators.empty() and syntax.precedence.at(syntax.level, 0).trigger == "'") {
		default_id = atoi(import_constant(syntax.arguments[1], tokens).c_str());
		result = import_argument(syntax, syntax.arguments[0], false, nets, default_id, tokens, auto_define);
	} else {
		for (int i = 0; i < (int)syntax.arguments.size(); i++) {
			// interpret the operands
			boolean::cover sub = import_argument(syntax, syntax.arguments[i], false, nets, default_id, tokens, auto_define);

			// interpret the operators
			bool err = false;
			if (i == 0) {
				if (syntax.precedence.isUnary(syntax.level)) {
					for (int j = (int)syntax.operators.size()-1; j >= 0; j--) {
						if (syntax.precedence.at(syntax.level, syntax.operators[j]).prefix == "~") {
							sub = ~sub;
							if (sub.cubes.size() > 1) {
								err = true;
							}
						} else if (syntax.precedence.at(syntax.level, syntax.operators[j]).prefix == "?") {
							sub = sub.nulled();
							if (sub.cubes.size() > 1) {
								err = true;
							}
						} else {
							err = true;
						}
					}
				}

				result = sub;
			} else if (syntax.precedence.isBinary(syntax.level) and syntax.precedence.at(syntax.level, syntax.operators[i-1]).infix == "&") {
				result &= sub;
			} else if (not syntax.operators.empty()) {
 
				err = true;
			}

			if (err) {
				if (tokens != nullptr) {
					tokens->load(&syntax);
					tokens->error("unsupported operation", __FILE__, __LINE__);
				} else {
					error(syntax.to_string(), "unsupported operation", __FILE__, __LINE__);
				}
				return boolean::cube();
			}
		}
	}

	if (result.cubes.empty()) {
		return boolean::cube(0);
	} else {
		return result.cubes[0];
	}
}

boolean::cover import_cover(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	if (syntax.level >= (int)syntax.precedence.size()) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		}
		return boolean::cover(0);
	}

	if (syntax.precedence.isModifier(syntax.level) and not syntax.operators.empty() and syntax.precedence.at(syntax.level, 0).trigger == "'") {
		default_id = atoi(import_constant(syntax.arguments[1], tokens).c_str());
		return import_argument(syntax, syntax.arguments[0], true, nets, default_id, tokens, auto_define);
	}

	boolean::cover result;
	for (int i = 0; i < (int)syntax.arguments.size(); i++) {
		// interpret the operands
		boolean::cover sub = import_argument(syntax, syntax.arguments[i], true, nets, default_id, tokens, auto_define);

		// interpret the operators
		bool err = false;
		if (i == 0) {
			if (syntax.precedence.isUnary(syntax.level)) {
				for (int j = (int)syntax.operators.size()-1; j >= 0; j--) {
					if (syntax.precedence.at(syntax.level, syntax.operators[j]).prefix == "~") {
						sub = ~sub;
					} else if (syntax.precedence.at(syntax.level, syntax.operators[j]).prefix == "?") {
						sub = sub.nulled();
					} else {
						err = true;
					}
				}
			}
			if (sub.is_null()) {
				result = 0;
			} else {
				result = sub;
			}
		} else if (syntax.precedence.isBinary(syntax.level)) {
			if (syntax.precedence.at(syntax.level, syntax.operators[i-1]).infix == "|") {
				result |= sub;
			} else if (syntax.precedence.at(syntax.level, syntax.operators[i-1]).infix == "&") {
				result &= sub;
			} else {
				err = true;
			}
		} else {
			err = true;
		}

		if (err) {
			if (tokens != nullptr) {
				tokens->load(&syntax);
				tokens->error("unsupported operation", __FILE__, __LINE__);
			} else {
				error(syntax.to_string(), "unsupported operation", __FILE__, __LINE__);
			}
		}
	}

	return result;
}

boolean::unsigned_int import_unsigned_int(const parse_expression::expression &syntax, map<string, boolean::unsigned_int> &variables, int default_id, tokenizer *tokens) {
	if (syntax.level >= (int)syntax.precedence.size()) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		}
		return boolean::unsigned_int(0);
	}

	boolean::unsigned_int result;
	if (syntax.precedence.isModifier(syntax.level) and not syntax.operators.empty() and syntax.precedence.at(syntax.level, 0).trigger == "'") {
		default_id = atoi(import_constant(syntax.arguments[1], tokens).c_str());
		// interpret the operands
		boolean::unsigned_int sub;
		if (syntax.arguments[0].sub.valid) {
			if (syntax.arguments[0].sub.level >= 12 and not syntax.arguments[0].sub.operators.empty()) {
				map<string, boolean::unsigned_int>::iterator id = variables.find(syntax.arguments[0].sub.to_string(""));
				if (id != variables.end()) {
					sub = id->second;
				} else if (tokens != nullptr) {
					tokens->load(&syntax);
					tokens->error("reference to undefined variable", __FILE__, __LINE__);
				} else {
					error(syntax.to_string(), "reference to undefined variable", __FILE__, __LINE__);
				}
			} else {
				sub = import_unsigned_int(syntax.arguments[0].sub, variables, default_id, tokens);
			}
		} else if (not syntax.arguments[0].literal.empty()) {
			map<string, boolean::unsigned_int>::iterator id = variables.find(syntax.arguments[0].literal);
			if (id != variables.end()) {
				sub = id->second;
			} else if (tokens != nullptr) {
				tokens->load(&syntax);
				tokens->error("reference to undefined variable", __FILE__, __LINE__);
			} else {
				error(syntax.to_string(), "reference to undefined variable", __FILE__, __LINE__);
			}
		} else if (not syntax.arguments[0].constant.empty()) {
			sub = boolean::unsigned_int(atoi(syntax.arguments[0].constant.c_str()));
		}
		result = sub;
	} else {
		for (int i = 0; i < (int)syntax.arguments.size(); i++) {
			// interpret the operands
			boolean::unsigned_int sub;
			if (syntax.arguments[i].sub.valid) {
				if (syntax.arguments[i].sub.level >= 12 and not syntax.arguments[i].sub.operators.empty()) {
					map<string, boolean::unsigned_int>::iterator id = variables.find(syntax.arguments[i].sub.to_string(""));
					if (id != variables.end()) {
						sub = id->second;
					} else if (tokens != nullptr) {
						tokens->load(&syntax);
						tokens->error("reference to undefined variable", __FILE__, __LINE__);
					} else {
						error(syntax.to_string(), "reference to undefined variable", __FILE__, __LINE__);
					}
				} else {
					sub = import_unsigned_int(syntax.arguments[i].sub, variables, default_id, tokens);
				}
			} else if (not syntax.arguments[i].literal.empty()) {
				map<string, boolean::unsigned_int>::iterator id = variables.find(syntax.arguments[i].literal);
				if (id != variables.end()) {
					sub = id->second;
				} else if (tokens != nullptr) {
					tokens->load(&syntax);
					tokens->error("reference to undefined variable", __FILE__, __LINE__);
				} else {
					error(syntax.to_string(), "reference to undefined variable", __FILE__, __LINE__);
				}
			} else if (not syntax.arguments[i].constant.empty()) {
				sub = boolean::unsigned_int(atoi(syntax.arguments[i].constant.c_str()));
			}

			// interpret the operators
			bool err = false;
			if (i == 0) {
				if (syntax.precedence.isUnary(syntax.level)) {
					for (int j = (int)syntax.operators.size()-1; j >= 0; j--) {
						if (syntax.precedence.at(syntax.level, j).prefix == "~") {
							sub = ~sub;
						} else if (syntax.precedence.at(syntax.level, j).prefix == "+") {
						//} else if (syntax.precedence.at(syntax.level, j).prefix == "-") {
						//	sub = -sub;
						} else {
							err = true;
						}
					}
				}

				result = sub;
			} else if (syntax.precedence.isBinary(syntax.level)) {
				if (syntax.precedence.at(syntax.level, i-1).infix == "|") {
					result |= sub;
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "&") {
					result &= sub;
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "^") {
					result ^= sub;
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "==") {
					result = boolean::unsigned_int(boolean::bitset(result == sub));
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "~=") {
					result = boolean::unsigned_int(boolean::bitset(result != sub));
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "<") {
					result = boolean::unsigned_int(boolean::bitset(result < sub));
				} else if (syntax.precedence.at(syntax.level, i-1).infix == ">") {
					result = boolean::unsigned_int(boolean::bitset(result > sub));
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "<=") {
					result = boolean::unsigned_int(boolean::bitset(result <= sub));
				} else if (syntax.precedence.at(syntax.level, i-1).infix == ">=") {
					result = boolean::unsigned_int(boolean::bitset(result >= sub));
				//} else if (syntax.precedence.at(syntax.level, i-1).infix == "<<") {
				//	result <<= sub;
				//} else if (syntax.precedence.at(syntax.level, i-1).infix == ">>") {
				//	result >>= sub;
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "+") {
					result += sub;
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "-") {
					result -= sub;
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "*") {
					result *= sub;
				} else if (syntax.precedence.at(syntax.level, i-1).infix == "/") {
					result /= sub;
				//} else if (syntax.precedence.at(syntax.level, i-1).infix == "%") {
				//	result %= sub;
				} else {
					err = true;
				}
			} else {
				err = true;
			}

			if (err) {
				if (tokens != nullptr) {
					tokens->load(&syntax);
					tokens->error("unsupported operation", __FILE__, __LINE__);
				} else {
					error(syntax.to_string(), "unsupported operation", __FILE__, __LINE__);
				}
			}
		}
	}

	return result;
}

}
