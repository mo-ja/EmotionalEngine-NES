#pragma once
#include <utility>
#include <stdint.h>
#include "constants.h"
#include "System.h"

namespace nes { namespace detail {
	class PpuBus;
	class CpuBus;

	struct Sprite
	{
		Sprite()
			: y(0)
			, patternTableIdx(0)
			, attribute(0)
			, x(0)
		{}

		Sprite(uint8_t aY, uint8_t aPatternTableIdx, uint8_t aAttribute, uint8_t aX)
			: y(aY)
			, patternTableIdx(aPatternTableIdx)
			, attribute(aAttribute)
			, x(aX)
		{}

		uint8_t y;
		uint8_t patternTableIdx;
		uint8_t attribute;
		uint8_t x;
	};

	enum class PpuInternalRegistertarget
	{
		PpuInternalRegistertarget_v,
		PpuInternalRegistertarget_t
	};

	// PPU �������W�X�^�ւ̃A�N�Z�X��񋟂���N���X
	class PpuInternalRegister
	{
	public:
		PpuInternalRegister()
			: v(0)
			, t(0)
			, x(0)
			, w(false)
		{}

		void SetCoarseX(PpuInternalRegistertarget target, uint8_t data);
		void SetCoarseY(PpuInternalRegistertarget target, uint8_t data);
		void SetNametableSelect(PpuInternalRegistertarget target, uint8_t data);
		void SetFineY(PpuInternalRegistertarget target, uint8_t data);
		void SetFineX(uint8_t data);

		uint8_t GetCoarseX(PpuInternalRegistertarget target);
		uint8_t GetCoarseY(PpuInternalRegistertarget target);
		uint8_t GetNametableSelect(PpuInternalRegistertarget target);
		uint8_t GetFineY(PpuInternalRegistertarget target);
		uint8_t GetFineX();

		// �`�撆�̃C���N�������g
		void IncrementCoarseX();
		void IncrementY();

		// ���݂̃^�C���� attribute table �̃A�h���X�擾
		uint16_t GetTileAddress();
		uint16_t GetAttributeAddress();

	private:
		// v, t : 15 bit
		uint16_t v;
		uint16_t t;
		// x: 3 bit
		uint8_t x;
		bool w;

		// constants
		const uint16_t NAMETABLE_SELECT_MASK = 0b000110000000000;
		const uint16_t COARSE_X_MASK		 = 0b000000000011111;
		const uint16_t COARSE_Y_MASK		 = 0b000001111100000;
		const uint16_t FINE_Y_MASK			 = 0b111000000000000;
	};

	enum class SpriteSize 
	{
		SpriteSize_8x8,
		SpriteSize_8x16,
	};

	class Ppu {
		// CPU �o�X����̏������݂�����
		friend CpuBus;
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

		// �N���b�N��^���Ă��̃N���b�N���� PPU ��i�߂�A1��ʕ����������� true ���Ԃ�
		bool Run(int clk);

		// ���W���w�肵�ăe�[�u���������Ĕw�i�F�Ɠ������ۂ�(���� = true)���擾����A�e�X�g�p�Ɍ��J���Ă���
		std::pair<uint8_t, bool> GetBackGroundPixelColor(int y, int x);

		// �X�v���C�g�̍�������_�Ƃ������W���w�肵�ăe�[�u���������ăX�v���C�g�̐F�Ɠ������ۂ�(���� = true)���擾����A�e�X�g�p�Ɍ��J���Ă���
		std::pair<uint8_t, bool> GetSpritePixelColor(Sprite sprite, int relativeY, int relativeX);

		// PPU �̊G���o�b�t�@�ɂ�������
		void GetPpuOutput(uint8_t pOutBuffer[PPU_OUTPUT_Y][PPU_OUTPUT_X]);

		// PPU�̏����擾
		void GetPpuInfo(int* pLines, int* pCycles);

		Ppu(PpuBus* pPpuBus)
			: PPUCTRL(0)
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
			, m_VramAddr(0)
			, m_IsVerticalScrollVal(false)
			, m_ScrollX(0)
			, m_ScrollY(0)
			, m_Lines(0)
			, m_Cycles(0)
			, m_Oam{}
			, m_PpuOutput{ {} }
			, m_IsBackgroundClear{ {} }
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

		// �`��p�������W�X�^(https://wiki.nesdev.com/w/index.php/PPU_scrolling#PPU_internal_registers)
		PpuInternalRegister m_InternalReg;

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

		int m_Lines;
		int m_Cycles;
		
		// �R���g���[�����W�X�^ �ǂݏ����n
		uint16_t GetVramOffset();
		void SetVBlankFlag(bool flag);

		//�@�w�i�� 1 Line ���`�悷��
		void BuildBackGroundLine();

		// �X�v���C�g ��S���`�悷��
		void BuildSprites();

		// �p�^�[���e�[�u���̃x�[�X�A�h���X���擾
		uint16_t GetBGPatternTableBase();
		uint16_t GetSpritePatternTableBase();

		// �X�v���C�g�T�C�Y���擾
		SpriteSize GetSpriteSize();

		// �C���f�b�N�X���w�肵�� OAM ����X�v���C�g����擾����
		Sprite GetSprite(int idx);

		// Sprite 0 hit ���Ă邩�H
		bool IsSprite0Hit(int y, int x);

		// PPU �� 256 byte �� Object Attribute Memory(Sprite ���������ޏꏊ)������
		uint8_t m_Oam[OAM_SIZE];

		// PPU �̏o��(�G)�B Ppu �Ɏ�������̂��K�؂��኱���������ǂƂ肠��������
		uint8_t m_PpuOutput[PPU_OUTPUT_Y][PPU_OUTPUT_X];

		// �w�i�������s�N�Z�����H
		bool m_IsBackgroundClear[PPU_OUTPUT_Y][PPU_OUTPUT_X];
	};
}}