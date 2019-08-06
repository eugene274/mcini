# mcini
# Data format to store information about initial and final stages of relativistic nucleus-nucleus collisions

HOW TO:

1. build the library:
```
$ cd mcini
$ mkdir build
$ cd build
$ cmake ..
$ make
```
2. set up the environment:
```
$ . ../macro/config.sh
```
3. convert UrQMD \*.f20 files (OSCAR1999A format):
```
root '../macro/convertUrQMD.C ("path/to/input/file","path/to/output/file")'
```
4. run demo analysis macro:
```
root '../macro/demo.C ("path/to/input/file")'
```
