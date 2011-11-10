/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef VESICLEVALIDATOR_H
#define VESICLEVALIDATOR_H

#include <EspinaPlugin.h>
#include <iSegmentationPlugin.h>

#include <selectionManager.h>

class VesicleValidatorSettings;
class VesicleValidatorFilter;

class VesicleValidator 
  : public ISegmentationPlugin
  , public IFilterFactory
{
  Q_OBJECT
  
  struct Region
  {
    IVOI *sva;
    TaxonomyNode *taxonomy;
  };
  
  enum STATE {NONE, DEFINING_SVA, REMOVING_SVA, VALIDATING};
  
public:
  explicit VesicleValidator(QObject *parent);
  virtual ~VesicleValidator();
  
  virtual EspinaFilter* createFilter(QString filter, ITraceNode::Arguments& args);
  
private slots:
  void defineSVAClicked(bool active);// define Synapse's Vesicle Area
  void removeSVAClicked(bool active);// remove Synapse's Vesicle Area
  void validateVesiclesClicked(bool active);
  
  void handleSelection(const ISelectionHandler::MultiSelection &sel);
  void abortSelection();
  void changeVOI(IVOI *voi);
  
  void updateBounds();
  
private:
  void buildUI();
  
  Region region(const QString &name) const;
  void updateRegions();
  void initSelector();
  void changeState(const STATE state);
  
  Region searchSVG(const Point &pos, double spacing[3]);
  void   defineSVA(const Point& pos, EspinaProduct* sample);
  void   removeSVA(const Point& pos);
  void validateVesicle(const ISelectionHandler::Selelection &selection);
  
  void showSVAs();
  void hideSVAs();
  void showActiveSVA();
  
private:
  QToolButton *m_defineArea;
  QToolButton *m_removeArea;
  QToolButton *m_validateVesicles;
  
  STATE m_state;
  ISelectionHandler *m_selector;
  
  Region m_SVA;
  QList<VesicleValidatorFilter *> m_validators;
  VesicleValidatorFilter *m_activeValidator;
  VesicleValidatorSettings *m_settings;
  
  QVector<Region> m_regions;
};

#endif// VESICLEVALIDATOR_H
