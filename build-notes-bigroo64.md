# Build notes — Bigroo64 (Ubuntu, Conda)

## Environment setup
```bash
source ~/source.miniconda
conda activate rmg-dev
```

## Build
```bash
cd ~/Development/RMG/rmgdft-sandbox-explore
mkdir -p build_explore && cd build_explore/

export CC="$CONDA_PREFIX/bin/mpicc"
export CXX="$CONDA_PREFIX/bin/mpicxx"
export FC="$CONDA_PREFIX/bin/mpif90"

cmake .. -DCMAKE_PREFIX_PATH="$CONDA_PREFIX" -DHDF5_ROOT="$CONDA_PREFIX" > cmake.log 2>&1
make VERBOSE=1 -j 4 >& make.log
```

## Rebuild a single file
```bash
cd build_explore/
touch ../TDDFT/RMG_TDDFT/RmgTddft.cpp && make -j 4
```
