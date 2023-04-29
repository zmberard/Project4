#!/bin/bash -l
#SBATCH --mem=4G
#SBATCH --time=03:00:00
#SBATCH --constraint=moles
#SBATCH --job-name=openMP
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4

/homes/jonahmbog/cis520/Project4/3way-openmp/main
