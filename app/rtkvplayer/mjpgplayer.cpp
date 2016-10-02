//---------------------------------------------------------------------------

#include "mjpgplayer.h"

#define JPG_MARKER		0xFF
#define JPG_FRM_START	0xD8
#define JPG_FRM_END		0xD9
#define TIMETAGH_LEN	60

//---------------------------------------------------------------------------
__fastcall TMjpgPlayer::TMjpgPlayer()
{
	LogSize = TagSize = 0;
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
	if (!OpenLog(file)) return 0;
	OpenTag(file);
	return 1;
}
//---------------------------------------------------------------------------
int __fastcall TMjpgPlayer::OpenLog(UnicodeString file)
{
	AnsiString ansi_file = file;
	FILE *fp;
	unsigned char buff[2] = {0};
	size_t file_size;
	
	if (!(fp = fopen(ansi_file.c_str(), "rb"))) return 0;
	
	fread(buff, 2, 1, fp);
	
	if (buff[0] != JPG_FRM_START || buff[1] != JPG_MARKER) {
		fclose(fp);
		return 0;
	}
	if (fseek(fp, 0, SEEK_END)) {
		fclose(fp);
		return 0;
	}
	LogSize = (size_t)ftell(fp);
	
	if (!(LogData = (unsigned char *)malloc(LogSize))) {
		fclose(fp);
		LogSize = 0;
		return 0;
	}
	rewind(fp);
	
	if (fread(LogData, LogSize, 1, fp) < 1) {
		free(LogData);
		fclose(fp);
		LogSize = 0;
		LogData = NULL;
		return 0;
	}
	fclose(fp);
	return 1;
}
//---------------------------------------------------------------------------
int __fastcall TMjpgPlayer::OpenTag(UnicodeString file)
{
	AnsiString ansi_file = file+".tag";
	FILE *fp;
	unsigned char buff[80] = {0};
	size_t file_size;
	
	if (!(fp = fopen(ansi_file.c_str(), "rb"))) return 1;
	
	fread(buff, 80, 1, fp);
	
	if (strncmp(buff, "TIMETAG RTKLIB", 14)) {
		fclose(fp);
		return 0;
	}
	if (fseek(fp, 0, SEEK_END)) {
		fclose(fp);
		return 0;
	}
	TagSize = ftell(fp);
	
	if (!(TagData = (unsigned char *)malloc(TagSize))) {
		fclose(fp);
		TagSize = 0;
		return 0;
	}
	rewind(fp);
	
	if (fread(TagData, TagSize, 1, fp) < 1) {
		free(TagData);
		TagSize = 0;
		TagData = NULL;
	}
	fclose(fp);
	return 1;
}
//---------------------------------------------------------------------------
void __fastcall TMjpgPlayer::ClearVideo(void)
{
	return;
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
