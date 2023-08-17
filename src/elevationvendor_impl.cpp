// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "eleman/curlutil.h"
#include "eleman/elevationvendor_impl.h"
#include "eleman/elevationexception.h"

#include <nlohmann/json.hpp>
#include <sstream>

eleman::ElevationVendorOTD::ElevationVendorOTD(std::string dataset)
{
	this->baseURL = "https://api.opentopodata.org/v1/";
	this->dataset = dataset;

	this->id	= "otd_" + dataset;
	this->name	= "OpenTopoData (" + dataset + ")";

	this->locationsPerRequest = 100;
	this->requestsPerSecond = 1;
	this->requestsPerDay = 1000;
}

eleman::ElevationVendorOTD::~ElevationVendorOTD()
{

}

eleman::VendorResponse eleman::ElevationVendorOTD::request(std::vector<Position> positions, std::string userAgent) const
{
	// "https://api.opentopodata.org/v1/test-dataset?locations=56,123"

	std::stringstream ss;
	ss << baseURL << dataset << "?locations=";
	ss.precision(16);

	for(size_t i = 0; i < positions.size(); i++)
	{
		ss << positions[i].latitude;
		ss << ",";
		ss << positions[i].longitude;

		if(i < positions.size() - 1)
			ss << separator;
	}

	std::string response = eleman::curlRequest(ss.str(), userAgent);

	printf("Request: %s\n", ss.str().c_str());
	printf("Response:\n%s\n", response.c_str());

	// TODO Some curl error handling (no response, no internet, ...)

	return parseResult(response);
}

eleman::VendorResponse eleman::ElevationVendorOTD::parseResult(std::string response) const
{
	nlohmann::json parsed = nlohmann::json::parse(response);

	VendorResponse vendorResponse;

	auto results = parsed["results"];
	for(size_t i = 0; i < results.size(); i++)
	{
		if(results[i]["elevation"].is_null())
			throw ElevationException("Dataset " + dataset + " has no data for given location");

		ElevationData result;
		result.latitude		= results[i]["location"]["lat"];
		result.longitude	= results[i]["location"]["lng"];
		result.elevation	= results[i]["elevation"];
		vendorResponse.results.push_back(result);
	}

	std::string status	= parsed["status"];
	if(status == "OK")
	{
		vendorResponse.code = eleman::OK;
	}
	else if(status == "INVALID_REQUEST")
	{
		vendorResponse.code = eleman::INVALID_REQUEST;
		vendorResponse.error= parsed["error"];
	}
	else if(status == "SERVER_ERROR")
	{
		vendorResponse.code = eleman::SERVER_ERROR;
		vendorResponse.error= parsed["error"];
	}
	else
	{
		vendorResponse.code = eleman::UNDEFINED;
	}

	return vendorResponse;
}


eleman::ElevationVendorGPXZ::ElevationVendorGPXZ(std::string api_key)
{
	this->baseURL = "https://api.gpxz.io/v1/elevation/points";
	this->api_key = api_key;

	this->id = "gpxz";
	this->name = "gpxz.io";

	this->locationsPerRequest = 50;
	this->requestsPerSecond = 1;
	this->requestsPerDay = 100;
}

eleman::ElevationVendorGPXZ::~ElevationVendorGPXZ()
{

}

eleman::VendorResponse eleman::ElevationVendorGPXZ::request(std::vector<Position> positions, std::string userAgent) const
{
	std::stringstream ss;
	ss << baseURL;
	ss << "?api-key=" << api_key;
	ss << "&latlons=";
	ss.precision(16);

	for(size_t i = 0; i < positions.size(); i++)
	{
		ss << positions[i].latitude;
		ss << ",";
		ss << positions[i].longitude;

		if(i < positions.size() - 1)
			ss << separator;
	}

	std::string response = curlRequest(ss.str(), userAgent);

	printf("Request: %s\n", ss.str().c_str());
	printf("Response:\n%s\n", response.c_str());

	// TODO Some curl error handling (no response, no internet, ...)

	return parseResult(response);
}

eleman::VendorResponse eleman::ElevationVendorGPXZ::parseResult(std::string response) const
{
	nlohmann::json parsed = nlohmann::json::parse(response);

	VendorResponse vendorResponse;

	auto results = parsed["results"];
	for(size_t i = 0; i < results.size(); i++)
	{
		ElevationData result;
		result.latitude		= results[i]["lat"];
		result.longitude	= results[i]["lon"];
		result.elevation	= results[i]["elevation"];
		vendorResponse.results.push_back(result);
	}

	std::string status	= parsed["status"];
	if(status == "OK")
	{
		vendorResponse.code = eleman::OK;
	}
	else if(status == "INVALID_REQUEST")
	{
		vendorResponse.code = eleman::INVALID_REQUEST;
		vendorResponse.error= parsed["error"];
	}
	else if(status == "SERVER_ERROR")
	{
		vendorResponse.code = eleman::SERVER_ERROR;
		vendorResponse.error= parsed["error"];
	}
	else
	{
		vendorResponse.code = eleman::UNDEFINED;
	}

	return vendorResponse;
}


