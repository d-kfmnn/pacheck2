/*------------------------------------------------------------------------*/
/*! \file signal_statistics.cpp
    \brief used to handle signals, messages and statistics

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/

#include "signal_statistics.h"

/*------------------------------------------------------------------------*/
// Global variable
int verbose = 0;


unsigned max_inferences;
unsigned max_terms;
unsigned total_terms;
unsigned hits_terms;
unsigned searched_terms;
unsigned collisions_terms;
/*------------------------------------------------------------------------*/
static int size_proof;
static int degree_proof;
static int length_proof;

/*------------------------------------------------------------------------*/

/**
    Returns name of signal

    @param sig integer

    @return char *
*/
static const char * signal_name(int sig) {
  switch (sig) {
    case SIGINT: return "SIGINT";
    case SIGSEGV: return "SIGSEGV";
    case SIGABRT: return "SIGABRT";
    case SIGTERM: return "SIGTERM";
    default: return "SIGUNKNOWN";
  }
}

/*------------------------------------------------------------------------*/
/**
    Catch signal and prints corresponding message

    @param sig integer
*/
static void catch_signal(int sig) {
  printf("c\nc caught signal '%s'(%d)\nc\n", signal_name(sig), sig);
  printf("c\nc raising signal '%s'(%d) again\n", signal_name(sig), sig);
  reset_all_signal_handlers();
  fflush(stdout);
  raise(sig);
}

/*------------------------------------------------------------------------*/
void(*original_SIGINT_handler)(int);
void(*original_SIGSEGV_handler)(int);
void(*original_SIGABRT_handler)(int);
void(*original_SIGTERM_handler)(int);
/*------------------------------------------------------------------------*/

void init_all_signal_handers() {
  original_SIGINT_handler  = signal(SIGINT,  catch_signal);
  original_SIGSEGV_handler = signal(SIGSEGV, catch_signal);
  original_SIGABRT_handler = signal(SIGABRT, catch_signal);
  original_SIGTERM_handler = signal(SIGTERM, catch_signal);
}

/*------------------------------------------------------------------------*/

void reset_all_signal_handlers() {
  (void) signal(SIGINT, original_SIGINT_handler);
  (void) signal(SIGSEGV, original_SIGSEGV_handler);
  (void) signal(SIGABRT, original_SIGABRT_handler);
  (void) signal(SIGTERM, original_SIGTERM_handler);
}

/*------------------------------------------------------------------------*/

void msg(const char *fmt, ...) {
  va_list ap;
  fputs_unlocked("[pck2] ", stdout);
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
  fputc_unlocked('\n', stdout);
  fflush(stdout);
}

/*------------------------------------------------------------------------*/

void die(const char *fmt, ...) {
  fflush(stdout);
  va_list ap;
  fputs_unlocked("*** [pck2] ", stderr);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  fflush(stderr);
  exit(1);
}

/*------------------------------------------------------------------------*/

static double percent(unsigned a, unsigned b) { return b ? 100.0*a/b : 0; }
static float average(unsigned a, unsigned b) { return b ? static_cast<float>(a)/b : 0; }

/*------------------------------------------------------------------------*/
void update_statistics_for_newly_added_polynomial(int degree, int size) {
    length_proof++;
    size_proof += size;
    if (degree > degree_proof) degree_proof = degree;
}
/*------------------------------------------------------------------------*/

void print_statistics(unsigned original_inferences,
  unsigned extension_inferences,
  unsigned lin_comb_inferences,
  unsigned deletion_inferences,
  unsigned num_inference_rules,
  unsigned addition_operations,
  unsigned multiplication_operations,
  unsigned new_patterns_count,
  unsigned apply_patterns_count) {
  msg("");
  msg("proof length: %22i (total number of polynomials)", length_proof);
  msg("proof size:   %22i (total number of monomials)",  size_proof);
  msg("proof degree: %22i ", degree_proof);
  msg("");
  msg("");
  msg("patterns: %26u", new_patterns_count);
  msg("apply patterns: %20i (average: %.1f apply per pattern)", apply_patterns_count, average(apply_patterns_count,new_patterns_count));
  msg("");
  msg("total inferences: %18" PRIu64, max_inferences);
  msg("original inferences: %15" PRIu64 " (%.0f%% of total rules)",
    original_inferences, percent(original_inferences, max_inferences));
  msg("proof rules:   %21" PRIu64 " (%.0f%% of total rules)",
      num_inference_rules, percent(num_inference_rules, max_inferences));
  msg("  extensions:  %21" PRIu64 " (%.0f%% of inference rules)",
     extension_inferences, percent(extension_inferences, num_inference_rules));
  msg("  linear combination: %14" PRIu64 " (%.0f%% of inference rules",
    lin_comb_inferences, percent(lin_comb_inferences, num_inference_rules));
  msg("                                       containing %" PRIu64 " additions",
        addition_operations);
  msg("                                       and %" PRIu64 " multiplications)",
        multiplication_operations);

  msg("rules deleted: %21" PRIu64 " (%.0f%% of total rules)",
      deletion_inferences,
      percent(deletion_inferences, num_inference_rules+original_inferences));
  msg("");

  msg("total allocated terms: %13" PRIu64, total_terms);
  msg("max allocated terms: %15" PRIu64 " (%.0f%% of total terms)",
      max_terms, percent(max_terms, total_terms));

  msg("searched terms: %20" PRIu64 " (%.0f%% hits,",
    searched_terms, percent(hits_terms, searched_terms));
  msg("                                       %.1f average collisions)",
      average(collisions_terms, searched_terms));
  msg("searched inferences: %15" PRIu64 " (%.1f average searches,",
    searched_inferences, average(searched_inferences, max_inferences));
  msg("                                       %.1f average collisions)",
    average(collisions_inferences, searched_inferences));
#ifdef HAVEGETRUSAGE
  struct rusage u;
  if (getrusage(RUSAGE_SELF, &u)) return;
  size_t s =((size_t) u.ru_maxrss) << 10;
  msg("");
  msg("maximum resident set size: %9.2f  MB", s/static_cast<double>(1<<20));
  double t = u.ru_utime.tv_sec + 1e-6 * u.ru_utime.tv_usec;
  t += u.ru_stime.tv_sec + 1e-6 * u.ru_stime.tv_usec;
  msg("process time: %22.2f  seconds", t);
#endif
}
