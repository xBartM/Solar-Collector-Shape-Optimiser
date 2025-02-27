# WORK IN PROGRESS (suspended - read Preface and Endnotes)

# Solar Collector Shape Optimizer

This project aims to optimize the shape of a solar collector to maximize its efficiency in reflecting light onto a predefined obstacle. It uses a genetic algorithm to evolve the shape of the collector, represented as a 3D mesh. The project is written in C++23 and uses the STL.

## Preface

Originally I wrote this project in 2021/2022. It's performance was terrible but it used CUDA kernels to speed the execution up. I wanted to revisit this project for I thought I could make it better. I changed it's structure to a more standardized one, refactored it so it's easier to maintain and understand, I made it 100x-1000x faster (on CPU) via various means, I changed data structure from AoS to SoA architecture to maximize parallel performance, I added another build target so it's easy to build on Termux (terminal emulator for Android), I removed the duplicated and/or unnecessary code, I added a ton of new functionalities...

It never occured to me, that using Genetic Algorithm to optimize a 3D mesh (that starts out as random noise) might **not be a viable solution**. So please, consider this work as an adventureous endeavor of a curious mind, and **NOT** as a solution to the given problem. Make yourself a favour and use existing libraries for 3D mesh optimization (might be using GA, might not) and define a problem before attempting to solve it. More on that at the end of this file.

## Project Structure

The project is organized into the following files:

-   **`Makefile`**:  Builds the project, supporting both x86-64 and ARMv7 architectures.
-   **`config.cfg`**:  Configuration file for the genetic algorithm and simulation parameters.
-   **`Solar-Collector-Shape-Optimiser/`**:  Directory containing the core source code.
    -   **`main.cpp`**:  Main program entry point.  Handles the genetic algorithm loop, population management, and I/O.
    -   **`config.cpp`**:  Parses the `config.cfg` file and stores configuration settings in static members.
    -   **`config.hpp`**:  Header file for `config.cpp`.  Defines the `Config` class.
    -   **`genome.cpp`**:  Implements the `Genome` class, representing the genetic information of a solar collector. Includes crossover, mutation, and serialization.
    -   **`genome.hpp`**:  Header file for `genome.cpp`.
    -   **`mesh3d.cpp`**:  Implements the `Mesh3d` class, representing a 3D mesh.  Handles STL import/export (both ASCII and binary), vertex/normal calculations, and bounding box calculations.
    -   **`mesh3d.hpp`**:  Header file for `mesh3d.cpp`.  Defines the `Mesh3d`, `vertex`, and `triangle` structures.
    -   **`solarcollector.cpp`**:  Implements the `SolarCollector` class.  This class inherits from `Genome` and represents a single solar collector instance. It includes methods to compute the mesh, calculate fitness, and interact with the obstacle.
    -   **`solarcollector.hpp`**:  Header file for `solarcollector.cpp`.
    -   **`stats.cpp`**: Implements a simple statistics class to track and display timing information for different parts of the program.
    -   **`stats.hpp`**: Header file for `stats.cpp`

## Dependencies

-   C++23 compliant compiler
-   STL (Standard Template Library)
-   For x86-64 (default)
	+   TBB (Threading Building Blocks) library (`-ltbb`) for parallel execution.
-	For ARMv7 (e.g., Termux on an old 32-bit smartphone)
	+   ARM compiler (e.g., `arm-linux-androideabi-g++`)
	+   OpenMP for parallel execution (`-fopenmp` flag).

## Building the Project

The `Makefile` provides targets for building the project on x86-64 and ARMv7 architectures.

### x86-64 (Default)

```bash
make
```

This will create an executable named `solar_optimiser`.

### ARMv7

```bash
make armv7
```

This will create an executable named `solar_optimiser_armv7`.
The `Makefile` automatically uses `-fopenmp` for the ARMv7 target.

### Cleaning

```bash
make clean
```

This removes all compiled object files and executables.

## Configuration (config.cfg)

The `config.cfg` file controls the parameters of the genetic algorithm and the simulation:

-   **`xsize`**:  Width of the solar collector (integer).
-   **`ysize`**:  Length of the solar collector (integer).
-   **`hmax`**:  Maximum height of the solar collector (integer).
-   **`popsize`**:  Population size (integer, must be > 0, ideally divisible by 4).
-   **`crossover_bias`**:  Probability of selecting a gene from the first parent during crossover (double, 0.0 to 1.0).
-   **`mutation_probability`**:  Probability of mutating a gene (double, 0.0 to 1.0).
-   **`mutation_range`**:  Maximum range of mutation (double).
-   **`termination_ratio`**: Fraction of the population to be replaced in each generation (double, 0.0 to 1.0).
-   **`checkpoint_every`**:  Number of generations between saving checkpoints (integer).
-   **`export_every`**:  Number of generations between exporting the best individual as an STL file (integer).
-   **`start_from_checkpoint`**:  Whether to load the population from a checkpoint (boolean, `true` or anything else for false).
-   **`ray`**:  Direction of the incoming light ray, specified as `x,y,z` (doubles). Multiple rays can be specified by adding multiple `ray` lines.

Example (provided also in `config.cfg` file):

```config
# Example configuration
xsize=180
ysize=940
hmax=180
popsize=240
crossover_bias=0.6
mutation_probability=0.05
mutation_range=0.225
termination_ratio=0.5
checkpoint_every=100
export_every=25
start_from_checkpoint=true
ray=0,-1,0
```

## Running the Program

```bash
./solar_optimiser <config_file_path>
```

For example:

```bash
./solar_optimiser ./config.cfg
```

The program will output the fitness of each individual in each generation to **standard output**, in a CSV-like format (semicolon-separated). It also outputs timing statistics to **standard error**.  The best individual's mesh is exported to an STL file every `export_every` generations.  Checkpoints are saved to the `./checkpoint/` directory every `checkpoint_every` generations.  The program creates `.genome` files for each individual in the population, allowing the simulation to be resumed from a checkpoint.

**Important Note about Obstacle File:**  You *must* provide an obstacle file named `obstacleBin.stl` in the same directory as the executable.  This file represents the target object that the solar collector should reflect light onto. The program expects this file to be in *binary* STL format.

## Checkpointing

The program saves checkpoints of the population to the `./checkpoint/` directory.  Each individual is saved as a separate `.genome` file.  If `start_from_checkpoint` is set to `true` in `config.cfg`, the program will attempt to load the population from these files. If an error occurs while loading a checkpoint, it will skip to creating a new random Genome, and will stop trying to load checkpoints (so it won't try to load the rest of the files).

## Output

-   **Standard Output:**  CSV-like output of the generation number and the fitness of each individual in the population.
-   **Standard Error:**  Timing statistics for various parts of the program.
-   **STL Files:**  The mesh of the best individual is exported as an STL file (e.g., `Gen100Fit500.stl`) every `export_every` generations.
-   **Checkpoint Files:**  `.genome` files are saved in the `./checkpoint/` directory, allowing the simulation to be resumed.

## Parallelism

The code uses `#ifndef NO_STD_EXECUTION` to conditionally compile with either `std::execution::unseq` for parallel execution using the C++ standard library or OpenMP pragmas (`#pragma omp parallel for`) if `NO_STD_EXECUTION` is defined.  This allows for flexibility in choosing the parallel execution backend (important when working with old architectures).

## Potential Improvements

-   **More general obstacle handling:**  Setting up custom obstacle needs some knowledge of this project's structure.
-   **Adaptive mutation:** Dynamically adjust the mutation rate and range based on the progress of the genetic algorithm.
-   **GUI:** Create a graphical user interface to visualize the simulation and control parameters.

## Endnotes

This project taught me a lot: from pattern recognition in coding (both problem related and visual code inspection related)

### Takeaways

-	**Don't write your own implementation** of existing tools without a reason (be it educational, performance related, license related, or other **WELL DEFINED** one).
-	**Don't optimize prematurely** -- ushort * ushort / ushort fits in ushort but it still **overflows**
-	Clash your current state and plans against **priority inversion** scenario (concept well-known in sheduling).
-	Once in a while **consider stepping away** from focused work on how to solve a set subproblems, and take a look at the **bigger picture** of what's actually happening:
	+	**Look back** at the path from the past to now:
		*	What were the projects assumptions at the beginning?
		*	Weren't they already met at some point?
		*	Have they evolved? And how?
	+	**Look around** as the world changes rapidly:
		*	Are the assumptions still relevant?
		*	Is there still a need (internal or external) for what you're working on?
		*	Are there any new tools that can help (enhance productivity, speedup testing and deployment, etc.)?
	+	**Look ahead**, check and revise your plans:
		*	Is the subproblem in your project you're working on actually important? 
		*	Will it save money or time or bring any other added value?
		*	How does your roadmap compare against real life?
	+	**Look ahead (IRL)** as there might be something move valuable (monetary or otherwise) on the horizon:
		*	Does the thing you're pursuing actually matter **TO YOU**?
		*	What are your priorities?
		*	Are you on a path you want to be at?
		*	



