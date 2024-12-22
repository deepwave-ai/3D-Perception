#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/shape_predictor.h>
#include <dlib/image_processing/face_recognition.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>

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
    dlib::anet_type face_recognizer;
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
            known_face_encoding = face_recognizer.compute_face_descriptor(dlib_frame, shape);
            reference_face_captured = true;
        }

        cv::imshow("Capture Reference Face", frame);
        if (cv::waitKey(1) == 'q') break;
    }

    // EKF initialization
    cv::Mat state = cv::Mat::zeros(3, 1, CV_64F); // [x, y, theta]
    cv::Mat P = cv::Mat::eye(3, 3, CV_64F) * 500;
    cv::Mat R = cv::Mat::diag(cv::Matx31d(0.5, 0.5, cv::deg2rad(5.0)));
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
            dlib::matrix<float, 0, 1> current_face_encoding = face_recognizer.compute_face_descriptor(dlib_frame, shape);

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
                                         std::to_string(cv::degrees(state.at<double>(2)));
                cv::putText(frame, state_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
            } else {
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
