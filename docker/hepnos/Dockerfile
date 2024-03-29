FROM  heponhpc/novas190703:2.0
MAINTAINER ssehrish@fnal.gov

RUN cd /cvmfs/nova.opensciencegrid.org/externals && \
    wget https://scisoft.fnal.gov/scisoft/packages/ups/v6_0_6/ups-6.0.6-Linux64bit%2B2.6-2.12.tar.bz2 && \
    tar xf ups-6.0.6-Linux64bit+2.6-2.12.tar.bz2 && \
    rm ups-6.0.6-Linux64bit+2.6-2.12.tar.bz2 && \
    wget https://scisoft.fnal.gov/scisoft/packages/upd/v5_0_1/upd-5.0.1-noarch.tar.bz2 && \
    tar xf upd-5.0.1-noarch.tar.bz2 && \
    rm upd-5.0.1-noarch.tar.bz2

WORKDIR /cvmfs/nova.opensciencegrid.org/externals
RUN source /cvmfs/nova.opensciencegrid.org/externals/setup

RUN wget https://archives.fedoraproject.org/pub/archive/epel/6/x86_64/Packages/p/patchelf-0.9-10.el6.x86_64.rpm && \
    rpm -i patchelf-0.9-10.el6.x86_64.rpm && \
    rm patchelf-0.9-10.el6.x86_64.rpm

RUN wget http://ftp.scientificlinux.org/linux/scientific/obsolete/6/x86_64/os/Packages/unzip-6.0-5.el6.x86_64.rpm && \
    rpm -i unzip-6.0-5.el6.x86_64.rpm && \
    rm unzip-6.0-5.el6.x86_64.rpm


RUN rm -rf /mpich

#RUN setup upd && \
#    upd install forge_tools v19_0_3

RUN mkdir -p /hepnos/spack

ENV SPACK_ROOT=/hepnos/spack \
    FORCE_UNSAFE_CONFIGURE=1

LABEL version="0.7.0"
# Clone spack and setup its environment
# spack@develop no longer supports python 2.6! Use the last release instead.
RUN curl -s -L https://github.com/spack/spack/releases/download/v0.17.3/spack-0.17.3.tar.gz | \
        tar xzC ${SPACK_ROOT} --strip 1 && \
    echo "source $SPACK_ROOT/share/spack/setup-env.sh" >> /etc/profile.d/spack.sh

SHELL ["/bin/bash", "-lc"]

# Clone mochi-spack-packages
RUN git clone https://github.com/mochi-hpc/mochi-spack-packages.git /hepnos/mochi-spack-packages

# Copy spack.yaml environment file to /hepnos
COPY spack.yaml /hepnos

RUN mv /hepnos/spack/etc/spack/defaults/config.yaml /hepnos/spack/etc/spack/config.yaml
RUN cd /hepnos/spack/var/spack/repos/builtin/packages/spdlog && \
    rm package.py && \
    wget https://raw.githubusercontent.com/spack/spack/develop/var/spack/repos/builtin/packages/spdlog/package.py && \
    cd /
RUN sed -i 's/rpath/runpath/g' /hepnos/spack/etc/spack/config.yaml

# Find compiler and build package cache
RUN spack compiler find && spack spec -I cmake

# Create the spack environment
RUN spack env create hepnos-env /hepnos/spack.yaml

# Concretize the environment
RUN spack env activate hepnos-env && spack -dd concretize -f

# Install the environment, cleanup after
RUN spack env activate hepnos-env && spack install --fail-fast --yes-to-all && spack gc -y
