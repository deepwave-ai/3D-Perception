import cv2
import numpy as np

def compute_matches(descriptors1, descriptors2):
    """
    Compute feature matches using FLANN-based matcher.
    """
    # FLANN parameters
    FLANN_INDEX_KDTREE = 1
    index_params = dict(algorithm=FLANN_INDEX_KDTREE, trees=5)
    search_params = dict(checks=50)
    flann = cv2.FlannBasedMatcher(index_params, search_params)

    # Match descriptors
    matches = flann.knnMatch(descriptors1, descriptors2, k=2)

    # Filter matches using the Lowe's ratio test
    good_matches = []
    for m, n in matches:
        if m.distance < 0.7 * n.distance:
            good_matches.append(m)
    return good_matches

def find_pose(matches, keypoints1, keypoints2, K):
    """
    Estimate camera pose (rotation and translation) using matched keypoints.
    """
    # Extract matched keypoints
    pts1 = np.float32([keypoints1[m.queryIdx].pt for m in matches])
    pts2 = np.float32([keypoints2[m.trainIdx].pt for m in matches])

    # Find essential matrix
    E, mask = cv2.findEssentialMat(pts1, pts2, K, method=cv2.RANSAC, prob=0.999, threshold=1.0)

    # Recover pose (R, t)
    _, R, t, _ = cv2.recoverPose(E, pts1, pts2, K)

    return R, t

def visual_slam(video_path, K):
    """
    Simple Visual SLAM using SIFT.
    """
    # Open the video
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print("Error: Unable to open video.")
        return

    # Initialize SIFT detector
    sift = cv2.SIFT_create()

    # Initialize previous frame data
    prev_frame = None
    prev_keypoints = None
    prev_descriptors = None

    # Process video frames
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break

        # Convert to grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Detect SIFT keypoints and descriptors
        keypoints, descriptors = sift.detectAndCompute(gray, None)

        if prev_frame is not None:
            # Match features with the previous frame
            matches = compute_matches(prev_descriptors, descriptors)

            # Estimate pose
            R, t = find_pose(matches, prev_keypoints, keypoints, K)

            # Display matches
            match_img = cv2.drawMatches(
                prev_frame, prev_keypoints, frame, keypoints, matches, None, flags=cv2.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS
            )
            cv2.imshow("Feature Matches", match_img)

            print("Rotation Matrix:\n", R)
            print("Translation Vector:\n", t)

        # Update previous frame data
        prev_frame = frame
        prev_keypoints = keypoints
        prev_descriptors = descriptors

        # Exit on pressing 'q'
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

# Camera intrinsic parameters (example for a typical camera)
K = np.array([
    [718.856, 0, 607.1928],
    [0, 718.856, 185.2157],
    [0, 0, 1]
])

# Path to video or live camera feed (use 0 for default camera)
video_path = 0;  # Replace with your video path or use 0 for a live camera feed
visual_slam(video_path, K)
