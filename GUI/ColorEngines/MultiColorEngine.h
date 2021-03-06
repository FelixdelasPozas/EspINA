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

#ifndef ESPINA_MULTI_COLOR_ENGINE_H
#define ESPINA_MULTI_COLOR_ENGINE_H

// ESPINA
#include "ColorEngine.h"

#include <GUI/Types.h>

// Qt
#include <QList>
#include <QReadWriteLock>

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      class EspinaGUI_EXPORT MultiColorEngine
      : public ColorEngine
      {
        Q_OBJECT

      public:
        explicit MultiColorEngine();

        virtual QColor color(ConstSegmentationAdapterPtr seg);

        virtual LUTSPtr lut(ConstSegmentationAdapterPtr seg);

        virtual ColorEngine::Composition supportedComposition() const;

        /** \brief Adds a color engine.
         * \param[in] engine to be added
         *
         */
        virtual void add(ColorEngineSPtr engine);

        /** \brief Removes a color engine.
         * \param[in] engine to be removed
         *
         */
        virtual void remove(ColorEngineSPtr engine);

        /** \brief Returns the list of active color engines in this color engine.
         *
         */
        const QList<ColorEngineSPtr> activeEngines() const;

        /** \brief Returns the list of registered color engines within this color engine.
         *
         */
        const QList<ColorEngineSPtr> availableEngines() const;

        /** \brief Returns the engine with the given id or nullptr if not registered in this engine.
         *
         */
        const ColorEngineSPtr getEngine(const QString &engineId);

        virtual ColorEngineSPtr clone()
        { return std::make_shared<MultiColorEngine>(); }

      private slots:
        void onColorEngineActivated(bool active);

      private:
        mutable QReadWriteLock m_lock;             /** protection mutex.               */

        QList<ColorEngine *>   m_activeEngines;    /** list of active engines.         */
        QList<ColorEngineSPtr> m_availableEngines; /** list of all registered engines. */
      };
    }
  }
}// namespace ESPINA

#endif // ESPINA_MULTI_COLOR_ENGINE_H
