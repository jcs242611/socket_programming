import subprocess
import time
import matplotlib.pyplot as plt
import re


def start_server():
    return subprocess.Popen(['./server_exe'], stdout=subprocess.DEVNULL)


def run_client_and_measure_time(num_clients):
    process = subprocess.Popen(
        ['./client_exe', str(num_clients)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    client_times = []
    for line in process.stdout:
        line = line.decode('utf-8').strip()
        match = re.search(
            r'\[CLIENT \| TIME \| \d+\] time-taken=(\d+) ms', line)
        if match:
            time_taken = int(match.group(1)) / 1000
            client_times.append(time_taken)

    process.wait()

    avg_time_per_client = sum(client_times) / \
        num_clients if num_clients > 0 else None
    return avg_time_per_client


def main():
    print("Starting the server...")
    server_process = start_server()
    time.sleep(2)

    try:
        client_counts = list(range(1, 33, 4))
        avg_times = []

        for count in client_counts:
            print(f"\nRunning client program with {count} clients...")
            avg_time = run_client_and_measure_time(count)
            if avg_time is not None:
                avg_times.append(avg_time)
                print(f"Avg time per client: {avg_time:.3f}s")
            else:
                avg_times.append(None)
                print(f"Failed to get results for {count} clients.")

        plt.figure(figsize=(10, 6))
        plt.plot(client_counts, avg_times,
                 marker='o', linestyle='-', color='b')
        plt.title('Average Completion Time vs Number of Clients')
        plt.xlabel('Number of Clients')
        plt.ylabel('Average Completion Time (s)')
        plt.grid(True)
        plt.savefig("plot.png")
    finally:
        print("Terminating the server...")
        server_process.terminate()


if __name__ == "__main__":
    main()
