#!/bin/bash

for j in {1..10}
do
	for i in 1 2 4 8 16
	do
	 	sbatch --mem=4G --constraint=elves --ntasks-per-node=$i --nodes=2 mpi_sbatch.sh
	done
done
