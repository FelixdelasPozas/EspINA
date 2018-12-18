/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#ifndef GUI_COLORENGINES_CONNECTIONSCOLORENGINE_H_
#define GUI_COLORENGINES_CONNECTIONSCOLORENGINE_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/ColorEngines/ColorEngine.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      /** \class ConnectionsColorEngine
       * \brief Colors the segmentations according to the number of connections it has.
       *
       */
      class EspinaGUI_EXPORT ConnectionsColorEngine
      : public ColorEngine
      {
        public:
          /** \brief ConnectionsColorEngine class constructor.
           *
           */
          ConnectionsColorEngine();

          /** \brief ConnectionsColorEngine class virtual destructor.
           *
           */
          ~ConnectionsColorEngine()
          {}

          virtual QColor color(ConstSegmentationAdapterPtr segmentation);

          virtual LUTSPtr lut (ConstSegmentationAdapterPtr segmentation);

          virtual Composition supportedComposition() const
          { return ColorEngine::Color; }

          virtual ColorEngineSPtr clone()
          { return std::make_shared<ConnectionsColorEngine>(); }

          /** \brief Sets the connection criteria for classifying segmentations.
           * \param[in] criteria List of segmentations' categories that represents a valid connection.
           * \param[in] valid Valid color hue value.
           * \param[in] invalid Invalid color hue value.
           * \param[in] incomplete Incomplete color hue value.
           * \param[in] unconnected Unconnected color hue value.
           *
           */
          void setCriteriaInformation(const QStringList &criteria,
                                      int valid       = QColor{Qt::green}.hue(),
                                      int invalid     = QColor{Qt::red}.hue(),
                                      int incomplete  = QColor{Qt::blue}.hue(),
                                      int unconnected = QColor{Qt::yellow}.hue());

        private:
          /** \brief Helper method to evaluate if a list of connections is valid according to the current
           * connection criteria.
           * \param[in] connections List of segmentation connections.
           *
           */
          const bool isValid(const ConnectionList &connections) const;

          int         m_validHUE;       /** hue value for valid segmentations according to criteria.                */
          int         m_invalidHUE;     /** hue value for invalid segmentations according to criteria.              */
          int         m_incompleteHUE;  /** hue value for incomplete segmentations according to criteria.           */
          int         m_unconnectedHUE; /** hue value for unconnected segmentations according to criteria.          */
          QStringList m_criteria;       /** definition of valid connection using a list of segmentation categories. */
      };
    }
  }
}


#endif // GUI_COLORENGINES_CONNECTIONSCOLORENGINE_H_
