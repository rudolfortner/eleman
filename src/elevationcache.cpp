// SPDX-FileCopyrightText: 2023 <rudolf.ortner> <rudolf.ortner.rottenbach@gmail.com>
// SPDX-License-Identifier: MIT

#include "eleman/elevationcache.h"

#include "eleman/elevationdata.h"
#include "eleman/elevationexception.h"
#include "eleman/elevationio.h"
#include "eleman/elevationmanager.h"
#include "eleman/elevationutils.h"

#include <assert.h>
#include <math.h>
#include <set>


eleman::ElevationCache::ElevationCache() : ElevationCache(100, 10.0)
{

}

eleman::ElevationCache::ElevationCache(uint16_t cellDivisions, double precision)
{
	this->cellDivisions = cellDivisions;
	this->precision = precision;
}


eleman::ElevationCache::~ElevationCache()
{
	unloadAll();
}

bool eleman::ElevationCache::get(Position pos, eleman::ElevationData& data)
{
	uint32_t cellID = toCellID(pos.latitude, pos.longitude, cellDivisions);

	// TODO get from cache

	return false;
}

void eleman::ElevationCache::put(const eleman::ElevationData& data)
{
	// TODO
}

eleman::ElevationData eleman::ElevationCache::get(double latitude, double longitude, ElevationRegion::Interpolation interpolation)
{
	return get({latitude, longitude}, interpolation);
}

eleman::ElevationData eleman::ElevationCache::get(Position pos, ElevationRegion::Interpolation interpolation)
{
	uint64_t cellID = toCellID(pos.latitude, pos.longitude, cellDivisions);
	loadCell(cellID);
	ElevationCacheCell* cell = getCell(cellID);
	printf("Cell %lu loaded at %p\n", cellID, cell);

	ElevationData data;
	data.latitude = pos.latitude;
	data.longitude = pos.longitude;
	data.elevation = cell->get(pos.latitude, pos.longitude, interpolation);

	return data;
}

std::vector< eleman::ElevationData > eleman::ElevationCache::get(const std::vector<Position>& positions, eleman::ElevationRegion::Interpolation interpolation)
{
	std::vector<ElevationData> data;

	for(const Position& pos : positions)
	{
		data.push_back(get(pos, interpolation));
	}

	return data;
}

eleman::ElevationRegion eleman::ElevationCache::get(double lat0, double lon0, double lat1, double lon1, double precision, eleman::ElevationRegion::Interpolation interpolation)
{
	ElevationRegion region(lat0, lon0, lat1, lon1, precision);
	fillRegion(region, interpolation);
	return region;
}

eleman::ElevationRegion eleman::ElevationCache::get(double lat0, double lon0, double lat1, double lon1, uint32_t gridSizeLat, uint32_t gridSizeLon, ElevationRegion::Interpolation interpolation)
{
	ElevationRegion region(lat0, lon0, lat1, lon1, gridSizeLat, gridSizeLon);
	fillRegion(region, interpolation);
	return region;
}

void eleman::ElevationCache::fillRegion(eleman::ElevationRegion& region, ElevationRegion::Interpolation interpolation)
{
	// Load all required cells
	std::vector<uint64_t> ids = cellsForRegion(region.getLat0(), region.getLon0(), region.getLat1(), region.getLon1());
	for(uint64_t& id : ids)
		loadCell(id);

	// Retrieve data for region
	for(uint32_t y = 0; y < region.getGridSizeLat(); y++)
	{
		for(uint32_t x = 0; x < region.getGridSizeLon(); x++)
		{
			// Calculate lat/lon for each grid position in region
			double lat, lon;
			region.gridToPos(y, x, lat, lon);

			// Retrieve appropriate cell
			uint64_t cellID = toCellID(lat, lon, cellDivisions);
			ElevationCacheCell* cell = getCell(cellID);

			// Retrieve data and store in grid
			region.setGrid(x, y, cell->get(lat, lon, interpolation));
		}
	}
}





