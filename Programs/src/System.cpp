#pragma once
#include <cassert>
#include <cstdlib>
#include "System.h"

namespace {
	uint16_t GetPaletteMirrorAddr(uint16_t addr)
	{
		if (addr == 0x3F10 || addr == 0x3F14 || addr == 0x3F18 || addr == 0x3F1C) {
			return addr - 0x10;
		}
		return addr;
	}
}

namespace nes { namespace detail {
	void System::PushButton(PadId id, PadButton button)
	{
		if (id == PadId::Zero)
		{
			m_Pads[0].PushButton(button);
		}
		if (id == PadId::One)
		{
			m_Pads[1].PushButton(button);
		}
	}
	void System::ReleaseButton(PadId id, PadButton button)
	{
		if (id == PadId::Zero)
		{
			m_Pads[0].ReleaseButton(button);
		}
		if (id == PadId::One)
		{
			m_Pads[1].ReleaseButton(button);
		}
	}


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
				exit(-1);
				break;
			}
		}
		else if (addr < CASSETTE_BASE) 
		{
			size_t idx = addr - APU_IO_REG_BASE;

			// APU
			if (addr == 0x4015) 
			{
				return m_pApu->ReadRegister4015();
			}
			// Pad
			else if (addr == 0x4016)
			{
				return m_pSystem->m_Pads[0].ReadPad();
			}
			else if (addr == 0x4017)
			{
				return m_pSystem->m_Pads[1].ReadPad();
			}

			// TORIAEZU: �������̃��W�X�^�ǂݏo���͎c���Ă���
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
				exit(-1);
				break;
			}
		}
		else if (addr < CASSETTE_BASE)
		{
			size_t idx = addr - APU_IO_REG_BASE;
			// TORIAEZU: �^���������݂͎c���Ă���(�Ӗ��Ȃ����ǁA�f�o�b�O�p���炢��)
			m_pSystem->m_IoReg[idx] = data;
			
			const int APU_CHANNEL_REG_MAX = 0x4013;
			if (addr <= APU_CHANNEL_REG_MAX) 
			{
				// APU ���W�X�^��������
				m_pApu->WriteRegister(data, addr);
			}

			// APU �R���g���[�����W�X�^
			if (addr == 0x4015 || addr == 0x4017) 
			{
				m_pApu->WriteRegister(data, addr);
			}

			// DMA
			if (addr == 0x4014)
			{
				KickDma(data);
			}

			// Pad
			if (addr == 0x4016)
			{
				// 4016 �̏������݂� Pad 0 1 �� setstrobe �����̂��������H ����: https://taotao54321.hatenablog.com/entry/2017/04/11/011850
				// 4017 �������݂� APU ����Ȃ̂ŁB
				m_pSystem->m_Pads[0].SetStrobe(static_cast<bool>(data & 1));
				m_pSystem->m_Pads[1].SetStrobe(static_cast<bool>(data & 1));
			}
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
			// mirror �v�Z
			uint16_t mirroredAddr = GetMirroredAddr(addr);

			// nametable �ǂݏo��
			size_t offset = mirroredAddr - NAMETABLE_BASE;

			return m_pPpuSystem->m_NameTable[offset];
		}
		else
		{
			addr = GetPaletteMirrorAddr(addr);

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
			uint16_t offset = addr - NAMETABLE_MIRROR_BASE;
			// nametable �̒���������ǂ������v�Z���Ă��� nametable mirror ���v�Z
			uint16_t nameTableAddr = NAMETABLE_BASE + offset;
			uint16_t nameTableMirroredAddr = GetMirroredAddr(nameTableAddr);

			size_t idx = nameTableMirroredAddr - NAMETABLE_BASE;
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
			// mirror �v�Z
			uint16_t mirroredAddr = GetMirroredAddr(addr);

			// nametable ��������
			size_t offset = mirroredAddr - NAMETABLE_BASE;
			m_pPpuSystem->m_NameTable[offset] = data;
		}
		else
		{
			addr = GetPaletteMirrorAddr(addr);

			// palette ��������
			size_t offset = addr - PALETTE_BASE;
			size_t idx = offset % PALETTE_SIZE;

			m_pPpuSystem->m_Pallettes[idx] = data;
		}
	}
	void PpuBus::GenerateCpuInterrupt()
	{
		assert(m_IsInitialized);
		m_pCpu->Interrupt(nes::detail::InterruptType::NMI);
	}

	void PpuBus::Initialize(Cpu* pCpu)
	{
		m_pCpu = pCpu;
		m_IsInitialized = true;
	}

	uint16_t PpuBus::GetMirroredAddr(uint16_t addr)
	{
		// nametable �ȊO�͈̔͂̃A�h���X���n���ꂽ��v���O���~���O�~�X
		assert(0x2000 <= addr && addr < 0x3000);

		Mirroring mirroring = m_pSystem->m_Cassette.GetMirroring();
		if (mirroring == Mirroring::Mirroring_Horizontal)
		{
			// �����~���[: [0x2000, 0x2400) �� [0x2400, 0x2800) �ɁA[0x2800, 0x2c00) ��[0x2c00, 0x3000) �Ƀ~���[�����
			// �~���[�͈͂�������G�Ɉ����Z�����邺
			if ((0x2400 <= addr && addr < 0x2800) ||
				(0x2C00 <= addr && addr < 0x3000))
			{
				addr -= 0x400;
			}
		}
		else if (mirroring == Mirroring::Mirroring_Vertical)
		{
			// �����~���[: [0x2000, 0x2800) �� [0x2800, 0x3000) �Ƀ~���[�����
			if (0x2800 <= addr)
			{
				addr -= 0x800;
			}
		}

		return addr;
	}

	int CpuBus::RunDma(uint64_t cpuCycles)
	{
		if (!m_IsDmaRunning) 
		{
			return 0;
		}
		assert(m_pPpu->OAMADDR == 0);
		int addr = m_DmaUpperSrcAddr << 8;

		for (int i = 0; i < OAM_SIZE; i++)
		{
			m_pPpu->m_Oam[i] = ReadByte(addr + i);
		}

		return  cpuCycles % 2 == 0 ? 513 : 514;
	}

	void CpuBus::KickDma(uint8_t upperSrcAddr)
	{
		m_DmaUpperSrcAddr = upperSrcAddr;
		m_IsDmaRunning = true;
	}

	void ApuBus::Initialize(Cpu* pCpu)
	{
		m_pCpu = pCpu;
		m_IsInitialized = true;
	}

	// �t���[���V�[�P���T�� CPU �� IRQ �����
	void ApuBus::GenerateCpuInterrupt()
	{
		assert(m_IsInitialized);
		m_pCpu->Interrupt(nes::detail::InterruptType::IRQ);
	}

	// APU ���� DMA �� CPU ��������Ԃ̒l��ǂݍ���
	uint8_t ApuBus::ReadByte(uint16_t addr)
	{
		assert(m_IsInitialized);
		// DMC �p DMA
		// $4012 �������ݎ��̋����� wrap �̋�������ADMA �œǂރA�h���X�� $8000 �ȍ~(�܂�A PRG ROM ������܂Ȃ�)�̂͂��A assert ���Ƃ�
		assert(addr >= CASSETTE_PRG_ROM_BASE);
		int offset = addr - CASSETTE_PRG_ROM_BASE;

		uint8_t res = 0;
		m_pSystem->m_Cassette.ReadPrgRom(&res, offset, 1);
		return res;
	}
}}