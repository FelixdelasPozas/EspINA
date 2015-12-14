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
#include <Core/Types.h>
#include <Support/Settings/Settings.h>

namespace ESPINA
{
  class SeedGrowSegmentationSettings
  : public QObject
  {
    Q_OBJECT
  public:
    /** \brief SeedGrowSegmentationSettins class constructor.
     *
     */
    explicit SeedGrowSegmentationSettings();

    /** \brief SeedGrowSegmentationSettins class destructor.
     *
     */
    ~SeedGrowSegmentationSettings(){}

    /** \brief Sets X size.
     * \param[in] value size value.
     */
    void setXSize(int value);

    /** \brief Returns X size value.
     *
     */
    int xSize() const
    {return m_xSize;}

    /** \brief Sets Y size.
     * \param[in] value size value.
     *
     */
    void setYSize(int value);

    /** \brief Returns Y size value.
     *
     */
    int ySize() const
    {return m_ySize;}

    /** \brief Sets Z size.
     * \param[in] value size value.
     *
     */
    void setZSize(int value);

    /** \brief Returns Z size value.
     *
     */
    int zSize() const
    {return m_zSize;}

    /** \brief Sets apply category flag.
     * \param[in] value true to use category ROI size values.
     *
     */
    void setApplyCategoryROI(bool value);

    /** \brief Returns true if the tool will use the category size values for the ROI.
     *
     */
    bool applyCategoryROI() const
    { return m_applyCategoryROI; }

    /** \brief Sets the best pixel value for the selector.
     * \param[in] value (0-255) value.
     */
    void setBestPixelValue(int value);

    /** \brief Returns the value of the best pixel.
     *
     */
    int bestPixelValue() const
    { return m_bestValue; }

    /** \brief Sets the closing flag value.
     * \param[in] value true to apply a morphological close operation after a seedgrow segmentation operation.
     */
    void setCloseRadius(int value);

    /** \brief Returns true if a closing operation is applied after a seedgrow segmentation operation.
     *
     */
    int closeRadius() const
    {return m_radius;}

    /** \brief Enables/disables the close morphological operation after a successful seedgrow operation.
     * \param[in] enable true to enable and false otherwise.
     *
     */
    void setApplyClose(bool enable);

    /** \brief Returns true if after a seedgrow operation a morphological close will be applied to the result.
     *
     */
    bool applyClose() const
    { return m_applyClose; }

  signals:
    void applyCategoryROIChanged(bool value);
    void bestValueChanged(int value);

  private:
    static const QString SGS_GROUP;
    template<typename T> void set(const QString &key, T value);

    int  m_xSize, m_ySize, m_zSize, m_radius;
    int  m_bestValue;
    bool m_applyCategoryROI;
    bool m_applyClose;
  };

} // namespace ESPINA

template<typename T>
inline void ESPINA::SeedGrowSegmentationSettings::set(const QString& key, T value)
{
  ESPINA_SETTINGS(settings);



  settings.beginGroup(SGS_GROUP);
  settings.setValue(key, value);
  settings.endGroup();

  settings.sync();
}

#endif // ESPINA_SEED_GROW_SEGMENTATION_SETTINGS_H
