#!/bin/sh

######################### READ AND VALIDATE USER INPUT #########################
prog="docker.sh"

default_distrib_codename="bionic"
# At present, only one Ubuntu distribution codename is allowed.
# In the future, `focal` may be also supported.
allowed_distrib_codenames="$default_distrib_codename"

codenames_msg=""
for name in $allowed_distrib_codenames; do
    codenames_msg="${codenames_msg:+${codenames_msg}, }\`${name}\`"
done

help_msg="\
Description:
    The program enters in a Docker container for developing this C++ project
    under Ubuntu of specified codename. The respective Docker image and
    container are automatically created if they don't exist. After that,
    the container is started and bash is executed in it interactively.
    The program must be called from the project's root directory.

Usage:
    $prog [--codename <codename>]

Options:
    --distrib_codename <codename>
        Codename of the desired Ubuntu distibution. At the moment,
        the following codenames are supported: $codenames_msg.
        Default is \`$default_distrib_codename\`."

print_error() {
    echo "Error: $1\n$help_msg" >&2
}

if [ ! -d ".git" ]; then
    print_error "$prog must be called from the project's root directory."
    exit 1
fi

distrib_codename=$default_distrib_codename
while [ "$#" -gt 0 ]; do
    case "$1" in
        -h|--help)
            echo "$help_msg"
            exit 0;
            ;;
        --distrib_codename)
            distrib_codename=$2
            shift 2
            ;;
        *)
            print_error "unrecognized option or argument \`$1\`"
            exit 1
            ;;
    esac
done

found=$(echo "$allowed_distrib_codenames" | \
    grep -P "(^|\s)$distrib_codename(\$|\s)")
if [ -z "$found" ]; then
    print_error "unsupported distrib_codename: \`$distrib_codename\`"
    exit 1
fi

############################ SET UP SOME CONSTANTS #############################
proj_name=$(basename $(pwd))
image_name=$proj_name
image_tag=$distrib_codename
container_name="${image_name}_${image_tag}"

########### CREATE DOCKER CONTAINER AND CONTAINER THEY DO NOT EXIST ############
if [ -z "$(docker image ls -q $image_name:$image_tag)" ]; then
    echo "no image"
    docker image build -f docker/Dockerfile_ubuntu_${distrib_codename} \
    --build-arg username=$(id -un) \
    --build-arg uid=$(id -u) \
    --build-arg gid=$(id -g) \
    -t $image_name:$image_tag .
fi

if [ -z "$(docker container ls -aq --filter name=$container_name)" ]; then
    docker container create \
    --privileged -it \
    --name=$container_name --hostname=$container_name \
    --user=$LOGNAME \
    -v $HOME:$HOME -v $(pwd):/$proj_name -w /$proj_name \
    $image_name:$image_tag /bin/bash
fi

########## START DOCKER CONTAINER AND EXECUTE BASH INTERACTIVELY THERE #########
docker container start $container_name
docker container exec -it $container_name bash
