FROM ubuntu

RUN apt-get update
RUN apt-get install git -y
RUN apt-get install python -y
RUN apt-get install build-essential cmake -y
RUN apt-get install python-dev -y
RUN apt-get install python-pip -y
RUN pip install numpy
RUN pip install networkx
RUN pip install matplotlib

WORKDIR /home
RUN git clone https://github.com/kkcam/graph-ismorphism.git
RUN git clone https://github.com/msoos/cryptominisat.git

WORKDIR cryptominisat
RUN mkdir build
WORKDIR build
RUN cmake ..
RUN make
RUN make install
RUN ldconfig

# Below must be run manually
# RUN apt-get install python-tk -y

ENTRYPOINT ["tail", "-f", "/dev/null"]