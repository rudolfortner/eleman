// SPDX-FileCopyrightText: 2023 <rudolf.ortner> <rudolf.ortner.rottenbach@gmail.com>
// SPDX-License-Identifier: MIT

#include "eleman/elevationio.h"

#include "eleman/elevationutils.h"
#include "eleman/elevationmanager.h"

#include <iomanip>
#include <fstream>
#include <nlohmann/json.hpp>

eleman::ElevationIO::ElevationIO(std::filesystem::path cacheDir)
{
	this->cacheDir = cacheDir;
	createCacheDir();
}

eleman::ElevationIO::~ElevationIO()
{

}

// TODO Rethink meaning of return value
bool eleman::ElevationIO::store(eleman::ElevationCacheCell& cell)
{
	if(!cell.isDirty()) return false;

	std::filesystem::path dir = getDirectory(cacheDir, cell);
	std::string filepath = dir / getFilename(cell);
	std::filesystem::create_directories(dir);
	printf("Storing cell %u in %s\n", cell.getID(), filepath.c_str());


	nlohmann::json jsonData;
	jsonData["cache"]["id"]		= cell.id;
	jsonData["cache"]["x"]		= cell.x;
	jsonData["cache"]["y"]		= cell.y;
	jsonData["cache"]["sizeLat"]= cell.sizeLat;
	jsonData["cache"]["sizeLon"]= cell.sizeLon;

	uint32_t count = 0;
	for(uint32_t y = 0; y < cell.sizeLat; y++)
	{
		for(uint32_t x = 0; x < cell.sizeLon; x++)
		{
			if(!cell.statusData->get(x, y)) continue;

			nlohmann::json object = nlohmann::json::object();
			object["x"]	= x;
			object["y"]	= y;
			object["elevation"]	= cell.elevationData->get(x, y);

			jsonData["cache"]["data"][count++] = object;
		}
	}

	std::ofstream file(filepath);
	file << jsonData.dump(1, '\t');
	file.close();

	cell.dirty = false;

	return true;
}

bool eleman::ElevationIO::load(eleman::ElevationCacheCell& cell)
{
	std::string filepath = getDirectory(cacheDir, cell) / getFilename(cell);
	if(!std::filesystem::exists(filepath)) return false;

	printf("Loading cell %u from %s\n", cell.getID(), filepath.c_str());

	std::ifstream file(filepath);
	nlohmann::json parsed = nlohmann::json::parse(file);
	file.close();

	if(parsed["cache"]["id"] != cell.id)
		throw std::runtime_error("FUCK");
	if(parsed["cache"]["x"]	!= cell.x)
		throw std::runtime_error("FUCK");
	if(parsed["cache"]["y"]	!= cell.y)
		throw std::runtime_error("FUCK");
	if(parsed["cache"]["sizeLat"]	!= cell.sizeLat)
		throw std::runtime_error("FUCK");
	if(parsed["cache"]["sizeLon"]	!= cell.sizeLon)
		throw std::runtime_error("FUCK");

	nlohmann::json data = parsed["cache"]["data"];
	for(size_t i = 0; i < data.size(); i++)
	{
		uint32_t x = data[i]["x"];
		uint32_t y = data[i]["y"];
		double elevation = data[i]["elevation"];

		cell.statusData->set(x, y, true);
		cell.elevationData->set(x, y, elevation);
	}
	cell.dirty = false;

	return true;
}


void eleman::ElevationIO::createCacheDir()
{
	std::filesystem::create_directories(cacheDir);
}

std::filesystem::path eleman::ElevationIO::getDirectory(const std::filesystem::path cacheDir, const eleman::ElevationCacheCell& cell)
{
	// By Vendor
	std::filesystem::path dir = cacheDir;
	dir /= cell.cache->getManager()->getVendor()->getID();

	// By CellDivisions
	std::stringstream ss;
	ss << std::setw(4) << std::setfill('0') << cell.cache->getCellDivisions();
	dir /= ss.str();

	// By Precision
	// TODO Maybe find better
	dir /= toHex(cell.getPrecision());

	return dir;
}

std::string eleman::ElevationIO::getFilename(const eleman::ElevationCacheCell& cell, const std::string& fileExtension)
{
	std::stringstream ss;
	ss << std::setw(2) << std::setfill('0') << cell.getID();
	ss << "." << fileExtension;

	return ss.str();
}


std::string eleman::cellname(uint8_t zoneNumber, char zoneLetter, int32_t eastID, int32_t northID)
{
	std::stringstream ss;
	ss << zoneLetter;
	ss << std::setw(2) << std::setfill('0') << zoneNumber;
	ss << "_";
	ss << (eastID <= 0 ? 'P' : 'N');
	ss << std::setw(6) << std::setfill('0') << abs(eastID);
	ss << "_";
	ss << (northID <= 0 ? 'P' : 'N');
	ss << std::setw(6) << std::setfill('0') << abs(northID);
	ss << ".ec";

	return ss.str();
}
