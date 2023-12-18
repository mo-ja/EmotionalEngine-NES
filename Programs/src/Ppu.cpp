#pragma once
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <utility>
#include "Ppu.h"

namespace {
	// �e�[�u�����Ђ��Ђ�����w���p�[�֐�
	// tile �� id �� attribute table �̗v�f -> �p���b�g�ԍ�
	uint8_t GetPaletteId(int tileId, uint8_t attributeTableElement)
	{
		uint8_t higher = (tileId / 64) % 2 == 0 ? 0 : 1;
		uint8_t lower = (tileId / 2) % 2 == 0 ? 0 : 1;
		higher <<= 1;

		uint8_t paletteIdx = higher | lower;

		// attribute table ���� paletteIdx �Ԗڂ̒l�����o��
		uint8_t paletteId = attributeTableElement & (0b11 << (paletteIdx * 2));
		paletteId >>= (paletteIdx * 2);

		return paletteId;
	}
}

namespace nes { namespace detail {
	bool Ppu::Run(int clk)
	{
		int beforeCycles = m_Cycles;
		int beforeLines = m_Lines;
		
		// �N���b�N�̉��Z�� BG �`�悵�Ȃ���s��
		DrawBackGround(clk);

		// Line 241 �ɂ��Ă��� NMI ����
		if (m_Lines != beforeLines && m_Lines == 241)
		{
			// ��ʂ��������钼�O�� sprite �`��
			BuildSprites();
			// VBLANK �t���O���Ă�
			SetVBlankFlag(true);

			if (PPUCTRL & (1 << 7))
			{
				m_pPpuBus->GenerateCpuInterrupt();
			}
		}

		if (m_Lines < 240)
		{
			// �����C���̂Ƃ����� Sprite 0 hit �� 1px ���`�F�b�N����B[2, 257] cycle �ڂŔ��肷��B
			// ���܂݂Ă� line �� beforecycles - 2 �܂Ō��I����Ă�͂��Ȃ̂ŁA���̎��̃s�N�Z�����猩��
			// line ���܂����ł�ꍇ�� start �� 0 �ɂ���A m_Cycles �� 1�ȉ� �̂Ƃ� end �����ɂȂ邯�ǉ��Ȃ��͂�
			int start = beforeCycles + clk >= 341 ? 0 : std::max(0, beforeCycles - 2 + 1);
			int end = std::min(255, m_Cycles - 2);

			for (int x = start; x <= end; x++)
			{
				if (IsSprite0Hit(m_Lines, x))
				{
					PPUSTATUS |= PPUSTATUS_SPRITE_0_HIT;
				}
			}
		}
		// line 261�̐擪�� sprite 0 hit �t���O �� VBlank �t���O��܂�
		if (m_Lines == PPU_OUTPUT_Y + PPU_VBLANK_Y - 1)
		{
			PPUSTATUS &= ~PPUSTATUS_SPRITE_0_HIT;
			SetVBlankFlag(false);
		}

		if (m_Lines == PPU_OUTPUT_Y + PPU_VBLANK_Y)
		{
			m_Lines = 0;
			return true;
		}
		return false;
	}

	SpriteSize Ppu::GetSpriteSize()
	{
		return (PPUCTRL & (1 << 5)) ? SpriteSize::SpriteSize_8x16 : SpriteSize::SpriteSize_8x8;
	}

	Sprite Ppu::GetSprite(int idx)
	{
		int offset = idx * sizeof(Sprite);
		// OAM ����͂ݏo���Ȃ�
		assert(offset + 3 < static_cast<int>(OAM_SIZE));

		return Sprite(m_Oam[offset], m_Oam[offset + 1], m_Oam[offset + 2], m_Oam[offset + 3]);
	}

