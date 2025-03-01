# WORK IN PROGRESS (suspended - see Preface and Endnotes)

# Solar Collector Shape Optimizer

This project aims to optimize the shape of a solar collector to maximize its efficiency in reflecting light onto a predefined obstacle. It uses a genetic algorithm to evolve the shape of the collector, represented as a 3D mesh. The project is written in C++23 and utilizes the STL.

## Preface

Originally I wrote this project in 2021/2022. Its performance was terrible, so I used CUDA kernels to speed the execution up. I wanted to revisit this project for I thought I could improve it. I changed its structure to a more standardized one, refactored it to be easier to maintain and understand, made it 100x-1000x faster (on CPU) via various means, changed the data structure from an AoS (array of Structures) to an SoA (Structure of Arrays) architecture to maximize parallel performance, added another build target for easy building in Termux (a terminal emulator for Android), removed duplicated and/or unnecessary code, added a ton of new functionalities...

It never occured to me that using a Genetic Algorithm to optimize a 3D mesh starting from random noise might **not be the most efficient solution**. Therefore, consider this work an adventureous endeavor of a curious mind, and **not** as a definitive solution to the given problem. I recommend (for your own sake) exploring existing libraries for 3D mesh optimization (which may or may not be using GA) and clearly defining the problem before attempting to solve it. More on this in the Endnotes.

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
    -   **`stats.hpp`**: Header file for `stats.cpp`.

## Dependencies

-   C++23 compliant compiler
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

Example (also provided in `config.cfg` file):

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

The code uses a preprocessor macro `#ifndef NO_STD_EXECUTION` to conditionally compile with either `std::execution::par_unseq` for parallel execution using the C++ standard library or OpenMP pragmas (`#pragma omp parallel for`) if `NO_STD_EXECUTION` is defined.  This allows for flexibility in choosing the parallel execution backend (convenient when working with older architectures).

## Potential Improvements

-   **More general obstacle handling:**  Setting up custom obstacle could be imrpoved and better documented (now it needs deeper knowledge of this project's structure).
-   **Adaptive mutation:** Dynamically adjust the mutation rate and range based on the progress of the genetic algorithm.
-   **GUI:** Create a graphical user interface to visualize the simulation and control parameters.

## Endnotes

This project was supposed to help me with coming up with an optimised shape of reflecting surfaces for solar thermal collector farm. I noticed early on that it would be faster (and perhaps easier) to use Mathematical Programming (future project?), as the domain would likely be convex. I decided to continue this project, as I had already invested extensive amount of time in it. This repository represents the state of the project as I'm leaving it at, due to time constraints and exhaustion of the knowledge base it was able to provide (for now).

This project taught me a lot:

-	**Pattern recognition in Coding:**
	+	**Problem-related:** Many problems share the same root and can be approached similarly.
	+	**Visual Code Inspection:** Getting around the code (both new and old) by skimming and not reading every single line to understand it.
-	**LLM Use:**
	+	Massive **boost in productivity**, if applied correctly.
	+	**"Fallback pattern-recognition engine":** For structuring fundamentals in a new domain (e.g., making sense of new information in new environments).
-	**Working with Documentation (C++ reference):** Complex constructs can lead to shorter, less bug-prone code (another area where LLMs are helpful).
-	Importance of **Consulting Experts** and **Asking the Right Quesions:**
	+	You can't learn every aspect of every tool you use.
	+	Other people have likely stumbled upon **same (or symmetric) problems** and left a trace of their knowledge somewhere on the Internet - **use it**.
	+	You **can do anything**, but you **can't do everything**.

### Takeaways

-	**Programming is a tool**, not a musical instrument. It's used to achieve results, not for virtuosity (though there are exceptions).
-	**Don't write your own implementation** of existing tools without a reason (be it educational, performance-related, licensing, or other **well-defined** one).
-	**Don't optimize prematurely**. Ensure the code is bug-free, implement all functionalities, **finish** what you set out to build - then you can think about *profiling* and finally *optimising*.
-	Clash your current project state and plans against a **priority inversion** scenario (a concept well-known in sheduling).
-	Once in a while **consider stepping away** from focused work on solving specific subproblems and look at the **bigger picture** of what's actually happening:
	+	**Look Back** at the path from the past to now:
		*	What were the project's initial assumptions?
		*	Weren't they already met at some point?
		*	Have they evolved, and if so, how?
	+	**Look Around**, as the world changes rapidly:
		*	Are the assumptions still relevant?
		*	Is there still a need (internal or external) for what you're working on?
		*	Are there any new tools that can help (enhance productivity, speed up testing and deployment, etc.)?
	+	**Look Ahead**, check and revise your plans:
		*	Is the (sub)problem you're working on actually important?
		*	Will it save money or time, or bring any other added value?
		*	How does your roadmap compare to real life?
	+	**Look Ahead (IRL)**, as there might be something move valuable (monetarily or otherwise) on the horizon:
		*	Does the thing you're pursuing actually matter **TO YOU**?
		*	What are your priorities?
		*	Are you on a path you want to be on?

### Last words

*Every project you'll ever work on will be unfinished, as the depth of any one problem is infinite.*  
Consider properly defining a scope of your project and constraining yourself to a set of requirements and assumptions - loosen them *when needed*, and tighten them *whenever possible*.
