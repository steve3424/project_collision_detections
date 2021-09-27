## Project Collision Detections
This is the 2nd project from MIT OCW 6.172 [Performance Engineering of Software Systems 6.712](https://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-172-performance-engineering-of-software-systems-fall-2018/)

The screensaver consists of a number of lines moving and bouncing off one another and the walls. The goal of the project was to speed up the collision detection algorithm from the standard n^2 search by implementing a quad tree.

All of the code other than the code in /quad_tree is starter code from the course. My addition is in /quad_tree.

### To Run

The following instructions are how to run the program on Windows Subsystem for Linux version 1 Ubuntu 20.04.

**To build:**

It is built using clang following gnu99 standard. It links to X11 library to run the graphics.

/build.sh - Shell script to run build command. There is also a makefile that was provided with the starter code, but everything I have done has used the build script so I can't guarantee if the makefile works properly. 

**Run without graphics:**


The outfile name is a.out by default. The '-q' option enables the quad_tree to be used over the default algorithm. Input files are contained in /input.

Example commands:

```
./a.out    500 "beaver.in"
./a.out -q 500 "koch.in"
sh run_tests.sh
```

**Run with graphics:**

First you have to run "export DISPLAY=:0" on the subsystem or add this to .bashrc. Next start an xserver such as [Xming](https://sourceforge.net/projects/xming/). Then run the same commands as above with '-g' option.

This also runs without the quad tree by default. Press 'q' once it is running to enable the quad tree. You should see a circle around the mouse arrow once quad tree is enabled. With quad tree enabled, press 'v' to see a visualization of the tree. Press space bar to pause.

Example commands:

```
./a.out -g 500 "koch.in"
sh run_graphics.sh
```
