import serial
import os
import time
import subprocess
import asyncio
from telegram import Bot

# Telegram bot configuration
BOT_TOKEN = "7827452985:AAHzLijecwRv_R290aJJ3ohNCCULeicg9v0"  # Replace with your bot token
CHAT_ID = "6240903545"  # Replace with your chat ID

# Serial configuration
SERIAL_PORT = "/dev/ttyACM0"  # Replace with the correct serial port
BAUD_RATE = 9600

# Initialize the Telegram bot
bot = Bot(token=BOT_TOKEN)

# Initialize the serial connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
ser.reset_input_buffer()  # Clear the serial input buffer
print("Serial connection established. Waiting for input...")

def capture_video_audio():
    """Capture video and audio using ffmpeg."""
    output_file = "capture.mp4"
    duration = 10  # Duration of the video in seconds

    # Command to capture video and audio
    command = [
        "ffmpeg",
        "-f", "v4l2", "-video_size", "640x480", "-i", "/dev/video0",
        "-f", "alsa", "-ac", "1", "-ar", "16000", "-sample_fmt", "s16", "-i", "hw:2,0",
        "-t", str(duration),
        "-c:v", "libx264", "-profile:v", "high", "-pix_fmt", "yuv420p",
        "-vf", "scale=640:480", "-b:v", "1M", "-r", "30", "-g", "60",
        "-c:a", "aac", "-b:a", "128k", "-movflags", "+faststart",
        output_file
    ]
    print(f"Running command: {' '.join(command)}")

    try:
        # Run the command using subprocess and suppress all output
        process = subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        process.wait()  # Wait for the command to complete

        if process.returncode == 0:
            print("Video capture successful.")
            return output_file
        else:
            print(f"Error during video capture. Return code: {process.returncode}")
            return None
    except Exception as e:
        print(f"Exception during video capture: {e}")
        return None

async def send_to_telegram(file_path):
    """Send the captured media to Telegram."""
    if file_path is None:
        print("No file to send.")
        return

    try:
        print(f"Attempting to send file: {file_path}")
        with open(file_path, "rb") as file:
            await bot.send_video(chat_id=CHAT_ID, video=file, supports_streaming=True)
        print("Video sent to Telegram.")

        # Delete the file after sending
        os.remove(file_path)
        print(f"Deleted file: {file_path}")
    except Exception as e:
        print(f"Failed to send video: {e}")

async def main():
    try:
        while True:
            if ser.in_waiting > 0:
                data = ser.readline().decode("utf-8", errors='ignore').strip()
            
                print(f"Received: {data} (Length: {len(data)})")

                # Only process if the received data is '1'
                if data  == "-1" or int(data) > 120:
                    print("Trigger received. Capturing video and audio...")
                    video_file = capture_video_audio()
                    if video_file:
                        print("Sending video to Telegram...")
                        await send_to_telegram(video_file)
                    else:
                        print("Failed to capture video.")
                    print("Done.")
                else:
                    print(f"Ignoring unexpected data: {data}")

    except KeyboardInterrupt:
        print("Program stopped.")

    finally:
        ser.close()

# Run the asyncio event loop
if _name_ == "_main_":
    asyncio.run(main())
