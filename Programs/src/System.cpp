#pragma once
#include <cassert>
#include "System.h"

namespace nes {
	namespace detail {
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
				// TODO: PPU ���W�X�^���L�̏���(�j��Ǐo���Ƃ�)
				return m_pSystem->m_PpuReg[idx];
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
				// TODO: PPU ���W�X�^���L�̏���(�j��Ǐo���Ƃ�)
				m_pSystem->m_PpuReg[idx] = data;
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
	}
}