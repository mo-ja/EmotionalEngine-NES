#pragma once
#include <memory>
#include <stdint.h>
#include <cassette.h>
#include <Cpu.h>
#include <Ppu.h>
#include <System.h>
#include <Nes_Pad.h>

namespace nes {
	struct Color
	{
		uint8_t Red;
		uint8_t Green;
		uint8_t Blue;
	};

	struct EmuInfo {
		// �G�ɋK��l��n���Ă���
		EmuInfo()
			:CpuInfo(0, 0, 0, 0, 0, 0, detail::Instruction(detail::Opcode::ADC, detail::AddressingMode::Immediate, 0, 0), 0, 0)
		{}

		detail::CpuInfo CpuInfo;
		int PpuLines;
		int PpuCycles;

		uint64_t CpuCycles;
		uint64_t CpuInstructionCount;
	};

	class Emulator {
	public:
		Emulator(std::shared_ptr<uint8_t[]> rom, size_t romSize)
			:m_Rom(rom)
			,m_System(m_Rom.get(), romSize)
			,m_PpuBus(&m_System, &m_PpuSystem)
			,m_Ppu(&m_PpuBus)
			,m_Apu(&m_ApuBus)
			,m_CpuBus(&m_System, &m_Ppu, &m_Apu)
			,m_Cpu(&m_CpuBus)
			,m_ClockCount(7)
			,m_InstructionCount(1)
		{
			// �R���X�g���N�^�œn���Əz�ˑ��ɂȂ��Ă��܂��̂ł������� Initialize �œn��
			m_PpuBus.Initialize(&m_Cpu);
			m_ApuBus.Initialize(&m_Cpu);
			// Reset ���荞��
			m_Cpu.Interrupt(nes::detail::InterruptType::RESET);
		}

		// �e�[�u���������� uint8_t ���� RGB �ɕϊ�
		Color GetColor(uint8_t src);
		// 1�t���[���i�߂�
		void StepFrame();
		// 1���ߐi�߂�A1 �t���[���������Ă��� true ���Ԃ�
		bool Step();
		// �e�[�u���ϊ��O�̊G���擾
		void GetPicture(uint8_t pBuffer[240][256]);
		// �e�[�u���ϊ���̊G���擾
		void GetPicture(Color pBuffer[240][256]);

		// TODO: RAM, VRAM ���Ƃ��悤�ɂ���
		// CPU �� PPU �̏����擾
		void GetEmuInfo(EmuInfo* pOutInfo);

		// Pad �̃{�^������
		void PushButton(PadId id, PadButton button);
		void ReleaseButton(PadId id, PadButton button);

	private:
		std::shared_ptr<uint8_t[]> m_Rom;
		detail::System m_System;
		detail::PpuSystem m_PpuSystem;
		detail::PpuBus m_PpuBus;
		detail::Ppu m_Ppu;
		detail::ApuBus m_ApuBus;
		detail::Apu m_Apu;
		detail::CpuBus m_CpuBus;
		detail::Cpu m_Cpu;
		uint64_t m_ClockCount;
		uint64_t m_InstructionCount;
	};

}