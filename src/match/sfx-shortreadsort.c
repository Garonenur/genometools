/*
  Copyright (c) 2011 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
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

#include <stdint.h>
#include "core/intbits.h"
#include "core/stack-inlined.h"
#include "core/minmax.h"
#include "core/assert_api.h"
#include "sfx-lcpvalues.h"
#include "sfx-shortreadsort.h"
#include "spmsuftab.h"
#include "seqnumrelpos.h"

typedef struct
{
  unsigned long suffixrepresentation;
  uint32_t tbeidx;
  unsigned int unitsnotspecial;
} GtShortreadsort;

struct GtShortreadsortworkinfo
{
  GtShortreadsort *shortreadsorttable;
  GtLcpvalues *sssplcpvalues;    /* always NULL in the context of
                                    firstcodes; otherwise: NULL iff
                                    lcpvalues are not required. */
  uint16_t *firstcodeslcpvalues; /* always NULL for firstcodes;
                                    otherwise: NULL if lcpvalues are
                                    not required */
  unsigned long *seqnum_relpos_bucket; /* only for firstcodes */
  GtArrayGtTwobitencoding tbereservoir;
  unsigned long tmplcplen, currentbucketsize, sumofstoredvalues;
  bool fwd, complement;
};

static unsigned long gt_shortreadsort_encoding_size(unsigned long bucketsize,
                                                    unsigned long maxremain)
{
  double factor = 1.0;

  if (maxremain > (2UL * (unsigned long) GT_UNITSIN2BITENC))
  {
    factor += maxremain/(2UL * (unsigned long) GT_UNITSIN2BITENC);
  } else
  {
    factor = 1.0;
  }
  return (unsigned long) (bucketsize * factor);
}

size_t gt_shortreadsort_size(bool firstcodes,unsigned long bucketsize,
                             unsigned long maxremain)
{
  size_t sizeforlcpvalues = firstcodes ? sizeof (uint16_t) * bucketsize : 0;
  return sizeforlcpvalues +
         (sizeof (GtShortreadsort) + sizeof (unsigned long)) * bucketsize +
         sizeof (GtTwobitencoding) *
         gt_shortreadsort_encoding_size(bucketsize,maxremain);
}

static void gt_shortreadsort_resize(GtShortreadsortworkinfo *srsw,
                                    bool firstcodes,
                                    unsigned long bucketsize,
                                    unsigned long maxremain)
{
  gt_assert(bucketsize <= (unsigned long) UINT32_MAX);
  if (srsw->currentbucketsize < bucketsize)
  {
    srsw->shortreadsorttable = gt_realloc(srsw->shortreadsorttable,
                                          sizeof (*srsw->shortreadsorttable) *
                                          bucketsize);
  }
  if (firstcodes)
  {
    if (srsw->currentbucketsize < bucketsize)
    {
      srsw->firstcodeslcpvalues
        = gt_realloc(srsw->firstcodeslcpvalues,
                     sizeof (*srsw->firstcodeslcpvalues) * bucketsize);
      srsw->firstcodeslcpvalues[0] = 0; /* since it is not set otherwise */
      srsw->seqnum_relpos_bucket
        = gt_realloc(srsw->seqnum_relpos_bucket,
                     sizeof (*srsw->seqnum_relpos_bucket) * bucketsize);
    }
  } else
  {
    srsw->firstcodeslcpvalues = NULL;
  }
  srsw->tbereservoir.nextfreeGtTwobitencoding = 0;
  if (srsw->currentbucketsize < bucketsize)
  {
    srsw->tbereservoir.allocatedGtTwobitencoding
        = gt_shortreadsort_encoding_size(bucketsize,maxremain);
    srsw->tbereservoir.spaceGtTwobitencoding
      = gt_realloc(srsw->tbereservoir.spaceGtTwobitencoding,
                   sizeof (GtTwobitencoding) *
                   srsw->tbereservoir.allocatedGtTwobitencoding);
    srsw->currentbucketsize = bucketsize;
  }
}

