#!/bin/bash
#PBS -N test
#PBS -l nodes=7:ppn=40
#PBS -l walltime=12:00:00
#PBS -j oe
#PBS -q long

cd $PBS_O_WORKDIR/

# load cuda
module load cuda/7.5

# execute code
mpirun -n 36 -hostfile dwarfnodes.txt ./mytestprogram
 
