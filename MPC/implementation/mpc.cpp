#include <Eigen/Dense>
#include <qpOASES.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>

using namespace Eigen;
using namespace qpOASES;

int main() {
    const double dt = 0.1;  // Time step

    // System dynamics
    Matrix2d A;
    A << 1, dt, 0, 1;

    Vector2d B;
    B << 0, dt;

    const int n = 2;  // Number of states
    const int m = 1;  // Number of control inputs

    // MPC parameters
    const int N = 10;  // Prediction horizon
    Vector2d x_goal;
    x_goal << 10, 0;  // Target state

    // Cost matrices
    Matrix2d Q = Matrix2d::Identity();  // State cost matrix
    MatrixXd R = 0.1 * MatrixXd::Identity(m, m);  // Control input cost matrix

    // Constraints
    const double u_min = -2, u_max = 2;

    // Initial state
    Vector2d x;
    x << 0, 0;

    // Store state and control trajectories
    std::vector<Vector2d> x_traj;
    std::vector<double> u_traj;
    x_traj.push_back(x);

    // Total simulation time steps
    const int T = 50;

    for (int t = 0; t < T; ++t) {
        // Set up QP problem
        MatrixXd H = MatrixXd::Zero(N * (n + m), N * (n + m));
        VectorXd g = VectorXd::Zero(N * (n + m));

        // Fill cost matrix and vector
        for (int i = 0; i < N; ++i) {
            H.block(i * n, i * n, n, n) = Q;  // State cost
            H.block(N * n + i * m, N * n + i * m, m, m) = R;  // Input cost
            g.segment(i * n, n) = -2.0 * Q * x_goal;  // Linear cost term for state
        }
        g.segment(N * n, m) = VectorXd::Zero(m);

        // Fill equality constraints (dynamics)
        MatrixXd Aeq = MatrixXd::Zero(N * n, N * (n + m));
        VectorXd beq = VectorXd::Zero(N * n);

        for (int i = 0; i < N; ++i) {
            if (i == 0) {
                Aeq.block(0, 0, n, n) = MatrixXd::Identity(n, n);
                beq.segment(0, n) = x;
            } else {
                Aeq.block(i * n, (i - 1) * n, n, n) = -A;
                Aeq.block(i * n, N * n + (i - 1) * m, n, m) = -B;
                Aeq.block(i * n, i * n, n, n) = MatrixXd::Identity(n, n);
            }
        }

        // Input constraints
        MatrixXd Ain = MatrixXd::Zero(N * m, N * (n + m));
        VectorXd lb = VectorXd::Constant(N * m, u_min);
        VectorXd ub = VectorXd::Constant(N * m, u_max);

        for (int i = 0; i < N; ++i) {
            Ain.block(i * m, N * n + i * m, m, m) = MatrixXd::Identity(m, m);
        }

        // Solve QP
        QProblem qp(N * (n + m), N * m);
        Options options;
        options.setToDefault();
        qp.setOptions(options);

        int nWSR = 100;
        qp.init(H.data(), g.data(), Ain.data(), lb.data(), ub.data(), Aeq.data(), beq.data(), nWSR);

        VectorXd z(N * (n + m));
        qp.getPrimalSolution(z.data());

        // Extract control input
        double u_opt = z[N * n];

        // Apply control input to the system
        x = A * x + B * u_opt;
        x_traj.push_back(x);
        u_traj.push_back(u_opt);
    }

    // Print results
    std::cout << "State Trajectory:\n";
    for (const auto& state : x_traj) {
        std::cout << state.transpose() << std::endl;
    }

    std::cout << "Control Trajectory:\n";
    for (const auto& u : u_traj) {
        std::cout << u << std::endl;
    }

    return 0;
}
