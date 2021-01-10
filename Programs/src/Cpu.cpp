#pragma once
#include <cassert>
#include <cstdlib>
#include <stdint.h>
#include "constants.h"
#include "Cpu.h"

namespace {
    // M, N �� 8bit�����t�������A C ���L�����[�t���O�Ƃ����Ƃ��AN + M + C ���I�[�o�[�t���[���邩�H
    // �����t�������̉����Z�I�[�o�[�t���[���肾���A������ uint8_t�ł��邱�Ƃɒ��ӂ���(�����t�������̃I�[�o�[�t���[�͖���`)
    bool isSignedOverFlowed(uint8_t N, uint8_t M, bool C)
    {
        uint8_t res = N + M + C;
        return ((M ^ res) & (N ^ res) & 0x80) == 0x80;
    }
}

namespace nes { namespace detail {
	Instruction ByteToInstruction(uint8_t byte)
	{
		switch (byte)
		{
        case 0x69:
            return Instruction(Opcode::ADC, AddressingMode::Immediate, 2, 2);
        case 0x65:
            return Instruction(Opcode::ADC, AddressingMode::ZeroPage, 2, 3);
        case 0x75:
            return Instruction(Opcode::ADC, AddressingMode::ZeroPageX, 2, 4);
        case 0x6D:
            return Instruction(Opcode::ADC, AddressingMode::Absolute, 3, 4);
        case 0x7D:
            return Instruction(Opcode::ADC, AddressingMode::AbsoluteX, 3, 4);
        case 0x79:
            return Instruction(Opcode::ADC, AddressingMode::AbsoluteY, 3, 4);
        case 0x61:
            return Instruction(Opcode::ADC, AddressingMode::IndirectX, 2, 6);
        case 0x71:
            return Instruction(Opcode::ADC, AddressingMode::IndirectY, 2, 5);
        case 0x29:
            return Instruction(Opcode::AND, AddressingMode::Immediate, 2, 2);
        case 0x25:
            return Instruction(Opcode::AND, AddressingMode::ZeroPage, 2, 3);
        case 0x35:
            return Instruction(Opcode::AND, AddressingMode::ZeroPageX, 2, 4);
        case 0x2D:
            return Instruction(Opcode::AND, AddressingMode::Absolute, 3, 4);
        case 0x3D:
            return Instruction(Opcode::AND, AddressingMode::AbsoluteX, 3, 4);
        case 0x39:
            return Instruction(Opcode::AND, AddressingMode::AbsoluteY, 3, 4);
        case 0x21:
            return Instruction(Opcode::AND, AddressingMode::IndirectX, 2, 6);
        case 0x31:
            return Instruction(Opcode::AND, AddressingMode::IndirectY, 2, 5);
        case 0x0A:
            return Instruction(Opcode::ASL, AddressingMode::Accumulator, 1, 2);
        case 0x06:
            return Instruction(Opcode::ASL, AddressingMode::ZeroPage, 2, 5);
        case 0x16:
            return Instruction(Opcode::ASL, AddressingMode::ZeroPageX, 2, 6);
        case 0x0E:
            return Instruction(Opcode::ASL, AddressingMode::Absolute, 3, 6);
        case 0x1E:
            return Instruction(Opcode::ASL, AddressingMode::AbsoluteX, 3, 7);
        case 0x90:
            return Instruction(Opcode::BCC, AddressingMode::Relative, 2, 2);
        case 0xB0:
            return Instruction(Opcode::BCS, AddressingMode::Relative, 2, 2);
        case 0xF0:
            return Instruction(Opcode::BEQ, AddressingMode::Relative, 2, 2);
        case 0x24:
            return Instruction(Opcode::BIT, AddressingMode::ZeroPage, 2, 3);
        case 0x2C:
            return Instruction(Opcode::BIT, AddressingMode::Absolute, 3, 4);
        case 0x30:
            return Instruction(Opcode::BMI, AddressingMode::Relative, 2, 2);
        case 0xD0:
            return Instruction(Opcode::BNE, AddressingMode::Relative, 2, 2);
        case 0x10:
            return Instruction(Opcode::BPL, AddressingMode::Relative, 2, 2);
        case 0x00:
            return Instruction(Opcode::BRK, AddressingMode::Implied, 1, 7);
        case 0x50:
            return Instruction(Opcode::BVC, AddressingMode::Relative, 2, 2);
        case 0x70:
            return Instruction(Opcode::BVS, AddressingMode::Relative, 2, 2);
        case 0x18:
            return Instruction(Opcode::CLC, AddressingMode::Implied, 1, 2);
        case 0xD8:
            return Instruction(Opcode::CLD, AddressingMode::Implied, 1, 2);
        case 0x58:
            return Instruction(Opcode::CLI, AddressingMode::Implied, 1, 2);
        case 0xB8:
            return Instruction(Opcode::CLV, AddressingMode::Implied, 1, 2);
        case 0xC9:
            return Instruction(Opcode::CMP, AddressingMode::Immediate, 2, 2);
        case 0xC5:
            return Instruction(Opcode::CMP, AddressingMode::ZeroPage, 2, 3);
        case 0xD5:
            return Instruction(Opcode::CMP, AddressingMode::ZeroPageX, 2, 4);
        case 0xCD:
            return Instruction(Opcode::CMP, AddressingMode::Absolute, 3, 4);
        case 0xDD:
            return Instruction(Opcode::CMP, AddressingMode::AbsoluteX, 3, 4);
        case 0xD9:
            return Instruction(Opcode::CMP, AddressingMode::AbsoluteY, 3, 4);
        case 0xC1:
            return Instruction(Opcode::CMP, AddressingMode::IndirectX, 2, 6);
        case 0xD1:
            return Instruction(Opcode::CMP, AddressingMode::IndirectY, 2, 5);
        case 0xE0:
            return Instruction(Opcode::CPX, AddressingMode::Immediate, 2, 2);
        case 0xE4:
            return Instruction(Opcode::CPX, AddressingMode::ZeroPage, 2, 3);
        case 0xEC:
            return Instruction(Opcode::CPX, AddressingMode::Absolute, 3, 4);
        case 0xC0:
            return Instruction(Opcode::CPY, AddressingMode::Immediate, 2, 2);
        case 0xC4:
            return Instruction(Opcode::CPY, AddressingMode::ZeroPage, 2, 3);
        case 0xCC:
            return Instruction(Opcode::CPY, AddressingMode::Absolute, 3, 4);
        case 0xC6:
            return Instruction(Opcode::DEC, AddressingMode::ZeroPage, 2, 5);
        case 0xD6:
            return Instruction(Opcode::DEC, AddressingMode::ZeroPageX, 2, 6);
        case 0xCE:
            return Instruction(Opcode::DEC, AddressingMode::Absolute, 3, 6);
        case 0xDE:
            return Instruction(Opcode::DEC, AddressingMode::AbsoluteX, 3, 7);
        case 0xCA:
            return Instruction(Opcode::DEX, AddressingMode::Implied, 1, 2);
        case 0x88:
            return Instruction(Opcode::DEY, AddressingMode::Implied, 1, 2);
        case 0x49:
            return Instruction(Opcode::EOR, AddressingMode::Immediate, 2, 2);
        case 0x45:
            return Instruction(Opcode::EOR, AddressingMode::ZeroPage, 2, 3);
        case 0x55:
            return Instruction(Opcode::EOR, AddressingMode::ZeroPageX, 2, 4);
        case 0x4D:
            return Instruction(Opcode::EOR, AddressingMode::Absolute, 3, 4);
        case 0x5D:
            return Instruction(Opcode::EOR, AddressingMode::AbsoluteX, 3, 4);
        case 0x59:
            return Instruction(Opcode::EOR, AddressingMode::AbsoluteY, 3, 4);
        case 0x41:
            return Instruction(Opcode::EOR, AddressingMode::IndirectX, 2, 6);
        case 0x51:
            return Instruction(Opcode::EOR, AddressingMode::IndirectY, 2, 5);
        case 0xE6:
            return Instruction(Opcode::INC, AddressingMode::ZeroPage, 2, 5);
        case 0xF6:
            return Instruction(Opcode::INC, AddressingMode::ZeroPageX, 2, 6);
        case 0xEE:
            return Instruction(Opcode::INC, AddressingMode::Absolute, 3, 6);
        case 0xFE:
            return Instruction(Opcode::INC, AddressingMode::AbsoluteX, 3, 7);
        case 0xE8:
            return Instruction(Opcode::INX, AddressingMode::Implied, 1, 2);
        case 0xC8:
            return Instruction(Opcode::INY, AddressingMode::Implied, 1, 2);
        case 0x4C:
            return Instruction(Opcode::JMP, AddressingMode::Absolute, 3, 3);
        case 0x6C:
            return Instruction(Opcode::JMP, AddressingMode::Indirect, 3, 5);
        case 0x20:
            return Instruction(Opcode::JSR, AddressingMode::Absolute, 3, 6);
        case 0xA9:
            return Instruction(Opcode::LDA, AddressingMode::Immediate, 2, 2);
        case 0xA5:
            return Instruction(Opcode::LDA, AddressingMode::ZeroPage, 2, 3);
        case 0xB5:
            return Instruction(Opcode::LDA, AddressingMode::ZeroPageX, 2, 4);
        case 0xAD:
            return Instruction(Opcode::LDA, AddressingMode::Absolute, 3, 4);
        case 0xBD:
            return Instruction(Opcode::LDA, AddressingMode::AbsoluteX, 3, 4);
        case 0xB9:
            return Instruction(Opcode::LDA, AddressingMode::AbsoluteY, 3, 4);
        case 0xA1:
            return Instruction(Opcode::LDA, AddressingMode::IndirectX, 2, 6);
        case 0xB1:
            return Instruction(Opcode::LDA, AddressingMode::IndirectY, 2, 5);
        case 0xA2:
            return Instruction(Opcode::LDX, AddressingMode::Immediate, 2, 2);
        case 0xA6:
            return Instruction(Opcode::LDX, AddressingMode::ZeroPage, 2, 3);
        case 0xB6:
            return Instruction(Opcode::LDX, AddressingMode::ZeroPageY, 2, 4);
        case 0xAE:
            return Instruction(Opcode::LDX, AddressingMode::Absolute, 3, 4);
        case 0xBE:
            return Instruction(Opcode::LDX, AddressingMode::AbsoluteY, 3, 4);
        case 0xA0:
            return Instruction(Opcode::LDY, AddressingMode::Immediate, 2, 2);
        case 0xA4:
            return Instruction(Opcode::LDY, AddressingMode::ZeroPage, 2, 3);
        case 0xB4:
            return Instruction(Opcode::LDY, AddressingMode::ZeroPageX, 2, 4);
        case 0xAC:
            return Instruction(Opcode::LDY, AddressingMode::Absolute, 3, 4);
        case 0xBC:
            return Instruction(Opcode::LDY, AddressingMode::AbsoluteX, 3, 4);
        case 0x4A:
            return Instruction(Opcode::LSR, AddressingMode::Accumulator, 1, 2);
        case 0x46:
            return Instruction(Opcode::LSR, AddressingMode::ZeroPage, 2, 5);
        case 0x56:
            return Instruction(Opcode::LSR, AddressingMode::ZeroPageX, 2, 6);
        case 0x4E:
            return Instruction(Opcode::LSR, AddressingMode::Absolute, 3, 6);
        case 0x5E:
            return Instruction(Opcode::LSR, AddressingMode::AbsoluteX, 3, 7);
        case 0xEA:
            return Instruction(Opcode::NOP, AddressingMode::Implied, 1, 2);
        case 0x09:
            return Instruction(Opcode::ORA, AddressingMode::Immediate, 2, 2);
        case 0x05:
            return Instruction(Opcode::ORA, AddressingMode::ZeroPage, 2, 3);
        case 0x15:
            return Instruction(Opcode::ORA, AddressingMode::ZeroPageX, 2, 4);
        case 0x0D:
            return Instruction(Opcode::ORA, AddressingMode::Absolute, 3, 4);
        case 0x1D:
            return Instruction(Opcode::ORA, AddressingMode::AbsoluteX, 3, 4);
        case 0x19:
            return Instruction(Opcode::ORA, AddressingMode::AbsoluteY, 3, 4);
        case 0x01:
            return Instruction(Opcode::ORA, AddressingMode::IndirectX, 2, 6);
        case 0x11:
            return Instruction(Opcode::ORA, AddressingMode::IndirectY, 2, 5);
        case 0x48:
            return Instruction(Opcode::PHA, AddressingMode::Implied, 1, 3);
        case 0x08:
            return Instruction(Opcode::PHP, AddressingMode::Implied, 1, 3);
        case 0x68:
            return Instruction(Opcode::PLA, AddressingMode::Implied, 1, 4);
        case 0x28:
            return Instruction(Opcode::PLP, AddressingMode::Implied, 1, 4);
        case 0x2A:
            return Instruction(Opcode::ROL, AddressingMode::Accumulator, 1, 2);
        case 0x26:
            return Instruction(Opcode::ROL, AddressingMode::ZeroPage, 2, 5);
        case 0x36:
            return Instruction(Opcode::ROL, AddressingMode::ZeroPageX, 2, 6);
        case 0x2E:
            return Instruction(Opcode::ROL, AddressingMode::Absolute, 3, 6);
        case 0x3E:
            return Instruction(Opcode::ROL, AddressingMode::AbsoluteX, 3, 7);
        case 0x6A:
            return Instruction(Opcode::ROR, AddressingMode::Accumulator, 1, 2);
        case 0x66:
            return Instruction(Opcode::ROR, AddressingMode::ZeroPage, 2, 5);
        case 0x76:
            return Instruction(Opcode::ROR, AddressingMode::ZeroPageX, 2, 6);
        case 0x6E:
            return Instruction(Opcode::ROR, AddressingMode::Absolute, 3, 6);
        case 0x7E:
            return Instruction(Opcode::ROR, AddressingMode::AbsoluteX, 3, 7);
        case 0x40:
            return Instruction(Opcode::RTI, AddressingMode::Implied, 1, 6);
        case 0x60:
            return Instruction(Opcode::RTS, AddressingMode::Implied, 1, 6);
        case 0xE9:
            return Instruction(Opcode::SBC, AddressingMode::Immediate, 2, 2);
        case 0xE5:
            return Instruction(Opcode::SBC, AddressingMode::ZeroPage, 2, 3);
        case 0xF5:
            return Instruction(Opcode::SBC, AddressingMode::ZeroPageX, 2, 4);
        case 0xED:
            return Instruction(Opcode::SBC, AddressingMode::Absolute, 3, 4);
        case 0xFD:
            return Instruction(Opcode::SBC, AddressingMode::AbsoluteX, 3, 4);
        case 0xF9:
            return Instruction(Opcode::SBC, AddressingMode::AbsoluteY, 3, 4);
        case 0xE1:
            return Instruction(Opcode::SBC, AddressingMode::IndirectX, 2, 6);
        case 0xF1:
            return Instruction(Opcode::SBC, AddressingMode::IndirectY, 2, 5);
        case 0x38:
            return Instruction(Opcode::SEC, AddressingMode::Implied, 1, 2);
        case 0xF8:
            return Instruction(Opcode::SED, AddressingMode::Implied, 1, 2);
        case 0x78:
            return Instruction(Opcode::SEI, AddressingMode::Implied, 1, 2);
        case 0x85:
            return Instruction(Opcode::STA, AddressingMode::ZeroPage, 2, 3);
        case 0x95:
            return Instruction(Opcode::STA, AddressingMode::ZeroPageX, 2, 4);
        case 0x8D:
            return Instruction(Opcode::STA, AddressingMode::Absolute, 3, 4);
        case 0x9D:
            return Instruction(Opcode::STA, AddressingMode::AbsoluteX, 3, 5);
        case 0x99:
            return Instruction(Opcode::STA, AddressingMode::AbsoluteY, 3, 5);
        case 0x81:
            return Instruction(Opcode::STA, AddressingMode::IndirectX, 2, 6);
        case 0x91:
            return Instruction(Opcode::STA, AddressingMode::IndirectY, 2, 6);
        case 0x86:
            return Instruction(Opcode::STX, AddressingMode::ZeroPage, 2, 3);
        case 0x96:
            return Instruction(Opcode::STX, AddressingMode::ZeroPageY, 2, 4);
        case 0x8E:
            return Instruction(Opcode::STX, AddressingMode::Absolute, 3, 4);
        case 0x84:
            return Instruction(Opcode::STY, AddressingMode::ZeroPage, 2, 3);
        case 0x94:
            return Instruction(Opcode::STY, AddressingMode::ZeroPageX, 2, 4);
        case 0x8C:
            return Instruction(Opcode::STY, AddressingMode::Absolute, 3, 4);
        case 0xAA:
            return Instruction(Opcode::TAX, AddressingMode::Implied, 1, 2);
        case 0xA8:
            return Instruction(Opcode::TAY, AddressingMode::Implied, 1, 2);
        case 0xBA:
            return Instruction(Opcode::TSX, AddressingMode::Implied, 1, 2);
        case 0x8A:
            return Instruction(Opcode::TXA, AddressingMode::Implied, 1, 2);
        case 0x9A:
            return Instruction(Opcode::TXS, AddressingMode::Implied, 1, 2);
        case 0x98:
            return Instruction(Opcode::TYA, AddressingMode::Implied, 1, 2);

		default:
			abort();
			break;
		}
	}

