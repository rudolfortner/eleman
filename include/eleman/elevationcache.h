// SPDX-FileCopyrightText: 2023 <rudolf.ortner> <rudolf.ortner.rottenbach@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef ELEVATIONCACHE_H
#define ELEVATIONCACHE_H

#include "elevationdata.h"
#include "elevationregion.h"

#include "grid.h"
#include <map>
// #include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace eleman
{
	class ElevationIO;
	class ElevationManager;
	class ElevationCacheCell;

	struct CacheMiss {
		uint64_t cellID;
		uint32_t x, y;
		double latitude, longitude;
	};

	/**
	* @todo write docs
	*/
	class ElevationCache
	{
	public:
		/**
		* Default constructor
		*/
		ElevationCache();
		ElevationCache(uint16_t cellDivisions, double precision);

		/**
		* Destructor
		*/
		~ElevationCache();

		bool get(Position pos, ElevationData& data);
		void put(const ElevationData& data);	// TODO Also allow non grid alligned values?

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

		// Precaching
		uint32_t precacheCells(double latitude0, double longitude0, double latitude1, double longitude1);
		uint32_t precacheRadius(double latitude, double longitude, double radius);
		uint32_t precacheRegion(Position pos0, Position pos1);
		uint32_t precacheRegion(double latitude0, double longitude0, double latitude1, double longitude1);

		void processCacheMiss(const CacheMiss& cacheMiss);
		void processMissing(const std::vector<CacheMiss>& missing);
		uint32_t reportMissing(std::vector<CacheMiss>& missing, uint32_t limit = 0) const;

		// Loading/Unloading cells
		bool isCellLoaded(uint64_t cellID);
		bool loadCell(uint64_t cellID);
		ElevationCacheCell* getCell(uint64_t cellID);
		bool unloadCell(uint64_t cellID);
		void unloadAll();
		std::vector<uint64_t> cellsForRegion(double latitude0, double longitude0, double latitude1, double longitude1);
		uint32_t cellsLoaded();

		// Cache control
		void clear();
		bool clearRadius(double latitude, double longitude, double radius);
		bool clearRegion(Position pos0, Position pos1);
		bool clearRegion(double latitude0, double longitude0, double latitude1, double longitude1);
		void flush();

		// Getters
		uint16_t getCellDivisions() const;
		double getPrecision() const;

		size_t size() const;		// Amount of grid positions with usefull data
		size_t sizeTotal() const;	// Amount of grid positions the whole datastructure has
		size_t memory() const;		// Amount of memory needed for whole data structure

		// GETTERS AND SETTERS
		void setManager(ElevationManager* manager);
		ElevationManager* getManager();
		void setIO(ElevationIO* io);
		ElevationIO* getIO();


		// CELL ID CONVERSION FUNCTIONS
		static void toCellXY(double latitude, double longitude, uint16_t cellDivisions, uint64_t& x, uint64_t& y);
		static void fromCellXY(uint64_t x, uint64_t y, uint16_t cellDivisions, double& latitude, double& longitude);

		static uint64_t toCellID(uint64_t x, uint64_t y, uint16_t cellDivisions);
		static void fromCellID(uint64_t id, uint64_t& x, uint64_t& y, uint16_t cellDivisions);

		static uint64_t toCellID(double latitude, double longitude, uint16_t cellDivisions);
		static void fromCellID(uint64_t id, double& latitude, double& longitude, uint16_t cellDivisions);


	private:
		ElevationManager* manager;
		ElevationIO* io = nullptr;

		uint16_t cellDivisions;
		double precision;
		std::map<uint32_t, ElevationCacheCell> cells;
	};

	class ElevationCacheCell : public ElevationRegion
	{
	public:
		ElevationCacheCell(ElevationCache* cache, uint64_t id, uint16_t cellDivisions, double precision);
		ElevationCacheCell(ElevationCache* cache, uint64_t x, uint64_t y, uint16_t cellDivisions, double precision);
		ElevationCacheCell(ElevationCache* cache, double lat0, double lon0, double lat1, double lon1, double precision);

		// Clear functions
		bool clearRegion(double latitude0, double longitude0, double latitude1, double longitude1);
		bool clearRadius(double latitude, double longitude, double radius);


		// Precaching functions
		uint32_t precacheCell();
		uint32_t precacheRadius(double latitude, double longitude, double radius);
		uint32_t precacheRegion(double latitude0, double longitude0, double latitude1, double longitude1);

		uint32_t reportMissing(std::vector<CacheMiss>& missing, uint32_t limit = 0) const;
		uint32_t reportMissingNeighbors(std::vector<CacheMiss>& missing, uint32_t cx, uint32_t cy, uint8_t radius, uint32_t limit = 0) const;
		// Get functions already specified in ElevationRegion
		// TODO But what do to in case of cache miss?

		void clearGrid(uint32_t x, uint32_t y);
		// overwrite grid functions from ElevationRegion
		double getGrid(uint32_t x, uint32_t y) const override;
		void setGrid(uint32_t x, uint32_t y, double value) override;

		// Getters
		uint32_t getID() const;
		double getPrecision() const;
		bool isDirty() const;


		// TODO Check if virtual specifier is needed in ElevationRegion
		size_t size() const;		// Amount of grid positions with usefull data
		size_t memory() const;		// Amount of memory needed for whole data structure
	private:
		// Cell Identification
		ElevationCache* cache;
		uint64_t id;
		uint64_t x, y;

		// Cell Data
		double precision;

		bool dirty = true;
		std::shared_ptr<Grid<bool>> statusData;

		friend class ElevationIO;
	};

}	// end namespace eleman

#endif // ELEVATIONCACHE_H
