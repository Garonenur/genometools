/* This file was generated by ./scripts/gen-display-struct.rb, do NOT edit. */
static GtSEdisplayStruct gt_display_arguments_table[] =
{
/* incolumn is true iff the helptext of the argument begins
   with the keyword "display" */
  {"alignment", Gt_Alignment_display, false},
  {"alignment length", Gt_Alignmentlength_display, true},
  {"bit score", Gt_Bitscore_display, true},
  {"blast", Gt_Blast_display, false},
  {"cigar", Gt_Cigar_display, true},
  {"cigarX", Gt_Cigarx_display, true},
  {"custom", Gt_Custom_display, false},
  {"dtrace", Gt_Dtrace_display, true},
  {"editdist", Gt_Editdist_display, true},
  {"evalue", Gt_Evalue_display, true},
  {"failed_seed", Gt_Failed_seed_display, false},
  {"fstperquery", Gt_Fstperquery_display, false},
  {"gap opens", Gt_Gapopens_display, true},
  {"gfa2", Gt_Gfa2_display, false},
  {"identity", Gt_Identity_display, true},
  {"indels", Gt_Indels_display, true},
  {"mismatches", Gt_Mismatches_display, true},
  {"polinfo", Gt_Polinfo_display, false},
  {"q.end", Gt_Q_end_display, true},
  {"q.len", Gt_Q_len_display, true},
  {"q.seqlen", Gt_Q_seqlen_display, true},
  {"q.seqnum", Gt_Q_seqnum_display, true},
  {"q.start", Gt_Q_start_display, true},
  {"query id", Gt_Queryid_display, true},
  {"s.end", Gt_S_end_display, true},
  {"s.len", Gt_S_len_display, true},
  {"s.seqlen", Gt_S_seqlen_display, true},
  {"s.seqnum", Gt_S_seqnum_display, true},
  {"s.start", Gt_S_start_display, true},
  {"score", Gt_Score_display, true},
  {"seed", Gt_Seed_display, false},
  {"seed.len", Gt_Seed_len_display, true},
  {"seed.q", Gt_Seed_q_display, true},
  {"seed.s", Gt_Seed_s_display, true},
  {"seed_in_algn", Gt_Seed_in_algn_display, false},
  {"strand", Gt_Strand_display, true},
  {"subject id", Gt_Subjectid_display, true},
  {"tabsep", Gt_Tabsep_display, false},
  {"trace", Gt_Trace_display, true}
};

static unsigned int gt_display_flag2index[] = {
   0,
   34,
   17,
   30,
   10,
   11,
   37,
   3,
   13,
   6,
   4,
   5,
   38,
   7,
   25,
   27,
   36,
   28,
   24,
   35,
   19,
   21,
   23,
   22,
   18,
   1,
   16,
   15,
   12,
   29,
   8,
   14,
   31,
   33,
   32,
   26,
   20,
   9,
   2
};