    void Cpu::SetNegativeFlag(bool flag)
    {
        if (flag)
        {
            P |= (1 << 7);
        }
        else {
            P &= ~(1 << 7);
        }
    }
    void Cpu::SetOverflowFlag(bool flag)
    {
        if (flag)
        {
            P |= (1 << 6);
        }
        else {
            P &= ~(1 << 6);
        }
    }
    void Cpu::SetBreakFlag(bool flag)
    {
        if (flag)
        {
            P |= (1 << 4);
        }
        else {
            P &= ~(1 << 4);
        }
    }
    void Cpu::SetDecimalFlag(bool flag)
    {
        if (flag)
        {
            P |= (1 << 3);
        }
        else {
            P &= ~(1 << 3);
        }
    }
    void Cpu::SetInterruptFlag(bool flag)
    {
        if (flag)
        {
            P |= (1 << 2);
        }
        else {
            P &= ~(1 << 2);
        }
    }
    void Cpu::SetZeroFlag(bool flag)
    {
        if (flag)
        {
            P |= (1 << 1);
        }
        else {
            P &= ~(1 << 1);
        }
    }
    void Cpu::SetCarryFlag(bool flag)
    {
        if (flag)
        {
            P |= 1;
        }
        else {
            P &= ~(1);
        }
    }

