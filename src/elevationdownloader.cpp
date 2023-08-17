// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "eleman/elevationdownloader.h"

#include "eleman/elevationmanager.h"

#include <math.h>

eleman::ElevationDownloader::ElevationDownloader(ElevationCache* cache)
{
	this->cache = cache;
}

eleman::ElevationDownloader::~ElevationDownloader()
{

}


void eleman::ElevationDownloader::run()
{
	while(running)
	{
		printf("ElevationDownloader run\n");
		download();
	}
}

void eleman::ElevationDownloader::start()
{
	running = true;
	printf("Concurrency %d\n", thread.hardware_concurrency());
	thread = std::thread(&ElevationDownloader::run, this);
}

void eleman::ElevationDownloader::stop()
{
	running = false;
	thread.join();
}

void eleman::ElevationDownloader::setMode(const eleman::Mode& mode)
{
	this->mode = mode;
}

void eleman::ElevationDownloader::setRegion(double latitude0, double longitude0, double latitude1, double longitude1)
{
	regionLatitude0 = latitude0;
	regionLatitude1 = latitude1;
	regionLongitude0 = longitude0;
	regionLongitude1 = longitude1;
}

void eleman::ElevationDownloader::setTarget(DownloadTarget* target)
{
	if(targets.size() == 0)
		targets.push_back(target);
	else
		targets[0] = target;
}

void eleman::ElevationDownloader::addTarget(eleman::DownloadTarget* target)
{
	targets.push_back(target);
}

void eleman::ElevationDownloader::removeTarget(eleman::DownloadTarget* target)
{
	targets.erase(std::remove(targets.begin(), targets.end(), target), targets.end());
}


void eleman::ElevationDownloader::setTargets(std::vector<DownloadTarget *> targets)
{
	this->targets = targets;
}

size_t eleman::ElevationDownloader::targetCount()
{
	return targets.size();
}



void eleman::ElevationDownloader::download()
{
	switch(mode)
	{
		case NONE:
			downloadNone();
			break;

		case TARGET_SINGLE:
			downloadTargetSingle();
			break;

		case TARGET_MULTIPLE:
			downloadTargetMultiple();
			break;

		case MISSING:
			downloadMissing();
			break;

		default:
			throw std::runtime_error("Unknown download mode");
	}
}

void eleman::ElevationDownloader::downloadNone()
{
	// AVOID BUSY WAITING IF THIS MODE IS SELECTED
	std::this_thread::sleep_for (std::chrono::seconds(1));
}

void eleman::ElevationDownloader::downloadTargetSingle()
{
	downloadTarget(*targets[0]);
}

void eleman::ElevationDownloader::downloadTargetMultiple()
{
	for(DownloadTarget* target : targets)
	{
		downloadTarget(*target);
	}
}

void eleman::ElevationDownloader::downloadMissing()
{
	std::vector<CacheMiss> missing;
	cache->reportMissing(missing, cache->getManager()->getVendor()->getLocationsPerRequest());
	if(missing.size() > 0)
		cache->processMissing(missing);
}





void eleman::ElevationDownloader::downloadTarget(eleman::DownloadTarget& target)
{
	uint32_t count = cache->precacheRadius(target.latitude, target.longitude, target.radius);
	printf("Downloaded %d points with radius %f\n", count, target.radius);


	// TODO More sophisticated calculation of radius to better control the amount of requests
	double factor = cache->getManager()->getVendor()->getLocationsPerRequest() / double(count);
	if(count == 0 || factor == NAN || factor == INFINITY) factor = 1.0;
	factor = sqrt(factor);
	printf("---> Factor = %f\n", factor);


	target.previousRadius = target.radius;
	target.radius = std::min(target.radius + cache->getPrecision() * factor, target.maxRadius);
}


