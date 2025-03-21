/*------------------------------------------------------------------------*/
/*! \file pacheck.cpp
    Main file

    Part of Pacheck 2.0 : PAC proof checker.
    Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#include "checker.h"
/*------------------------------------------------------------------------*/
#define VERSION "2.0"

static const char * usage =
"pacheck [ <option> ... ]  [ <polynomials> <proof>] [<spec>]\n"
"\n"
"where <option> is one of the following\n"
"\n"
"  -h | --help           print this command line option summary and exit\n"
"\n"
"  -s | --no-target      only check inferences but not that target is infered\n"
"\n"
"  -d | --no-delete      ignore delete rules\n"
"\n"
"  -s0                   sort variables according to strcmp(default)\n"
"  -s1                   sort variables according to -1*strcmp\n"
"  -s2                   sort variables according to input order\n"
"  -s3                   sort variables according to reverse input order\n"
"\n"
"The <polynomials> argument should point to a file with the\n"
"original set of polynomials and <proof> is a path to a proof file\n"
"interpreted as a sequence of inferences in the polynomial calculus.\n"
"The tool checks that all inferences in the sequence are correct.\n"
"\n"
"<spec> is optional. Ommiting this file is the same as choosing option '-s'\n"
"It should point to a file with a single polynomial which\n"
"should be generated by the proof.\n"
"The exit code is zero if and only if all checks succeed.\n";
/*------------------------------------------------------------------------*/
static void banner() {
  msg("Pacheck Version " VERSION);
  msg("Practical Algebraic Calculus Proof Checker");
  msg("Copyright(C) 2020, Daniela Kaufmann, Johannes Kepler University Linz");
}

int main(int argc, char ** argv) {
  const char * poly_file_name = 0;
  const char * proof_file_name = 0;
  const char * target_file_name = 0;
  int sort_chosen = 0;

  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-h") ||
        !strcmp(argv[i], "--help")) {
      fputs(usage, stdout), exit(0);
    } else if (!strcmp(argv[i], "-s") ||
             !strcmp(argv[i], "--no-target")) {
      check_target = 0;
    } else if (!strcmp(argv[i], "-v") ||
             !strcmp(argv[i], "--verbose")) {
      verbose = 1;
    } else if (!strcmp(argv[i], "-d") ||
             !strcmp(argv[i], "--no-delete")) {
      delete_mode = 0;
    } else if (!strcmp(argv[i], "-s0")) {
      sort = 0;
      sort_chosen++;
    } else if (!strcmp(argv[i], "-s1")) {
      sort = 1;
      sort_chosen++;
    } else if (!strcmp(argv[i], "-s2")) {
      sort = 2;
      sort_chosen++;
    } else if (!strcmp(argv[i], "-s3")) {
      sort = 3;
      sort_chosen++;
    } else if (argv[i][0] == '-') {
      die("invalid command line option '%s'(try '-h')", argv[i]);
    } else if (target_file_name) {
      die("too many command line arguments(try '-h')");
    } else if (proof_file_name) {
      target_file_name = argv[i];
    } else if (poly_file_name) { proof_file_name = argv[i];
    } else {
      poly_file_name = argv[i];
    }
  }

  if (!target_file_name) check_target = 0;
  if (!proof_file_name) die("too few command line arguments(try '-h')");
  if (sort_chosen > 1) die("too many variable orderings selected");

  banner();
  init_nonces();


  if (sort == 1) msg("sorting according to reverse strcmp");
  else if (sort == 2) msg("sorting according to input order");
  else if (sort == 3) msg("sorting according to reverse input order");
  else
    msg("sorting according to strcmp");

  if (target_file_name && check_target) {
    msg("checking target enabled");
    parse_target_polynomial(target_file_name);
  }
  msg("");

  parse_and_check_proof(poly_file_name, proof_file_name);

  reset();

  msg("");
  msg("----------------------------------------------------------------------");
  if (check_target && target_polynomial_inferences) msg("c TARGET CHECKED");
  else if (check_target) msg("c INFERENCES CHECKED - TARGET IS NOT INFERRED");
  else
    msg("c INFERENCES CHECKED");
  msg("----------------------------------------------------------------------");

  if (constant_one_polynomial_inferences) msg("% CORRECT REFUTATION");

  checker_statistics();

  return 0;
}
