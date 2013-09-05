/*
  Copyright (c) 2011 Sascha Kastens <mail@skastens.de>
  Copyright (c) 2011 Center for Bioinformatics, University of Hamburg

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

#ifndef LTR_CLUSTER_STREAM_API_H
#define LTR_CLUSTER_STREAM_API_H

#include "core/encseq_api.h"
#include "core/error_api.h"
#include "extended/node_stream_api.h"

typedef struct GtLTRClusterStream GtLTRClusterStream;

/* Implements the <GtNodeStream> interface. <GtLTRClusterStream> annotates
   all LTR features with cluster IDs, based on matches. */
GtNodeStream* gt_ltr_cluster_stream_new(GtNodeStream *in_stream,
                                        GtEncseq *encseq,
                                        int match_score,
                                        int mismatch_cost,
                                        int gap_open_cost,
                                        int gap_ext_cost,
                                        int xdrop,
                                        int ydrop,
                                        int zdrop,
                                        int k,
                                        int mscoregapped,
                                        int mscoregapless,
                                        GtUword plarge,
                                        GtUword psmall,
                                        char **current_state,
                                        GtError *err);

#endif
