//---------------------------------------------------------------------------

#include "mjpgplayer.h"

#define JPG_MARKER      0xFF
#define JPG_FRM_START   0xD8
#define JPG_FRM_END     0xD9

//---------------------------------------------------------------------------
__fastcall TMjpgPlayer::TMjpgPlayer()
{
    CurrentFrm = NumFrm = Width = Height = 0;
    FPS = 10.0;
}
//---------------------------------------------------------------------------
__fastcall TMjpgPlayer::~TMjpgPlayer()
{
}
//---------------------------------------------------------------------------
int __fastcall TMjpgPlayer::OpenVideo(UnicodeString file)
{
    return 0;
}
//---------------------------------------------------------------------------
void __fastcall TMjpgPlayer::ClearVideo(void)
{
}
//---------------------------------------------------------------------------
int __fastcall TMjpgPlayer::PlayVideo(void)
{
    return 0;
}
//---------------------------------------------------------------------------
void __fastcall TMjpgPlayer::StopVideo(void)
{
}
//---------------------------------------------------------------------------
float __fastcall TMjpgPlayer::GetVideoPos(void)
{
    return NumFrm <= 1 ? 0.0f : (float)CurrentFrm/(NumFrm-1);
}
//---------------------------------------------------------------------------
void __fastcall TMjpgPlayer::SetVideoPos(float pos)
{
}
//---------------------------------------------------------------------------
void __fastcall TMjpgPlayer::GetVideoTime(double &time, double &period)
{
    time = CurrentFrm/FPS;
    period = NumFrm <=1 ? 0.0 : (NumFrm-1)/FPS;
}
//---------------------------------------------------------------------------
void __fastcall TMjpgPlayer::GetVideoSize(int &width, int &height)
{
    width = Width;
    height = Height;
}
//---------------------------------------------------------------------------
void __fastcall TMjpgPlayer::UpdateVideo(void)
{
}
//---------------------------------------------------------------------------
