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

#COPY git clone https://ssehrish@bitbucket.org/hep_on_hpc/hepnos_example.git && \
#    cd hepnos_example && \ 
#    git checkout master && \
#    git checkout hepnosdemo_1_60 && \ 
#    cd  eventselection && \
#    mkdir build && \
#    cd build && \
#    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_C_COMPILER=$(which gcc) -DDIY_INCLUDE_DIRS=/usr/local/diy/include .. && \ 
#    make -j8
#
