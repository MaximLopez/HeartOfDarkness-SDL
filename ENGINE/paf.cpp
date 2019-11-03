#include "fs.h"
#include "paf.h"
#include "system.h"
#include "util.h"

static const char *_filenames[] = {
	"hod.paf",
	"hod_demo.paf",
	"hod_demo2.paf",
	0
};

static bool openPaf(FileSystem *fs, File *f) {
	for (int i = 0; _filenames[i]; ++i) {
		FILE *fp = fs->openFile(_filenames[i]);
		if (fp) {
			f->setFp(fp);
			return true;
		}
	}
	return false;
}

static void closePaf(FileSystem *fs, File *f) {
	if (f->_fp) {
		fs->closeFile(f->_fp);
		f->_fp = 0;
	}
}

PafPlayer::PafPlayer(System *system, FileSystem *fs)
	: _skipCutscenes(false), _system(system), _fs(fs) {
	if (!openPaf(_fs, &_file)) {
		_skipCutscenes = true;
	}
	_videoNum = -1;
	memset(&_pafHdr, 0, sizeof(_pafHdr));
	memset(_pageBuffers, 0, sizeof(_pageBuffers));
	_demuxAudioFrameBlocks = 0;
	_demuxVideoFrameBlocks = 0;
	_audioQueue = _audioQueueTail = 0;
	_playedMask = 0;
}

PafPlayer::~PafPlayer() {
	unload();
	closePaf(_fs, &_file);
}

void PafPlayer::preload(int num) {
	debug(kDebug_PAF, "preload %d", num);
	assert(num >= 0 && num < kMaxVideosCount);
	if (_videoNum != num) {
		unload(_videoNum);
	}
	_file.seek(num * 4, SEEK_SET);
	_videoOffset = _file.readUint32();
	_file.seek(_videoOffset, SEEK_SET);
	memset(&_pafHdr, 0, sizeof(_pafHdr));
	readPafHeader();
	if (_pafHdr.framesCount == 0) {
		return;
	}
	_videoNum = num;
	for (int i = 0; i < 4; ++i) {
		_pageBuffers[i] = (uint8_t *)calloc(kPageBufferSize, 1);
	}
	_demuxVideoFrameBlocks = (uint8_t *)calloc(_pafHdr.maxVideoFrameBlocksCount, _pafHdr.readBufferSize);
	if (_pafHdr.maxAudioFrameBlocksCount != 0) {
		_demuxAudioFrameBlocks = (uint8_t *)calloc(_pafHdr.maxAudioFrameBlocksCount, _pafHdr.readBufferSize);
		_flushAudioSize = (_pafHdr.maxAudioFrameBlocksCount - 1) * _pafHdr.readBufferSize;
	} else {
		_demuxAudioFrameBlocks = 0;
		_flushAudioSize = 0;
	}
	_audioBufferOffsetRd = 0;
	_audioBufferOffsetWr = 0;
	_audioQueue = _audioQueueTail = 0;
}

void PafPlayer::play(int num) {
	debug(kDebug_PAF, "play %d", num);
	if (_videoNum != num) {
		preload(num);
	}
	if (_videoNum == num) {
		_playedMask |= 1 << num;
		mainLoop();
	}
}

void PafPlayer::unload(int num) {
	if (_videoNum < 0) {
		return;
	}
	for (int i = 0; i < 4; ++i) {
		free(_pageBuffers[i]);
		_pageBuffers[i] = 0;
	}
	free(_demuxVideoFrameBlocks);
	_demuxVideoFrameBlocks = 0;
	free(_demuxAudioFrameBlocks);
	_demuxAudioFrameBlocks = 0;
	free(_pafHdr.frameBlocksCountTable);
	free(_pafHdr.framesOffsetTable);
	free(_pafHdr.frameBlocksOffsetTable);
	memset(&_pafHdr, 0, sizeof(_pafHdr));
	_videoNum = -1;
	while (_audioQueue) {
		PafAudioQueue *next = _audioQueue->next;
		free(_audioQueue->buffer);
		free(_audioQueue);
		_audioQueue = next;
	}
	_audioQueueTail = 0;
}

