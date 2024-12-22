# A* Algorithm

Overview

The A* (A-star) algorithm is a graph-based search algorithm widely used for pathfinding and graph traversal. It is optimal and complete when its heuristic function 
h(n) is admissible (never overestimates the actual cost to reach the goal).

Key Concepts

(1) Graph Representation:

The algorithm operates on a graph where nodes represent states, and edges represent transitions or paths between states with associated costs.

(2) Cost Function:

	f(n) = g(n) + h(n)

		g(n): The cost of the path from the start node to the current node 

		h(n): The heuristic estimate of the cost from n to the goal.

(3) Open and Closed Lists:

	Open List: Contains nodes to be evaluated.
	Closed List: Contains nodes already evaluated.

(4) Optimality:

A* is guaranteed to find the shortest path if the heuristic h(n) is admissible and consistent.

Use Cases

	Robot Navigation
	Game Development
	Network Routing
	Logistics and Supply Chain

Implementation

(1) Input:

grid: Represents the environment (1 for obstacles, 0 for free space).
start: Starting coordinates.
goal: Goal coordinates.
Algorithm Workflow:

(2) Start from the initial node, compute 

	f(n)=g(n)+h(n) for each neighbor, and explore the node with the smallest f(n).
	Add nodes to the closed list once evaluated.
	Stop when the goal is reached or the open list is empty.

(3) Visualization:

The output shows the grid, the obstacle, and the path from start to goal.

![aStarPlaning](https://github.com/user-attachments/assets/6e848112-af89-460e-bdc1-1eaf44805cc6)

C++ output:
![aStarPath_cpp](https://github.com/user-attachments/assets/e268bea4-85d1-4ca7-94fe-cecfaccb0b2d)
