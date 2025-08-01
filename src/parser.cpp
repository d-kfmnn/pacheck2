/*------------------------------------------------------------------------*/
/*! \file parser.cpp
    \brief core functions for parsing

  Part of Pacheck 3.0 : PAC proof checker.
*/
/*------------------------------------------------------------------------*/
#include "parser.h"

#include <iostream>
#include <regex>
#include <string>
#include <unordered_set>
/*------------------------------------------------------------------------*/
std::unordered_map<int, Polynomial> id_to_poly;
std::unordered_set<std::string> allowed_variables;
std::vector<std::pair<std::string, int>> substitution_stack;
std::unordered_map<std::string, std::unordered_set<int>> declared_roots;

/*------------------------------------------------------------------------*/
int axiomrules = 0;  // Count of axiom rules processed
int lincomb_rules = 0;  // Count of linear combination rules processed
int branch_rules = 0;  // Count of branch rules processed
int delete_rules = 0;  // Count of delete rules processed
int root_rules = 0;  // Count of root rules processed
/*------------------------------------------------------------------------*/
int mod_value = -1;
bool mod_set = false;
/*------------------------------------------------------------------------*/
std::vector<Token> tokenize(const std::string& input) {
  std::vector<Token> tokens;
  std::string buffer;
  for (size_t i = 0; i < input.size(); ++i) {
    char ch = input[i];

    if (isspace(ch)) continue;

    if (isdigit(ch)) {
      buffer = ch;
      while (i + 1 < input.size() && isdigit(input[i + 1])) {
        buffer += input[++i];
      }
      tokens.push_back({TokenType::Number, buffer});
    } else if (isalpha(ch) || ch == 'l') {
      buffer = ch;
      while (i + 1 < input.size() && (isalnum(input[i + 1]) || input[i + 1] == '_')) {
        buffer += input[++i];
      }
      tokens.push_back({TokenType::Identifier, buffer});
    } else if (ch == '+' || ch == '-' || ch == '*' || ch == '^' || ch == '(' || ch == ')') {
      tokens.push_back({TokenType::Operator, std::string(1, ch)});
    } else {
      std::cerr << "Unexpected character in input: " << ch << "\n";
      exit(1);
    }
  }

  tokens.push_back({TokenType::End, ""});
  return tokens;
}
/*------------------------------------------------------------------------*/
Polynomial parseTerm(std::vector<Token>& tokens, size_t& i) {
  Polynomial result = parseFactor(tokens, i);

  while (tokens[i].type == TokenType::Operator && tokens[i].value == "*") {
    ++i;  // skip '*'
    Polynomial rhs = parseFactor(tokens, i);
    result = multiplyPolynomials(result, rhs);
  }

  return result;
}
/*------------------------------------------------------------------------*/
Polynomial parseExpression(std::vector<Token>& tokens, size_t& i) {
  Polynomial result = parseTerm(tokens, i);

  while (tokens[i].type == TokenType::Operator && (tokens[i].value == "+" || tokens[i].value == "-")) {
    std::string op = tokens[i++].value;
    Polynomial rhs = parseTerm(tokens, i);
    if (op == "+") {
      result = addPolynomials(result, rhs);
    } else {
      rhs = multiplyPolynomialByConstant(rhs, -1);
      result = addPolynomials(result, rhs);
    }
  }

  return result;
}
/*------------------------------------------------------------------------*/
Polynomial parseFactor(std::vector<Token>& tokens, size_t& i) {
  int sign = 1;

  // Handle unary minus
  while (tokens[i].type == TokenType::Operator && (tokens[i].value == "-" || tokens[i].value == "+")) {
    if (tokens[i].value == "-") sign *= -1;
    ++i;
  }

  if (tokens[i].type == TokenType::Number) {
    int coeff = std::stoi(tokens[i++].value);

    if (tokens[i].type == TokenType::Operator && tokens[i].value == "*") {
      ++i;
      Polynomial p = parseFactor(tokens, i);
      return multiplyPolynomialByConstant(p, sign * coeff);
    } else {
      return makePolynomial(sign * coeff);
    }
  }

  if (tokens[i].type == TokenType::Identifier) {
    std::string var = tokens[i++].value;
    int exp = 1;

    if (tokens[i].type == TokenType::Operator && tokens[i].value == "^") {
      ++i;
      if (tokens[i].type != TokenType::Number) {
        std::cerr << "Expected exponent after '^'\n";
        exit(1);
      }
      exp = std::stoi(tokens[i++].value);
    }

    Monomial mono = {{var, exp}};
    return makePolynomial(sign, mono);
  }

  if (tokens[i].type == TokenType::Operator && tokens[i].value == "(") {
    ++i;  // skip '('
    Polynomial p = parseExpression(tokens, i);
    if (tokens[i].value != ")") {
      std::cerr << "Expected ')' in expression\n";
      exit(1);
    }
    ++i;  // skip ')'
    return multiplyPolynomialByConstant(p, sign);
  }

  std::cerr << "Unexpected token in expression\n";
  exit(1);
}
/*------------------------------------------------------------------------*/



