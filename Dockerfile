FROM ubuntu

# Set up env
RUN apt-get update
RUN apt-get install git -y
RUN apt-get install python -y
RUN apt-get install build-essential cmake -y
RUN apt-get install python-dev -y
RUN apt-get install python-pip -y
RUN apt-get install nauty -y
RUN pip install numpy
RUN pip install networkx
RUN pip install matplotlib
RUN apt-get update
RUN apt-get install nano -y
RUN mkdir -p /root/.config/matplotlib
RUN touch /root/.config/matplotlib/matplotlibrc
RUN echo "backend: Agg" > /root/.config/matplotlib/matplotlibrc
RUN apt-get install tmux -y

# Set up code
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
WORKDIR /home/graph-isomorphism/program

# Run
EXPOSE 5000
ENTRYPOINT ["tail", "-f", "/dev/null"]

# Following lines must be run manually
# apt-get install python-tk -y
# python main.py
# make /home/graph-isomorphism/assets/programs/conauto-2.03/src
# make /home/graph-isomorphism/assets/programs/bliss-0.73