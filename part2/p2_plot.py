import subprocess
import time
import matplotlib.pyplot as plt


def start_server():
    return subprocess.Popen(['./server_exe'], stdout=subprocess.DEVNULL)


def run_client_and_measure_time(num_clients):
    start_time = time.time()
    subprocess.run(['./client_exe', str(num_clients)])
    end_time = time.time()
    total_time = end_time - start_time
    avg_time_per_client = total_time / num_clients
    return total_time, avg_time_per_client


def main():
    client_counts = list(range(1, 33, 4))
    avg_times = []

    for count in client_counts:
        print("Starting the server...")
        server_process = start_server()
        time.sleep(1)
        print(f"\nRunning client program with {count} clients...")
        total_time, avg_time = run_client_and_measure_time(count)
        if avg_time is not None:
            avg_times.append(avg_time)
            print(f"Total time: {total_time:.3f}s | Avg time per client: {
                  avg_time:.3f}s")
        else:
            avg_times.append(None)
            print(f"Failed to get results for {count} clients.")
        print("Terminating the server...")
        server_process.terminate()

    plt.figure(figsize=(10, 6))
    plt.plot(client_counts, avg_times,
             marker='o', linestyle='-', color='b')
    plt.title('Average Completion Time vs Number of Clients')
    plt.xlabel('Number of Clients')
    plt.ylabel('Average Completion Time (s)')
    plt.grid(True)
    plt.savefig("plot.png")


if __name__ == "__main__":
    main()