GtShortreadsortworkinfo *gt_shortreadsort_new(unsigned long maxwidth,
                                              unsigned long maxremain,
                                              GtReadmode readmode,
                                              bool firstcodes)
{
  GtShortreadsortworkinfo *srsw;

  srsw = gt_malloc(sizeof (*srsw));
  srsw->fwd = GT_ISDIRREVERSE(readmode) ? false : true;
  srsw->complement = GT_ISDIRCOMPLEMENT(readmode) ? true : false;
  srsw->sssplcpvalues = NULL;
  srsw->sumofstoredvalues = 0;
  srsw->currentbucketsize = 0;
  srsw->shortreadsorttable = NULL;
  srsw->firstcodeslcpvalues = NULL;
  srsw->seqnum_relpos_bucket = NULL;
  GT_INITARRAY(&srsw->tbereservoir,GtTwobitencoding);
  if (maxwidth > 0)
  {
    gt_shortreadsort_resize(srsw,firstcodes,maxwidth,maxremain);
  }
  return srsw;
}

unsigned long gt_shortreadsort_sumofstoredvalues(const GtShortreadsortworkinfo
                                                       *srsw)
{
  return srsw->sumofstoredvalues;
}

void gt_shortreadsort_delete(GtShortreadsortworkinfo *srsw)
{
  if (srsw != NULL)
  {
    gt_free(srsw->shortreadsorttable);
    srsw->shortreadsorttable = NULL;
    gt_free(srsw->firstcodeslcpvalues);
    srsw->firstcodeslcpvalues = NULL;
    gt_free(srsw->seqnum_relpos_bucket);
    srsw->seqnum_relpos_bucket = NULL;
    GT_FREEARRAY(&srsw->tbereservoir,GtTwobitencoding);
    gt_free(srsw);
  }
}

void gt_shortreadsort_assigntableoflcpvalues(GtShortreadsortworkinfo *srsw,
                                             GtLcpvalues *tableoflcpvalues)
{
  if (srsw != NULL)
  {
    srsw->sssplcpvalues = tableoflcpvalues;
  }
}

static int gt_shortreadsort_compare(const GtShortreadsort *aq,
                                    const GtShortreadsort *bq,
                                    GtShortreadsortworkinfo *srsw)
{
  int idx, retval;
  unsigned int maxprefix;
  GtCommonunits commonunits;

  for (idx=0, maxprefix = (unsigned int) GT_UNITSIN2BITENC;
       /* Nothing */;
       idx++, maxprefix+=(unsigned int) GT_UNITSIN2BITENC)
  {
    if (aq->unitsnotspecial >= maxprefix &&
        bq->unitsnotspecial >= maxprefix)
    {
      if (srsw->tbereservoir.spaceGtTwobitencoding[aq->tbeidx + idx] !=
          srsw->tbereservoir.spaceGtTwobitencoding[bq->tbeidx + idx])
      {
        retval = gt_encseq_compare_pairof_different_twobitencodings(
                              srsw->fwd,
                              srsw->complement,
                              &commonunits,
                              srsw->tbereservoir.spaceGtTwobitencoding[
                                                      aq->tbeidx + idx],
                              srsw->tbereservoir.spaceGtTwobitencoding[
                                                      bq->tbeidx + idx]);
        srsw->tmplcplen
          = (unsigned long) (maxprefix - GT_UNITSIN2BITENC +
                             commonunits.common);
        return retval;
      }
    } else
    {
      GtEndofTwobitencoding tbe_a, tbe_b;

      tbe_a.referstartpos = aq->suffixrepresentation;
      tbe_b.referstartpos = bq->suffixrepresentation;
      tbe_a.unitsnotspecial
        = aq->unitsnotspecial >= maxprefix
           ? maxprefix
           : aq->unitsnotspecial + GT_UNITSIN2BITENC - maxprefix;
      tbe_a.tbe = (tbe_a.unitsnotspecial > 0)
        ? srsw->tbereservoir.spaceGtTwobitencoding[aq->tbeidx + idx]
        : 0;
      tbe_b.unitsnotspecial
        = bq->unitsnotspecial >= maxprefix
           ? maxprefix
           : bq->unitsnotspecial + GT_UNITSIN2BITENC - maxprefix;
      tbe_b.tbe = (tbe_b.unitsnotspecial > 0)
        ? srsw->tbereservoir.spaceGtTwobitencoding[bq->tbeidx + idx]
        : 0;
      retval = gt_encseq_compare_pairof_twobitencodings(srsw->fwd,
                                                        srsw->complement,
                                                        &commonunits,
                                                        &tbe_a,
                                                        &tbe_b);
      srsw->tmplcplen = (unsigned long) (maxprefix - GT_UNITSIN2BITENC +
                                         commonunits.common);
      return retval;
    }
  }
  /*@ignore@*/
  return 0;
  /*@end@*/
}