	// PPU Internal Register �Ɋ�Â��� BackGround �`��Ɋ�Â��� Sprite 0 hit
	// pre: ���Y Run �ɂ����āA DrawBackGround(clk) ��
	bool Ppu::IsSprite0Hit(int y, int x)
	{
		// Sprite 0 hit ���������Ȃ������ɓ��Ă͂܂��Ă���Ȃ瑁�� return �����Ⴄ
		bool enableClippingBg	  = (PPUMASK & PPUMASKS_ENABLE_BG_MASK)		== 0;
		bool enableClippingSprite = (PPUMASK & PPUMASKS_ENABLE_SPRITE_MASK) == 0;
		bool enableBg			  = (PPUMASK & PPUMASKS_ENABLE_BG)		    == PPUMASKS_ENABLE_BG;
		bool enableSprite		  = (PPUMASK & PPUMASKS_ENABLE_SPRITE)      == PPUMASKS_ENABLE_SPRITE;

		// �N���b�s���O�L�� or {�w�i or �X�v���C�g�`�斳��} �Ȃ�X�v���C�g0hit���Ȃ�
		if (enableClippingBg || enableClippingSprite || !enableBg || !enableSprite)
		{
			return false;
		}

		Sprite sprite0 = GetSprite(0);
		// OAM �Ɋi�[�����y���W�� -1 ����Ă�̂ő���
		sprite0.y++;

		// �X�v���C�g���̑��΍��W �v�Z
		int relativeY = y - sprite0.y;
		int relativeX = x - sprite0.x;

		// TORIAEZU: sprite size 8x8 �̂ݑΉ�
		assert(GetSpriteSize() == SpriteSize::SpriteSize_8x8);

		if (GetSpriteSize() == SpriteSize::SpriteSize_8x8)
		{
			// �͈͊O
			if (relativeX < 0 || relativeY < 0 || relativeX >= 8 || relativeY >= 8)
			{
				return false;
			}
			// �͈͓� �Ȃ� pattern table �����A����ق����̂� ���� or not �����Ȃ̂� second ��������
			bool isSpriteClear = GetSpritePixelColor(sprite0, relativeY, relativeX).second;
			// ���O����(DrawBackGround(clk) ��)�𖞂����Ă���΁A m_IsBackgroundClear[y][x] �ɂ͊��ɐ������l�������Ă���͂��Ȃ̂ŁA����ł悢�͂�
			bool isBgClear = m_IsBackgroundClear[y][x];

			// �����s�����Ȃ� hit
			return (!isSpriteClear && !isBgClear) ? true : false;
		}
		else 
		{
			return false;
		}
	}

	// �X�v���C�g�̍�������_�Ƃ������W���w�肵�ăe�[�u���������ăX�v���C�g�̐F�Ɠ������ۂ�(���� = true)���擾����A�e�X�g�p�Ɍ��J���Ă���
	std::pair<uint8_t, bool> Ppu::GetSpritePixelColor(Sprite sprite, int relativeY, int relativeX)
	{
		assert(relativeX < 8);
		assert(relativeY < 8);

		// ���]�t���O���m�F���ė����Ă��甽�]������
		bool invHorizontal = ((1 << 6) & sprite.attribute) == (1 << 6);
		bool invVertical   = ((1 << 7) & sprite.attribute) == (1 << 7);

		if (invHorizontal)
		{
			relativeX = 7 - relativeX;
		}
		if (invVertical)
		{
			relativeY = 7 - relativeY;
		}

		uint8_t patternTableLower = m_pPpuBus->ReadByte(GetSpritePatternTableBase() + sprite.patternTableIdx * static_cast<uint16_t>(PATTERN_TABLE_ELEMENT_SIZE) + relativeY);
		uint8_t patternTableUpper = m_pPpuBus->ReadByte(GetSpritePatternTableBase() + sprite.patternTableIdx * static_cast<uint16_t>(PATTERN_TABLE_ELEMENT_SIZE) + 8 + relativeY);

		int bitPos = 7 - relativeX;

		uint8_t colorLower = (patternTableLower & (1 << bitPos)) == (1 << bitPos);
		uint8_t colorUpper = (patternTableUpper & (1 << bitPos)) == (1 << bitPos);
		colorUpper <<= 1;

		uint8_t color = colorLower | colorUpper;
		assert(color <= 3);

		// �p���b�g�� id �� Sprite �������Ă�
		// �J���[�p���b�g�̏��2bit(http://pgate1.at-ninja.jp/NES_on_FPGA/nes_ppu.htm#sprite) �Ƃ����̂̈Ӗ����킩��Ȃ����Asprite palette base �� 0x3f10 �ŁA��������̃I�t�Z�b�g��4bit�Ŏw�肷��ƍl����Ƃ��܂������̂ł����l����
		// �o�O������^��(�ň�)
		uint16_t spritePaletteBase = PALETTE_BASE + SPRITE_PALETTE_OFFSET;
		// attribute �̉��� 2 bit �� sprite palette �� index
		uint16_t spritePaletteId = sprite.attribute & 0b11;
		// �e���œ�������ɂ��������� or �ɂ��Ƃ�(spritePaletteId �� sprite palette �̃C���f�b�N�X�Ƃ݂Ȃ��� + spritePaletteId * 4 �Ƃ��Ă��Ӗ��͂��Ȃ��͂�)
		uint16_t spritePaletteAddr = spritePaletteBase | (spritePaletteId << 2);
		// color �𑫂��Ď��ۂɓǂނׂ��A�h���X�ɂ���
		uint16_t colorAddr = spritePaletteAddr + color;

		uint8_t ret = m_pPpuBus->ReadByte(colorAddr);

		return std::make_pair(ret, color == 0);
	}

