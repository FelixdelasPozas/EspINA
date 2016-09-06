/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef PLUGINS_COUNTINGFRAME_REPRESENTATIONS_REPRESENTATIONSWITCH_H_
#define PLUGINS_COUNTINGFRAME_REPRESENTATIONS_REPRESENTATIONSWITCH_H_

#include "CountingFramePlugin_Export.h"

// ESPINA
#include <GUI/Widgets/NumericalInput.h>
#include <Support/Representations/BasicRepresentationSwitch.h>

// C++
#include <memory>

class QSettings;

namespace ESPINA
{
  namespace CF
  {
    /** \class RepresentationSwitch
     * \brief Representation switch for 3D counting frame representation.
     *
     */
    class CountingFramePlugin_EXPORT CFRepresentationSwitch
    : public BasicRepresentationSwitch
    {
        Q_OBJECT
      public:
        /** \brief RepresentationSwith class constructor.
         *
         */
        explicit CFRepresentationSwitch(GUI::Representations::RepresentationManagerSPtr manager,
                                      Support::Context                               &context);

        /** \brief RepresentationSwitch class virtual destructor.
         *
         */
        virtual ~CFRepresentationSwitch();

        virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

        virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

      signals:
        void opacityChanged(float);

      private slots:
        /** \brief Propagates changes in opacity to the widgets.
         *
         */
        void onOpacityChanged(int value);

      private:
        /** \brief Initializes the opacity widget of the switch.
         *
         */
        void initWidget();

        GUI::Widgets::NumericalInput *m_opacityWidget;
    };
  
  } // namespace CF
} // namespace ESPINA

#endif // PLUGINS_COUNTINGFRAME_REPRESENTATIONS_REPRESENTATIONSWITCH_H_
