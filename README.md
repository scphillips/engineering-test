# engineering-test

* [Introduction](#introduction)
* [Getting Started](#getting-started)
* [Features](#features)
  * [Exercise 1: Color-Matching](#exercise-1-color-matching)
  * [Exercise 2: Ball Physics](#exercise-2-ball-physics)
  * [Exercise 3: Racing Game](#exercise-3-racing-game)

## Introduction
A collection of three programming exercises written in C++.

## Getting Started
The project was created using Visual Studio 2017 Community Edition.
It uses C++11 and includes a Windows-specific console renderer.
It can be compiled directly on both 32 and 64 bit Windows machines.
Other platforms will require porting to replace the console functionality.

## Features
### Exercise 1: Color-Matching
Demonstrates a simple tiled board for matching colored cells by swapping pairs.
The board is pseudo-randomly generated from a fixed seed, and the most optimal move is calculated by traversing the grid and comparing the score each valid move would gain.
The result of the best move is printed to the console, including any cascading effects the move may trigger.

### Exercise 2: Ball Physics
A focused example function to calculate the position of a ball when it reaches a specified height, if at all. This solution goes through several stages to reach the answer:
* Derive velocity `v` at height `h` using integration of starting velocity plus acceleration with respect to displacement.
* If the result is negative, the path of the ball does not intersect with `h` and so no result can be determined.
* If positive, the time taken to reach velocity `v` is derived from the rate of acceleration `a` from starting velocity `u`.
* The potential horizontal position of the ball at time `t` is found by its starting position plus total displacement by its horizontal velocity.
* The final horizontal position is found by reflecting its position within its bounding area of zero to `w`.

### Exercise 3: Racing Game
This exercise takes an existing function in need of optimization and structural improvements. A new version, `updateRacersV2`, is provided to perform the same functionality in a more ideal way. Some basic rationale is provided in code comments, and a more comprehensive accompanying document lists how the function could be further improved upon.