	// ��ʓ����W(�`����W)����͂��Ă��̃s�N�Z���̐F���擾����A���̐F���������ǂ������擾����
	// �����ɂ͎g���Ȃ����A�e�X�g�p�Ɏc���Ă���
	std::pair<uint8_t, bool> Ppu::GetBackGroundPixelColor(int y, int x)
	{
		return std::make_pair(m_PpuOutput[y][x], m_IsBackgroundClear[y][x]);
	}

	void Ppu::DrawBackGround(int clk)
	{
		// clk �N���b�N PPU �������߂�
		for (int i = 0; i < clk; i++)
		{
			// 1 cycle �� 1 px �`�悷�邱�Ƃɒ��ӂ���
			if (m_Cycles == 0)
			{
				// Line �̍ŏ��� Fine X Scroll ��K�p����
				m_BGRelativeX = m_InternalReg.GetFineX();
			}

			int x = m_Cycles;
			int y = m_Lines;

			// �`��͈͓��Ȃ�`�悷��
			if (y < PPU_OUTPUT_Y && x < PPU_OUTPUT_X)
			{
				uint16_t tileAddr		= m_InternalReg.GetTileAddress();
				uint16_t attributeAddr  = m_InternalReg.GetAttributeAddress();

				uint8_t attributeTable = m_pPpuBus->ReadByte(attributeAddr);

				// �g���p���b�g����肷��
				// ���� 10 bit �� tile id 
				uint16_t tileId = tileAddr & 0b1111111111;
				uint8_t paletteId = GetPaletteId(tileId, attributeTable);

				// tile ���ł̑��΍��W
				int RelativeX = m_BGRelativeX;
				int RelativeY = m_InternalReg.GetFineY(PpuInternalRegistertarget::PpuInternalRegistertarget_v);

				// nametable ����
				uint8_t spriteNum = m_pPpuBus->ReadByte(static_cast<uint16_t>(tileAddr));

				// pattern table ����
				uint8_t patternTableLower = m_pPpuBus->ReadByte(GetBGPatternTableBase() + spriteNum * static_cast<uint16_t>(PATTERN_TABLE_ELEMENT_SIZE) + RelativeY);
				uint8_t patternTableUpper = m_pPpuBus->ReadByte(GetBGPatternTableBase() + spriteNum * static_cast<uint16_t>(PATTERN_TABLE_ELEMENT_SIZE) + RelativeY + 8);

				int bitPos = 7 - RelativeX;

				uint8_t colorLower = (patternTableLower & (1 << bitPos)) == (1 << bitPos);
				uint8_t colorUpper = (patternTableUpper & (1 << bitPos)) == (1 << bitPos);
				colorUpper <<= 1;

				uint8_t color = colorLower | colorUpper;
				assert(color <= 3);

				// palette[paletteId][color] �����ۂɊG�Ƃ��Č����F�B color == 0 �̂Ƃ��͓����F
				const uint16_t PaletteSize = 4;
				uint8_t outColor = m_pPpuBus->ReadByte(PALETTE_BASE + PaletteSize * paletteId + color);

				m_PpuOutput[y][x] = outColor;
				m_IsBackgroundClear[y][x] = color == 0;

				uint8_t masterBg = m_pPpuBus->ReadByte(PALETTE_BASE);
				if (color == 0)
				{
					m_PpuOutput[y][x] = masterBg;
				}
			}

			// v �X�V
			if (x == 257)
			{
				m_InternalReg.UpdateHorizontalV();
			}

			if (m_Lines == 261 && 280 <= x && x <= 304)
			{
				m_InternalReg.UpdateVerticalV();
			}

			// �`����W�Ɋ�Â��ē������W�X�^���C���N�������g����
			// Line �� 256 dot �ڂɂ��Ă��� Y �C���N�������g
			if (x == 256)
			{
				m_InternalReg.IncrementY();
			}

			// X �̑��΍��W(fine X ����n�܂�z) �� 8 �ɂȂ��Ă����� Coarse X ���C���N�������g
			// Between dot 328 of a scanline, and 256 of the next scanline ???(https://wiki.nesdev.com/w/index.php/PPU_scrolling#Between_dot_328_of_a_scanline.2C_and_256_of_the_next_scanline)
			// ���E���₵������
			//if (x >= 328 || x < 256) 
			if (x < 256)
			{
				m_BGRelativeX++;
				if (m_BGRelativeX == 8)
				{
					m_BGRelativeX = 0;
					m_InternalReg.IncrementCoarseX();
				}
			}

			m_Cycles++;
			// PPU �T�C�N���� mod 341 �ŕێ�����(341 PPU cycles �� 1 Line �`�悳���̂�)
			if (m_Cycles >= 341)
			{
				m_Cycles %= 341;
				m_Lines++;
			}
		}
	}

