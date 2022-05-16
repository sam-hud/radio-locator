# radio-locator
## About this project
The radio locator is a research project investigating the function of an RSSI (received signal strength indication)
positioning and navigation system.

## Hardware requirements
The system requires 3 beacon nodes, a target node and a navigation node. All nodes are based on the TTGO LoRa32 v1.0 boards.
The navigation node also has a QMC5883L compass module to assist with navigation.

## Setup
Platform IO on VSCode is used for programming of the hardware. The beacon nodes are programmed with the beacon_node code.
Before programming each node, the node ID must be set. The beacon nodes take the numbers 1, 2, and 3. Keep track of which
node is assigned which number.
The target node is programmed with the target_node code. This device takes the ID of 0.
Finally, the navigation node is programmed with the navigation_node code. This device takes the ID of 4. The compass may
need calibrating. Please see the QMC5883L repo for more info on this.

Each beacon device should be placed spaced out in an open area. The locations of each node should be noted on a satellite map.
A grid should be drawn over the locations, with the y-axis aligned with North. The location of each beacon should then be calculated from this gridded map. The locations of each beacon can then be placed into the navigation_node system variables.

RSSI distance calibration will be needed. This can be quite an intensive process. Using R1m values of around -60, and Cpl values of between 2 and 3 may be sufficient.

## Operation
With the target node placed within range of all 3 spaced out beacons, the user can stand in a different location (within range of the beacons still) with the navigation node. When the user button on the navigation node is pressed, a series of pings will be sent from the navigation node, and then to the target node. After a short amount of time, the results should show on the display, with a line pointing in the calculated bearing direction of the target node, relative to the direction that the navigation node is pointing. The button can be pressed again to recalculate this direction.

Note: QMC5883L does not contain an accelerometer and thus no tilt compensation is included. The device must be kept flat (board parallel to the ground) for the compass to give an accurate reading.