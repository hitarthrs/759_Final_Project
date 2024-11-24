sudo docker run \
    --rm \
    -it \
    -v .:/workspace \
    -w /workspace \
    --gpus=all \
    fractal-image \
    /bin/bash