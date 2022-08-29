FROM ubuntu:20.04
LABEL maintainer="Francisco Santos <francisco.velez@tecnico.ulisboa.pt>"

# -------------------------------
# Create the container user
# -------------------------------
ARG user=wsn
ARG passwd=wsn
ARG uid=1000
ARG gid=1000

ENV USER=$user
ENV PASSWD=$passwd
ENV UID=$uid
ENV GID=$gid


RUN groupadd $USER && \
    useradd --create-home --no-log-init -g $USER $USER && \
    usermod -aG sudo $USER && \
    echo "$PASSWD:$PASSWD" | chpasswd && \
    chsh -s /bin/bash $USER && \
    usermod --uid $UID $USER && \
    groupmod --gid $GID $USER

ENV HOME=/home/$USER
WORKDIR $HOME

# -------------------------------
# Install basic tools
# -------------------------------
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
    wget \
    curl \
    git \
    ssh \
    python3-pip \
    doxygen \
    unzip \
    && rm -rf /var/lib/apt/lists/*

# -------------------------------------------------
# Install the documentation generation requrirement
# -------------------------------------------------
RUN pip3 install --no-cache-dir \
    mkdocs \
    mkdocs-material \
    mkdocs-bibtex \
    mkdocs-git-revision-date-plugin \
    mkdocs-git-revision-date-localized-plugin \
    mkdocs-monorepo-plugin \
    mkdocs-macros-plugin \
    mkdocs-include-markdown-plugin \
    mkdocs-git-revision-date-localized-plugin \
    mkdocs-include-dir-to-nav \
    mike \
    ruamel.yaml

RUN mkdir doxybook2 && \
    cd doxybook2 && \
    wget https://github.com/matusnovak/doxybook2/releases/download/v1.4.0/doxybook2-linux-amd64-v1.4.0.zip && \
    unzip doxybook2-linux-amd64-v1.4.0.zip && \
    cd bin && mv doxybook2 /usr/bin/ && \
    cd ${HOME} && rm -f -r doxybook2

USER $USER