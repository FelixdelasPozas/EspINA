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
  /** \class SeedGrowSegmentationSettings
   * \brief Implements seed grow segmentation tool settins storage.
   *
   */
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
      virtual ~SeedGrowSegmentationSettings()
      {};

      /** \brief Sets X size.
       * \param[in] value size value.
       */
      void setXSize(long long value);

      /** \brief Returns X size value.
       *
       */
      inline long long xSize() const
      { return m_xSize; }

      /** \brief Sets Y size.
       * \param[in] value size value.
       *
       */
      void setYSize(long long value);

      /** \brief Returns Y size value.
       *
       */
      inline long long ySize() const
      { return m_ySize; }

      /** \brief Sets Z size.
       * \param[in] value size value.
       *
       */
      void setZSize(long long value);

      /** \brief Returns Z size value.
       *
       */
      inline long long zSize() const
      { return m_zSize; }

      /** \brief Sets apply category ROI flag.
       * \param[in] value true to use category ROI size values and false otherwise.
       *
       */
      void setApplyCategoryROI(bool value);

      /** \brief Returns true if the tool will use the category size values for the ROI.
       *
       */
      inline bool applyCategoryROI() const
      { return m_applyCategoryROI; }

      /** \brief Sets the best pixel value for the selector.
       * \param[in] value (0-255) value.
       */
      void setBestPixelValue(int value);

      /** \brief Returns the value of the best pixel.
       *
       */
      inline int bestPixelValue() const
      { return m_bestValue; }

      /** \brief Sets the closing operation radius.
       * \param[in] value Numerical value in [1, N).
       */
      void setCloseRadius(int value);

      /** \brief Returns the close operation radius.
       *
       */
      inline int closeRadius() const
      { return m_radius; }

      /** \brief Enables/disables the close morphological operation after a successful seedgrow operation.
       * \param[in] enable true to enable and false otherwise.
       *
       */
      void setApplyClose(bool enable);

      /** \brief Returns true if after a seedgrow operation a morphological close will be applied to the result.
       *
       */
      inline bool applyClose() const
      { return m_applyClose; }

    signals:
      void applyCategoryROIChanged(bool value);
      void bestValueChanged(int value);
      void closeRadiusChanged(int value);
      void applyCloseChanged(bool value);

    private:
      static const QString SGS_GROUP;                             /** SeedGrowSegmentation group id. */

      /** \brief Helper method to modify the settings.
       * \param[in] key key id.
       * \param[in] value key value.
       *
       */
      template<typename T> void set(const QString &key, T value);

      long long m_xSize;            /** size of ROI in the X axis.                                              */
      long long m_ySize;            /** size of ROI in the Y axis.                                              */
      long long m_zSize;            /** size of ROI in the Z axis.                                              */
      int       m_radius;           /** close radius value.                                                     */
      int       m_bestValue;        /** best pixel value.                                                       */
      bool      m_applyCategoryROI; /** true to apply ROI values and false otherwise.                           */
      bool      m_applyClose;       /** true to apply a morphological close after seedgrow and false otherwise. */
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
