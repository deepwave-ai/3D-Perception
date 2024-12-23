//
// Copyright(c) 2024 deepwave-ai. All Rights Reserved.
//
// orb.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

int main() {
    // Create a VideoCapture object to capture video from the default camera (index 0)
    cv::VideoCapture cap(0);

    // Check if the camera is opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    // Create an ORB detector
    cv::Ptr<cv::ORB> orb = cv::ORB::create();

    while (true) {
        cv::Mat frame;

        // Capture a frame from the camera
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Failed to capture frame." << std::endl;
            break;
        }

        // Convert the frame to grayscale (ORB works on grayscale images)
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Detect keypoints and compute descriptors
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        orb->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);

        // Draw keypoints on the original frame
        cv::Mat frame_with_keypoints;
        cv::drawKeypoints(frame, keypoints, frame_with_keypoints, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        // Display the frame with keypoints
        cv::imshow("ORB with Camera", frame_with_keypoints);

        // Break the loop on pressing 'q'
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Release the camera and close windows
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
