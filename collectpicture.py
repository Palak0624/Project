from picamera2 import Picamera2
import os
import time

capture_dir = "captured_images"
os.makedirs(capture_dir, exist_ok=True)

cam = Picamera2()
cam_config = cam.create_still_configuration(main={"size": (3280, 2464)})
cam.configure(cam_config)
cam.start()

try:
    for i in range(500):
        image_path = os.path.join(capture_dir, f"image_{i+1}.jpg")
        cam.capture_file(image_path)
        print(f"Captured image saved to: {image_path}")
        time.sleep(5)

finally:
    cam.close()
