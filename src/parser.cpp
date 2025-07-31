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

/*------------------------------------------------------------------------*/
int mod_value = -1;
bool mod_set = false;

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

Polynomial parseTerm(std::vector<Token>& tokens, size_t& i) {
  Polynomial result = parseFactor(tokens, i);

  while (tokens[i].type == TokenType::Operator && tokens[i].value == "*") {
    ++i;  // skip '*'
    Polynomial rhs = parseFactor(tokens, i);
    result = multiplyPolynomials(result, rhs);
  }

  return result;
}

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

Polynomial parsePolynomial(const std::string& input) {
  std::vector<Token> tokens = tokenize(input);
  size_t index = 0;
  return parseExpression(tokens, index);
}

void processLine(std::string line) {
  // Remove inline comments (starting with //)
  size_t comment_pos = line.find("//");
  if (comment_pos != std::string::npos) {
    line = line.substr(0, comment_pos);
  }

  // Trim leading and trailing whitespace
  line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");

  std::smatch match;
  // Axiom rule: "<id> a <polynomial>;"
  static std::regex axiom_rule(R"(^\s*(\d+)\s+a\s+(.+?)\s*;?\s*$)");

  // Delete rule: "<id> d;"
  static std::regex delete_rule(R"(^\s*(\d+)\s+d\s*;?\s*$)");

  // Modulus rule: "m <modulus>;"
  static std::regex mod_rule(R"(^\s*m\s+(\d+)\s*;?\s*$)");

  // Percent rule: "<id> % <op> , <result>;"
  static std::regex percent_rule(R"(^\s*(\d+)\s*%\s*(.+?)\s*,\s*(.+?)\s*;?\s*$)");

  // Branch instantiation rule: "b <var> <value>;"
  static std::regex branch_rule(R"(^\s*b\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(-?\d+)\s*;?\s*$)");

  // Root declaration rule: "<id> r <var> <roots...>;"
  static std::regex root_rule(R"(^\s*(\d+)\s*r\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+((?:-?\d+\s*)+)\s*;?\s*$)");

  if (std::regex_match(line, match, mod_rule)) {
    if (mod_set) {
      std::cerr << "Error: 'mod' rule already set.\n";
      exit(1);
    }
    mod_value = std::stoi(match[1]);
    mod_set = true;
    return;
  }

  if (std::regex_match(line, match, axiom_rule)) {
    int id = std::stoi(match[1]);
    Polynomial poly = parsePolynomial(match[2]);
    std::unordered_set<std::string> vars = getVariables(poly);
    allowed_variables.insert(vars.begin(), vars.end());
    id_to_poly[id] = poly;
    return;
  }

  if (std::regex_match(line, match, delete_rule)) {
    int id = std::stoi(match[1]);
    id_to_poly.erase(id);
    return;
  }

  if (std::regex_match(line, match, percent_rule)) {
    int target_id = std::stoi(match[1]);
    std::string operations = match[2];
    std::string result_str = match[3];

    Polynomial result;

    // Parse each term like "6 * (l32-1)"
    std::regex term_regex(R"((\d+)\s*\*\s*\(([^)]+)\))");
    auto it = std::sregex_iterator(operations.begin(), operations.end(), term_regex);
    auto end = std::sregex_iterator();

    std::unordered_set<std::string> used_vars;

    for (; it != end; ++it) {
      int poly_id = std::stoi((*it)[1]);
      std::string multiplier_expr = (*it)[2];

      if (id_to_poly.find(poly_id) == id_to_poly.end()) {
        std::cerr << "Error in line with ID " << target_id << ": Unknown polynomial ID" << poly_id << "\n";
        exit(1);
      }

      const Polynomial& base = id_to_poly[poly_id];
      Polynomial multiplier = parsePolynomial(multiplier_expr);

      // Check that multiplier uses no new variables
      if (!isSubset(getVariables(multiplier), allowed_variables)) {
        std::cerr << "Error in line with ID " << target_id << ": Invalid multiplier introduces new variables\n";
        exit(1);
      }

      Polynomial term = multiplyPolynomials(base, multiplier);
      result = addPolynomials(result, term);
    }

    Polynomial expected = parsePolynomial(result_str);

    if (!polynomialsEqual(result, expected)) {
      std::cerr << "Mismatch in proof for ID " << target_id << "\n";
      exit(1);
    }

    id_to_poly[target_id] = expected;

    if (polynomialsOne(expected)) {  
      while (!substitution_stack.empty()) {
        auto [var, value] = substitution_stack.back();
        substitution_stack.pop_back();
        // Remove the resolved root from declared_roots
        declared_roots[var].erase(value);
        std::cout << "✅ Derived 1 under assumption " << var << " = " << value << "\n";
        std::cout << "Removed root " << value << " for variable " << var << "\n";

        // If no more roots left for var, we can pop the substitution
        if (declared_roots[var].empty()) {
          std::cout << "▶ All roots for variable " << var << " resolved — closing branch.\n";
          declared_roots.erase(var);
          

          // Rebuild current substitution
          current_substitution.clear();
          for (const auto& [v, val] : substitution_stack) {
            current_substitution[v] = val;
          }
        } else {
          // There are still unresolved roots for this variable, stop recursion
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

    return;
  }

  if (std::regex_match(line, match, root_rule)) {
    int id = std::stoi(match[1]);
    std::string var = match[2];
    std::istringstream roots_stream(match[3]);
    std::vector<int> roots;
    int r;
    while (roots_stream >> r) {
      roots.push_back(r);
    }

    // Check existence
    if (id_to_poly.find(id) == id_to_poly.end()) {
      std::cerr << "Error: Unknown polynomial ID " << id << " in root rule.\n";
      exit(1);
    }

    Polynomial poly = id_to_poly[id];

    // Check univariate
    if (!isUnivariateIn(poly, var)) {
      std::cerr << "Error: Polynomial ID " << id << " is not univariate in variable '" << var << "'\n";
      exit(1);
    }

    // Check all roots
    for (int root : roots) {
      int eval = evaluateAt(poly, var, root);
      if (eval != 0) {
        std::cerr << "Error: " << root << " is not a root of polynomial ID " << id << "\n";
        exit(1);
      }

      declared_roots[var].insert(root);
    }
    return;
  }

  if (std::regex_match(line, match, branch_rule)) {
    std::string var = match[1];
    int value = std::stoi(match[2]);

    if (declared_roots.count(var) == 0 || declared_roots[var].count(value) == 0) {
      std::cerr << "Error: Instantiation of " << var << " = " << value
                << " is invalid — root not declared.\n";
      exit(1);
    }

    substitution_stack.emplace_back(var, value);
    current_substitution[var] = value;

    std::cout << "Branch on " << var << " = " << value << std::endl;
    return;
  }

  std::cerr << "Unrecognized or invalid line:\n"
            << line << std::endl;
  exit(1);
}
