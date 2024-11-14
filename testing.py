import os
import tensorflow as tf
from tensorflow.keras.preprocessing import image
import numpy as np

# Load the saved model
model = tf.keras.models.load_model("classification_model.h5")

# Define the directories for correct and incorrect images
test_dirs = [	
    '/home/yashpinder/testing_images'
]

# Define a function to test images in a directory
def test_images_in_directory(test_dir):
    print(f"Testing images in directory: {test_dir}")
    for img_name in os.listdir(test_dir):
        img_path = os.path.join(test_dir, img_name)
        if os.path.isfile(img_path):  # Ensure it's a file, not a directory
            img = image.load_img(img_path, target_size=(150, 150))  # Resize the image
            img_array = image.img_to_array(img)  # Convert image to array
            img_array = np.expand_dims(img_array, axis=0)  # Add batch dimension
            img_array /= 255.0  # Rescale the image

            # Make prediction
            prediction = model.predict(img_array)
            prediction_label = 'Incorrect' if prediction[0] > 0.5 else 'Correct'
            print(f"Image: {img_name} - Prediction: {prediction_label}")

# Test all images in both the correct and incorrect directories
for test_dir in test_dirs:
    if os.path.exists(test_dir):  # Ensure the directory exists
        test_images_in_directory(test_dir)
    else:
        print(f"Directory {test_dir} not found!")
