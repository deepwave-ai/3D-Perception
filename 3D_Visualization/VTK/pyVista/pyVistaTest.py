import pyvista as pv
import numpy as np

# Create a sphere
sphere = pv.Sphere(radius=1.0, center=(0, 0, 0))

# Plot the sphere
plotter = pv.Plotter()
plotter.add_mesh(sphere, color='red', show_edges=True)
plotter.show()

# Create a cylinder
cylinder = pv.Cylinder(center=(0, 0, 0), direction=(0, 1, 0), radius=1.0, height=2.0, resolution=50)

# Plot the cylinder
plotter = pv.Plotter()
plotter.add_mesh(cylinder, color='blue', show_edges=True)
plotter.show()

# Create two meshes: a sphere and a cube
sphere = pv.Sphere(radius=1.0, center=(0, 0, 0))
cube = pv.Cube(center=(0.5, 0.5, 0), x_length=1.5, y_length=1.5, z_length=1.5)

# Ensure both meshes are triangulated
sphere = sphere.triangulate()
cube = cube.triangulate()

# Perform a boolean difference (subtract cube from sphere)
difference = sphere.boolean_difference(cube)

# Plot the result
plotter = pv.Plotter()
plotter.add_mesh(difference, color='orange', show_edges=True)
plotter.show()


# Define the dimensions of the grid
nx, ny, nz = 20, 20, 20  # Number of points along each axis

# Create the coordinates for the structured grid
x = np.linspace(-1, 1, nx)
y = np.linspace(-1, 1, ny)
z = np.linspace(-1, 1, nz)
x, y, z = np.meshgrid(x, y, z, indexing="ij")

# Flatten the coordinates
points = np.c_[x.ravel(), y.ravel(), z.ravel()]

# Create the vector field
u = -y.flatten()
v = x.flatten()
w = np.zeros_like(u)

# Create a StructuredGrid object
grid = pv.StructuredGrid()
grid.points = points
grid.dimensions = (nx, ny, nz)

# Assign the vector field
grid["vectors"] = np.stack((u, v, w), axis=-1)

# Generate streamlines
streamlines, seeds = grid.streamlines("vectors", return_source=True, n_points=100)

# Visualize the streamlines
plotter = pv.Plotter()
plotter.add_mesh(streamlines, line_width=2, color="black")  # Add streamlines
plotter.add_mesh(seeds, render_points_as_spheres=True, color="red", point_size=5)  # Add seed points
plotter.show()

# Generate random points
points = np.random.rand(100, 3)  # 100 random points in 3D space

# Create a point cloud
point_cloud = pv.PolyData(points)

# Plot the point cloud
plotter = pv.Plotter()
plotter.add_mesh(point_cloud, point_size=10, render_points_as_spheres=True, color="green")
plotter.show()

point_cloud = pv.read("complex_sample.ply")
pv.global_theme.show_edges = False  # Avoid potential edge rendering issues

# Create and display the plotter
plotter = pv.Plotter(window_size=(800, 600), notebook=False)
plotter.add_mesh(point_cloud, point_size=10, render_points_as_spheres=True, color="green")
plotter.show()


# Define the grid dimensions
nx, ny, nz = 50, 50, 50  # Number of points along each axis

# Create coordinates for the structured grid
x = np.linspace(-1, 1, nx)
y = np.linspace(-1, 1, ny)
z = np.linspace(-1, 1, nz)
x, y, z = np.meshgrid(x, y, z, indexing="ij")

# Flatten the coordinate arrays to create point arrays
points = np.c_[x.ravel(), y.ravel(), z.ravel()]

# Create a StructuredGrid object
grid = pv.StructuredGrid()
grid.points = points
grid.dimensions = (nx, ny, nz)

# Create scalar data for visualization
scalar_data = np.sqrt(x**2 + y**2 + z**2).ravel()

# Assign scalars to the grid
grid["values"] = scalar_data  # Use this to add scalars

# Visualize the grid as a volume
plotter = pv.Plotter()
plotter.add_volume(grid, scalars="values", cmap="plasma", opacity="sigmoid")
plotter.show()
