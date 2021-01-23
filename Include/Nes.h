#pragma once
#include <memory>
#include <stdint.h>
#include <cassette.h>
#include <Cpu.h>
#include <Ppu.h>
#include <System.h>

namespace nes {
	struct Color
	{
		uint8_t Red;
		uint8_t Green;
		uint8_t Blue;
	};

	class Emulator {
	public:
		Emulator(std::shared_ptr<uint8_t[]> rom, size_t romSize)
			:m_Rom(rom)
			,m_System(m_Rom.get(), romSize)
			,m_PpuBus(&m_System, &m_PpuSystem)
			,m_Ppu(&m_PpuBus)
			,m_CpuBus(&m_System, &m_Ppu)
			,m_Cpu(&m_CpuBus)
			,m_ClockCount(0)
			,m_InstructionCount(0)
		{}

		// �e�[�u���������� uint8_t ���� RGB �ɕϊ�
		Color GetColor(uint8_t src);
		// 1�t���[���i�߂�
		void StepFrame();
		// �e�[�u���ϊ��O�̊G���擾
		void GetPicture(uint8_t pBuffer[240][256]);
		// �e�[�u���ϊ���̊G���擾
		void GetPicture(Color pBuffer[240][256]);

		// TODO: System �Ƃ��̏�Ԃ����� API �𐮔�

	private:
		std::shared_ptr<uint8_t[]> m_Rom;
		detail::System m_System;
		detail::PpuSystem m_PpuSystem;
		detail::PpuBus m_PpuBus;
		detail::Ppu m_Ppu;
		detail::CpuBus m_CpuBus;
		detail::Cpu m_Cpu;
		uint64_t m_ClockCount;
		uint64_t m_InstructionCount;
	};

}