set -x
sudo apt-get update
sudo apt-get install build-essential cmake python3-pip

# WeGotYouCovered dependencies
sudo apt-get install libargtable2-dev libomp-dev

# Gurobi
wget https://packages.gurobi.com/12.0/gurobi12.0.1_linux64.tar.gz
mv gurobi12.0.1_linux64.tar.gz src/ext
cd src/ext
tar -xvzf gurobi12.0.1_linux64.tar.gz && rm gurobi12.0.1_linux64.tar.gz

# Stresstesting
pip3 install tqdm || sudo apt-get install python3-tqdm