    bool Cpu::GetNegativeFlag()
    {
        return (P & (1 << 7)) == (1 << 7);
    }
    bool Cpu::GetOverflowFlag()
    {
        return (P & (1 << 6)) == (1 << 6);
    }
    bool Cpu::GetBreakFlag()
    {
        return (P & (1 << 4)) == (1 << 4);
    }
    bool Cpu::GetDecimalFlag()
    {
        return (P & (1 << 3)) == (1 << 3);
    }
    bool Cpu::GetInterruptFlag()
    {
        return (P & (1 << 2)) == (1 << 2);
    }
    bool Cpu::GetZeroFlag()
    {
        return (P & (1 << 1)) == (1 << 1);
    }
    bool Cpu::GetCarryFlag()
    {
        return (P & 1) == 1;
    }
    // �Ώۂ̃A�h���X���t�F�b�`����N�A�l�̎擾���C���[�͈��̑w�������Ă����ł��
    void Cpu::FetchAddr(AddressingMode mode, uint16_t* pOutAddr, uint8_t* pOutAdditionalCyc)
    {
        // �A�h���X����Ȃ��͂��̐l�炪���Ă���v���O���~���O�~�X�Ȃ̂� assert ���Ƃ�
        assert(mode != AddressingMode::Implied && mode != AddressingMode::Immediate && mode != AddressingMode::Accumulator);

        *pOutAddr = 0;
        *pOutAdditionalCyc = 0;

        // PC �͖��߂ƃI�y�����h�̃t�F�b�`�ł͓��������A���ߎ��s��ɂ܂Ƃ߂ē�����(�f�o�b�O���O�̎����ŗL���ɂȂ�c�c�͂�)
        if (mode == AddressingMode::Absolute)
        {
            uint16_t upper = 0;
            uint16_t lower = 0;
            lower = m_CpuBus.ReadByte(PC + 1);
            upper = m_CpuBus.ReadByte(PC + 2);

            uint16_t addr = 0;
            addr |= lower;
            addr |= (upper << 8);

            *pOutAddr = addr;
        }
        else if (mode == AddressingMode::ZeroPage)
        {
            uint16_t addr = m_CpuBus.ReadByte(PC + 1);

            *pOutAddr = addr;
        }
        else if (mode == AddressingMode::ZeroPageX)
        {
            uint8_t lower = m_CpuBus.ReadByte(PC + 1);
            
            // ��ʃo�C�g�ւ̌��グ�͖����A�Ȃ̂� uint8 �̂܂܉��Z����
            lower += X;
            *pOutAddr = lower;
        }
        else if (mode == AddressingMode::ZeroPageY)
        {
            uint8_t lower = m_CpuBus.ReadByte(PC + 1);

            // ��ʃo�C�g�ւ̌��グ�͖����A�Ȃ̂� uint8 �̂܂܉��Z����
            lower += Y;
            *pOutAddr = lower;
        }
        else if (mode == AddressingMode::AbsoluteX)
        {
            uint16_t upper = 0;
            uint16_t lower = 0;
            lower = m_CpuBus.ReadByte(PC + 1);
            upper = m_CpuBus.ReadByte(PC + 2);

            uint16_t addr = 0;
            addr |= lower;
            addr |= (upper << 8);

            uint16_t beforeAddr = addr;
            addr += X;

            *pOutAddr = addr;
            // �y�[�W�N���X�� +1 �N���b�N
            if ((beforeAddr & 0xFF00) != (addr & 0xFF00))
            {
                *pOutAdditionalCyc = 1;
            }

        }
        else if (mode == AddressingMode::AbsoluteY)
        {
            uint16_t upper = 0;
            uint16_t lower = 0;
            lower = m_CpuBus.ReadByte(PC + 1);
            upper = m_CpuBus.ReadByte(PC + 2);

            uint16_t addr = 0;
            addr |= lower;
            addr |= (upper << 8);

            uint16_t beforeAddr = addr;
            addr += Y;

            *pOutAddr = addr;
            // �y�[�W�N���X�� +1 �N���b�N
            if ((beforeAddr & 0xFF00) != (addr & 0xFF00))
            {
                *pOutAdditionalCyc = 1;
            }
        }
        else if (mode == AddressingMode::Relative)
        {
            uint8_t offset = m_CpuBus.ReadByte(PC + 1);
            // �����g�� ����(�኱�������̂ŁA�o�O������^��(�ň�))
            int32_t signedOffset = static_cast<int8_t>(offset);
            // TORIAEZU: �t�F�b�`�ςƂ����Ƃ��� PC ���N�_�ɂ���
            int32_t signedPC = PC + 2;

            int32_t signedAddr = signedPC + signedOffset;
            // uint16_t �Ɏ��܂��Ă��邱�Ƃ��m�F
            assert(signedAddr >= 0 && signedAddr <= 0xFFFF);
            uint16_t addr = static_cast<uint16_t>(signedAddr);

            *pOutAddr = addr;
            // �y�[�W�N���X�� +1 �N���b�N�ARelative �̓u�����`���߂Ŏg���邪�A�u�����`�������ɂ͂���� +1 ����邱�Ƃɒ��ӂ���
            if ((PC & 0xFF00) != (addr & 0xFF00))
            {
                *pOutAdditionalCyc = 1;
            }
        }
        else if (mode == AddressingMode::IndirectX)
        {
            // *(lower + X)
            uint8_t indirectLower = 0;
            indirectLower = m_CpuBus.ReadByte(PC + 1);
            // �L�����[�͖��� = �I�[�o�[�t���[���Ă��C�ɂ��Ȃ�(�����Ȃ������̃I�[�o�[�t���[�͖���`�łȂ����Ƃ��m�F�ς�)
            uint8_t lowerAddr = indirectLower + X;
            uint8_t upperAddr = lowerAddr + 1;
            // Indirect �Ȃ̂ŁAFetchAddr ����1��Q�Ƃ𔍂���
            uint16_t lower = m_CpuBus.ReadByte(lowerAddr);
            uint16_t upper = m_CpuBus.ReadByte(upperAddr);

            uint16_t addr = lower | (upper << 8);

            *pOutAddr = addr;
        }
        else if (mode == AddressingMode::IndirectY)
        {
            // *(lower) + Y
            // �L�����[�͖��� = �I�[�o�[�t���[���Ă��C�ɂ��Ȃ�
            uint8_t lowerAddr = m_CpuBus.ReadByte(PC + 1);
            uint8_t upperAddr = lowerAddr + 1;
            // Indirect �Ȃ̂ŁAFetchAddr ����1��Q�Ƃ𔍂���
            uint16_t lower = m_CpuBus.ReadByte(lowerAddr);
            uint16_t upper = m_CpuBus.ReadByte(upperAddr);

            uint16_t addr = lower | (upper << 8);
            uint16_t beforeAddr = addr;

            addr += Y;

            *pOutAddr = addr;
            // �y�[�W�N���X�� +1 �N���b�N
            if ((beforeAddr & 0xFF00) != (addr & 0xFF00))
            {
                *pOutAdditionalCyc = 1;
            }
        }
        else if (mode == AddressingMode::Indirect)
        {
            // **(addr)
            uint16_t indirectAddr = 0;
            uint8_t indirectLower = 0;
            uint8_t indirectUpper = 0;

            indirectLower = m_CpuBus.ReadByte(PC + 1);
            indirectUpper = m_CpuBus.ReadByte(PC + 2);

            // �C���N�������g�ɂ����ĉ��ʃo�C�g����̃L�����[�𖳎����邽�߂ɁA���ʃo�C�g�ɉ��Z���Ă���L���X�g����(�ق�܂��H�H�H�H�H)
            // �����Ȃ������̉��Z�̃I�[�o�[�t���[���̋��������҂��Ă���̂ŁA����`����(TODO: ���ׂ�)
            uint8_t indirectLower2 = indirectLower + 1;

            // Indirect �Ȃ̂ŁAFetchAddr ����1��Q�Ƃ𔍂���
            uint16_t addrLower = m_CpuBus.ReadByte(static_cast<uint16_t>(indirectLower) | (static_cast<uint16_t>(indirectUpper) << 8));
            uint16_t addrUpper = m_CpuBus.ReadByte(static_cast<uint16_t>(indirectLower2) | (static_cast<uint16_t>(indirectUpper) << 8));

            uint16_t addr = addrLower | (addrUpper << 8);
            *pOutAddr = addr;
        }
        else
        {
            // unexpected default
            abort();
        }
    }

