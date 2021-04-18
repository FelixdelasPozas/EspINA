/*

 Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_SUPPORT_CONTEXT_H
#define ESPINA_SUPPORT_CONTEXT_H

#include <Support/EspinaSupport_Export.h>

// ESPINA
#include <GUI/Types.h>
#include <GUI/View/ViewState.h>
#include <GUI/View/Selection.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Types.h>
#include <Support/ROIAccumulator.h>

// Qt
#include <QUndoStack>
#include <QMap>

class QMainWindow;
class QUndoStack;

namespace ESPINA
{
  namespace Support
  {
    /** \class Context
     * \brief Class for grouping several connected classes that are usually passed as parameters.
     */
    class EspinaSupport_EXPORT Context
    {
      public:
        /** \brief Context class constructor.
         * \param[in] mainWindow application main window.
         *
         */
        explicit Context(QMainWindow *mainWindow, bool *minimizedStatus);

        /** \brief Context class destructor.
         *
         */
        ~Context();

        /** \brief Returns the application's task scheduler.
         *
         */
        SchedulerSPtr scheduler() const;

        /** \brief Returns the current model adapter.
         *
         */
        ModelAdapterSPtr model() const;

        /** \brief Returns the current ROI accumulator.
         *
         */
        ROIAccumulatorSPtr roiProvider() const;

        /** \brief Returns the application's view state.
         *
         */
        GUI::View::ViewState &viewState();

        /** \brief Returns the application's multi color engine.
         *
         */
        GUI::ColorEngines::ColorEngineSPtr colorEngine() const;

        /** \brief Returns the application's undo/redo stack.
         *
         */
        QUndoStack *undoStack();

        /** \brief Returns the list of available representations for the view items.
         *
         */
        const RepresentationFactorySList availableRepresentations() const;

        /** \brief Returns the list of current representations for the view items.
         *
         */
        const RepresentationList representations() const;

        /** \brief Adds the factory to the available representation factories and instanciates and returns a representation.
         * \param[in] factory Representation factory.
         *
         */
        const Representation &addRepresentation(const RepresentationFactorySPtr factory);

        /** \brief Returns the instanciated representation for the given factory.
         * \param[in] factory RepresentationFactory
         *
         */
        const Representation representation(const RepresentationFactorySPtr factory) const;

        /** \brief Returns the current model's factory.
         *
         */
        ModelFactorySPtr factory() const;

        /** \brief Adds a new color engine to the multi-color engine.
         * \param[in] engine color engine object.
         *
         */
        void addColorEngine(GUI::ColorEngines::ColorEngineSPtr engine);

        /** \brief Adds a new panel to the application.
         *
         */
        void addPanel(Panel *panel);

        /** \brief Returns true if the application is currently minimized and false otherwise.
         *
         */
        bool isMinimized() const;

      private:
        Context(Context &context) = delete;

        using ViewState   = GUI::View::ViewState;

        ViewState                                       m_viewState;       /** application's view state.                          */
        ModelAdapterSPtr                                m_model;           /** application's model adapter.                       */
        ROIAccumulatorSPtr                              m_activeROI;       /** application's ROI accumulator.                     */
        SchedulerSPtr                                   m_scheduler;       /** application's task scheduler.                      */
        QUndoStack                                      m_undoStack;       /** application's undo/redo stack.                     */
        ModelFactorySPtr                                m_factory;         /** application's model adapter factory.               */
        GUI::ColorEngines::MultiColorEngineSPtr         m_colorEngine;     /** application's multi color engine.                  */
        QMap<RepresentationFactorySPtr, Representation> m_representations; /** available representations <-> representations map. */

        bool                                           *m_minimizedStatus; /** pointer to minimize status boolean value.          */
        QMainWindow                                    *m_mainWindow;      /** pointer to application's main window.              */
    };

    /** \brief Returns the current selection of the application for the given context.
     * \param[in] context application context.
     *
     */
    GUI::View::SelectionSPtr EspinaSupport_EXPORT getSelection(Context &context);

    /** \brief Returns the active channel of the application for the given context.
     * \param[in] context application context.
     *
     */
    ChannelAdapterPtr EspinaSupport_EXPORT getActiveChannel(Context &context);

    /** \brief Returns the list of currently selected segmentations of the application for the given context.
     * \param[in] context application context.
     *
     */
    SegmentationAdapterList EspinaSupport_EXPORT getSelectedSegmentations(Context &context);

    /** \class WithContext
     * \brief Super class for objects with heavy use of context methods.
     *
     */
    class EspinaSupport_EXPORT WithContext
    {
      public:
        /** \brief WithContext class constructor.
         * \param[in] context context object.
         *
         */
        explicit WithContext(Context &context);

        /** \brief Returns the application context.
         *
         */
        Context &getContext() const;

        /** \brief Returns the active channel of the application.
         *
         */
        ChannelAdapterPtr getActiveChannel() const;

        /** \brief Returns the current selection of the application.
         *
         */
        GUI::View::SelectionSPtr getSelection() const;

        /** \brief Returns the list of currently selected segmentations of the application.
         *
         */
        SegmentationAdapterList getSelectedSegmentations() const;

        /** \brief Returns the view state of the application.
         *
         */
        GUI::View::ViewState &getViewState() const;

        /** \brief Returns the model factory of the application.
         *
         */
        ModelFactorySPtr getFactory() const;

        /** \brief Returns the task sheduler of the application.
         *
         */
        SchedulerSPtr getScheduler() const;

        /** \brief Returns the model adapter of the application.
         *
         */
        ModelAdapterSPtr getModel() const;

        /** \brief Returns the undo/redo stack of the application.
         *
         */
        QUndoStack *getUndoStack() const;

      private:
        Context &m_context; /** context object reference. */
    };
  }
}

#endif // ESPINA_SUPPORT_CONTEXT_H
