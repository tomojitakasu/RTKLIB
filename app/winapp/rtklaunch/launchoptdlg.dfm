object LaunchOptDialog: TLaunchOptDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 129
  ClientWidth = 223
  Color = clWhite
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object OptMkl: TRadioButton
    Left = 15
    Top = 30
    Width = 172
    Height = 18
    Caption = 'RTKPOST_MKL, RTKNAVI_MKL'
    TabOrder = 3
  end
  object OptWin64: TRadioButton
    Left = 15
    Top = 52
    Width = 201
    Height = 18
    Caption = 'RTKPOST_WIN64, RTKNAVI_WIN64'
    TabOrder = 4
  end
  object BtnCancel: TButton
    Left = 118
    Top = 96
    Width = 95
    Height = 29
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BtnOk: TButton
    Left = 15
    Top = 96
    Width = 95
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object OptNormal: TRadioButton
    Left = 15
    Top = 8
    Width = 189
    Height = 17
    Caption = 'Normal APs (32 bit without MKL)'
    TabOrder = 2
  end
  object Minimize: TCheckBox
    Left = 15
    Top = 73
    Width = 97
    Height = 17
    Caption = 'Minimize'
    TabOrder = 5
  end
end
