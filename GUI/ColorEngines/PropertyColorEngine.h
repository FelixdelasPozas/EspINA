/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_GUI_PROPERTYCOLORENGINE_H
#define ESPINA_GUI_PROPERTYCOLORENGINE_H

#include <GUI/ColorEngines/ColorEngine.h>
#include <Support/Context.h>

namespace ESPINA
{
  namespace GUI
  {
    class PropertyColorEngine
    : public ColorEngine
    , private Support::WithContext
    {
    public:
      explicit PropertyColorEngine(Support::Context &context);

      void setProperty(const SegmentationExtension::InfoTag &property, double min, double max);

      virtual QColor color(SegmentationAdapterPtr segmentation);

      virtual LUTSPtr lut(SegmentationAdapterPtr segmentation);

      virtual Composition supportedComposition() const
      { return ColorEngine::Color; }

    private:
      double adjustRange(double value) const;

      double interpolateFactor(double value) const;

    private:
      double m_minValue;
      double m_maxValue;

      QColor m_minColor;
      QColor m_maxColor;

      QString m_extensionType;
      SegmentationExtension::InfoTag m_property;

      QMap<QColor, LUTSPtr> m_luts;
    };
  }
}

#endif // ESPINA_GUI_PROPERTYCOLORENGINE_H
