#include "polynomial.hpp"
#include <cmath>


std::unordered_map<std::string, int> current_substitution;

int mod(int x) {
    if (mod_value <= 0) return x; // no mod set
    int r = x % mod_value;
    return r < 0 ? r + mod_value : r;
}
//------------------------------------------------------------------------
Polynomial makePolynomial(int coeff, const Monomial& mono) {
    Polynomial p;
    if (mod(coeff) != 0) {
        p[mono] = mod(coeff);
    }
    return p;
}

//------------------------------------------------------------------------
Polynomial addPolynomials(const Polynomial& a, const Polynomial& b) {
    Polynomial result = a;

    for (const auto& [mono, coeff] : b) {
        result[mono] += coeff;
        result[mono] = mod(result[mono]);

        // Remove zero terms
        if (result[mono] == 0) {
            result.erase(mono);
        }
    }

    return result;
}

//------------------------------------------------------------------------
Polynomial multiplyPolynomialByConstant(const Polynomial& poly, int c) {
    Polynomial result;
    for (const auto& [mono, coeff] : poly) {
        int new_coeff = mod(coeff * c);
        if (new_coeff != 0) {
            result[mono] = new_coeff;
        }
    }
    return result;
}
//------------------------------------------------------------------------
Polynomial multiplyPolynomials(const Polynomial& a, const Polynomial& b) {
    Polynomial result;

    for (const auto& [ma, ca] : a) {
        for (const auto& [mb, cb] : b) {
            Monomial m;
            // Combine exponents
            m = ma;
            for (const auto& [var, exp] : mb) {
                m[var] += exp;
            }

            int coeff = mod(ca * cb);
            if (coeff != 0) {
                result[m] += coeff;
                result[m] = mod(result[m]);
                if (result[m] == 0) result.erase(m);
            }
        }
    }

    return result;
}

//------------------------------------------------------------------------
Polynomial substitute(const Polynomial& poly, const std::unordered_map<std::string, int>& subs) {
    Polynomial result;

    for (const auto& [monomial, coeff] : poly) {
        int numeric_factor = coeff;
        Monomial new_monomial;

        for (const auto& [var, exp] : monomial) {
            auto it = subs.find(var);
            if (it != subs.end()) {
                // Variable is substituted: multiply numeric_factor by (value^exp)
                numeric_factor *= static_cast<int>(std::pow(it->second, exp));
                numeric_factor = mod(numeric_factor);
            } else {
                // Variable not substituted: keep in monomial
                new_monomial[var] = exp;
            }
        }

        // Add to result
        if (numeric_factor != 0) {
            // Combine like terms:
            result[new_monomial] += numeric_factor;
            result[new_monomial] = mod(result[new_monomial]);
        }
    }

    // Remove zero coeff monomials if needed
    for (auto it = result.begin(); it != result.end(); ) {
        if (it->second == 0) it = result.erase(it);
        else ++it;
    }

    return result;
}
//------------------------------------------------------------------------
bool polynomialsEqual(const Polynomial& a, const Polynomial& b) {
    if (a == b) return true;

    Polynomial a_sub = substitute(a, current_substitution);
    Polynomial b_sub = substitute(b, current_substitution);
    if (a_sub == b_sub) return true;


    // If they are not equal, print the polynomials for debugging

    std::cerr << "Polynomials do not match:\n";
    std::cerr << "Expected (RHS): ";
    printPolynomial(b_sub);
    std::cerr << "\nComputed (LHS): ";
    printPolynomial(a_sub);
    std::cerr << "\n\n";

    return false;
}

//------------------------------------------------------------------------
bool polynomialsOne(const Polynomial& a) {
    Polynomial b = makePolynomial(1);
    if (a == b) return true;

    Polynomial a_sub = substitute(a, current_substitution);
    if (a_sub == b) return true;


    return false;
}

//------------------------------------------------------------------------
bool isUnivariateIn(const Polynomial& p, const std::string& var) {
    for (const auto& [mono, coeff] : p) {
        for (const auto& [v, _] : mono) {
            if (v != var)
                return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------
int evaluateAt(const Polynomial& p, const std::string& var, int value) {
    int result = 0;
    for (const auto& [mono, coeff] : p) {
        int term = coeff;
        for (const auto& [v, exp] : mono) {
            if (v != var)
                return -1; // multivariate, should not happen if univariate check passed
            int pow_val = 1;
            for (int i = 0; i < exp; ++i)
                pow_val = mod(pow_val * value);
            term = mod(term * pow_val);
        }
        result = mod(result + term);
    }
    return result;
}
//------------------------------------------------------------------------

void printPolynomial(const Polynomial& p) {
    if (p.empty()) {
        std::cout << "0";
        return;
    }

    bool first = true;
    for (const auto& [mono, coeff] : p) {
        int c = mod(coeff);
        if (c == 0) continue;

        // Sign
        if (!first) {
            if (c > 0) std::cout << " + ";
            else std::cout << " - ";
        } else {
            if (c < 0) std::cout << "-";
        }

        int abs_c = std::abs(c);
        bool need_coeff = (abs_c != 1 || mono.empty());

        if (need_coeff) std::cout << abs_c;

        for (const auto& [var, exp] : mono) {
            if (!need_coeff) {
                // if no coeff, don't add `*`
                std::cout << var;
            } else {
                std::cout << "*" << var;
            }

            if (exp != 1) {
                std::cout << "^" << exp;
            }

            need_coeff = true; // All further variables must be prefixed with '*'
        }

        first = false;
    }
}
//------------------------------------------------------------------------


std::unordered_set<std::string> getVariables(const Polynomial& poly) {
    std::unordered_set<std::string> vars;
    for (const auto& [mono, coeff] : poly) {
        for (const auto& [var, exp] : mono) {
            vars.insert(var);
        }
    }
    return vars;
}
//------------------------------------------------------------------------
bool isSubset(const std::unordered_set<std::string>& small, const std::unordered_set<std::string>& large) {
    for (const auto& v : small) {
        if (!large.count(v)) return false;
    }
    return true;
}

