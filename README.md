Plog Converter
===============================

To convert the analyzer bug report to different formats (xml, tasks and so on) you can use the Plog Converter.
It is applicable for cross-platform working scenarios (C++, Java) on all supported operating systems when checking Makefile, CMake, QMake, Ninja, WAF projects.

More detailed description is available on the [documentation page](https://www.viva64.com/en/m/0036/), section "Plog Converter Utility".

Compilation
--------------

You should use g++ (versions 5.4 or newer) or clang++ (versions 3.8 or newer) and CMake to compile the utility. No additional libraries are required.

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8
sudo make install
```

Compiling plog-converter with Clang:

```
mkdir build
cd build
CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8
sudo make install
```

Usage
-------------

An example of a command that will be suitable for most users for opening the report in QtCreator:

```
plog-converter -a GA:1,2 -t tasklist -o /path/to/project.tasks /path/to/project.log
```