uint32_t eleman::ElevationCache::precacheCells(double latitude0, double longitude0, double latitude1, double longitude1)
{
	uint32_t count = 0;
	std::vector<uint64_t> ids = cellsForRegion(latitude0, longitude0, latitude1, longitude1);
	for(uint64_t& id : ids)
	{
		bool wasLoaded = isCellLoaded(id);
		if(!wasLoaded) loadCell(id);
		count += getCell(id)->precacheCell();
		if(!wasLoaded) unloadCell(id);
	}
	return count;
}

uint32_t eleman::ElevationCache::precacheRegion(Position pos0, Position pos1)
{
	return precacheRegion(pos0.latitude, pos0.longitude, pos1.latitude, pos1.longitude);
}


uint32_t eleman::ElevationCache::precacheRegion(double latitude0, double longitude0, double latitude1, double longitude1)
{
	uint32_t count = 0;
	std::vector<uint64_t> ids = cellsForRegion(latitude0, longitude0, latitude1, longitude1);
	for(uint64_t& id : ids)
	{
		bool wasLoaded = isCellLoaded(id);
		if(!wasLoaded) loadCell(id);
		count += getCell(id)->precacheRegion(latitude0, longitude0, latitude1, longitude1);
		if(!wasLoaded) unloadCell(id);
	}
	return count;
}

uint32_t eleman::ElevationCache::precacheRadius(double latitude, double longitude, double radius)
{
	double radiusLat = meters2degrees(radius, 0.0);	// Latitude does not change
	double radiusLon = meters2degrees(radius, latitude);

	uint32_t count = 0;
	// TODO Edge cases like at -180 longitude
	std::vector<uint64_t> ids = cellsForRegion(latitude - radiusLat, longitude - radiusLon, latitude + radiusLat, longitude + radiusLon);
	for(uint64_t& id : ids)
	{
		bool wasLoaded = isCellLoaded(id);
		if(!wasLoaded) loadCell(id);
		count += getCell(id)->precacheRadius(latitude, longitude, radius);
		if(!wasLoaded) unloadCell(id);
	}
	return count;
}

void eleman::ElevationCache::processCacheMiss(const eleman::CacheMiss& cacheMiss)
{
	ElevationVendor* vendor = manager->getVendor();
	ElevationCacheCell* cell = getCell(cacheMiss.cellID);
	printf("Cell %p is loaded = %s\n", cell, isCellLoaded(cacheMiss.cellID) ? "true" : "false");

	std::vector<CacheMiss> missing;
	cell->reportMissingNeighbors(missing, cacheMiss.x, cacheMiss.y, 32, vendor->getLocationsPerRequest());
	printf("Got %zu missing neighbors\n", missing.size());

	printf("For cell %d\n", cell->getID());

	processMissing(missing);
}

void eleman::ElevationCache::processMissing(const std::vector<CacheMiss>& missing)
{
	// Prepare request
	std::vector<Position> positions;
	for(const CacheMiss& miss : missing)
	{
		positions.push_back({miss.latitude, miss.longitude});
	}

	// Perform request
	VendorResponse response = manager->requestRaw(positions);
	if(response.code != OK)
		throw ElevationException(response.error);

	// Process request
	for(size_t i = 0; i < response.results.size(); i++)
	{
		ElevationCacheCell* cell = getCell(missing[i].cellID);
		cell->setGrid(missing[i].x, missing[i].y, response.results[i].elevation);
	}
}

uint32_t eleman::ElevationCache::reportMissing(std::vector< eleman::CacheMiss >& missing, uint32_t limit) const
{
	uint32_t count = 0;
	for(auto zonePair : cells)
	{
		if(limit > 0 && count >= limit) return count;
		count += zonePair.second.reportMissing(missing);
	}
	return count;
}



bool eleman::ElevationCache::isCellLoaded(uint64_t cellID)
{
#if (__cplusplus >= 202002L)
	return cells.contains(cellID);
#else
	return cells.find(cellID) != cells.end();
#endif
}

bool eleman::ElevationCache::loadCell(uint64_t cellID)
{
	if(isCellLoaded(cellID))
		return false;

	printf("Loading cell %lu...\n", cellID);
	ElevationCacheCell cell(this, cellID, cellDivisions, precision);
	if(io)
		io->load(cell);
	cells.insert({cellID, cell});

	return true;
}

