import os
import numpy as np
import cv2
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.metrics import confusion_matrix, ConfusionMatrixDisplay
import tensorflow as tf
from tf_keras.applications import ResNet50
from tf_keras.models import Model, Sequential
from tf_keras.layers import GlobalAveragePooling2D, Dense, Dropout, Conv2D, Input
from tf_keras.preprocessing.image import ImageDataGenerator
from tf_keras.optimizers import Adam
from tf_keras.utils import to_categorical


# Load and preprocess the OCT dataset
def load_data(data_dir, categories, img_size=128):
    images = []
    labels = []
    for label, category in enumerate(categories):
        category_path = os.path.join(data_dir, category)
        for img_file in os.listdir(category_path):
            try:
                img_path = os.path.join(category_path, img_file)
                img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
                img_resized = cv2.resize(img, (img_size, img_size))
                img_rgb = cv2.cvtColor(img_resized, cv2.COLOR_GRAY2RGB)  # Convert to RGB
                images.append(img_rgb)
                labels.append(label)
            except Exception as e:
                print(f"Error loading image {img_file}: {e}")
    #images = np.array(images).reshape(-1, img_size, img_size, 1)
    images = np.array(images)
    labels = np.array(labels)
    return images / 255.0, to_categorical(labels, num_classes=len(categories))

# Set paths and categories
data_dir = "F:\work\Ocular_Disease_Recognition\OCT2017\dest"  # Replace with the dataset path
categories = ["CNV", "DME", "DRUSEN", "NORMAL"]  # Define categories

# Load data
X, y = load_data(data_dir, categories)
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Data augmentation
datagen = ImageDataGenerator(
    rotation_range=15,
    width_shift_range=0.1,
    height_shift_range=0.1,
    zoom_range=0.2,
    horizontal_flip=True,
    fill_mode='nearest'
)
datagen.fit(X_train)

# Load the pre-trained ResNet50 model
base_model = ResNet50(weights='imagenet', include_top=False, input_shape=(128, 128, 3))

# Freeze the base model
for layer in base_model.layers:
    layer.trainable = False

# Add custom layers
x = base_model.output
x = GlobalAveragePooling2D()(x)
x = Dense(256, activation='relu')(x)
x = Dropout(0.5)(x)
output = Dense(len(categories), activation='softmax')(x)

# Create the complete model
model = Model(inputs=base_model.input, outputs=output)

for i, layer in enumerate(model.layers):
    print(f"{i}: {layer.name} ({layer.__class__.__name__})")

# Compile the model
model.compile(optimizer=Adam(learning_rate=0.001), loss='categorical_crossentropy', metrics=['accuracy'])

# Train the top layers
history = model.fit(
    datagen.flow(X_train, y_train, batch_size=32),
    validation_data=(X_test, y_test),
    epochs=10
)

# Unfreeze some layers for fine-tuning
for layer in base_model.layers[-30:]:
    layer.trainable = True

# Recompile the model with a lower learning rate
model.compile(optimizer=Adam(learning_rate=0.0001), loss='categorical_crossentropy', metrics=['accuracy'])

# Fine-tune the model
fine_tune_history = model.fit(
    datagen.flow(X_train, y_train, batch_size=32),
    validation_data=(X_test, y_test),
    epochs=10
)

# Plot training history
def plot_training_history(history, fine_tune_history=None):
    plt.figure(figsize=(12, 4))

    plt.subplot(1, 2, 1)
    plt.plot(history.history['accuracy'], label='Train Accuracy')
    plt.plot(history.history['val_accuracy'], label='Validation Accuracy')
    if fine_tune_history:
        plt.plot(fine_tune_history.history['accuracy'], label='Fine-Tune Train Accuracy')
        plt.plot(fine_tune_history.history['val_accuracy'], label='Fine-Tune Validation Accuracy')
    plt.title('Accuracy')
    plt.xlabel('Epochs')
    plt.ylabel('Accuracy')
    plt.legend()

    plt.subplot(1, 2, 2)
    plt.plot(history.history['loss'], label='Train Loss')
    plt.plot(history.history['val_loss'], label='Validation Loss')
    if fine_tune_history:
        plt.plot(fine_tune_history.history['loss'], label='Fine-Tune Train Loss')
        plt.plot(fine_tune_history.history['val_loss'], label='Fine-Tune Validation Loss')
    plt.title('Loss')
    plt.xlabel('Epochs')
    plt.ylabel('Loss')
    plt.legend()

    plt.show()

plot_training_history(history, fine_tune_history)

# Confusion matrix
y_pred = np.argmax(model.predict(X_test), axis=1)
y_true = np.argmax(y_test, axis=1)
cm = confusion_matrix(y_true, y_pred, labels=range(len(categories)))

disp = ConfusionMatrixDisplay(confusion_matrix=cm, display_labels=categories)
disp.plot(cmap='Blues', xticks_rotation='vertical')
plt.title("Confusion Matrix")
plt.show()

def generate_grad_cam(model, image, class_idx):
    """
    Generate Grad-CAM heatmap for a specific image and class index.
    """
    # Specify the last convolutional layer for ResNet50
    last_conv_layer_name = "conv5_block3_out"  # Confirmed from model.summary()

    # Get the last convolutional layer
    last_conv_layer = model.get_layer(last_conv_layer_name)

    # Define the Grad model to output the activations of the last conv layer and predictions
    grad_model = Model(
        inputs=model.input,  # Explicitly reference the original model's input
        outputs=[last_conv_layer.output, model.output]
    )

    # Compute the gradient of the loss with respect to the activations of the last conv layer
    with tf.GradientTape() as tape:
        # Ensure the image has a batch dimension
        inputs = tf.expand_dims(image, axis=0)  # Add batch dimension
        conv_outputs, predictions = grad_model(inputs)
        loss = predictions[:, class_idx]

    # Compute gradients of the loss with respect to the convolutional outputs
    grads = tape.gradient(loss, conv_outputs)

    # Pool the gradients across the spatial dimensions
    pooled_grads = tf.reduce_mean(grads, axis=(0, 1, 2))

    # Weight the feature maps by the gradients
    conv_outputs = conv_outputs[0]
    heatmap = tf.reduce_sum(pooled_grads * conv_outputs, axis=-1)

    # Normalize the heatmap
    heatmap = tf.maximum(heatmap, 0) / tf.math.reduce_max(heatmap)

    return heatmap.numpy()


for i, layer in enumerate(model.layers):
    print(f"{i}: {layer.name} ({layer.__class__.__name__})")
print("Model Input Shape:", model.input.shape)
print("Model Output Shape:", model.output.shape)

# Visualize Grad-CAM for a test image
index = 0
image = X_test[index]
true_class = np.argmax(y_test[index])

heatmap = generate_grad_cam(model, image, true_class)

plt.figure(figsize=(8, 4))
plt.subplot(1, 2, 1)
plt.title("Original Image")
plt.imshow(image)

plt.subplot(1, 2, 2)
plt.title("Grad-CAM Heatmap")
plt.imshow(image)
plt.imshow(heatmap, cmap='jet', alpha=0.5)
plt.colorbar()
plt.show()
