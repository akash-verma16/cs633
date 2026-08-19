import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Read the data
try:
    df = pd.read_csv("experiment_results.csv")
except FileNotFoundError:
    print("Error: experiment_results.csv not found. Run the shell script first.")
    exit()

# Set up the plot style
sns.set(style="whitegrid")

# Get unique M values to create subplots or separate clusters
m_values = df['M'].unique()

plt.figure(figsize=(12, 6))

# Use boxplots
# Time on y-axis, Processes (P) on x-axis
# We use 'hue' to differentiate by M (Data Size)
sns.boxplot(x="P", y="Time", hue="M", data=df)

plt.title("Execution Time per Process Count for Different Data Sizes (M)")
plt.xlabel("Number of Processes (P)")
plt.ylabel("Time (seconds)")
plt.legend(title="Data Size (M)")

# Save the plot
plt.savefig("execution_time_boxplot.png")
print("Plot saved as execution_time_boxplot.png")
plt.show()