    // �A�h���b�V���O���[�h�Ƃ��܂� PC �̒l���疽�߂ł�������(�H)���擾����
    void Cpu::FetchArg(AddressingMode mode, uint8_t* pOutValue, uint8_t* pOutAdditionalCyc)
    {
        // �����������Ȃ��A�h���b�V���O���[�h�ŌĂ΂ꂽ��v���O���~���O�~�X�Ȃ̂� assert ���Ƃ�
        assert(mode != AddressingMode::Implied);

        *pOutValue = 0;
        *pOutAdditionalCyc = 0;

        if (mode == AddressingMode::Accumulator)
        {
            *pOutValue = A;
        }
        else if (mode == AddressingMode::Immediate)
        {
            // Immediate �� PC + 1 ����f���ɓǂ�
            *pOutValue = m_CpuBus.ReadByte(PC + 1);
        }
        else
        {
            // ���̓A�h���X���I�y�����h�ɂȂ��Ă�͂��Ȃ̂ŁA�A�h���X�������Ă���1��Q�Ƃ𔍂���(Indirect ��2��Q�Ƃ𔍂����K�v�����邪�A1��� FetchAddr ���Ŕ������Ă���)
            uint16_t addr = 0;
            FetchAddr(mode, &addr, pOutAdditionalCyc);
            *pOutValue = m_CpuBus.ReadByte(addr);
        }
    }

