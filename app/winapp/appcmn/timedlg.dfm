object TimeDialog: TTimeDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Time'
  ClientHeight = 143
  ClientWidth = 170
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
  object Message: TLabel
    Left = 8
    Top = 8
    Width = 153
    Height = 127
    Alignment = taCenter
    AutoSize = False
    Caption = 'message'
  end
  object BtnOk: TButton
    Left = 134
    Top = 118
    Width = 35
    Height = 25
    Caption = 'OK'
    ModalResult = 1
    TabOrder = 0
  end
end
