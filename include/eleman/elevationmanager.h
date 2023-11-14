// SPDX-FileCopyrightText: 2023 <rudolf.ortner> <rudolf.ortner.rottenbach@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef ELEVATIONMANAGER_H
#define ELEVATIONMANAGER_H

#include "elevationcache.h"
#include "elevationdata.h"
#include "elevationvendor.h"

#include <chrono>
#include <map>
#include <mutex>

namespace eleman
{

	const uint8_t VERSION_MAJOR = 0;
	const uint8_t VERSION_MINOR = 2;
	const uint8_t VERSION_PATCH = 0;
	const std::string VERSION_STRING = std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_PATCH);
	const std::string USER_AGENT = "eleman/" + VERSION_STRING;

	/**
	* @todo write docs
	*/
	class ElevationManager
	{
	public:
		/**
		* Default constructor
		*/
		ElevationManager();

		/**
		* Destructor
		*/
		~ElevationManager();

		// Raw request but managed (to avoid exceeding rate-limit)
		VendorResponse requestManaged(std::vector<Position> positions);
		// Raw requests, no caching
		VendorResponse requestRaw(Position position);
		VendorResponse requestRaw(std::vector<Position> positions);
		// TODO GRID

		// Request methods
		// TODO request single location
		ElevationData get(double latitude, double longitude, ElevationRegion::Interpolation interpolation = ElevationRegion::LINEAR);
		ElevationData get(Position pos, ElevationRegion::Interpolation interpolation = ElevationRegion::LINEAR);
		// TODO request multiple points
		std::vector<ElevationData> get(const std::vector<Position>& positions, ElevationRegion::Interpolation interpolation = ElevationRegion::LINEAR);
		// TODO request grid of points
		ElevationRegion get(double lat0, double lon0, double lat1, double lon1, double precision, ElevationRegion::Interpolation interpolation = ElevationRegion::LINEAR);
		ElevationRegion get(double lat0, double lon0, double lat1, double lon1, uint32_t gridSizeLat, uint32_t gridSizeLon, ElevationRegion::Interpolation interpolation = ElevationRegion::LINEAR);
		void fillRegion(ElevationRegion& region, ElevationRegion::Interpolation interpolation = ElevationRegion::LINEAR);


		// Precache methods

		uint32_t getTotalRequests();



		void setCache(ElevationCache* cache);
		ElevationCache* getCache();
		void setVendor(ElevationVendor* vendor);
		ElevationVendor* getVendor();

		void setUserAgent(const std::string& userAgent);
		std::string getUserAgent();

	private:
		ElevationCache* cache;
		ElevationVendor* vendor;

		std::mutex requestMutex;
		std::map<ElevationVendor*, std::chrono::time_point<std::chrono::steady_clock>> lastRequest;
		std::map<ElevationVendor*, uint32_t> totalRequests;

		std::string userAgent = USER_AGENT;
	};

}	// end namespace eleman
#endif // ELEVATIONMANAGER_H
