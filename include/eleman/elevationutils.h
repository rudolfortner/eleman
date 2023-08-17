// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef ELEVATIONUTILS_H
#define ELEVATIONUTILS_H

#include <cmath>
#include <stdint.h>
#include <string>

namespace eleman
{
	// Length of 1° longitude at the equator in meters
	static double ARC_LENGTH = 60.0 * 1852.0;

	/**
	 *	@todo
	 *	@param degreesLon Length of the arc in degrees of longitude
	 *	@param latitude At which latitude the given arc lies
	 */
	double degrees2meters(double degreesLon, double latitude);

	/**
	 *	@todo
	 *	@param metersLon Length of the arc in meters of longitude
	 *	@param latitude At which latitude the given arc lies
	 */
	double meters2degrees(double metersLon, double latitude);

	/**
	 *	Calculate the optimal grid dimensions for a region on the globe with the given size
	 *	@param dLat Dimension of the area in degrees of latitude
	 *	@param dLon Dimension of the area in degrees of longitude
	 *	@param referenceLatitude Reference latitude used for calculating the longitudal size in meters
	 *	@param precision Maximum distance between two grid points in meters
	 *
	 *	@result sizeLat Size of the grid in latitude  (y direction)
	 *	@result sizeLon Size of the grid in longitude (x direction)
	*/
	void calculateGridSize(double dLat, double dLon,
						   double referenceLatitude, double precision,
						   uint32_t& sizeLat, uint32_t& sizeLon);


	double interpolate(double x, double x0, double y0, double x1, double y1);

	std::string toHex(double value);

	template <typename T>
	bool inEllipse(T x, T y, T cx, T cy, T rx, T ry)
	{
		if(rx <= 0.0 || ry <= 0.0) return false;
		return pow(x - cx, 2.0) * pow(ry, 2.0) + pow(y - cy, 2.0) * pow(rx, 2.0) <= pow(rx, 2.0) * pow(ry, 2.0);
	}

	std::string formatMemory(size_t bytes);


	template <typename T>
	T roundMultiplier(T value, T multiplier)
	{
		return std::round(value * multiplier) / multiplier;
	}

	template <typename T>
	T roundDigits(T value, uint8_t digits)
	{
		return roundMultiplier(value, pow(10.0, digits));
	}


	template <typename T>
	T absmax(T a, T b)
	{
		a = fabs(a);
		b = fabs(b);
		return (b > a) ? b : a;
	}

	template <typename T>
	T diff(T a, T b)
	{
		return fabs(a - b);
	}

	template <typename T>
	bool check_equal(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
	{
		return diff(a, b) <= epsilon * absmax(a, b);
	}

	template <typename T>
	bool check_greater(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
	{
		return (a - b) > epsilon * absmax(a, b);
	}

	template <typename T>
	bool check_less(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
	{
		return (b - a) > epsilon * absmax(a, b);
	}

	template <typename T>
	bool check_range(T value, T lower, T upper, T epsilon = std::numeric_limits<T>::epsilon())
	{
		if(check_less(value, lower, epsilon)) return false;
		if(check_greater(value, upper, epsilon)) return false;
		return true;
	}

	template <typename T>
	bool check_bounds(T x, T y, T lowerX, T upperX, T lowerY, T upperY, T epsilon = std::numeric_limits<T>::epsilon())
	{
		if(!check_range(x, lowerX, upperX, epsilon)) return false;
		if(!check_range(y, lowerY, upperY, epsilon)) return false;
		return true;
	}

}	// end namespace eleman

#endif // ELEVATIONUTILS_H