#ifdef QSORTNAME
#undef QSORTNAME
#endif

#define QSORTNAME(NAME)                    shortread_##NAME
#define shortread_ARRAY_GET(ARR,IDX)       data->shortreadsorttable[IDX]
#define shortread_ARRAY_SET(ARR,IDX,VALUE) data->shortreadsorttable[IDX] = VALUE

typedef GtShortreadsortworkinfo * QSORTNAME(Datatype);

static int QSORTNAME(qsortcmparr) (unsigned long a,
                                   unsigned long b,
                                   const QSORTNAME(Datatype) data)
{
  return gt_shortreadsort_compare(&QSORTNAME(ARRAY_GET)(NULL,a),
                                  &QSORTNAME(ARRAY_GET)(NULL,b),
                                  data);
}

typedef unsigned long QSORTNAME(Sorttype);

/*
 * Qsort routine from Bentley & McIlroy's ``Engineering a Sort Function''.
 */

#ifndef GT_QSORT_ARR_SWAP
#define GT_QSORT_ARR_SWAP(ARR,A,B)\
        if ((A) != (B))\
        {\
          tmp = QSORTNAME(ARRAY_GET)(ARR,A);\
          QSORTNAME(ARRAY_SET)(ARR,A,QSORTNAME(ARRAY_GET)(ARR,B));\
          QSORTNAME(ARRAY_SET)(ARR,B,tmp);\
        }
#endif

#ifndef GT_QSORT_ARR_VECSWAP
#define GT_QSORT_ARR_VECSWAP(ARR,A,B,N)\
        aidx = A;\
        bidx = B;\
        while ((N)-- > 0)\
        {\
          tmp = QSORTNAME(ARRAY_GET)(ARR,aidx);\
          QSORTNAME(ARRAY_SET)(ARR,aidx,QSORTNAME(ARRAY_GET)(ARR,bidx));\
          QSORTNAME(ARRAY_SET)(ARR,bidx,tmp);\
          aidx++;\
          bidx++;\
        }
#endif

static inline unsigned long QSORTNAME(gt_inlined_qsort_arr_r_med3)
                     (unsigned long a, unsigned long b, unsigned long c,
                      QSORTNAME(Datatype) data)
{
  return QSORTNAME(qsortcmparr) (a, b, data) < 0
           ? (QSORTNAME(qsortcmparr) (b, c, data) < 0
                ? b
                : (QSORTNAME(qsortcmparr) (a, c, data) < 0
                     ? c : a))
           : (QSORTNAME(qsortcmparr) (b, c, data) > 0
                ? b
                : (QSORTNAME(qsortcmparr) (a, c, data) < 0
                     ? a
                     : c));
}

#ifndef GT_STACK_INTERVALARRAYTOBESORTED_DEFINED
typedef struct
{
  unsigned long startindex,
                len;
} Intervalarrtobesorted;

GT_STACK_DECLARESTRUCT(Intervalarrtobesorted,32UL);
#define GT_STACK_INTERVALARRAYTOBESORTED_DEFINED
#endif

