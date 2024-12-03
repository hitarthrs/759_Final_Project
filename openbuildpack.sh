sudo docker run \
    --rm \
    -it \
    -v .:/workspace \
    -w /workspace \
    $BUILDPACK_GPU_OPTION \
    -e DISPLAY=$(ip route list default | awk '{print $3}'):0 \
    fractal-image \
    /bin/bash
