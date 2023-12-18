#include <testUtil.h>
#include "DxLib.h"
#include "Nes.h"

namespace {
	// NES �̉�ʃT�C�Y
	const int NesGraphicWidth = 256;
	const int NesGraphicHeight = 240;

	// HELLO WORLD ���� ROM �ǂݍ���
	void ReadRom(std::shared_ptr<uint8_t[]>* pOutBuf, size_t* pOutSize)
	{
		auto rootPath = test::GetRepositoryRootPath();
		assert(rootPath);

		auto nesFile = rootPath.value();
		//nesFile += "/Tests/TestBinaries/helloworld/sample1/sample1.nes";
		// nesFile += "/Tests/TestBinaries/nestest/nestest.nes";
		//nesFile += "/Tests/TestBinaries/giko/giko011.nes";
		//nesFile += "/Tests/TestBinaries/rom/donkey.nes";
		// nesFile += "/Tests/TestBinaries/giko/giko013.nes";
		// nesFile += "/Tests/TestBinaries/rom/live_UNROM.nes";
		// nesFile += "/Tests/TestBinaries/rom/dpcm_test.nes";
		nesFile += "/Tests/TestBinaries/rom/mario.nes";

		test::ReadFile(nesFile, pOutBuf, pOutSize);
	}

	void DrawEmulatedPicture(int softImage, nes::Color result[][256])
	{
		BYTE* p = (BYTE*)GetImageAddressSoftImage(softImage);
		const int OffsetX = 64;

		for (int y = 0; y < 240 * 2; y++) {
			for (int x = 0; x < 256 * 2; x++) {
				auto& c = result[y/2][x/2];
				// B, G, R �̏��� https://dxlib.xsrv.jp/cgi/patiobbs/patio.cgi?mode=view&no=4620&p=2
				p[0] = c.Blue;
				p[1] = c.Green;
				p[2] = c.Red;

				// 1 px �� 3 byte
				p += 3;
			}
		}

		DrawSoftImage(OffsetX, 0, softImage);
		//DrawSoftImage(256, 0, softImage);
		//DrawSoftImage(0, 240, softImage);
		//DrawSoftImage(256, 240, softImage);
	}

	// �L�[���͂��G�~���ɓ����
	void InputKey(nes::Emulator* emu)
	{
		char keyInput[256];
		GetHitKeyStateAll(keyInput);

		// �ق�Ƃ͓��̗͂����オ�藧������������o���������A�G�~���̎����セ�����Ȃ��Ă������̂ŃT�{��(�萔�{�x���Ȃ邯��)
		if (keyInput[KEY_INPUT_LEFT] != 0) 
		{
			emu->PushButton(nes::PadId::Zero, nes::PadButton::LEFT);
		}
		else 
		{
			emu->ReleaseButton(nes::PadId::Zero, nes::PadButton::LEFT);
		}

		if (keyInput[KEY_INPUT_RIGHT] != 0) 
		{
			emu->PushButton(nes::PadId::Zero, nes::PadButton::RIGHT);
		}
		else 
		{
			emu->ReleaseButton(nes::PadId::Zero, nes::PadButton::RIGHT);
		}

		if (keyInput[KEY_INPUT_UP] != 0) 
		{
			emu->PushButton(nes::PadId::Zero, nes::PadButton::UP);
		}
		else 
		{
			emu->ReleaseButton(nes::PadId::Zero, nes::PadButton::UP);
		}

		if (keyInput[KEY_INPUT_DOWN] != 0) 
		{
			emu->PushButton(nes::PadId::Zero, nes::PadButton::DOWN);
		}
		else 
		{
			emu->ReleaseButton(nes::PadId::Zero, nes::PadButton::DOWN);
		}

		if (keyInput[KEY_INPUT_Z] != 0) 
		{
			emu->PushButton(nes::PadId::Zero, nes::PadButton::A);
		}
		else 
		{
			emu->ReleaseButton(nes::PadId::Zero, nes::PadButton::A);
		}

		if (keyInput[KEY_INPUT_X] != 0) 
		{
			emu->PushButton(nes::PadId::Zero, nes::PadButton::B);
		}
		else 
		{
			emu->ReleaseButton(nes::PadId::Zero, nes::PadButton::B);
		}

		if (keyInput[KEY_INPUT_S] != 0) 
		{
			emu->PushButton(nes::PadId::Zero, nes::PadButton::SELECT);
		}
		else 
		{
			emu->ReleaseButton(nes::PadId::Zero, nes::PadButton::SELECT);
		}

		if (keyInput[KEY_INPUT_A] != 0) 
		{
			emu->PushButton(nes::PadId::Zero, nes::PadButton::START);
		}
		else 
		{
			emu->ReleaseButton(nes::PadId::Zero, nes::PadButton::START);
		}
	}

