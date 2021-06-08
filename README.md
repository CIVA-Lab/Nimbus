# Nimbus

Cross-platform point cloud visualization tool created by Dr. Joshua Fraser at the University of Missouri.

## Features

<img src="http://meru.cs.missouri.edu/~jbf8cf/github/assets/nimbus/images/Nimbus.png" alt="Nimbus Logo" align="right" />

- Large interactive point cloud visualization
- Cross-platform: macOS, Linux, Windows
- PLY file support and point cloud generation
- Optional support for LAS/LAZ files
- Data caching to GPU using OpenGL VBOs
- Editable camera paths for playback
- Hardware and anaglyph stereo support
<!-- ![Available panels](https://user-images.githubusercontent.com/69218608/106373162-5b8bac80-633c-11eb-814c-443d74bc664f.png) -->

## Screenshots

![Image of Nimbus showing point cloud of Columbia, MO](http://meru.cs.missouri.edu/~jbf8cf/github/assets/nimbus/images/Nimbus-Columbia-small.png)
> Nimbus displaying a point cloud of Columbia, MO containing over 52 million points

![Screenshot of Nimbus showing Los Angeles, CA point cloud in anaglyph stereo](http://meru.cs.missouri.edu/~jbf8cf/github/assets/nimbus/images/Nimbus-LA-Stereo-small.png)
> Nimbus displaying point cloud of Los Angeles, CA in anaglyph stereo.  Point cloud contains over 31 million points.

## Optional LAS/LAZ Support

Optional support for reading LAS and LAZ files is provided by [LASTools](https://github.com/LAStools/).
The libLAS library and headers should be installed in `3rdpary/src/` as shown
below.

### Install LASTools libLAS libraries:

```bash
git clone https://github.com/LAStools/LAStools.git 3rdparty/src/LASTools
mkdir build; cd build
cmake -DCMAKE_CXX_STANDARD=11 -DCMAKE_INSTALL_PREFIX=../3rdparty ../3rdparty/src/LASTools
make -j; make install
```

After installing the LASTools libraries, the `build` and `3rdparty/src` and
`3rdparty/bin` directories can be removed.

## Compiling Nimbus

Without LAS/LAZ support:

```
qmake CONFIG+=release
make -j
```

With optional LAS/LAZ support (requires libLAS installation above):

```
qmake CONFIG+=release READERS+=lastools
make -j
```

## Contact

Created by: Joshua Fraser  
[Computational Imaging and Visanalysis (CIVA) Lab](http://cell.missouri.edu)  
Department of Electrical Engineering and Computer Science 
University of Missouri-Columbia  

For more information, contact:

* **Dr. Joshua Fraser**  
226 Naka Hall (EBW)  
University of Missouri-Columbia  
Columbia, MO 65211  
jbf8cf@missouri.edu  

* **Dr. F. Bunyak**  
219 Naka Hall (EBW)  
University of Missouri-Columbia  
Columbia, MO 65211  
bunyak@missouri.edu  

* **Dr. K. Palaniappan**  
205 Naka Hall (EBW)  
University of Missouri-Columbia  
Columbia, MO 65211  
palaniappank@missouri.edu 

Copyright © 2013-2021 Joshua Fraser and Prof. K. Palaniappan and Curators of the University of Missouri, a public corporation. All Rights Reserved.

## Credits

Nimbus uses the following open source packages:

- [Qt Toolkit](www.qt.io)
- [LASTools](https://github.com/LAStools/)
- [libQGLViewer](http://libqglviewer.com)
- [rply](http://w3.impa.br/~diego/software/rply/)

