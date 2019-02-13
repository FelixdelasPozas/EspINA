/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_DEFAULT_VIEW_H
#define ESPINA_DEFAULT_VIEW_H

// ESPINA
#include <Core/Types.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <Support/Settings/SettingsPanel.h>
#include <Support/Representations/RepresentationFactory.h>
#include <Support/Context.h>

// Qt
#include <QAbstractItemView>

class QMainWindow;
class QDockWidget;
class QUndoStack;

namespace ESPINA
{
  class Dialog3D;

  class DefaultView
  : public QWidget
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief DefaultView class constructor.
       * \param[in] context application context
       * \param[in] parent pointer of the object parent of this one.
       */
      explicit DefaultView(Support::Context &context,
                           QMainWindow      *parent = nullptr);

      /** \brief DefaultView class virtual destructor.
       *
       */
      virtual ~DefaultView();

      void addRepresentation(const Representation &representation);

      /** \brief Returns a reference to the XZ panel
       *
       */
      Panel *panelXZ();

      /** \brief Returns a reference to the YZ panel
       *
       */
      Panel *panelYZ();

      /** \brief Returns a reference to the 3D dialog.
       *
       */
      Dialog3D *dialog3D();

      /** \brief Returs the RenderViews' group.
       *
       */
      QList<RenderView *> renderviews() const;

      /* Used by other classes */
      static const QString FIT_TO_SLICES_KEY;

    public slots:
      /** \brief Shows/hides the view's ruler.
       * \param[in] visible true to show the ruler, false to hide.
       */
      void setRulerVisibility(bool visible);

      /** \brief Shows/hides the thumbnail in 2D views.
       * \param[in] visible true to show the thumbnail, false to hide.
       *
       */
      void showThumbnail(bool visible);

      /** \brief Enables/disables fit to slices.
       * \param[in] enabled true to enable and false otherwise.
       *
       */
      void setFitToSlices(bool enabled);

    private slots:
      void onColorEngineModified();

    private:
      void initView(RenderView *view, QMainWindow *parent);

      /** \brief Creates keyboard shortcuts for slice navigation for the given view.
       * \param[in] view planar view pointer.
       *
       */
      void createView2DShortcuts(View2D* view);

      void initDialog3D(Dialog3D *dialog, QMainWindow *parent);

      void addRepresentationManager(GUI::Representations::RepresentationManagerSPtr manager);

    private:
      ModelSources                                     m_sources;         /** items sources list.                          */
      RepresentationPoolSList                          m_pools;           /** list of pools in the view.                   */
      GUI::Representations::RepresentationManagerSList m_repManagers;     /** list of representation managers in the view. */
      View2D                                          *m_viewXY;          /** XY view.                                     */
      View2D                                          *m_viewYZ;          /** YZ view.                                     */
      View2D                                          *m_viewXZ;          /** XZ view.                                     */
      Panel                                           *m_panelYZ;         /** panel of YZ view.                            */
      Panel                                           *m_panelXZ;         /** panel of XZ view.                            */
      Dialog3D                                        *m_dialog3D;        /** pointer to 3D view dialog.                   */

      QList<RenderView *>                              m_views;           /** list of views.                               */
  };

  using DefaultViewSPtr = std::shared_ptr<DefaultView>;

} // namespace ESPINA

#endif // ESPINA_DEFAULT_VIEW_H