	// Player �̃n���h��(�T�E���h)
	int g_SoftSoundPlayerHandle = -1;
	// �T���v�����O���g�� 44100 Hz
	const int SamplingFreq = 44100;
}

void InitializeSound()
{
	// �Ƃ肠�����A �`�����l����:1 �ʎq���r�b�g��:8bit �T���v�����O���g��:44KHz �ł���Ă݂�
	g_SoftSoundPlayerHandle = MakeSoftSoundPlayer1Ch8Bit44KHz();
}

void AddWaveSample(int sample) 
{
	// 3F �Ԃ�
	const int MaxSampleCount = SamplingFreq / 30;
	static int skipSampleCount = 0;

	int sampleCount = GetStockDataLengthSoftSoundPlayer(g_SoftSoundPlayerHandle);

	if (sampleCount >= MaxSampleCount) 
	{
		skipSampleCount++;
		skipSampleCount %= 80;

		// ���ӂ�Ă���T���v�����O��������Ƃւ炷
		if (skipSampleCount == 0) 
		{
			return;
		}
	}
	// �Ă��Ƃ�
	sample *= 10;
	AddOneDataSoftSoundPlayer(g_SoftSoundPlayerHandle, sample, sample);
}

// �v���O������ WinMain ����n�܂�܂�
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	// �G NES �G�~�� ����
	std::shared_ptr<uint8_t[]> rom;
	size_t size;
	ReadRom(&rom, &size);

	nes::Emulator emu(rom, size, AddWaveSample);

	if (ChangeWindowMode(true) != DX_CHANGESCREEN_OK) 
	{
		return -1;
	}

    if( DxLib_Init() == -1) 
	{
        return -1 ;
    }

	InitializeSound();

	// �`��o�b�t�@
	nes::Color result[NesGraphicHeight][NesGraphicWidth];

	// dxlib �œ��I�ɃC���[�W��`�悷�邽�߂ɂ������
	auto softImage = MakeRGB8ColorSoftImage(NesGraphicWidth * 2, NesGraphicHeight * 2);

	// �`���𗠉�ʂɂ���
	SetDrawScreen(DX_SCREEN_BACK);

	while (1) 
	{
		if (ProcessMessage() != 0) 
		{ // ���b�Z�[�W����
			break;//�E�B���h�E�́~�{�^���������ꂽ�烋�[�v�𔲂���
		}

		// �� �X�^�[�g
		if (CheckStartSoftSoundPlayer(g_SoftSoundPlayerHandle) == FALSE) 
		{
			StartSoftSoundPlayer(g_SoftSoundPlayerHandle);
		}

		nes::EmuInfo info;
		emu.GetEmuInfo(&info);
		if (info.CpuCycles % 50 == 0) {
			clsDx();
			int sampleCount = GetStockDataLengthSoftSoundPlayer(g_SoftSoundPlayerHandle);
			// �����ς��Ȃ��ꏊ�� printfDx ����ƂȂ����c����������̂ŎG�� NES �̉�ʂɔ��悤�ɗ��܂��Ă�T���v�������o���Ƃ�(�ň�)
			printfDx("       %d samples\n", sampleCount);
		}

		InputKey(&emu);
		emu.StepFrame();
		emu.GetPicture(result);

		DrawEmulatedPicture(softImage, result);

		ScreenFlip();
	}

    WaitKey();
    DxLib_End();
    return 0;
}