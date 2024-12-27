# FaceRecognition With Particle Filter Tracking

Overview

Face Recognition With Particle Filter Tracking can be used for

(1) Real-time face tracking in dynamic environments
(2) Face-based motion estimation and state tracking
(3) Augmented reality or robotics integration for vision-guided tasks

Implementation

The code implements a particle filter-based face tracking system with face recognition using OpenCV and dlib. Here's what it does:

1. Initialization:

(1) Sets up video capture using OpenCV.

(2) Loads dlib's face detector, shape predictor, and face recognition model.

(3) Initializes a particle filter with 100 particles uniformly distributed across the frame.

2. Face Recognition:

Captures a face encoding of a target individual for identification. If no face encoding exists, it prompts the user to provide one by showing frames from the camera feed.
Particle Filter:

(1) Predicts particle states with random motion noise.

(2) Updates particle weights based on proximity to a detected face.

(3) Resamples particles using systematic resampling to maintain the filter's diversity.

3. Face Tracking:

(1) Detects faces in each frame.

(2) Compares detected faces to the target face encoding using Euclidean distance.

(3) If the face matches the target, it updates the particle filter to estimate the target's position in the frame.

(4) Displays the estimated position and visualizes the particles on the video feed.

5. Output:

Shows the video feed with face bounding boxes, particle visualization, and estimated position.

6. C++ Project
   
6.1 Dlib Lib Linking

6.1.1 Steps to Compile Dlib with Visual Studio

Step 1: Configure the Build with CMake

(1) Open CMake GUI.
(2) Set the following paths:
    Where is the source code: Path to the Dlib source directory (e.g., C:\path\to\dlib). 
   
    Where to build the binaries: Path to a build directory (e.g., C:\path\to\dlib\build).

(3) Click Configure.

    Choose your Visual Studio version and platform (e.g., "Visual Studio 17 2022" and x64).
    
(4) CMake will process and display configurable options.

Step 2: Generate the Project Files
    After configuration is successful, click Generate.
    
    This creates Visual Studio project files in your build directory.

Step 3: Open the Project in Visual Studio

    Open the .sln file in the build directory using Visual Studio.

    Set the build configuration to Release or Debug.
    
Step 4: Compile Dlib

In Visual Studio, build the solution (Ctrl + Shift + B).

This will compile Dlib and generate the necessary .lib and .dll files.

6.1.2 Using Dlib in Your Project

After successfully building Dlib:

(1) Include Directories:

Add the Dlib include directory (e.g., C:\path\to\dlib\include) to your project.

(2) Link Libraries:

Link against the Dlib .lib file generated in the build directory.

Ensure your project's Runtime Library matches Dlib's (e.g., /MD or /MDd).

6.1.2. Common Issues and Solutions

Issue 1: C1128: number of sections exceeded object file format limit

Enable /bigobj in your project's C++ → Command Line settings.

Issue 2: LNK2001: unresolved external symbol

(1) Ensure consistent build configurations (e.g., Runtime Library settings). 
(2) Verify you are linking the correct .lib file
(3) Add dlib_path\dlib\all\source.cpp from the dlib source directory to your project and re-compile it.

6.2 OpenCV Lib Linking

Install opencv-4.10.0-windows.exe and add corresponding include, linker path and precompiled lib to your project.


   
