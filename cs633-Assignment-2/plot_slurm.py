import re
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# --- CONFIGURATION ---
# In the filename variable, put the actual name of the file downloaded from the cluster
filename = "slurm.out" 

def parse_slurm_file(filepath):
    data = []
    current_P = None
    current_M = None
    
    # Regex to find the parameter line: "Run 1 | P=8 M=262144 ..."
    param_pattern = re.compile(r"P=(\d+)\s+M=(\d+)")
    
    # Regex to find the result line: "9987... 14.16... 0.1123"
    # Looks for 3 floating point numbers separated by whitespace
    result_pattern = re.compile(r"^\s*[\d\.]+\s+[\d\.]+\s+([\d\.]+)")

    try:
        with open(filepath, 'r') as f:
            for line in f:
                # 1. Check if line contains parameters
                param_match = param_pattern.search(line)
                if param_match:
                    current_P = int(param_match.group(1))
                    current_M = int(param_match.group(2))
                    continue

                # 2. Check if line contains the output (Time)
                # We only parse data if we have seen a "Run" line previously
                if current_P is not None and current_M is not None:
                    result_match = result_pattern.search(line)
                    if result_match:
                        time_val = float(result_match.group(1))
                        
                        data.append({
                            "P": current_P,
                            "M": current_M,
                            "Time": time_val
                        })
    except FileNotFoundError:
        print(f"Error: The file '{filepath}' was not found.")
        print("Please make sure you downloaded the .out file from the cluster.")
        return []

    return data

# --- MAIN EXECUTION ---
data_points = parse_slurm_file(filename)

if not data_points:
    print("No data found! Check the filename or the file content format.")
else:
    print(f"Successfully parsed {len(data_points)} data points.")
    df = pd.DataFrame(data_points)

    # Plotting
    plt.figure(figsize=(10, 6))
    sns.set(style="whitegrid")

    # Create Boxplot
    sns.boxplot(x="P", y="Time", hue="M", data=df, palette="Set2")

    plt.title("Execution Time vs. Processes (Parsed from SLURM Log)")
    plt.xlabel("Number of Processes (P)")
    plt.ylabel("Time (seconds)")
    plt.legend(title="Data Size (M)")
    
    output_img = "final_plot.png"
    plt.savefig(output_img, dpi=300)
    print(f"Plot saved as {output_img}")
    plt.show()