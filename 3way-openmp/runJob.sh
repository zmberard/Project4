#!/bin/sh
sbatch --mem=4G --time=03:00:00 --constraint=moles --job-name=openMP --nodes=1 --ntasks-per-node=4 Job.sh