void PafPlayer::readPafHeader() {
	static const char *kSignature = "Packed Animation File V1.0\n(c) 1992-96 Amazing Studio\n";
	_file.read(_bufferBlock, kBufferBlockSize);
	if (memcmp(_bufferBlock, kSignature, strlen(kSignature)) != 0) {
		return;
	}
	_pafHdr.startOffset = READ_LE_UINT32(_bufferBlock + 0xA4);
	_pafHdr.preloadFrameBlocksCount = READ_LE_UINT32(_bufferBlock + 0x9C);
	_pafHdr.readBufferSize = READ_LE_UINT32(_bufferBlock + 0x98);
	assert(_pafHdr.readBufferSize == kBufferBlockSize);
	_pafHdr.framesCount = READ_LE_UINT32(_bufferBlock + 0x84);
	_pafHdr.maxVideoFrameBlocksCount = READ_LE_UINT32(_bufferBlock + 0xA8);
	_pafHdr.maxAudioFrameBlocksCount = READ_LE_UINT32(_bufferBlock + 0xAC);
	_pafHdr.frameBlocksCount = READ_LE_UINT32(_bufferBlock + 0xA0);
	_pafHdr.frameBlocksCountTable = (uint32_t *)malloc(_pafHdr.framesCount * sizeof(uint32_t));
	readPafHeaderTable(_pafHdr.frameBlocksCountTable, _pafHdr.framesCount);
	_pafHdr.framesOffsetTable = (uint32_t *)malloc(_pafHdr.framesCount * sizeof(uint32_t));
	readPafHeaderTable(_pafHdr.framesOffsetTable, _pafHdr.framesCount);
	_pafHdr.frameBlocksOffsetTable = (uint32_t *)malloc(_pafHdr.frameBlocksCount * sizeof(uint32_t));
	readPafHeaderTable(_pafHdr.frameBlocksOffsetTable, _pafHdr.frameBlocksCount);
}

void PafPlayer::readPafHeaderTable(uint32_t *dst, int count) {
	for (int i = 0; i < count; ++i) {
		dst[i] = _file.readUint32();
	}
	const int align = (count * 4) & 0x7FF;
	if (align != 0) {
		_file.seek(0x800 - align, SEEK_CUR);
	}
}

void PafPlayer::decodeVideoFrame(const uint8_t *src) {
	const uint8_t *base = src;
	const int code = *src++;
	if (code & 0x20) {
		for (int i = 0; i < 4; ++i) {
			memset(_pageBuffers[i], 0, kPageBufferSize);
		}
		memset(_paletteBuffer, 0, sizeof(_paletteBuffer));
		_currentPageBuffer = 0;
	}
	if (code & 0x40) {
		int index = src[0];
		int count = (src[1] + 1) * 3;
		assert(index * 3 + count <= 768);
		src += 2;
		memcpy(_paletteBuffer + index * 3, src, count);
		src += count;
	}
	switch (code & 0xF) {
	case 0:
		decodeVideoFrameOp0(base, src, code);
		break;
	case 1:
		decodeVideoFrameOp1(src);
		break;
	case 2:
		decodeVideoFrameOp2(src);
		break;
	case 4:
		decodeVideoFrameOp4(src);
		break;
	}
}

static void pafCopy4x4h(uint8_t *dst, const uint8_t *src) {
	for (int i = 0; i < 4; ++i) {
		memcpy(dst, src, 4);
		src += 4;
		dst += 256;
	}
}

static void pafCopy4x4v(uint8_t *dst, const uint8_t *src) {
	for (int i = 0; i < 4; ++i) {
		memcpy(dst, src, 4);
		src += 256;
		dst += 256;
	}
}

static void pafCopySrcMask(uint8_t mask, uint8_t *dst, const uint8_t *src) {
	for (int i = 0; i < 4; ++i) {
		if (mask & (1 << (3 - i))) {
			dst[i] = src[i];
		}
	}
}

static void pafCopyColorMask(uint8_t mask, uint8_t *dst, uint8_t color) {
	for (int i = 0; i < 4; ++i) {
		if (mask & (1 << (3 - i))) {
			dst[i] = color;
		}
	}
}

static const char *updateSequences[] = {
	"",
	"\x02",
	"\x05\x07",
	"\x05",
	"\x06",
	"\x05\x07\x05\x07",
	"\x05\x07\x05",
	"\x05\x07\x06",
	"\x05\x05",
	"\x03",
	"\x06\x06",
	"\x02\x04",
	"\x02\x04\x05\x07",
	"\x02\x04\x05",
	"\x02\x04\x06",
	"\x02\x04\x05\x07\x05\x07"
};

uint8_t *PafPlayer::getVideoPageOffset(uint8_t a, uint8_t b) {
	const int x = b & 0x7F;
	const int y = ((a & 0x3F) << 1) | ((b >> 7) & 1);
	const int page = (a & 0xC0) >> 6;
	return _pageBuffers[page] + y * 2 * 256 + x * 2;
}

