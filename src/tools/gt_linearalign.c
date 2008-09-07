/*
  Copyright (c) 2007-2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2007-2008 Center for Bioinformatics, University of Hamburg

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "core/bioseq.h"
#include "core/option.h"
#include "core/unused_api.h"
#include "core/xansi.h"
#include "extended/alignment.h"
#include "extended/linearalign.h"
#include "tools/gt_linearalign.h"

static OptionParser* gt_linearalign_option_parser_new(GT_UNUSED
                                                      void *tool_arguments)
{
  OptionParser *op;
  op = option_parser_new("[option ...] seq_file_1 seq_file_2",
                         "Globally align each sequence in seq_file_1 with each "
                         "sequence in seq_file_2.\nThe memory consumption of "
                         "the alignment procedure is linear.");
  option_parser_set_min_max_args(op, 2, 2);
  return op;
}

static int gt_linearalign_runner(GT_UNUSED int argc, const char **argv,
                                 int parsed_args, GT_UNUSED void *tool_arguments,
                                 GT_Error *err)
{
  Bioseq *bioseq_1, *bioseq_2 = NULL;
  unsigned long i, j;
  int had_err = 0;
  Alignment *a;
  gt_error_check(err);

  /* init */
  bioseq_1 = bioseq_new(argv[parsed_args], err);
  if (!bioseq_1)
    had_err = -1;
  if (!had_err) {
    bioseq_2 = bioseq_new(argv[parsed_args+1], err);
    if (!bioseq_2)
      had_err = -1;
  }

  /* aligning all sequence combinations */
  if (!had_err) {
    for (i = 0; i < bioseq_number_of_sequences(bioseq_1); i++) {
      for (j = 0; j < bioseq_number_of_sequences(bioseq_2); j++) {
        a = linearalign(bioseq_get_sequence(bioseq_1, i),
                        bioseq_get_sequence_length(bioseq_1, i),
                        bioseq_get_sequence(bioseq_2, j),
                        bioseq_get_sequence_length(bioseq_2, j));
        alignment_show(a, stdout);
        xputchar('\n');
        alignment_delete(a);
      }
    }
  }

  /* free */
  bioseq_delete(bioseq_2);
  bioseq_delete(bioseq_1);

  return had_err;
}

Tool *gt_linearalign(void)
{
  return tool_new(NULL,
                  NULL,
                  gt_linearalign_option_parser_new,
                  NULL,
                  gt_linearalign_runner);
}
