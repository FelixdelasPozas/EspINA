/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_SEED_GROW_SEGMENTATION_SETTINGS_H
#define ESPINA_SEED_GROW_SEGMENTATION_SETTINGS_H

#include <Core/EspinaTypes.h>

namespace ESPINA
{
  class SeedGrowSegmentationSettings
  : public QObject
  {
    Q_OBJECT
  public:
    explicit SeedGrowSegmentationSettings();
    ~SeedGrowSegmentationSettings(){}

    void setXSize(int value);
    int xSize() const {return m_xSize;}

    void setYSize(int value);
    int ySize() const {return m_ySize;}

    void setZSize(int value);
    int zSize() const {return m_zSize;}

    void setApplyCategoryROI(bool value);
    bool applyCategoryROI() const
    { return m_applyCategoryROI; }

    void setBestPixelValue(int value);
    int bestPixelValue() const
    { return m_bestValue; }

    void setClosing(int value);
    int closing() const {return m_closing;}

  signals:
    void applyCategoryROIChanged(bool value);
    void bestValueChanged(int value);

  private:
    int  m_xSize, m_ySize, m_zSize, m_closing;
    int  m_bestValue;
    bool m_applyCategoryROI;
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_SETTINGS_H
