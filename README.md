## 759_Final_Project
Final Project for CS759 - Fall 2024

#### Getting started on Linux
- Install Docker
    - If using WSL, install Docker Desktop in Windows
    - else, follow instructions at https://docs.docker.com/engine/install/
- Ensure base docker image exists and is accessible
	- `docker pull nvidia/cuda:12.6.2-devel-ubuntu24.04`
- Build fractal-image docker image from within project dir
    - `sudo docker build -t fractal-image .`
- If your machine has an Nvidia GPU
    - add the line `export BUILDPACK_GPU_OPTION="--gpus=all"` to `~/.bashrc`
    - run `source ~/.bashrc`
    - If you accidentally follow this step without an nvidia gpu
        - remove the above line from `~/.bashrc`
        - `unset BUILDPACK_GPU_OPTION`
- Launch image
    - `./openbuildpack.sh`
- Build
    - `mkdir build`
    - `cd build`
    - `cmake ..`
    - `make`
- Run
    - `./fractal`
