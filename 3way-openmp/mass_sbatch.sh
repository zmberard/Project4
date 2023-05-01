#!/bin/bash

for i in 1 2 4 8 16
do
	 sbatch --mem=4G --constraint=moles --ntasks-per-node=$i --nodes=1 openmp_sbatch.sh
done
