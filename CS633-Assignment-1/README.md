Here is the corrected and complete `README.md` file. Ensure you save this content exactly as **README.md**.

```markdown
# Assignment 1: MPI Data Exchange & Computation

## Overview
This project implements a parallel program using MPI where processes exchange arrays of `M` doubles with neighbors at distances `D1` and `D2`. It performs computations (square and log) on received data and updates buffers for `T` iterations.

## Prerequisites
* **MPI Library:** OpenMPI or MPICH (must support `mpicc` and `mpirun`).
* **Python 3:** With `pandas`, `seaborn`, and `matplotlib` installed.
* **Compiler:** GCC.

## Project Structure
* `assignment1.c`: Main MPI source code.
* `run_experiments.sh`: Shell script to run the 5-repetition experiments.
* `plot_results.py`: Python script to generate boxplots.

---

## 1. Compilation
To compile the C program, use the following command. The `-lm` flag is essential for the math library (logarithm functions).

```bash
mpicc assignment1.c -o assignment1 -lm

```

---

## 2. Manual Execution

To run the program with a specific configuration manually:

```bash
mpirun -np <P> ./assignment1 <M> <D1> <D2> <T> <seed>

```

**Arguments:**

* 
`P`: Number of processes.


* 
`M`: Number of doubles per array.


* 
`D1`: Distance 1 neighbor offset.


* 
`D2`: Distance 2 neighbor offset.


* 
`T`: Number of iterations.


* 
`seed`: Random seed for data generation.



**Example:**

```bash
mpirun -np 16 ./assignment1 262144 2 4 10 1000

```

**Output:**
The program outputs a single line containing:

```
<maximumD1> <maximumD2> <time>

```

---

## 3. Running Automated Experiments

The assignment requires generating a report based on specific parameters () while varying  and  .

1. **Make the script executable:**
```bash
chmod +x run_experiments.sh

```


2. **Run the experiments:**
```bash
./run_experiments.sh

```


*This process may take a few minutes. It runs each configuration 5 times and saves the output to `experiment_results.csv`.*

---

## 4. Generating the Plot

Once the `experiment_results.csv` file is generated, run the Python script to create the boxplot.

1. **Run the script:**
```bash
python3 plot_results.py

```


2. **Check the output:**
* A file named `execution_time_boxplot.png` will be created.
* This plot shows the **Time (y-axis)** vs **Processes (x-axis)** grouped by **Data Size M** .





---

## Troubleshooting

* **`mpicc: command not found`**: Ensure MPI is installed and added to your PATH.
* **`Permission denied`**: Run `chmod +x run_experiments.sh` again.
* **Python Errors**: Install missing libraries using `pip install pandas seaborn matplotlib`.
* **Math errors**: If the code fails to compile with undefined references to `log`, ensure you included `-lm` at the **end** of the compile command.

```

```