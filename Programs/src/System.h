#pragma once
#include <stdint.h>
#include <Nes_Pad.h>
#include "cassette.h"
#include "constants.h"
#include "Cpu.h"
#include "Pad.h"
#include "Ppu.h"

namespace nes { namespace detail {
	class CpuBus;
	class PpuBus;
	class Ppu;
	class Cpu;


	// PPU �̃�������Ԃ���Q�Ƃ����A���O���኱��₱����(�Ƃ������~�X���Ă�)���A�v����� VRAM
	class PpuSystem {
		friend PpuBus;
	public:
		PpuSystem()
			:m_NameTable{}
			,m_Pallettes{}
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

		void PushButton(PadId id, PadButton button);
		void ReleaseButton(PadId id, PadButton button);

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

		// Pad: 0x4016, 0x4017
		Pad m_Pads[2];

		detail::Cassette m_Cassette;
	};

	// System �ւ̃|�C���^�������ACPU ���猩���郁������ԂɊ�Â��ăA�N�Z�X����N���X
	class CpuBus {
	public:
		CpuBus(System* pSystem, Ppu* pPpu)
			:m_pSystem(pSystem)
			,m_pPpu(pPpu)
			,m_IsDmaRunning(false)
			,m_DmaUpperSrcAddr(0)
		{}
		uint8_t ReadByte(uint16_t addr);
		void WriteByte(uint16_t addr, uint8_t data);

		// DMA �p API
		// System ��̃��������� PPU ��� OAM �� DMA ����B DMA �͊�{�I�� VBLANK ���ɍs���邱�Ƃɒ��ӂ���B(4.5Line �����炢��PPU���Ԃ�����邪�A����ɂ���� NMI �𔭐������˂��� sprite 0 hit �������˂��肷�邱�Ƃ͂Ȃ��B)

		// DMA ���s���Ă��������N���b�N����Ԃ��A Kick ����ĂȂ��ꍇ��0
		int RunDma(uint64_t cpuCycles);
		// �]������ �A�h���X�̏�ʃo�C�g��^����DMA �N���A���ʃo�C�g�� 00
		void KickDma(uint8_t upperSrcAddr);
	private:
		System* m_pSystem;
		Ppu* m_pPpu;
		// DMA �� CpuBus ���S��(���ۂ� HW �\���������Ȃ��Ă���͂��B)
		bool m_IsDmaRunning;
		uint8_t m_DmaUpperSrcAddr;
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
		// �I�u�W�F�N�g�������̏z�ˑ���؂邽�߁A Initialize �� Cpu �ւ̃|�C���^��n��
		void Initialize(Cpu* pCpu);
		// �`��I���̃^�C�~���O�ɍ��킹�� NMI ������
		void GenerateCpuInterrupt();
	private:
		// �J�Z�b�g�Ŏw�肳���~���[���@�Ɋ�Â��ă~���[���̃A�h���X���v�Z����
		uint16_t GetMirroredAddr(uint16_t addr);

		System* m_pSystem;
		PpuSystem* m_pPpuSystem;
		// CPU �Ƀo�X���q��
		Cpu* m_pCpu;
		bool m_IsInitialized;
	};

}}