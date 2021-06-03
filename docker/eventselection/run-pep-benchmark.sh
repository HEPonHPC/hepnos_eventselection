#!/bin/bash

echo "Starting HEPnOS daemons"
mpirun -np 2 hepnos-daemon hepnos-daemon-config.yaml client.yaml &
PID=$!

sleep 2

echo "Running data loader"
mpirun -n 2 hepnos-dataloader -c client.yaml -i files.txt -o NOvA  -l a -a -t 1 -b 1024 \
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

echo "Running PEP benchmark"
mpirun -n 1 hepnos-pep-benchmark -c client.yaml -d NOvA -t 2 -v debug -l a \
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

echo "Shutting down HEPnOS"
hepnos-shutdown client.yaml
