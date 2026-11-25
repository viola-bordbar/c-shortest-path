# Shortest Path Between Cities (Dijkstra's Algorithm in C)

This project was developed as part of an algorithms and data structures assignment at KTH. It loads a list of cities and distances from a CSV file, builds a graph, and computes the shortest path using Dijkstra's algorithm. The program also measures execution time and reports how many nodes were visited during the search.

## How to Run

Compile the program:

```bash
gcc -o shortest_path shortest_path.c
```

Run the program:

```bash
./shortest_path
```

Make sure the file `europe.csv` is in the same folder as the compiled program.

## CSV Format

The program expects the CSV file to contain city pairs and their distances:

```csv
CityA,CityB,Distance
CityB,CityC,Distance
CityA,CityD,Distance
```

Example:

```csv
Stockholm,Södertälje,21
Södertälje,Norrköping,59
Stockholm,Västerås,90
```

## Note

The project was originally developed on Windows and uses `QueryPerformanceCounter` for execution-time measurements.