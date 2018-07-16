#!/usr/bin/env bash

function pozri() {
    O=`pwd`
    DIR=`mktemp -d`
    cd $DIR

    wget -t inf --waitretry=60 http://158.195.28.94/records/$1.tar.gz
    tar xf $1.tar.gz
    $O/observer/observer $1

    cd $O
    rm -rf $DIR
}

IP="158.195.28.94"
SI=$1

while true; do
    pozri $(printf "%06d" $SI)
    SI=$((SI+1))
done
