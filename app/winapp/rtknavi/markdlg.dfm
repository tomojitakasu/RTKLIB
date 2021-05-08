object MarkDialog: TMarkDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Add Mark'
  ClientHeight = 263
  ClientWidth = 345
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
  object Label2: TLabel
    Left = 14
    Top = 55
    Width = 45
    Height = 13
    Caption = 'Comment'
  end
  object LabelPosMode: TLabel
    Left = 14
    Top = 141
    Width = 114
    Height = 13
    Caption = 'Switch Positioning Mode'
  end
  object Label1: TLabel
    Left = 288
    Top = 9
    Width = 41
    Height = 13
    Caption = '%r=001'
  end
  object BtnRepDlg: TSpeedButton
    Left = 236
    Top = 6
    Width = 36
    Height = 19
    Caption = '?'
    Flat = True
    OnClick = BtnRepDlgClick
  end
  object LabelPos: TLabel
    Left = 15
    Top = 183
    Width = 112
    Height = 13
    Caption = 'Lat/Lon/Height (deg/m)'
  end
  object BtnOk: TButton
    Left = 116
    Top = 228
    Width = 108
    Height = 32
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 226
    Top = 228
    Width = 108
    Height = 32
    Caption = '&Cancel'
    Default = True
    ModalResult = 2
    TabOrder = 0
    OnClick = BtnCancelClick
  end
  object RadioStop: TRadioButton
    Left = 125
    Top = 157
    Width = 91
    Height = 17
    Caption = 'STOP (Static)'
    TabOrder = 5
    OnClick = RadioGoClick
  end
  object RadioGo: TRadioButton
    Left = 14
    Top = 157
    Width = 96
    Height = 17
    Caption = 'GO (Kinematic)'
    TabOrder = 4
    OnClick = RadioGoClick
  end
  object MarkerName: TComboBox
    Left = 12
    Top = 25
    Width = 321
    Height = 21
    TabOrder = 3
  end
  object ChkMarkerName: TCheckBox
    Left = 12
    Top = 7
    Width = 97
    Height = 17
    Caption = 'Marker Name'
    TabOrder = 2
    OnClick = ChkMarkerNameClick
  end
  object MarkerComment: TEdit
    Left = 12
    Top = 69
    Width = 321
    Height = 60
    AutoSelect = False
    AutoSize = False
    TabOrder = 6
  end
  object RadioFix: TRadioButton
    Left = 233
    Top = 157
    Width = 91
    Height = 17
    Caption = 'FIX (Fixed)'
    Enabled = False
    TabOrder = 7
    OnClick = RadioGoClick
  end
  object EditLat: TEdit
    Left = 13
    Top = 198
    Width = 97
    Height = 21
    TabOrder = 8
    Text = '0.00000000'
  end
  object EditLon: TEdit
    Left = 112
    Top = 198
    Width = 101
    Height = 21
    TabOrder = 9
    Text = '0.00000000'
  end
  object EditHgt: TEdit
    Left = 215
    Top = 198
    Width = 93
    Height = 21
    TabOrder = 10
    Text = '0.0000'
  end
  object BtnPos: TButton
    Left = 309
    Top = 197
    Width = 25
    Height = 23
    Caption = '...'
    TabOrder = 11
    OnClick = BtnPosClick
  end
end
