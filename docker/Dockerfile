FROM ubuntu

RUN apt update -y && \
    apt upgrade -y

RUN apt-get install -y git

RUN git clone -v https://github.com/vericoin/vericoin.git ~/git/vericoin/

RUN apt-get install -y build-essential \
                       libboost-all-dev \
                       libssl1.0-dev \
                       libdb++-dev \
                       libminiupnpc-dev \
                       libcurl4-gnutls-dev \
                       libminizip-dev

RUN cd ~/git/vericoin/src/ && make -f makefile.unix

RUN cp ~/git/vericoin/src/vericoind /usr/bin/

EXPOSE 58684

CMD ["vericoind"]