    void Cpu::Interrupt(InterruptType type)
    {
        uint16_t lower = 0;
        uint16_t upper = 0;

        // ���荞�݃t���O�����Ă�
        SetInterruptFlag(true);

        // TODO: RESET �ȊO����������
        if (type != nes::detail::InterruptType::RESET)
        {
            // TORIAEZU: RESET �ȊO�̓A�{�[�g
            abort();
        }

        switch (type)
        {
        case nes::detail::InterruptType::NMI:
            break;
        case nes::detail::InterruptType::RESET:
            lower = m_CpuBus.ReadByte(0xFFFC);
            upper = m_CpuBus.ReadByte(0xFFFD);

            // https://www.pagetable.com/?p=410
            SP = 0xFD;

            PC = lower | (upper << 8);
            break;
        case nes::detail::InterruptType::IRQ:
            break;
        case nes::detail::InterruptType::BRK:
            break;
        default:
            break;
        }
    }

    // stack �� 0x0100-0x01FF �ł��邱�ƂɋC��t����
    void Cpu::PushStack(uint8_t data)
    {
        m_CpuBus.WriteByte(SP | (1 << 8), data);
        SP--;
    }

    uint8_t Cpu::PopStack()
    {
        SP++;
        return m_CpuBus.ReadByte(SP | (1 << 8));
    }

