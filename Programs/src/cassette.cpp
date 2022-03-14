#include <cassert>
#include <cstdlib>
#include <string.h>

#include "./constants.h"
#include "./cassette.h"

namespace nes { namespace detail {
	void Cassette::Initialize(uint8_t* pBuffer, size_t bufferSize)
	{
		// iNES �w�b�_�[��ǂݍ���
		assert(bufferSize >= 16);
		memcpy(&m_Header, pBuffer, sizeof(INESHeader));

		// �}�W�b�N�i���o�[ �m�F
		assert(!memcmp(m_Header.m_Magic, "NES\x1A", 4));
		
		// TODO: �o�b�t�@�T�C�Y�̃`�F�b�N

		// PROGRAM ROM �؂�o��
		size_t prgRomSize = static_cast<size_t>(m_Header.m_PrgRomSize) * PRG_ROM_UNIT;
		memcpy(m_PrgRom, pBuffer + sizeof(INESHeader), prgRomSize);
		// CHARACTER ROM �؂�o��
		size_t chrRomSize = static_cast<size_t>(m_Header.m_ChrRomSize) * CHR_ROM_UNIT;
		memcpy(m_ChrRom, pBuffer + sizeof(INESHeader) + prgRomSize, chrRomSize);

		m_PrgRomSize = prgRomSize;
		m_ChrRomSize = chrRomSize;
	
		m_Initialized = true;
	}

	void Cassette::ReadPrgRom(uint8_t* pBuffer, int offset, size_t size)
	{
		assert(m_Initialized);
		const int BANK_SIZE = 0x4000;

		for (size_t i = 0; i < size; i++)
		{
			int addr = i + offset + CASSETTE_PRG_ROM_BASE;
			// ROM �� 16 KB �̏ꍇ�̃~���[�����O
			if (addr >= 0xC000) 
			{
				// 0xC000 �ȍ~�͏�ɍŏI�o���N
				size_t bankOffset = addr - 0xC000;
				pBuffer[i] = m_PrgRom[BANK_SIZE * 15 + bankOffset];
			}
			else
			{
				size_t bankOffset = addr - 0x8000;
				pBuffer[i] = m_PrgRom[BANK_SIZE * m_BankNum + bankOffset];
			}
		}
	}
	void Cassette::WritePrgRom(const uint8_t* pBuffer, int offset, size_t size)
	{
		assert(m_Initialized);
		assert(offset + size <= m_PrgRomSize);
		// memcpy(m_PrgRom + offset, pBuffer, size);
		// mapper 2 �ł̓o���N��؂�ւ���
		m_BankNum = *pBuffer;
	}
	void Cassette::ReadChrRom(uint8_t* pBuffer, int offset, size_t size)
	{
		assert(m_Initialized);
		// �K�v�Ȃ� mirror ���邱��(�K�v�Ȃ� assert ����������͂�)
		// assert(offset + size <= m_ChrRomSize);
		memcpy(pBuffer, m_ChrRom + offset, size);
	}
	void Cassette::WriteChrRom(const uint8_t* pBuffer, int offset, size_t size)
	{
		assert(m_Initialized);
		assert(offset + size <= m_ChrRomSize);
		memcpy(m_ChrRom + offset, pBuffer, size);
	}

	Mirroring Cassette::GetMirroring()
	{
		if (m_Header.m_Flags6 & 1)
		{
			return Mirroring::Mirroring_Vertical;
		}
		else
		{
			return Mirroring::Mirroring_Horizontal;
		}
	}

}}