#!/bin/bash -i

PROJECT="sensor_network"

docker build -t wsn_docs_generator docs

docker run -i -v $(pwd):/$PROJECT -d --name wsn_docs_generator wsn_docs_generator bash

chmod 755 docs/generate_docs.sh

docker exec wsn_docs_generator /$PROJECT/docs/generate_docs.sh

docker stop wsn_docs_generator

docker rm wsn_docs_generator


