#include "AudioConvert.h"

u32 Audio_ConvertBigEndianFloat80(AiffInfo* aiffInfo) {
	f80 float80 = 0;
	u8* pointer = (u8*)&float80;
	
	for (s32 i = 0; i < 10; i++) {
		pointer[i] = (u8)aiffInfo->sampleRate[9 - i];
	}
	
	return (s32)float80;
}
u32 Audio_ByteSwap_FromHighLow(u16* h, u16* l) {
	Lib_ByteSwap(h, SWAP_U16);
	Lib_ByteSwap(l, SWAP_U16);
	
	return (*h << 16) | *l;
}
void Audio_ByteSwap_ToHighLow(u32* source, u16* h) {
	h[0] = (source[0] & 0xFFFF0000) >> 16;
	h[1] = source[0] & 0x0000FFFF;
	
	Lib_ByteSwap(h, SWAP_U16);
	Lib_ByteSwap(&h[1], SWAP_U16);
}

void Audio_ByteSwap(AudioSampleInfo* sampleInfo) {
	if (sampleInfo->bit == 16) {
		for (s32 i = 0; i < sampleInfo->samplesNum * sampleInfo->channelNum; i++) {
			Lib_ByteSwap(&sampleInfo->audio.u16[i], sampleInfo->bit / 8);
		}
		
		return;
	}
	if (sampleInfo->bit == 24) {
		for (s32 i = 0; i < sampleInfo->samplesNum * sampleInfo->channelNum; i++) {
			Lib_ByteSwap(&sampleInfo->audio.u8[3 * i], sampleInfo->bit / 8);
		}
		
		return;
	}
	if (sampleInfo->bit == 32) {
		for (s32 i = 0; i < sampleInfo->samplesNum * sampleInfo->channelNum; i++) {
			Lib_ByteSwap(&sampleInfo->audio.u32[i], sampleInfo->bit / 8);
		}
		
		return;
	}
}
void Audio_Normalize(AudioSampleInfo* sampleInfo) {
	u32 sampleNum = sampleInfo->samplesNum;
	u32 channelNum = sampleInfo->channelNum;
	f32 max;
	f32 mult;
	
	printf_info("Normalizing.");
	
	if (sampleInfo->bit == 16) {
		max = 0;
		
		for (s32 i = 0; i < sampleNum * channelNum; i++) {
			if (sampleInfo->audio.s16[i] > max) {
				max = ABS(sampleInfo->audio.s16[i]);
			}
			
			if (max >= __INT16_MAX__) {
				return;
			}
		}
		
		mult = __INT16_MAX__ / max;
		
		for (s32 i = 0; i < sampleNum * channelNum; i++) {
			sampleInfo->audio.s16[i] *= mult;
		}
		
		return;
	}
	
	if (sampleInfo->bit == 32) {
		max = 0;
		
		for (s32 i = 0; i < sampleNum * channelNum; i++) {
			if (sampleInfo->audio.f32[i] > max) {
				max = fabsf(sampleInfo->audio.f32[i]);
			}
			
			if (max == 1.0f) {
				return;
			}
		}
		
		mult = 1.0f / max;
		
		for (s32 i = 0; i < sampleNum * channelNum; i++) {
			sampleInfo->audio.f32[i] *= mult;
		}
	}
	
	printf_debug("normalizermax: %f", max);
}
void Audio_ConvertToMono(AudioSampleInfo* sampleInfo) {
	if (sampleInfo->channelNum != 2)
		return;
	
	printf_info("Converting to mono.");
	
	if (sampleInfo->bit == 16) {
		for (s32 i = 0, j = 0; i < sampleInfo->samplesNum; i++, j += 2) {
			sampleInfo->audio.s16[i] = ((f32)sampleInfo->audio.s16[j] + (f32)sampleInfo->audio.s16[j + 1]) * 0.5f;
		}
	}
	
	if (sampleInfo->bit == 32) {
		for (s32 i = 0, j = 0; i < sampleInfo->samplesNum; i++, j += 2) {
			sampleInfo->audio.f32[i] = (sampleInfo->audio.f32[j] + sampleInfo->audio.f32[j + 1]) * 0.5f;
		}
	}
	
	sampleInfo->size /= 2;
	sampleInfo->channelNum = 1;
}
void Audio_Downsample(AudioSampleInfo* sampleInfo) {
	if (sampleInfo->targetBit == 16) {
		if (sampleInfo->bit == 24) {
			printf_info("Resampling from 24-bit to 16-bit.");
			for (s32 i = 0; i < sampleInfo->samplesNum * sampleInfo->channelNum; i++) {
				u16 samp = *(u16*)&sampleInfo->audio.u8[3 * i + 1];
				sampleInfo->audio.s16[i] = samp;
			}
			
			sampleInfo->bit = 16;
			sampleInfo->size *= (2.0 / 3.0);
		}
		
		if (sampleInfo->bit == 32) {
			if (sampleInfo->dataIsFloat == true) {
				printf_info("Resampling from 32-bit float to 16-bit.");
				for (s32 i = 0; i < sampleInfo->samplesNum * sampleInfo->channelNum; i++) {
					sampleInfo->audio.s16[i] = sampleInfo->audio.f32[i] * __INT16_MAX__;
				}
				
				sampleInfo->bit = 16;
				sampleInfo->size *= 0.5;
				sampleInfo->dataIsFloat = false;
			} else {
				printf_info("Resampling from 32-bit int to 16-bit.");
				for (s32 i = 0; i < sampleInfo->samplesNum * sampleInfo->channelNum; i++) {
					sampleInfo->audio.s16[i] = sampleInfo->audio.s32[i] * ((f32)__INT16_MAX__ / (f32)__INT32_MAX__);
				}
				
				sampleInfo->bit = 16;
				sampleInfo->size *= 0.5;
			}
		}
	}
	if (sampleInfo->targetBit == 24) {
		if (sampleInfo->bit == 32) {
			printf_error("Resample to 24-bit not supported yet.");
			// printf_info("Resampling from 32-bit to 16-bit.");
			// for (s32 i = 0; i < sampleInfo->samplesNum * sampleInfo->channelNum; i++) {
			// 	sampleInfo->audio.data[i] = sampleInfo->audio.dataFloat[i] * __INT16_MAX__;
			// }
			//
			// sampleInfo->bit = 16;
			// sampleInfo->size *= 0.5;
		}
	}
}
void Audio_Upsample(AudioSampleInfo* sampleInfo) {
	MemFile newMem;
	u32 samplesNum = sampleInfo->samplesNum;
	u32 channelNum = sampleInfo->channelNum;
	
	printf_info("Target bitrate higher than source. Upsampling.");
	printf_debug("%s: Upsampling will iterate %d times. %d x %d", __FUNCTION__, samplesNum * channelNum, samplesNum, channelNum);
	
	Lib_MallocMemFile(&newMem, samplesNum * sizeof(f32) * channelNum);
	
	if (sampleInfo->bit == 16) {
		for (s32 i = 0; i < samplesNum * channelNum; i++) {
			newMem.cast.f32[i] = (f32)sampleInfo->audio.s16[i] / __INT16_MAX__;
		}
	}
	
	if (sampleInfo->bit == 24) {
		for (s32 i = 0; i < samplesNum * channelNum; i++) {
			newMem.cast.f32[i] = (f32)(*(s16*)&sampleInfo->audio.u8[3 * i + 1]) / __INT16_MAX__;
		}
	}
	
	printf_debug("%s: Upsampling done.", __FUNCTION__);
	
	newMem.dataSize = sampleInfo->size =
	    sampleInfo->samplesNum *
	    sampleInfo->channelNum *
	    sizeof(f32);
	sampleInfo->bit = 32;
	sampleInfo->dataIsFloat = true;
	MemFile_Free(&sampleInfo->memFile);
	sampleInfo->memFile = newMem;
	sampleInfo->audio.p = newMem.data;
}
void Audio_Resample(AudioSampleInfo* sampleInfo) {
	if (sampleInfo->targetBit > sampleInfo->bit) {
		Audio_Upsample(sampleInfo);
	}
	
	Audio_Downsample(sampleInfo);
}

