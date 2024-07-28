import socket
import time

def main():
    # Define the server address and port
    server_address = ('localhost', 8080)

    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Connect the socket to the server
        sock.connect(server_address)

        # Define the message in bytes
        message = bytes([10, 0, 0, 0, 1, 0, 1, 0, 0, 0, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63])

        # Send the message to the server
        sock.sendall(message)
        print(f"Sent: {message}")

        # Properly shutdown the sending side of the socket
        #sock.shutdown(socket.SHUT_WR)

        # Wait for 2 seconds before closing the socket
        time.sleep(2)

    except Exception as e:
        print(f"An error occurred: {e}")

    finally:
        # Close the socket
        sock.close()
        print("Socket closed")

if __name__ == "__main__":
    main()
