#!/bin/bash

#Vystupni adresar - musi existovat
outdir=OOUT

#Pocet kroku pro kazdy vstu[
steps=100

#Ziska seznam vstupu
cd IN
names=`ls lattice_*.in`
cd ..

#Odstrani obsah vystupni slozky
rm -f $outdir/*

#Pruhod pres vstupy
for name in $names
do
	#Kopie do lattice
	cp IN/$name lattice
	#Vyfiltrovani cisla
	outNumber=`echo $name | sed 's/lattice_\([0-9]*\).*[.]in/\1/'`
	#Vystupni soubor
	./test $steps >> $outdir/lattice_$outNumber.out 
	#Porovnani
	diff $outdir/lattice_$outNumber.out OUT/lattice_$outNumber.out
	if [ $? -ne 0 ]
	then
		echo "lattice_$outNumber.out --- [KO]"
	else
		echo "lattice_$outNumber.out --- [OK]"	
	fi
done
