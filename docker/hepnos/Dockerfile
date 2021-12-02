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
RUN source /cvmfs/nova.opensciencegrid.org/externals/setup && \
    wget https://scisoft.fnal.gov/scisoft/packages/cmake/v3_14_3/cmake-3.14.3-slf6-x86_64.tar.bz2 && \
    tar xf cmake-3.14.3-slf6-x86_64.tar.bz2 && \
    rm cmake-3.14.3-slf6-x86_64.tar.bz2

RUN wget https://archives.fedoraproject.org/pub/archive/epel/6/x86_64/Packages/p/patchelf-0.9-10.el6.x86_64.rpm && \
    rpm -i patchelf-0.9-10.el6.x86_64.rpm && \
    rm patchelf-0.9-10.el6.x86_64.rpm

RUN rm -rf /mpich

#RUN setup upd && \
#    upd install forge_tools v19_0_3

RUN mkdir -p /hepnos/spack

ENV SPACK_ROOT=/hepnos/spack \
    FORCE_UNSAFE_CONFIGURE=1

LABEL version="0.5"
# Clone spack and setup its environment
# spack@develop no longer supports python 2.6! Use the last release instead.
RUN curl -s -L https://github.com/spack/spack/releases/download/v0.17.0/spack-0.17.0.tar.gz | \
        tar xzC ${SPACK_ROOT} --strip 1 && \
    echo "source $SPACK_ROOT/share/spack/setup-env.sh" >> /etc/profile.d/spack.sh

SHELL ["/bin/bash", "-lc"]

# Clone mochi-spack-packages
RUN git clone https://github.com/mochi-hpc/mochi-spack-packages.git /hepnos/mochi-spack-packages

# Copy spack.yaml environment file to /hepnos
COPY spack.yaml /hepnos

RUN mv /hepnos/spack/etc/spack/defaults/config.yaml /hepnos/spack/etc/spack/config.yaml
RUN sed -i 's/rpath/runpath/g' /hepnos/spack/etc/spack/config.yaml

# Create the spack environment
RUN spack env create hepnos-env /hepnos/spack.yaml

# Concretize the environment
RUN spack env activate hepnos-env && spack concretize

# Install the environment, cleanup after
RUN spack env activate hepnos-env && spack install --fail-fast && spack gc -y

WORKDIR /usr/local/diy
RUN git checkout -- CMakeLists.txt && \
    git checkout -- examples/simple/until-done.cpp && \
    git pull && \
    git checkout d2ee1c5ce8b22baf47e29cb090bd3762aef31344

