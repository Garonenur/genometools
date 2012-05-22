/*
  Copyright (c) 2011 Giorgio Gonnella <gonnella@zbh.uni-hamburg.de>
  Copyright (c) 2012 Stefan Kurtz <gonnella@zbh.uni-hamburg.de>
  Copyright (c) 2011-2012 Center for Bioinformatics, University of Hamburg

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

#include <limits.h>
#include <string.h>
#include "core/intbits.h"
#include "core/log.h"
#include "core/ma.h"
#include "core/minmax.h"
#include "core/stack-inlined.h"
#include "sfx-lcpvalues.h"
#include "radixsort_str.h"

typedef uint8_t gt_radixsort_str_kmercode_t;
/* note: if kmercode_t size is changed, then the RADIXSORT_STR_REVCOMPL
 * macro must be changed to handle the new size too */

typedef uint16_t gt_radixsort_str_bucketnum_t;

#define GT_RADIXSORT_STR_UINT8_REVCOMPL(CODE)\
  (GT_RADIXSORT_STR_KMERCODE_MAX ^\
   (((((CODE) & (3 << 6)) >> 6) | (((CODE) & 3) << 6)) |\
    ((((CODE) & (3 << 4)) >> 2) | (((CODE) & (3 << 2)) << 2))))

#define GT_RADIXSORT_STR_REVCOMPL(CODE) GT_RADIXSORT_STR_UINT8_REVCOMPL(CODE)

#define GT_RADIXSORT_STR_KMERCODE_BITS\
        (sizeof (gt_radixsort_str_kmercode_t) * CHAR_BIT)
#define GT_RADIXSORT_STR_KMERSIZE         (GT_RADIXSORT_STR_KMERCODE_BITS >> 1)
#define GT_RADIXSORT_STR_KMERSIZELOG      2
#define GT_RADIXSORT_STR_NOFKMERCODES     (1 << GT_RADIXSORT_STR_KMERCODE_BITS)
#define GT_RADIXSORT_STR_KMERCODE_MAX\
        ((gt_radixsort_str_kmercode_t)(GT_RADIXSORT_STR_NOFKMERCODES - 1))

#define GT_RADIXSORT_STR_OVERFLOW_MASK ((1 << GT_RADIXSORT_STR_KMERSIZELOG) - 1)

#define GT_RADIXSORT_STR_NOFBUCKETS\
        (size_t)(((1 << (GT_RADIXSORT_STR_KMERSIZE << 1)) *\
                  GT_RADIXSORT_STR_KMERSIZE) + 1)

#define GT_RADIXSORT_STR_SPECIAL_BUCKET\
        ((gt_radixsort_str_bucketnum_t)(GT_RADIXSORT_STR_NOFBUCKETS - 1))

#define GT_RADIXSORT_STR_SPECIAL_BUCKET_BIT\
        (1 << (GT_RADIXSORT_STR_KMERCODE_BITS + GT_RADIXSORT_STR_KMERSIZELOG))

#define GT_RADIXSORT_STR_HAS_OVERFLOW(BUCKETNUM)\
        (((BUCKETNUM) & GT_RADIXSORT_STR_OVERFLOW_MASK) ||\
         ((BUCKETNUM) == GT_RADIXSORT_STR_SPECIAL_BUCKET))

#define GT_RADIXSORT_STR_BUCKETNUM(CODE, OVERFLOW)\
        (((gt_radixsort_str_bucketnum_t)\
            (CODE) << GT_RADIXSORT_STR_KMERSIZELOG) + (OVERFLOW))

#define GT_RADIXSORT_STR_OVERFLOW_NONSPECIAL(BUCKETNUM)\
        ((BUCKETNUM) & GT_RADIXSORT_STR_OVERFLOW_MASK)

#define GT_RADIXSORT_STR_OVERFLOW(BUCKETNUM)\
        (((BUCKETNUM) == GT_RADIXSORT_STR_SPECIAL_BUCKET)\
          ? GT_RADIXSORT_STR_KMERSIZE\
          : GT_RADIXSORT_STR_OVERFLOW_NONSPECIAL(BUCKETNUM))