	void Ppu::BuildSprites()
	{
		// OAM �ɕێ��ł���X�v���C�g�� 64 ��
		for (int i = 0; i < OAM_SIZE / sizeof(Sprite); i++)
		{
			Sprite sprite = GetSprite(i);

			int offsetY = sprite.y + 1;
			int offsetX = sprite.x;

			for (int ry = 0; ry < 8; ry++)
			{
				for (int rx = 0; rx < 8; rx++)
				{
					// �͂ݏo���Ă�Ȃ�Ȃ�����Ȃ�
					if (ry + offsetY >= PPU_OUTPUT_Y || rx + offsetX >= PPU_OUTPUT_X) 
					{
						continue;
					}

					auto [color, isClear] = GetSpritePixelColor(sprite, ry, rx);
					if (isClear) {
						// �����F�Ȃ�Ȃ�����Ȃ�
						continue;
					}

					bool isFront = (sprite.attribute & (1 << 5)) == 0;
					if (isFront)
					{
						// front: �ⓚ���p�ŕ`��
						m_PpuOutput[ry + offsetY][rx + offsetX] = color;
					}
					else if(m_IsBackgroundClear[ry + offsetY][rx + offsetX])
					{
						// back: �w�i�������Ȃ�`��
						m_PpuOutput[ry + offsetY][rx + offsetX] = color;
					}
				}
			}
		}
	}

	uint16_t Ppu::GetBGPatternTableBase()
	{
		// PPUCTRL[4] �Ŕw�i�p�^�[���e�[�u���A�h���X�𕪊򂷂�
		return (PPUCTRL & 1 << 4) ? 0x1000 : 0x0000;
	}
	uint16_t Ppu::GetSpritePatternTableBase()
	{
		// PPUCTRL[3] �ŃX�v���C�g�p�^�[���e�[�u���A�h���X�𕪊򂷂�
		return (PPUCTRL & 1 << 3) ? 0x1000 : 0x0000;
	}