void PafPlayer::decodeVideoFrameOp0(const uint8_t *base, const uint8_t *src, uint8_t code) {
	int count = *src++;
	if (count != 0) {
		if ((code & 0x10) != 0) {
			int align = src - base;
			align &= 3;
			if (align != 0) {
				src += 4 - align;
			}
		}
		do {
			uint8_t *dst = getVideoPageOffset(src[0], src[1]);
			uint32_t offset = (src[1] & 0x7F) * 2;
			uint32_t end = READ_LE_UINT16(src + 2); src += 4;
			end += offset;
			do {
				++offset;
				pafCopy4x4h(dst, src);
				src += 16;
				if ((offset & 0x3F) == 0) {
					dst += 256 * 3;
				}
				dst += 4;
			} while (offset < end);
		} while (--count != 0);
	}

	uint8_t *dst = _pageBuffers[_currentPageBuffer];
	count = 0;
	do {
		const uint8_t *src2 = getVideoPageOffset(src[0], src[1]); src += 2;
		pafCopy4x4v(dst, src2);
		++count;
		if ((count & 0x3F) == 0) {
			dst += 256 * 3;
		}
		dst += 4;
	} while (count < 256 * 192 / 16);

	const uint32_t opcodesSize = READ_LE_UINT16(src); src += 4;

	const char *opcodes;
	const uint8_t *opcodesData = src;
	src += opcodesSize;

	uint8_t mask = 0;
	uint8_t color = 0;
	const uint8_t *src2 = 0;

	dst = _pageBuffers[_currentPageBuffer];
	for (int y = 0; y < 192; y += 4, dst += 256 * 3) {
		for (int x = 0; x < 256; x += 4, dst += 4) {
			if ((x & 4) == 0) {
				opcodes = updateSequences[*opcodesData >> 4];
			} else {
				opcodes = updateSequences[*opcodesData & 15];
				++opcodesData;
			}
			while (*opcodes) {
				uint32_t offset = 256 * 2;
				const int code = *opcodes++;
				switch (code) {
				case 2:
					offset = 0;
				case 3:
					color = *src++;
				case 4:
					mask = *src++;
					pafCopyColorMask(mask >> 4, dst + offset, color);
					offset += 256;
					pafCopyColorMask(mask & 15, dst + offset, color);
					break;
				case 5:
					offset = 0;
				case 6:
					src2 = getVideoPageOffset(src[0], src[1]); src += 2;
				case 7:
					mask = *src++;
					pafCopySrcMask(mask >> 4, dst + offset, src2 + offset);
					offset += 256;
					pafCopySrcMask(mask & 15, dst + offset, src2 + offset);
					break;
				}
			}
		}
	}
}

void PafPlayer::decodeVideoFrameOp1(const uint8_t *src) {
	memcpy(_pageBuffers[_currentPageBuffer], src + 2, kVideoWidth * kVideoHeight);
}

void PafPlayer::decodeVideoFrameOp2(const uint8_t *src) {
	int page = *src++;
	if (page != _currentPageBuffer) {
		memcpy(_pageBuffers[_currentPageBuffer], _pageBuffers[page], kVideoWidth * kVideoHeight);
	}
}

void PafPlayer::decodeVideoFrameOp4(const uint8_t *src) {
	uint8_t *dst = _pageBuffers[_currentPageBuffer];
	src += 2;
	int size = kVideoWidth * kVideoHeight;
	while (size != 0) {
		int8_t code = *src++;
		int count;
		if (code < 0) {
			count = 1 - code;
			const uint8_t color = *src++;
			memset(dst, color, count);
		} else {
			count = code + 1;
			memcpy(dst, src, count);
			src += count;
		}
		dst += count;
		size -= count;
	}
}

void PafPlayer::decodeAudioFrame(const uint8_t *src, uint32_t offset, uint32_t size) {
	assert(size == _pafHdr.readBufferSize);

	// copy must be sequential
	assert(offset == _audioBufferOffsetWr);

	_audioBufferOffsetWr = offset + size;

	const int count = (_audioBufferOffsetWr - _audioBufferOffsetRd) / kAudioStrideSize;
	if (count != 0) {
		PafAudioQueue *sq = (PafAudioQueue *)malloc(sizeof(PafAudioQueue));
		if (sq) {
			sq->offset = 0;
			sq->size = count * kAudioSamples * 2;
			sq->buffer = (int16_t *)calloc(sq->size, sizeof(int16_t));
			if (sq->buffer) {
				for (int i = 0; i < count; ++i) {
					decodeAudioFrame2205(src + _audioBufferOffsetRd + i * kAudioStrideSize, sq->buffer + i * kAudioSamples * 2);
				}
			}
			sq->next = 0;

			_system->lockAudio();
			if (_audioQueueTail) {
				_audioQueueTail->next = sq;
			} else {
				assert(!_audioQueue);
				_audioQueue = sq;
			}
			_audioQueueTail = sq;
			_system->unlockAudio();
		}
	}
	_audioBufferOffsetRd += count * kAudioStrideSize;
	if (_audioBufferOffsetWr == _flushAudioSize) {
		_audioBufferOffsetWr = 0;
		_audioBufferOffsetRd = 0;
	}
}

