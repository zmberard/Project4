#!/bin/sh
sbatch --mem=4G --time=03:00:00 --constraint=moles --job-name=openMP --nodes=10 --ntasks-per-node=4 main.c