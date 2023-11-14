// SPDX-FileCopyrightText: 2023 <rudolf.ortner> <rudolf.ortner.rottenbach@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef GRID_H
#define GRID_H

#include <algorithm>
#include <stdint.h>

/**
 * @todo write docs
 */
template <typename ElemType>
class Grid
{
public:

    /**
     * Default constructor
     */
	Grid(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;
		this->data = new ElemType[width * height];
	}

	Grid(uint32_t width, uint32_t height, ElemType value)
	{
		this->width = width;
		this->height = height;
		this->data = new ElemType[width * height];

		for(size_t i = 0; i < width * height; i++)
			this->data[i] = value;
	}

	Grid(const Grid& grid)
	{
		this->width		= grid.width;
		this->height	= grid.height;
		this->data		= new ElemType[width * height];

		for(size_t i = 0; i < width * height; i++)
			this->data[i] = grid.data[i];
	}

	/**
	* Destructor
	*/
	~Grid()
	{
		delete[] data;
	}


	ElemType& at(const uint32_t& x, const uint32_t& y)
	{
		return data[y * width + x];
	}

	ElemType get(const uint32_t& x, const uint32_t& y) const
	{
		return data[y * width + x];
	}

	void set(const uint32_t& x, const uint32_t& y, const ElemType& value)
	{
		data[y * width + x] = value;
	}


	void resize(const uint32_t& width, const uint32_t& height)
	{
		ElemType* newData = new ElemType[width * height];

		uint32_t rangeX = std::min(this->width, width);
		uint32_t rangeY = std::min(this->height, height);

		for(uint32_t y = 0; y < rangeY; y++)
		{
			for(uint32_t x = 0; x < rangeX; x++)
			{
				newData[y * width + x] = data[y * width + x];
			}
		}
		delete[] data;
		data = newData;
	}


	uint32_t getWidth() const
	{
		return width;
	}
	uint32_t getHeight() const
	{
		return height;
	}

	uint64_t memory() const
	{
		return sizeof(Grid<ElemType>) +  width * height * sizeof(ElemType);
	}

private:
	ElemType* data;
	uint32_t width, height;
};

#endif // GRID_H
