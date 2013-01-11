/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SEEDGROWSEGMENTATION_SETTINGS_H
#define SEEDGROWSEGMENTATION_SETTINGS_H

#include <Core/EspinaTypes.h>

namespace EspINA
{
  class BestPixelPicker;

  class SeedGrowSegmentationSettings
  {
  public:
    explicit SeedGrowSegmentationSettings(BestPixelPicker *selector);
    ~SeedGrowSegmentationSettings(){}

    void setXSize(int value);
    int xSize() const {return m_xSize;}

    void setYSize(int value);
    int ySize() const {return m_ySize;}

    void setZSize(int value);
    int zSize() const {return m_zSize;}

    void setTaxonomicalVOI(bool value);
    bool taxonomicalVOI() const { return m_taxonomicalVOI; }

    void setBestPixelValue(int value);
    int bestPixelValue() const;

    void setClosing(int value);
    int closing() const {return m_closing;}

  private:
    BestPixelPicker *m_selector;
    int m_xSize, m_ySize, m_zSize, m_closing;
    bool m_taxonomicalVOI;
  };

} // namespace EspINA

#endif // SEEDGROWSEGMENTATION_SETTINGS_H
