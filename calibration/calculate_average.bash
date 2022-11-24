#!/bin/bash

for TYPE in h t; do
  echo -n "${TYPE} "
  for ID in 180 183 184 185 186 187; do
    #echo -n "${ID} ${TYPE} "
    # This format is good for copy-pasting into spreadsheet
    grep "172.16.7.${ID}.*type=${TYPE}" ${1} | grep -v nan | sed -e 's/.*raw=//;s/ .*//' | awk '{ s+=$1; n += 1 } END { printf "%.5f\n", s/n }' ;
# this is more informative for testing
#printf "Samples:%d Total:%.5f Average:%.5f\n", n, s, s/n }' ;
  done
done

