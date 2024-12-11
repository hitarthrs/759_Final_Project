## 759_Final_Project
Final Project for CS759 - Fall 2024

#### Getting started on Linux
- Install X11 Server
    - Since we are running in a container,
      graphics must be sent to host OS via X11 forwarding
    - On Windows host machine, Xming is known to work
        - Search for, download, launch "Xming" on host machine
        - Will remain in system tray until window is sent from container
        - X11 can be tested by running `xeyes` within container
- Install Docker
    - If using WSL, install Docker Desktop in Windows
    - else, follow instructions at https://docs.docker.com/engine/install/
- Ensure base docker image exists and is accessible
	- `docker pull nvidia/cuda:12.6.2-devel-ubuntu24.04`
- Build fractal-image docker image from within project dir
    - `sudo docker build -t fractal-image .`
- If your machine has an Nvidia GPU
    - `echo export BUILDPACK_GPU_OPTION=--gpus=all >> ~/.bashrc`
    - `source ~/.bashrc`
- Launch image
    - `./openbuildpack.sh`
- Build
    - `mkdir build`
    - `cd build`
    - `cmake ..`
    - `make`
- Run
    - `./fractal`