void Audio_InitSampleInfo(AudioSampleInfo* sampleInfo, char* input, char* output) {
	memset(sampleInfo, 0, sizeof(*sampleInfo));
	sampleInfo->instrument.note = 60;
	sampleInfo->instrument.highNote = __INT8_MAX__;
	sampleInfo->instrument.lowNote = 0;
	sampleInfo->input = input;
	sampleInfo->output = output;
}
void Audio_FreeSample(AudioSampleInfo* sampleInfo) {
	MemFile_Free(&sampleInfo->memFile);
	
	*sampleInfo = (AudioSampleInfo) { 0 };
}

void Audio_GetSampleInfo_Wav(WaveInstrumentInfo** waveInstInfo, WaveSampleInfo** waveSampleInfo, WaveHeader* header, u32 fileSize) {
	WaveInfo* wavInfo = Lib_MemMem(header, fileSize, "fmt ", 4);
	WaveDataInfo* wavData = Lib_MemMem(header, fileSize, "data", 4);
	u32 startOffset = sizeof(WaveHeader) + sizeof(WaveChunk) + wavInfo->chunk.size + sizeof(WaveChunk) + wavData->chunk.size;
	u32 searchLength = header->chunk.size - startOffset;
	u8* hayStart = ((u8*)header) + startOffset;
	
	printf_debug("%s: StartOffset  [0x%08X]", __FUNCTION__, startOffset);
	printf_debug("%s: SearchLength [0x%08X]", __FUNCTION__, searchLength);
	
	*waveInstInfo = Lib_MemMem(hayStart, searchLength, "inst", 4);
	*waveSampleInfo = Lib_MemMem(hayStart, searchLength, "smpl", 4);
	
	if (*waveInstInfo) {
		printf_debug("%s: Found InstrumentInfo", __FUNCTION__);
	}
	
	if (*waveSampleInfo) {
		printf_debug("%s: Found SampleInfo", __FUNCTION__);
	}
}
void Audio_GetSampleInfo_Aiff(AiffInstrumentInfo** aiffInstInfo, AiffMarkerInfo** aiffMarkerInfo, AiffHeader* header, u32 fileSize) {
	AiffInfo* aiffInfo = Lib_MemMem(header, fileSize, "COMM", 4);
	u32 startOffset = sizeof(AiffHeader) + aiffInfo->chunk.size;
	u32 searchLength = header->chunk.size - startOffset;
	u8* hayStart = ((u8*)header) + startOffset;
	
	printf_debug("%s: StartOffset  [0x%08X]", __FUNCTION__, startOffset);
	printf_debug("%s: SearchLength [0x%08X]", __FUNCTION__, searchLength);
	
	*aiffInstInfo = Lib_MemMem(hayStart, searchLength, "INST", 4);
	*aiffMarkerInfo = Lib_MemMem(hayStart, searchLength, "MARK", 4);
	
	if (*aiffInstInfo) {
		printf_debug("%s: Found InstrumentInfo", __FUNCTION__);
	}
	
	if (*aiffMarkerInfo) {
		printf_debug("%s: Found SampleInfo", __FUNCTION__);
	}
}
void Audio_GetSampleInfo_AifcVadpcm(AiffInstrumentInfo** aiffInstInfo, AiffHeader* header, u32 fileSize) {
	*aiffInstInfo = Lib_MemMem(header, fileSize, "INST", 4);
	
	if (*aiffInstInfo) {
		printf_debug("%s: Found InstrumentInfo", __FUNCTION__);
	}
}

