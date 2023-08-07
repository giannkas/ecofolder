#!/bin/bash

echo "Converting to dot..."
#./llnet2dot $1.ll_net
./llnet2dot $1.ll

echo "Producing the pdf..."
dot -T pdf $1_net.dot > $1_net.pdf

echo "Displaying the pdf..."
evince $1_net.pdf

#rm -f $1.ll_net $1.dot $1.pdf $1.mci
#rm -f $1.ll_net $1.dot $1.mci