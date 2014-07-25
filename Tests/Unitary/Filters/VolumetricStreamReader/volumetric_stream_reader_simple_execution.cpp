/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "Filters/VolumetricStreamReader.h"
#include "Tests/Unitary/Testing_Support.h"
#include <Core/Analysis/Data/VolumetricData.h>
#include <itkImageFileWriter.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int volumetric_stream_reader_simple_execution(int argc, char** argv)
{
  bool error = false;

  Filter::Type  type{"VolumetricStreamReader"};
  SchedulerSPtr scheduler;
  VolumetricStreamReader reader(InputSList(), type, scheduler);

  itkVolumeType::SizeType size;
  size.Fill(10);

  auto image = Testing_Support<itkVolumeType>::Create_Test_Image(size, 75);

  using Writer = itk::ImageFileWriter<itkVolumeType>;
  auto writer = Writer::New();

  QFileInfo filename("volumetric.mhd");
  writer->SetFileName(filename.fileName().toStdString());
  writer->SetInput(image);
  writer->Write();

  reader.setFileName(filename);
  reader.update();

  if (reader.numberOfOutputs() != 1) {
    cerr << "Unexpected number of outputs were created by the filter: " << reader.numberOfOutputs() << endl;  
    error = true;
  }

  if (!reader.validOutput(0)) {
    cerr << "Invalid reader output" << endl;
    error = true;
  }

  DefaultVolumetricDataSPtr volume = volumetricData(reader.output(0));

  Bounds imageBounds = equivalentBounds<itkVolumeType>(image, image->GetLargestPossibleRegion());
  if (volume->bounds() != imageBounds) {
    cerr << "Volume Bounds differ from image bounds: " << volume->bounds() << ", " << imageBounds << endl;
    error = true;
  }

  auto readImage = volume->itkImage();

  if (readImage->GetLargestPossibleRegion() != image->GetLargestPossibleRegion()) {
    cerr << "Image regions doesn't match" << endl;
    error = true;
  }

  return error;
}