#pragma once
#include <cassert>
#include <utility>
#include "Ppu.h"

namespace {
	// �e�[�u�����Ђ��Ђ�����w���p�[�֐�

	// ���W -> (nametable id, tile id)�̃y�A
	std::pair<int, int> PositionToTileid(int y, int x)
	{
		int nameTableId = 0;

		if (y >= nes::PPU_OUTPUT_Y)
		{
			y -= nes::PPU_OUTPUT_Y;
			nameTableId += 2;
		}
		if (x >= nes::PPU_OUTPUT_X)
		{
			x -= nes::PPU_OUTPUT_X;
			nameTableId += 1;
		}

		int tileX = x / 8;
		int tileY = y / 8;
		int tileId = tileY * 32 + tileX;

		return std::make_pair(nameTableId, tileId);
	}

	// ���W -> (attribute table �� idx, attribute table ���� idx)�̃y�A
	std::pair<int, int> PositionToAttributeTable(int y, int x)
	{
		// TODO: �X�N���[�� �Ή�

		// �p���b�g�� 16 * 16���ƁA Attribute Table �� 32 * 32 ����  
		const int PaletteUnit = 16;
		const int AttributeUnit = PaletteUnit * 2;

		int attributeY = y / AttributeUnit;
		int attributeX = x / AttributeUnit;

		// Attribute Table �� 1�v�f �� 32 * 32 px ��1�Ή�����̂ŁA 1�s��8�܂܂�� 
		int attributeIdx = attributeY * 8 + attributeX;

		int baseX = attributeX * AttributeUnit;
		int baseY = attributeY * AttributeUnit;

		int dx = x - baseX;
		int dy = y - baseY;

		int paletteX = dx / PaletteUnit;
		int paletteY = dy / PaletteUnit;

		int paletteIdx = paletteY * 2 + paletteX;

		assert(paletteIdx <= 3);

		return std::make_pair(attributeIdx, paletteIdx);
	}
}

