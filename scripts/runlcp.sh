#!/bin/sh
set -e -x

if test $# -eq 0
then
  filenames="`find testdata/ -name '*.fna'` testdata/at1MB"
else
  if test $1 == 'valgrind'
  then
    RUNNER=valgrind.sh
    shift
  else
    RUNNER=time
  fi
  if test $# -eq 0
  then
    filenames="`find testdata/ -name '*.fna'` testdata/at1MB"
  else
    filenames=$*
  fi
fi

suffixerator()
{
  ${RUNNER} gt suffixerator -v -showtime -dna -tis -lcp -suf -des -ssp -db ${filename} $*
}

suffixeratoronlysuf()
{
  ${RUNNER} gt suffixerator -v -showtime -dna -tis -suf -db ${filename} $*
}

sfxmap()
{
  gt dev sfxmap -lcp -suf $*
}

sfxmaponlysuf()
{
  gt dev sfxmap -suf $*
}

for filename in ${filenames}
do
  suffixerator -indexname sfx-idx 
  sfxmap sfx-idx
  suffixerator -dir rev -indexname sfx-idx 
  sfxmap sfx-idx
  suffixerator -maxdepth -indexname sfx-idx
  sfxmap sfx-idx
  maxdepth=`grep '^prefixlength=' sfx-idx.prj | sed -e 's/prefixlength=//'`
  maxdepth=`expr ${maxdepth} \* 2`
  suffixerator -maxdepth ${maxdepth} -indexname sfx-idx${maxdepth}
  sfxmap sfx-idx${maxdepth}
  suffixerator -parts 3 -indexname sfx-idx
  sfxmap sfx-idx
  suffixerator -parts 3 -maxdepth -indexname sfx-idx
  sfxmap sfx-idx
  suffixerator -parts 3 -maxdepth he -indexname sfx-idx
  sfxmap sfx-idx
  suffixerator -parts 3 -maxdepth abs -indexname sfx-idx
  sfxmap sfx-idx
  suffixeratoronlysuf -dc 32 -indexname sfx-idx
  sfxmaponlysuf sfx-idx
  suffixeratoronlysuf -cmpcharbychar -dc 32 -indexname sfx-idx
  sfxmaponlysuf sfx-idx
  suffixeratoronlysuf -cmpcharbychar -dc 16 -indexname sfx-idx
  sfxmaponlysuf sfx-idx
  suffixeratoronlysuf -cmpcharbychar -dc 8 -indexname sfx-idx
  sfxmaponlysuf sfx-idx
  ${RUNNER} gt suffixerator -v -showtime -smap Transab -tis -suf -dc 64 -db testdata/fib25.fas.gz -indexname sfx-idx
  rm -f sfx-idx.* sfx-idx${maxdepth}.*
done
echo "${filenames}"