static void QSORTNAME(gt_inlinedarr_qsort_r) (
                                   unsigned long insertionsortthreshold,
                                   bool handlenotswapped,
                                   unsigned long len,
                                   QSORTNAME(Datatype) data,
                                   unsigned long depth,
                                   unsigned long subbucketleft)
{
  unsigned long pa, pb, pc, pd, pl, pm, pn, aidx, bidx, s,
                smallermaxlcp, greatermaxlcp;
  GtShortreadsort tmp;
  int r;
  bool swapped;
  GtStackIntervalarrtobesorted intervalstack;
  Intervalarrtobesorted current;

  GT_STACK_INIT(&intervalstack,32UL);
  current.startindex = 0;
  current.len = len;
  GT_STACK_PUSH(&intervalstack,current);
  if (insertionsortthreshold <= 2UL)
  {
    insertionsortthreshold = 6UL;
  }
  while (!GT_STACK_ISEMPTY(&intervalstack))
  {
    swapped = false;
    current = GT_STACK_POP(&intervalstack);
    if (current.len <= insertionsortthreshold)
    {
      for (pm = current.startindex + 1;
           pm < current.startindex + current.len; pm++)
      {
        for (pl = pm; pl > current.startindex; pl--)
        {
          r = QSORTNAME(qsortcmparr) (pl - 1, pl, data);
          if (data->sssplcpvalues != NULL)
          {
            unsigned long lcpindex = subbucketleft + pl;
            if (pl < pm && r > 0)
            {
              lcptab_update(data->sssplcpvalues,lcpindex+1,
                            lcpsubtab_getvalue(data->sssplcpvalues,
                                               lcpindex));
            }
            lcptab_update(data->sssplcpvalues,lcpindex,
                          depth + data->tmplcplen);
          } else
          {
            if (data->firstcodeslcpvalues != NULL)
            {
              if (pl < pm && r > 0)
              {
                data->firstcodeslcpvalues[pl+1] = data->firstcodeslcpvalues[pl];
              }
              gt_assert(depth + data->tmplcplen <= UINT16_MAX);
              data->firstcodeslcpvalues[pl]
                = (uint16_t) (depth + data->tmplcplen);
            }
          }
          if (r <= 0)
          {
            break;
          }
          GT_QSORT_ARR_SWAP (arr, pl, pl - 1);
        }
      }
      continue;
    }
    pm = current.startindex + GT_DIV2 (current.len);
    if (current.len > 7UL)
    {
      pl = current.startindex;
      pn = current.startindex + current.len - 1;
      if (current.len > 40UL)
      {
        s = GT_DIV8 (current.len);
        pl = QSORTNAME(gt_inlined_qsort_arr_r_med3) (pl, pl + s,
                                                     pl + GT_MULT2 (s), data);
        gt_assert(pm >= s);
        pm = QSORTNAME(gt_inlined_qsort_arr_r_med3) (pm - s, pm,
                                                     pm + s, data);
        gt_assert(pn >= GT_MULT2(s));
        pn = QSORTNAME(gt_inlined_qsort_arr_r_med3) (pn - GT_MULT2 (s),
                                                     pn - s, pn, data);
      }
      pm = QSORTNAME(gt_inlined_qsort_arr_r_med3) (pl, pm, pn, data);
    }
    GT_QSORT_ARR_SWAP (arr, current.startindex, pm);
    pa = pb = current.startindex + 1;
    pc = pd = current.startindex + current.len - 1;
    smallermaxlcp = greatermaxlcp = 0;
    while (1)
    {
      while (pb <= pc)
      {
        r = QSORTNAME(qsortcmparr) (pb, current.startindex, data);
        if (r > 0)
        {
          GT_BSR_UPDATEMAXLCP(greatermaxlcp,data->tmplcplen);
          break;
        }
        if (r == 0)
        {
          swapped = true;
          GT_QSORT_ARR_SWAP (arr, pa, pb);
          pa++;
        } else
        {
          GT_BSR_UPDATEMAXLCP(smallermaxlcp,data->tmplcplen);
        }
        pb++;
      }
      while (pb <= pc)
      {
        r = QSORTNAME(qsortcmparr) (pc, current.startindex, data);
        if (r < 0)
        {
          GT_BSR_UPDATEMAXLCP(smallermaxlcp,data->tmplcplen);
          break;
        }
        if (r == 0)
        {
          swapped = true;
          GT_QSORT_ARR_SWAP (arr, pc, pd);
          gt_assert(pd > 0);
          pd--;
        } else
        {
          GT_BSR_UPDATEMAXLCP(greatermaxlcp,data->tmplcplen);
        }
        gt_assert(pc > 0);
        pc--;
      }
      if (pb > pc)
      {
        break;
      }
      GT_QSORT_ARR_SWAP (arr, pb, pc);
      swapped = true;
      pb++;
      gt_assert(pc > 0);
      pc--;
    }
    /* The following switch is not explained in the above mentioned
       paper and therefore we ignore it. */
    if (handlenotswapped && !swapped)
    {                                  /* Switch to insertion sort */
      gt_assert(current.len <= 40UL);
      for (pm = current.startindex + 1;
           pm < current.startindex + current.len; pm++)
      {
        for (pl = pm; pl > current.startindex; pl--)
        {
          r = QSORTNAME(qsortcmparr) (pl - 1, pl, data);
          if (r <= 0)
          {
            break;
          }
          GT_QSORT_ARR_SWAP (arr, pl, pl - 1);
        }
      }
      continue;
    }
    pn = current.startindex + current.len;
    gt_assert(pa >= current.startindex);
    gt_assert(pb >= pa);
    s = MIN ((unsigned long) (pa - current.startindex),
             (unsigned long) (pb - pa));
    gt_assert(pb >= s);
    GT_QSORT_ARR_VECSWAP (arr, current.startindex, pb - s, s);
    gt_assert(pd >= pc);
    gt_assert(pn > pd);
    s = MIN ((unsigned long) (pd - pc), (unsigned long) (pn - pd - 1));
    gt_assert(pn > s);
    GT_QSORT_ARR_VECSWAP (arr, pb, pn - s, s);
    gt_assert(pb >= pa);
    if ((s = (unsigned long) (pb - pa)) > 0)
    {
      if (data->sssplcpvalues != NULL)
      {
        /*
          left part has suffix with lcp up to length smallermaxlcp w.r.t.
          to the pivot. This lcp belongs to a suffix on the left
          which is at a minimum distance to the pivot and thus to an
          element in the final part of the left side.
        */
        lcptab_update(data->sssplcpvalues,subbucketleft+current.startindex+s,
                      depth+smallermaxlcp);
      } else
      {
        if (data->firstcodeslcpvalues != NULL)
        {
          gt_assert(depth + smallermaxlcp <= UINT16_MAX);
          data->firstcodeslcpvalues[current.startindex+s]
            = (uint16_t) (depth + smallermaxlcp);
        }
      }
      if (s > 1UL)
      {
        current.len = s;
        GT_STACK_PUSH(&intervalstack,current);
      }
    }
    gt_assert(pd >= pc);
    if ((s = (unsigned long) (pd - pc)) > 0)
    {
      if (data->sssplcpvalues != NULL)
      {
        /*
          right part has suffix with lcp up to length largermaxlcp w.r.t.
          to the pivot. This lcp belongs to a suffix on the right
          which is at a minimum distance to the pivot and thus to an
          element in the first part of the right side.
        */
        gt_assert(pn >= s);
        lcptab_update(data->sssplcpvalues, subbucketleft + pn - s,
                      depth + greatermaxlcp);
      } else
      {
        if (data->firstcodeslcpvalues != NULL)
        {
          gt_assert(depth + greatermaxlcp <= UINT16_MAX);
          data->firstcodeslcpvalues[pn - s]
            = (uint16_t) (depth + greatermaxlcp);
        }
      }
      if (s > 1UL)
      {
        gt_assert(pn >= s);
        current.startindex = pn - s;
        current.len = s;
        GT_STACK_PUSH(&intervalstack,current);
      }
    }
  }
  GT_STACK_DELETE(&intervalstack);
}

