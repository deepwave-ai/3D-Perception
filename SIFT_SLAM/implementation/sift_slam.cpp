#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <vector>

// Function to compute matches using FLANN-based matcher
std::vector<cv::DMatch> computeMatches(const cv::Mat& descriptors1, const cv::Mat& descriptors2) {
    const int FLANN_INDEX_KDTREE = 1;
    cv::Ptr<cv::flann::IndexParams> indexParams = cv::makePtr<cv::flann::KDTreeIndexParams>(5);
    cv::Ptr<cv::flann::SearchParams> searchParams = cv::makePtr<cv::flann::SearchParams>(50);
    cv::FlannBasedMatcher flann(indexParams, searchParams);

    std::vector<std::vector<cv::DMatch>> knnMatches;
    flann.knnMatch(descriptors1, descriptors2, knnMatches, 2);

    // Filter matches using Lowe's ratio test
    std::vector<cv::DMatch> goodMatches;
    for (const auto& knnMatch : knnMatches) {
        if (knnMatch.size() >= 2 && knnMatch[0].distance < 0.7f * knnMatch[1].distance) {
            goodMatches.push_back(knnMatch[0]);
        }
    }
    return goodMatches;
}

// Function to estimate camera pose (rotation and translation)
void findPose(const std::vector<cv::DMatch>& matches, const std::vector<cv::KeyPoint>& keypoints1, const std::vector<cv::KeyPoint>& keypoints2, const cv::Mat& K, cv::Mat& R, cv::Mat& t) {
    std::vector<cv::Point2f> pts1, pts2;
    for (const auto& match : matches) {
        pts1.push_back(keypoints1[match.queryIdx].pt);
        pts2.push_back(keypoints2[match.trainIdx].pt);
    }

    cv::Mat mask;
    cv::Mat E = cv::findEssentialMat(pts1, pts2, K, cv::RANSAC, 0.999, 1.0, mask);

    cv::recoverPose(E, pts1, pts2, K, R, t, mask);
}

// Main Visual SLAM function
void visualSLAM(const std::string& videoPath, const cv::Mat& K) {
    cv::VideoCapture cap(videoPath == "0" ? 0 : videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open video." << std::endl;
        return;
    }

    // Initialize SIFT detector
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();

    cv::Mat prevFrame, prevDescriptors;
    std::vector<cv::KeyPoint> prevKeypoints;

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            break;
        }

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Detect SIFT keypoints and descriptors
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        sift->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);

        if (!prevFrame.empty()) {
            // Match features with the previous frame
            std::vector<cv::DMatch> matches = computeMatches(prevDescriptors, descriptors);

            // Estimate pose
            cv::Mat R, t;
            findPose(matches, prevKeypoints, keypoints, K, R, t);

            // Display matches
            cv::Mat matchImg;
            cv::drawMatches(prevFrame, prevKeypoints, frame, keypoints, matches, matchImg, cv::Scalar::all(-1), cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
            cv::imshow("Feature Matches", matchImg);

            std::cout << "Rotation Matrix:\n" << R << std::endl;
            std::cout << "Translation Vector:\n" << t << std::endl;
        }

        prevFrame = frame.clone();
        prevKeypoints = keypoints;
        prevDescriptors = descriptors;

        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
}

int main() {
    // Camera intrinsic parameters (example values)
    cv::Mat K = (cv::Mat_<double>(3, 3) << 718.856, 0, 607.1928, 0, 718.856, 185.2157, 0, 0, 1);

    // Path to video or live camera feed (use "0" for default camera)
    std::string videoPath = "0"; // Replace with your video path or "0" for live feed
    
    visualSLAM(videoPath, K);

    return 0;
}
