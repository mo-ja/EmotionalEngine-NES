#pragma once
#include <stdint.h>
#include "constants.h"
#include "System.h"

namespace nes { namespace detail {
	class PpuBus;
	class Ppu {
	public:
		// PPU ���W�X�^�͕s�v�c�ȕ���p���������񂠂�̂ŁA����������ł���悤�ɂ��ׂăA�N�Z�T�[�ŃA�N�Z�X���邱�Ƃɂ���
		void WritePpuCtrl(uint8_t data);
		void WritePpuMask(uint8_t data);
		void WriteOamAddr(uint8_t data);
		void WriteOamData(uint8_t data);
		void WritePpuScroll(uint8_t data);
		void WritePpuAddr(uint8_t data);
		void WritePpuData(uint8_t data);

		uint8_t ReadPpuStatus();
		uint8_t ReadPpuData();

		// ���W�X�^�ǂݏ��� �I
		void Run(int clk);

		Ppu(PpuBus* pPpuBus)
			:PPUCTRL(0)
			, PPUMASK(0)
			, PPUSTATUS(0)
			, OAMADDR(0)
			, PPUSCROLL(0)
			, PPUADDR(0)
			, PPUDATA(0)
			, OAMDMA(0)
			, m_VramReadBuf(0)
			, m_pPpuBus(pPpuBus)
			, m_IsLowerPpuAddr(false)
			, m_IsValidPpuAddr(false)
			,m_VramAddr(0)
			, m_IsVerticalScrollVal(false)
			,m_ScrollX(0)
			,m_ScrollY(0)
		{}

	private:
		// ��ɂ���ă��W�X�^�͖����K���ɏ]��Ȃ����Ƃɂ��܂�
		// 0x2000
		uint8_t PPUCTRL;
		// 0x2001
		uint8_t PPUMASK;
		// 0x2002
		uint8_t PPUSTATUS;
		// 0x2003
		uint8_t OAMADDR;
		// 0x2004 = OAMDATA �� OAM(Sprite RAM) �ɏ������̂ŕێ����Ȃ�
		// 0x2005
		uint8_t PPUSCROLL;
		// 0x2006
		uint8_t PPUADDR;
		// 0x2007
		uint8_t PPUDATA;
		// 0x4014
		uint8_t OAMDMA;

		uint8_t m_VramReadBuf;
		// PPU Bus �o�R�� VRAM ��ǂݏ�������
		PpuBus* m_pPpuBus;

		// 2�x�����p�t���O�V���[�Y
		// PPUADDR �͏�ʃo�C�g -> ���ʃo�C�g�̏��ɂ�������
		bool m_IsLowerPpuAddr;
		// PPUADDR �ւ�2�񏑂����݂��������Ă��邩�H
		bool m_IsValidPpuAddr;
		uint16_t m_VramAddr;
		// PPUSCROLL �͐����X�N���[���l -> �����X�N���[���l �̏��ɏ�������(NES on FPGA �� NesDev �Ō����Ă邱�Ƃ��t�����c�c)
		bool m_IsVerticalScrollVal;
		uint8_t m_ScrollX;
		uint8_t m_ScrollY;
		
		// �R���g���[�����W�X�^ �ǂݏ����n
		uint16_t GetVramOffset();
		void SetVBlankFlag(bool flag);

	};
}}