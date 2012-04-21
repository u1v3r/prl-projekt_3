#!/bin/bash

sucet(){ echo "$1+$2" | bc;}

#Pocet kroku pro kazdy vstu[
steps=$1
time=0.0
sum_nums=0
sum_time=0

#Odstrani obsah vystupni slozky
rm -f ./times.out

#Ziska seznam vstupu
cd IN_TEST
names=`ls lattice_*.in`
cd ..

#Pruhod pres vstupy
for name in $names
do
	#Kopie do lattice
	cp IN_TEST/$name lattice
	echo "------------------------------------------"
	echo $name
	#Vystupni soubor
	./test $steps > ./times.out
	nums=`cat ./times.out | grep '[^ ]' | wc -l`

	while read line        
	do        
	    time=$(sucet $line $time);
	done < ./times.out
	sum_nums=$(sucet $sum_nums $nums)
	sum_time=$(sucet $sum_time $time)
	echo "CPUs cas: $time s"
	echo "Pocet riadkov:" $nums
	echo "Priemerny cas: `echo "$time/$nums" | bc -l` s"
done

echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo "Celkovy CPUs cas: $sum_time s";
echo "Celkovy pocet riadkov: $sum_nums";
echo "Primerny cas: `echo " $sum_time/$sum_nums" | bc -l` s"
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"






