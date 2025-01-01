import open3d as o3d
import numpy as np

# Load a point cloud from a file
# You can replace "sample.ply" with your file path
try:
    point_cloud = o3d.io.read_point_cloud("complex_sample.ply")
    if len(np.asarray(point_cloud.points)) == 0:
        raise ValueError("Point cloud contains no points.")
except Exception as e:
    print(f"Error loading point cloud: {e}")
print("Loaded point cloud:", point_cloud)

print("Point cloud details:")
print(point_cloud)
print("Number of points:", np.asarray(point_cloud.points).shape)

# Visualize the point cloud
print("Displaying original point cloud...")
o3d.visualization.draw_geometries([point_cloud], window_name="Original Point Cloud")

# Downsample the point cloud with a voxel size of 0.05
voxel_size = 0.02
downsampled_pcd = point_cloud.voxel_down_sample(voxel_size=voxel_size)
print("Point cloud after downsampling:", downsampled_pcd)

# Estimate normals for the downsampled point cloud
downsampled_pcd.estimate_normals(
    search_param=o3d.geometry.KDTreeSearchParamHybrid(radius=0.1, max_nn=30)
)

# Visualize the downsampled point cloud
print("Displaying downsampled point cloud with estimated normals...")
o3d.visualization.draw_geometries([downsampled_pcd], window_name="Downsampled Point Cloud")

# Perform a transformation (e.g., translation)
transformation = np.eye(4)
transformation[0, 3] = 1.0  # Translate 1 unit along the x-axis
transformed_pcd = downsampled_pcd.transform(transformation)

# Visualize the transformed point cloud
print("Displaying transformed point cloud...")
o3d.visualization.draw_geometries([transformed_pcd], window_name="Transformed Point Cloud")