eleman::ElevationCacheCell* eleman::ElevationCache::getCell(uint64_t cellID)
{
	if(!isCellLoaded(cellID))
		loadCell(cellID);

	auto it = cells.find(cellID);

	if(it == cells.end())
		return nullptr;

	return &it->second;
}

bool eleman::ElevationCache::unloadCell(uint64_t cellID)
{
	if(!isCellLoaded(cellID))
		return false;

	ElevationCacheCell* cell = getCell(cellID);

	if(io)
		io->store(*cell);

	cells.erase(cellID);

	return true;
}

void eleman::ElevationCache::unloadAll()
{
	while(cells.size() > 0)
	{
		uint64_t cellID = (*cells.begin()).first;
		unloadCell(cellID);
	}
	printf("All Cells unloaded\n");
}


std::vector<uint64_t> eleman::ElevationCache::cellsForRegion(double latitude0, double longitude0,
															 double latitude1, double longitude1)
{
	double lat0 = std::min(latitude0, latitude1);
	double lat1 = std::max(latitude0, latitude1);
	double lon0 = std::min(longitude0, longitude1);
	double lon1 = std::max(longitude0, longitude1);

	uint64_t x0, y0;
	uint64_t x1, y1;
	toCellXY(lat0, lon0, cellDivisions, x0, y0);
	toCellXY(lat1, lon1, cellDivisions, x1, y1);

	printf("%lu %lu %lu %lu\n", x0, x1, y0, y1);

	uint64_t minX = std::min(x0, x1);
	uint64_t maxX = std::max(x0, x1);
	uint64_t minY = std::min(y0, y1);
	uint64_t maxY = std::max(y0, y1);

	std::vector<uint64_t> ids;
	for(uint64_t y = minY; y <= maxY; y++)
	{
		for(uint64_t x = minX; x <= maxX; x++)
		{
			uint64_t id = toCellID(x, y, cellDivisions);
			printf("---> ID %lu\n", id);
			ids.push_back(id);
		}
	}
	return ids;
}

uint32_t eleman::ElevationCache::cellsLoaded()
{
	return cells.size();
}


void eleman::ElevationCache::clear()
{
	// TODO Delete all cache files via ElevationIO
}

void eleman::ElevationCache::flush()
{
	if(io == nullptr) return;

	for(auto cellPair : cells)
	{
		io->store(cellPair.second);
	}
}



uint16_t eleman::ElevationCache::getCellDivisions() const
{
	return cellDivisions;
}

double eleman::ElevationCache::getPrecision() const
{
	return precision;
}





size_t eleman::ElevationCache::size() const
{
	size_t size = 0;
	for(auto cellPair : cells)
	{
		size += cellPair.second.size();
	}
	return size;
}

size_t eleman::ElevationCache::sizeTotal() const
{
	size_t size = 0;
	for(auto cellPair : cells)
	{
		size += cellPair.second.sizeTotal();
	}
	return size;
}

size_t eleman::ElevationCache::memory() const
{
	size_t memory = sizeof(ElevationCache);
	for(auto cellPair : cells)
	{
		memory += cellPair.second.memory();
	}
	return memory;
}

void eleman::ElevationCache::setManager(eleman::ElevationManager* manager)
{
	this->manager = manager;
}

eleman::ElevationManager * eleman::ElevationCache::getManager()
{
	return manager;
}

void eleman::ElevationCache::setIO(eleman::ElevationIO* io)
{
	this->io = io;
}

eleman::ElevationIO * eleman::ElevationCache::getIO()
{
	return io;
}


void eleman::ElevationCache::toCellXY(double latitude, double longitude, uint16_t cellDivisions, uint64_t& x, uint64_t& y)
{
	double lat = latitude + 90.0;
	double lon = longitude + 180.0;
	y = floor(lat * cellDivisions);
	x = floor(lon * cellDivisions);
}

void eleman::ElevationCache::fromCellXY(uint64_t x, uint64_t y, uint16_t cellDivisions, double& latitude, double& longitude)
{
	latitude	= double(y) / cellDivisions - 90.0;
	longitude	= double(x) / cellDivisions - 180.0;
}

uint64_t eleman::ElevationCache::toCellID(uint64_t x, uint64_t y, const uint16_t cellDivisions)
{
	// columns or width of virtual grid
	uint64_t numLon = 360.0 * cellDivisions;
	return y * numLon + x;
}

