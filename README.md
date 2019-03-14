# Bug's Life

Course: EPFL - CS 112 - Programming II (spring semester 2016/2017)

Project: a 2D simulation of anthills competing for food 🐜

Partner: Florian Hartmann

![Alt](pic_0.PNG)

## Behavior and rules of the simulation

The rules set for the simulations (behavior of the ants, population evolution, food generation/consumption, anthill size, etc) can be found in 

## Simulation text files format

WIP

## User manual (Linux)

First install the following libraries:

- GLUT: use `sudo apt-get install freeglut3 freeglut3-dev`
- GLUI: download `libglui2c2` and `libglui` (check [here](http://www.rpmseek.com/index.html?hl=com)), then use `dpkg -i <package name>`.

Then you should be able to compile in source folder using `make`. Many test files are available in the test_files folder. Most of them are meant to trigger a specific error scenario. To get a a working simulation try `F04.txt` or build your own simulation text file! 

The programme has 4 working modes:

- Error (for grading purposes only)
- Verification (for grading purposes only)
- Graphic (for grading purposes only)
- Final ✔️

`./bugslife.x [Error|Verification|Graphic|Final, F04.txt]`

The default mode is Final.

Through the UI you can open a simulation text file, save the current state of the simulation, start/stop the simulation, record the simulation and control food generation. The record files (`.dat`) can be plotted with `gnuplot` using `plot "out.dat" using 1:2 with lines,"out.dat" using 1:3 with lines, "out.dat" using 1:4 with lines,"out.dat"`

![Alt](pic_3.png)

