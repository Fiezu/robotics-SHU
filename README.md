# Robotics-SHU (Assessment)
This project presents a robotic exploration within a maze, designed to simulate a network of streets, with the objective of identifying multiple designated points, referred to as "houses", without the necessity to return to its initial starting position. The robot operates on a white surface, with the maze boundaries depicted by black lines, demonstrating its capability to navigate through complex paths.

## Zumo 32U4 Setup Guide
### Installation

1. Open Arduino IDE, go to `Tools > Board > Boards Manager`, search for "Arduino AVR Boards", and click "Install".
2. Access `Sketch > Include Library > Manage Libraries`, find "Zumo32U4", select it, and hit "Install". If asked, choose "Install all" for dependencies.

### Uploading Code

1. Connect Zumo via USB, select "Arduino Leonardo" under `Tools > Board`.
2. Under `Tools > Port`, pick the port with 'Serial USB'.
3. Press "Upload" to transfer your sketch.

Refer to [Arduino website](https://www.arduino.cc/en/Guide/Libraries) for more.

### Starting The Exploration
1. Switch ON the robot.
2. Press the button A to start navigating through the maze.
