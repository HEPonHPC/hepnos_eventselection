#!/bin/bash

#COBALT -A HEP_on_HPC
#COBALT -n 8
#COBALT -t 00:30:00
#COBALT --mode script

export MPICH_GNI_NDREG_ENTRIES=1024

PREFIX_PATH=/hepnos/opt/spack/cray-cnl7-mic_knl/gcc-9.3.0
PDOMAIN=hepnos-sajid
SSGFILE=/projects/HEP_on_HPC/sajid/hepnos_es/hepnos.ssg
CONNECTIONFILE=/projects/HEP_on_HPC/sajid/hepnos_es/connection.json
rm ${SSGFILE}
rm ${CONNECTIONFILE}

echo "%%% start $(date)"
module unload darshan
module swap PrgEnv-intel PrgEnv-gnu
module swap cray-libsci/20.06.1 cray-libsci/20.03.1
module load gcc/9.3.0 
module swap cray-mpich/7.7.14 cray-mpich-abi/7.7.14
echo "%%% after_module_loads $(date)"

echo "Setting up protection domain"
apstat -P | grep ${PDOMAIN} || apmgr pdomain -c -u ${PDOMAIN}

. /projects/HEP_on_HPC/hepnos/spack/share/spack/setup-env.sh
spack env activate hepnos_20211020_gcc_930

export MPICH_MAX_THREAD_SAFETY=multiple

echo "%%% before_start_daemon $(date)"
aprun -n 16 -N 4 \
	-cc none -d 16 \
	-p ${PDOMAIN} \
	bedrock ofi+gni -c hepnos.json & 
sleep 30
echo "%%% after_start_daemon $(date)"
while [ ! -f ${SSGFILE} ]; do sleep 10 && echo "waiting for ssgfile"; done

echo "%%% before_start_list_dbs $(date)"
aprun -n 1 -N 1 \
	-p ${PDOMAIN} \
	hepnos-list-databases ofi+gni -s ${SSGFILE} > ${CONNECTIONFILE}
sleep 2
echo "%%% after_start_list_dbs $(date)"
while [ ! -f ${CONNECTIONFILE} ]; do sleep 10; done
sed -i '$ d' ${CONNECTIONFILE} # we have to because aprun adds a line

unset PYTHONSTARTUP
unset LD_PRELOAD
PROJECT_DIR=/projects/HEP_on_HPC
HEPNOS_DIR=${PROJECT_DIR}/hepnos
SCRIPTS=${PROJECT_DIR}/sajid/hepnos_es

# "-l a" below means dataset label a, and we have hardcoded in the event selection code the dataset label is a. 
echo "%%% before_start_dataloader $(date)"
aprun -n 8 -N 4 \
      -d 16 \
      -cc none \
      -p ${PDOMAIN} \
      hepnos-dataloader -c ${CONNECTIONFILE} \
                        -p ofi+gni \
                        -i ${SCRIPTS}/fiftyfiles.txt \
                        -o NOvA \
                        -l a \
			-t 32 \
			-v info \
                        -b 1024 \
			-n hep::rec_energy_numu  \
			-n hep::rec_hdr  \
			-n hep::rec_sel_contain  \
			-n hep::rec_sel_cvn2017  \
			-n hep::rec_sel_cvnProd3Train  \
			-n hep::rec_sel_remid  \
			-n hep::rec_slc  \
			-n hep::rec_spill  \
			-n hep::rec_trk_cosmic  \
			-n hep::rec_trk_kalman  \
			-n hep::rec_trk_kalman_tracks  \
			-n hep::rec_vtx  \
			-n hep::rec_vtx_elastic_fuzzyk  \
			-n hep::rec_vtx_elastic_fuzzyk_png  \
			-n hep::rec_vtx_elastic_fuzzyk_png_cvnpart  \
			-n hep::rec_vtx_elastic_fuzzyk_png_shwlid 
echo "%%% after_end_dataloader $(date)"

#spack env deactivate

