// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "eleman/elevationregion.h"

#include "eleman/elevationutils.h"

#include <fstream>
#include <stdio.h>
#include <math.h>

eleman::ElevationRegion::ElevationRegion()
{

}

eleman::ElevationRegion::~ElevationRegion()
{

}

eleman::ElevationRegion::ElevationRegion(double lat0, double lon0, double lat1, double lon1, double precision)
{
	this->lat0 = lat0;
	this->lon0 = lon0;
	this->lat1 = lat1;
	this->lon1 = lon1;

	double referenceLatitude = std::min(std::abs(lat0), std::abs(lat1));
	calculateGridSize(lat1-lat0, lon1-lon0,
					  referenceLatitude, precision,
				   sizeLat, sizeLon);

	elevationData = std::make_shared<Grid<double>>(sizeLon, sizeLat, NAN);

	printf("[ElevationRegion] Creating region from precision %f -> %d %d\n", precision, sizeLat, sizeLon);
}

eleman::ElevationRegion::ElevationRegion(double lat0, double lon0, double lat1, double lon1, uint32_t gridSizeLat, uint32_t gridSizeLon)
{
	this->lat0 = lat0;
	this->lon0 = lon0;
	this->lat1 = lat1;
	this->lon1 = lon1;

	this->sizeLat = gridSizeLat;
	this->sizeLon = gridSizeLon;

	elevationData = std::make_shared<Grid<double>>(sizeLon, sizeLat, NAN);

	printf("[ElevationRegion] Creating region from size %d %d\n", sizeLat, sizeLon);
}

eleman::ElevationRegion::ElevationRegion(const eleman::ElevationRegion& region)
{
	this->lat0 = region.lat0;
	this->lon0 = region.lon0;
	this->lat1 = region.lat1;
	this->lon1 = region.lon1;

	this->sizeLat = region.sizeLat;
	this->sizeLon = region.sizeLon;

	this->elevationData = std::make_shared<Grid<double>>(*region.elevationData.get());

	printf("[ElevationRegion] Creating region from other region\n");
}


double& eleman::ElevationRegion::atGrid(uint32_t x, uint32_t y)
{
	return elevationData->at(x, y);
}

double eleman::ElevationRegion::getGrid(uint32_t x, uint32_t y) const
{
	return elevationData->get(x, y);
}

void eleman::ElevationRegion::setGrid(uint32_t x, uint32_t y, double value)
{
	return elevationData->set(x, y, value);
}

double eleman::ElevationRegion::get(double latitude, double longitude, eleman::ElevationRegion::Interpolation interpolation) const
{
	double lat = roundDigits(latitude, 9);
	double lon = roundDigits(longitude, 9);
	// TODO Maybe support regions crossing borders in the future
	if(!check_bounds(lat, lon, lat0, lat1, lon0, lon1)) {
		printf("%30.20f - %30.20f - %30.20f\n", lat0, lat, lat1);
		printf("%30.20f - %30.20f - %30.20f\n", lon0, lon, lon1);
		throw std::runtime_error("[ElevationRegion] lat/lon out of bounds");
	}

	switch(interpolation)
	{
		case NEAREST:
			return getNearest(lat, lon);

		case LINEAR:
			return getLinear(lat, lon);

		case CUBIC:
			return getCubic(lat, lon);

		default:
			throw std::runtime_error("[ElevationRegion] Unknown interpolation type");
	}
	return NAN;
}

double eleman::ElevationRegion::getNearest(double latitude, double longitude) const
{
	double lat = roundDigits(latitude, 9);
	double lon = roundDigits(longitude, 9);
	// TODO Maybe support regions crossing borders in the future
	if(!check_bounds(lat, lon, lat0, lat1, lon0, lon1))
		throw std::runtime_error("[ElevationRegion] lat/lon out of bounds");

	double gridLat, gridLon;
	posToGridFloat(lat, lon, gridLat, gridLon);
	printf("Getting nearest from %f %f\n", gridLat, gridLon);
	return getGrid(round(gridLon), round(gridLat));
}

double eleman::ElevationRegion::getLinear(double latitude, double longitude) const
{
	double lat = roundDigits(latitude, 9);
	double lon = roundDigits(longitude, 9);
	// TODO Maybe support regions crossing borders in the future
	if(!check_bounds(lat, lon, lat0, lat1, lon0, lon1))
		throw std::runtime_error("[ElevationRegion] lat/lon out of bounds");

	double gridLat, gridLon;
	posToGridFloat(lat, lon, gridLat, gridLon);

	uint32_t x0 = floor(gridLon);
	uint32_t x1 = ceil(gridLon);
	uint32_t y0 = floor(gridLat);
	uint32_t y1 = ceil(gridLat);

	double Q11 = getGrid(x0, y0);
	double Q12 = getGrid(x0, y1);
	double Q21 = getGrid(x1, y0);
	double Q22 = getGrid(x1, y1);

	double R0 = interpolate(gridLon, x0, Q11, x1, Q21);
	double R1 = interpolate(gridLon, x0, Q12, x1, Q22);

	double P = interpolate(gridLat, y0, R0, y1, R1);

	return P;
}

double eleman::ElevationRegion::getCubic(double latitude, double longitude) const
{
	double lat = roundDigits(latitude, 9);
	double lon = roundDigits(longitude, 9);
	// TODO Maybe support regions crossing borders in the future
	if(!check_bounds(lat, lon, lat0, lat1, lon0, lon1))
		throw std::runtime_error("[ElevationRegion] lat/lon out of bounds");

	// TODO CUBIC INTERPOLATION
	throw std::runtime_error("[ElevationRegion] bicubic interpolation not supported yet!");
}

