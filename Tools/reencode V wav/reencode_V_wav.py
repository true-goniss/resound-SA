import os
import subprocess

def convert_wav_to_mp3(input_folder):
    ffmpeg_dir = os.path.dirname(os.path.abspath(__file__))
    os.environ["PATH"] += os.pathsep + ffmpeg_dir
    
    for root, dirs, files in os.walk(input_folder):
        for filename in files:
            if filename.endswith('.wav'):
                input_file = os.path.join(root, filename)
                output_file = os.path.splitext(input_file)[0] + '.mp3'
                
                command = [
                    'ffmpeg',
                    '-i', input_file,        # Input file
                    '-vn',                   # No video
                    '-ar', '44100',          # audio rate
                    '-ac', '2',              # number of audio channels
                    '-b:a', '320k',          # bitrate
                    output_file
                ]
                
                try:
                    subprocess.run(command, check=True)
                    os.remove(input_file)  # Delete the WAV file after successful conversion
                except Exception as e:
                    print(f"Error converting {input_file}: {e}")

    print("Conversion completed successfully.")

if __name__ == "__main__":
    script_folder = os.path.dirname(os.path.abspath(__file__))
    input_folder = script_folder

    convert_wav_to_mp3(input_folder)