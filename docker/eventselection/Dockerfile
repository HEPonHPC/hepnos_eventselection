FROM heponhpc/hepnos:4.3
MAINTAINER ssehrish@fnal.gov

WORKDIR /code

LABEL version="1.62"

RUN spack load -r hepnos && \
    export PATH=$PATH:/mpich/bin && \
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mpich/lib && \
    cd /code && \
    git clone https://github.com/hepnos/HEPnOS-NOvA-Classes.git && \
    ln -s /code/HEPnOS-NOvA-Classes /usr/include/dataformat && \
    export DATAFORMAT_INC=/usr/include/dataformat && \
    export CEREAL_INC=`spack location -i cereal`/include && \
    export THALLIUM_INC=`spack location -i mochi-thallium`/include

COPY *.hpp *.cc *.C hepnos-daemon-config.yaml CMakeLists.txt /code/ 

WORKDIR /code

RUN spack load -r hepnos && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_PREFIX_PATH="/hepnos/opt/spack/linux-scientificfermi6-k10/gcc-6.4.0/hepnos-main-pm7ljvdi2tp2s252qdhy52fq2zfhud5n/share/cmake/hepnos/;/hepnos/opt/spack/linux-scientificfermi6-k10/gcc-6.4.0/mercury-2.0.0-w75p6aetlejtvrv34ix2nlz6mkrp72sk/share/cmake/mercury/" -DCMAKE_BUILD_TYPE=Release -DMPI_HOME=/mpich -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_C_COMPILER=$(which gcc) -DDIY_INCLUDE_DIRS=/usr/local/diy/include .. && \ 
    make -j8

