object ConfDialog: TConfDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Confirmation'
  ClientHeight = 95
  ClientWidth = 315
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 4
    Top = 8
    Width = 305
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = 'File exists. Overwrite it?'
  end
  object Label2: TLabel
    Left = 4
    Top = 24
    Width = 305
    Height = 37
    Alignment = taCenter
    AutoSize = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clGray
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object BtnOverwrite: TButton
    Left = 82
    Top = 65
    Width = 75
    Height = 29
    Caption = '&Overwrite'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ModalResult = 1
    ParentFont = False
    TabOrder = 1
  end
  object BtnCancel: TButton
    Left = 158
    Top = 65
    Width = 75
    Height = 29
    Caption = '&Cancel'
    Default = True
    ModalResult = 2
    TabOrder = 0
  end
end
