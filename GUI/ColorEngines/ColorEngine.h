/*

 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_COLOR_ENGINE_H
#define ESPINA_COLOR_ENGINE_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Types.h>

// Qt
#include <QColor>

// VTK
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

namespace ESPINA
{
  using LUTSPtr = vtkSmartPointer<vtkLookupTable>;

  namespace GUI
  {
    namespace ColorEngines
    {
      /** \class ColorEngine
       * \brief Base class for the classes that implement the algorithms for coloring the segmentations.
       *
       */
      class EspinaGUI_EXPORT ColorEngine
      : public QObject
      {
        Q_OBJECT

      public:
        enum Components
        {
          None         = 0x0,
          Color        = 0x1,
          Transparency = 0x2
        };
        Q_DECLARE_FLAGS(Composition, Components)

        using LUTMap = QMap<QString, LUTSPtr>;

      public:
        /** \brief Color Engine constructor
         * \param[in] id must be unique. It is used to store and retrieve information used
         * \param[in] tootip description of the color engine
         *
         */
        explicit ColorEngine(const QString &id, const QString &tooltip)
        : m_id     {id}
        , m_tooltip{tooltip}
        , m_active {false}
        {}

        /** \brief Returns the id of the Color Engine
         *
         */
        QString id() const
        { return m_id; }

        /** \brief Returns the tooltip of the Color Engine
         *
         */
        QString tooltip() const
        { return m_tooltip; }

        /** \brief Returns if a color engine should be used
         *
         */
        bool isActive() const
        { return m_active; }


        /** \brief Returns the color associated with the given segmentation.
         * \param[in] seg segmentation adapter raw pointer.
         *
         */
        virtual QColor color(ConstSegmentationAdapterPtr segmentation) = 0;

        /** \brief Returns the lut associated with the given segmentation.
         * \param[in] seg segmentation adapter raw pointer.
         *
         */
        virtual LUTSPtr lut (ConstSegmentationAdapterPtr segmentation) = 0;

        /** \brief Returns the flags of the composition methods supported by the color engine.
         *
         */
        virtual Composition supportedComposition() const = 0;

      public slots:
        /** \brief Notifies Color Engine observers that it has been requested to be used
         *
         */
        void setActive(bool active)
        {
          if(m_active != active)
          {
            m_active = active;

            emit activated(active);
          }
        }

      signals:
        void activated(bool active);

        void modified();

        void lutModified();

      private:
        QString m_id;      /** color engine identifier.            */
        QString m_tooltip; /** engine tooltip text.                */
        bool    m_active;  /** true if in use and false otherwise. */
      };

      Q_DECLARE_OPERATORS_FOR_FLAGS(ColorEngine::Composition)
    }
  }
}// namespace ESPINA

#endif // COLORENGINE_H
