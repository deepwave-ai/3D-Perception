# FaceRecognition With Particle Filter Tracking

Overview

Face Recognition With EKF Tracking can be used for

(1) Real-time face tracking in dynamic environments
(2) Face-based motion estimation and state tracking
(3) Augmented reality or robotics integration for vision-guided tasks

Implementation

code implements a particle filter-based face tracking system with face recognition using OpenCV and dlib. Here's what it does:

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
Exits when the user presses 'q'.