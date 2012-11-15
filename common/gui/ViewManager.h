/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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


//----------------------------------------------------------------------------
// File:    ViewManager.h
// Purpose: Singleton to register and keep views updated
//----------------------------------------------------------------------------
#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

// EspINA
#include "common/EspinaTypes.h"
#include "common/colorEngines/ColorEngine.h"
#include "common/tools/IVOI.h"
#include "common/widgets/EspinaWidget.h"

// Qt
#include <QList>
#include <QMap>
#include <QColor>

// VTK
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

class SliceSelectorWidget;
class Channel;
class EspinaRenderView;
class IEspinaView;
class IPicker;
class ITool;
class IVOI;
class PickableItem;
class QCursor;
class QEvent;
class Segmentation;
class SliceView;
class TaxonomyElement;

class ViewManager
: public QObject
{
  Q_OBJECT
public:
  explicit ViewManager();
  ~ViewManager();

  void registerView(IEspinaView *view);
  void registerView(EspinaRenderView *view);
  void registerView(SliceView *view);

  void unregisterView(IEspinaView *view);
  void unregisterView(EspinaRenderView *view);
  void unregisterView(SliceView *view);

private:
  QList<IEspinaView *>      m_espinaViews;
  QList<EspinaRenderView *> m_renderViews;
  QList<SliceView *>        m_sliceViews;

  //---------------------------------------------------------------------------
  /*************************** Selection API *********************************/
  //---------------------------------------------------------------------------?
public:
  typedef QList<PickableItem *> Selection;

  /// Enable item selection in render views
  void setSelectionEnabled(bool enable);
  /// Synchronize @selection between all registered views
  void setSelection(Selection selection);
  /// Returns current selection
  Selection selection() const { return m_selection; }
  SegmentationList selectedSegmentations() const;
//   const Nm *selectionCenter() const
//   { return m_selectionCenter; }

signals:
  void selectionChanged(ViewManager::Selection);

private:
  Selection m_selection;

  //---------------------------------------------------------------------------
  /*************************** Picking API *********************************/
  //---------------------------------------------------------------------------
public:
  void setVOI(IVOI *);
  IVOI *voi() {return m_voi;}
  IVOI::Region voiRegion() {return m_voi?m_voi->region():NULL;}
  /// Set @tool as active tool. If other tool is already active,
  /// it will be disactivated
  void setActiveTool(ITool *tool);
  /// Unset any active tool
  void unsetActiveTool();
  /// Unset @tool as active tool
  void unsetActiveTool(ITool *tool);
  /// Filter @view's @event.
  /// Delegate active voi event handling. If the event is not filtered by
  /// active voi, then active tool, if any, filter the event. If it neither
  /// filter the event, the function returns false. Otherwise, returns true.
  bool filterEvent(QEvent *event, EspinaRenderView *view=NULL);
  QCursor cursor() const;

private:
  IVOI   *m_voi;
  ITool  *m_tool;

  //---------------------------------------------------------------------------
  /***************************** Widget API **********************************/
  //---------------------------------------------------------------------------
public:
  void addWidget(EspinaWidget *widget);
  void removeWidget(EspinaWidget *widget);

  //---------------------------------------------------------------------------
  /*********************** View Synchronization API **************************/
  //---------------------------------------------------------------------------
public:
  enum SliceSelector
  {
    None=0x0, From = 0x1, To = 0x2
  };Q_DECLARE_FLAGS(SliceSelectors, SliceSelector)

  /// Reset Camera
  void resetViewCameras();
  /// Focus
  void focusViewsOn(Nm *);
  /// Update Segmentation Representation
  void updateSegmentationRepresentations();
  /// Toggle crosshair
  void showCrosshair(bool);
  /// Set Slice Selection flags to all registered Slice Views
  void addSliceSelectors(SliceSelectorWidget *widget,
                         SliceSelectors selectors);
  /// Unset Slice Selection flags to all registered Slice Views
  void removeSliceSelectors(SliceSelectorWidget *widget);

public slots:
  /// Request all registered views to update themselves
  void updateViews();


  //---------------------------------------------------------------------------
  /*********************** Active Elements API *******************************/
  //---------------------------------------------------------------------------
  // These are specified by the user to be used when one element of the      //
  // proper type is required                                                 //
  //---------------------------------------------------------------------------
public:
  void setActiveChannel(Channel *channel);
  Channel *activeChannel() { return m_activeChannel; }
  void setActiveTaxonomy(TaxonomyElement *taxonomy) { m_activeTaxonomy = taxonomy; }
  TaxonomyElement *activeTaxonomy() { return m_activeTaxonomy; }

signals:
  void activeChannelChanged(Channel *);
  void activeTaxonomyChanged(TaxonomyElement *);

  //---------------------------------------------------------------------------
  /************************* Color Engine API ********************************/
  //---------------------------------------------------------------------------
public:
  QColor color(Segmentation *seg);
  LUTPtr lut(Segmentation *seg);

  void setColorEngine(ColorEngine *engine);

private:
  Nm m_slicingStep[3];

  Channel      *m_activeChannel;
  TaxonomyElement *m_activeTaxonomy;

  ColorEngine *m_colorEngine;
  vtkSmartPointer<vtkLookupTable> seg_lut;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ViewManager::SliceSelectors)

#endif // VIEWMANAGER_H
