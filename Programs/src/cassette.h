#pragma once
#include <cstddef>
#include <stdint.h>
#include "constants.h"

namespace nes { namespace detail {
	const int PRG_ROM_UNIT = 16 * 1024;
	const int CHR_ROM_UNIT = 8 * 1024;

	enum class Mirroring
	{
		Mirroring_Horizontal,
		Mirroring_Vertical
		// TODO: �ς� Mirroring(4��ʑS���Ƃ�)
	};

	struct INESHeader
	{
		uint8_t m_Magic[4];
		// 16KiB �P�ʂ� PRG ROM �̃T�C�Y
		uint8_t m_PrgRomSize;
		// 8 KiB �P�ʂ� CHR ROM �̃T�C�Y
		uint8_t m_ChrRomSize;
		// TODO: �t���O�̒��g�𒲍�����
		uint8_t m_Flags6;
		uint8_t m_Flags7;
		uint8_t m_Flags8;
		uint8_t m_Flags9;
		uint8_t m_Flags10;

		uint8_t padding[5];
	};
	static_assert(sizeof(INESHeader) == 16, "sizeof(INESHeader) != 16");

	class Cassette {
	public:
		Cassette()
			:m_Initialized(false)
			,m_PrgRom{}
			,m_ChrRom{}
			,m_Header{}
			,m_PrgRomSize(0)
			,m_ChrRomSize(0)
		{}
		// ROM �̃o�C�i�����������񂾃o�b�t�@�Ƃ��̃T�C�Y�������ɂƂ��ď�����
		void Initialize(uint8_t* pBuffer, size_t bufferSize);
		void ReadPrgRom(uint8_t* pBuffer, int offset, size_t size);
		void WritePrgRom(const uint8_t* pBuffer, int offset, size_t size);
		void ReadChrRom(uint8_t* pBuffer, int offset, size_t size);
		void WriteChrRom(const uint8_t* pBuffer, int offset, size_t size);
		Mirroring GetMirroring();
	private:
		uint8_t m_PrgRom[PRG_ROM_MAX];
		uint8_t m_ChrRom[CHR_ROM_MAX];
		bool m_Initialized;
		INESHeader m_Header;
		size_t m_PrgRomSize;
		size_t m_ChrRomSize;
	};
}}