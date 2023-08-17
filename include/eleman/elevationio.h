// SPDX-FileCopyrightText: 2023 <rudolf.ortner> <rudolf.ortner.rottenbach@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef ELEVATIONIO_H
#define ELEVATIONIO_H


#include "elevationcache.h"

#include <filesystem>
#include <sstream>
#include <string>

namespace eleman
{

	/**
	* @todo write docs
	*/
	class ElevationIO
	{
	public:
		/**
		* Default constructor
		*/
		ElevationIO(std::filesystem::path cacheDir);

		/**
		* Destructor
		*/
		~ElevationIO();

		bool store(ElevationCacheCell& cell);
		bool load(ElevationCacheCell& cell);

		static std::filesystem::path getDirectory(const std::filesystem::path cacheDir, const ElevationCacheCell& cell);
		static std::string getFilename(const ElevationCacheCell& cell, const std::string& fileExtension = "edc");

	private:
		std::filesystem::path cacheDir;

		void createCacheDir();
	};
	std::string cellname(uint8_t zoneNumber, char zoneLetter, int32_t eastID, int32_t northID);

}	// end namespace elemam

#endif // ELEVATIONIO_H
