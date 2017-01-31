/// @file   heightMapper.hpp
/// @author Enrico Fraccaroli
/// @date   Jan 05 2017
/// @copyright
/// Copyright (c) 2017 Enrico Fraccaroli <enrico.fraccaroli@gmail.com>
/// Permission is hereby granted, free of charge, to any person obtaining a
/// copy of this software and associated documentation files (the "Software"),
/// to deal in the Software without restriction, including without limitation
/// the rights to use, copy, modify, merge, publish, distribute, sublicense,
/// and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///     The above copyright notice and this permission notice shall be included
///     in all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
/// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.

#pragma once

#include "mapTile.hpp"
#include "mapCell.hpp"

namespace terrain
{
class Terrain;
}

/// @brief Class which maps the values of an height-map to types of terrain.
class HeightMapper
{
public:
    /// The vnum of the mapper.
    unsigned int vnum;
    /// The name of the mapper.
    std::string name;
    /// The current thresholds.
    std::map<MapTile, double> thresholdMap;

    /// @brief Constructor.
    HeightMapper(const unsigned int & _vnum,
                 const std::string & _name);

    /// @brief Reset the thresholds to default values.
    void reset();

    void addThreshold(const std::shared_ptr<terrain::Terrain> & terrain,
                      const double & threshold);

    /// @brief Returns the MapTile associated with the given height w.r.t.
    /// the current thresholds.
    MapTile getHeightMap(const double & height);
};