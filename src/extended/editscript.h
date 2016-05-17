/*
  Copyright (c) 2013 Ole Eigenbrod <ole.eigenbrod@gmx.de>
  Copyright (c) 2013 Dirk Willrodt <willrodt@zbh.uni-hamburg.de>
  Copyright (c) 2013 Center for Bioinformatics, University of Hamburg

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

#ifndef EDITSCRIPT_H
#define EDITSCRIPT_H

#include "core/alphabet_api.h"
#include "core/encseq_api.h"
#include "core/error_api.h"
#include "extended/io_function_pointers.h"
#include "extended/multieoplist.h"

/* Class <GtEditscript> stores everything from an alignment of two sequences
   that is needed to reproduce sequence 2 (called v) from sequence one (u). The
   purpose is reduction in space for sequence v.
   As with <GtMultieoplist> the order of edit operations is expected to be
   reversed. */
typedef struct GtEditscript GtEditscript;

/* Class <GtEditscriptBuilder> is used to fill a <GtEditscript> one operation a
   time. We recommend to use the function gt_editscript_new_with_sequences() to
   fill the <GtEditscript> object directly. Operations are added left to right,
   not as in <GtAlignment> or <GtMultieop> classes! */
typedef struct GtEditscriptBuilder GtEditscriptBuilder;

/* Returns new empty <GtEditscript> object. */
GtEditscript* gt_editscript_new(GtAlphabet *alphabet);

/* print out a string representing the editscript */
void          gt_editscript_show(const GtEditscript *editscript,
                                 GtAlphabet *alphabet);
/* Delete content of <editscript> */
void          gt_editscript_reset(GtEditscript *editscript);

/* Returns new <GtEditscript> object and fills it with <encseq> as "second" or v
   sequence and <multieops> representing the edit operations when aligned to a
   sequence u. <start> and <dir> define where sequence v starts within
   <encseq>. Expects <multieops> to be in reverse (right to left) order. */
GtEditscript* gt_editscript_new_with_sequences(const GtEncseq *encseq,
                                               GtMultieoplist *multieops,
                                               GtUword start,
                                               GtReadmode dir);
/* Sets <match> <mismatch> <insertion> and <deletion> to the number of
   occurrences of those edit operations within the <editscript> */
void          gt_editscript_get_stats(const GtEditscript *editscript,
                                      GtUword *match,
                                      GtUword *mismatch,
                                      GtUword *insertion,
                                      GtUword *deletion);
/* Returns the length of the target sequence v. */
GtUword       gt_editscript_get_target_len(const GtEditscript *editscript);

/* Returns the length of the source sequence u. */
GtUword       gt_editscript_get_source_len(const GtEditscript *editscript);

/* Returns the length of the target subsequence of v that corresponds to u
   starting at <srcfrom> with length <srclen>. */
GtUword       gt_editscript_get_target_subseq_len(
                                                 const GtEditscript *editscript,
                                                 GtUword srcfrom,
                                                 GtUword srclen);
/* Transform sequence u defined by <start> and <dir> in <encseq> using the edit
   operations in <editscript> and stores the result in <buffer> of size
   <bufsize>. As the length of the corresponding v can not be known in an
   alignment, <buffer> will be resized and <bufsize> set accordingly. If
   <buffer> is NULL, space will be allocated and <bufsize> set.
   Returns the number of <GtUchar>s written to buffer. */
GtUword       gt_editscript_get_sequence(const GtEditscript *editscript,
                                         const GtEncseq *encseq,
                                         GtUword start,
                                         GtReadmode dir,
                                         GtUchar **buffer,
                                         GtUword *bufsize);
/* Transform sequence u defined by <start> and <dir> in <encseq> using the edit
   operations in <editscript> and stores the result in <buffer> which has to be
   large enough to hold v (the resulting sequence). Extracts only positions
   <vfrompos> to <vtopos> (inclusive) relative to v.
   Returns the number of <GtUchar>s written to buffer. */
GtUword       gt_editscript_get_sub_sequence_v(const GtEditscript *editscript,
                                               const GtEncseq *encseq,
                                               GtUword start,
                                               GtReadmode dir,
                                               GtUword vfrompos,
                                               GtUword vtopos,
                                               GtUchar *buffer);
/* Transform sequence u defined by <start> and <dir> in <encseq> using the edit
   operations in <editscript> and stores the result in <buffer> of size
   <bufsize>. As the length of the corresponding v can not be known in an
   alignment, <buffer> will be resized and <bufsize> set accordingly. If
   <buffer> is NULL, space will be allocated and <bufsize> set.
   Extracts only positions <ufrompos> to <utopos> (inclusive) relative to start
   of u.
   Returns the number of <GtUchar>s written to buffer. */
GtUword       gt_editscript_get_sub_sequence_u(const GtEditscript *editscript,
                                               const GtEncseq *encseq,
                                               GtUword start,
                                               GtReadmode dir,
                                               GtUword ufrompos,
                                               GtUword utopos,
                                               GtUchar **buffer,
                                               GtUword *bufsize);
/* Read or write to/from File, depending on <editscript>. If <NULL>, it
   allocates memory for a new <GtEditscript> object and tries to fill it from
   file <fp>. If not <NULL> it writs the content of <editscript> to <fp>.
   Returns <NULL> on error, in which case <editscript> will be deleted and <err>
   will be set. */
GtEditscript* gt_editscript_io(GtEditscript *editscript, FILE *fp,
                                 GtError *err);
/* Returns the size of the <editscript> in bytes in memory. */
size_t        gt_editscript_size(const GtEditscript *editscript);

/* Free all memory associated with <editscript> */
void          gt_editscript_delete(GtEditscript *editscript);

/* Returns new <GtEditscriptBuilder> object */
GtEditscriptBuilder* gt_editscript_builder_new(GtEditscript *editscript);

/* Resets the <GtEditscriptBuilder> object with a new <GtEditscript>. The
   <GtEditscript> will be reset, too. */
void                 gt_editscript_builder_reset(
                                                GtEditscriptBuilder *es_builder,
                                                GtEditscript *editscript);
/* Add one deletion to <editscript>. */
void                 gt_editscript_builder_add_deletion(
                                               GtEditscriptBuilder *es_builder);
/* Add one insertion to <editscript>. <c> is the encoded character in v that
   was inserted. */
void                 gt_editscript_builder_add_insertion(
                                                GtEditscriptBuilder *es_builder,
                                                GtUchar c);
/* Add one match to <editscript>. */
void                 gt_editscript_builder_add_match(
                                               GtEditscriptBuilder *es_builder);
/* Add one mismatch to <editscript>. <c> is the encoded character in v that
   replaces one character in u. */
void                 gt_editscript_builder_add_mismatch(
                                                GtEditscriptBuilder *es_builder,
                                                GtUchar c);
int gt_editscript_unit_test(GtError *err);
#endif
