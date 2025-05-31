import subprocess

def generate_certificate():
    try:
        command = (
            "openssl req -x509 -newkey rsa:4096 "
            "-keyout server.key -out server.crt "
            "-days 365 -nodes -subj /CN=localhost"
        )
        subprocess.run(command.split(), check=True)
        print("Generated server.key and server.crt in the current directory.")
    except subprocess.CalledProcessError as e:
        print(f"Error generating certificate: {e}")

if __name__ == "__main__":
    generate_certificate()