import subprocess
import time
import numpy as np
import matplotlib.pyplot as plt
from scipy import stats


def run_server(p):
    result = subprocess.Popen(['./server', str(p)])
    return result


def run_client():
    result = subprocess.run(['./client'])
    return result


def measure_time_for_p(p, num_trials=10):
    completion_times = []

    for _ in range(num_trials):
        run_server(p)
        time.sleep(1)

        start_time = time.time()
        run_client()
        end_time = time.time()

        completion_times.append(end_time - start_time)

    return completion_times


def calculate_avg_and_confidence_intervals(times):
    avg_time = np.mean(times)
    confidence_interval = stats.sem(
        times) * stats.t.ppf((1 + 0.95) / 2., len(times)-1)
    return avg_time, confidence_interval


def run_experiment():
    p_values = list(range(1, 11))
    avg_times = []
    confidence_intervals = []

    for p in p_values:
        print(f"Running experiment for p = {p}")
        completion_times = measure_time_for_p(p)
        avg_time, confidence_interval = calculate_avg_and_confidence_intervals(
            completion_times)
        avg_times.append(avg_time)
        confidence_intervals.append(confidence_interval)

    return p_values, avg_times, confidence_intervals


def plot_results(p_values, avg_times, confidence_intervals):
    plt.errorbar(p_values, avg_times,
                 yerr=confidence_intervals, fmt='-o', capsize=5)
    plt.xlabel("Number of Words per Packet (p)")
    plt.ylabel("Average Completion Time (seconds)")
    plt.title("Average Completion Time for Different Values of p")
    plt.savefig("plot.png")


if __name__ == "__main__":
    p_values, avg_times, confidence_intervals = run_experiment()
    plot_results(p_values, avg_times, confidence_intervals)
