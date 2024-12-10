
ESP32 Camera Car

Hardware is based on LAFVIN Robot Starter Kit Project with ESP32 Module and Code Learning Develop Skill Full Version Smart Car Kit Compatible with Arduino IDE.

https://www.amazon.com/dp/B0CYT8LLDL?ref=ppx_yo2ov_dt_b_fed_asin_title

The Arduino sketch implements a web-controlled robot car with camera streaming using the ESP32. Here's a breakdown of the components:

1. Includes and Setup:

(1) esp_camera.h: For ESP32 camera control.

(2) WiFi.h, AsyncTCP.h, ESPAsyncWebServer.h: Libraries to set up Wi-Fi and an asynchronous web server.

(3) Various structures (MOTOR_PINS) and constants define motor control and camera pins.

2. Camera Initialization:

(1) The setupCamera() function configures the ESP32 camera, initializing pins and settings.

(2) Images are captured and sent to a client through WebSocket.

3. Motor Control:

(1) Two motors are controlled using rotateMotor and moveCar functions.

(2) GPIO pins are toggled to control motor direction and speed.

(3) PWM (Pulse Width Modulation) controls motor speed and light brightness.

4. Web Server and WebSocket:

(1) An asynchronous web server (AsyncWebServer) hosts an HTML control page.

(2) WebSockets (AsyncWebSocket) handle communication between the ESP32 and the client:

  /Camera: Sends captured camera frames.
  /CarInput: Processes user inputs for motor and light control.

5. Main Functions:

(1) setup(): Configures pins, initializes Wi-Fi, sets up the web server, and initializes the camera.

(2) loop(): Periodically cleans up WebSocket clients and sends camera frames.

6. Key RTOS Behavior:

(1) The loop() function is called repeatedly as part of the FreeRTOS scheduler.

(2) Background tasks, like Wi-Fi handling and WebSocket processing, are managed by FreeRTOS.

7. How to build:
   
(1) Use Arduino IDE to open Camera_Car folder;

(2) Include library with Zipped file to select AsyncTCP-master.zip and ESPAsyncWebServer-master.zip.

(3) Load to ESP32 board with a serical port (usually a USB COM port).


Motor Control

The L298N motor driver has a two terminal block in each side for each motor. OUT1 and OUT2 at the left and OUT3 and OUT4 at the right.

OUT1: DC motor A + terminal

OUT2: DC motor A – terminal

OUT3: DC motor B + terminal

OUT4: DC motor B – terminal

There are four input pins and two enable terminals. The input pins are used to control the direction of your DC motors, and the enable pins are used to control the speed of each motor.

IN1: Input 1 for Motor A

IN2: Input 2 for Motor A

IN3: Input 1 for Motor B

IN4: Input 2 for Motor B

EN1: Enable pin for Motor A

EN2: Enable pin for Motor B
Enable pins

The enable pins are like an ON and OFF switch for your motors. For example:

If you send a HIGH signal to the enable 1 pin, motor A is ready to be controlled and at the maximum speed;

If you send a LOW signal to the enable 1 pin, motor A turns off;

If you send a PWM signal, you can control the speed of the motor. The motor speed is proportional to the duty cycle.

However, note that for small duty cycles, the motors might not spin, and make a continuous buzz sound.

Input pins

The input pins control the direction the motors are spinning. Input 1 and input 2 control motor A, and input 3 and 4 control motor B. 

If you apply LOW to input1 and HIGH to input 2, the motor will spin forward;

If you apply power the other way around: HIGH to input 1 and LOW to input 2, the motor will rotate backwards. Motor B can be controlled using the same method but applying HIGH or LOW to input 3 and input 4.

For example, if you want your robot to move forward, both motors should be rotating forward. To make it go backwards, both should be rotating backwards.

To turn the robot in one direction, you need to spin the opposite motor faster. For example, to make the robot turn right, enable the motor at the left, and disable the motor at the right. The following table shows the input pins’ state combinations for the robot directions.

The ESP32-CAM

The ESP32-CAM is based upon the ESP32-S module, so it shares the same specifications.	It has the following features:

(1) 802.11b/g/n Wi-Fi

(2) Bluetooth 4.2 with BLE

(3) UART, SPI, I2C and PWM interfaces

(4) Clock speed up to 160 MHz

(5) Computing power up to 600 DMIPS

(6) 520 KB SRAM plus 4 MB PSRAM

(7) Supports WiFi Image Upload

(8) Multiple Sleep modes

(9) Firmware Over the Air (FOTA) upgrades possible

(10) 9 GPIO ports,Built-in Flash LED

![image](https://github.com/user-attachments/assets/d1d00723-410b-4dbf-8f32-e6e927c25d77)

Camera Sensor (OV2640)

The OV2640 sensor used in ESP32-CAM can output data in various formats, such as JPEG, RGB565, YUV422, etc., but it does not support hardware-accelerated H.264 encoding. The OV2640 can output JPEG-compressed images directly, which are then streamed as a series of frames. This method is lightweight and well-suited to the ESP32's capabilities, but it results in larger files compared to H.264 since MJPEG does not use inter-frame compression.


