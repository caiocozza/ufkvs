import socket
import time

def main():
    # Define the server address and port
    server_address = ('localhost', 8080)

    # Create a TCP/IP socket
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error as err:
        print(f"Socket creation failed with error {err}")
        return

    try:
        # Connect the socket to the server
        sock.connect(server_address)
    except socket.gaierror:
        print("Address-related error connecting to server")
        return
    except socket.herror:
        print("Connection error")
        return
    except socket.error as err:
        print(f"Connection failed with error {err}")
        return

    try:
        # Define the message in bytes
        message = bytes([6, 0, 0, 0, 2, 0, 1, 0, 0, 0, 2, 0, 0, 0, 63, 63])

        # Send the message to the server
        try:
            sock.sendall(message)
            print(f"Sent: {message}")
        except socket.error as err:
            print(f"Sending data failed with error {err}")
            return

        # Wait for a message back from the server
        try:
            received_message = sock.recv(1024)  # Buffer size set to 1024 bytes
            print(f"Received: {received_message}")
        except socket.error as err:
            print(f"Receiving data failed with error {err}")
            return

    except Exception as e:
        print(f"An unexpected error occurred: {e}")

    finally:
        try:
            # Close the socket
            sock.close()
        except socket.error as err:
            print(f"Closing socket failed with error {err}")

if __name__ == "__main__":
    main()
