// SPDX-FileCopyrightText: 2023 <rudolf.ortner> <rudolf.ortner.rottenbach@gmail.com>
// SPDX-License-Identifier: MIT

#include "eleman/elevationvendor.h"

#include "eleman/curlutil.h"
#include <nlohmann/json.hpp>
#include <sstream>


eleman::ElevationVendor::ElevationVendor()
{

}

eleman::ElevationVendor::ElevationVendor(std::string name)
{
	this->name = name;
}


eleman::ElevationVendor::~ElevationVendor()
{

}

void eleman::ElevationVendor::setID(const std::string& id)
{
	this->id = id;
}

std::string eleman::ElevationVendor::getID() const
{
	return id;
}

void eleman::ElevationVendor::setName(const std::string& name)
{
	this->name = name;
}

std::string eleman::ElevationVendor::getName() const
{
	return name;
}

void eleman::ElevationVendor::setBaseURL(const std::string& baseURL)
{
	this->baseURL = baseURL;
}

std::string eleman::ElevationVendor::getBaseURL() const
{
	return baseURL;
}



void eleman::ElevationVendor::setLocationsPerRequest(uint16_t lpr)
{
	this->locationsPerRequest = lpr;
}

uint16_t eleman::ElevationVendor::getLocationsPerRequest() const
{
	return locationsPerRequest;
}

void eleman::ElevationVendor::setRequestsPerSecond(uint16_t rps)
{
	this->requestsPerSecond = rps;
}

uint16_t eleman::ElevationVendor::getRequestsPerSecond() const
{
	return requestsPerSecond;
}

void eleman::ElevationVendor::setRequestsPerDay(uint16_t rpd)
{
	this->requestsPerDay = rpd;
}

uint16_t eleman::ElevationVendor::getRequestsPerDay() const
{
	return requestsPerDay;
}






eleman::ElevationVendorHTTP::ElevationVendorHTTP()
{

}

eleman::ElevationVendorHTTP::~ElevationVendorHTTP()
{

}

void eleman::ElevationVendorHTTP::setSeparator(const std::string& separator)
{
	this->separator = separator;
}

std::string eleman::ElevationVendorHTTP::getSeparator() const
{
	return separator;
}





