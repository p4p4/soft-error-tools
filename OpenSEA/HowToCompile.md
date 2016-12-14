# How to install:

# tested on CentOs 6.5 / RHEL
## 1 install dependencies (run as root):
`yum install -y gcc gcc-c++ make cmake zlib-devel git glibc-static zlib-static`

## 2 clone repository:
`git clone https://github.com/p4p4/soft-error-tools.git`

## 3 create folder + environment variable for libraries 
`mkdir soft-error-tools/libs/`

```
echo export IMMORTALTP=`pwd`/soft-error-tools/libs >> .bashrc
```

`source ~/.bashrc`

## 4 install the libraries
`cd soft-error-tools/OpenSEA/ext_tools/`

`sh install_all.sh`

## 5 compile OpenSEA
`cd .. && make`

## UBUNTU 
`sudo apt-get update`

`sudo apt-get install -y zlib1g-dev cmake build-essential git`

`git clone https://github.com/p4p4/soft-error-tools.git`

`mkdir soft-error-tools/libs/`

```
echo export IMMORTALTP=`pwd`/soft-error-tools/libs >> .bashrc
```
`source ~/.bashrc`

`cd soft-error-tools/OpenSEA/ext_tools/`

`sh install_all.sh`

`cd .. && make`


## Troubleshoot
*Problem*:
the last step (make in OpenSEA/) fails with the following error message:
`minisat/minisat/mtl/Map.h:32:99: error: missing template arguments before ‘(’ token`
(seems to be a problem with newer gcc versions, eg. gcc version 6.2.0 20161005

*Solution*:
run: `sh ext_tools/patch_minisat.sh`



