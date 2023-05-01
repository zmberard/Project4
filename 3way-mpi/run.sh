#!/bin/bash -l
#SBATCH --mem=4G
#SBATCH --time=00:30:00
#SBATCH --constraint=elves
#SBATCH --job-name=mpi
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4

/homes/jonahmbog/cis520/Project4/3way-mpi/mpi
