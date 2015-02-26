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

#ifndef ESPINA_REPRESENTATION_MANAGER_H
#define ESPINA_REPRESENTATION_MANAGER_H

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
  class RepresentationManager;
  using RepresentationManagerSPtr  = std::shared_ptr<RepresentationManager>;
  using RepresentationManagerSList = QList<RepresentationManagerSPtr>;

  class RenderView;

  class EspinaGUI_EXPORT RepresentationManager
  : public QObject
  {
    Q_OBJECT
  public:
    enum class PipelineStatus: int8_t { NOT_READY = 1, READY = 2, RANGE_DEPENDENT = 3 };

  public:
    virtual ~RepresentationManager()
    {}

    /** \brief Sets the name of the representation manager
     *
     */
    void setName(const QString &name);

    /** \brief Returns the name of the representation manager
     *
     */
    QString name() const;

    /** \brief Sets the description of the representation manager
     *
     */
    void setDescription(const QString &description);

    /** \brief Returns the description of the representation manager
     *
     */
    QString description() const;

    /** \brief Sets the icon of the representation manager
     *
     */
    void setIcon(const QIcon &icon);

    /** \brief Returns the icon of the representation manager
     *
     */
    QIcon icon() const;

    /** \brief Sets the view where representation are managed
     *
     */
    void setView(RenderView *view);

    ViewTypeFlags supportedViews() const
    { return m_supportedViews; }

    /** \brief Specify the resolution of the view
     *
     */
    virtual void setResolution(const NmVector3 &resolution) = 0;

    /** \brief Returns if managed representations are visible or not
     *
     */
    bool representationsVisibility() const
    { return m_showPipelines; }

    /** \brief Shows all representations
     *
     */
    void show();

    /** \brief Hides all representations
     *
     */
    void hide();

    /** \brief Returns if the manager has been requested to display its actors
     *
     */
    bool isActive();

    bool requiresRender() const;

    /** \brief Returns the status of its pipelines.
     *
     */
    virtual PipelineStatus pipelineStatus() const = 0;

    /** \brief Returns the range of ready pipelines.
     *
     */
    virtual TimeRange readyRange() const = 0;

    /** \brief Updates view's actors with those available at the given time.
     *
     */
    void display(TimeStamp time);

    /** \brief Returns the item picked
     *
     */
    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const = 0;

    /** \brief Returns a new instance of the class.
     *
     */
    RepresentationManagerSPtr clone();

  public slots:
    void onCrosshairChanged(NmVector3 crosshair, TimeStamp time);

    /** \brief Set representations visibility
     *
     */
    void setRepresentationsVisibility(bool value);

  signals:
    void renderRequested();

  protected slots:
    void emitRenderRequest(TimeStamp time);

    void invalidateActors();

  protected:
    explicit RepresentationManager(ViewTypeFlags supportedViews);

  private:
    virtual bool hasSources() const = 0;

    virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp time) = 0;

    virtual RepresentationPipeline::Actors actors(TimeStamp time) = 0;

    virtual void invalidatePreviousActors(TimeStamp time) = 0;

    virtual void connectPools() = 0;

    virtual void disconnectPools() = 0;

    virtual RepresentationManagerSPtr cloneImplementation() = 0;

    void enableRepresentations();

    void disableRepresentations();

    void removeCurrentActors();

    void displayActors(const TimeStamp time);

  protected:
    QString m_name;
    QIcon   m_icon;
    QString m_description;
    bool    m_showPipelines;
    bool    m_requiresRender;

  private:
    RenderView   *m_view;
    ViewTypeFlags m_supportedViews;
    NmVector3     m_crosshair;
    TimeStamp     m_lastRequestTime;
    TimeStamp     m_lastRenderRequestTime;

    RepresentationManagerSList m_childs;

    RepresentationPipeline::Actors m_viewActors; // actors being rendered by its view
  };

  class RepresentationManager2D
  {
  public:
    /** \brief Class RepresentationManager2D virtual destructor.
     *
     */
    virtual ~RepresentationManager2D()
    {};

    virtual void setPlane(Plane plane) = 0;
    virtual void setRepresentationDepth(Nm depth) = 0;
  };

}// namespace ESPINA

#endif // ESPINA_REPRESENTATION_MANAGER_H
