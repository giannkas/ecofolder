#!/bin/bash

echo "Unfolding..."
#./ecofolder $1.ll_net
./ecofolder -i $1.ll_net

echo "Converting to dot..."
./mci2dot $1_unf.mci > $1_unf.dot

echo "Producing the pdf..."
dot -T pdf $1_unf.dot > $1_unf.pdf

echo "Displaying the pdf..."
evince $1_unf.pdf

#rm -f $1.ll_net $1.dot $1.pdf $1.mci
#rm -f $1.ll_net $1.dot $1.mci