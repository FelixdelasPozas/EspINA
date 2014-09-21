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

// ESPINA
#include <Core/EspinaTypes.h>

namespace ESPINA
{
  class SeedGrowSegmentationSettings
  : public QObject
  {
    Q_OBJECT
  public:
    /** brief SeedGrowSegmentationSettins class constructor.
     *
     */
    explicit SeedGrowSegmentationSettings();

    /** brief SeedGrowSegmentationSettins class destructor.
     *
     */
    ~SeedGrowSegmentationSettings(){}

    /** brief Sets X size.
     * \param[in] value, size value.
     */
    void setXSize(int value);

    /** brief Returns X size value.
     *
     */
    int xSize() const
    {return m_xSize;}

    /** brief Sets Y size.
     * \param[in] value, size value.
     *
     */
    void setYSize(int value);

    /** brief Returns Y size value.
     *
     */
    int ySize() const
    {return m_ySize;}

    /** brief Sets Z size.
     * \param[in] value, size value.
     *
     */
    void setZSize(int value);

    /** brief Returns Z size value.
     *
     */
    int zSize() const
    {return m_zSize;}

    /** brief Sets apply category flag.
     * \param[in] value, true to use category ROI size values.
     *
     */
    void setApplyCategoryROI(bool value);

    /** brief Returns true if the tool will use the category size values for the ROI.
     *
     */
    bool applyCategoryROI() const
    { return m_applyCategoryROI; }

    /** brief Sets the best pixel value for the selector.
     * \param[in] value, (0-255) value.
     */
    void setBestPixelValue(int value);

    /** brief Returns the value of the best pixel.
     *
     */
    int bestPixelValue() const
    { return m_bestValue; }

    /** brief Sets the closing flag value.
     * \param[in] value, true to apply a morpholofical close operation after a seedgrow segmentation operation.
     */
    void setClosing(int value);

    /** brief Returns true if a closing operation is applied after a seedgrow segmentation operation.
     *
     */
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