void eleman::ElevationCache::fromCellID(uint64_t id, uint64_t& x, uint64_t& y, uint16_t cellDivisions)
{
	uint64_t numLon = 360.0 * cellDivisions;
	y = id / numLon;
	x = id % numLon;
}

uint64_t eleman::ElevationCache::toCellID(double latitude, double longitude, const uint16_t cellDivisions)
{
	uint64_t x, y;
	toCellXY(latitude, longitude, cellDivisions, x, y);
	return toCellID(x, y, cellDivisions);
}

void eleman::ElevationCache::fromCellID(uint64_t id, double& latitude, double& longitude, uint16_t cellDivisions)
{
	uint64_t x, y;
	fromCellID(id, x, y, cellDivisions);
	fromCellXY(x, y, cellDivisions, latitude, longitude);
}




eleman::ElevationCacheCell::ElevationCacheCell(ElevationCache* cache, uint64_t id, uint16_t cellDivisions, double precision)
{
	// Cell ID
	this->cache = cache;
	this->id = id;
	ElevationCache::fromCellID(id, x, y, cellDivisions);

	// CELL DATA
	ElevationCache::fromCellID(id, lat0, lon0, cellDivisions);
	lat0 = roundDigits(lat0, 9);
	lon0 = roundDigits(lon0, 9);
	lat1 = lat0 + 1.0 / cellDivisions;
	lon1 = lon0 + 1.0 / cellDivisions;
	lat1 = roundDigits(lat1, 9);
	lon1 = roundDigits(lon1, 9);
	this->precision = precision;

	double referenceLatitude = std::min(std::abs(lat0), std::abs(lat1));
	calculateGridSize(lat1-lat0, lon1-lon0,
					  referenceLatitude, precision,
					  sizeLat, sizeLon);

	statusData		= std::make_shared<Grid<bool>>(sizeLon, sizeLat, false);
	elevationData	= std::make_shared<Grid<double>>(sizeLon, sizeLat, NAN);

	printf("Constructing cell from ID %lu (%f - %f / %f - %f)\n", id, lat0, lat1, lon0, lon1);
}

eleman::ElevationCacheCell::ElevationCacheCell(ElevationCache* cache, uint64_t x, uint64_t y, uint16_t cellDivisions, double precision)
{
	// Cell ID
	this->cache = cache;
	this->id = ElevationCache::toCellID(x, y, cellDivisions);
	this->x = x;
	this->y = y;

	// Cell Data
	ElevationCache::fromCellXY(x, y, cellDivisions, lat0, lon0);
	lat0 = roundDigits(lat0, 9);
	lon0 = roundDigits(lon0, 9);
	lat1 = lat0 + 1.0 / cellDivisions;
	lon1 = lon0 + 1.0 / cellDivisions;
	lat1 = roundDigits(lat1, 9);
	lon1 = roundDigits(lon1, 9);
	this->precision = precision;

	double referenceLatitude = std::min(std::abs(lat0), std::abs(lat1));
	calculateGridSize(lat1-lat0, lon1-lon0,
					  referenceLatitude, precision,
					  sizeLat, sizeLon);

	statusData		= std::make_shared<Grid<bool>>(sizeLon, sizeLat, false);
	elevationData	= std::make_shared<Grid<double>>(sizeLon, sizeLat, NAN);

	printf("Constructing cell from XY %lu/%lu (%f - %f / %f - %f)\n", x, y, lat0, lat1, lon0, lon1);
}



eleman::ElevationCacheCell::ElevationCacheCell(ElevationCache* cache, double lat0, double lon0, double lat1, double lon1, double precision)
{
	throw std::runtime_error("Should not be used for now!");
	// TODO ID
	this->cache = cache;
	this->lat0 = lat0;
	this->lon0 = lon0;
	this->lat1 = lat1;
	this->lon1 = lon1;
	this->precision = precision;

	double referenceLatitude = std::min(std::abs(lat0), std::abs(lat1));
	calculateGridSize(lat1-lat0, lon1-lon0,
					  referenceLatitude, precision,
					  sizeLat, sizeLon);

	statusData		= std::make_shared<Grid<bool>>(sizeLon, sizeLat, false);
	elevationData	= std::make_shared<Grid<double>>(sizeLon, sizeLat, NAN);

	printf("Constructing cell from lat/lon (%f - %f / %f - %f)\n", lat0, lat1, lon0, lon1);
}


