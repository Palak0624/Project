from flask import Flask, request
from threading import Thread
from tkinter import Tk, Label, Button
from PIL import Image, ImageTk
import os
import time
import numpy as np
import tensorflow as tf
from tensorflow.keras.preprocessing import image
from picamera2 import Picamera2

# Load the saved model
model = tf.keras.models.load_model("classification_model.h5")

# Initialize the camera
picam2 = Picamera2()
config = picam2.create_still_configuration(main={"size": (3280, 2464)})
picam2.configure(config)

# Create the folder for storing captured images if it doesn't exist
folder_name = "testing_images"
os.makedirs(folder_name, exist_ok=True)

# Variables to keep track of predictions
total_predictions = 0
correct_predictions = 0

# Function to predict if the image is correct or incorrect
def predict_image(img_path):
    img = image.load_img(img_path, target_size=(150, 150))
    img_array = image.img_to_array(img)
    img_array = np.expand_dims(img_array, axis=0)
    img_array /= 255.0

    prediction = model.predict(img_array)
    prediction_label = 'Correct' if prediction[0] <= 0.5 else 'Incorrect'
    return prediction_label

# Flask setup
app = Flask(__name__)

@app.route('/trigger', methods=['GET'])
def trigger_capture():
    global total_predictions, correct_predictions

    # Capture and save the image
    image_filename = os.path.join(folder_name, f"_{int(time.time())}.jpg")
    picam2.capture_file(image_filename)

    # Predict and update accuracy
    prediction_label = predict_image(image_filename)
    total_predictions += 1
    if prediction_label == 'Correct':
        correct_predictions += 1

    # Calculate the accuracy percentage
    accuracy = (correct_predictions / total_predictions) * 100 if total_predictions > 0 else 0

    # Update GUI with the current image, prediction, and accuracy
    update_gui(image_filename, prediction_label, accuracy)

    return {"status": "success", "prediction": prediction_label}

# Function to update the GUI with the latest image and accuracy
def update_gui(image_path, prediction, accuracy):
    # Update current prediction
    current_prediction_label.config(text=f"Current Prediction: {prediction}")

    # Load and display the image
    img = Image.open(image_path).resize((300, 300))  # Resize for display
    img = ImageTk.PhotoImage(img)
    image_display.config(image=img)
    image_display.image = img  # Keep a reference to avoid garbage collection

    # Update the accuracy label
    accuracy_label.config(text=f"Product Line Accuracy: {accuracy:.2f}%")

# Set up the Tkinter GUI
root = Tk()
root.title("Prediction GUI")
root.geometry("500x600")  # Initial window size

# Display current prediction
current_prediction_label = Label(root, text="Current Prediction: None", font=("Helvetica", 14))
current_prediction_label.pack(pady=20)

# Display the image
image_display = Label(root)
image_display.pack(pady=20)

# Display the accuracy percentage
accuracy_label = Label(root, text="Product Line Accuracy: 0.00%", font=("Helvetica", 14))
accuracy_label.pack(pady=20)

# Button to capture and predict
capture_button = Button(root, text="Capture and Predict", font=("Helvetica", 12), command=lambda: trigger_capture())
capture_button.pack(pady=20)

# Start the Flask server in a separate thread
flask_thread = Thread(target=lambda: app.run(host='0.0.0.0', port=5000))
flask_thread.daemon = True
flask_thread.start()

# Start the camera and Tkinter main loop
picam2.start()
root.mainloop()

# Cleanup the camera after closing the GUI
picam2.close()