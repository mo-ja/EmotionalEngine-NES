#pragma once
#include <stdint.h>
#include "cassette.h"
#include "constants.h"

namespace nes {
	namespace detail {
		class CpuBus;
	}

	// �V�X�e���S�̂������J�N���X
	class System {
		friend detail::CpuBus;
	public:
		// TORIAEZU: �J�Z�b�g�̓��e�̓R���X�g���N�^�Ŏ󂯎��
		System(uint8_t* pBuffer, size_t bufferSize)
			:m_Wram{}
			,m_PpuReg{}
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
		uint8_t m_PpuReg[PPU_REG_SIZE];
		// 0x4000 - 0x401F: APU IO, PAD
		uint8_t m_IoReg[APU_IO_REG_SIZE];

		detail::Cassette m_Cassette;

	};

	namespace detail {
		// System �ւ̃|�C���^�������ACPU ���猩���郁������ԂɊ�Â��ăA�N�Z�X����N���X
		class CpuBus {
		public:
			CpuBus(System* pSystem) 
				:m_pSystem(pSystem)
			{}
			uint8_t ReadByte(uint16_t addr);
			void WriteByte(uint16_t addr, uint8_t data);
		private:
			System* m_pSystem;
		};
	}
}