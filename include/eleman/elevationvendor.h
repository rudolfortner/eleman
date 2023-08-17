// SPDX-FileCopyrightText: 2023 <rudolf.ortner> <rudolf.ortner.rottenbach@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef ELEVATIONVENDOR_H
#define ELEVATIONVENDOR_H

#include "elevationdata.h"

#include <string>
#include <vector>

namespace eleman
{

	enum VendorResponseCode {
		OK,
		INVALID_REQUEST,
		SERVER_ERROR,
		UNDEFINED
	};

	struct VendorResponse {
		std::vector<ElevationData> results;
		VendorResponseCode code;
		std::string error;
	};

	/**
	* @todo write docs
	*/
	class ElevationVendor
	{
	public:
		bool gpxz = false;
		/**
		* Default constructor
		*/
		ElevationVendor();
		ElevationVendor(std::string name);

		/**
		* Destructor
		*/
		virtual ~ElevationVendor();

		virtual VendorResponse request(std::vector<Position> positions, std::string userAgent) const = 0;

		// Getters and setters
		std::string getID() const;
		std::string getName() const;
		std::string getBaseURL() const;

		uint16_t getLocationsPerRequest() const;
		uint16_t getRequestsPerSecond() const;
		uint16_t getRequestsPerDay() const;

	protected:
		std::string id = "unknown";
		std::string name = "Unknown Vendor, please set a name!";
		std::string baseURL;

		uint16_t locationsPerRequest;
		uint16_t requestsPerSecond;
		uint16_t requestsPerDay;

		void setID(const std::string& id);
		void setName(const std::string& name);
		void setBaseURL(const std::string& baseURL);

		void setLocationsPerRequest(uint16_t lpr);
		void setRequestsPerSecond(uint16_t rps);
		void setRequestsPerDay(uint16_t rpd);
	};


	/**
	 * @todo write docs
	 */
	class ElevationVendorHTTP : public ElevationVendor
	{
	public:
		ElevationVendorHTTP();
		~ElevationVendorHTTP();

		virtual VendorResponse request(std::vector<Position> positions, std::string userAgent) const = 0;
		virtual VendorResponse parseResult(std::string response) const = 0;

		std::string getSeparator() const;

	protected:
		std::string separator = "|";
		void setSeparator(const std::string& separator);
	};

	// TODO More vendor templates like ElevationVendorDatabase


}	// end namespace eleman
#endif // ELEVATIONVENDOR_H