const char *gt_querymatch_display_help(void)
{
  return "specify what information about the matches to display\n"
         "alignment:        show alignment (possibly followed by\n"
         "                  =<number> to specify width of alignment\n"
         "                  columns, default is )\n"
         "seed_in_algn:     mark the seed in alignment\n"
         "polinfo:          add polishing information for shown\n"
         "                  alignment\n"
         "seed:             abbreviation for seed.len seed.s seed.q\n"
         "failed_seed:      show the coordinates of a seed extension,\n"
         "                  which does not satisfy the filter conditions\n"
         "fstperquery:      output only the first found match per query\n"
         "tabsep:           separate columns by tabulator, instead of\n"
         "                  space as default\n"
         "blast:            output matches in blast format 7 (tabular with\n"
         "                  comment lines; instead of gap opens, indels are\n"
         "                  displayed)\n"
         "gfa2:             output matches in gfa2 format\n"
         "custom:           output matches in custom format, i.e. no\n"
         "                  columns are pre-defined; all columns have to be\n"
         "                  specified by the user\n"
         "cigar:            display cigar string representing alignment\n"
         "                  (no distinction between match and mismatch)\n"
         "cigarX:           display cigar string representing alignment\n"
         "                  (distinction between match (=) and mismatch\n"
         "                  (X))\n"
         "trace:            display trace, i.e. a compact representation\n"
         "                  of an alignment (possibly followed by =<delta>)\n"
         "                  to specify the delta-value; default value of\n"
         "                  delta is 50\n"
         "dtrace:           display trace as differences; like trace, but\n"
         "                  instead of an absolute value x, report the\n"
         "                  difference delta-x. This leads to smaller\n"
         "                  numbers and thus a more compact representation\n"
         "s.len:            display length of match on subject sequence\n"
         "s.seqnum:         display sequence number of subject sequence\n"
         "subject id:       display sequence description of subject\n"
         "                  sequence\n"
         "s.start:          display start position of match on subject\n"
         "                  sequence\n"
         "s.end:            display end position of match on subject\n"
         "                  sequence\n"
         "strand:           display strand of match using symbols F\n"
         "                  (forward) and P (reverse complement)\n"
         "q.len:            display length of match on query sequence\n"
         "q.seqnum:         display sequence number of query sequence\n"
         "query id:         display sequence description of query\n"
         "                  sequence\n"
         "q.start:          display start position of match on query\n"
         "                  sequence\n"
         "q.end:            display end position of match on query\n"
         "                  sequence\n"
         "alignment length: display length of alignment\n"
         "mismatches:       display number of mismatches in alignment\n"
         "indels:           display number of indels in alignment\n"
         "gap opens:        display number of indels in alignment\n"
         "score:            display score of match\n"
         "editdist:         display unit edit distance\n"
         "identity:         display percent identity of match\n"
         "seed.len:         display length seed of the match\n"
         "seed.s:           display start position of seed in subject\n"
         "seed.q:           display start position of seed in query\n"
         "s.seqlen:         display length of subject sequence in which\n"
         "                  match occurs\n"
         "q.seqlen:         display length of query sequence in which\n"
         "                  match occurs\n"
         "evalue:           display evalue\n"
         "bit score:        display bit score\n"
;
}
#define GT_SE_POSSIBLE_DISPLAY_ARGS "alignment"\
        ", seed_in_algn"\
        ", polinfo"\
        ", seed"\
        ", failed_seed"\
        ", fstperquery"\
        ", tabsep"\
        ", blast"\
        ", gfa2"\
        ", custom"\
        ", cigar"\
        ", cigarX"\
        ", trace"\
        ", dtrace"\
        ", s.len"\
        ", s.seqnum"\
        ", subject id"\
        ", s.start"\
        ", s.end"\
        ", strand"\
        ", q.len"\
        ", q.seqnum"\
        ", query id"\
        ", q.start"\
        ", q.end"\
        ", alignment length"\
        ", mismatches"\
        ", indels"\
        ", gap opens"\
        ", score"\
        ", editdist"\
        ", identity"\
        ", seed.len"\
        ", seed.s"\
        ", seed.q"\
        ", s.seqlen"\
        ", q.seqlen"\
        ", evalue"\
        ", bit score"

bool gt_querymatch_seed_in_algn_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Seed_in_algn_display);
}

bool gt_querymatch_polinfo_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Polinfo_display);
}

bool gt_querymatch_seed_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Seed_display);
}

bool gt_querymatch_failed_seed_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Failed_seed_display);
}

bool gt_querymatch_fstperquery_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Fstperquery_display);
}

bool gt_querymatch_tabsep_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Tabsep_display);
}

bool gt_querymatch_blast_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Blast_display);
}

bool gt_querymatch_gfa2_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Gfa2_display);
}

bool gt_querymatch_custom_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Custom_display);
}

bool gt_querymatch_cigar_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Cigar_display);
}

bool gt_querymatch_cigarX_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Cigarx_display);
}

bool gt_querymatch_trace_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Trace_display);
}

bool gt_querymatch_dtrace_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Dtrace_display);
}

bool gt_querymatch_s_len_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_S_len_display);
}

bool gt_querymatch_s_seqnum_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_S_seqnum_display);
}

bool gt_querymatch_subjectid_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Subjectid_display);
}

bool gt_querymatch_s_start_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_S_start_display);
}

bool gt_querymatch_s_end_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_S_end_display);
}

bool gt_querymatch_strand_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Strand_display);
}

bool gt_querymatch_q_len_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Q_len_display);
}

bool gt_querymatch_q_seqnum_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Q_seqnum_display);
}

bool gt_querymatch_queryid_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Queryid_display);
}

bool gt_querymatch_q_start_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Q_start_display);
}

bool gt_querymatch_q_end_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Q_end_display);
}

bool gt_querymatch_alignmentlength_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Alignmentlength_display);
}

bool gt_querymatch_mismatches_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Mismatches_display);
}

bool gt_querymatch_indels_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Indels_display);
}

bool gt_querymatch_gapopens_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Gapopens_display);
}

bool gt_querymatch_score_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Score_display);
}

bool gt_querymatch_editdist_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Editdist_display);
}

bool gt_querymatch_identity_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Identity_display);
}

bool gt_querymatch_seed_len_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Seed_len_display);
}

bool gt_querymatch_seed_s_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Seed_s_display);
}

bool gt_querymatch_seed_q_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Seed_q_display);
}

bool gt_querymatch_s_seqlen_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_S_seqlen_display);
}

bool gt_querymatch_q_seqlen_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Q_seqlen_display);
}

bool gt_querymatch_evalue_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Evalue_display);
}

bool gt_querymatch_bitscore_display(const GtSeedExtendDisplayFlag
                                        *display_flag)
{
  return gt_querymatch_display_on(display_flag,Gt_Bitscore_display);
}
