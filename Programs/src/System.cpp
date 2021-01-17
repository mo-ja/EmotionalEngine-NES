#pragma once
#include <cassert>
#include <cstdlib>
#include "System.h"

namespace nes { namespace detail {
	uint8_t CpuBus::ReadByte(uint16_t addr)
	{
		// WRAM ����̓ǂݏo��
		if (addr < PPU_REG_BASE)
		{
			// Mirror �Ή��̂��� WRAM SIZE �ł��܂���Ƃ�
			size_t idx = addr % WRAM_SIZE;
			return m_pSystem->m_Wram[idx];
		}
		else if (addr < APU_IO_REG_BASE)
		{
			size_t offset = addr - PPU_REG_BASE;
			size_t idx = offset % PPU_REG_SIZE;

			switch (idx)
			{
			case 2:
				return m_pPpu->ReadPpuStatus();
			case 7:
				return m_pPpu->ReadPpuData();
			default:
				// unexpected
				abort();
				break;
			}
		}
		else if (addr < CASSETTE_BASE) 
		{
			size_t idx = addr - APU_IO_REG_BASE;
			// TODO: PAD, APU ����
			return m_pSystem->m_IoReg[idx];
		}
		else 
		{
			// TORIAEZU: �J�Z�b�g�̊g�� ROM RAM �͋C�ɂ��Ȃ�
			assert(addr >= CASSETTE_PRG_ROM_BASE);

			int offset = addr - CASSETTE_PRG_ROM_BASE;
			uint8_t ret;
			// CPU ���猩����̂� PRG ROM �̂�
			m_pSystem->m_Cassette.ReadPrgRom(&ret, offset, 1);
			return ret;
		}
	}

	void CpuBus::WriteByte(uint16_t addr, uint8_t data)
	{
		// WRAM ����̓ǂݏo��
		if (addr < PPU_REG_BASE)
		{
			// Mirror �Ή��̂��� WRAM SIZE �ł��܂���Ƃ�
			size_t idx = addr % WRAM_SIZE;
			m_pSystem->m_Wram[idx] = data;
		}
		else if (addr < APU_IO_REG_BASE)
		{
			size_t offset = addr - PPU_REG_BASE;
			size_t idx = offset % PPU_REG_SIZE;
			switch (idx)
			{
			case 0:
				m_pPpu->WritePpuCtrl(data);
				break;
			case 1:
				m_pPpu->WritePpuMask(data);
				break;
			case 3:
				m_pPpu->WriteOamAddr(data);
				break;
			case 4:
				m_pPpu->WriteOamData(data);
				break;
			case 5:
				m_pPpu->WritePpuScroll(data);
				break;
			case 6:
				m_pPpu->WritePpuAddr(data);
				break;
			case 7:
				m_pPpu->WritePpuData(data);
				break;
			default:
				// unexpected
				abort();
				break;
			}
		}
		else if (addr < CASSETTE_BASE)
		{
			size_t idx = addr - APU_IO_REG_BASE;
			// TODO: PAD, APU ����
			m_pSystem->m_IoReg[idx] = data;
		}
		else
		{
			// TORIAEZU: �J�Z�b�g�̊g�� ROM RAM �͋C�ɂ��Ȃ�
			assert(addr >= CASSETTE_PRG_ROM_BASE);

			int offset = addr - CASSETTE_PRG_ROM_BASE;
			// CPU ���猩����̂� PRG ROM �̂�
			m_pSystem->m_Cassette.WritePrgRom(&data, offset, 1);
		}
	}

	uint8_t PpuBus::ReadByte(uint16_t addr)
	{
		// CHR ROM ����̓ǂݏo��
		if (addr < NAMETABLE_BASE)
		{
			uint8_t ret = 0;
			m_pSystem->m_Cassette.ReadChrRom(&ret, addr, 1);
			return ret;
		}
		else if (addr < PALETTE_BASE)
		{
			// nametable �ǂݏo��
			size_t offset = addr - NAMETABLE_BASE;
			// mirror
			size_t idx = offset % NAMETABLE_SIZE;
			return m_pPpuSystem->m_NameTable[idx];
		}
		else
		{
			// palette �ǂݏo��
			size_t offset = addr - PALETTE_BASE;
			size_t idx = offset % PALETTE_SIZE;

			return m_pPpuSystem->m_Pallettes[idx];
		}
	}

	uint8_t PpuBus::ReadByte(uint16_t addr, bool isPpuBuffering)
	{
		if (!isPpuBuffering)
		{
			return ReadByte(addr);
		}

		if (addr >= PALETTE_BASE)
		{
			// �p���b�g�̈�̑���� nametable �ǂݏo��
			size_t offset = addr - NAMETABLE_BASE;
			// mirror
			size_t idx = offset % NAMETABLE_SIZE;
			return m_pPpuSystem->m_NameTable[idx];
		}
		else
		{
			return ReadByte(addr);
		}
	}

	void PpuBus::WriteByte(uint16_t addr, uint8_t data)
	{
		// CHR ROM ��������(�H)
		if (addr < NAMETABLE_BASE)
		{
			m_pSystem->m_Cassette.WriteChrRom(&data, addr, 1);
		}
		else if (addr < PALETTE_BASE)
		{
			// nametable ��������
			size_t offset = addr - NAMETABLE_BASE;
			// mirror
			size_t idx = offset % NAMETABLE_SIZE;
			m_pPpuSystem->m_NameTable[idx] = data;
		}
		else
		{
			// palette ��������
			size_t offset = addr - PALETTE_BASE;
			size_t idx = offset % PALETTE_SIZE;

			m_pPpuSystem->m_Pallettes[idx] = data;
		}
	}
}}