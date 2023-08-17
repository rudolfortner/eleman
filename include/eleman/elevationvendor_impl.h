// SPDX-FileCopyrightText: 2023 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef ELEVATIONVENDOROTD_H
#define ELEVATIONVENDOROTD_H

#include "elevationvendor.h"

namespace eleman
{

	/**
	* @todo write docs
	*/
	class ElevationVendorOTD : public ElevationVendorHTTP
	{
	public:
		/**
		* Default constructor
		*/
		ElevationVendorOTD(std::string dataset);

		/**
		* Destructor
		*/
		~ElevationVendorOTD();

		VendorResponse request(std::vector<Position> positions, std::string userAgent) const;
		VendorResponse parseResult(std::string response) const;

	private:
		std::string dataset;
	};

	/**
	 * @todo write docs
	 */
	class ElevationVendorGPXZ : public ElevationVendorHTTP
	{
	public:
		/**
		 * Default constructor
		 */
		ElevationVendorGPXZ(std::string api_key);

		/**
		 * Destructor
		 */
		~ElevationVendorGPXZ();

		VendorResponse request(std::vector<Position> positions, std::string userAgent) const;
		VendorResponse parseResult(std::string response) const;

	private:
		std::string api_key;
	};

}	// end namespace eleman
#endif // ELEVATIONVENDOROTD_H
