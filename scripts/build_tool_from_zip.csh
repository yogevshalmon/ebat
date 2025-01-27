#! /bin/csh -f 

# the script is used to build a tool after unzipping the tool package (meaning no download from git)
# please run it when you are in the tool package directory

# manually clone the needed libraries
git clone https://github.com/alexander-nadel/intel_sat_solver.git libs/intel_sat_solver
git clone https://github.com/hriener/lorina.git libs/lorina
git clone https://github.com/arminbiere/cadical.git libs/sat/cadical


cmake -S . -B build
cd build
make