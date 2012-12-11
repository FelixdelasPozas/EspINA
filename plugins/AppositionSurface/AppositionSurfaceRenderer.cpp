/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "AppositionSurfaceRenderer.h"

#include "AppositionSurfaceExtension.h"

#include <common/model/Segmentation.h>

#include <itkTimeStamp.h>

#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkDecimatePro.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkPolyDataNormals.h>

#include <QDebug>
#include <QApplication>

typedef vtkSmartPointer<vtkPolyDataMapper> PolyDataMapper;
typedef vtkSmartPointer<vtkDecimatePro> DecimatePro;
typedef vtkSmartPointer<vtkWindowedSincPolyDataFilter> Smoother;
typedef vtkSmartPointer<vtkPolyDataNormals> Normals;
typedef vtkSmartPointer<vtkPolyDataMapper> PolyDataMapper;

struct AppositionSurfaceRenderer::State
{
  vtkActor *actor;
  bool      visible;
};

struct AppositionSurfaceRenderer::Representation
{
  DecimatePro    decimate;
  Smoother       smoother;
  Normals        normals;
  PolyDataMapper mapper;
  itk::TimeStamp timeStamp;
};

QMap<ModelItem *, AppositionSurfaceRenderer::Representation *> AppositionSurfaceRenderer::m_representations;

//-----------------------------------------------------------------------------
bool AppositionSurfaceRenderer::addItem(ModelItem* item)
{
  if (ModelItem::SEGMENTATION != item->type())
    return false;

  Q_ASSERT(!m_state.contains(item));

  Representation *rep;
  if (!m_representations.contains(item))
  {
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    ModelItemExtension       *mie = seg->extension(AppositionSurfaceExtension::ID);
    AppositionSurfaceExtension *ape = dynamic_cast<AppositionSurfaceExtension*>(mie);

    rep = new Representation();
    rep->decimate = DecimatePro::New();
    rep->decimate->ReleaseDataFlagOn();
    rep->decimate->SetGlobalWarningDisplay(false);
    rep->decimate->SetTargetReduction(0.95);
    rep->decimate->PreserveTopologyOn();
    rep->decimate->SplittingOff();
    rep->decimate->SetInputConnection(ape->appositionSurface()->GetProducerPort());

    rep->smoother = Smoother::New();
    rep->smoother->ReleaseDataFlagOn();
    rep->smoother->SetGlobalWarningDisplay(false);
    rep->smoother->BoundarySmoothingOn();
    rep->smoother->FeatureEdgeSmoothingOn();
    rep->smoother->SetNumberOfIterations(15);
    rep->smoother->SetFeatureAngle(120);
    rep->smoother->SetEdgeAngle(90);
    rep->smoother->SetInputConnection(rep->decimate->GetOutputPort());

    rep->normals = Normals::New();
    rep->normals->ReleaseDataFlagOn();
    rep->normals->SetFeatureAngle(120);
    rep->normals->SetInputConnection(rep->smoother->GetOutputPort());

    rep->mapper = PolyDataMapper::New();
    rep->mapper->ReleaseDataFlagOn();
    rep->mapper->ImmediateModeRenderingOn();
    rep->mapper->ScalarVisibilityOff();
    rep->mapper->SetInputConnection(rep->normals->GetOutputPort());
    PolyDataMapper mapper = PolyDataMapper::New();
    mapper->ReleaseDataFlagOn();
    mapper->ImmediateModeRenderingOn();
    mapper->ScalarVisibilityOn();
    mapper->SetInputConnection(rep->normals->GetOutputPort());

    m_representations[item] = rep;
  } else
    rep = m_representations[item];

  State *state = new State();
  state->actor = vtkActor::New();
  state->actor->SetMapper(rep->mapper);
  state->actor->GetProperty()->SetColor(1.0,1.0,0.0);
  state->actor->GetProperty()->SetOpacity(1.0);

  state->visible = false;

  m_state[item] = state;

  return true;
}

//-----------------------------------------------------------------------------
bool AppositionSurfaceRenderer::updateItem(ModelItem* item)
{
  if (ModelItem::SEGMENTATION != item->type())
    return false;

  if (!m_representations.contains(item))
    return false;

  Q_ASSERT(m_state.contains(item));

  bool updated = false;
  Segmentation *seg = dynamic_cast<Segmentation *>(item);
  State        *state = m_state[item];

  if (m_enable && seg->visible())
  {
    if (!state->visible)
    {
      Representation *rep = m_representations[item];
      if (rep->timeStamp != seg->itkVolume()->GetTimeStamp())
      {
        ModelItemExtension       *mie = seg->extension(AppositionSurfaceExtension::ID);
        AppositionSurfaceExtension *ape = dynamic_cast<AppositionSurfaceExtension*>(mie);
        ape->updateAppositionSurface();

        rep->timeStamp = seg->itkVolume()->GetTimeStamp();
      }
      m_renderer->AddActor(state->actor);
      state->visible = true;
      updated = true;
    }
  }
  else if (state->visible)
  {
    m_renderer->RemoveActor(state->actor);
    state->visible = false;
    updated = true;
  }

  return updated;
}

//-----------------------------------------------------------------------------
bool AppositionSurfaceRenderer::removeItem(ModelItem* item)
{
  if (ModelItem::SEGMENTATION != item->type())
    return false;

  Q_ASSERT(m_representations.contains(item));
  Q_ASSERT(m_state.contains(item));

  State *state = m_state[item];
  if (state->visible)
    m_renderer->RemoveActor(state->actor);

  state->actor->Delete();
  delete state;
  m_state.remove(item);

  if (m_representations.contains(item))
    m_representations.remove(item);

  return true;
}

//-----------------------------------------------------------------------------
void AppositionSurfaceRenderer::hide()
{
  if (!this->m_enable)
    return;
  
  m_enable = false;
  foreach(State *state, m_state)
  {
    if (state->visible)
    {
      m_renderer->RemoveActor(state->actor);
      state->visible = false;
    }
  }
  
  emit renderRequested();
}

//-----------------------------------------------------------------------------
void AppositionSurfaceRenderer::show()
{
  if (this->m_enable)
    return;
  
  m_enable = true;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  
  foreach(ModelItem *item, m_state.keys())
  {
    State *state = m_state[item];
    if (!state->visible)
    {
      Representation *rep = m_representations[item];
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      if (rep->timeStamp != seg->itkVolume()->GetTimeStamp())
      {
	ModelItemExtension       *mie = item->extension(AppositionSurfaceExtension::ID);
	AppositionSurfaceExtension *ape = dynamic_cast<AppositionSurfaceExtension*>(mie);
	ape->updateAppositionSurface();
	
	rep->timeStamp = seg->itkVolume()->GetTimeStamp();
      }
      m_renderer->AddActor(state->actor);
      state->visible = true;
    }
  }
    
  emit renderRequested();
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
Renderer* AppositionSurfaceRenderer::clone()
{
  return new AppositionSurfaceRenderer();
}

