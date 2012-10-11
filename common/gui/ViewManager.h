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
#include "common/widgets/EspinaWidget.h"

// Qt
#include <QList>
#include <QMap>
#include <QColor>

// VTK
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

class QCursor;
class IPicker;
class QEvent;
class EspinaRenderView;
class Channel;
class Segmentation;
class TaxonomyElement;
class PickableItem;
class ColorEngine;
class IEspinaView;

class ViewManager
: public QObject
{
  Q_OBJECT
public:
  explicit ViewManager();
  ~ViewManager();

  void registerView(IEspinaView *view);
  void registerView(EspinaRenderView *view);

  //---------------------------------------------------------------------------
  /*************************** Selection API *********************************/
  //---------------------------------------------------------------------------
public:
  typedef QList<PickableItem *> Selection;

  void setSelection(Selection selection);
  /// Returns current selection
  Selection selection() const { return m_selection; }
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
  /// Register @picker as active Picker
  void setPicker(IPicker *sh);
  /// Disable @picker as active Picker
  void unsetPicker(IPicker *picker);
  /// Filter event according to responsabilty chain
  bool filterEvent(QEvent *e, EspinaRenderView *view=NULL) const;
  /// Return cursor of active picker
  QCursor cursor() const;

private:
  IPicker *m_picker;
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
  /// Reset Camera
  void resetViewCameras();
  /// Update Segmentation Representation
  void updateSegmentationRepresentations();

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
  QColor color(const Segmentation *seg);
  vtkSmartPointer<vtkLookupTable> lut(const Segmentation *seg);

  void setColorEngine(ColorEngine *engine);

private:
  Nm m_slicingStep[3];

  QList<IEspinaView *>      m_espinaViews;
  QList<EspinaRenderView *> m_renderViews;

  Channel      *m_activeChannel;
  TaxonomyElement *m_activeTaxonomy;

  ColorEngine * m_colorEngine;
  vtkSmartPointer<vtkLookupTable> seg_lut;
};

#endif // VIEWMANAGER_H
