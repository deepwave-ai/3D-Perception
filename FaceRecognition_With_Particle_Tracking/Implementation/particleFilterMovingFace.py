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

# Particle Filter parameters
num_particles = 100  # Number of particles
particles = np.random.rand(num_particles, 2) * [640, 480]  # Initial particles across the frame
particle_weights = np.ones(num_particles) / num_particles  # Uniform weights initially

# Set up known face encoding (replace with a captured encoding for your target face)
known_face_encoding = None
def get_face_encoding(frame, detector, predictor, recognizer):
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

# Particle Filter functions
def particle_predict(particles, motion_noise=10):
    """Predict the next state for each particle by adding random motion noise."""
    particles += np.random.normal(0, motion_noise, particles.shape)  # Random walk with noise

def particle_update(particles, weights, measurement, measurement_noise=50):
    """Update particle weights based on the distance to the measurement."""
    distances = np.linalg.norm(particles - measurement, axis=1)
    weights *= np.exp(-distances**2 / (2 * measurement_noise**2))
    weights += 1.e-300  # Avoid division by zero
    weights /= sum(weights)  # Normalize weights

def particle_resample(particles, weights):
    """Resample particles according to weights using systematic resampling."""
    cumulative_sum = np.cumsum(weights)
    cumulative_sum[-1] = 1.  # Avoid rounding errors
    indexes = np.searchsorted(cumulative_sum, np.random.rand(num_particles))

    # Resample particles and reset weights
    particles[:] = particles[indexes]
    weights.fill(1.0 / num_particles)

# Initialize video capture again
cap = cv2.VideoCapture(0)

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # Detect faces
    faces = face_detector(frame)
    if faces:
        for face in faces:
            # Draw rectangle around face
            x, y, w, h = (face.left(), face.top(), face.width(), face.height())
            cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)

            # Get encoding of the detected face
            shape = shape_predictor(frame, face)
            current_face_encoding = np.array(face_recognizer.compute_face_descriptor(frame, shape))

            # Compare with known face encoding
            if known_face_encoding is not None:
                match_distance = distance.euclidean(known_face_encoding, current_face_encoding)
                if match_distance < 0.6:  # Threshold for a match
                    cv2.putText(frame, "Trump", (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
                    face_center = np.array([x + w / 2, y + h / 2])

                    # Particle Filter predict and update
                    particle_predict(particles)
                    particle_update(particles, particle_weights, face_center)

                    # Resample particles
                    particle_resample(particles, particle_weights)

                    # Estimate position from weighted mean of particles
                    estimated_position = np.average(particles, weights=particle_weights, axis=0)

                    # Display estimated position on the frame
                    cv2.putText(frame, f"Est. Position: x={estimated_position[0]:.2f}, y={estimated_position[1]:.2f}",
                                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 1, cv2.LINE_AA)

                    # Draw particles
                    for particle in particles:
                        cv2.circle(frame, (int(particle[0]), int(particle[1])), 1, (0, 255, 0), -1)

    # Show the current frame
    cv2.imshow('Particle Filter Face Tracking', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