void gt_shortreadsort_sssp_sort(GtShortreadsortworkinfo *srsw,
                                const GtEncseq *encseq,
                                unsigned long maxremain,
                                GtReadmode readmode,
                                GtEncseqReader *esr,
                                GtSuffixsortspace *sssp,
                                unsigned long subbucketleft,
                                unsigned long width,
                                unsigned long depth)
{
  unsigned long idx, pos;
  GtSuffixsortspace_exportptr *exportptr;

  gt_shortreadsort_resize(srsw, false, width, maxremain);
  exportptr = gt_suffixsortspace_exportptr(subbucketleft, sssp);
  srsw->tbereservoir.nextfreeGtTwobitencoding = 0;
  if (exportptr->ulongtabsectionptr != NULL)
  {
    for (idx = 0; idx < width; idx++)
    {
      pos = exportptr->ulongtabsectionptr[idx];
      srsw->shortreadsorttable[idx].suffixrepresentation = pos;
      srsw->shortreadsorttable[idx].tbeidx
        = (uint32_t) srsw->tbereservoir.nextfreeGtTwobitencoding;
      srsw->shortreadsorttable[idx].unitsnotspecial
        = gt_encseq_extract2bitencvector(&srsw->tbereservoir,
                                         encseq,
                                         esr,
                                         readmode,
                                         pos+depth);
    }
  } else
  {
    for (idx = 0; idx < width; idx++)
    {
      pos = (unsigned long) exportptr->uinttabsectionptr[idx];
      srsw->shortreadsorttable[idx].suffixrepresentation = pos;
      srsw->shortreadsorttable[idx].tbeidx
        = (uint32_t) srsw->tbereservoir.nextfreeGtTwobitencoding;
      srsw->shortreadsorttable[idx].unitsnotspecial
        = gt_encseq_extract2bitencvector(&srsw->tbereservoir,
                                         encseq,
                                         esr,
                                         readmode,
                                         pos+depth);
    }
  }
  QSORTNAME(gt_inlinedarr_qsort_r) (6UL, false, width, srsw, depth,
                                    subbucketleft);
  if (exportptr->ulongtabsectionptr != NULL)
  {
    for (idx = 0; idx < width; idx++)
    {
      exportptr->ulongtabsectionptr[idx]
        = srsw->shortreadsorttable[idx].suffixrepresentation;
      if (exportptr->ulongtabsectionptr[idx] == 0)
      {
        gt_suffixsortspace_updatelongest(sssp,subbucketleft + idx);
      }
    }
  } else
  {
    for (idx = 0; idx < width; idx++)
    {
      exportptr->uinttabsectionptr[idx]
        = (uint32_t) srsw->shortreadsorttable[idx].suffixrepresentation;
      if (exportptr->uinttabsectionptr[idx] == 0)
      {
        gt_suffixsortspace_updatelongest(sssp,subbucketleft + idx);
      }
    }
  }
  gt_suffixsortspace_export_done(sssp);
}

