#!/bin/bash

sucet(){ echo "$1+$2" | bc;}

#Pocet kroku pro kazdy vstu[
steps=$1
time=0.0

#Odstrani obsah vystupni slozky
rm -f ./times.out

#Ziska seznam vstupu
cd IN
names=`ls lattice_*.in`
cd ..

#Pruhod pres vstupy
for name in $names
do
	#Kopie do lattice
	cp IN/$name lattice
	echo $name
	#Vystupni soubor
	./test $steps >> ./times.out
done
while read line        
do        
    time=$(sucet $line $time);
done < ./times.out

nums=`cat ./times.out | grep '[^ ]' | wc -l`

echo "Celkovy cas: $time"
echo "Pocet hodnot:" $nums
echo "$time/$nums" | bc -l;


