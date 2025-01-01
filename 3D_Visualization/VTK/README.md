# 3D-Visualization with VTK
Overview

VTK (Visualization Toolkit) is a powerful, open-source software library for 3D computer graphics, image processing, and visualization.
The tool kit is written in C++ but has bindings for Python, Java, and Tcl, making it versatile for developers.
It is suitable for high-performance, large-scale scientific and engineering applications.

The Key Features:

(1) Data Visualization: Handles scalar, vector, and tensor data visualization.

(2) Geometry Creation: Provides primitives like spheres, cubes, cylinders, etc.

(3) Structured & Unstructured Grids: Supports advanced grid-based representations for scientific data.

(4) Algorithms: Includes algorithms for contouring, clipping, smoothing, and Boolean operations.

(5) Rendering: Produces interactive 3D renderings with customizable camera views and lighting.

(6) Extensibility: Highly customizable with a modular architecture.

(7) File I/O: Reads and writes many 3D data formats (e.g., .vtk, .ply, .obj, .stl, .vtp).

PyVista is a Python library built on top of VTK, offering a high-level, user-friendly interface. It simplifies 3D visualization and computational geometry for Python users.
It is popular in academia and among Python enthusiasts for quick prototyping and visualization.

In summary, VTK is a robust toolkit for advanced visualization and scientific computing, while PyVista simplifies these tasks for Python developers by abstracting much of VTK’s complexity. Both are excellent tools depending on the level of customization and ease of use required.

This Python code demonstrates a variety of 3D visualization and computational geometry tasks using the PyVista library. Below is a detailed breakdown of each section of the code:

1. Creating and Plotting a Sphere

Purpose: Creates a 3D sphere of radius 1 centered at the origin and visualizes it.

Key Functions:

(1) pv.Sphere: Generates a sphere mesh.

(2) pv.Plotter: Sets up a rendering environment.

(3) add_mesh: Adds the sphere to the plot with red color and visible edges.

(4) show: Displays the rendering.
<img width="617" alt="sphere" src="https://github.com/user-attachments/assets/7f42785d-2d28-4417-918a-de195e5eeb04" />

2. Creating and Plotting a Cylinder

Purpose: Creates a cylinder with:

Radius = 1

Height = 2

Axis aligned along the y-direction.

Visualization: The cylinder is displayed in blue.
<img width="608" alt="cylinder" src="https://github.com/user-attachments/assets/dab5077b-b0c8-43a0-a6c3-b5d060acd90b" />

3. Boolean Operations Between a Sphere and a Cube

Purpose: Demonstrates a Boolean operation where a cube is subtracted from a sphere.

Steps:

(1) Create a sphere and a cube.

(2) Ensure both meshes are triangulated.

(3) Use boolean_difference to subtract the cube from the sphere.

(4) Visualize the result in orange.
<img width="520" alt="sphere_cube" src="https://github.com/user-attachments/assets/02ce10a6-0aa5-4b89-89ad-507c546f4a96" />

4. Streamline Visualization in a Vector Field

Purpose: Simulates and visualizes streamlines in a 3D vector field.

Steps:

(1) Create a structured grid of points using np.meshgrid.

(2) Define a vector field (u, v, w) over the grid.

(3) Use grid.streamlines to compute streamlines and seed points.

(4) Visualize the streamlines and seed points.
<img width="473" alt="vector_field" src="https://github.com/user-attachments/assets/6437857a-fb4b-4d2e-a3cd-4837aefdb1ed" />

5. Point Cloud Visualization

Purpose: Displays a 3D point cloud of 100 random points.

Key Functions:

(1) pv.PolyData: Creates a point cloud object.

(2) add_mesh: Visualizes points as green spheres.
<img width="595" alt="cloud_point_random" src="https://github.com/user-attachments/assets/c299d4b3-7253-4034-bba7-97bf63e7db1e" />

6. Loading and Displaying a PLY File

Purpose: Reads and visualizes a point cloud from a .ply file.

Key Functions:

(1) pv.read: Reads the .ply file.

(2) global_theme.show_edges: Controls the rendering of edges.

7. Visualizing Scalars on a Structured Grid as a Volume

Purpose: Visualizes scalar values on a structured grid as a 3D volume.
<img width="464" alt="cloud_point_file" src="https://github.com/user-attachments/assets/bac6f9a4-ff80-4315-9f7b-9cb002343159" />

Steps:

(1) Create a structured grid with dimensions (50, 50, 50).

(2) Assign scalar values (sqrt(x^2 + y^2 + z^2)).

(3) Use add_volume to render the grid as a colored volume.

Summary

This code showcases various 3D visualization techniques in PyVista, including creating and visualizing geometries (spheres, cylinders, cubes), performing Boolean operations, simulating streamlines, and working with structured grids and point clouds. It highlights PyVista’s ease of use for handling both geometry and data visualization tasks. The video demo files can be found in video_demo directory.
