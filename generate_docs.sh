#!/bin/bash 

PROJECT=sensor_network
REPO_NAME=sensor_network_docs

PACKETS=( node gateway_serial)
PACKETS_NAME=( Node Gateway )
length=${#PACKETS[@]}


token=`cat /$PROJECT/.secret`  

for (( j=0; j<length; j++ ));
do
    cd /$PROJECT/${PACKETS[$j]}

    touch mkdocs.yml
    printf "site_name: ${PACKETS_NAME[$j]}\n" >> mkdocs.yml
    printf "nav:\n" >> mkdocs.yml
    printf "  - Files: \"Files/index.md\"\n" >> mkdocs.yml
    printf "  - Classes: \"Classes/index.md\"\n" >> mkdocs.yml

    mkdir docs
    cd docs

    BASE_URL=/$REPO_NAME/${PACKETS_NAME[$j]}/
    #rm -r * 
    # fix
    doxygen ../../docs/Doxyfile

    echo "Generated xml and html documentation for '${PACKETS_NAME[$j]}'"

    doxybook2 --input xml \
              --output . \
              --config ../../docs/doxybook_conf.json \
              --config-data '{"baseUrl": "'$BASE_URL'"}'\
              -q

    echo "Generated markdown documentation for '${PACKETS_NAME[$j]}'"
done

cd /$PROJECT
git remote rm origin
git remote add origin https://hardtekpt:$token@github.com/hardtekpt/sensor_network_docs.git
mkdocs gh-deploy

echo "Done !!"

rm -r site

for (( j=0; j<length; j++ ));
do
    cd /$PROJECT/${PACKETS[$j]}
    rm -r docs
    rm mkdocs.yml
done




