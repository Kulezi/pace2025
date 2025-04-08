# WeGotYouCovered dependencies
sudo apt-get install libargtable2-dev libomp-dev

# Gurobi
wget https://packages.gurobi.com/12.0/gurobi12.0.1_linux64.tar.gz
mv gurobi12.0.1_linux64.tar.gz src/ext
cd src/ext
tar -xvzf gurobi12.0.1_linux64.tar.gz

# Stresstesting
pip3 install tqdm