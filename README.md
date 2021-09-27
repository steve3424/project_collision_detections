## Project Collision Detections
This is the 2nd project from MIT OCW 6.172 [Performance Engineering of Software Systems 6.712](https://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-172-performance-engineering-of-software-systems-fall-2018/)

The screensaver consists of a number of lines moving and bouncing off one another and the walls. The goal of the project was to speed up the collision detection algorithm from the standard n^2 search by implementing a quad tree.

All of the code other than the code in /quad_tree is starter code from the course. My addition is in /quad_tree.

### To Run

This can be run with or without graphics. There is a shell script for each

/run_tests.sh

You can specify the number of frames and the input file in the shell script.

/run_graphics.sh

This will display the screensaver. For windows, this can work with Windows Subsystem Linux. You need to use an xserver such as [Xming](https://sourceforge.net/projects/xming/). Run "export DISPLAY=:0" or add to .bashrc. Start the xserver and run the "run_graphics.sh". By default it runs without the quad_tree enabled. Press 'q' to enable the quad_tree. With quad_tree enabled, press 'v' for a visualization.