double eleman::ElevationRegion::minElevation() const
{
	double min = std::numeric_limits<double>::max();
	for(uint32_t y = 0; y < sizeLat; y++)
	{
		for(uint32_t x = 0; x < sizeLon; x++)
		{
			min = std::min(min, getGrid(x, y));
		}
	}
	return min;
}

double eleman::ElevationRegion::maxElevation() const
{
	double max = std::numeric_limits<double>::min();
	for(uint32_t y = 0; y < sizeLat; y++)
	{
		for(uint32_t x = 0; x < sizeLon; x++)
		{
			max = std::max(max, getGrid(x, y));
		}
	}
	return max;
}



void eleman::ElevationRegion::gridToPos(uint32_t gridLat, uint32_t gridLon, double& latitude, double& longitude) const
{
	latitude	= interpolate(gridLat, 0.0, lat0, sizeLat - 1.0, lat1);
	longitude	= interpolate(gridLon, 0.0, lon0, sizeLon - 1.0, lon1);
}

void eleman::ElevationRegion::posToGridFloat(double latitude, double longitude, double& gridFloatLat, double& gridFloatLon) const
{
	gridFloatLat = interpolate(latitude, lat0, 0.0, lat1, sizeLat - 1.0);
	gridFloatLon = interpolate(longitude, lon0, 0.0, lon1, sizeLon - 1.0);
}

void eleman::ElevationRegion::gridFloatToPos(double gridFloatLat, double gridFloatLon, double& latitude, double& longitude) const
{
	latitude	= interpolate(gridFloatLat, 0.0, lat0, sizeLat - 1.0, lat1);
	longitude	= interpolate(gridFloatLon, 0.0, lon0, sizeLon - 1.0, lon1);
}

void eleman::ElevationRegion::posToGrid(double latitude, double longitude, uint32_t& gridLat, uint32_t& gridLon) const
{
	gridLat = round(interpolate(latitude, lat0, 0.0, lat1, sizeLat - 1.0));
	gridLon = round(interpolate(longitude, lon0, 0.0, lon1, sizeLon - 1.0));
}

// Blender: Import OBJ Up-Axis Z, Forward-Axis Y
void eleman::ElevationRegion::toMesh(const std::string& filepath, double scale, bool centerHorizontal, bool centerVertical) const
{
	// TODO create directory
	// TODO Place mesh at correct position (place multiple besides) when not centered horizontally

	std::ofstream file(filepath);
	// TODO Replace with global getVersionString() or something like that
	file << "# EleMan v0.01" << std::endl;

	// Write position data
	double referenceLatitude = std::min(std::abs(lat0), std::abs(lat1));
	double metersLat = degrees2meters(lat1-lat0, 0.0);
	double metersLon = degrees2meters(lon1-lon0, referenceLatitude);
	double cellWidth = metersLon  / (sizeLon - 1.0);
	double cellHeight= metersLat / (sizeLat - 1.0);

	// Values for moving output
	double centerX = 0.0;
	double centerY = 0.0;
	double elevationOffset = 0.0;

	if(centerHorizontal)
	{
		centerX = metersLon / 2.0;
		centerY = metersLat / 2.0;
	}

	if(centerVertical)
	{
		double min = minElevation();
		double max = maxElevation();
		double avg = (min + max) / 2.0;
		elevationOffset = -avg;
	}

	printf("Writing position data\n");
	for(uint32_t y = 0; y < sizeLat; y++)
	{
		for(uint32_t x = 0; x < sizeLon; x++)
		{
			file << "v ";
			file << scale * (x * cellWidth  - centerX) << " ";
			file << scale * (y * cellHeight - centerY) << " ";
			file << scale * (elevationData->get(x, y) + elevationOffset);
			file << std::endl;
		}
	}

	printf("Writing UV data\n");
	for(uint32_t y = 0; y < sizeLat; y++)
	{
		for(uint32_t x = 0; x < sizeLon; x++)
		{
			file << "vt ";
			file << interpolate(x, 0, 0.0, sizeLon-1, 1.0);
			file << " ";
			file << interpolate(y, 0, 0.0, sizeLat-1, 1.0);
			file << std::endl;
		}
	}

	printf("Writing face data\n");
	uint32_t quadsX = sizeLon - 1;
	uint32_t quadsY = sizeLat - 1;
	for(uint32_t y = 0; y < quadsY; y++)
	{
		for(uint32_t x = 0; x < quadsX; x++)
		{
			uint32_t index0 = 1 + y * sizeLon + x;
			uint32_t index1 = 1 + y * sizeLon + x + sizeLon;
			uint32_t index2 = index1 + 1;
			uint32_t index3 = index0 + 1;

			file << "f ";
			file << index0 << "/" << index0 << " ";
			file << index1 << "/" << index1 << " ";
			file << index2 << "/" << index2 << " ";
			file << index3 << "/" << index3 << " ";
			file << std::endl;
		}
	}

	file.close();
}


size_t eleman::ElevationRegion::size() const
{
	// In case of ElevationRegion we always assume the whole region to be filled with data
	return sizeTotal();
}

size_t eleman::ElevationRegion::sizeTotal() const
{
	return sizeLon * sizeLat;
}

size_t eleman::ElevationRegion::memory() const
{
	return sizeof(this) + elevationData->memory();
}


