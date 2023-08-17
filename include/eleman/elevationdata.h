#ifndef ELEVATIONDATA_H
#define ELEVATIONDATA_H

namespace eleman
{

	struct Position {
		double latitude, longitude;
	};

	struct ElevationData {
		double latitude, longitude;
		double elevation;
	};

}	// end namespace eleman

#endif // ELEVATIONDATA_H
