/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Pe�a Pastor <jpena@cesvima.upm.es>

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

#include "TubularSegmentationFilter.h"

// EspINA
#include <Core/VTK/vtkTube.h>
#include <App/Tools/TubularSegmentation/TubularTool.h>
#include <Core/EspinaVolume.h>

// VTK
#include <vtkMath.h>
#include <vtkSphere.h>
#include <vtkTransform.h>

// Qt
#include <QDebug>
#include <QVector4D>

namespace EspINA
{
  const QString TubularSegmentationFilter::FILTER_TYPE = "TubularSegmentation::TubularSegmentationFilter";

  typedef ModelItem::ArgumentId ArgumentId;
  const ArgumentId TubularSegmentationFilter::SPACING = "Spacing";
  const ArgumentId TubularSegmentationFilter::NODES = "Nodes";

  //-----------------------------------------------------------------------------
  TubularSegmentationFilter::TubularSegmentationFilter(Filter::NamedInputs inputs, ModelItem::Arguments args, Filter::FilterType type)
  : SegmentationFilter(inputs, args, type)
  , m_param(m_args)
  , RoundedExtremes(false)
  , LazyExecution(true)
  , m_filterInspector(NULL)
  , m_tool(NULL)
  {
    Q_ASSERT(inputs.isEmpty());
  }

  //-----------------------------------------------------------------------------
  TubularSegmentationFilter::~TubularSegmentationFilter()
  {
  }

  //-----------------------------------------------------------------------------
  void TubularSegmentationFilter::setNodes(NodeList nodes)
  {
    m_param.setNodeList(nodes);

    if (!LazyExecution)
      updateVolume();
  }

  //-----------------------------------------------------------------------------
  void TubularSegmentationFilter::updateVolume()
  {
    foreach(vtkImplicitFunction *f, m_implicitFunctions)
    {
      f->Delete();
    }
    m_implicitFunctions.clear();

    if (m_param.nodeList().isEmpty())
      return;
    //   qDebug() << "Update Image Extent";
    bool init = false;
    foreach(QVector4D node, m_param.nodeList())
    {
      int cx = vtkMath::Round(node.x() / m_param.spacing()[0]);
      int cy = vtkMath::Round(node.y() / m_param.spacing()[1]);
      int cz = vtkMath::Round(node.z() / m_param.spacing()[2]);
      int rx = round(node.w()/m_param.spacing()[0]);
      int ry = round(node.w()/m_param.spacing()[1]);
      int rz = round(node.w()/m_param.spacing()[2]);

      if (!init)
      {
        Extent[0] = Extent[1] = cx;
        Extent[2] = Extent[3] = cy;
        Extent[4] = Extent[5] = cz;
        init = true;
      }

      Extent[0] = std::min(Extent[0], cx - rx);
      Extent[1] = std::max(Extent[1], cx + rx);
      Extent[2] = std::min(Extent[2], cy - ry);
      Extent[3] = std::max(Extent[3], cy + ry);
      Extent[4] = std::min(Extent[4], cz - rz);
      Extent[5] = std::max(Extent[5], cz + rz);
    }

    //   qDebug() << "Updating Implicit Functions";
    double node[4];
    double prevNode[4];
    for (int i=0; i < m_param.nodeList().size(); i++)
    {
      node[0] = m_param.nodeList()[i].x() - Extent[0]*m_param.spacing()[0];
      node[1] = m_param.nodeList()[i].y() - Extent[2]*m_param.spacing()[1];
      node[2] = m_param.nodeList()[i].z() - Extent[4]*m_param.spacing()[2];
      node[3] = m_param.nodeList()[i].w();

      // if !RoundedEdges spheres 0 and n-1 souldn't be into the list of implicit functions
      if (!(!RoundedExtremes && ((0 == i) || (m_param.nodeList().size()-1 == i))))
      {
        vtkSphere *sphere = vtkSphere::New();
        sphere->SetCenter(node);
        sphere->SetRadius(node[3]);
        m_implicitFunctions << sphere;
      }

      if (i > 0)
      {
        vtkTube *link = vtkTube::New();
        link->SetBaseCenter(prevNode);
        link->SetBaseRadius(prevNode[3]);
        link->SetTopCenter(node);
        link->SetTopRadius(node[3]);
        m_implicitFunctions << link;
      }
      memcpy(prevNode, node, 4*sizeof(double));
    }

    itkVolumeType::SizeType size;
    size[0] = Extent[1]-Extent[0];
    size[1] = Extent[3]-Extent[2];
    size[2] = Extent[5]-Extent[4];

    ImplicitSource::PointType origin;
    origin[0] = Extent[0]*m_param.spacing()[0];
    origin[1] = Extent[2]*m_param.spacing()[1];
    origin[2] = Extent[4]*m_param.spacing()[2];

    m_filter = ImplicitSource::New();
    m_filter->SetOrigin(origin);
    m_filter->SetSize(size);
    m_filter->SetSpacing(m_param.spacing());
    m_filter->SetImplicitFunctions(m_implicitFunctions);
    m_filter->Update();

    createOutput(0, m_filter->GetOutput());

    emit modified(this);
  }

