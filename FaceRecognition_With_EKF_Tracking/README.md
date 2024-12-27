# FaceRecognition With EKF Tracking

Overview

Face Recognition With EKF Tracking can be used for

(1) Real-time face tracking in dynamic environments
(2) Face-based motion estimation and state tracking
(3) Augmented reality or robotics integration for vision-guided tasks

Implementation

1. Libraries Used

cv2 (OpenCV): Handles video capture and image processing.

numpy: Performs numerical operations, including matrix manipulations.

dlib: Provides face detection and facial feature extraction capabilities.

scipy.spatial.distance: Computes distances between vectors, used here for face matching.

2. Face Recognition Setup

Video Capture:
cap = cv2.VideoCapture(0) initializes the webcam for capturing frames.

Dlib Models:

Face Detector: face_detector = dlib.get_frontal_face_detector() detects faces in an image.

Landmark Predictor: shape_predictor predicts 68 facial landmarks (eyes, nose, etc.).

Face Encoder: face_recognizer converts facial features into a numerical vector (encoding).

Encoding Known Faces:

Initially, the script captures one face from the webcam to create a "known face encoding":

known_face_encoding = get_face_encoding(frame, face_detector, shape_predictor, face_recognizer)

This encoding will later be used to compare and recognize the same face.

Face Matching:

The Euclidean distance between the known encoding and a detected face's encoding determines a match:

match_distance = distance.euclidean(known_face_encoding, current_face_encoding)

If match_distance < 0.6, the face is considered recognized.

3. EKF for State Estimation

An Extended Kalman Filter (EKF) estimates the position of the face in a dynamic environment. EKF operates in two steps:

Predict Step:

Predicts the next state based on the current state and motion model:

state = state + u  # Apply control input (motion)

P = F @ P @ F.T + R  # Update covariance

state: Current state [x, y, theta] (position and orientation).

u: Control input [dx, dy, dtheta] (assumed as 0 in this example).

P: Covariance matrix (uncertainty in state).

Update Step:

Updates the prediction using actual measurements:

y = z - predicted_observation[:2]

S = H @ P @ H.T + Q

K = P @ H.T @ np.linalg.inv(S)

state = state + (K @ y)

P = (np.eye(3) - K @ H) @ P

z: Measured position (face center).

H: Measurement model linking state to observations.

K: Kalman gain for optimal state adjustment.

Face Position as Measurement:

The face's bounding box center is used as a measurement:

face_center = np.array([x + w / 2, y + h / 2])

z = face_center


4. Real-Time Processing
In each frame:

Faces are detected, and encodings are compared with the known face.

If matched:

Face position is used in the EKF for updating state.

Estimated state (x, y, theta) is displayed on the screen.

The frame is shown with annotations:

Bounding boxes, match labels, and state information.

5. C++ Project
5.1 Dlib Lib Linking

5.1.1 Steps to Compile Dlib with Visual Studio

Step 1: Configure the Build with CMake

(1) Open CMake GUI. (2) Set the following paths: Where is the source code: Path to the Dlib source directory (e.g., C:\path\to\dlib).

Where to build the binaries: Path to a build directory (e.g., C:\path\to\dlib\build).
(3) Click Configure.

Choose your Visual Studio version and platform (e.g., "Visual Studio 17 2022" and x64).
(4) CMake will process and display configurable options.

Step 2: Generate the Project Files After configuration is successful, click Generate.

This creates Visual Studio project files in your build directory.
Step 3: Open the Project in Visual Studio

Open the .sln file in the build directory using Visual Studio.

Set the build configuration to Release or Debug.
Step 4: Compile Dlib

In Visual Studio, build the solution (Ctrl + Shift + B).

This will compile Dlib and generate the necessary .lib and .dll files.

5.1.2 Using Dlib in Your Project

After successfully building Dlib:

(1) Include Directories:

Add the Dlib include directory (e.g., C:\path\to\dlib\include) to your project.

(2) Link Libraries:

Link against the Dlib .lib file generated in the build directory.

Ensure your project's Runtime Library matches Dlib's (e.g., /MD or /MDd).

5.1.2. Common Issues and Solutions

Issue 1: C1128: number of sections exceeded object file format limit

Enable /bigobj in your project's C++ → Command Line settings.

Issue 2: LNK2001: unresolved external symbol

(1) Ensure consistent build configurations (e.g., Runtime Library settings). (2) Verify you are linking the correct .lib file (3) Add dlib_path\dlib\all\source.cpp from the dlib source directory to your project and re-compile it. Do not copy it to your project path, just add it from dlib source directory.

5.2 OpenCV Lib Linking

Install opencv-4.10.0-windows.exe and add corresponding include, linker path and precompiled lib to your project.