void PafPlayer::decodeAudioFrame2205(const uint8_t *src, int16_t *dst) {

	const uint8_t *samples = src;
	src += 256 * sizeof(int16_t);

	for (int i = 0; i < kAudioSamples; ++i) {
		for (int channel = 0; channel < 2; ++channel) {
			const uint8_t num = *src++;
			const int16_t pcm = READ_LE_UINT16(samples + num * sizeof(int16_t));
			*dst++ = pcm;
		}
	}
}

void PafPlayer::mix(int16_t *buf, int samples) {
	while (_audioQueue && samples > 0) {
		assert(_audioQueue->size != 0);
		*buf++ = _audioQueue->buffer[_audioQueue->offset++];
		*buf++ = _audioQueue->buffer[_audioQueue->offset++];
		samples -= 2;
		if (_audioQueue->offset >= _audioQueue->size) {
			assert(_audioQueue->offset == _audioQueue->size);
			PafAudioQueue *next = _audioQueue->next;
			free(_audioQueue->buffer);
			free(_audioQueue);
			_audioQueue = next;
		}
	}
	if (!_audioQueue) {
		_audioQueueTail = 0;
	}
	if (samples > 0) {
		warning("PafPlayer::mix() soundQueue underrun %d", samples);
	}
}

static void mixAudio(void *userdata, int16_t *buf, int len) {
	((PafPlayer *)userdata)->mix(buf, len);
}

void PafPlayer::mainLoop() {
	assert(_pafHdr.readBufferSize == 2048);
	_file.seek(_videoOffset + _pafHdr.startOffset, SEEK_SET);
	for (int i = 0; i < 4; ++i) {
		memset(_pageBuffers[i], 0, kVideoWidth * kVideoHeight);
	}
	memset(_paletteBuffer, 0, sizeof(_paletteBuffer));
	_currentPageBuffer = 0;
	int currentFrameBlock = 0;

	AudioCallback audioCb;
	audioCb.proc = mixAudio;
	audioCb.userdata = this;
	AudioCallback prevAudioCb = _system->setAudioCallback(audioCb);

	uint32_t frameTime = _system->getTimeStamp() + 1000 / kFramesPerSec;

	for (int i = 0; i < (int)_pafHdr.framesCount; ++i) {
		// read buffering blocks
		uint32_t blocksCountForFrame = (i == 0) ? _pafHdr.preloadFrameBlocksCount : _pafHdr.frameBlocksCountTable[i - 1];
		while (blocksCountForFrame != 0) {
			_file.read(_bufferBlock, _pafHdr.readBufferSize);
			const uint32_t dstOffset = _pafHdr.frameBlocksOffsetTable[currentFrameBlock] & ~(1 << 31);
			if (_pafHdr.frameBlocksOffsetTable[currentFrameBlock] & (1 << 31)) {
				assert(dstOffset + _pafHdr.readBufferSize <= _pafHdr.maxAudioFrameBlocksCount * _pafHdr.readBufferSize);
				memcpy(_demuxAudioFrameBlocks + dstOffset, _bufferBlock, _pafHdr.readBufferSize);
				decodeAudioFrame(_demuxAudioFrameBlocks, dstOffset, _pafHdr.readBufferSize);
			} else {
				assert(dstOffset + _pafHdr.readBufferSize <= _pafHdr.maxVideoFrameBlocksCount * _pafHdr.readBufferSize);
				memcpy(_demuxVideoFrameBlocks + dstOffset, _bufferBlock, _pafHdr.readBufferSize);
			}
			++currentFrameBlock;
			--blocksCountForFrame;
		}
		// decode video data
		decodeVideoFrame(_demuxVideoFrameBlocks + _pafHdr.framesOffsetTable[i]);
		_system->setPalette(_paletteBuffer, 256, 6);
		_system->copyRect(0, 0, kVideoWidth, kVideoHeight, _pageBuffers[_currentPageBuffer], 256);
		_system->updateScreen(false);
		_system->processEvents();
		if (_system->inp.quit || _system->inp.keyPressed(SYS_INP_ESC)) {
			break;
		}

		const int delay = MAX(10, int(frameTime - _system->getTimeStamp()));
		_system->sleep(delay);
		frameTime = _system->getTimeStamp() + 1000 / kFramesPerSec;

		// set next decoding video page
		++_currentPageBuffer;
		_currentPageBuffer &= 3;
	}
	unload();
	// restore audio callback
	_system->setAudioCallback(prevAudioCb);
}