typedef struct {
  unsigned long *suffixes;
  unsigned long width;
  unsigned long depth;
  unsigned long lcp;
} GtRadixsortStrBucketInfo;

struct GtRadixsortstringinfo
{
  const GtTwobitencoding *twobitencoding;
  size_t bytesinsizesofbuckets;
  unsigned long equallengthplus1,
                realtotallength,
                maxwidth,
                *sizesofbuckets,
                *sorted;
  gt_radixsort_str_bucketnum_t *oracle;
};

GtRadixsortstringinfo *gt_radixsort_str_new(const GtTwobitencoding
                                             *twobitencoding,
                                            unsigned long realtotallength,
                                            unsigned long equallengthplus1,
                                            unsigned long maxwidth)
{
  GtRadixsortstringinfo *rsi = gt_malloc(sizeof(*rsi));

  rsi->twobitencoding = twobitencoding;
  rsi->equallengthplus1 = equallengthplus1;
  rsi->realtotallength = realtotallength;
  rsi->maxwidth = maxwidth;
  rsi->bytesinsizesofbuckets = sizeof (*rsi->sizesofbuckets) *
                               GT_RADIXSORT_STR_NOFBUCKETS;
  rsi->sizesofbuckets = gt_malloc(rsi->bytesinsizesofbuckets);
  rsi->sorted = gt_malloc(sizeof (*rsi->sorted) * maxwidth);
  rsi->oracle = gt_malloc(sizeof (*rsi->oracle) * maxwidth);
  return rsi;
}

void gt_radixsort_str_delete(GtRadixsortstringinfo *rsi)
{
  if (rsi != NULL)
  {
    gt_free(rsi->sizesofbuckets);
    gt_free(rsi->sorted);
    gt_free(rsi->oracle);
    gt_free(rsi);
  }
}

static inline gt_radixsort_str_kmercode_t gt_radixsort_str_code_at_position(
    const GtTwobitencoding *twobitencoding, unsigned long pos)
{
  unsigned int unitoffset = (unsigned int) GT_MODBYUNITSIN2BITENC(pos);
  unsigned long unitindex = GT_DIVBYUNITSIN2BITENC(pos);

  if (unitoffset <=
      (unsigned int) (GT_UNITSIN2BITENC - GT_RADIXSORT_STR_KMERSIZE))
  {
    return (gt_radixsort_str_kmercode_t)
      (twobitencoding[unitindex] >>
       GT_MULT2(GT_UNITSIN2BITENC - GT_RADIXSORT_STR_KMERSIZE - unitoffset))
       & GT_RADIXSORT_STR_KMERCODE_MAX;
  } else
  {
    unsigned int shiftleft =
      GT_MULT2(unitoffset + (unsigned int) GT_RADIXSORT_STR_KMERSIZE -
               GT_UNITSIN2BITENC);
    return (gt_radixsort_str_kmercode_t)
                  ((twobitencoding[unitindex] << shiftleft) |
                   (twobitencoding[unitindex + 1] >>
                       (GT_MULT2(GT_UNITSIN2BITENC) - shiftleft)))
                  & GT_RADIXSORT_STR_KMERCODE_MAX;
  }
}

