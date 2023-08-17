#include <iostream>

#include <curl/curl.h>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <chrono>
#include <thread>

#include <eleman/elevationcache.h>
#include <eleman/elevationio.h>
#include <eleman/elevationmanager.h>
#include <eleman/elevationutils.h>
#include <eleman/elevationvendor.h>
#include <eleman/elevationvendor_impl.h>


int main(int argc, char **argv) {
	std::cout << "[Elevation Manager] Starting up..." << std::endl;

	// DEFINE VENDORS
	eleman::ElevationVendorOTD otd_eudem25m("eudem25m");
	eleman::ElevationVendorOTD otd_mapzen("mapzen");
	eleman::ElevationVendorOTD otd_aster30m("aster30m");
	eleman::ElevationVendorOTD otd_srtm30m("srtm30m");
	eleman::ElevationVendorGPXZ gpxz("--- API-KEY ---");


	// Create an IO for the Cache
	eleman::ElevationIO fileIO(".");

	// Create a Cache
	eleman::ElevationCache cache;
	cache.setIO(&fileIO);

	// Create the actual manager
	eleman::ElevationManager manager;

	// Link all components together
	manager.setCache(&cache);
	manager.setVendor(&otd_eudem25m);
	cache.setManager(&manager);

	// Precache example, might take some while
	//cache.precacheRegion(47.64, 12.4, 48.78, 14.94);	// Upper Austria

	// Request elevation of single position
	eleman::ElevationData data = manager.get(47.086476, 12.679198);
	printf("Elevation at %f/%f is %f\n", data.latitude, data.longitude, data.elevation);

	// Request a full region (10m grid resolution)
	eleman::ElevationRegion region = manager.get(47.07, 12.67, 47.08, 12.68, 10.0);
	printf("Region contains %02zu points\n", region.size());

	std::cout << "[Elevation Manager] Shutting down..." << std::endl;
    return 0;
}
