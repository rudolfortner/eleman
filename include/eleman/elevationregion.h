// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef ELEVATIONREGION_H
#define ELEVATIONREGION_H

#include <memory>
#include <stdint.h>

#include "grid.h"

namespace eleman
{

/**
 * @todo write docs
 */
class ElevationRegion
{
public:

	enum Interpolation {
		NEAREST,
		LINEAR,
		CUBIC
	};
    /**
     * Default constructor
	 */
	// Constructor with no arguments when this class is derived from
	ElevationRegion();
	ElevationRegion(double lat0, double lon0, double lat1, double lon1, double precision);
	ElevationRegion(double lat0, double lon0, double lat1, double lon1, uint32_t gridSizeLat, uint32_t gridSizeLon);

    /**
     * Destructor
     */
    ~ElevationRegion();

	// Access the raw grid
	double& atGrid(uint32_t x, uint32_t y);
	virtual double getGrid(uint32_t x, uint32_t y) const;
	virtual void setGrid(uint32_t x, uint32_t y, double value);

	// Access by lat/lon
	double get(double latitude, double longitude, Interpolation interpolation = LINEAR) const;
	double getNearest(double latitude, double longitude) const;
	double getLinear(double latitude, double longitude) const;
	double getCubic(double latitude, double longitude) const;

	double minElevation() const;
	double maxElevation() const;

	// CELL COORDINATE CONVERSION FUNCTIONS
	void gridToPos(uint32_t gridLat, uint32_t gridLon, double& latitude, double& longitude) const;
	void posToGrid(double latitude, double longitude, uint32_t& gridLat, uint32_t& gridLon) const;
	void gridFloatToPos(double gridFloatLat, double gridFloatLon, double& latitude, double& longitude) const;
	void posToGridFloat(double latitude, double longitude, double& gridFloatLat, double& gridFloatLon) const;

	// UTILS
	void toMesh(const std::string& filepath, double scale = 1.0, bool centerHorizontal = true, bool centerVertical = true) const;

	double getLat0() const { return lat0; }
	double getLon0() const { return lon0; }
	double getLat1() const { return lat1; }
	double getLon1() const { return lon1; }

	uint32_t getGridSizeLat() const { return sizeLat; }
	uint32_t getGridSizeLon() const { return sizeLon; }

	size_t size() const;			// Amount of grid positions with usefull data
	size_t sizeTotal() const;		// Amount of grid positions the whole datastructure has
	size_t memory() const;			// Amount of memory needed for whole data structure

protected:
	double lat0, lat1;
	double lon0, lon1;
	uint32_t sizeLat, sizeLon;

	std::shared_ptr<Grid<double>> elevationData;
};

}	// end namespace eleman

#endif // ELEVATIONREGION_H
