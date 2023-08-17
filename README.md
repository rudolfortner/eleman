# eleman - Elevation Manager

Elevation Manager is a small library for requesting, caching and using elevation data.

## Structure
The whole system is structured into separate smaller modules. This makes it easier for developers to adapt the code and to add new custom compontents.

* elevationmanager.cpp
is used as the main interface to the system when it comes to elevation requests. Here also concurrency is handled.

* elevationcache.cpp
is used to cache the retrieved elevation data and handles cache requests as well as cache misses.

* elevationio.cpp
is used to store the cached data on disk in json. In theory this could be extended to any backend.

* elevationvendor.cpp
is used to represent a vendor that we can use to retrieve elevation data from. In our case there is a predefined class for retrieving data via a REST API used for example for [Google Elevation API](https://developers.google.com/maps/documentation/elevation/start), [OpenTopoData](https://www.opentopodata.org/) and [GPXZ.io](https://www.gpxz.io/). Again, due to the modularity of the project it is possible to extend this to database access or loading data from geoTIFFs or something similar (see [Future](#Future))

* elevationdownloader.cpp
is meant more like a tool and should be used for precaching elevation data in the background


## Building
Elevation Manager uses [CMake](https://cmake.org/) as a build system. By default it builds into a static library.

## Usage
There is an example file [demo.cpp](demo.cpp) that shows how eleman might be used in an application.

## Future
Here is a list of features/changes we might implement in the future:

* Better API and project structure
* Better error handling
* Better statistics/telemetry
* Dynamic Cache that adapts to the vendor's resolution
* 


## License
This project is under [MIT License](LICENSE) without any warranty or support.