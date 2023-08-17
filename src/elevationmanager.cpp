// SPDX-FileCopyrightText: 2023 <rudolf.ortner> <rudolf.ortner.rottenbach@gmail.com>
// SPDX-License-Identifier: MIT

#include "eleman/elevationmanager.h"

#include "eleman/elevationdata.h"

#include <cmath>
#include <string>
#include <sstream>
#include <thread>


eleman::ElevationManager::ElevationManager()
{

}

eleman::ElevationManager::~ElevationManager()
{

}

eleman::VendorResponse eleman::ElevationManager::requestManaged(std::vector<Position> positions)
{
	std::unique_lock<std::mutex> lock(requestMutex);

	printf("requestManaged\n");
	double timeDiff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastRequest[vendor]).count();
	double wait = 1000.0 / vendor->getRequestsPerSecond() - timeDiff;
	printf("TimeDiff %f\n", timeDiff);
	if(wait > 0.0)
	{
		printf("Waiting for %fms\n", wait);
		std::this_thread::sleep_for(std::chrono::milliseconds((long) wait));
	}

	// Actual Request
	auto start = std::chrono::steady_clock::now();
	VendorResponse response = vendor->request(positions, userAgent);
	auto ende = std::chrono::steady_clock::now();
	double duration = std::chrono::duration_cast<std::chrono::milliseconds>(ende - start).count();
	printf("Request took %fms\n", duration);

	// Update statistics
	lastRequest[vendor] = std::chrono::steady_clock::now();
	totalRequests[vendor]++;

	return response;
}

eleman::VendorResponse eleman::ElevationManager::requestRaw(Position position)
{
	return requestRaw(std::vector<Position>({position}));
}

eleman::VendorResponse eleman::ElevationManager::requestRaw(std::vector<Position> positions)
{
	uint16_t splitSize = vendor->getLocationsPerRequest();
	if(positions.size() <= splitSize)
		return requestManaged(positions);

	printf("Request too large -> splitting\n");
	eleman::VendorResponse response;
	response.code = OK;

	size_t splits = ceil(double(positions.size()) / splitSize);
	for(size_t split = 0; split < splits; split++)
	{
		size_t start = split * splitSize;
		size_t end = std::min(start + splitSize, positions.size());

		auto first	= positions.begin() + start;
		auto last	= positions.begin() + end;

		std::vector<Position> splitPos(first, last);

		eleman::VendorResponse splitResponse = requestManaged(splitPos);
		response.results.insert(response.results.end(), splitResponse.results.begin(), splitResponse.results.end());
		if(response.code == OK)
		{
			response.code	= splitResponse.code;
			response.error	= splitResponse.error;
		}
	}

	return response;
}

eleman::ElevationData eleman::ElevationManager::get(double latitude, double longitude, ElevationRegion::Interpolation interpolation)
{
	return get({latitude, longitude}, interpolation);
}

eleman::ElevationData eleman::ElevationManager::get(Position pos, ElevationRegion::Interpolation interpolation)
{
	if(cache == nullptr)
		throw std::runtime_error("No cache is set");

	// TODO QUERY CACHE
	return cache->get(pos, interpolation);

}

std::vector<eleman::ElevationData> eleman::ElevationManager::get(const std::vector<Position>& positions, ElevationRegion::Interpolation interpolation)
{
	if(cache == nullptr)
		throw std::runtime_error("No cache is set!");

	// TODO QUERY CACHE
	return cache->get(positions, interpolation);
}

eleman::ElevationRegion eleman::ElevationManager::get(double lat0, double lon0, double lat1, double lon1, double precision, ElevationRegion::Interpolation interpolation)
{
	if(cache == nullptr)
		throw std::runtime_error("No cache is set!");

	return cache->get(lat0, lon0, lat1, lon1, precision, interpolation);
}

eleman::ElevationRegion eleman::ElevationManager::get(double lat0, double lon0, double lat1, double lon1, uint32_t gridSizeLat, uint32_t gridSizeLon, ElevationRegion::Interpolation interpolation)
{
	if(cache == nullptr)
		throw std::runtime_error("No cache is set!");

	return cache->get(lat0, lon0, lat1, lon1, gridSizeLat, gridSizeLon, interpolation);
}


uint32_t eleman::ElevationManager::getTotalRequests()
{
	return totalRequests[vendor];
}




void eleman::ElevationManager::setCache(eleman::ElevationCache* cache)
{
	this->cache = cache;
}

eleman::ElevationCache* eleman::ElevationManager::getCache()
{
	return cache;
}


void eleman::ElevationManager::setVendor(eleman::ElevationVendor* vendor)
{
	this->vendor = vendor;
}

eleman::ElevationVendor* eleman::ElevationManager::getVendor()
{
	return vendor;
}


void eleman::ElevationManager::setUserAgent(const std::string& userAgent)
{
	this->userAgent = userAgent;
}

std::string eleman::ElevationManager::getUserAgent()
{
	return userAgent;
}
