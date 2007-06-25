/*
  Copyright (c) 2005-2007 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2005-2007 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#include "gt.h"

#define DELIMITER         ','
#define FORWARDSTRANDCHAR '+'
#define REVERSESTRANDCHAR '-'

typedef struct {
  Str* id;
  bool forward;
  Array *exons; /* the exon ranges */
} SplicedAlignment;

static void initSplicedAlignment(SplicedAlignment *sa, Env *env)
{
  assert(sa);
  sa->id = str_new(env);
  sa->forward = true;
  sa->exons = array_new(sizeof (Range), env);
}

static int parse_input_line(SplicedAlignment *alignment, const char *line,
                            unsigned long line_length, Env *env)
{
  long leftpos, rightpos;
  unsigned long i = 0;
  Range exon;
  env_error_check(env);

#define CHECKLINELENGTH\
        if (i >= line_length) {                    \
          env_error_set(env, "incomplete input line\n" \
                         "line=%s", line);         \
          return -1;                               \
        }

  /* parsing id */
  for (;;) {
    CHECKLINELENGTH;
    if (line[i] == DELIMITER) {
      /* reference id has been saved, skip this character and break */
      i++;
      CHECKLINELENGTH;
      break;
    }
    else {
      /* save this character of the reference id */
      str_append_char(alignment->id, line[i], env);
    }

    /* increase counter */
    i++;
  }

  /* parsing orientation */
  if (line[i] == FORWARDSTRANDCHAR)
    alignment->forward = true;
  else if (line[i] == REVERSESTRANDCHAR)
    alignment->forward = false;
  else {
    env_error_set(env, "wrong formatted input line, orientation must be %c or "
                  "%c\nline=%s", FORWARDSTRANDCHAR, REVERSESTRANDCHAR, line);
    return -1;
  }
  i++;
  CHECKLINELENGTH;

  if (line[i] != DELIMITER) {
    env_error_set(env, "incomplete input line\nline=%s", line);
    return -1;
  }

  for (;;) {
    if (line[i] == DELIMITER) {
      i++;
      CHECKLINELENGTH;
      if (sscanf(line+i, "%ld-%ld", &leftpos, &rightpos) != 2) {
        env_error_set(env, "incomplete input line\nline=%s", line);
        return -1;
      }
      exon.start = leftpos;
      exon.end   = rightpos;

      /* save exon */
      array_add(alignment->exons, exon, env);
    }
    i++;
    if (i >= line_length)
      break;
  }

  /* alignment contains at least one exon */
  assert(array_size(alignment->exons));

  return 0;
}

static int parse_input_file(Array *spliced_alignments,
                             const char *file_name, Env *env)
{
  FILE *input_file;
  SplicedAlignment sa;
  int had_err = 0;
  Str *line;
  env_error_check(env);

  line = str_new(env);
  input_file = env_fa_xfopen(env, file_name, "r");

  while (!had_err && str_read_next_line(line, input_file, env) != EOF) {
    /* init new spliced alignment */
    initSplicedAlignment(&sa, env);
    /* parse input line and save result in spliced alignment */
    had_err = parse_input_line(&sa, str_get(line), str_length(line), env);
    if (!had_err) {
      /* store spliced alignment */
      array_add(spliced_alignments, sa, env);
      /* reset array */
      str_reset(line);
    }
  }

  env_fa_xfclose(input_file, env);
  str_delete(line, env);
  return had_err;
}

static Range get_genomic_range(const void *sa)
{
  SplicedAlignment *alignment = (SplicedAlignment*) sa;
  Range range;
  assert(alignment);
  range.start = ((Range*) array_get_first(alignment->exons))->start;
  range.end   = ((Range*) array_get_last(alignment->exons))->end;
  return range;
}

static Strand get_strand(const void *sa)
{
  SplicedAlignment *alignment = (SplicedAlignment*) sa;
  if (alignment->forward)
    return STRAND_FORWARD;
  return STRAND_REVERSE;
}

