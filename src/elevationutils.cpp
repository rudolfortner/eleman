// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "eleman/elevationutils.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>


double eleman::degrees2meters(double degreesLon, double latitude)
{
	return ARC_LENGTH * std::abs(degreesLon) * cos(latitude * M_PI / 180.0);
}

double eleman::meters2degrees(double metersLon, double latitude)
{
	return metersLon / (ARC_LENGTH * cos(latitude * M_PI / 180.0));
}

void eleman::calculateGridSize(double dLat, double dLon,
							   double referenceLatitude, double precision,
							   uint32_t& sizeLat, uint32_t& sizeLon)
{
	// Latitudal distance always the same
	double metersLat = degrees2meters(dLat, 0.0);
	double metersLon = degrees2meters(dLon, referenceLatitude);

	uint32_t divisionsLat = ceil(metersLat / precision);
	uint32_t divisionsLon = ceil(metersLon / precision);

	sizeLat = divisionsLat + 1;
	sizeLon = divisionsLon + 1;
}

double eleman::interpolate(double x, double x0, double y0, double x1, double y1)
{
	// Corner case
	if(x0 == x1 && y0 == y1) return y0;

	return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

std::string eleman::toHex(double value)
{
	std::stringstream ss;
	uint64_t ll = (*(uint64_t*) (&value));
	ss << std::hex << std::setw(16) << std::setfill('0') << ll;

	return ss.str();
}


std::string eleman::formatMemory(size_t bytes)
{
	std::string suffix[] = {"B", "KB", "MB", "GB", "TB", "PB"};

	size_t length = sizeof(suffix) / sizeof(suffix[0]);

	int i;
	double dblBytes = bytes;
	for(i = 0; (bytes / 1024) > 0 && i < length-1; i++, bytes /= 1024) {
		dblBytes = bytes / 1024.0;
	}


	std::stringstream ss;
	ss << std::fixed;
	ss << std::setprecision(2);
	ss << dblBytes;
	ss << " ";
	ss << suffix[i];

	return ss.str();
}

