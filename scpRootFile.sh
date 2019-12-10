#!/bin/bash

commitid=$1
ly=$3
ref=$2
pgun=$4

mkdir "$commitid-ref$ref-ly$ly-$pgun"
scp "kekcc:~/tools/optsim-superfgd/$commitid-ref$ref-ly$ly-$pgun/*.root" "./$commitid-ref$ref-ly$ly-$pgun/"