  //-----------------------------------------------------------------------------
  QVariant TubularSegmentationFilter::data(int role) const
  {
    if (role == Qt::DisplayRole)
      return FILTER_TYPE;
    else
      return QVariant();
  }

  //-----------------------------------------------------------------------------
  bool TubularSegmentationFilter::needUpdate() const
  {
    return m_outputs[0].isValid();
  }

  //-----------------------------------------------------------------------------
  bool TubularSegmentationFilter::fetchSnapshot()
  {
    return Filter::fetchSnapshot();
  }

  //-----------------------------------------------------------------------------
  void TubularSegmentationFilter::setRoundedExtremes(bool value)
  {
    if (value == RoundedExtremes)
      return;

    RoundedExtremes = value;

    // (!m_tool->isInUse) to force recomputing the volume if it's lazy but
    // there's no widget (round extremes requested in the filter inspector
    // flag should update instantly)
    if (!LazyExecution || !m_tool->isInUse())
      updateVolume();
  }

  //-----------------------------------------------------------------------------
  bool TubularSegmentationFilter::getRoundedExtremes()
  {
    return RoundedExtremes;
  }

  //-----------------------------------------------------------------------------
  void TubularSegmentationFilter::setTool(TubularTool *tool)
  {
    qDebug() << "set tool" << tool;
    m_tool = tool;
  }

  //-----------------------------------------------------------------------------
  void TubularSegmentationFilter::setLazyExecution(bool value)
  {
    LazyExecution = value;
  }

  //-----------------------------------------------------------------------------
  bool TubularSegmentationFilter::getLazyExecution()
  {
    return LazyExecution;
  }

  //-----------------------------------------------------------------------------
  void TubularSegmentationFilter::executeFilter()
  {
    if (LazyExecution)
      updateVolume();
  }

  //-----------------------------------------------------------------------------
  TubularSegmentationFilter::Parameters::Parameters(ModelItem::Arguments& args)
      : m_args(args)
  {
    QStringList values = m_args[SPACING].split(",", QString::SkipEmptyParts);
    if (values.size() == 3)
    {
      for (int i = 0; i < 3; i++)
        m_spacing[i] = values[i].toDouble();
    }
    else
    {
      m_spacing[0] = m_spacing[1] = m_spacing[2] = 1.0;
    }
    values = m_args[NODES].split(",", QString::SkipEmptyParts);
    for (int i = 0; i < values.size(); i++)
    {
      QStringList p = values[i].split(" ");
      QVector4D node(p[0].toDouble(), p[1].toDouble(), p[2].toDouble(), p[3].toDouble());
      m_nodes << node;
    }
  }

  //-----------------------------------------------------------------------------
  void TubularSegmentationFilter::Parameters::setSpacing(double value[3])
  {
    for (int i = 0; i < 3; i++)
      m_spacing[i] = value[i];
    m_args[SPACING] = QString("%1,%2,%3").arg(value[0]).arg(value[1]).arg(value[2]);
  }

  //-----------------------------------------------------------------------------
  void TubularSegmentationFilter::Parameters::setNodeList(TubularSegmentationFilter::NodeList nodes)
  {
    if (m_nodes == nodes)
      return;

    m_nodes = nodes;
    QStringList nodeList;
    foreach(QVector4D node, m_nodes)
    {
      nodeList << QString("%1 %2 %3 %4").arg(node.x())
      .arg(node.y())
      .arg(node.z())
      .arg(node.w());
    }

    m_args[NODES] = nodeList.join(",");
    //   qDebug() << m_args[NODES];
  }

}
