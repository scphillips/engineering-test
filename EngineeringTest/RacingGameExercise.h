#pragma once

#include <vector>

namespace
{

// Added stub implementation to allow compilation
class Racer
{
public:
	bool isAlive() const { return true; }
	bool isCollidable() const { return true; }
	bool collidesWith(const Racer* other) const { return true; }
	void update(float deltaTimeMS) {}
};

void onRacerExplodes(Racer* racer) {}

} // namespace

inline std::vector<Racer*> createRacerCollection(int count)
{
	std::vector<Racer*> collection;
	for (int i = 0; i < count; i++)
	{
		collection.push_back(new Racer());
	}
	return collection;
}

// Original implementation, retained to compare results with new version
inline void updateRacers(float deltaTimeS, std::vector<Racer*>& racers)
{
	std::vector<Racer*> racersNeedingRemoved;
	racersNeedingRemoved.clear();

	// Fixed so racers are updated in reverse order now
	for (size_t nRacer = 0; nRacer < racers.size(); nRacer++)
	{
		if (racers[racers.size() - nRacer - 1]->isAlive())
		{
			//Racer update takes milliseconds
			racers[racers.size() - nRacer - 1]->update(deltaTimeS * 1000.0f);
		}
	}

	// Collides
	for (std::vector<Racer*>::iterator it1 = racers.begin(); it1 != racers.end(); it1++)
	{
		for (std::vector<Racer*>::iterator it2 = racers.begin(); it2 != racers.end(); it2++)
		{
			if (it1 != it2)
			{
				if ((*it1)->isCollidable() && (*it2)->isCollidable() && (*it1)->collidesWith(*it2))
				{
					onRacerExplodes(*it1);
					racersNeedingRemoved.push_back(*it1);
					racersNeedingRemoved.push_back(*it2);
				}
			}
		}
	}
	// Gets the racers that are still alive
	std::vector<Racer*> newRacerList;
	for (std::vector<Racer*>::iterator it = racers.begin(); it != racers.end(); it++)
	{
		// check if this racer must be removed
		std::vector<Racer*>::iterator racerIt = std::find(racersNeedingRemoved.begin(), racersNeedingRemoved.end(), *it);
		if (racerIt == racersNeedingRemoved.end())
		{
			newRacerList.push_back(*it);
		}
	}
	// Get rid of all the exploded racers
	for (std::vector<Racer*>::iterator it = racersNeedingRemoved.begin(); it != racersNeedingRemoved.end(); it++)
	{
		std::vector<Racer*>::iterator racerIt = std::find(racers.begin(), racers.end(), *it);
		if (racerIt != racers.end()) // Check we've not removed this already!
		{
			Racer* tmp = *racerIt;
			racers.erase(racerIt);
			delete tmp;
		}
	}
	// Builds the list of remaining racers
	racers.clear();
	for (std::vector<Racer*>::iterator it = newRacerList.begin(); it != newRacerList.end(); it++)
	{
		racers.push_back(*it);
	}

	for (std::vector<Racer*>::iterator it = newRacerList.begin(); it != newRacerList.end();)
	{
		it = newRacerList.erase(it);
	}
}

inline void updateRacersV2(float deltaTimeS, std::vector<Racer*>& racers)
{
	// TODO: Choose consistent time format
	float racerUpdateTick = deltaTimeS * 1000.0f;

	// Racers need to be updated in reverse order. TODO: Investigate importance of ordering
	for (auto it = racers.rbegin(); it != racers.rend(); ++it)
	{
		auto* racer = *it;
		if (racer->isAlive())
		{
			racer->update(racerUpdateTick);
		}
	}

	std::set<int> entriesToRemove;
	// Already empty on construction, no need to call clear()

	// Perform collision detection between racers
	size_t racersCount = racers.size();
	for (size_t i = 0; i < racersCount; i++)
	{
		Racer* lhs = racers[i]; // Micro-optimization, retrieve racer once here
		if (lhs->isCollidable())
		{
			bool lhsHasCollided = false;
			// Begin at it1 + 1, reduce loop from O(n^2) to O(n*(n-1)/2)
			for (size_t j = i + i; j < racersCount; j++)
			{
				Racer* rhs = racers[j];
				if (rhs->isCollidable() && lhs->collidesWith(rhs))
				{
					onRacerExplodes(lhs);
					lhsHasCollided = true; // Set flag rather than adding to entriesToRemove here to ensure it is added once

					// Optimised loop to only check collisions one way. We must now call onRacerExplodes for rhs here as well
					onRacerExplodes(rhs);
					entriesToRemove.insert(j);

					// TODO: Investigate behavior of onRacerExplodes; if lhs is no longer collidable, can skip to next racer.
				}
			}
			if (lhsHasCollided)
			{
				entriesToRemove.insert(i);
			}
		}
	}

	// Get rid of all the exploded racers. Work in reverse order to maintain iterators and minimise shuffling of memory
	for (auto it = entriesToRemove.crbegin(); it != entriesToRemove.crend(); it++)
	{
		delete racers[*it];
		racers.erase(racers.begin() + *it);
	}
}
