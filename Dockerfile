FROM nvidia/cuda:12.6.2-devel-ubuntu24.04

WORKDIR /workspace

RUN apt-get update

RUN apt-get install -y cmake
RUN apt-get install -y libsdl2-dev
RUN apt-get install -y x11-apps

RUN rm -rf /var/lib/apt/lists/*

ENV LIBGL_ALWAYS_SOFTWARE=1
