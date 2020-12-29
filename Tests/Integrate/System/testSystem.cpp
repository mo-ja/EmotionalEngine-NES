#include <cassert>
#include <cassette.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <testUtil.h>
#include <System.h>

// HELLO WORLD ���� ROM �ǂݍ���
void ReadHelloWorldNes(std::shared_ptr<uint8_t[]>* pOutBuf, size_t* pOutSize)
{
    auto rootPath = test::GetRepositoryRootPath();
    assert(rootPath);

    auto nesFile = rootPath.value();
    nesFile += "/Tests/TestBinaries/helloworld/sample1/sample1.nes";

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

int main()
{
    TestSystem_ReadWrite();

    return 0;
}