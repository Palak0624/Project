from picamera2 import Picamera2
import time
import os

# Define the folder name
folder_name = "images"

# Create the folder if it doesn't exist
os.makedirs(folder_name, exist_ok=True)

# Initialize the camera
picam2 = Picamera2()

# Configure the camera for 3280x2464 resolution
config = picam2.create_still_configuration(main={"size": (3280, 2464)})
picam2.configure(config)

# Start the camera
picam2.start()  # Start the camera

try:
    for i in range(500):
        # Capture and save the image directly to a file in the specified folder
        image_filename = os.path.join(folder_name, f"_{i + 1}.jpg")  # Naming convention
        picam2.capture_file(image_filename)
        print(f"Captured image saved as {image_filename}")

        # Wait for 2 seconds before the next capture
        time.sleep(2)

except KeyboardInterrupt:
    print("Exiting...")

finally:
    picam2.close()  # Close the camera
