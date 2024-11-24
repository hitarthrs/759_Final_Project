FROM nvidia/cuda:12.6.2-devel-ubuntu24.04

WORKDIR /workspace

RUN apt-get update && \
    apt-get install -y cmake && \
    rm -rf /var/lib/apt/lists/*
