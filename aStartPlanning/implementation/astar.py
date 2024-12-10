import heapq
import numpy as np
import matplotlib.pyplot as plt

def heuristic(a, b):
    """Heuristic function: Manhattan distance."""
    return abs(a[0] - b[0]) + abs(a[1] - b[1])

def astar(grid, start, goal):
    """
    A* algorithm for pathfinding.
    Args:
    - grid: 2D numpy array (1: obstacle, 0: free space).
    - start: Tuple of start coordinates (x, y).
    - goal: Tuple of goal coordinates (x, y).

    Returns:
    - List of path coordinates from start to goal.
    """
    neighbors = [(0, 1), (1, 0), (0, -1), (-1, 0)]  # 4-connected grid

    open_list = []
    heapq.heappush(open_list, (0 + heuristic(start, goal), 0, start, [start]))
    closed_set = set()

    while open_list:
        f, g, current, path = heapq.heappop(open_list)

        if current in closed_set:
            continue

        closed_set.add(current)

        if current == goal:
            return path  # Return path when goal is reached

        for dx, dy in neighbors:
            neighbor = (current[0] + dx, current[1] + dy)

            if 0 <= neighbor[0] < grid.shape[0] and 0 <= neighbor[1] < grid.shape[1]:
                if grid[neighbor] == 1 or neighbor in closed_set:
                    continue

                new_path = path + [neighbor]
                heapq.heappush(open_list, (g + 1 + heuristic(neighbor, goal), g + 1, neighbor, new_path))

    return None  # Return None if no path is found

# Define grid, start, and goal
grid = np.zeros((10, 10))
grid[3:8, 5] = 1  # Add an obstacle

start = (0, 0)
goal = (9, 9)

# Run A* and retrieve the path
path = astar(grid, start, goal)

# Visualization
plt.figure(figsize=(6, 6))
plt.imshow(grid, cmap='binary', origin='upper')
if path:
    path_x, path_y = zip(*path)
    plt.plot(path_y, path_x, 'r-o', label='Path')
plt.scatter(*start[::-1], c='green', label='Start', s=100)
plt.scatter(*goal[::-1], c='blue', label='Goal', s=100)
plt.legend()
plt.title("A* Pathfinding")
plt.show()