static inline gt_radixsort_str_bucketnum_t gt_radixsort_str_get_code(
    const GtTwobitencoding *twobitencoding, unsigned long suffixnum,
    unsigned long depth, unsigned long equallengthplus1,
    unsigned long realtotallength)
{
  unsigned long relpos = suffixnum % equallengthplus1 + depth;

  if (relpos >= equallengthplus1 - 1) /* suffix starts on separator position */
  {
    return GT_RADIXSORT_STR_SPECIAL_BUCKET;
  } else
  {
    uint8_t overflow = 0;
    gt_radixsort_str_kmercode_t code;
    unsigned long remaining, pos = suffixnum + depth;

    remaining = equallengthplus1 - 1 - relpos;
    if (suffixnum <= realtotallength)
    {
      code = gt_radixsort_str_code_at_position(twobitencoding, pos);
      if (remaining < (unsigned long) GT_RADIXSORT_STR_KMERSIZE)
      {
        overflow = GT_RADIXSORT_STR_KMERSIZE - remaining;
        code |= (1 << GT_MULT2(overflow)) - 1;
      }
    } else
    {
      pos = GT_MULT2(realtotallength + 1) - pos - 1;
      pos -= (remaining > (unsigned long) GT_RADIXSORT_STR_KMERSIZE)
               ? (unsigned long) GT_RADIXSORT_STR_KMERSIZE
               : remaining;
      gt_assert(pos < realtotallength);
      code = GT_RADIXSORT_STR_REVCOMPL(gt_radixsort_str_code_at_position(
                                       twobitencoding, pos));
      if (remaining < (unsigned long) GT_RADIXSORT_STR_KMERSIZE)
      {
        overflow = GT_RADIXSORT_STR_KMERSIZE - remaining;
        code = (code << GT_MULT2(overflow)) | ((1 << GT_MULT2(overflow)) - 1);
      }
    }
    return GT_RADIXSORT_STR_BUCKETNUM(code, overflow);
  }
}

/* the following calculates the lcp of two buckets */
static inline gt_radixsort_str_bucketnum_t
        gt_radixsort_str_codeslcp(gt_radixsort_str_bucketnum_t a,
                                  gt_radixsort_str_bucketnum_t b)
{
  if (a != GT_RADIXSORT_STR_SPECIAL_BUCKET &&
      b != GT_RADIXSORT_STR_SPECIAL_BUCKET)
  {
    gt_radixsort_str_bucketnum_t xorvalue = a ^ b, idx,
    codeslcp = GT_RADIXSORT_STR_KMERSIZE, maxcodeslcp, ovb;
    for (idx = GT_RADIXSORT_STR_KMERSIZE; idx > 0; idx--)
    {
      /* + KMERSIZELOG because of overflow bits */
      if (xorvalue & (3 << (GT_MULT2(idx-1) + GT_RADIXSORT_STR_KMERSIZELOG)))
      {
        codeslcp -= idx;
        break;
      }
    }
    /* now take the overflow into account */
    maxcodeslcp = GT_RADIXSORT_STR_OVERFLOW_NONSPECIAL(a);
    ovb = GT_RADIXSORT_STR_OVERFLOW_NONSPECIAL(b);
    if (ovb > maxcodeslcp)
    {
      maxcodeslcp = ovb;
    }
    maxcodeslcp = GT_RADIXSORT_STR_KMERSIZE - maxcodeslcp;
    if (codeslcp > maxcodeslcp)
    {
      codeslcp = maxcodeslcp;
    }
    return codeslcp;
  }
  return 0;
}

unsigned long gt_radixsort_str_minwidth(void)
{
  return (unsigned long) GT_RADIXSORT_STR_NOFBUCKETS;
}

