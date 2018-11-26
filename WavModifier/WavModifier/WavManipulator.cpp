#include "pch.h"
#include "WavManipulator.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>


typedef struct  WAV_HEADER
{
	/* RIFF Chunk Descriptor */
	uint8_t         RIFF[4];        // RIFF Header Magic header
	uint32_t        ChunkSize;      // RIFF Chunk Size
	uint8_t         WAVE[4];        // WAVE Header
	/* "fmt" sub-chunk */
	uint8_t         fmt[4];         // FMT header
	uint32_t        Subchunk1Size;  // Size of the fmt chunk
	uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
	uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
	uint32_t        bytesPerSec;    // bytes per second
	uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
	uint16_t        bitsPerSample;  // Number of bits per sample
	/* "data" sub-chunk */
	uint8_t         Subchunk2ID[4]; // "data"  string
	uint32_t        Subchunk2Size;  // Sampled data length
} wav_hdr;

typedef struct  LIST_CHUNK
{
	uint8_t         ListTypeId[4]; // "data"  string
	uint8_t			data[64];
} list_chunk;

typedef struct SubChunk_Header {
	uint8_t         SubchunkID[4]; // "data"  string
	uint32_t        SubchunkSize;  // Sampled data length
} subchunk_header;
/*
 *
 * Wave (*.wav) PCM File Format
 *
 * OFFSET  SIZE  NAME             DESCRIPTION
 * 0         4   ChunkID          Contains the letters "RIFF"
 * 4         4   ChunkSize        38 + ExtraParamSize + SubChunk2Size
 *                                This is the offset of the actual sound data
 * 8         4   Format           Contains the letters "WAVE"
 * 12        4   Subchunk1ID      Contains the letters "fmt "
 * 16        4   Subchunk1Size    18 + ExtraParamSize
 * 20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
 * 22        2   NumChannels      mono = 1, stereo = 2, etc.
 * 24        4   SampleRate       8000, 44100, etc.
 * 28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
 * 32        2   BlockAlign       == NumChannels * BitsPerSample/8
 * 34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
 * 36        4   Subchunk2ID      Contains the letters "data"
 * 40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
 *                                the number of data bytes
 * 44        *   Data             the actual sound data.
 *
 */


WavManipulator::WavManipulator()
{
}


WavManipulator::~WavManipulator()
{
}

void WavManipulator::InsertBeepIntoStream(std::string fileName)
{
	std::ifstream origFile;
	std::ofstream destFile;

	origFile.open(fileName, std::ios::in | std::ios::binary);
	destFile.open("c:\\ffmpeg\\bin\\TestResult.wav", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);


	wav_hdr wavHeader;
	list_chunk listHeader;

	if (origFile.good() && destFile.good()) {
		std::copy(
			std::istreambuf_iterator<char>(origFile),
			std::istreambuf_iterator<char>(),
			std::ostreambuf_iterator<char>(destFile));

		uint32_t dataSize;
		origFile.seekg(0);
		origFile.read(reinterpret_cast<char*>(&wavHeader), sizeof(wav_hdr));

		uint8_t listChunk[4]{ 'L','I','S','T' };
		if (std::equal(std::begin(wavHeader.Subchunk2ID), std::end(wavHeader.Subchunk2ID), std::begin(listChunk))) {
			origFile.read(reinterpret_cast<char*>(&listHeader), wavHeader.Subchunk2Size);
			subchunk_header subchnk;
			origFile.read(reinterpret_cast<char*>(&subchnk), sizeof(subchunk_header));
			dataSize = subchnk.SubchunkSize;
		}
		else
		{
			dataSize = wavHeader.Subchunk2Size;
		}

		int length = dataSize / wavHeader.SamplesPerSec / (wavHeader.bitsPerSample / 8);
		int seek = origFile.tellg();
		if (length > 3) {
			seek += wavHeader.SamplesPerSec * (wavHeader.bitsPerSample / 8);
		}

		destFile.seekp(seek);

		std::vector<char> noise(wavHeader.SamplesPerSec * (wavHeader.bitsPerSample / 8), 255);
		destFile.write(noise.data(), noise.size());
	}

	origFile.close();


	destFile.close();
}
