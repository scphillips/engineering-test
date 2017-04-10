# Code discussion: `updateRacers()`
--------
## Current behaviour
This function is currently responsible for three distinct, CPU-intensive phases of the game update loop.
* Calling `update()` on each Racer in turn;
* Performing collision checks against each Racer;
* Deleting every Racer that collided.

## Drawbacks of current implementation
The current collection of Racers is passed as a modifiable reference to an `std::vector`. This is required in order to perform the third phase of deleting Racers that have collided, but also means that code outside this function cannot rely on the collection being a fixed size. Any threaded work that attempts to read (or indeed modify) this collection runs the risk of bad access exceptions.

The Racers are stored as pointers within the collection. As above, this is required to delete Racers that have collided, but the function does not perform any checks against nullptr values. Passing a collection of references would better indicate to the reviewer that these Racer entries cannot be null. In addition, it is only possible to run this function using dynamically allocated Racers (using the `new` keyword), as the corresponding `delete` call will trigger an exception on other types of allocations.

The `update()` function of each Racer seems to be dependent on a specific order, though the significance of this order is not made clear. For additional reasons listed below, it is important that this restriction be identified and fixed, as it is highly beneficial to allow more fluid ordering of the collection.

The collision checks are handled in an inefficient O(n^2) loop, where one thousand Racers would require 999,000 comparisons. This has been optimised to only check in one direction, resulting in 499,500 checks in this example, but it is still a very intensive calculation. There are several divide-and-conquer strategies that could be used, some of which are listed in the proposed changes below. Ultimately, is it believed that the collision checks are too costly to be run in the same function call and thread as the game update loop.

The function builds a collection of `racersNeedingRemoved`, populated by all Racers that have collided in that frame. This is a very specific and restrictive approach, as there are any number of game-related actions that could be triggered by a collision. For example, the game could spawn an explosion and allow the physics engine to throw the Racer's model around for a few seconds, perhaps colliding with additional Racers, before it is cleaned up. This also assumes that a Racer can only receive a single collision regardless of force before it is removed from the game.

## Proposed changes to improve this function
The `updateRacers` function has full access to the game's collection of Racers, and relies on the state of each Racer in order to operate correctly. Instead of manually checking `isAlive` before calling `update`, there should be a clear interface to retrieve a collection of "live" Racers to be updated.

A consistent tick value would be more easily understood by the reviewer, either by adopting the `std::chrono` types or by using seconds or milliseconds throughout.

The collision checks should be put into a more sophisticated data structure to alleviate the sheer number of collision checks involved. If a physics engine is deemed too heavyweight a solution, the Racers could be stored in a quad tree (assuming this is a racing game played on a largely 2D surface) to reduce the algorithmic complexity from O(n^2) to a best case of O(n*log(n)). This would also be an intuitive way of dividing the input data to be processed, and so could be offloaded to additional threads.

The result of a collision between Racers is not clearly defined or exposed for other parts of the codebase to make use of. The current `onRacerExplodes` function does not provide any details of _why_ the Racer exploded, preventing features such as a DeathCam to follow the other Racer which caused its demise (who also exploded). Unfortunately, amending this by adding the other Racer into the function would mean that colliding with a wall or similar would cause a messy situation of perhaps passing a null Racer. A better approach would be to remove this behaviour, and introduce a separate system for handling changes in the state of a Racer. This would allow more complex game mechanics such as health pools and variable amounts of damage from the force of collisions (or weapons), with the added bonus of keeping the `updateRacers` function clean and concise.

Directly manipulating the `racers` collection limits the flexibility of the game, as it cannot be read by other threads or have other code observing when the collection has been modified. It also relies on the `new` and `delete` keywords for keeping track of active Racers, which limits potential options for optimising the code. Keeping the collection of Racers in a contiguous block of memory (as opposed to a contiguous vector of _pointers_ to widely-distributed blocks of memory) can be leveraged for significant performance boosts, especially on slow or otherwise limited hardware.
