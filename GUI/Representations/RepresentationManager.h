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
    void setView(RenderView *view)
    { m_view = view; }

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

    virtual bool isReady() const = 0;

    /** \brief Returns a new instance of the class.
     *
     */
    RepresentationManagerSPtr clone();

  public slots:
    virtual void onCrosshairChanged(NmVector3 crosshair) = 0;

    /** \brief Set representations visibility
     *
     */
    void setRepresentationsVisibility(bool value);

  signals:
    void renderRequested();

  protected:
    explicit RepresentationManager();

  protected slots:
    /** \brief Update all actors of the managed representations
     *
     */
    void updateRepresentationActors();

  private:
    virtual RepresentationPipelineSList pipelines() = 0;

    virtual void updatePipelines() = 0;

    virtual void notifyPoolUsed() = 0;

    virtual void notifyPoolNotUsed() = 0;

    virtual RepresentationManagerSPtr cloneImplementation() = 0;

  protected:
    QString m_name;
    QIcon   m_icon;
    QString m_description;
    bool    m_showPipelines;

  private:
    RenderView *m_view;

    RepresentationManagerSList m_childs;

    RepresentationPipelineSList m_viewPipelines; // pipeline being rendered by its view
  };

  class RepresentationManager2D
  {
  public:
    virtual void setPlane(Plane plane) = 0;
  };

}// namespace ESPINA

#endif // ESPINA_REPRESENTATION_MANAGER_H