    uint8_t Cpu::Run()
    {
        // ���� �t�F�b�`
        uint8_t instByte = m_CpuBus.ReadByte(PC);
        Instruction inst = ByteToInstruction(instByte);
        // TODO: ���ߎ��s�O�ɖ��߂� disas �ƍ��̏�Ԃ����O�ɏo��

        switch (inst.m_Opcode)
        {
        case Opcode::ADC:
        {
            // �I�y�����h �t�F�b�`
            uint8_t operand;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &operand, &additionalCyc);

            uint16_t calc = static_cast<uint16_t>(A) + operand + GetCarryFlag();
            uint8_t res = static_cast<uint8_t>(calc);

            SetCarryFlag(calc > 0xff);
            SetZeroFlag(res == 0);
            SetNegativeFlag((res & 0x80) == 0x80);
            // http://forums.nesdev.com/viewtopic.php?t=6331
            SetOverflowFlag(isSignedOverFlowed(A, operand, GetCarryFlag()));

            A = res;
            // PC �i�߂�
            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::AND:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = A & arg;
            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            A = res;
            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::ASL:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = arg << 1;

            // MSB �������Ă鎞�ɍ��V�t�g������ carry �ɂȂ�
            bool carryFlag = (arg & 0x80) == 0x80;
            bool zeroFlag = (res == 0);
            bool negativeFlag = (res & 0x80) == 0x80;

            SetCarryFlag(carryFlag);
            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            if (inst.m_AddressingMode == AddressingMode::Accumulator)
            {
                A = res;
            }
            else
            {
                uint16_t addr;
                uint8_t dummy;
                FetchAddr(inst.m_AddressingMode, &addr, &dummy);
                m_CpuBus.WriteByte(addr, res);
            }

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::BCC:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            // �L�����[�t���O�������ĂȂ������番��
            if (!GetCarryFlag())
            {
                PC = addr;
                // ���򐬗����� + 1 �N���b�N�T�C�N��
                return inst.m_Cycles + additionalCyc + 1;
            }
            else
            {
                PC += inst.m_Bytes;
                return inst.m_Cycles + additionalCyc;
            }
        }
        case Opcode::BCS:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            if (GetCarryFlag())
            {
                PC = addr;
                return inst.m_Cycles + additionalCyc + 1;
            }
            else
            {
                PC += inst.m_Bytes;
                return inst.m_Cycles + additionalCyc;
            }
        }
        case Opcode::BEQ:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            if (GetZeroFlag())
            {
                PC = addr;
                return inst.m_Cycles + additionalCyc + 1;
            }
            else
            {
                PC += inst.m_Bytes;
                return inst.m_Cycles + additionalCyc;
            }
        }
        case Opcode::BIT:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            bool negativeFlag = (arg & 0x80) == 0x80;
            bool overflowFlag = (arg & 0x40) == 0x40;
            // Set if the result if the AND is zero(�H�H�H�H�H�H�H)
            bool zeroFlag = (A & arg) == 0;