	void Ppu::GetPpuOutput(uint8_t pOutBuffer[PPU_OUTPUT_Y][PPU_OUTPUT_X])
	{
		for (int y = 0; y < PPU_OUTPUT_Y; y++) {
			for (int x = 0; x < PPU_OUTPUT_X; x++) {
				pOutBuffer[y][x] = m_PpuOutput[y][x];
			}
		}
	}
	
	void Ppu::WritePpuCtrl(uint8_t data)
	{
		PPUCTRL = data;
		
		// �������W�X�^�� nametable select �ɔ��f(���̕� ���� https://wiki.nesdev.com/w/index.php/PPU_scrolling#Register_controls)
		uint8_t arg = data & 0b11;
		m_InternalReg.SetNametableSelect(PpuInternalRegistertarget::PpuInternalRegistertarget_t, arg);
	}
	void Ppu::WritePpuMask(uint8_t data)
	{
		PPUMASK = data;
	}
	void Ppu::WriteOamAddr(uint8_t data)
	{
		OAMADDR = data;
	}
	void Ppu::WriteOamData(uint8_t data)
	{
		// ���ʂ� DMA �����炵���̂ŁA���܂�@����Ȃ�����
		// OAMADDR �C���N�������g�̍���: http://pgate1.at-ninja.jp/NES_on_FPGA/nes_ppu.htm
		m_Oam[OAMADDR] = data;
		OAMADDR++;
	}
	void Ppu::WritePpuScroll(uint8_t data)
	{
		if (!m_InternalReg.GetW())
		{
			uint8_t coarseX = data >> 3;
			uint8_t fineX = data & 0b111;

			m_InternalReg.SetCoarseX(PpuInternalRegistertarget::PpuInternalRegistertarget_t, coarseX);
			m_InternalReg.SetFineX(fineX);
			m_InternalReg.SetW(true);
		}
		else
		{
			uint8_t coarseY = data >> 3;
			uint8_t fineY = data & 0b111;

			m_InternalReg.SetCoarseY(PpuInternalRegistertarget::PpuInternalRegistertarget_t, coarseY);
			m_InternalReg.SetFineY(PpuInternalRegistertarget::PpuInternalRegistertarget_t, fineY);
			m_InternalReg.SetW(false);
		}
	}
	void Ppu::WritePpuAddr(uint8_t data)
	{
		if (m_IsLowerPpuAddr)
		{
			m_VramAddr |= data;
			m_IsLowerPpuAddr = false;
			m_IsValidPpuAddr = true;
		}
		else
		{
			m_VramAddr = static_cast<uint16_t>(data);
			m_VramAddr <<= 8;
			m_IsLowerPpuAddr = true;
			m_IsValidPpuAddr = false;
		}

		// �������W�X�^�ɂ��ꉞ���f�����Ƃ�
		if (!m_InternalReg.GetW())
		{
			m_InternalReg.SetUpperPpuAddr(data);
		}
		else
		{
			m_InternalReg.SetLowerPpuAddr(data);
		}
	}
	void Ppu::WritePpuData(uint8_t data)
	{
		m_pPpuBus->WriteByte(m_VramAddr, data);
		m_VramAddr += GetVramOffset();
	}

	uint8_t Ppu::ReadPpuStatus()
	{
		// 2��ǂ݃t���O�����Z�b�g
		m_InternalReg.SetW(false);
		m_IsLowerPpuAddr = false;
		m_IsValidPpuAddr = false;

		// VBLANK �t���O �N���A����O�ɒl��ێ����Ă���
		uint8_t ret = PPUSTATUS;

		// VBlank �t���O���N���A
		SetVBlankFlag(false);

		return ret;
	}
	uint8_t Ppu::ReadPpuData()
	{
		uint8_t buffered = m_VramReadBuf;
		if (m_VramAddr >= PALETTE_BASE)
		{
			// �p���b�g�e�[�u���͑����ǂݏo���A "��"�ɂ���l�[���e�[�u���̃~���[���o�b�t�@�ɓ���
			m_VramReadBuf = m_pPpuBus->ReadByte(m_VramAddr, true);
			uint8_t ret = m_pPpuBus->ReadByte(m_VramAddr);
			m_VramAddr += GetVramOffset();

			return ret;
		}
		else
		{
			m_VramReadBuf = m_pPpuBus->ReadByte(m_VramAddr);
			m_VramAddr += GetVramOffset();
		}

		return buffered;
	}