export HEPNOS_LIBS=${PREFIX_PATH}/`spack location -i argobots | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i mochi-margo | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i tclap | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i libfabric | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i mochi-sdskv | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i cereal | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i mercury | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i mochi-thallium | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i hepnos-dataloader | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i mochi-abt-io | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i json-c | sed 's/.*\///'`/lib64:${PREFIX_PATH}/`spack location -i hepnos | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i mochi-ch-placement | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i spdlog | sed 's/.*\///'`/lib64:${PREFIX_PATH}/`spack location -i jsoncpp | sed 's/.*\///'`/lib64:${PREFIX_PATH}/`spack location -i mochi-bedrock | sed 's/.*\///'`/lib:${PREFIX_PATH}/`spack location -i mochi-ssg | sed 's/.*\///'`/lib:/opt/cray/rca/2.2.20-7.0.2.1_2.78__g8e3fb5b.ari/lib64/:/opt/cray/udreg/2.3.2-7.0.2.1_2.32__g8175d3d.ari/lib64:/opt/cray/ugni/6.0.14.0-7.0.2.1_3.60__ge78e5b0.ari/lib64:/opt/cray/xpmem/2.2.20-7.0.2.1_2.59__g87eb960.ari/lib64:/opt/cray/rca/2.2.20-7.0.2.1_2.76__g8e3fb5b.ari/lib64:/opt/cray/ugni/6.0.14.0-7.0.2.1_3.59__ge78e5b0.ari/lib64:/opt/cray/alps/6.6.59-7.0.2.1_3.62__g872a8d62.ari/lib64

