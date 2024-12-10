import numpy as np
import matplotlib.pyplot as plt
import cvxpy as cp

# Define the system dynamics (discrete-time linear system)
dt = 0.1  # Time step
A = np.array([[1, dt], [0, 1]])  # State transition matrix
B = np.array([[0], [dt]])  # Control input matrix
n = A.shape[0]  # Number of states
m = B.shape[1]  # Number of control inputs

# MPC parameters
N = 10  # Prediction horizon
x_goal = np.array([10, 0])  # Target state

# Define the cost matrices
Q = np.eye(n)  # State cost matrix
R = 0.1 * np.eye(m)  # Control input cost matrix

# Constraints
u_min, u_max = -2, 2
x_min, x_max = -np.inf, np.inf

# Initialize state and control trajectory
x = np.array([0, 0])  # Initial state
u_traj = []

# Simulation
T = 50  # Total simulation time steps
x_traj = [x]

for _ in range(T):
    # Define optimization variables
    x_var = cp.Variable((n, N + 1))
    u_var = cp.Variable((m, N))

    # Define the cost function
    cost = 0
    constraints = []
    for t in range(N):
        cost += cp.quad_form(x_var[:, t] - x_goal, Q) + cp.quad_form(u_var[:, t], R)
        constraints += [x_var[:, t + 1] == A @ x_var[:, t] + B @ u_var[:, t]]
        constraints += [u_min <= u_var[:, t], u_var[:, t] <= u_max]

    # Terminal cost
    cost += cp.quad_form(x_var[:, N] - x_goal, Q)

    # Initial state constraint
    constraints += [x_var[:, 0] == x]

    # State constraints
    constraints += [x_min <= x_var, x_var <= x_max]

    # Solve the optimization problem
    problem = cp.Problem(cp.Minimize(cost), constraints)
    problem.solve()

    # Apply the first control input
    u_opt = u_var[:, 0].value
    x = A @ x + B @ u_opt
    x_traj.append(x)
    u_traj.append(u_opt)

# Convert trajectories to arrays for plotting
x_traj = np.array(x_traj)
u_traj = np.array(u_traj)

# Plot results
plt.figure(figsize=(12, 5))

plt.subplot(1, 2, 1)
plt.plot(x_traj[:, 0], label='Position')
plt.plot([x_goal[0]] * len(x_traj), 'r--', label='Target Position')
plt.xlabel('Time step')
plt.ylabel('Position')
plt.title('State Trajectory')
plt.legend()

plt.subplot(1, 2, 2)
plt.step(range(len(u_traj)), u_traj, where='post')
plt.xlabel('Time step')
plt.ylabel('Control Input')
plt.title('Control Input')

plt.tight_layout()
plt.show()
