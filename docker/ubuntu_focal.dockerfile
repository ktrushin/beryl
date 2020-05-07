FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TERM linux

ARG username
ARG uid
ARG gid
ARG locale=en_US.UTF-8

# Don't drop man pages and other files from the packages being installed
RUN mv /etc/dpkg/dpkg.cfg.d/excludes /tmp/dpkg_excludes.bk
# Reinstall all already installed packages in order to get the man pages back
RUN dpkg -l | grep ^ii | cut -d' ' -f3 | \
        xargs apt-get install --yes --no-install-recommends --reinstall

# Create the user and allow him to execute `sudo` without password
RUN addgroup --gid $gid $username && \
    adduser --uid $uid --gid $gid --home /home/$username \
        --disabled-password --gecos '' $username && \
    adduser $username sudo && \
    mkdir -p /etc/sudoers.d/ && \
    echo "$username ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/$username
ENV LOGNAME=$username USER=$username

# Install apt-utils before anything else
RUN apt-get update && apt-get install --yes --no-install-recommends apt-utils

# Set the locale
RUN apt-get install --yes --no-install-recommends locales
RUN locale-gen $locale && update-locale LANG=$locale LC_CTYPE=$locale
ENV LANG=$locale LC_ALL=$locale

# Install the packages for C++ development and convenient command line usage
RUN apt-get install --yes --no-install-recommends \
        sudo lsb-release software-properties-common tzdata \
        bash-completion coreutils less tree tmux vim gnupg2 ca-certificates \
        man-db manpages manpages-dev \
        ack git-core \
        g++ clang clang-format clang-tidy \
        ccache \
        autoconf automake m4 autotools-dev libtool \
        make ninja-build \
        cmake cmake-data \
        meson \
        gdb lldb \
        build-essential debhelper devscripts fakeroot dput \
        libboost-all-dev libjemalloc-dev \
        python3 python3-dev python3-setuptools python3-pip python3-wheel \
        iproute2 bind9 bind9-dnsutils tcpdump

ENV CCACHE_DIR=/var/c_cache
RUN mkdir -p $CCACHE_DIR && chown $username:$username $CCACHE_DIR
ENV PYTHONDONTWRITEBYTECODE=1
