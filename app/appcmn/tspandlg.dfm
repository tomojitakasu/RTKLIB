object SpanDialog: TSpanDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Time Span/Interval'
  ClientHeight = 119
  ClientWidth = 303
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
  object BtnTime2: TSpeedButton
    Left = 281
    Top = 36
    Width = 13
    Height = 23
    Caption = '?'
    Flat = True
    OnClick = BtnTime2Click
  end
  object BtnTime1: TSpeedButton
    Left = 281
    Top = 10
    Width = 13
    Height = 23
    Caption = '?'
    Flat = True
    OnClick = BtnTime1Click
  end
  object BtnCancel: TButton
    Left = 207
    Top = 87
    Width = 87
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object BtnOk: TButton
    Left = 117
    Top = 87
    Width = 87
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object TimeStartF: TCheckBox
    Left = 10
    Top = 12
    Width = 105
    Height = 17
    Caption = 'Time Start (GPST)'
    TabOrder = 2
    OnClick = TimeStartFClick
  end
  object TimeEndF: TCheckBox
    Left = 10
    Top = 38
    Width = 107
    Height = 17
    Caption = 'Time End (GPST)'
    TabOrder = 7
    OnClick = TimeEndFClick
  end
  object TimeIntF: TCheckBox
    Left = 10
    Top = 64
    Width = 75
    Height = 17
    Caption = 'Interval (s)'
    TabOrder = 12
    OnClick = TimeIntFClick
  end
  object TimeY2: TEdit
    Left = 118
    Top = 36
    Width = 63
    Height = 21
    TabOrder = 8
    Text = '2001/01/01'
    OnKeyDown = TimeY2KeyDown
  end
  object TimeY1: TEdit
    Left = 118
    Top = 10
    Width = 63
    Height = 21
    TabOrder = 3
    Text = '2000/01/01'
    OnKeyDown = TimeY1KeyDown
  end
  object TimeY1UD: TUpDown
    Left = 182
    Top = 9
    Width = 21
    Height = 23
    Min = -32000
    Max = 32000
    TabOrder = 4
    Wrap = True
    OnChangingEx = TimeY1UDChangingEx
  end
  object TimeY2UD: TUpDown
    Left = 182
    Top = 35
    Width = 21
    Height = 23
    Min = -32000
    Max = 32000
    TabOrder = 9
    Wrap = True
    OnChangingEx = TimeY2UDChangingEx
  end
  object TimeH2: TEdit
    Left = 207
    Top = 36
    Width = 51
    Height = 21
    TabOrder = 10
    Text = '23:59:59'
    OnKeyDown = TimeH2KeyDown
  end
  object TimeH1: TEdit
    Left = 207
    Top = 10
    Width = 51
    Height = 21
    TabOrder = 5
    Text = '00:00:00'
    OnKeyDown = TimeH1KeyDown
  end
  object TimeH1UD: TUpDown
    Left = 259
    Top = 9
    Width = 21
    Height = 23
    Min = -32000
    Max = 32000
    TabOrder = 6
    Wrap = True
    OnChangingEx = TimeH1UDChangingEx
  end
  object TimeH2UD: TUpDown
    Left = 259
    Top = 35
    Width = 21
    Height = 23
    Min = -32000
    Max = 32000
    TabOrder = 11
    Wrap = True
    OnChangingEx = TimeH2UDChangingEx
  end
  object EditTimeInt: TComboBox
    Left = 118
    Top = 62
    Width = 85
    Height = 21
    DropDownCount = 16
    TabOrder = 13
    Text = '1'
    Items.Strings = (
      '0.1'
      '0.2'
      '0.25'
      '0.5'
      '1'
      '2'
      '5'
      '10'
      '15'
      '30'
      '60'
      '300'
      '600'
      '1800'
      '3600')
  end
end
