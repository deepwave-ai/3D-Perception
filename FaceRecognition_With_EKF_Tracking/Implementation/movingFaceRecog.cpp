//
// Copyright(c) 2024 deepwave-ai. All Rights Reserved.
//
// faceEKFTracking.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/dnn.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>

// Alias for dlib's face recognition model

using namespace dlib;
using namespace std;


template <template <int, template<typename> class, int, typename> class block, int N, template<typename> class BN, typename SUBNET>
using residual = dlib::add_prev1<block<N, BN, 1, dlib::tag1<SUBNET>>>;

template <template <int, template<typename> class, int, typename> class block, int N, template<typename> class BN, typename SUBNET>
using residual_down = dlib::add_prev2<dlib::avg_pool<2, 2, 2, 2, dlib::skip1<dlib::tag2<block<N, BN, 2, dlib::tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block = BN<dlib::con<N, 3, 3, 1, 1, dlib::relu<BN<dlib::con<N, 3, 3, stride, stride, SUBNET>>>>>;

template <int N, typename SUBNET> using ares = dlib::relu<residual<block, N, dlib::affine, SUBNET>>;
template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block, N, dlib::affine, SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
template <typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

using anet_type = dlib::loss_metric<dlib::fc_no_bias<128, dlib::avg_pool_everything<
    alevel0<
    alevel1<
    alevel2<
    alevel3<
    alevel4<
    dlib::max_pool<3, 3, 2, 2, dlib::relu<dlib::affine<dlib::con<32, 7, 7, 2, 2,
    dlib::input_rgb_image_sized<150> >>>>>>>>>>>>;


// EKF Predict Step
void predict(cv::Mat& state, cv::Mat& P, const cv::Mat& u, const cv::Mat& R) {
    state += u;
    P = P + R;
}

// EKF Update Step
void update(cv::Mat& state, cv::Mat& P, const cv::Mat& z, const cv::Mat& H, const cv::Mat& Q) {
    cv::Mat predicted_observation = H * state;
    cv::Mat y = z - predicted_observation(cv::Range(0, 2), cv::Range::all());
    cv::Mat S = H * P * H.t() + Q;
    cv::Mat K = P * H.t() * S.inv();
    state += K * y;
    cv::Mat I = cv::Mat::eye(3, 3, CV_64F);
    P = (I - K * H) * P;
}

int main() {
    // Initialize video capture
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open the camera." << std::endl;
        return -1;
    }

    // Load Dlib models
    dlib::frontal_face_detector face_detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor shape_predictor;
    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> shape_predictor;
    anet_type face_recognizer;
    dlib::deserialize("dlib_face_recognition_resnet_model_v1.dat") >> face_recognizer;

    // Capture a reference face encoding
    dlib::matrix<float, 0, 1> known_face_encoding;
    bool reference_face_captured = false;
    while (!reference_face_captured) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        dlib::cv_image<dlib::bgr_pixel> dlib_frame(frame);
        std::vector<dlib::rectangle> faces = face_detector(dlib_frame);

        if (!faces.empty()) {
            dlib::full_object_detection shape = shape_predictor(dlib_frame, faces[0]);
            // Align face and create a face chip
            matrix<rgb_pixel> face_chip;
            extract_image_chip(dlib_frame, get_face_chip_details(shape, 150, 0.25), face_chip);

            known_face_encoding = face_recognizer(face_chip);
            reference_face_captured = true;
        }

        cv::imshow("Capture Reference Face", frame);
        if (cv::waitKey(1) == 'q') break;
    }

    // EKF initialization
    cv::Mat state = cv::Mat::zeros(3, 1, CV_64F); // [x, y, theta]
    cv::Mat P = cv::Mat::eye(3, 3, CV_64F) * 500;

    cv::Mat R = (cv::Mat_<double>(3, 3) << 0.5, 0, 0,
        0, 0.5, 0,
        0, 0, 5.0 * CV_PI / 180.0);

    cv::Mat Q = cv::Mat::eye(2, 2, CV_64F) * 2;

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        dlib::cv_image<dlib::bgr_pixel> dlib_frame(frame);
        std::vector<dlib::rectangle> faces = face_detector(dlib_frame);

        for (const auto& face : faces) {
            int x = face.left();
            int y = face.top();
            int w = face.width();
            int h = face.height();
            cv::rectangle(frame, cv::Rect(x, y, w, h), cv::Scalar(255, 0, 0), 2);

            dlib::full_object_detection shape = shape_predictor(dlib_frame, face);
            matrix<rgb_pixel> face_chip;
            extract_image_chip(dlib_frame, get_face_chip_details(shape, 150, 0.25), face_chip);

            dlib::matrix<float, 0, 1> current_face_encoding = face_recognizer(face_chip);

            double match_distance = dlib::length(current_face_encoding - known_face_encoding);
            if (match_distance < 0.6) {
                cv::putText(frame, "Matched", cv::Point(x, y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

                cv::Mat face_center = (cv::Mat_<double>(2, 1) << x + w / 2.0, y + h / 2.0);

                // EKF Predict and Update
                cv::Mat u = cv::Mat::zeros(3, 1, CV_64F); // No control input
                predict(state, P, u, R);
                cv::Mat H = cv::Mat::eye(2, 3, CV_64F);
                update(state, P, face_center, H, Q);

                std::string state_text = "State: x=" + std::to_string(state.at<double>(0)) + ", y=" +
                    std::to_string(state.at<double>(1)) + ", theta=" +
                    std::to_string(state.at<double>(2) * 180.0 / CV_PI);

                cv::putText(frame, state_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
            }
            else {
                cv::putText(frame, "Unknown", cv::Point(x, y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
            }
        }

        cv::imshow("EKF-SLAM with Face Recognition", frame);
        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
