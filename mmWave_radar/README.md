Getting Started with IWR6843AOP in ROS 2

This README provides step-by-step instructions to use the Texas Instruments IWR6843AOP mmWave Sensor with ROS 2. The guide outlines hardware setup, software installation, and integration with ROS 2 for visualizing radar data and using it in robotics applications.

Overview

The IWR6843AOP is an integrated mmWave radar sensor designed for object detection, tracking, and environment sensing. Its compatibility with ROS 2 allows seamless integration into robotics workflows, enabling applications such as obstacle avoidance, SLAM, and perception in autonomous systems.

Prerequisites

Hardware Requirements

1. TI IWR6843AOP Sensor
2. MMWave EVM Development Kit
3. USB Cable (for data and power)
4. Host Computer with: Ubuntu 20.04 or newer
5. Ethernet or Serial Ports

Software Requirements

1. ROS 2 (Foxy, Galactic, Humble, or newer)
2. TI mmWave ROS Driver (supports ROS 2)
3. TI Uniflash (for firmware updates)

Step 1: Hardware Setup

1. Connect the Sensor:

(1) Use the USB cable to connect the IWR6843AOP sensor to your host computer.
(2) Ensure the sensor's DIP switches are configured correctly for functional mode (refer to the sensor's user guide).

2. Power On:

Plug the sensor into your computer, and verify that the power LED on the device lights up.

3. Verify USB Connection:

Confirm that the sensor is detected by the host system:

ls /dev/ttyUSB*

or 

ls /dev/ttyACM*

if mmWave EVM MMWAVEICBOOST is used. For ICBOOST, make sure the SOP2 jumper on the TI mmWave EVM has been removed or switched to the off position and then power it using the 5VDC power supply (which must be rated for at least 2.5Amps) and connect the EVM via USB (port labeled “XDS110 USB” on the MMWAVEICBOOST) to the host machine.

You should see two USB devices, e.g., /dev/ttyUSB0 (configuration port) and /dev/ttyUSB1 (data port).
or /dev/ttyACM0 and /dev/ttyACM1.

Step 2: Software Setup

1. Install ROS 2
Follow the official ROS 2 installation guide for your distribution:

ROS 2 Installation Guide

2. Install Dependencies
Install required packages:

sudo apt update
sudo apt install git python3-colcon-common-extensions

4. Clone and Build the TI mmWave ROS Driver
Clone the driver repository into your ROS 2 workspace:

mkdir -p ~/ros2_ws/src
cd ~/ros2_ws/src
git clone https://github.com/TI-mmWave-Radar/ti_mmwave_ros2.git

Build the workspace:

cd ~/ros2_ws
colcon build
source install/setup.bash

Step 3: Configure the Sensor

1. Flash Firmware
Use TI Uniflash to flash the appropriate firmware onto the IWR6843AOP:

(1) Download and install TI Uniflash.
(2) Flash the mmWave Demo binary to the device.

2. Set Sensor Configuration
Create a configuration file (e.g., sensor_config.cfg) tailored to your application. Example parameters:

profileCfg 0 60.0 5.0 3 100.0 0 0 10.0 1 256 3000
chirpCfg 0 0 0 0 0 0 0 1
frameCfg 0 0 64 0 100

Step 4: Launching the ROS 2 Driver

Run the Sensor Node: Launch the ROS 2 driver to interface with the IWR6843AOP:

ros2 launch ti_mmwave_ros2_driver ti_mmwave_node.launch.py config_file:=/path/to/sensor_config.cfg

Verify Data: Use rviz2 to visualize radar point clouds:

rviz2

or just type:
ros2 launch ti_mmwave_rospkg 6843AOP_tracking.py

with this launch, Rviz window will be opened automatically.

Add a PointCloud2 display.
Set the topic to /ti_mmwave/radar_scan.

Step 5: Recording Data (Optional)
To save radar data for later analysis, use ROS 2's bagging feature:

ros2 bag record /ti_mmwave/radar_scan

Troubleshooting

Common Issues

No Data in RViz:

Ensure the correct serial ports are being used.
Verify the sensor configuration file matches the hardware setup.
Driver Crashes on Startup:

Check for conflicting serial port access.
Ensure the sensor is in functional mode (not flashing mode).

Firmware Update Required:

Re-flash the firmware using TI Uniflash and confirm the version matches the driver requirements.

Additional Resources

(1) TI mmWave ROS Driver Repository

(2) IWR6843AOP Datasheet

(3) ROS 2 Documentation
