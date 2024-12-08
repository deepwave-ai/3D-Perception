import cv2
import numpy as np
import dlib
from scipy.spatial import distance

# Initialize video capture (0 is the default camera)
cap = cv2.VideoCapture(0)

# Load dlib face detector and face recognition model
face_detector = dlib.get_frontal_face_detector()
face_recognizer = dlib.face_recognition_model_v1("dlib_face_recognition_resnet_model_v1.dat")
shape_predictor = dlib.shape_predictor("shape_predictor_68_face_landmarks.dat")

# Set up known face encoding (replace with a captured encoding for your target face)
known_face_encoding = None
def get_face_encoding(frame, detector, predictor, recognizer):
    # Detect faces in the color frame
    faces = detector(frame)
    if faces:
        shape = predictor(frame, faces[0])
        face_encoding = np.array(recognizer.compute_face_descriptor(frame, shape))
        return face_encoding
    return None

# If known_face_encoding is not set, capture a sample for it
while known_face_encoding is None:
    ret, frame = cap.read()
    if ret:
        known_face_encoding = get_face_encoding(frame, face_detector, shape_predictor, face_recognizer)
        cv2.imshow("Capture Known Face", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

cap.release()
cv2.destroyAllWindows()

# EKF parameters
state = np.array([0, 0, 0], dtype=float)  # [x, y, theta]
P = np.eye(3) * 500  # Covariance matrix
R = np.diag([0.5, 0.5, np.deg2rad(5)])  # Motion noise covariance
Q = np.eye(2) * 2  # Measurement noise covariance

def predict(state, P, u):
    """Predict step of EKF."""
    F = np.eye(3)  # State transition model
    state = state + u  # Apply motion (u is [dx, dy, dtheta])
    P = F @ P @ F.T + R  # Update covariance
    return state, P

def update(state, P, z, H):
    """Update step of EKF."""
    predicted_observation = H @ state
    y = z - predicted_observation[:2]
    S = H @ P @ H.T + Q
    K = P @ H.T @ np.linalg.inv(S)
    state = state + (K @ y)
    P = (np.eye(3) - K @ H) @ P
    return state, P

# Initialize video capture again
cap = cv2.VideoCapture(0)

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # Detect faces in the color frame
    faces = face_detector(frame)
    if faces:
        for face in faces:
            # Draw rectangle around face
            x, y, w, h = (face.left(), face.top(), face.width(), face.height())
            cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)

            # Get encoding of the detected face
            shape = shape_predictor(frame, face)
            current_face_encoding = np.array(face_recognizer.compute_face_descriptor(frame, shape))

            # Compare with known face encoding
            if known_face_encoding is not None:
                match_distance = distance.euclidean(known_face_encoding, current_face_encoding)
                if match_distance < 0.6:  # Threshold for a match
                    cv2.putText(frame, "Trump", (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
                    face_center = np.array([x + w / 2, y + h / 2])

                    # EKF Predict Step
                    u = np.array([0, 0, 0])  # Assuming no control input
                    state, P = predict(state, P, u)

                    # EKF Update Step (Use the face center as the measurement)
                    z = face_center  # Measurement
                    H = np.eye(2, 3)  # Measurement model
                    state, P = update(state, P, z, H)

                    # Display the estimated state on the frame
                    cv2.putText(frame, f"State: x={state[0]:.2f}, y={state[1]:.2f}, theta={np.degrees(state[2]):.2f}",
                                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 1, cv2.LINE_AA)
                else:
                    cv2.putText(frame, "Unknown Face", (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)

    # Show the current frame
    cv2.imshow('EKF-SLAM with Face Recognition', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
