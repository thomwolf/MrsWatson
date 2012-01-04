//
//  SampleSourcePcmFile.c
//  MrsWatson
//
//  Created by Nik Reiman on 1/2/12.
//  Copyright (c) 2012 Teragon Audio. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SampleSourcePcmFile.h"
#import "EventLogger.h"

static boolean _openSampleSourcePcmFile(void* sampleSourcePtr, const SampleSourceOpenAs openAs) {
  SampleSource sampleSource = sampleSourcePtr;

  SampleSourcePcmFileData extraData = sampleSource->extraData;
  extraData->dataBufferNumItems = 0;
  if(openAs == SAMPLE_SOURCE_OPEN_READ) {
    extraData->fileHandle = fopen(sampleSource->sourceName->data, "rb");
  }
  else if(openAs == SAMPLE_SOURCE_OPEN_WRITE) {
    extraData->fileHandle = fopen(sampleSource->sourceName->data, "wb");
  }
  else {
    logInternalError("Invalid type for openAs in PCM file");
    return false;
  }

  if(extraData == NULL) {
    logError("PCM File '%s' could not be opened for %s",
      sampleSource->sourceName->data, openAs == SAMPLE_SOURCE_OPEN_READ ? "reading" : "writing");
    return false;
  }

  sampleSource->openedAs = openAs;
  return true;
}

static void _convertPcmDataToSampleBuffer(const short* inPcmSamples, SampleBuffer sampleBuffer, const long numInterlacedSamples) {
  for(long interlacedIndex = 0, deinterlacedIndex = 0; interlacedIndex < numInterlacedSamples; interlacedIndex++) {
    for(int channelIndex = 0; channelIndex < sampleBuffer->numChannels; channelIndex++) {
      Sample convertedSample = (Sample)inPcmSamples[interlacedIndex] / 32768.0f;
      // Apply brickwall limiter to prevent clipping
      if(convertedSample > 1.0f) {
        convertedSample = 1.0f;
      }
      else if(convertedSample < -1.0f) {
        convertedSample = -1.0f;
      }
      sampleBuffer->samples[channelIndex][deinterlacedIndex] = convertedSample;
    }
    deinterlacedIndex++;
  }
}

static boolean _readBlockFromPcmFile(void* sampleSourcePtr, SampleBuffer sampleBuffer) {
  SampleSource sampleSource = sampleSourcePtr;
  SampleSourcePcmFileData extraData = sampleSource->extraData;
  if(extraData->dataBufferNumItems == 0) {
    extraData->dataBufferNumItems = (size_t)(sampleBuffer->numChannels * sampleBuffer->blocksize);
    extraData->interlacedPcmDataBuffer = malloc(sizeof(short) * extraData->dataBufferNumItems);
  }

  // Clear the PCM data buffer, or else the last block will have dirty samples in the end
  memset(extraData->interlacedPcmDataBuffer, 0, sizeof(short) * extraData->dataBufferNumItems);

  boolean result = true;
  size_t bytesRead = fread(extraData->interlacedPcmDataBuffer, sizeof(short), extraData->dataBufferNumItems, extraData->fileHandle);
  if(bytesRead < extraData->dataBufferNumItems * sizeof(short)) {
    // TODO: End of file reached -- do something special?
    result = false;
  }

  _convertPcmDataToSampleBuffer(extraData->interlacedPcmDataBuffer, sampleBuffer, extraData->dataBufferNumItems);
  return result;
}

static boolean _writeBlockFromPcmFile(void* sampleSourcePtr, const SampleBuffer sampleBuffer) {
  return false;
}

static void _freeInputSourceDataPcmFile(void* sampleSourceDataPtr) {
  SampleSourcePcmFileData extraData = sampleSourceDataPtr;
  free(extraData->interlacedPcmDataBuffer);
  if(extraData->fileHandle != NULL) {
    fclose(extraData->fileHandle);
  }
  free(extraData);
}

SampleSource newSampleSourcePcmFile(const CharString sampleSourceName) {
  SampleSource sampleSource = malloc(sizeof(SampleSourceMembers));

  sampleSource->sampleSourceType = SAMPLE_SOURCE_TYPE_PCM_FILE;
  sampleSource->openedAs = SAMPLE_SOURCE_OPEN_NOT_OPENED;
  sampleSource->sourceName = newCharString();
  copyCharStrings(sampleSource->sourceName, sampleSourceName);
  // TODO: Need a way to pass in channels, bitrate, sample rate
  sampleSource->numChannels = 2;
  sampleSource->sampleRate = 44100.0f;

  sampleSource->openSampleSource = _openSampleSourcePcmFile;
  sampleSource->readSampleBlock = _readBlockFromPcmFile;
  sampleSource->writeSampleBlock = _writeBlockFromPcmFile;
  sampleSource->freeSampleSourceData = _freeInputSourceDataPcmFile;

  SampleSourcePcmFileData extraData = malloc(sizeof(SampleSourcePcmFileDataMembers));
  extraData->fileHandle = NULL;
  extraData->dataBufferNumItems = 0;
  extraData->interlacedPcmDataBuffer = NULL;
  sampleSource->extraData = extraData;

  return sampleSource;
}