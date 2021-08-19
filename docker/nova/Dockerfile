FROM  dingpf/slf6.7:mpich
MAINTAINER ssehrish@fnal.gov 

# The tarball containing the /cvmfs filesystem does NOT
# contain the leading "/" directory, so we want to unwind
# the contents into /.
#ADD nova_cvmfs.tar.bz2 /
ADD cvmfs /cvmfs

COPY env_s190703.sh /env_s190703.sh
RUN echo "source /env_s190703.sh" >> /etc/profile.d/setup_nova_env_s190703.sh
COPY make_symlinks.sh /etc/profile.d
RUN mkdir /input && mkdir /output && mkdir /job
COPY ./job/* /job/
