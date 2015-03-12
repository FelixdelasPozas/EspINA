/*

    Copyright (C) 2014  Jorge Pe�a Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_ACTOR_MANAGER_H
#define ESPINA_ACTOR_MANAGER_H

#include "RepresentationManager.h"
#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/View/SelectableView.h>
#include <GUI/View/ViewTypeFlags.h>
#include <GUI/Representations/RepresentationPipeline.h>

// Qt
#include <QString>
#include <QIcon>

namespace ESPINA
{
  class RenderView;

  class EspinaGUI_EXPORT ActorManager
  : public RepresentationManager
  {
    Q_OBJECT
  public:
    virtual ~ActorManager()
    {}

    /** \brief Updates view's actors with those available at the given time.
     *
     */
    void display(TimeStamp time);

  protected:
    explicit ActorManager(ViewTypeFlags supportedViews);

  private:
    virtual bool hasSources() const = 0;

    virtual void onHide();

    virtual void onShow();

    virtual RepresentationPipeline::Actors actors(TimeStamp time) = 0;

    virtual void invalidatePreviousActors(TimeStamp time) = 0;

    virtual void connectPools() = 0;

    virtual void disconnectPools() = 0;

    virtual RepresentationManagerSPtr cloneImplementation() = 0;

    void enableRepresentations();

    void disableRepresentations();

    void removeCurrentActors();

    void displayActors(const TimeStamp time);

  private:
    RepresentationPipeline::Actors m_viewActors; // actors being rendered by its view
  };

}// namespace ESPINA

#endif // ESPINA_REPRESENTATION_MANAGER_H