namespace nes { namespace detail {
	bool Ppu::Run(int clk)
	{
		int beforeCycles = m_Cycles;
		m_Cycles += clk;

		// 1 ���C���`�悷��̂� 341 �T�C�N��������
		if (m_Cycles >= 341)
		{
			m_Cycles -= 341;
			BuildBackGroundLine();
			m_Lines++;
			// Line 241 �ɂ��Ă��� NMI ����
			if (m_Lines == 241)
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

	bool Ppu::IsSprite0Hit(int y, int x)
	{
		// Sprite 0 hit ���������Ȃ������ɓ��Ă͂܂��Ă���Ȃ瑁�� return �����Ⴄ
		bool enableClippingBg	  = (PPUMASK & PPUMASKS_ENABLE_BG_MASK)		== 0;
		bool enableClippingSprite = (PPUMASK & PPUMASKS_ENABLE_SPRITE_MASK) == 0;
		bool enableBg			  = (PPUMASK & PPUMASKS_ENABLE_BG)		    == PPUMASKS_ENABLE_BG;
		bool enableSprite		  = (PPUMASK & PPUMASKS_ENABLE_SPRITE)      == PPUMASKS_ENABLE_SPRITE;

		// �N���b�s���O�L�� or {�w�i or �X�v���C�g�`�斳���Ȃ�X�v���C�g0hit���Ȃ�}
		if (enableClippingBg || enableClippingSprite || !enableBg || !enableSprite)
		{
			return false;
		}

		Sprite sprite0 = GetSprite(0);
		// OAM �Ɋi�[�����y���W�� -1 ����Ă�̂ő���
		sprite0.y++;

		// ���΍��W �v�Z
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
			bool isBgClear = GetBackGroundPixelColor(y, x).second;

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

	// ���4�����̍��W(�܂�A�`����W�ł͂Ȃ�)����͂��Ă��̃s�N�Z���̐F���擾����A���̐F���������ǂ������擾����
	std::pair<uint8_t, bool> Ppu::GetBackGroundPixelColor(int y, int x)
	{
		auto [nametableId, tileId] = PositionToTileid(y, x);
		// paletteIdx: Attribute Table ���̉��Ԗڂ̃p���b�g���g�����H
		auto [attributeIdx, paletteIdx] = PositionToAttributeTable(y, x);

		uint32_t addr = NAMETABLE_BASE + nametableId * NAME_TABLE_AND_ATTRIBUTE_TABLE_SINGLE_SIZE + tileId;
		// �l�[���e�[�u�� / �����e�[�u��������̈�̃A�h���X�Ɏ��܂��ĂĂق���
		assert(addr < 0x3F00);

		uint8_t spriteNum = m_pPpuBus->ReadByte(static_cast<uint16_t>(addr));
		// �w�i�̃X�v���C�g�� 8 * 8 �Ȃ̂ŁA 8 �Ŋ������]��� offset ���o��
		int offsetY = y % 8;
		int offsetX = x % 8;

		uint8_t patternTableLower = m_pPpuBus->ReadByte(GetBGPatternTableBase() + spriteNum * static_cast<uint16_t>(PATTERN_TABLE_ELEMENT_SIZE) + offsetY);
		uint8_t patternTableUpper = m_pPpuBus->ReadByte(GetBGPatternTableBase() + spriteNum * static_cast<uint16_t>(PATTERN_TABLE_ELEMENT_SIZE) + offsetY + 8);

		int bitPos = 7 - offsetX;

		uint8_t colorLower = (patternTableLower & (1 << bitPos)) == (1 << bitPos);
		uint8_t colorUpper = (patternTableUpper & (1 << bitPos)) == (1 << bitPos);
		colorUpper <<= 1;

		uint8_t color = colorLower | colorUpper;
		assert(color <= 3);

		// attribute table ����p���b�g�� id �����߂�
		uint32_t attributeTableAddr = NAMETABLE_BASE + nametableId * NAME_TABLE_AND_ATTRIBUTE_TABLE_SINGLE_SIZE + NAME_TABLE_SINGLE_SIZE + attributeIdx;
		assert(attributeTableAddr < 0x3F00);

		uint8_t attributeTable = m_pPpuBus->ReadByte(static_cast<uint16_t>(attributeTableAddr));

		// attribute table ���� paletteIdx �Ԗڂ̒l�����o��
		uint8_t paletteId = attributeTable & (0b11 << (paletteIdx * 2));
		paletteId >>= (paletteIdx * 2);

		// palette[paletteId][color] �����ۂɊG�Ƃ��Č����F�B color == 0 �̂Ƃ��͓����F�Ȃ̂ŁA���̏����܂Ƃ߂ĕԂ�
		const uint16_t PaletteSize = 4;
		uint8_t ret = m_pPpuBus->ReadByte(PALETTE_BASE + PaletteSize * paletteId + color);
		return std::make_pair(ret, color == 0);
	}


	void Ppu::BuildBackGroundLine()
	{
		// TODO: �X�N���[��(y �� x �� SCROLL���W�X�^�̒l�𑫂��Ƃ�)
		// �G �X�N���[�� 0xfe ���
		if (m_ScrollY >= 240) {
			return;
		}

		int y = m_Lines;

		// VBLANK �ɂ͂Ȃ�����Ȃ�
		if (y >= 240) 
		{
			return;
		}

		for (int x = 0; x < PPU_OUTPUT_X; x++)
		{
			int nameTableNum = PPUCTRL & 0b11;
			int nameTableBaseX = (nameTableNum % 2) * PPU_OUTPUT_X;
			int nameTableBaseY = (nameTableNum / 2) * PPU_OUTPUT_Y;

			int actualY = (y + m_ScrollY + nameTableBaseY) % (static_cast<int>(PPU_OUTPUT_Y) * 2);
			int actualX = (x + m_ScrollX + nameTableBaseX) % (static_cast<int>(PPU_OUTPUT_X) * 2);

			auto [color, isClear] = GetBackGroundPixelColor(actualY, actualX);
			m_PpuOutput[y][x] = color;
			m_IsBackgroundClear[y][x] = isClear;
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
		if (m_IsVerticalScrollVal)
		{
			m_ScrollY = data;
			m_IsVerticalScrollVal = false;
		}
		else
		{
			m_ScrollX = data;
			m_IsVerticalScrollVal = true;
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
	}
	void Ppu::WritePpuData(uint8_t data)
	{
		m_pPpuBus->WriteByte(m_VramAddr, data);
		m_VramAddr += GetVramOffset();
	}

	uint8_t Ppu::ReadPpuStatus()
	{
		// 2��ǂ݃t���O�����Z�b�g
		m_IsVerticalScrollVal = false;
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
			abort();
		}
	}
	uint8_t PpuInternalRegister::GetCoarseY(PpuInternalRegistertarget target)
	{
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			return static_cast<uint8_t>(t & COARSE_Y_MASK);
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			return static_cast<uint8_t>(v & COARSE_Y_MASK);
		}
		else
		{
			// unexpected default
			abort();
		}
	}
	uint8_t PpuInternalRegister::GetNametableSelect(PpuInternalRegistertarget target)
	{
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			return static_cast<uint8_t>(t & NAMETABLE_SELECT_MASK);
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			return static_cast<uint8_t>(v & NAMETABLE_SELECT_MASK);
		}
		else
		{
			// unexpected default
			abort();
		}
	}
	uint8_t PpuInternalRegister::GetFineY(PpuInternalRegistertarget target)
	{
		if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_t)
		{
			return static_cast<uint8_t>(t & FINE_Y_MASK);
		}
		else if (target == PpuInternalRegistertarget::PpuInternalRegistertarget_v)
		{
			return static_cast<uint8_t>(v & FINE_Y_MASK);
		}
		else
		{
			// unexpected default
			abort();
		}
	}
	uint8_t PpuInternalRegister::GetFineX()
	{
		return x;
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