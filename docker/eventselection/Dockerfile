FROM heponhpc/hepnos:4.5
MAINTAINER ssehrish@fnal.gov

WORKDIR /code

LABEL version="1.65"

RUN spack env activate hepnos-env && \
    export PATH=$PATH:/mpich/bin && \
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mpich/lib && \
    cd /code && \
    git clone https://github.com/hepnos/HEPnOS-NOvA-Classes.git && \
    ln -s /code/HEPnOS-NOvA-Classes /usr/include/dataformat && \
    export DATAFORMAT_INC=/usr/include/dataformat && \
    export CEREAL_INC=`spack location -i cereal`/include && \
    export THALLIUM_INC=`spack location -i mochi-thallium`/include

COPY *.hpp *.cc *.C *.txt *.yaml *.sh /code/

WORKDIR /code

RUN spack env activate hepnos-env && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DMPI_HOME=/mpich \
          -DCMAKE_CXX_COMPILER=$(which g++) \
          -DCMAKE_C_COMPILER=$(which gcc) \
          -DDIY_INCLUDE_DIRS=/usr/local/diy/include .. && \
    make -j8

RUN spack env activate hepnos-env && \
    cd build && \
    make 