export NOVA_LIBS=/cvmfs/nova.opensciencegrid.org/externals/library_shim/v04.00/NULL/lib/sl6:/cvmfs/nova.opensciencegrid.org/externals/tbb/v2018_2a/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/ifdhc/v2_4_1/Linux64bit+2.6-2.12-e15-p2714b-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/novarwgt/v00.20/slf6.x86_64.e15.genie3.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/pythia/v6_4_28k/Linux64bit+2.6-2.12-gcc640-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/gsl/v2_4/Linux64bit+2.6-2.12-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/cetlib/v3_06_00/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/git/v2_14_1/Linux64bit+2.6-2.12/lib64:/local/s190703/lib/Linux2.6-GCC-maxopt:/cvmfs/nova.opensciencegrid.org/novasoft/slf6/novasoft/releases/S19-07-03/lib/Linux2.6-GCC-maxopt:/cvmfs/nova.opensciencegrid.org/externals/novabeamlinefragments/v07_00_02/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/hep_hpc/v0_08_07/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/xgboost/v0.60/Linux64bit+2.6-2.12/lib:/cvmfs/nova.opensciencegrid.org/externals/tensorflow/v1_12_0a/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/opencv/v3_4_2/Linux64bit+2.6-2.12-e15/lib64:/cvmfs/nova.opensciencegrid.org/externals/hdf5/v1_10_2a/Linux64bit+2.6-2.12-e15/lib:/cvmfs/nova.opensciencegrid.org/externals/lmdb/v0_9_21/Linux64bit+2.6-2.12/lib:/cvmfs/nova.opensciencegrid.org/externals/protobuf/v3_5_2/Linux64bit+2.6-2.12-e15/lib:/cvmfs/nova.opensciencegrid.org/externals/gflags/v2_2_1a/Linux64bit+2.6-2.12-e15/lib:/cvmfs/nova.opensciencegrid.org/externals/glog/v0_3_5a/Linux64bit+2.6-2.12-e15/lib:/cvmfs/nova.opensciencegrid.org/externals/snappy/v1_1_7b/Linux64bit+2.6-2.12-e15/lib64:/cvmfs/nova.opensciencegrid.org/externals/leveldb/v1_20b/Linux64bit+2.6-2.12-e15/lib:/cvmfs/nova.opensciencegrid.org/externals/lapack/v3_8_0a/Linux64bit+2.6-2.12-e15-prof/lib64:/cvmfs/nova.opensciencegrid.org/externals/caffe/v1_0k/Linux64bit+2.6-2.12-e15-prof/lib64:/cvmfs/nova.opensciencegrid.org/externals/novarwgt/v00.20/slf6.x86_64.e15.genie3.prof/lib:/cvmfs/fermilab.opensciencegrid.org/products/common/db/../prd/curl/v7_64_1/Linux64bit-2-6/lib:/cvmfs/nova.opensciencegrid.org/externals/ppfx/v02_06/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/artdaq_core/v3_04_10/slf6.x86_64.e15.s79.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/TRACE/v3_13_11/slf6.x86_64./lib:/cvmfs/nova.opensciencegrid.org/externals/nutools/v2_28_02/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/ifdh_art/v2_07_03/slf6.x86_64.e15.s79.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/nucondb/v2_3_0/Linux64bit+2.6-2.12-e15-p2714b-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/libwda/v2_26_0/Linux64bit+2.6-2.12/lib:/cvmfs/nova.opensciencegrid.org/externals/ifdhc/v2_4_1/Linux64bit+2.6-2.12-e15-p2714b-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/ifbeam/v2_3_0/Linux64bit+2.6-2.12-e15-p2714b-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/geant4/v4_10_4_p02b/Linux64bit+2.6-2.12-e15-prof/lib64:/cvmfs/nova.opensciencegrid.org/externals/xerces_c/v3_2_0a/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/cry/v1_7k/Linux64bit+2.6-2.12-e15-prof/cry_v1.7/lib:/cvmfs/nova.opensciencegrid.org/externals/log4cpp/v1_1_3a/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/lhapdf/v5_9_1k/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/genie/v3_00_04/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/dk2nugenie/v01_07_02g/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/art/v2_12_01/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/nusimdata/v1_16_01/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/dk2nudata/v01_07_02/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/canvas_root_io/v1_01_10/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/xrootd/v4_8_0b/Linux64bit+2.6-2.12-e15-prof/lib64:/cvmfs/nova.opensciencegrid.org/externals/libxml2/v2_9_5/Linux64bit+2.6-2.12-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/mysql_client/v5_5_58a/Linux64bit+2.6-2.12-e15/lib:/cvmfs/nova.opensciencegrid.org/externals/postgresql/v9_6_6a/Linux64bit+2.6-2.12-p2714b/lib:/cvmfs/nova.opensciencegrid.org/externals/pythia/v6_4_28k/Linux64bit+2.6-2.12-gcc640-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/gsl/v2_4/Linux64bit+2.6-2.12-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/fftw/v3_3_6_pl2/Linux64bit+2.6-2.12-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/root/v6_12_06a/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/canvas/v3_06_00/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/clhep/v2_3_4_6/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/messagefacility/v2_02_05/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/hep_concurrency/v1_00_03/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/cppunit/v1_13_2c/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/tbb/v2018_2a/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/fhiclcpp/v4_06_09/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/python/v2_7_14b/Linux64bit+2.6-2.12/lib:/cvmfs/nova.opensciencegrid.org/externals/cetlib/v3_06_00/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/sqlite/v3_20_01_00/Linux64bit+2.6-2.12/lib:/cvmfs/nova.opensciencegrid.org/externals/boost/v1_66_0a/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/cetlib_except/v1_02_01/slf6.x86_64.e15.prof/lib:/cvmfs/nova.opensciencegrid.org/externals/gcc/v6_4_0/Linux64bit+2.6-2.12/lib64:/cvmfs/nova.opensciencegrid.org/externals/gcc/v6_4_0/Linux64bit+2.6-2.12/lib:.:/cvmfs/nova.opensciencegrid.org/externals/lhapdf/v5_9_1k/Linux64bit+2.6-2.12-e15-prof/lib:/cvmfs/nova.opensciencegrid.org/externals/genie/v3_00_04/Linux64bit+2.6-2.12-e15-prof/GENIE-Generator/../lib:/cvmfs/nova.opensciencegrid.org/externals/roounfold/v1_1_1_a/NULL_e15_prof:/cvmfs/nova.opensciencegrid.org/externals/library_shim/v04.00/NULL/lib/sl6

# in order to pass environment variables to a Singularity container create the variable with the SINGULARITYENV_ prefix!
export SINGULARITYENV_GENIE=/cvmfs/nova.opensciencegrid.org/externals/genie/v3_00_04/Linux64bit+2.6-2.12-e15-prof/GENIE-Generator
export SINGULARITYENV_FW_BASE=/cvmfs/nova.opensciencegrid.org/novasoft/slf6/novasoft/releases/S19-07-03
export SINGULARITYENV_ROOT_INCLUDE_PATH=/cvmfs/nova.opensciencegrid.org/externals/genie/v3_00_04/Linux64bit+2.6-2.12-e15-prof/include/GENIE
#export SINGULARITYENV_MPICH_ENV_DISPLAY=1
export SINGULARITYENV_MPICH_MAX_THREAD_SAFETY=multiple
export SINGULARITYENV_MPICH_DBG_LEVEL=VERBOSE

