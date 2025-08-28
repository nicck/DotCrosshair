FROM ubuntu:20.04

ARG arch=amd64

SHELL ["/bin/bash", "--login", "-c"]

RUN apt-get update --fix-missing && \
    apt-get install -y sudo net-tools apt-utils curl git zip unzip xz-utils upx

RUN useradd -ms /bin/bash developer
RUN adduser developer sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER developer

RUN echo "source /usr/share/bash-completion/completions/git" >> /home/developer/.bashrc

WORKDIR /workspaces/DotCrosshair

RUN curl -SL -O "https://ziglang.org/download/0.15.1/zig-x86_64-linux-0.15.1.tar.xz" && \
    tar -xvf zig-x86_64-linux-0.15.1.tar.xz && \
    mv zig-x86_64-linux-0.15.1 /home/developer/.zig && \
    rm zig-x86_64-linux-0.15.1.tar.xz

ENV PATH="$PATH:/home/developer/.zig"

ENTRYPOINT ["bash", "-i"]
CMD []