uint32_t eleman::ElevationCacheCell::precacheCell()
{
	printf("Precaching cell (%f - %f / %f - %f)\n", lat0, lat1, lon0, lon1);
	if(cache->getManager() == nullptr)
		printf("FUCK precacheCell\n");

	std::vector<Position> positions;
	for(uint32_t y = 0; y < sizeLat; y++)
	{
		for(uint32_t x = 0; x < sizeLon; x++)
		{
			if(statusData->get(x, y)) continue;
			double lat, lon;
			gridToPos(y, x, lat, lon);
			positions.push_back({lat, lon});
		}
	}
	if(positions.size() <= 0) return 0;

	printf("Requesting %zu locations\n", positions.size());
	VendorResponse response = cache->getManager()->requestRaw(positions);
	if(response.code != OK)
		throw ElevationException(response.error);


	for(ElevationData& result : response.results)
	{
		uint32_t y, x;
		posToGrid(result.latitude, result.longitude, y, x);
		statusData->set(x, y, true);
		elevationData->set(x, y, result.elevation);
	}
	dirty = true;
	return positions.size();
}

uint32_t eleman::ElevationCacheCell::precacheRegion(double latitude0, double longitude0,
												double latitude1, double longitude1)
{
	printf("Precaching region (%f - %f / %f - %f)\n", latitude0, latitude1, longitude0, longitude1);
	printf("GRID SIZE: %d %d\n", sizeLon, sizeLat);
// 	return;

	if(cache->getManager() == nullptr)
		printf("FUCK precacheCell\n");

	std::vector<Position> positions;
	for(uint32_t y = 0; y < sizeLat; y++)
	{
		for(uint32_t x = 0; x < sizeLon; x++)
		{
			// Already in cache
			if(statusData->get(x, y)) continue;

			// Get lat/lon of cell
			double lat, lon;
			gridToPos(y, x, lat, lon);

			// TODO Maybe rethink double precision errors 48.329999999999
			if(!check_bounds(lat, lon, latitude0, latitude1, longitude0, longitude1)) continue;
			positions.push_back({lat, lon});
		}
	}
	if(positions.size() <= 0) return 0;

	printf("Requesting %zu locations\n", positions.size());
	VendorResponse response = cache->getManager()->requestRaw(positions);
	if(response.code != OK)
		throw ElevationException(response.error);


	for(ElevationData& result : response.results)
	{
		uint32_t x, y;
		posToGrid(result.latitude, result.longitude, y, x);
		printf("Pos %f %f to cell %d %d\n", result.latitude, result.longitude, x, y);
		printf("%d %d\n", sizeLat, sizeLon);
		statusData->set(x, y, true);
		elevationData->set(x, y, result.elevation);
	}
	dirty = true;
	return positions.size();
}

uint32_t eleman::ElevationCacheCell::precacheRadius(double latitude, double longitude, double radius)
{
	printf("Precaching cell (%f - %f / %f - %f) AT %f %f with radius %fm\n", lat0, lat1, lon0, lon1, latitude, longitude, radius);

	if(cache->getManager() == nullptr)
		printf("FUCK precacheCell\n");

	double radiusLat = meters2degrees(radius, 0.0);	// Latitude does not change
	double radiusLon = meters2degrees(radius, latitude);
	printf("radiusLat %f\n", radiusLat);
	printf("radiusLon %f\n", radiusLon);

	std::vector<Position> positions;
	for(uint32_t y = 0; y < sizeLat; y++)
	{
		for(uint32_t x = 0; x < sizeLon; x++)
		{
			if(statusData->get(x, y)) continue;
			double lat, lon;
			gridToPos(y, x, lat, lon);

			if(!inEllipse(lat, lon, latitude, longitude, radiusLat, radiusLon)) continue;
			positions.push_back({lat, lon});
		}
	}
	if(positions.size() <= 0) return 0;

	printf("Requesting %zu locations\n", positions.size());
	VendorResponse response = cache->getManager()->requestRaw(positions);
	if(response.code != OK)
		throw ElevationException(response.error);

	for(ElevationData& result : response.results)
	{
		uint32_t y, x;
		posToGrid(result.latitude, result.longitude, y, x);
		statusData->set(x, y, true);
		elevationData->set(x, y, result.elevation);
	}
	dirty = true;
	return positions.size();
}

