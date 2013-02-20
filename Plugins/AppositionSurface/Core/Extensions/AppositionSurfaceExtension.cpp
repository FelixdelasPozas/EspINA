/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
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

#include "AppositionSurfaceExtension.h"
#include <Filter/AppositionSurfaceFilter.h>

// EspINA
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>
#include <Core/EspinaSettings.h>

// boost
#include <boost/concept_check.hpp>

using namespace EspINA;

///-----------------------------------------------------------------------
/// APPOSITION SURFACE EXTENSION-
///-----------------------------------------------------------------------
/// Information Provided:
/// - Apposition Surface Area
/// - Apposition Surface Perimeter
/// - Apposition Surface Tortuosity
/// - Synapse from which the Apposition Surface was obtained

const ModelItem::ExtId   AppositionSurfaceExtension::ID = "AppositionSurfaceExtension";

const Segmentation::InfoTag AppositionSurfaceExtension::AREA       = "AS Area";
const Segmentation::InfoTag AppositionSurfaceExtension::PERIMETER  = "AS Perimeter";
const Segmentation::InfoTag AppositionSurfaceExtension::TORTUOSITY = "AS Tortuosity";
const Segmentation::InfoTag AppositionSurfaceExtension::SYNAPSE    = "AS Synapse";

const QString AppositionSurfaceExtension::EXTENSION_FILE = "AppositionSurfaceExtension/AppositionSurfaceExtension.csv";

const std::string FILE_VERSION = AppositionSurfaceExtension::ID.toStdString() + " 1.0\n";
const char SEP = ',';

QMap<SegmentationPtr, AppositionSurfaceExtension::CacheEntry> AppositionSurfaceExtension::s_cache;

const double UNDEFINED = -1.;

//------------------------------------------------------------------------
AppositionSurfaceExtension::CacheEntry::CacheEntry()
: Modified(false)
, Area(UNDEFINED)
, Perimeter(UNDEFINED)
, Tortuosity(UNDEFINED)
, FromSynapse(QString())
{
};

//------------------------------------------------------------------------
AppositionSurfaceExtension::AppositionSurfaceExtension()
{
}

//------------------------------------------------------------------------
AppositionSurfaceExtension::~AppositionSurfaceExtension()
{
  invalidate();
}

//------------------------------------------------------------------------
ModelItem::ExtId AppositionSurfaceExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
Segmentation::InfoTagList AppositionSurfaceExtension::availableInformations() const
{
  Segmentation::InfoTagList tags;

  tags << AREA << PERIMETER << TORTUOSITY << SYNAPSE;

  return tags;
}

//------------------------------------------------------------------------
QVariant AppositionSurfaceExtension::information(const Segmentation::InfoTag &tag)
{
  QString fullTaxonomy = m_seg->taxonomy()->qualifiedName();
  if (!fullTaxonomy.startsWith("AS/") && (fullTaxonomy.compare("AS") != 0))
    return QVariant();

  if (!s_cache.contains(m_seg))
  {
    AppositionSurfaceFilter *filter = dynamic_cast<AppositionSurfaceFilter *>(m_seg->filter().data());
    s_cache[m_seg].Area = filter->getArea();
    s_cache[m_seg].Perimeter = filter->getPerimeter();
    s_cache[m_seg].Tortuosity = filter->getTortuosity();
    s_cache[m_seg].FromSynapse = filter->getOriginSegmentation();
  }

  if (AREA == tag)
    return s_cache[m_seg].Area;
  if (PERIMETER == tag)
    return s_cache[m_seg].Perimeter;
  if (TORTUOSITY == tag)
    return s_cache[m_seg].Tortuosity;
  if (SYNAPSE == tag)
    return s_cache[m_seg].FromSynapse;

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::initialize(ModelItem::Arguments args)
{
}

//------------------------------------------------------------------------
Segmentation::InformationExtension AppositionSurfaceExtension::clone()
{
  return new AppositionSurfaceExtension();
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::isCacheFile(const QString &file) const
{
  return EXTENSION_FILE == file;
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::invalidate()
{
  s_cache.remove(m_seg);
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::loadCache(QuaZipFile &file, const QDir &tmpDir, EspinaModel *model)
{
  QString header(file.readLine());
  if (header.toStdString() != FILE_VERSION)
    return false;

  char buffer[1024];
  while (file.readLine(buffer, sizeof(buffer)) > 0)
  {
    QString line(buffer);
    QStringList fields = line.split(SEP);

    Q_ASSERT(fields.size() == 6);

    SegmentationPtr extensionSegmentation = NULL;
    int i = 0;
    while (!extensionSegmentation && i < model->segmentations().size())
    {
      SegmentationSPtr segmentation = model->segmentations()[i];
      if ( segmentation->filter()->id()       == fields[0]
        && segmentation->outputId()           == fields[1].toInt()
        && segmentation->filter()->cacheDir() == tmpDir)
      {
        extensionSegmentation = segmentation.data();
      }
      i++;
    }
    // NOTE: This assert means someone's removing an extension from the model
    //       without invalidating its extensions
    Q_ASSERT(extensionSegmentation);

    s_cache[extensionSegmentation].Area        = fields[2].toDouble();
    s_cache[extensionSegmentation].Perimeter   = fields[3].toDouble();
    s_cache[extensionSegmentation].Tortuosity  = fields[4].toDouble();
    s_cache[extensionSegmentation].FromSynapse = fields[5].remove('\n');
  }

  return true;
}

//------------------------------------------------------------------------
bool AppositionSurfaceExtension::saveCache(Snapshot &cacheList)
{
  // TODO: save disabled
  return false;

  foreach(SegmentationPtr segmentation, s_cache.keys())
  {
    if (segmentation->isVolumeModified() && !s_cache[segmentation].Modified)
      s_cache.remove(segmentation);
  }

  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  foreach(SegmentationPtr segmentation, s_cache.keys())
  {
    cache << segmentation->filter()->id().toStdString();
    cache << SEP << segmentation->outputId();

    cache << SEP << s_cache[segmentation].Area;
    cache << SEP << s_cache[segmentation].Perimeter;
    cache << SEP << s_cache[segmentation].Tortuosity;
    cache << SEP << s_cache[segmentation].FromSynapse.toStdString();

    cache << std::endl;
  }

  cacheList << QPair<QString, QByteArray>(EXTENSION_FILE, cache.str().c_str());

  return true;
}