void Audio_LoadSample_Wav(AudioSampleInfo* sampleInfo) {
	MemFile_LoadToMemFile_ReqExt(&sampleInfo->memFile, sampleInfo->input, ".wav");
	WaveHeader* waveHeader = sampleInfo->memFile.data;
	WaveDataInfo* waveData;
	WaveInfo* waveInfo;
	
	if (!sampleInfo->memFile.data) {
		printf_error("Closing.");
	}
	
	if (!Lib_MemMem(waveHeader, 0x10, "WAVE", 4) || !Lib_MemMem(waveHeader, 0x10, "RIFF", 4)) {
		char headerA[5] = { 0 };
		char headerB[5] = { 0 };
		
		memcpy(headerA, waveHeader->chunk.name, 4);
		memcpy(headerB, waveHeader->format, 4);
		
		printf_error(
			"[%s] header does not match what's expected\n"
			"\t\tChunkHeader: \e[0;91m[%s]\e[m -> [RIFF]\n"
			"\t\tFormat:      \e[0;91m[%s]\e[m -> [WAVE]",
			sampleInfo->input,
			headerA,
			headerB
		);
	}
	
	printf_debug("%s: File [%s] loaded to memory", __FUNCTION__, sampleInfo->input);
	
	if (sampleInfo->memFile.dataSize == 0)
		printf_error("%s: Something has gone wrong loading file [%s]", __FUNCTION__, sampleInfo->input);
	waveInfo = Lib_MemMem(waveHeader, sampleInfo->memFile.dataSize, "fmt ", 4);
	waveData = Lib_MemMem(waveHeader, sampleInfo->memFile.dataSize, "data", 4);
	if (!waveInfo || !waveData) {
		if (!waveData) {
			printf_error(
				"%s: could not locate [data] from [%s]",
				__FUNCTION__,
				sampleInfo->input
			);
		}
		if (!waveInfo) {
			printf_error(
				"%s: could not locate [fmt] from [%s]",
				__FUNCTION__,
				sampleInfo->input
			);
		}
	}
	
	sampleInfo->channelNum = waveInfo->channelNum;
	sampleInfo->bit = waveInfo->bit;
	sampleInfo->sampleRate = waveInfo->sampleRate;
	sampleInfo->samplesNum = waveData->chunk.size / (waveInfo->bit / 8) / waveInfo->channelNum;
	sampleInfo->size = waveData->chunk.size;
	sampleInfo->audio.p = waveData->data;
	sampleInfo->instrument.loop.end = sampleInfo->samplesNum;
	
	WaveInstrumentInfo* waveInstInfo;
	WaveSampleInfo* waveSampleInfo;
	
	Audio_GetSampleInfo_Wav(&waveInstInfo, &waveSampleInfo, waveHeader, sampleInfo->memFile.dataSize);
	if (waveInfo->format == IEEE_FLOAT) {
		printf_debug("%s: Uses 32-bit floats", __FUNCTION__);
		sampleInfo->dataIsFloat = true;
	}
	if (waveInstInfo) {
		sampleInfo->instrument.fineTune = waveInstInfo->fineTune;
		sampleInfo->instrument.note = waveInstInfo->note;
		sampleInfo->instrument.highNote = waveInstInfo->hiNote;
		sampleInfo->instrument.lowNote = waveInstInfo->lowNote;
		
		if (waveSampleInfo && waveSampleInfo->numSampleLoops) {
			sampleInfo->instrument.loop.start = waveSampleInfo->loopData[0].start;
			sampleInfo->instrument.loop.end = waveSampleInfo->loopData[0].end;
			sampleInfo->instrument.loop.count = waveSampleInfo->loopData[0].count;
		}
	}
}
void Audio_LoadSample_Aiff(AudioSampleInfo* sampleInfo) {
	MemFile_LoadToMemFile_ReqExt(&sampleInfo->memFile, sampleInfo->input, ".aiff");
	AiffHeader* aiffHeader = sampleInfo->memFile.data;
	AiffDataInfo* aiffData;
	AiffInfo* aiffInfo;
	
	if (!sampleInfo->memFile.data) {
		printf_error("Closing.");
	}
	
	if (!Lib_MemMem(aiffHeader, 0x10, "FORM", 4) || !Lib_MemMem(aiffHeader, 0x10, "AIFF", 4)) {
		char headerA[5] = { 0 };
		char headerB[5] = { 0 };
		
		memcpy(headerA, aiffHeader->chunk.name, 4);
		memcpy(headerB, aiffHeader->formType, 4);
		
		printf_error(
			"[%s] header does not match what's expected\n"
			"\t\tChunkHeader: \e[0;91m[%s]\e[m -> [FORM]\n"
			"\t\tFormat:      \e[0;91m[%s]\e[m -> [AIFF]",
			sampleInfo->input,
			headerA,
			headerB
		);
	}
	
	printf_debug("%s: File [%s] loaded to memory", __FUNCTION__, sampleInfo->input);
	
	if (sampleInfo->memFile.dataSize == 0)
		printf_error("%s: Something has gone wrong loading file [%s]", __FUNCTION__, sampleInfo->input);
	aiffInfo = Lib_MemMem(aiffHeader, sampleInfo->memFile.dataSize, "COMM", 4);
	aiffData = Lib_MemMem(aiffHeader, sampleInfo->memFile.dataSize, "SSND", 4);
	if (!aiffInfo || !aiffData) {
		if (!aiffData) {
			printf_error(
				"%s: could not locate [SSND] from [%s]",
				__FUNCTION__,
				sampleInfo->input
			);
		}
		if (!aiffInfo) {
			printf_error(
				"%s: could not locate [COMM] from [%s]",
				__FUNCTION__,
				sampleInfo->input
			);
		}
	}
	
	// Swap Chunk Sizes
	Lib_ByteSwap(&aiffHeader->chunk.size, SWAP_U32);
	Lib_ByteSwap(&aiffInfo->chunk.size, SWAP_U32);
	Lib_ByteSwap(&aiffData->chunk.size, SWAP_U32);
	
	// Swap INFO
	Lib_ByteSwap(&aiffInfo->channelNum, SWAP_U16);
	Lib_ByteSwap(&aiffInfo->sampleNumH, SWAP_U16);
	Lib_ByteSwap(&aiffInfo->sampleNumL, SWAP_U16);
	Lib_ByteSwap(&aiffInfo->bit, SWAP_U16);
	
	// Swap DATA
	Lib_ByteSwap(&aiffData->offset, SWAP_U32);
	Lib_ByteSwap(&aiffData->blockSize, SWAP_U32);
	
	u32 sampleNum = aiffInfo->sampleNumL + (aiffInfo->sampleNumH << 8);
	
	sampleInfo->channelNum = aiffInfo->channelNum;
	sampleInfo->bit = aiffInfo->bit;
	sampleInfo->sampleRate = Audio_ConvertBigEndianFloat80(aiffInfo);
	sampleInfo->samplesNum = sampleNum;
	sampleInfo->size = aiffData->chunk.size - 0x8;
	sampleInfo->audio.p = aiffData->data;
	
	sampleInfo->instrument.loop.end = sampleInfo->samplesNum;
	
	AiffInstrumentInfo* aiffInstInfo;
	AiffMarkerInfo* aiffMarkerInfo;
	
	Audio_GetSampleInfo_Aiff(&aiffInstInfo, &aiffMarkerInfo, aiffHeader, sampleInfo->memFile.dataSize);
	
	if (aiffInstInfo) {
		sampleInfo->instrument.note = aiffInstInfo->baseNote;
		sampleInfo->instrument.fineTune = aiffInstInfo->detune;
		sampleInfo->instrument.highNote = aiffInstInfo->highNote;
		sampleInfo->instrument.lowNote = aiffInstInfo->lowNote;
	}
	
	if (aiffMarkerInfo && aiffInstInfo && aiffInstInfo->sustainLoop.playMode >= 1) {
		s32 startIndex = aiffInstInfo->sustainLoop.start - 1;
		s32 endIndex = aiffInstInfo->sustainLoop.end - 1;
		u16 loopStartH = aiffMarkerInfo->marker[startIndex].positionH;
		u16 loopStartL = aiffMarkerInfo->marker[startIndex].positionL;
		u16 loopEndH = aiffMarkerInfo->marker[endIndex].positionH;
		u16 loopEndL = aiffMarkerInfo->marker[endIndex].positionL;
		
		sampleInfo->instrument.loop.start = Audio_ByteSwap_FromHighLow(&loopStartH, &loopStartL);
		sampleInfo->instrument.loop.end = Audio_ByteSwap_FromHighLow(&loopEndH, &loopEndL);
		sampleInfo->instrument.loop.count = 0xFFFFFFFF;
	}
	
	Audio_ByteSwap(sampleInfo);
}
void Audio_LoadSample_AifcVadpcm(AudioSampleInfo* sampleInfo) {
	MemFile_LoadToMemFile_ReqExt(&sampleInfo->memFile, sampleInfo->input, ".aifc");
	AiffHeader* aiffHeader = sampleInfo->memFile.data;
	AiffDataInfo* aiffData;
	AiffInfo* aiffInfo;
	
	if (!sampleInfo->memFile.data) {
		printf_error("Closing.");
	}
	
	if (!Lib_MemMem(aiffHeader, 0x10, "FORM", 4) || !Lib_MemMem(aiffHeader, 0x10, "AIFC", 4)) {
		char headerA[5] = { 0 };
		char headerB[5] = { 0 };
		
		memcpy(headerA, aiffHeader->chunk.name, 4);
		memcpy(headerB, aiffHeader->formType, 4);
		
		printf_error(
			"[%s] header does not match what's expected\n"
			"\t\tChunkHeader: \e[0;91m[%s]\e[m -> [FORM]\n"
			"\t\tFormat:      \e[0;91m[%s]\e[m -> [AIFC]",
			sampleInfo->input,
			headerA,
			headerB
		);
	}
	
	if (!Lib_MemMem(aiffHeader, 0x90, "VAPC\x0BVADPCM", 10)) {
		printf_error("[%s] does not appear to use VADPCM compression type.", sampleInfo->input);
	}
	
	printf_debug("%s: File [%s] loaded to memory", __FUNCTION__, sampleInfo->input);
	
	if (sampleInfo->memFile.dataSize == 0)
		printf_error("Audio_LoadSample_AifcVadpcm: Something has gone wrong loading file [%s]", sampleInfo->input);
	aiffInfo = Lib_MemMem(aiffHeader, sampleInfo->memFile.dataSize, "COMM", 4);
	aiffData = Lib_MemMem(aiffHeader, sampleInfo->memFile.dataSize, "SSND", 4);
	if (!aiffInfo || !aiffData) {
		if (!aiffData) {
			printf_error(
				"%s: could not locate [SSND] from [%s]",
				__FUNCTION__,
				sampleInfo->input
			);
		}
		if (!aiffInfo) {
			printf_error(
				"%s: could not locate [COMM] from [%s]",
				__FUNCTION__,
				sampleInfo->input
			);
		}
	}
	
	// Swap Chunk Sizes
	Lib_ByteSwap(&aiffHeader->chunk.size, SWAP_U32);
	Lib_ByteSwap(&aiffInfo->chunk.size, SWAP_U32);
	Lib_ByteSwap(&aiffData->chunk.size, SWAP_U32);
	
	// Swap INFO
	Lib_ByteSwap(&aiffInfo->channelNum, SWAP_U16);
	Lib_ByteSwap(&aiffInfo->sampleNumH, SWAP_U16);
	Lib_ByteSwap(&aiffInfo->sampleNumL, SWAP_U16);
	Lib_ByteSwap(&aiffInfo->bit, SWAP_U16);
	
	// Swap DATA
	Lib_ByteSwap(&aiffData->offset, SWAP_U32);
	Lib_ByteSwap(&aiffData->blockSize, SWAP_U32);
	
	u32 sampleNum = aiffInfo->sampleNumL + (aiffInfo->sampleNumH << 8);
	
	sampleInfo->channelNum = aiffInfo->channelNum;
	sampleInfo->bit = aiffInfo->bit;
	sampleInfo->sampleRate = Audio_ConvertBigEndianFloat80(aiffInfo);
	sampleInfo->samplesNum = sampleNum;
	sampleInfo->size = aiffData->chunk.size - 0x8;
	sampleInfo->audio.p = aiffData->data;
	
	sampleInfo->instrument.loop.end = sampleInfo->samplesNum;
	
	AiffInstrumentInfo* aiffInstInfo;
	
	Audio_GetSampleInfo_AifcVadpcm(&aiffInstInfo, aiffHeader, sampleInfo->memFile.dataSize);
	
	if (aiffInstInfo) {
		sampleInfo->instrument.note = aiffInstInfo->baseNote;
		sampleInfo->instrument.fineTune = aiffInstInfo->detune;
		sampleInfo->instrument.highNote = aiffInstInfo->highNote;
		sampleInfo->instrument.lowNote = aiffInstInfo->lowNote;
	}
	
	// VADPCM PREDICTORS
	VadpcmInfo* pred = Lib_MemMem(aiffHeader, sampleInfo->memFile.dataSize, "APPL", 4);
	
	if (pred && Lib_MemMem(pred, 24, "VADPCMCODES", 11)) {
		sampleInfo->vadpcm.pred.p = pred->data;
		sampleInfo->vadpcm.sizePred = pred->chunk.size - sizeof(VadpcmInfo);
	}
	
	// VADPCM LOOP PREDICTORS
	u32 size = sampleInfo->memFile.dataSize - ((u64)aiffData - (u64)aiffHeader);
	
	pred = Lib_MemMem(aiffData, size, "APPL", 4);
	
	if (pred && Lib_MemMem(pred, 24, "VADPCMLOOPS", 11)) {
		sampleInfo->vadpcm.loopPred.p = pred->data;
		sampleInfo->vadpcm.sizePredLoop = pred->chunk.size - sizeof(VadpcmInfo);
	}
}
void Audio_LoadSample(AudioSampleInfo* sampleInfo) {
	char* keyword[] = {
		".wav",
		".aiff",
		".aifc",
	};
	AudioFunc loadSample[] = {
		Audio_LoadSample_Wav,
		Audio_LoadSample_Aiff,
		Audio_LoadSample_AifcVadpcm,
		NULL
	};
	
	if (!sampleInfo->input)
		printf_error("Audio_LoadSample: No input file set");
	if (!sampleInfo->output)
		printf_error("Audio_LoadSample: No output file set");
	
	for (s32 i = 0; i < ARRAY_COUNT(loadSample); i++) {
		if (loadSample[i] == NULL) {
			printf_warning("[%s] does not match expected extensions. This tool supports these extensions as input:", sampleInfo->input);
			for (s32 j = 0; j < ARRAY_COUNT(keyword); j++) {
				printf("[%s] ", keyword[j]);
			}
			printf("\n");
			printf_error("Closing.");
		}
		if (Lib_MemMem(sampleInfo->input, strlen(sampleInfo->input), keyword[i], strlen(keyword[i]))) {
			char basename[128];
			
			String_GetBasename(basename, sampleInfo->input);
			printf_info("Loading [%s%s]", basename, keyword[i]);
			
			loadSample[i](sampleInfo);
			break;
		}
	}
}

