#! /bash/sh
apt-get update
apt-get install -y --allow-unauthenticated cmake

cd /home/vsts/work/1/s/build
pwd

ls

bin/mlpack_test -s -r console AKNNDualCoverTreeTest

# less results.xml

