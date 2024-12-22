//
// Copyright(c) 2024 deepwave-ai. All Rights Reserved.
//
// aStarPathPlanning.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <set>
#include <utility>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace std;

struct Node {
    pair<int, int> position;
    int g; // Cost from start to current node
    int f; // Total estimated cost (g + h)
    vector<pair<int, int>> path;

    bool operator>(const Node& other) const {
        return f > other.f;
    }
};

int heuristic(pair<int, int> a, pair<int, int> b) {
    // Manhattan distance
    return abs(a.first - b.first) + abs(a.second - b.second);
}

vector<pair<int, int>> astar(const vector<vector<int>>& grid, pair<int, int> start, pair<int, int> goal) {
    vector<pair<int, int>> neighbors = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} }; // 4-connected grid
    priority_queue<Node, vector<Node>, greater<Node>> open_list;
    set<pair<int, int>> closed_set;

    open_list.push({ start, 0, heuristic(start, goal), {start} });

    while (!open_list.empty()) {
        Node current = open_list.top();
        open_list.pop();

        if (closed_set.find(current.position) != closed_set.end()) {
            continue;
        }

        closed_set.insert(current.position);

        if (current.position == goal) {
            return current.path; // Return path when goal is reached
        }

        for (const auto& neighbor : neighbors) {
            int dx = neighbor.first;
            int dy = neighbor.second;

            int neighbor_x = current.position.first + dx;
            int neighbor_y = current.position.second + dy;

            // Ensure the neighbor is within grid bounds
            if (neighbor_x >= 0 && neighbor_x < grid.size() &&
                neighbor_y >= 0 && neighbor_y < grid[0].size()) {

                // Ensure the neighbor is not an obstacle and hasn't been visited
                if (grid[neighbor_x][neighbor_y] == 1 || closed_set.find({ neighbor_x, neighbor_y }) != closed_set.end()) {
                    continue;
                }

                vector<pair<int, int>> new_path = current.path;
                new_path.push_back({ neighbor_x, neighbor_y });
                int g = current.g + 1;
                int f = g + heuristic({ neighbor_x, neighbor_y }, goal);

                open_list.push({ {neighbor_x, neighbor_y}, g, f, new_path });
            }
        }
    }

    return {}; // Return empty vector if no path is found
}


int main() {
    // Define grid, start, and goal
    vector<vector<int>> grid(10, vector<int>(10, 0));
    for (int i = 3; i < 8; ++i) {
        grid[i][5] = 1; // Add an obstacle
    }

    pair<int, int> start = { 0, 0 };
    pair<int, int> goal = { 9, 9 };

    // Run A* and retrieve the path
    vector<pair<int, int>> path = astar(grid, start, goal);

    // Visualization using OpenCV
    cv::Mat image = cv::Mat::zeros(grid.size(), grid[0].size(), CV_8UC3);
    for (int i = 0; i < grid.size(); ++i) {
        for (int j = 0; j < grid[0].size(); ++j) {
            if (grid[i][j] == 1) {
                image.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0); // Black for obstacles
            }
            else {
                image.at<cv::Vec3b>(i, j) = cv::Vec3b(255, 255, 255); // White for free space
            }
        }
    }

    if (!path.empty()) {
        std::cout << "Path coordinates:" << std::endl;
        for (const auto& coordinate : path) {
            std::cout << "(" << coordinate.first << ", " << coordinate.second << ")" << std::endl;

            int y = coordinate.first;  // Row (y)
            int x = coordinate.second; // Column (x)
            if (y >= 0 && y < image.rows && x >= 0 && x < image.cols) {
                image.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 255); // Red for path
            }
        }
    }

    image.at<cv::Vec3b>(start.first, start.second) = cv::Vec3b(0, 255, 0); // Green for start
    image.at<cv::Vec3b>(goal.first, goal.second) = cv::Vec3b(255, 0, 0);   // Blue for goal

    cv::resize(image, image, cv::Size(500, 500), 0, 0, cv::INTER_NEAREST);
    cv::imshow("A* Pathfinding", image);
    cv::waitKey(0);
}
