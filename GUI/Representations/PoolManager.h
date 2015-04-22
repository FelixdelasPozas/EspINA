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

#ifndef ESPINA_POOL_MANAGER_H
#define ESPINA_POOL_MANAGER_H

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

  class EspinaGUI_EXPORT PoolManager
  : public RepresentationManager
  {
    Q_OBJECT
  public:
    /** \brief RangedActorManager class virtual destructor.
     *
     */
    virtual ~PoolManager()
    {}

  protected:
    /** \brief RangedActorManager class protected constructor.
     * \param[in] supportedViews flags of the views supported by the manager.
     */
    explicit PoolManager(ViewTypeFlags supportedViews, ManagerFlags flags);

    bool hasActorsInDisplay() const;

  private:
    virtual void displayRepresentations(TimeStamp t) override;

    virtual void hideRepresentations(TimeStamp t) override;

    virtual RepresentationPipeline::Actors actors(TimeStamp t) = 0;

    virtual void invalidatePreviousActors(TimeStamp t) = 0;

    virtual RepresentationManagerSPtr cloneImplementation() = 0;

  protected:
    RepresentationPipeline::Actors m_viewActors; // actors being rendered by its view
  };

}// namespace ESPINA

#endif // ESPINA_POOL_MANAGER_H
