from collections import defaultdict
import os
import signal
import tempfile
import subprocess
import shutil

SERVER_PORT = 5555
SERVER_BINARY = './server'
CLIENT_BINARY = './client'
TEST_FOLDER = "./random_files"


def start_server():
    server = subprocess.Popen([SERVER_BINARY, f"{SERVER_PORT}"], stdout=subprocess.PIPE)
    return server

def _generate_folder_with_random_files(number_of_files):
    shutil.rmtree(TEST_FOLDER, ignore_errors=True)
    os.mkdir(TEST_FOLDER)

    for _ in range(number_of_files):
        tmp = tempfile.NamedTemporaryFile(dir=TEST_FOLDER, delete=False)
        tmp.write(os.urandom(1024))

def _run_client_on_file(file_name, number_of_readable_chars):
    p = subprocess.run([CLIENT_BINARY, "127.0.0.1", f"{SERVER_PORT}", file_name],
                       capture_output=True, text=True)

    if p.stderr:
        raise Exception(f"FAILED: {p.stderr}")

    assert p.stdout == f"# of printable characters: {number_of_readable_chars}\n", f"file name: {file_name}\nexpected number of readable: {number_of_readable_chars}\nclient output: {p.stdout}\n"


def run_client_on_folder():
    printalbe_characters = defaultdict(int)

    for file in os.listdir(TEST_FOLDER):
        file_full_path = f"{TEST_FOLDER}/{file}"
        with open(file_full_path, 'rb') as f:
            current_printalbe_characters = 0

            while (byte := f.read(1)):
                if b'\x20' <= byte <= b'\x7E':
                    printalbe_characters[byte.decode("ascii")] += 1
                    current_printalbe_characters += 1

            _run_client_on_file(file_full_path, current_printalbe_characters)

    return printalbe_characters

def parse_server_output(server, printalbe_characters):
    server.send_signal(signal.SIGINT)
    server_stdout, _ = server.communicate()
    for line in server_stdout.decode('latin-1').split('\n')[:-1]:
        symbol = line[6:7]
        print(line)
        count = int(line[10:].split(' ')[1])
        assert printalbe_characters[symbol] == count, f"Wrong count for symbol, symbol: '{symbol}' expected count: {printalbe_characters[symbol]}, actual count: {count}, line: {line}"


def run_tests(number_of_clients):
    server = start_server()
    _generate_folder_with_random_files(number_of_clients)
    try:
        printalbe_characters = run_client_on_folder()
        parse_server_output(server, printalbe_characters)
        print("PASSED")
    except Exception as e:
        print("FAILED")
        raise e
    finally:
        server.kill()


if __name__ == '__main__':
    run_tests(30)
