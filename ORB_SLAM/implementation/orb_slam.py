import cv2
import numpy as np

def compute_matches(descriptors1, descriptors2):
    """
    Compute feature matches using a brute-force matcher.
    """
    # Create a BFMatcher object with Hamming distance (suitable for ORB)
    bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)

    # Match descriptors
    matches = bf.match(descriptors1, descriptors2)

    # Sort matches by distance
    matches = sorted(matches, key=lambda x: x.distance)
    return matches

def find_pose(matches, keypoints1, keypoints2, K):
    """
    Estimate camera pose (rotation and translation) using matched keypoints.
    """
    # Extract matched keypoints
    pts1 = np.float32([keypoints1[m.queryIdx].pt for m in matches]).reshape(-1, 1, 2)
    pts2 = np.float32([keypoints2[m.trainIdx].pt for m in matches]).reshape(-1, 1, 2)

    # Find essential matrix
    E, mask = cv2.findEssentialMat(pts1, pts2, K, method=cv2.RANSAC, prob=0.999, threshold=1.0)

    # Recover pose (R, t)
    _, R, t, _ = cv2.recoverPose(E, pts1, pts2, K)

    return R, t, mask

def visual_slam(video_path, K):
    """
    Simple Visual SLAM using ORB.
    """
    # Open the video
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print("Error: Unable to open video.")
        return

    # Create an ORB detector
    orb = cv2.ORB_create()

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

        # Detect ORB keypoints and descriptors
        keypoints, descriptors = orb.detectAndCompute(gray, None)

        if prev_frame is not None:
            # Match features with the previous frame
            matches = compute_matches(prev_descriptors, descriptors)

            # Estimate pose
            R, t, mask = find_pose(matches, prev_keypoints, keypoints, K)

            # Display matches
            match_img = cv2.drawMatches(
                prev_frame, prev_keypoints, frame, keypoints, matches[:50], None, flags=cv2.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS
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
video_path = 0  # Replace with your video path or use 0 for a live camera feed
visual_slam(video_path, K)