static void gt_radixsort_str_insertionsort(GtRadixsortstringinfo *rsi,
                                           unsigned long sortmaxdepth,
                                           const GtRadixsortStrBucketInfo
                                             *bucket,
                                           GtLcpvalues *lcpvalues,
                                           unsigned long subbucketleft)
{
  unsigned long pm, pl, u, v, depth;

  for (pm = 1UL; pm < bucket->width; pm++)
  {
    for (pl = pm; pl > 0; pl--)
    {
      gt_radixsort_str_bucketnum_t codeslcp, unk = 0, vnk = 0;
      int uvcmp = 0;

      u = bucket->suffixes[pl-1];
      v = bucket->suffixes[pl];
      for (depth = bucket->depth;
           (sortmaxdepth == 0 || depth <= sortmaxdepth) && uvcmp == 0;
           /* Nothing */)
      {
        unk = gt_radixsort_str_get_code(rsi->twobitencoding, u, depth,
                                        rsi->equallengthplus1,
                                        rsi->realtotallength);
        vnk = gt_radixsort_str_get_code(rsi->twobitencoding, v, depth,
                                        rsi->equallengthplus1,
                                        rsi->realtotallength);
        codeslcp = gt_radixsort_str_codeslcp(unk, vnk);
        depth += codeslcp;
        if (unk < vnk)
        {
          uvcmp = -1;
        } else
        {
          if (unk > vnk)
          {
            uvcmp = 1;
          } else
          {
            if (GT_RADIXSORT_STR_HAS_OVERFLOW(unk))
            {
              if (GT_RADIXSORT_STR_HAS_OVERFLOW(vnk))
              {
                uvcmp = -1;
              } else
              {
                uvcmp = 1;
              }
            } else
            {
              if (GT_RADIXSORT_STR_HAS_OVERFLOW(vnk))
              {
                uvcmp = -1;
              } else
              {
                uvcmp = 0;
              }
            }
          }
        }
        gt_assert((uvcmp == 0 && codeslcp == GT_RADIXSORT_STR_KMERSIZE) ||
                   (uvcmp != 0 && codeslcp < GT_RADIXSORT_STR_KMERSIZE));
      }
      if (lcpvalues != NULL)
      {
        if (pl < pm && uvcmp > 0)
        {
          gt_lcptab_update(lcpvalues,subbucketleft,pl+1,
                           gt_lcptab_getvalue(lcpvalues,subbucketleft,pl));
        }
        gt_lcptab_update(lcpvalues,subbucketleft,pl,
                         sortmaxdepth == 0 ? depth : MIN(depth,sortmaxdepth));
      }
      if (uvcmp < 0)
      {
        break;
      }
      bucket->suffixes[pl-1] = v;
      bucket->suffixes[pl] = u;
    }
  }
}

GT_STACK_DECLARESTRUCT(GtRadixsortStrBucketInfo, 1024);

