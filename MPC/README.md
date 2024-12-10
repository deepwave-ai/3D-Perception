# MPC
Overview

Model Predictive Control (MPC) is an advanced control technique that uses optimization to calculate control actions. It is particularly useful for systems with constraints, nonlinearities, or coupled dynamics.

MPC solves an optimization problem over a finite prediction horizon at each control step. The optimization minimizes a cost function while predicting system behavior subject to constraints.

1. Key Features
(1) Prediction Horizon

1.a Defines how far into the future the controller predicts.

1.b Trade-off:

    Short Horizon: Faster computation but less foresight.

    Long Horizon: Better performance but higher computational cost.

(2) Receding Horizon Principle

    Only the first control input is applied.

    At the next step, the optimization is re-solved with updated states.

(3) Weighting Matrices

Q,R,P tune the balance between state errors and control effort:

    High Q: Prioritize state accuracy.

    High  R: Penalize large control inputs.

    Terminal weight P: Improves stability.

(4) Stability

Terminal state constraints and terminal cost functions ensure stability.

Lyapunov methods are often used to verify stability properties.

2. Computational Aspects

2.a Online Optimization

At each time step, MPC solves an optimization problem, typically quadratic (linear systems) or nonlinear.

2.b Efficient Solvers

    Linear Systems: Quadratic Programming (QP) solvers (e.g., OSQP, qpOASES).

    Nonlinear Systems: Nonlinear Programming (NLP) solvers (e.g., CasADi, IPOPT).

2.c Real-Time Feasibility

Advanced MPC implementations exploit:

	Problem sparsity.

	Warm-starting from the previous solution.

	Parallel computing for faster optimization.

3. Applications

    Process Control: Chemical plants, refineries.

    Autonomous Vehicles: Path following, collision avoidance.

    Robotics: Arm motion planning, balancing.

    Energy Systems: Optimal power flow, demand response.

4. Strengths and Weaknesses

4.a Strengths

    Handles multivariable systems and constraints effectively.

    Predictive nature allows for anticipatory control.

    Flexible for linear and nonlinear systems.

4.b Weaknesses

    Computationally expensive, especially for long horizons or complex dynamics.

    Requires accurate system models and real-time solvers.

    May be sensitive to model inaccuracies.

5. Demo

To demonstrate Model Predictive Control (MPC) using Python, I'll provide a simple example where we control a car-like robot to follow a given trajectory. We'll use the cvxpy library to solve the optimization problem in MPC.

5.a System Dynamics:

The system is a simple linear model with position and velocity states.

5.b MPC Formulation:

The optimization problem minimizes the weighted sum of state and control input costs.

5.c Constraints:

Input (u) constraints and optional state constraints.

5.d Results:

Plots the trajectory of the system states and the control inputs applied over time.

![mpc](https://github.com/user-attachments/assets/94e6ec34-f20a-4ad4-9b40-a876fa952a43)
