// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef ELEVATIONDOWNLOADER_H
#define ELEVATIONDOWNLOADER_H

#include "elevationcache.h"

#include <thread>

namespace eleman
{

	enum Mode {
		NONE,
// 		RANDOM_CELL,
// 		RANDOM_LOCATION,
		REGION,
		TARGET_SINGLE,
		TARGET_MULTIPLE,
		MISSING
	};

	struct DownloadTarget {
		double latitude, longitude;
		double previousRadius, radius = 1.0;
		double maxRadius = 100.0 * 1000.0;
	};

	/**
	* @todo write docs
	*/
	class ElevationDownloader
	{
	public:
		/**
		* Default constructor
		*/
		ElevationDownloader(ElevationCache* cache);

		/**
		* Destructor
		*/
		~ElevationDownloader();

		// Threading control
		void run();
		void start();
		void stop();
		// TODO also a join() method ?

		// Download control
		void setMode(const Mode& mode);

		void setRegion(double latitude0, double longitude0, double latitude1, double longitude1);
		void setTarget(DownloadTarget* target);
		void addTarget(DownloadTarget* target);
		void removeTarget(DownloadTarget* target);
		void setTargets(std::vector<DownloadTarget*> targets);
		size_t targetCount();

		// Download methods
		void download();
		void downloadNone();
		void downloadTargetSingle();
		void downloadTargetMultiple();
		void downloadMissing();


		// utility functions
		void downloadTarget(DownloadTarget& target);

	private:
		// The cache to download for
		ElevationCache* cache;

		// Thread control
		bool running = false;
		std::thread thread;

		// Download Mode control
		Mode mode = NONE;
		// TARGETS
		std::vector<DownloadTarget*> targets;
		double regionLatitude0 = -90.0;
		double regionLatitude1 = +90.0;
		double regionLongitude0 = -180.0;
		double regionLongitude1 = +180.0;
	};

}	// end namespace eleman

#endif // ELEVATIONDOWNLOADER_H