void gt_radixsort_str_eqlen(GtRadixsortstringinfo *rsi,
                            unsigned long *suffixes,
                            GtLcpvalues *lcpvalues,
                            unsigned long subbucketleft,
                            unsigned long depth,
                            unsigned long sortmaxdepth,
                            unsigned long width)
{
  unsigned long idx, previousbucketsize, offset,
                *bucketindex; /* overlay with sizesofbuckets */
  GtStackGtRadixsortStrBucketInfo stack;
  GtRadixsortStrBucketInfo bucket, subbucket;

  gt_assert(width <= rsi->maxwidth);
  gt_assert(sortmaxdepth == 0 ||
            (depth <= sortmaxdepth && sortmaxdepth <= rsi->equallengthplus1));

  GT_STACK_INIT(&stack, 1024UL);

  bucket.suffixes = suffixes;
  bucket.width = width;
  bucket.depth = depth;
  bucket.lcp = depth;
  GT_STACK_PUSH(&stack, bucket);

  bucketindex = rsi->sizesofbuckets;
  while (!GT_STACK_ISEMPTY(&stack))
  {
    gt_radixsort_str_bucketnum_t prevbucketnum
      = GT_RADIXSORT_STR_SPECIAL_BUCKET;
                                   /* = undefined */
    bucket = GT_STACK_POP(&stack);
    memset(rsi->sizesofbuckets, 0, rsi->bytesinsizesofbuckets);

    /* Loop A */
    for (idx = 0; idx < bucket.width; idx++)
    {
      rsi->oracle[idx] = gt_radixsort_str_get_code(rsi->twobitencoding,
                                                   bucket.suffixes[idx],
                                                   bucket.depth,
                                                   rsi->equallengthplus1,
                                                   rsi->realtotallength);
    }
    for (idx = 0; idx < bucket.width; idx++)
    {
      rsi->sizesofbuckets[rsi->oracle[idx]]++;
    }
    previousbucketsize = rsi->sizesofbuckets[0];
    bucketindex[0] = 0;
    for (idx = 1UL; idx < (unsigned long) GT_RADIXSORT_STR_NOFBUCKETS; idx++)
    {
      unsigned long tmp;

      tmp = bucketindex[idx-1] + previousbucketsize;
      previousbucketsize = rsi->sizesofbuckets[idx];
      bucketindex[idx] = tmp;
    }

    /* Loop B */
    gt_assert(width > 1UL);
    if (bucket.suffixes[0] > bucket.suffixes[1])
    {
      for (idx = bucket.width; idx > 0; /* Nothing */)
      {
        idx--;
        rsi->sorted[bucketindex[rsi->oracle[idx]]++] = bucket.suffixes[idx];
      }
    } else
    {
      for (idx = 0; idx < bucket.width; idx++)
      {
        rsi->sorted[bucketindex[rsi->oracle[idx]]++] = bucket.suffixes[idx];
      }
    }

    memcpy(bucket.suffixes, rsi->sorted, sizeof (*rsi->sorted) * bucket.width);

    subbucket.suffixes = bucket.suffixes;
    subbucket.depth = bucket.depth + GT_RADIXSORT_STR_KMERSIZE;
    if (bucket.depth < rsi->equallengthplus1)
    {
      gt_radixsort_str_bucketnum_t bucketnum;

      for (bucketnum = 0;
           bucketnum < (gt_radixsort_str_bucketnum_t)
                       GT_RADIXSORT_STR_NOFBUCKETS;
           bucketnum++)
      {
        subbucket.width
          = bucketnum > 0 ? (bucketindex[bucketnum] - bucketindex[bucketnum-1])
                          : bucketindex[bucketnum];
        if (subbucket.width > 0)
        {
          offset = (unsigned long) (subbucket.suffixes - suffixes);
          if (prevbucketnum == GT_RADIXSORT_STR_SPECIAL_BUCKET)
          {
            subbucket.lcp = bucket.lcp;
          } else
          {
            subbucket.lcp = bucket.depth +
                            gt_radixsort_str_codeslcp(prevbucketnum,bucketnum);
          }
          if (lcpvalues != NULL)
          {
            gt_lcptab_update(lcpvalues,subbucketleft,offset,
                             sortmaxdepth == 0
                               ? subbucket.lcp
                               : MIN(subbucket.lcp,sortmaxdepth));
          }
          if (GT_RADIXSORT_STR_HAS_OVERFLOW(bucketnum))
          {
            if (lcpvalues != NULL)
            {
              unsigned long j,
                            lcpvalue = bucket.depth
                                       + GT_RADIXSORT_STR_KMERSIZE
                                       - GT_RADIXSORT_STR_OVERFLOW(bucketnum);

              if (sortmaxdepth > 0 && lcpvalue > sortmaxdepth)
              {
                lcpvalue = sortmaxdepth;
              }
              for (j = 1UL; j < subbucket.width; j++)
              {
                gt_lcptab_update(lcpvalues,subbucketleft,offset+j,lcpvalue);
              }
            }
          } else
          {
            if (subbucket.width > 1UL)
            {
              if (sortmaxdepth == 0 || subbucket.depth <= sortmaxdepth)
              {
                const unsigned long radixsort_str_insertion_sort_max = 31UL;

                if (subbucket.width <= radixsort_str_insertion_sort_max)
                {
                  gt_radixsort_str_insertionsort(rsi,
                                                 sortmaxdepth,
                                                 &subbucket,
                                                 lcpvalues,
                                                 subbucketleft + offset);
                } else
                {
                  GT_STACK_PUSH(&stack, subbucket);
                }
              } else
              {
                if (lcpvalues != NULL)
                {
                  unsigned long j;
                  for (j = 1UL; j < subbucket.width; j++)
                  {
                    gt_lcptab_update(lcpvalues,subbucketleft,offset+j,
                                     sortmaxdepth);
                  }
                }
              }
            }
          }
          subbucket.suffixes += subbucket.width;
          gt_assert(subbucket.suffixes <= bucket.suffixes + bucket.width);
          prevbucketnum = bucketnum;
        }
      }
      gt_assert(bucket.suffixes + bucket.width == subbucket.suffixes);
    }
  }
  GT_STACK_DELETE(&stack);
}
