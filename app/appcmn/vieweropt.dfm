object ViewerOptDialog: TViewerOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Viewer Options'
  ClientHeight = 105
  ClientWidth = 190
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label6: TLabel
    Left = 14
    Top = 11
    Width = 50
    Height = 13
    Caption = 'Text Color'
  end
  object Label1: TLabel
    Left = 14
    Top = 33
    Width = 84
    Height = 13
    Caption = 'Background Color'
  end
  object Label15: TLabel
    Left = 14
    Top = 55
    Width = 22
    Height = 13
    Caption = 'Font'
  end
  object FontLabel: TLabel
    Left = 108
    Top = 55
    Width = 52
    Height = 13
    Alignment = taRightJustify
    Caption = 'Font Name'
  end
  object BtnOk: TButton
    Left = 18
    Top = 78
    Width = 79
    Height = 21
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 100
    Top = 78
    Width = 79
    Height = 21
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object Color1: TPanel
    Left = 101
    Top = 8
    Width = 61
    Height = 19
    BevelInner = bvRaised
    BevelOuter = bvLowered
    Color = clInfoText
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindow
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentBackground = False
    ParentFont = False
    TabOrder = 2
  end
  object BtnColor1: TButton
    Left = 164
    Top = 8
    Width = 16
    Height = 18
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    OnClick = BtnColor1Click
  end
  object Color2: TPanel
    Left = 101
    Top = 30
    Width = 61
    Height = 19
    BevelInner = bvRaised
    BevelOuter = bvLowered
    Color = clWindow
    ParentBackground = False
    TabOrder = 4
  end
  object BtnColor2: TButton
    Left = 164
    Top = 30
    Width = 16
    Height = 18
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 5
    OnClick = BtnColor2Click
  end
  object BtnFont: TButton
    Left = 164
    Top = 52
    Width = 16
    Height = 18
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 6
    OnClick = BtnFontClick
  end
  object ColorDialog: TColorDialog
    Left = 42
    Top = 50
  end
  object FontDialog: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Options = []
    Left = 72
    Top = 50
  end
end
