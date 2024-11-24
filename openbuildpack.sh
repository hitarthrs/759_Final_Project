sudo docker run \
    --rm \
    -it \
    -v .:/workspace \
    -w /workspace \
    $BUILDPACK_GPU_OPTION \
    fractal-image \
    /bin/bash