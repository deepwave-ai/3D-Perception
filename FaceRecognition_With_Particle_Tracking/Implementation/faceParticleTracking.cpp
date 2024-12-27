//
// Copyright(c) 2024 deepwave-ai. All Rights Reserved.
//
// faceParticleTracking.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/dnn.h>
#include <random>
#include <vector>
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

// Particle Filter Parameters
const int num_particles = 100;
std::vector<cv::Point2f> particles(num_particles);
std::vector<float> particle_weights(num_particles, 1.0f / num_particles);

// Particle Filter Functions
void predict_particles(std::vector<cv::Point2f>& particles, float motion_noise) {
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::normal_distribution<float> dist(0.0f, motion_noise);

    for (auto& particle : particles) {
        particle.x += dist(gen);
        particle.y += dist(gen);
    }
}

void update_particles(const std::vector<cv::Point2f>& particles, std::vector<float>& weights,
    const cv::Point2f& measurement, float measurement_noise) {
    for (size_t i = 0; i < particles.size(); ++i) {
        float distance = cv::norm(particles[i] - measurement);
        weights[i] *= exp(-distance * distance / (2 * measurement_noise * measurement_noise));
    }
    float sum_weights = std::accumulate(weights.begin(), weights.end(), 0.0f);
    for (auto& weight : weights) weight /= sum_weights; // Normalize weights
}

void resample_particles(std::vector<cv::Point2f>& particles, std::vector<float>& weights) {
    std::vector<cv::Point2f> new_particles(particles.size());
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<float> cumulative_weights(weights.size());
    std::partial_sum(weights.begin(), weights.end(), cumulative_weights.begin());

    for (size_t i = 0; i < particles.size(); ++i) {
        float r = dist(gen);
        auto it = std::lower_bound(cumulative_weights.begin(), cumulative_weights.end(), r);
        new_particles[i] = particles[it - cumulative_weights.begin()];
    }
    particles = new_particles;
    std::fill(weights.begin(), weights.end(), 1.0f / particles.size());
}


// ResNet Definitions (as previously defined)
// Main Function

int main() {
    // Initialize video capture
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open the camera.\n";
        return -1;
    }

    // Initialize dlib models
    frontal_face_detector face_detector = get_frontal_face_detector();
    shape_predictor shape_predictor;
    deserialize("shape_predictor_68_face_landmarks.dat") >> shape_predictor;

    anet_type face_recognizer;
    deserialize("dlib_face_recognition_resnet_model_v1.dat") >> face_recognizer;

    // Initialize particles
    for (auto& particle : particles) {
        particle.x = std::rand() % 640;
        particle.y = std::rand() % 480;
    }

    // Main loop
    while (cap.isOpened()) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        // Convert frame to dlib format
        cv_image<bgr_pixel> dlib_frame(frame);

        // Detect faces
        std::vector<rectangle> faces = face_detector(dlib_frame);
        if (!faces.empty()) {
            for (const auto& face : faces) {
                // Get face landmarks
                full_object_detection shape = shape_predictor(dlib_frame, face);

                // Align face and create a face chip
                matrix<rgb_pixel> face_chip;
                extract_image_chip(dlib_frame, get_face_chip_details(shape, 150, 0.25), face_chip);
                // Get face encoding
                matrix<float, 0, 1> current_face_encoding = face_recognizer(face_chip);

                // Known encoding comparison (placeholder)
                float match_distance = dlib::length(current_face_encoding);

                // Display descriptor size
                cout << "Face descriptor size: " << current_face_encoding.size() << "Match distance: "<< match_distance << endl;

                if (match_distance < 1.5) {
                    // Face matches: Update particle filter
                    cv::Point2f face_center(face.left() + face.width() / 2.0f, face.top() + face.height() / 2.0f);
                    predict_particles(particles, 10.0f);
                    update_particles(particles, particle_weights, face_center, 50.0f);
                    resample_particles(particles, particle_weights);

                    // Estimate position
                    cv::Point2f estimated_position(0, 0);
                    for (size_t i = 0; i < particles.size(); ++i) {
                        estimated_position += particles[i] * particle_weights[i];
                    }

                    // Draw particles
                    for (const auto& particle : particles) {
                        cv::circle(frame, particle, 1, cv::Scalar(0, 255, 0), -1);
                    }

                    // Display estimated position
                    cv::circle(frame, estimated_position, 5, cv::Scalar(0, 255, 255), -1);
                }

                // Draw face bounding box
                cv::rectangle(frame, cv::Rect(face.left(), face.top(), face.width(), face.height()),
                    cv::Scalar(255, 0, 0), 2);
            }
        }

        // Display the frame
        cv::imshow("Particle Filter Face Tracking", frame);
        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