void Audio_SaveSample_Wav(AudioSampleInfo* sampleInfo) {
	WaveHeader header = { 0 };
	WaveInfo info = { 0 };
	WaveDataInfo dataInfo = { 0 };
	WaveInstrumentInfo instrument = { 0 };
	WaveSampleInfo sample = { 0 };
	WaveSampleLoop loop = { 0 };
	
	char basename[128];
	
	String_GetBasename(basename, sampleInfo->output);
	
	/* Write chunk headers */ {
		header.chunk.size = 4 +
		    sizeof(WaveInfo) +
		    sizeof(WaveDataInfo) +
		    sampleInfo->size +
		    sizeof(WaveInstrumentInfo) +
		    sizeof(WaveSampleInfo) +
		    sizeof(WaveSampleLoop);
		info.chunk.size = sizeof(WaveInfo) - sizeof(WaveChunk);
		dataInfo.chunk.size = sampleInfo->size;
		instrument.chunk.size = sizeof(WaveInstrumentInfo) - sizeof(WaveChunk);
		sample.chunk.size = sizeof(WaveSampleInfo) - sizeof(WaveChunk) + sizeof(WaveSampleLoop);
		
		info.format = PCM;
		if (sampleInfo->bit == 32 && sampleInfo->dataIsFloat)
			info.format = IEEE_FLOAT;
		info.channelNum = sampleInfo->channelNum;
		info.sampleRate = sampleInfo->sampleRate;
		info.byteRate = sampleInfo->sampleRate * sampleInfo->channelNum * sampleInfo->bit;
		info.blockAlign = sampleInfo->channelNum * (sampleInfo->bit / 8);
		info.bit = sampleInfo->bit;
		
		instrument.note = sampleInfo->instrument.note;
		instrument.fineTune = sampleInfo->instrument.fineTune;
		instrument.gain = __INT8_MAX__;
		instrument.lowNote = sampleInfo->instrument.lowNote;
		instrument.hiNote = sampleInfo->instrument.highNote;
		instrument.lowNote = 0;
		instrument.hiVel = __INT8_MAX__;
		
		sample.numSampleLoops = 1;
		loop.count = sampleInfo->instrument.loop.count;
		loop.start = sampleInfo->instrument.loop.start;
		loop.end = sampleInfo->instrument.loop.end;
		loop.type = 1;
	}
	memcpy(header.chunk.name, "RIFF", 4);
	memcpy(header.format, "WAVE", 4);
	memcpy(info.chunk.name, "fmt ", 4);
	memcpy(dataInfo.chunk.name, "data", 4);
	memcpy(sample.chunk.name, "smpl", 4);
	memcpy(instrument.chunk.name, "inst", 4);
	
	FILE* output = fopen(sampleInfo->output, "w");
	
	if (output == NULL)
		printf_error("Audio_SaveSample_Wav: Could not open outputfile [%s]", sampleInfo->output);
	
	fwrite(&header, 1, sizeof(header), output);
	fwrite(&info, 1, sizeof(info), output);
	fwrite(&dataInfo, 1, sizeof(dataInfo), output);
	fwrite(sampleInfo->audio.p, 1, sampleInfo->size, output);
	fwrite(&instrument, 1, sizeof(instrument), output);
	fwrite(&sample, 1, sizeof(sample), output);
	fwrite(&loop, 1, sizeof(loop), output);
	
	fclose(output);
}
void Audio_SaveSample_Aiff(AudioSampleInfo* sampleInfo) {
	AiffHeader header = { 0 };
	AiffInfo info = { 0 };
	AiffDataInfo dataInfo = { 0 };
	AiffMarker marker[2] = { 0 };
	AiffMarkerInfo markerInfo = { 0 };
	AiffInstrumentInfo instrument = { 0 };
	
	char basename[128];
	
	String_GetBasename(basename, sampleInfo->output);
	
	// AIFF 32-bit == s32
	if (sampleInfo->bit == 32 && sampleInfo->dataIsFloat) {
		for (s32 i = 0; i < sampleInfo->samplesNum * sampleInfo->channelNum; i++) {
			sampleInfo->audio.s32[i] = sampleInfo->audio.f32[i] * 2147483000.0f;
		}
	}
	
	Audio_ByteSwap(sampleInfo);
	
	/* Write chunk headers */ {
		info.chunk.size = sizeof(AiffInfo) - sizeof(AiffChunk) - 2;
		dataInfo.chunk.size = sampleInfo->size + 0x8;
		markerInfo.chunk.size = sizeof(u16) + sizeof(AiffMarker) * 2;
		instrument.chunk.size = sizeof(AiffInstrumentInfo) - sizeof(AiffChunk);
		header.chunk.size = info.chunk.size +
		    dataInfo.chunk.size +
		    markerInfo.chunk.size +
		    instrument.chunk.size +
		    sizeof(AiffChunk) +
		    sizeof(AiffChunk) +
		    sizeof(AiffChunk) +
		    sizeof(AiffChunk) + 4;
		
		Lib_ByteSwap(&info.chunk.size, SWAP_U32);
		Lib_ByteSwap(&dataInfo.chunk.size, SWAP_U32);
		Lib_ByteSwap(&markerInfo.chunk.size, SWAP_U32);
		Lib_ByteSwap(&instrument.chunk.size, SWAP_U32);
		Lib_ByteSwap(&header.chunk.size, SWAP_U32);
		
		f80 sampleRate = sampleInfo->sampleRate;
		u8* p = (u8*)&sampleRate;
		
		for (s32 i = 0; i < 10; i++) {
			info.sampleRate[i] = p[9 - i];
		}
		
		info.channelNum = sampleInfo->channelNum;
		Audio_ByteSwap_ToHighLow(&sampleInfo->samplesNum, &info.sampleNumH);
		info.bit = sampleInfo->bit;
		Lib_ByteSwap(&info.channelNum, SWAP_U16);
		Lib_ByteSwap(&info.bit, SWAP_U16);
		
		markerInfo.markerNum = 2;
		marker[0].index = 1;
		Audio_ByteSwap_ToHighLow(&sampleInfo->instrument.loop.start, &marker[0].positionH);
		marker[1].index = 2;
		Audio_ByteSwap_ToHighLow(&sampleInfo->instrument.loop.end, &marker[1].positionH);
		
		Lib_ByteSwap(&markerInfo.markerNum, SWAP_U16);
		Lib_ByteSwap(&marker[0].index, SWAP_U16);
		Lib_ByteSwap(&marker[1].index, SWAP_U16);
		
		instrument.baseNote = sampleInfo->instrument.note;
		instrument.detune = sampleInfo->instrument.fineTune;
		instrument.lowNote = sampleInfo->instrument.lowNote;
		instrument.highNote = sampleInfo->instrument.highNote;
		instrument.lowVelocity = 0;
		instrument.highNote = 127;
		instrument.gain = __INT16_MAX__;
		Lib_ByteSwap(&instrument.gain, SWAP_U16);
		
		instrument.sustainLoop.start = 1;
		instrument.sustainLoop.end = 2;
		instrument.sustainLoop.playMode = 1;
		Lib_ByteSwap(&instrument.sustainLoop.start, SWAP_U16);
		Lib_ByteSwap(&instrument.sustainLoop.end, SWAP_U16);
		Lib_ByteSwap(&instrument.sustainLoop.playMode, SWAP_U16);
	}
	memcpy(header.chunk.name, "FORM", 4);
	memcpy(header.formType, "AIFF", 4);
	memcpy(info.chunk.name, "COMM", 4);
	memcpy(info.compressionType, "NONE", 4);
	memcpy(dataInfo.chunk.name, "SSND", 4);
	memcpy(markerInfo.chunk.name, "MARK", 4);
	memcpy(instrument.chunk.name, "INST", 4);
	
	FILE* output = fopen(sampleInfo->output, "w");
	
	if (output == NULL)
		printf_error("Audio_SaveSample_Aiff: Could not open outputfile [%s]", sampleInfo->output);
	
	fwrite(&header, 1, sizeof(header), output);
	fwrite(&info, 1, 0x16 + sizeof(AiffChunk), output);
	fwrite(&markerInfo, 1, 0xA, output);
	fwrite(&marker[0], 1, sizeof(AiffMarker), output);
	fwrite(&marker[1], 1, sizeof(AiffMarker), output);
	fwrite(&instrument, 1, sizeof(instrument), output);
	fwrite(&dataInfo, 1, 16, output);
	fwrite(sampleInfo->audio.p, 1, sampleInfo->size, output);
	fclose(output);
}
void Audio_SaveSample_VadpcmC(AudioSampleInfo* sampleInfo) {
	char basename[1024];
	FILE* output = fopen(sampleInfo->output, "w");
	
	String_GetBasename(basename, sampleInfo->output);
	
	fprintf(
		output,
		"ALADPCMPredictor %sPred[] = {\n",
		basename
	);
	
	Lib_ByteSwap(&sampleInfo->vadpcm.pred.u16[1], SWAP_U16);
	Lib_ByteSwap(&sampleInfo->vadpcm.pred.u16[2], SWAP_U16);
	
	u32 numPred = sampleInfo->vadpcm.pred.u16[2];
	
	for (s32 j = 0; j < numPred; j++) {
		fprintf(
			output,
			"	{\n"
		);
		for (s32 i = 0; i < 0x10; i++) {
			Lib_ByteSwap(&sampleInfo->vadpcm.pred.u16[3 + i + 0x10 * j], SWAP_U16);
			fprintf(output, "		0x%04X,\n", sampleInfo->vadpcm.pred.u16[3 + i + 0x10 * j]);
		}
		fprintf(
			output,
			"	},\n"
		);
	}
	
	fprintf(
		output,
		"};\n\n"
	);
	
	Lib_ByteSwap(&sampleInfo->vadpcm.loopPred.u32[1], SWAP_U32);
	Lib_ByteSwap(&sampleInfo->vadpcm.loopPred.u32[2], SWAP_U32);
	Lib_ByteSwap(&sampleInfo->vadpcm.loopPred.u32[3], SWAP_U32);
	
	if (sampleInfo->vadpcm.loopPred.p) {
		fprintf(
			output,
			"ALADPCMLoopU %sLoop = {\n"
			"	.withtail = {\n"
			"		.main = {\n"
			"			.start = %d,\n"
			"			.end = %d,\n"
			"			.count = 0x%08X,\n"
			"			.adpcmState = 0x%08X,\n"
			"		},\n"
			"		.tail = {\n"
			"			.data = {\n",
			basename,
			sampleInfo->vadpcm.loopPred.u32[1],
			sampleInfo->vadpcm.loopPred.u32[2],
			sampleInfo->vadpcm.loopPred.u32[3],
			sampleInfo->samplesNum > sampleInfo->vadpcm.loopPred.u32[2] ? sampleInfo->samplesNum : 0
		);
		
		for (s32 i = 0; i < 0x10; i++) {
			Lib_ByteSwap(&sampleInfo->vadpcm.loopPred.u16[8 + i], SWAP_U16);
			fprintf(
				output,
				"				0x%04X,\n",
				sampleInfo->vadpcm.loopPred.u16[8 + i]
			);
		}
		fprintf(
			output,
			"			},\n"
			"		},\n"
			"	},\n"
			"};\n\n"
		);
	} else {
		fprintf(
			output,
			"ALADPCMLoopU %sLoop = {\n"
			"	.notail = {\n"
			"		.start = %d,\n"
			"		.end = %d,\n"
			"		.count = 0x%08X,\n"
			"		.adpcmState = 0x%08X,\n"
			"	}\n"
			"};\n\n",
			basename,
			sampleInfo->vadpcm.loopPred.u32[1],
			sampleInfo->vadpcm.loopPred.u32[2],
			sampleInfo->vadpcm.loopPred.u32[3],
			sampleInfo->samplesNum > sampleInfo->vadpcm.loopPred.u32[2] ? sampleInfo->samplesNum : 0
		);
	}
	
	fprintf(
		output,
		"ABSample %sSample = {\n"
		"	.len = %d,\n"
		"	.loop = &%sLoop,\n"
		"	.book = %sPred,\n"
		"};\n\n",
		basename,
		sampleInfo->samplesNum,
		basename,
		basename
	);
	
	fprintf(
		output,
		"ABEnvelope %sEnv = {\n"
		"	.points = {\n"
		"		{\n"
		"			.rate = 0,\n"
		"			.level = 0,\n"
		"		},\n"
		"		{\n"
		"			.rate = 0,\n"
		"			.level = 0,\n"
		"		},\n"
		"		{\n"
		"			.rate = 0,\n"
		"			.level = 0,\n"
		"		},\n"
		"		{\n"
		"			.rate = 0,\n"
		"			.level = 0,\n"
		"		},\n"
		"	}\n"
		"};\n\n",
		basename
	);
	
	fprintf(
		output,
		"ABInstrument %sInstrument = {\n"
		"	.splitHigh = %d,\n"
		"	.splitLow = %d,\n"
		"	.releaseRate = 0,\n"
		"	.envelope = &%sEnv,\n"
		"	.splits = {\n"
		"		{ /* Not supported yet */ },\n"
		"		{\n"
		"			.sample = &%sSample,\n"
		"			.tuning = %gf,\n"
		"		},\n"
		"		{ /* Not supported yet */ },\n"
		"	}\n"
		"};\n",
		basename,
		// For some reason the splits have to subtracted with 21 to match
		// how they're mapped in OoT
		CLAMP(sampleInfo->instrument.highNote - 21, 0, 127),
		CLAMP(sampleInfo->instrument.lowNote - 21, 0, 127),
		basename,
		basename,
		((f32)sampleInfo->sampleRate / 32000.0f) * pow(
			pow(2, 1.0 / 12.0),
			60.0 - (f64)sampleInfo->instrument.note +
			0.01 * sampleInfo->instrument.fineTune
		)
	);
	
	fclose(output);
	String_Merge(basename, "Data.bin");
	printf_info("Saving [%s]", basename);
	fopen(basename, "w");
	fwrite(sampleInfo->audio.p, 1, sampleInfo->size, output);
}
void Audio_SaveSample(AudioSampleInfo* sampleInfo) {
	char* keyword[] = {
		".wav",
		".aiff",
		".c",
	};
	AudioFunc saveSample[] = {
		Audio_SaveSample_Wav,
		Audio_SaveSample_Aiff,
		Audio_SaveSample_VadpcmC,
		NULL
	};
	char* funcName[] = {
		"Audio_SaveSample_Wav",
		"Audio_SaveSample_Aiff",
		"Audio_SaveSample_VadpcmC"
	};
	
	for (s32 i = 0; i < ARRAY_COUNT(saveSample); i++) {
		if (saveSample[i] == NULL) {
			printf_warning("[%s] does not match expected extensions. This tool supports these extensions as output:", sampleInfo->output);
			for (s32 j = 0; j < ARRAY_COUNT(keyword); j++) {
				printf("[%s] ", keyword[j]);
			}
			printf("\n");
			printf_error("Closing.");
		}
		
		if (Lib_MemMem(sampleInfo->output, strlen(sampleInfo->output), keyword[i], strlen(keyword[i]))) {
			char basename[128];
			
			if (i == 2 && !Lib_MemMem(sampleInfo->input, strlen(sampleInfo->input), ".aifc", 5)) {
				printf_error("C output is currently only supported for vadpcm [.aifc] input");
			}
			
			String_GetBasename(basename, sampleInfo->output);
			printf_info("Saving [%s%s]", basename, keyword[i]);
			printf_debug("%s", funcName[i]);
			
			saveSample[i](sampleInfo);
			break;
		}
	}
}