uint32_t eleman::ElevationCacheCell::reportMissing(std::vector<CacheMiss>& missing, uint32_t limit) const
{
	uint32_t count = 0;
	for(uint32_t y = 0; y < sizeLat; y++)
	{
		for(uint32_t x = 0; x < sizeLon; x++)
		{
			if(limit > 0 && count >= limit) return count;
			if(statusData->get(x, y)) continue;

			CacheMiss miss;
			miss.cellID = id;
			miss.x = x;
			miss.y = y;
			gridToPos(y, x, miss.latitude, miss.longitude);

			count++;
			missing.push_back(miss);
		}
	}
	return count;
}

uint32_t eleman::ElevationCacheCell::reportMissingNeighbors(std::vector<CacheMiss>& missing, uint32_t cx, uint32_t cy, uint8_t radius, uint32_t limit) const
{
// 	printf("reportMissingNeighbors %d %d %d %d\n", cx, cy, radius, limit);

	uint32_t count = 0;
	for(uint8_t r = 0; r <= radius; r++)
	{
// 		printf("reportMissingNeighbors using radius %d\n", r);

		int32_t minX = int32_t(cx) - r;
		int32_t maxX = int32_t(cx) + r;
		int32_t minY = int32_t(cy) - r;
		int32_t maxY = int32_t(cy) + r;

		std::set<std::pair<int32_t, int32_t>> coordinates;
		// horizontal edges
		for(int32_t x = minX; x <= maxX; x++)
		{
			coordinates.insert({x, minY});
			coordinates.insert({x, maxY});
		}
		// vertical edges
		for(int32_t y = minY + 1; y <= maxY - 1; y++)
		{
			coordinates.insert({minX, y});
			coordinates.insert({maxX, y});
		}

		for(const std::pair<int32_t, int32_t>& pair : coordinates)
		{
			const int32_t& x = pair.first;
			const int32_t& y = pair.second;
// 			printf("Checking location %d %d\n", x, y);

			if(limit > 0 && count >= limit) return count;
			if(x < 0 || x >= sizeLon) continue;
			if(y < 0 || y >= sizeLat) continue;
			if(statusData->get(x, y)) continue;

			CacheMiss miss;
			miss.cellID = id;
			miss.x = x;
			miss.y = y;
			gridToPos(y, x, miss.latitude, miss.longitude);

			count++;
			missing.push_back(miss);
		}
	}
	return count;
}


double eleman::ElevationCacheCell::getGrid(uint32_t x, uint32_t y) const
{
	if(!statusData->get(x, y))
	{
// 		printf("CACHE MISS in cell %lu\n", id);
		CacheMiss miss;
		miss.cellID = id;
		miss.y = y;
		miss.x = x;
		cache->processCacheMiss(miss);
	}else{
// 		printf("CACHE HIT\n");
	}
	return ElevationRegion::getGrid(x, y);
}

void eleman::ElevationCacheCell::setGrid(uint32_t x, uint32_t y, double value)
{
	printf("Setting data in cell %lu\n", id);
	ElevationRegion::setGrid(x, y, value);
	statusData->set(x, y, true);
	dirty = true;
}



uint32_t eleman::ElevationCacheCell::getID() const
{
	return id;
}

double eleman::ElevationCacheCell::getPrecision() const
{
	return precision;
}

bool eleman::ElevationCacheCell::isDirty() const
{
	return dirty;
}


size_t eleman::ElevationCacheCell::size() const
{
	uint64_t size = 0;
	for(uint32_t y = 0; y < sizeLat; y++)
	{
		for(uint32_t x = 0; x < sizeLon; x++)
		{
			if(statusData->get(x, y)) size++;
		}
	}
	return size;
}

size_t eleman::ElevationCacheCell::memory() const
{
	return sizeof(ElevationCacheCell) + statusData->memory() + elevationData->memory();
}


