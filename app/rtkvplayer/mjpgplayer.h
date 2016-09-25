//---------------------------------------------------------------------------

#ifndef mjpgplayerH
#define mjpgplayerH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include "mjpgplayer.h"

//---------------------------------------------------------------------------
class TMjpgPlayer
{
private:
    int Width, Height;
    int CurrentFrm, NumFrm;
    double FPS;
public:
    int __fastcall OpenVideo(UnicodeString file);
    void __fastcall ClearVideo(void);
    int  __fastcall PlayVideo(void);
    void __fastcall StopVideo(void);
    void __fastcall SetVideoPos(float pos);
    float __fastcall GetVideoPos(void);
    void __fastcall GetVideoTime(double &time, double &period);
    void __fastcall GetVideoSize(int &width, int &height);
    void __fastcall UpdateVideo(void);
    __fastcall TMjpgPlayer();
    __fastcall ~TMjpgPlayer();
};

#endif