void gt_shortreadsort_firstcodes_sort(GtShortreadsortresult *srsresult,
                                      GtShortreadsortworkinfo *srsw,
                                      const GtSeqnumrelpos *snrp,
                                      const GtEncseq *encseq,
                                      const GtSpmsuftab *spmsuftab,
                                      unsigned long subbucketleft,
                                      unsigned long width,
                                      unsigned long depth)
{
  unsigned long idx, pos, seqnum, relpos, seqnum_relpos;

  srsw->tbereservoir.nextfreeGtTwobitencoding = 0;
  for (idx = 0; idx < width; idx++)
  {
    if (gt_spmsuftab_usebitsforpositions(spmsuftab))
    {
      pos = gt_spmsuftab_get(spmsuftab,subbucketleft + idx);
      seqnum = gt_encseq_seqnum(encseq,pos);
      relpos = pos - gt_encseq_seqstartpos(encseq,seqnum);
      srsw->shortreadsorttable[idx].suffixrepresentation
        = gt_seqnumrelpos_encode(snrp, seqnum, relpos);
    } else
    {
      seqnum_relpos = gt_spmsuftab_get(spmsuftab,subbucketleft + idx);
      seqnum = gt_seqnumrelpos_decode_seqnum(snrp,seqnum_relpos);
      relpos = gt_seqnumrelpos_decode_relpos(snrp,seqnum_relpos);
      srsw->shortreadsorttable[idx].suffixrepresentation = seqnum_relpos;
    }
    srsw->shortreadsorttable[idx].tbeidx
      = (uint32_t) srsw->tbereservoir.nextfreeGtTwobitencoding;
    srsw->shortreadsorttable[idx].unitsnotspecial
      = gt_encseq_relpos_extract2bitencvector(&srsw->tbereservoir,
                                              encseq,
                                              seqnum,
                                              relpos + depth);
  }
  srsw->sumofstoredvalues += srsw->tbereservoir.nextfreeGtTwobitencoding;
  QSORTNAME(gt_inlinedarr_qsort_r) (6UL, false, width, srsw, depth,
                                    subbucketleft);
  for (idx = 0; idx < width; idx++)
  {
    srsw->seqnum_relpos_bucket[idx]
      = srsw->shortreadsorttable[idx].suffixrepresentation;
  }
  srsresult->suftab_bucket = srsw->seqnum_relpos_bucket;
  srsresult->lcptab_bucket = srsw->firstcodeslcpvalues;
}
