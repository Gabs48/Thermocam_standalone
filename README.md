Thermocam_standalone
====================

Stand alone acquisition software for Thermocam FLIR A5

- Go to inside the Thermocam_standalone folder and create a build folder:
```
mkdir build
```

- Go inside the build folder and create the makefile:
```
cd build
cmake ..
```

- Compile the program:
```
make
```

- Plug the ethernet Thermocam, change your IP to a static 192.168.1.1 (no need to change anything else) and launch the soft:
```
./Main
```

- By default images are saved in *build/img*, calibration matrix in *build* and timestamp data in *build*
