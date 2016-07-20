object StartDialog: TStartDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Start Time'
  ClientHeight = 88
  ClientWidth = 247
  Color = clBtnFace
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
  object BtnOk: TButton
    Left = 46
    Top = 58
    Width = 79
    Height = 27
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 130
    Top = 58
    Width = 79
    Height = 27
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 247
    Height = 55
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 2
    object Label1: TLabel
      Left = 10
      Top = 8
      Width = 227
      Height = 13
      Caption = 'Input Approx. Log Start Time (GPST)'
    end
    object TimeY1: TEdit
      Left = 10
      Top = 27
      Width = 65
      Height = 21
      TabOrder = 0
      Text = '2000/01/01'
    end
    object TimeY1UD: TUpDown
      Left = 76
      Top = 26
      Width = 19
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 1
      Wrap = True
      OnChangingEx = TimeY1UDChangingEx
    end
    object TimeH1: TEdit
      Left = 100
      Top = 27
      Width = 55
      Height = 21
      TabOrder = 2
      Text = '00:00:00'
    end
    object TimeH1UD: TUpDown
      Left = 156
      Top = 26
      Width = 19
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 3
      Wrap = True
      OnChangingEx = TimeH1UDChangingEx
    end
    object BtnFileTime: TButton
      Left = 178
      Top = 26
      Width = 59
      Height = 23
      Caption = 'File Time'
      TabOrder = 4
      OnClick = BtnFileTimeClick
    end
  end
end
