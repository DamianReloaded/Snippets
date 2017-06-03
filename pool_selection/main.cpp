/*
OwlSL (Owl Script Language)

Copyright (c) 2013-2014 Damian Reloaded <>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <iostream>
#include <vector>
#include <numeric>
#include <ctime>
int main()
{
	// Seed random generator (std::rand is awful but's what we got)
	std::srand(std::time(0));

	// array of fitness values
	std::vector<size_t> fitnesses;
	fitnesses.push_back(2050);	// 0
	fitnesses.push_back(1800);	// 1
	fitnesses.push_back(1500);	// 2
	fitnesses.push_back(789);	// 3
	fitnesses.push_back(456);  	// 4
	fitnesses.push_back(123);  	// 5
	fitnesses.push_back(12);  	// 6

	// for logging results
	std::vector<size_t> summary(fitnesses.size());

	// Sum of all fitnesses
	size_t total = std::accumulate(fitnesses.begin(), fitnesses.end(), 0);

	while (true)
	{
		// Offset for the next interval
		size_t accum = fitnesses[0];

		// Pick a random number between 0 and the sum of all fitnesses
		// You should really consider using a random generator other than std::rand()
		size_t val = std::rand()%((int)total);

		// flags
		size_t selected = 0;
		bool found = false;

		// If the random value is in the range of the first value, select the first value
		if (val<accum)
		{
			selected = 0;
			found = true;
		}

		// Iterate all other firnesses and look for a matching range
		for (size_t i=1; i<fitnesses.size() && !found; i++)
		{
			// If the Random value is more than the offset but less than offset+current_fitness, select and exit
			if (val>=accum && val < accum + fitnesses[i])
			{
				selected = i;
				found = true;
				break;
			}
			accum += fitnesses[i];
		}

		summary[selected] += 1;

		system("cls");
		for (size_t i=0; i<summary.size(); i++)
		{
			std::cout << summary[i] << std::endl;
		}
	}
 	return 0;
}