static void get_exons(Array *exon_ranges, const void *sa, Env *env)
{
  SplicedAlignment *alignment = (SplicedAlignment*) sa;
  assert(alignment);
  array_add_array(exon_ranges, alignment->exons, env);
}

static void process_splice_form(Array *spliced_alignments_in_form,
                                /*@unused@*/ const void *set_of_sas,
                                /*@unused@*/ unsigned long number_of_sas,
                                /*@unused@*/ size_t size_of_sa,
                                /*@unused@*/ void *userdata,
                                /*@unused@*/ Env *env)
{
  unsigned long i;

  printf("contains [");
  for (i = 0; i < array_size(spliced_alignments_in_form); i++) {
    if (i)
      xputchar(',');
    printf("%lu", *((unsigned long*) array_get(spliced_alignments_in_form, i)));
  }
  printf("]\n");
}

static int range_compare_long_first(Range range_a, Range range_b)
{
  assert(range_a.start <= range_a.end && range_b.start <= range_b.end);

  if ((range_a.start == range_b.start) && (range_a.end == range_b.end))
    return 0; /* range_a == range_b */

  if ((range_a.start < range_b.start) ||
      ((range_a.start == range_b.start) && (range_a.end > range_b.end)))
    return -1; /* range_a < range_b */

  return 1; /* range_a > range_b */
}

static int compare_spliced_alignment(const void *a, const void *b)
{
  SplicedAlignment *sa_a = (SplicedAlignment*) a,
                   *sa_b = (SplicedAlignment*) b;
  Range range_a, range_b;
  range_a.start = ((Range*) array_get_first(sa_a->exons))->start;
  range_a.end   = ((Range*) array_get_last(sa_a->exons))->end;
  range_b.start = ((Range*) array_get_first(sa_b->exons))->start;
  range_b.end   = ((Range*) array_get_last(sa_b->exons))->end;
  return range_compare_long_first(range_a, range_b);
}

static OPrval parse_options(int *parsed_args, int argc, const char **argv,
                            Env *env)
{
  OptionParser *op;
  OPrval oprval;
  env_error_check(env);
  op = option_parser_new("spliced_alignment_file", "Read file containing "
                         "spliced alingments, compute consensus spliced "
                         "alignments,\nand print them to stdout.", env);
  oprval = option_parser_parse_min_max_args(op, parsed_args, argc, argv,
                                            versionfunc, 1, 1, env);
  option_parser_delete(op, env);
  return oprval;
}

int gt_consensus_sa(int argc, const char **argv, Env *env)
{
  Array *spliced_alignments;
  SplicedAlignment *sa;
  unsigned long i;
  int parsed_args, had_err = 0;
  env_error_check(env);

  /* option parsing */
  switch (parse_options(&parsed_args, argc, argv, env)) {
    case OPTIONPARSER_OK: break;
    case OPTIONPARSER_ERROR: return -1;
    case OPTIONPARSER_REQUESTS_EXIT: return 0;
  }
  assert(parsed_args == 1);

  /* parse input file and store resuilts in the spliced alignment array */
  spliced_alignments = array_new(sizeof (SplicedAlignment), env);
  had_err = parse_input_file(spliced_alignments, argv[1], env);

  if (!had_err) {
    /* sort spliced alignments */
    qsort(array_get_space(spliced_alignments), array_size(spliced_alignments),
          sizeof (SplicedAlignment), compare_spliced_alignment);

    /* compute the consensus spliced alignments */
    consensus_sa(array_get_space(spliced_alignments),
                 array_size(spliced_alignments), sizeof (SplicedAlignment),
                 get_genomic_range, get_strand, get_exons, process_splice_form,
                 NULL, env);
  }

  /* free */
  for (i = 0; i < array_size(spliced_alignments); i++) {
    sa = array_get(spliced_alignments, i);
    str_delete(sa->id, env);
    array_delete(sa->exons, env);
  }
  array_delete(spliced_alignments, env);

  return had_err;
}