            SetNegativeFlag(negativeFlag);
            SetOverflowFlag(overflowFlag);
            SetZeroFlag(zeroFlag);

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::BMI:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            if (GetNegativeFlag())
            {
                PC = addr;
                return inst.m_Cycles + additionalCyc + 1;
            }
            else
            {
                PC += inst.m_Bytes;
                return inst.m_Cycles + additionalCyc;
            }
        }
        case Opcode::BNE:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            if (!GetZeroFlag())
            {
                PC = addr;
                return inst.m_Cycles + additionalCyc + 1;
            }
            else
            {
                PC += inst.m_Bytes;
                return inst.m_Cycles + additionalCyc;
            }
        }
        case Opcode::BPL:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            if (!GetNegativeFlag())
            {
                PC = addr;
                return inst.m_Cycles + additionalCyc + 1;
            }
            else
            {
                PC += inst.m_Bytes;
                return inst.m_Cycles + additionalCyc;
            }
        }
        case Opcode::BRK:
        {
            SetBreakFlag(true);
            Interrupt(InterruptType::BRK);
            return inst.m_Cycles;
        }
        case Opcode::BVC:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            if (!GetOverflowFlag())
            {
                PC = addr;
                return inst.m_Cycles + additionalCyc + 1;
            }
            else
            {
                PC += inst.m_Bytes;
                return inst.m_Cycles + additionalCyc;
            }
        }
        case Opcode::BVS:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            if (GetOverflowFlag())
            {
                PC = addr;
                return inst.m_Cycles + additionalCyc + 1;
            }
            else
            {
                PC += inst.m_Bytes;
                return inst.m_Cycles + additionalCyc;
            }
        }
        case Opcode::CLC:
        {
            SetCarryFlag(false);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::CLD:
        {
            SetDecimalFlag(false);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::CLI:
        {
            SetInterruptFlag(false);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::CLV:
        {
            SetOverflowFlag(false);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::CMP:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = A - arg;

            bool carryFlag = A >= arg;
            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetCarryFlag(carryFlag);
            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::CPX:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = X - arg;

            bool carryFlag = X >= arg;
            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetCarryFlag(carryFlag);
            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::CPY:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = Y - arg;

            bool carryFlag = Y >= arg;
            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetCarryFlag(carryFlag);
            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::DEC:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            uint16_t addr;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = arg - 1;

            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            m_CpuBus.WriteByte(addr, res);

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::DEX:
        {
            // implied �̂�
            uint8_t res = X - 1;

            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            X = res;

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::DEY:
        {
            // implied �̂�
            uint8_t res = Y - 1;

            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            Y = res;

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::EOR:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = A ^ arg;
            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            A = res;

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::INC:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            uint16_t addr;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = arg + 1;

            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            m_CpuBus.WriteByte(addr, res);

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::INX:
        {
            // implied �̂�
            uint8_t res = X + 1;

            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            X = res;

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::INY:
        {
            // implied �̂�
            uint8_t res = Y + 1;

            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            Y = res;

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::JMP:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);
            assert(additionalCyc == 0);

            PC = addr;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::JSR:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);
            assert(additionalCyc == 0);

            // ���^�[���A�h���X�� PC + 3 �����A���ꂩ�� 1 �����������̂� stack �Ƀv�b�V������(���������d�l)
            uint16_t retAddr = PC + 2;

            // upper -> lower �̏��� push
            PushStack(static_cast<uint8_t>(retAddr >> 8));
            PushStack(static_cast<uint8_t>(retAddr & 0xFF));

            PC = addr;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::LDA:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            bool zeroFlag = arg == 0;
            bool negativeFlag = (arg & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            A = arg;

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::LDX:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            bool zeroFlag = arg == 0;
            bool negativeFlag = (arg & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            X = arg;

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::LDY:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            bool zeroFlag = arg == 0;
            bool negativeFlag = (arg & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            Y = arg;

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::LSR:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            uint16_t addr = 0;;

            if (inst.m_AddressingMode != AddressingMode::Accumulator)
            {
                FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);
            }
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = arg >> 1;

            bool carryFlag = (arg & 1) == 1;
            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetCarryFlag(carryFlag);
            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            if (inst.m_AddressingMode == AddressingMode::Accumulator)
            {
                A = res;
            }
            else 
            {
                m_CpuBus.WriteByte(addr, res);
            }

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::NOP:
        {
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::ORA:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = A | arg;

            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) & res;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            A = res;

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::PHA:
        {
            PushStack(A);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::PHP:
        {
            // http://wiki.nesdev.com/w/index.php/Status_flags: P �� 4bit �ڂ� 5bit �ڂ𗧂ĂăX�^�b�N�Ƀv�b�V��
            PushStack(P | B_FLAG_MASK);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::PLA:
        {
            uint8_t res = PopStack();

            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);
            A = res;

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::PLP:
        {
            uint8_t res = PopStack();

            // http://wiki.nesdev.com/w/index.php/Status_flags: P�� 4bit �ڂ� 5bit �ڂ͍X�V���Ȃ�
            P = (res & ~B_FLAG_MASK) | (P & B_FLAG_MASK);

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::ROL:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            uint16_t addr = 0;

            if (inst.m_AddressingMode != AddressingMode::Accumulator)
            {
                FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);
            }
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = arg << 1;
            res |= GetCarryFlag() ? 1 : 0;

            bool carryFlag = (arg & 0x80) == 0x80;
            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetCarryFlag(carryFlag);
            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            if (inst.m_AddressingMode == AddressingMode::Accumulator)
            {
                A = res;
            }
            else 
            {
                m_CpuBus.WriteByte(addr, res);
            }

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::ROR:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            uint16_t addr = 0;;

            if (inst.m_AddressingMode != AddressingMode::Accumulator)
            {
                FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);
            }
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            uint8_t res = arg >> 1;
            res |= GetCarryFlag() ? 0x80 : 0;

            bool carryFlag = (arg & 1) == 1;
            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            SetCarryFlag(carryFlag);
            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            if (inst.m_AddressingMode == AddressingMode::Accumulator)
            {
                A = res;
            }
            else
            {
                m_CpuBus.WriteByte(addr, res);
            }

            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::RTI:
        {
            uint8_t res = PopStack();

            // http://wiki.nesdev.com/w/index.php/Status_flags: P�� 4bit �ڂ� 5bit �ڂ͍X�V���Ȃ�
            P = (res & ~B_FLAG_MASK) | (P & B_FLAG_MASK);

            uint16_t lower = PopStack();
            uint16_t upper = PopStack();
            PC = lower | (upper << 8);
            
            return inst.m_Cycles;
        }
        case Opcode::RTS:
        {
            uint16_t lower = PopStack();
            uint16_t upper = PopStack();
            PC = lower | (upper << 8);

            // JSR �ŃX�^�b�N�Ƀv�b�V�������A�h���X�� JSR �̍Ō�̃A�h���X�ŁARTS ���ŃC���N�������g�����
            PC++;
            return inst.m_Cycles;
        }
        case Opcode::SBC:
        {
            uint8_t arg;
            uint8_t additionalCyc;
            FetchArg(inst.m_AddressingMode, &arg, &additionalCyc);

            // �����Z�ɕϊ�
            // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html#:~:text=The%20definition%20of%20the%206502,fit%20into%20a%20signed%20byte.&text=For%20each%20set%20of%20input,and%20the%20overflow%20bit%20V.
            // A - arg - borrow == A + ~arg + carry

            arg = ~arg;

            uint16_t calc = static_cast<int16_t>(A) + arg + GetCarryFlag();
            uint8_t res = static_cast<uint8_t>(calc);

            // �����Z�ɕϊ� �����̂ŁA�����Z�Ɠ����悤�Ƀt���O�v�Z�\
            bool overflowFlag = isSignedOverFlowed(A, arg, GetCarryFlag());
            bool carryFlag = calc > 0xff;
            bool negativeFlag = (res & 0x80) == 0x80;
            bool zeroFlag = res == 0;

            SetOverflowFlag(overflowFlag);
            SetCarryFlag(carryFlag);
            SetNegativeFlag(negativeFlag);
            SetZeroFlag(zeroFlag);

            A = res;
            PC += inst.m_Bytes;
            return inst.m_Cycles + additionalCyc;
        }
        case Opcode::SEC:
        {
            SetCarryFlag(true);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::SED:
        {
            SetDecimalFlag(true);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::SEI:
        {
            SetInterruptFlag(true);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::STA:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            m_CpuBus.WriteByte(addr, A);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::STX:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            m_CpuBus.WriteByte(addr, X);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::STY:
        {
            uint16_t addr;
            uint8_t additionalCyc;
            FetchAddr(inst.m_AddressingMode, &addr, &additionalCyc);

            m_CpuBus.WriteByte(addr, Y);
            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::TAX:
        {
            bool zeroFlag = A == 0;
            bool negativeFlag = (A & 0x80) == 0x80;

            X = A;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::TAY:
        {
            bool zeroFlag = A == 0;
            bool negativeFlag = (A & 0x80) == 0x80;

            Y = A;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::TSX:
        {
            uint8_t res = static_cast<uint8_t>(SP & 0xFF);

            bool zeroFlag = res == 0;
            bool negativeFlag = (res & 0x80) == 0x80;

            X = res;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::TXA:
        {
            bool zeroFlag = X == 0;
            bool negativeFlag = (X & 0x80) == 0x80;

            A = X;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::TXS:
        {
            // 1 Byte �����g��Ȃ�
            SP = static_cast<uint16_t>(X);

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }
        case Opcode::TYA:
        {
            bool zeroFlag = Y == 0;
            bool negativeFlag = (Y & 0x80) == 0x80;

            A = Y;

            SetZeroFlag(zeroFlag);
            SetNegativeFlag(negativeFlag);

            PC += inst.m_Bytes;
            return inst.m_Cycles;
        }


        default:
            // unexpected default
            abort();
            break;
        }
        return 0;
    }

    CpuInfo Cpu::GetCpuInfoForDebug()
    {
        // ���܂� PC ���� ���� �擾
        uint8_t opcode = m_CpuBus.ReadByte(PC);
        Instruction inst = ByteToInstruction(opcode);
        uint8_t instBytes[3];

        assert(inst.m_Bytes <= 3);
        for (int i = 0; i < inst.m_Bytes; i++)
        {
            uint8_t byte = m_CpuBus.ReadByte(i + PC);
            instBytes[i] = byte;
        }

        return CpuInfo(A, X, Y, PC, SP, P, inst, instBytes, sizeof(instBytes));
    }

    void Cpu::SetPCForDebug(uint16_t newPC)
    {
        PC = newPC;
    }
}}