Polynomial parsePolynomial(const std::string& input) {
  std::vector<Token> tokens = tokenize(input);
  size_t index = 0;
  return parseExpression(tokens, index);
}
/*------------------------------------------------------------------------*/
void handleModRule(const std::smatch& match, int lineno) {
  if (mod_set) {
    std::cerr << "Error (line " << lineno << "): 'mod' rule already set.\n";
    exit(1);
  }
  mod_value = std::stoi(match[1]);
  mod_set = true;
}

/*------------------------------------------------------------------------*/
void handleAxiomRule(const std::smatch& match, int lineno) {
  int id = std::stoi(match[1]);
  if(id_to_poly.find(id) != id_to_poly.end()) {
    std::cerr << "Error (line " << lineno << "): Axiom rule ID " << id << " already exists.\n";
    exit(1);
  }
  if (!mod_set) {
    std::cerr << "Error (line " << lineno << "): 'mod' rule must be set before axiom rules.\n";
    exit(1);
  }

  Polynomial poly = parsePolynomial(match[2]);
  std::unordered_set<std::string> vars = getVariables(poly);
  allowed_variables.insert(vars.begin(), vars.end());
  id_to_poly[id] = poly;
}

/*------------------------------------------------------------------------*/
void handleDeleteRule(const std::smatch& match, int lineno) {
  if(id_to_poly.find(std::stoi(match[1])) == id_to_poly.end()) {
    std::cerr << "Error (line " << lineno << "): Delete rule for unknown ID.\n";
    exit(1);
  }

  int id = std::stoi(match[1]);
  id_to_poly.erase(id);
}

/*------------------------------------------------------------------------*/
void handleLinCombRule(const std::smatch& match, int lineno) {
  int target_id = std::stoi(match[1]);
  std::string operations = match[2];
  std::string result_str = match[3];

  Polynomial result;
  std::regex term_regex(R"((\d+)\s*\*\s*\(([^)]+)\))");
  auto it = std::sregex_iterator(operations.begin(), operations.end(), term_regex);
  auto end = std::sregex_iterator();

  for (; it != end; ++it) {
    int poly_id = std::stoi((*it)[1]);
    std::string multiplier_expr = (*it)[2];

    if (id_to_poly.find(poly_id) == id_to_poly.end()) {
      std::cerr << "Error (line " << lineno << "): Unknown polynomial ID " << poly_id << "\n";
      exit(1);
    }

    const Polynomial& base = id_to_poly[poly_id];
    Polynomial multiplier = parsePolynomial(multiplier_expr);

    if (!isSubset(getVariables(multiplier), allowed_variables)) {
      std::cerr << "Error (line " << lineno << "): Invalid multiplier introduces new variables\n";
      exit(1);
    }

    Polynomial term = multiplyPolynomials(base, multiplier);
    result = addPolynomials(result, term);
  }

  Polynomial expected = parsePolynomial(result_str);

  if (!polynomialsEqual(result, expected)) {
    std::cerr << "Error (line " << lineno << "): Mismatch in proof for ID " << target_id << "\n";
    exit(1);
  }

  id_to_poly[target_id] = expected;

  if (polynomialsOne(expected)) {
    while (!substitution_stack.empty()) {
      auto [var, value] = substitution_stack.back();
      substitution_stack.pop_back();
      declared_roots[var].erase(value);
      std::cout << "Derived 1 under assumption " << var << " = " << value << "\n";
      std::cout << "Removed root " << value << " for variable " << var << "\n";

      if (declared_roots[var].empty()) {
        std::cout << "▶ All roots for variable " << var << " resolved — closing branch.\n";
        declared_roots.erase(var);
        current_substitution.clear();
        for (const auto& [v, val] : substitution_stack) {
          current_substitution[v] = val;
        }
      } else {
        std::cout << "Remaining roots for variable " << var << ": ";
        for (int root : declared_roots[var]) {
          std::cout << root << " ";
        }
        std::cout << "\n";
        break;
      }
    }

    if (declared_roots.empty()) {
      std::cout << "No active substitutions left.\n";
    } else {
      std::cout << "Active substitutions: ";
      for (const auto& [var, val] : substitution_stack) {
        std::cout << var << "=" << val << " ";
      }
      std::cout << "\n";
    }
  }
}

