/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "VolumetricData.h"

// #include <itkMetaImageIO.h>
// #include <itkImageFileWriter.h>
// #include <QDir>

using namespace EspINA;


const Data::Type VOLUMETRIC_TYPE = "VolumetricData";

// //----------------------------------------------------------------------------


// typedef itk::ImageFileWriter<itkVolumeType> EspinaVolumeWriter;
// 
// //----------------------------------------------------------------------------
// const FilterOutput::OutputRepresentationName ChannelVolume::TYPE = "ChannelVolume";
// 
// //----------------------------------------------------------------------------
// EspinaRegion ChannelVolume::representationBounds()
// {
//   return espinaRegion();
// }
// 
// //----------------------------------------------------------------------------
// ChannelVolumePtr EspINA::channelVolume(OutputPtr output)
// {
//   ChannelOutputPtr channelOutput = dynamic_cast<ChannelOutputPtr>(output);
//   Q_ASSERT(channelOutput);
//   return dynamic_cast<ChannelVolume *>(channelOutput->representation(ChannelVolume::TYPE).get());
// }
// 
// //----------------------------------------------------------------------------
// ChannelVolumeSPtr EspINA::channelVolume(OutputSPtr output)
// {
//   ChannelOutputSPtr channelOutput = boost::dynamic_pointer_cast<ChannelOutput>(output);
//   Q_ASSERT(channelOutput.get());
//   return boost::dynamic_pointer_cast<ChannelVolume>(channelOutput->representation(ChannelVolume::TYPE));
// }
// 
// 
// //----------------------------------------------------------------------------
// const FilterOutput::OutputRepresentationName SegmentationVolume::TYPE = "SegmentationVolume";
// 
// //----------------------------------------------------------------------------
// EspinaRegion SegmentationVolume::representationBounds()
// {
//   return espinaRegion();
// }
// 
// //----------------------------------------------------------------------------
// bool SegmentationVolume::EditedVolumeRegion::dump(QDir           cacheDir,
//                                                   const QString &regionName,
//                                                   Snapshot      &snapshot) const
// {
//   bool dumped = false;
// 
//   itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
//   EspinaVolumeWriter::Pointer writer = EspinaVolumeWriter::New();
// 
//   QDir temporalDir = QDir::tempPath();
// 
//   QString mhd = temporalDir.absoluteFilePath(regionName + ".mhd");
//   QString raw = temporalDir.absoluteFilePath(regionName + ".raw");
// 
//   if (Volume)
//   {
//     bool releaseFlag = Volume->GetReleaseDataFlag();
//     Volume->ReleaseDataFlagOff();
// 
//     io->SetFileName(mhd.toUtf8());
//     writer->SetFileName(mhd.toUtf8().data());
// 
//     writer->SetInput(Volume);
//     writer->SetImageIO(io);
//     writer->Write();
// 
//     Volume->SetReleaseDataFlag(releaseFlag);
//   } else
//   {
//     if (cacheDir.exists(SegmentationVolume::TYPE))
//       cacheDir.cd(SegmentationVolume::TYPE);
// 
//     // dump cached volumes
//     mhd = cacheDir.absoluteFilePath(regionName + ".mhd");
//     raw = cacheDir.absoluteFilePath(regionName + ".raw");
//   }
// 
//   QFileInfo mhdFileInfo(mhd);
//   QFileInfo rawFileInfo(raw);
//   if (mhdFileInfo.exists() && rawFileInfo.exists())
//   {
//     QFile mhdFile(mhd);
//     mhdFile.open(QIODevice::ReadOnly);
//     QFile rawFile(raw);
//     rawFile.open(QIODevice::ReadOnly);
// 
//     QByteArray mhdArray(mhdFile.readAll());
//     QByteArray rawArray(rawFile.readAll());
// 
//     mhdFile.close();
//     rawFile.close();
// 
//     SnapshotEntry mhdEntry(cachePath(regionName + ".mhd"), mhdArray);
//     SnapshotEntry rawEntry(cachePath(regionName + ".raw"), rawArray);
// 
//     snapshot << mhdEntry << rawEntry;
// 
//     dumped = true;
//   }
// 
//   if (Volume)
//   {
//     temporalDir.remove(mhd);
//     temporalDir.remove(raw);
//   }
// 
//   return dumped;
// }
// 
// //----------------------------------------------------------------------------
// SegmentationVolumePtr EspINA::segmentationVolume(OutputPtr output)
// {
//   SegmentationOutputPtr segmentationOutput = dynamic_cast<SegmentationOutputPtr>(output);
//   Q_ASSERT(segmentationOutput);
//   return dynamic_cast<SegmentationVolume *>(segmentationOutput->representation(SegmentationVolume::TYPE).get());
// }
// 
// //----------------------------------------------------------------------------
// SegmentationVolumeSPtr EspINA::segmentationVolume(OutputSPtr output)
// {
//   SegmentationOutputSPtr segmentationOutput = boost::dynamic_pointer_cast<SegmentationOutput>(output);
//   Q_ASSERT(segmentationOutput.get());
//   return boost::dynamic_pointer_cast<SegmentationVolume>(segmentationOutput->representation(SegmentationVolume::TYPE));
// }
// 
// //----------------------------------------------------------------------------
// SegmentationVolumeSPtr EspINA::segmentationVolume(SegmentationOutputSPtr output)
// {
//   return boost::dynamic_pointer_cast<SegmentationVolume>(output->representation(SegmentationVolume::TYPE));
// }
// 