	void Ppu::SetVBlankFlag(bool flag)
	{
		if (flag)
		{
			PPUSTATUS |= (1 << 7);
		}
		else
		{
			PPUSTATUS &= ~(1 << 7);
		}
	}

	uint16_t Ppu::GetVramOffset()
	{
		return (PPUCTRL & (1 << 2)) ? 32 : 1;
	}

	void Ppu::GetPpuInfo(int* pLines, int* pCycles)
	{
		*pLines = m_Lines;
		*pCycles = m_Cycles;
	}

	// PPU �������W�X�^�A�N�Z�T�[
	void PpuInternalRegister::SetCoarseX(PpuInternalRegistertarget target, uint8_t data)
	{
		// ���� 5 bit �̂ݗL��
		assert((0b11100000 & data) == 0);
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			t &= ~COARSE_X_MASK;
			t |= data;
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			v &= ~COARSE_X_MASK;
			v |= data;
		}
	}
	void PpuInternalRegister::SetCoarseY(PpuInternalRegistertarget target, uint8_t data)
	{
		// ���� 5 bit �̂ݗL��
		assert((0b11100000 & data) == 0);
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			t &= ~COARSE_Y_MASK;
			uint16_t writeData = static_cast<uint16_t>(data) << 5;
			t |= writeData;
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			v &= ~COARSE_Y_MASK;
			uint16_t writeData = static_cast<uint16_t>(data) << 5;
			v |= writeData;
		}
	}
	void PpuInternalRegister::SetNametableSelect(PpuInternalRegistertarget target, uint8_t data)
	{
		// ���� 2 bit �̂ݗL��
		assert((0b11111100 & data) == 0);
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			t &= ~NAMETABLE_SELECT_MASK;
			uint16_t writeData = static_cast<uint16_t>(data) << 10;
			t |= writeData;
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			v &= ~NAMETABLE_SELECT_MASK;
			uint16_t writeData = static_cast<uint16_t>(data) << 10;
			v |= writeData;
		}
	}
	void PpuInternalRegister::SetFineY(PpuInternalRegistertarget target, uint8_t data)
	{
		// ���� 3 bit �̂ݗL��
		assert((0b11111000 & data) == 0);
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			t &= ~FINE_Y_MASK;
			uint16_t writeData = static_cast<uint16_t>(data) << 12;
			t |= writeData;
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			v &= ~FINE_Y_MASK;
			uint16_t writeData = static_cast<uint16_t>(data) << 12;
			v |= writeData;
		}
	}
	void PpuInternalRegister::SetFineX(uint8_t data)
	{
		// ���� 3 bit �̂ݗL��
		assert((0b11111000 & data) == 0);
		x = data;
	}
	void PpuInternalRegister::SetW(bool data)
	{
		w = data;
	}
	void PpuInternalRegister::SetUpperPpuAddr(uint8_t data)
	{
		// �� 2 bit ���}�X�N����(https://wiki.nesdev.com/w/index.php/PPU_scrolling#Register_controls)
		data &= 0b00111111;
		uint16_t writeData = static_cast<uint16_t>(data) << 8;
		t &= 0xFF;
		t |= writeData;
		w = true;
	}
	void PpuInternalRegister::SetLowerPpuAddr(uint8_t data)
	{
		// ����8bit ���X�V���邾��
		t &= 0xFF00;
		t |= data;
		v = t;

		w = false;
	}

	uint8_t PpuInternalRegister::GetCoarseX(PpuInternalRegistertarget target)
	{
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			return static_cast<uint8_t>(t & COARSE_X_MASK);
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			return static_cast<uint8_t>(v & COARSE_X_MASK);
		}
		else
		{
			// unexpected default
			exit(-1);
		}
	}
	uint8_t PpuInternalRegister::GetCoarseY(PpuInternalRegistertarget target)
	{
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			return static_cast<uint8_t>((t & COARSE_Y_MASK) >> 5);
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			return static_cast<uint8_t>((v & COARSE_Y_MASK) >> 5);
		}
		else
		{
			// unexpected default
			exit(-1);
		}
	}
	uint8_t PpuInternalRegister::GetNametableSelect(PpuInternalRegistertarget target)
	{
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			return static_cast<uint8_t>((t & NAMETABLE_SELECT_MASK) >> 10);
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			return static_cast<uint8_t>((v & NAMETABLE_SELECT_MASK) >> 10);
		}
		else
		{
			// unexpected default
			exit(-1);
		}
	}
	uint8_t PpuInternalRegister::GetFineY(PpuInternalRegistertarget target)
	{
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			return static_cast<uint8_t>((t & FINE_Y_MASK) >> 12);
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			return static_cast<uint8_t>((v & FINE_Y_MASK) >> 12);
		}
		else
		{
			// unexpected default
			exit(-1);
		}
	}
	uint8_t PpuInternalRegister::GetFineX()
	{
		return x;
	}
	bool PpuInternalRegister::GetW()
	{
		return w;
	}


	// �`�撆�̃C���N�������g(https://wiki.nesdev.com/w/index.php/PPU_scrolling#Wrapping_around)
	void PpuInternalRegister::IncrementCoarseX()
	{
		uint8_t coarseX = GetCoarseX(PpuInternalRegistertarget::PpuInternalRegistertarget_v);
		// tile 31 �̎��� 0 �ɂ��ǂ��A��Ŏ��������A�N�Z�T�[�͎g�킸�� nesdev wiki �̋^���R�[�h�����̂܂܎g�����Ⴄ
		if (coarseX == 31)
		{
			v &= ~0x001F;          // coarse X = 0
			v ^= 0x0400;           // switch horizontal nametable
		}
		else
		{
			v++;
		}
	}

	void PpuInternalRegister::IncrementY()
	{
		// ������� nesdev wiki �̋^���R�[�h�����̂܂܎g�����Ⴄ
		if ((v & 0x7000) != 0x7000)				// if fine Y < 7
		{
			v += 0x1000;						// increment fine Y
		}
		else
		{
			v &= ~0x7000;						// fine Y = 0
			int y = (v & 0x03E0) >> 5;			// let y = coarse Y
			if (y == 29)
			{
				y = 0;                          // coarse Y = 0
				v ^= 0x0800;                    // switch vertical nametable
			}
			else if (y == 31)
			{
				y = 0;                          // coarse Y = 0, nametable not switched
			}
			else 
			{
				y += 1;							// increment coarse Y
			}
			v = (v & ~0x03E0) | (y << 5);		// put coarse Y back into v
		}
	}

	void PpuInternalRegister::UpdateHorizontalV()
	{
		const uint16_t HORIZONTAL_MASK = 0b000010000011111;
		v &= ~HORIZONTAL_MASK;
		uint16_t update = t & HORIZONTAL_MASK;

		v |= update;
	}
	void PpuInternalRegister::UpdateVerticalV()
	{
		const uint16_t VERTICAL_MASK = 0b111101111100000;
		v &= ~VERTICAL_MASK;
		uint16_t update = t & VERTICAL_MASK;

		v |= update;
	}

	// ���݂̃^�C���� attribute table �̃A�h���X�擾(https://wiki.nesdev.com/w/index.php/PPU_scrolling#Tile_and_attribute_fetching)
	uint16_t PpuInternalRegister::GetTileAddress()
	{
		return static_cast<uint16_t>(0x2000 | (v & 0x0FFF));
	}
	uint16_t PpuInternalRegister::GetAttributeAddress()
	{
		return static_cast<uint16_t>(0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07));
	}
}}