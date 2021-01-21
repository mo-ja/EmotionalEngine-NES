#pragma once
#include <stdint.h>
#include "cassette.h"
#include "constants.h"
#include "Ppu.h"

namespace nes { namespace detail {
	class CpuBus;
	class PpuBus;
	class Ppu;

	// PPU �̃��������
	class PpuSystem {
		friend PpuBus;
	public:
		PpuSystem()
			:m_NameTable{}
			, m_Pallettes{}
		{}
	private:
		// 0x0000 - 0x1FFF: CHR-ROM(System �̃J�Z�b�g���Q�Ƃ���)
		// 0x2000 - 0x2FFF: Nametable
		uint8_t m_NameTable[NAMETABLE_SIZE];
		// 0x3000 - 0x3EFF: 0x2000-0x2EFF �̃~���[(0x2FFF �܂łł͂Ȃ����Ƃɒ��ӂ���)
		// 0x3F00 - 0x3F1F: Palette
		uint8_t m_Pallettes[PALETTE_SIZE];
		// 0x3F20 - 0x3FFF: 0x3F00 - 0x3FFF �̃~���[
	};

	class System {
		friend detail::CpuBus;
		friend detail::PpuBus;
	public:
		// TORIAEZU: �J�Z�b�g�̓��e�̓R���X�g���N�^�Ŏ󂯎��
		System(uint8_t* pBuffer, size_t bufferSize)
			:m_Wram{}
			,m_IoReg{}
		{
			m_Cassette.Initialize(pBuffer, bufferSize);
		}

		// TODO: PAD ���������֐������J����
		// TODO: VRAM �̒��g���擾����֐������J����
	private:
		// 0x0000 - 0x07FF: WRAM
		// 0x0800 - 0x1FFF: WRAM Mirror * 3 
		uint8_t m_Wram[WRAM_SIZE];
		// 0x2000 - 0x2007: PPU IO Register
		// 0x2009 - 0x3FFF: PPU IO Register Mirror * 1023
		// PPU ���W�X�^�� CpuBus ��ʂ��Ē��� PPU �̃��W�X�^��ǂݏ������邱�Ƃɂ���

		// 0x4000 - 0x401F: APU IO, PAD
		uint8_t m_IoReg[APU_IO_REG_SIZE];

		detail::Cassette m_Cassette;
	};

	// System �ւ̃|�C���^�������ACPU ���猩���郁������ԂɊ�Â��ăA�N�Z�X����N���X
	class CpuBus {
	public:
		CpuBus(System* pSystem, Ppu* pPpu) 
			:m_pSystem(pSystem)
			,m_pPpu(pPpu)
		{}
		uint8_t ReadByte(uint16_t addr);
		void WriteByte(uint16_t addr, uint8_t data);
	private:
		System* m_pSystem;
		Ppu* m_pPpu;
	};

	// PPU ���猩���郁������ԂɊ�Â��ăA�N�Z�X����N���X�APPU <-> �J�Z�b�g�A VRAM �ւ̃o�X
	// CPU -> PPU �� CpuBus �Ɏ�������
	// TODO: �{���� Cassette �����ւ̈ˑ��ł����̂� System �S�̂ւ̈ˑ��ɂȂ��Ă��܂��Ă���̂𒼂�
	class PpuBus {
	public:
		PpuBus(System* pSystem, PpuSystem* pPpuSystem)
			:m_pSystem(pSystem)
			,m_pPpuSystem(pPpuSystem)
		{}
		uint8_t ReadByte(uint16_t addr);
		// �p���b�g�e�[�u���� "��"�ɂ��� nametable�̃~���[�� PPU �̓����o�b�t�@�ɓǂ܂��̂ł���ɑΉ�����
		uint8_t ReadByte(uint16_t addr, bool isPpuBuffering);
		void WriteByte(uint16_t addr, uint8_t data);
	private:
		System* m_pSystem;
		PpuSystem* m_pPpuSystem;
	};

}}