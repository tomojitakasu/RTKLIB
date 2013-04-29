object StartDialog: TStartDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Start Time'
  ClientHeight = 83
  ClientWidth = 224
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
    Left = 34
    Top = 58
    Width = 79
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 116
    Top = 58
    Width = 79
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 224
    Height = 55
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 2
    object Label2: TLabel
      Left = 184
      Top = 30
      Width = 25
      Height = 13
      Caption = 'GPST'
    end
    object Label1: TLabel
      Left = 18
      Top = 8
      Width = 188
      Height = 13
      Caption = 'Input Approx. Log Start Time for RTCM'
    end
    object TimeY1: TEdit
      Left = 16
      Top = 28
      Width = 65
      Height = 21
      TabOrder = 0
      Text = '2000/01/01'
    end
    object TimeY1UD: TUpDown
      Left = 82
      Top = 27
      Width = 17
      Height = 20
      Min = -32000
      Max = 32000
      TabOrder = 1
      Wrap = True
      OnChangingEx = TimeY1UDChangingEx
    end
    object TimeH1: TEdit
      Left = 106
      Top = 28
      Width = 55
      Height = 21
      TabOrder = 2
      Text = '00:00:00'
    end
    object TimeH1UD: TUpDown
      Left = 162
      Top = 27
      Width = 17
      Height = 20
      Min = -32000
      Max = 32000
      TabOrder = 3
      Wrap = True
      OnChangingEx = TimeH1UDChangingEx
    end
  end
end
