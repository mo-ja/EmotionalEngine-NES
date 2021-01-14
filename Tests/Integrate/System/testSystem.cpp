#include <cassert>
#include <cassette.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <testUtil.h>
#include <System.h>
#include <Cpu.h>

// HELLO WORLD ���� ROM �ǂݍ���
void ReadHelloWorldNes(std::shared_ptr<uint8_t[]>* pOutBuf, size_t* pOutSize)
{
    auto rootPath = test::GetRepositoryRootPath();
    assert(rootPath);

    auto nesFile = rootPath.value();
    nesFile += "/Tests/TestBinaries/helloworld/sample1/sample1.nes";

    test::ReadFile(nesFile, pOutBuf, pOutSize);
}
// nestest.nes �ǂݍ���
void ReadNesTestNes(std::shared_ptr<uint8_t[]>* pOutBuf, size_t* pOutSize)
{
    auto rootPath = test::GetRepositoryRootPath();
    assert(rootPath);

    auto nesFile = rootPath.value();
    nesFile += "/Tests/TestBinaries/nestest/nestest.nes";

    test::ReadFile(nesFile, pOutBuf, pOutSize);
}

// �e�A�h���X��ǂݏ�������A�Ƃ肠���� WRAM �ƃJ�Z�b�g ����
void TestSystem_ReadWrite()
{
    std::cout << "====" << __FUNCTION__ << "====\n";
    std::shared_ptr<uint8_t[]> rom;
    size_t size;
    ReadHelloWorldNes(&rom, &size);
    nes::detail::System sys(rom.get(), size);
    nes::detail::CpuBus bus(&sys);

    // WRAM �ǂݏ����ł���
    // �����l�� 0
    assert(bus.ReadByte(0) == 0);
    // �����Ă݂�
    bus.WriteByte(0, 0x41);

    // ���������̂��ǂ߂�
    assert(bus.ReadByte(0) == 0x41);
    // �~���[ �ǂ߂�
    assert(bus.ReadByte(0x0800) == 0x41);
    // �~���[ ������
    bus.WriteByte(0x0800, 0x42);
    // �~���[�ɏ������񂾂��̂����̏ꏊ�����߂�
    assert(bus.ReadByte(0) == 0x42);

    // �J�Z�b�g �ǂ߂�(PRG-ROM �� 1 byte��)
    assert(bus.ReadByte(0x8000) == 0x78);
    // �J�Z�b�g ������(�H)
    bus.WriteByte(0x8000, 0x10);
    // �J�Z�b�g ���������ʂ����f����Ă�
    assert(bus.ReadByte(0x8000) == 0x10);

    // RESET���荞�� �ǂ�ł݂�
    uint16_t reset = 0;
    reset |= static_cast<uint16_t>(bus.ReadByte(0xFFFD)) << 8;
    reset |= bus.ReadByte(0xFFFC);
    printf("Entry Point: 0x%hx\n", reset);

    std::cout << "====" << __FUNCTION__ << " END ====\n";
}

// Hello World
void TestSystem_HelloWorld()
{
    std::cout << "====" << __FUNCTION__ << "====\n";
    std::shared_ptr<uint8_t[]> rom;
    size_t size;
    ReadHelloWorldNes(&rom, &size);
    nes::detail::System sys(rom.get(), size);
    nes::detail::Cpu cpu(&sys);

    cpu.Interrupt(nes::detail::InterruptType::RESET);

    uint64_t clk = 7;
    uint64_t inst = 1;

    for (int i = 0; i < 175; i++) {
        auto info = cpu.GetCpuInfoForDebug();
        test::LogCpuStatusFceuxStyle(&info, clk, inst);
        clk += cpu.Run();
        inst++;
    }

    std::cout << "====" << __FUNCTION__ << " END ====\n";
}

// nestest.nes
void TestSystem_NesTest()
{
    //std::cout << "====" << __FUNCTION__ << "====\n";
    std::shared_ptr<uint8_t[]> rom;
    size_t size;
    ReadNesTestNes(&rom, &size);
    nes::detail::System sys(rom.get(), size);
    nes::detail::Cpu cpu(&sys);

    cpu.Interrupt(nes::detail::InterruptType::RESET);

    // CPU �����Ŏ��s���邽�߂ɁA PC �� 0xC000 �ɃZ�b�g
    cpu.SetPCForDebug(0xC000);

    uint64_t clk = 7;
    uint64_t inst = 1;

    // official ����: 5003 �܂ŁA unofficial ����: 8991�܂�
    for (int i = 0; i < 8991; i++) {
        auto info = cpu.GetCpuInfoForDebug();
        test::LogCpuStatusNesTestStyle(&info, clk, inst);
        clk += cpu.Run();
        inst++;
    }

    std::cout << "====" << __FUNCTION__ << " END ====\n";
}

int main()
{
    //TestSystem_ReadWrite();
    //TestSystem_HelloWorld();
    TestSystem_NesTest();

    return 0;
}