# include CRAY_LD_LIBRARY_PATH in to the system library path
export LD_LIBRARY_PATH=$CRAY_LD_LIBRARY_PATH:$LD_LIBRARY_PATH:$HEPNOS_LIBS
# also need this additional library (why doesn't alcf document any of this!)
export LD_LIBRARY_PATH=/etc/alternatives/cray-wlm_detect/lib64:$LD_LIBRARY_PATH

# LD_LIBRARY_PATH within the container needs to assist with the executable:
# [a] pre-pend everything by $CRAY_LD_LIBRARY_PATH and extras (done above) 
# [b] finding the CAF-ANA dependencies for NoVA analysis
# [c] overriding the libfabric within the container with the libfabric built on theta with ugni support
# mounting symlinks is tricky, which is why we use full filesystem paths with identical locations which is tedious, but gets the job done (for now at least!)
# future note: Be sure to check CRAY_LD_LIBRARY_PATH instead of tediously updaing the library version numbers one by one!
export SINGULARITYENV_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NOVA_LIBS:/opt/cray/pe/pmi/5.0.16/lib64/:/opt/cray/pe/lib64/:/hepnos/spack/var/spack/environments/hepnos-env/.spack-env/view/lib:/hepnos/spack/var/spack/environments/hepnos-env/.spack-env/view/lib64

# margo initialtion within the container
export SINGULARITYENV_MPICH_GNI_NDREG_ENTRIES=1024

echo "%%% before_event_selection $(date)"
aprun -n 16 \
      -N 4 \
      -p ${PDOMAIN} \
      singularity exec --home /home/sajid \
                       -B /etc/alternatives/cray-wlm_detect/lib64:/etc/alternatives/cray-wlm_detect/lib64:ro \
                       -B /opt:/opt:ro \
                       -B /var/opt:/var/opt:ro \
                       -B /projects/HEP_on_HPC/hepnos/spack:/hepnos \
                       -B /lib64:/lib64:ro \
                       -B /lib:/lib:ro \
                       -B /usr/lib64:/usr/lib64:ro \
                       -B /usr/lib:/usr/lib:ro \
		       -B /opt/cray:/opt/cray:ro \
		       -B /projects/HEP_on_HPC/sajid/hepnos_es:/scripts:rw \
			/projects/HEP_on_HPC/sajid/hepnos_es/hepnos_es_latest.sif /code/build/bm_eventselection -c /scripts/connection.json -p ofi+gni -d NOvA -t 16 -v warning -l a -n hep::rec_energy_numu  -n hep::rec_hdr  -n hep::rec_sel_contain  -n hep::rec_sel_cvn2017  -n hep::rec_sel_cvnProd3Train  -n hep::rec_sel_remid  -n hep::rec_slc  -n hep::rec_spill  -n hep::rec_trk_cosmic  -n hep::rec_trk_kalman  -n hep::rec_trk_kalman_tracks  -n hep::rec_vtx  -n hep::rec_vtx_elastic_fuzzyk  -n hep::rec_vtx_elastic_fuzzyk_png  -n hep::rec_vtx_elastic_fuzzyk_png_cvnpart  -n hep::rec_vtx_elastic_fuzzyk_png_shwlid --preload -O /scripts/  

echo "%%% after_event_selection $(date)"


spack env activate hepnos_20211020_gcc_930

#echo "%%% before_start_ls $(date)"
#aprun -n 1 \
#      -N 1 \
#      -p ${PDOMAIN} \
#      hepnos-ls ofi+gni ${CONNECTIONFILE}
#echo "%%% after_end_ls $(date)"

echo "%%% before_start_shutdown $(date)"
aprun -n 1 \
      -N 1 \
      -p ${PDOMAIN} \
      hepnos-shutdown ofi+gni ${CONNECTIONFILE}
echo "%%% after_end_shutdown $(date)"

echo "Destroying protection domain"
apmgr pdomain -r -u ${PDOMAIN}