/*------------------------------------------------------------------------*/
void handleRootRule(const std::smatch& match, int lineno) {
  int id = std::stoi(match[1]);
  std::string var = match[2];
  std::istringstream roots_stream(match[3]);
  std::vector<int> roots;
  int r;
  while (roots_stream >> r) {
    roots.push_back(r);
  }

  if (id_to_poly.find(id) == id_to_poly.end()) {
    std::cerr << "Error (line " << lineno << "): Unknown polynomial ID " << id << " in root rule.\n";
    exit(1);
  }

  Polynomial poly = id_to_poly[id];

  if (!isUnivariateIn(poly, var)) {
    std::cerr << "Error (line " << lineno << "): Polynomial ID " << id << " is not univariate in variable '" << var << "'\n";
    exit(1);
  }

  for (int root : roots) {
    int eval = evaluateAt(poly, var, root);
    if (eval != 0) {
      std::cerr << "Error (line " << lineno << "): " << root << " is not a root of polynomial ID " << id << "\n";
      exit(1);
    }

    declared_roots[var].insert(root);
  }
}

/*------------------------------------------------------------------------*/
void handleBranchRule(const std::smatch& match, int lineno) {
  std::string var = match[1];
  int value = std::stoi(match[2]);

  if (declared_roots.count(var) == 0 || declared_roots[var].count(value) == 0) {
    std::cerr << "Error (line " << lineno << "): Instantiation of " << var << " = " << value
              << " is invalid — root not declared.\n";
    exit(1);
  }

  substitution_stack.emplace_back(var, value);
  current_substitution[var] = value;

  std::cout << "Branch on " << var << " = " << value << std::endl;
}

/*------------------------------------------------------------------------*/
void processLine(std::string line, int lineno) {
  size_t comment_pos = line.find("//");
  if (comment_pos != std::string::npos) {
    line = line.substr(0, comment_pos);
  }

  line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");

  std::smatch match;
  static std::regex axiom_rule(R"(^\s*(\d+)\s+a\s+(.+?)\s*;?\s*$)");
  static std::regex delete_rule(R"(^\s*(\d+)\s+d\s*;?\s*$)");
  static std::regex mod_rule(R"(^\s*m\s+(\d+)\s*;?\s*$)");
  static std::regex linComb_rule(R"(^\s*(\d+)\s*%\s*(.+?)\s*,\s*(.+?)\s*;?\s*$)");
  static std::regex branch_rule(R"(^\s*b\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(-?\d+)\s*;?\s*$)");
  static std::regex root_rule(R"(^\s*(\d+)\s*r\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+((?:-?\d+\s*)+)\s*;?\s*$)");

  if (std::regex_match(line, match, mod_rule)) {
    handleModRule(match, lineno);
  } else if (std::regex_match(line, match, axiom_rule)) {
    handleAxiomRule(match, lineno);
    axiomrules++;
  } else if (std::regex_match(line, match, delete_rule)) {
    handleDeleteRule(match, lineno);
    delete_rules++;
  } else if (std::regex_match(line, match, linComb_rule)) {
    handleLinCombRule(match, lineno);
    lincomb_rules++;
  } else if (std::regex_match(line, match, root_rule)) {
    handleRootRule(match, lineno);
    root_rules++;
  } else if (std::regex_match(line, match, branch_rule)) {
    handleBranchRule(match, lineno);
    branch_rules++;
  } else {
    std::cerr << "Error (line " << lineno << "): Unrecognized or invalid line:\n" << line << std::endl;
    exit(1);
  }
}
/*------------------------------------------------------------------------*/


void printFinalStatistics() {
  
  std::cout << "  Axiom rules processed: " << axiomrules << "\n";
  std::cout << "  Linear combination rules processed: " << lincomb_rules << "\n";
  std::cout << "  Branch rules processed: " << branch_rules << "\n";
  std::cout << "  Delete rules processed: " << delete_rules << "\n";
  std::cout << "  Root rules processed: " << root_rules << "\n";

}
