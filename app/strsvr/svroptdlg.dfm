object SvrOptDialog: TSvrOptDialog
  Left = 0
  Top = 0
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 276
  ClientWidth = 379
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
  object Label1: TLabel
    Left = 10
    Top = 9
    Width = 90
    Height = 13
    Caption = 'Buffer Size (bytes)'
  end
  object Label2: TLabel
    Left = 10
    Top = 31
    Width = 88
    Height = 13
    Caption = 'Server Cycle  (ms)'
  end
  object Label3: TLabel
    Left = 10
    Top = 53
    Width = 104
    Height = 13
    Caption = 'Inactive Timeout (ms)'
  end
  object Label6: TLabel
    Left = 201
    Top = 9
    Width = 93
    Height = 13
    Caption = 'Period of Rate (ms)'
  end
  object Label7: TLabel
    Left = 201
    Top = 53
    Width = 98
    Height = 13
    Caption = 'Output Debug Trace'
  end
  object Label8: TLabel
    Left = 10
    Top = 122
    Width = 71
    Height = 13
    Caption = 'Lat/Lon/Height'
  end
  object Label4: TLabel
    Left = 9
    Top = 212
    Width = 90
    Height = 13
    Caption = 'FTP/HTTP Local Dir'
  end
  object Label9: TLabel
    Left = 201
    Top = 31
    Width = 96
    Height = 13
    Caption = 'File Swap Margin (s)'
  end
  object Label5: TLabel
    Left = 10
    Top = 76
    Width = 119
    Height = 13
    Caption = 'Reconnect Interval  (ms)'
  end
  object Label10: TLabel
    Left = 10
    Top = 234
    Width = 90
    Height = 13
    Caption = 'HTTP/NTRIP Proxy'
  end
  object Label11: TLabel
    Left = 10
    Top = 144
    Width = 81
    Height = 13
    Caption = 'Offset E/N/U (m)'
  end
  object Label12: TLabel
    Left = 10
    Top = 166
    Width = 64
    Height = 13
    Caption = 'Antenna Info'
  end
  object Label13: TLabel
    Left = 10
    Top = 188
    Width = 65
    Height = 13
    Caption = 'Receiver Info'
  end
  object BtnOk: TButton
    Left = 179
    Top = 253
    Width = 95
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 274
    Top = 253
    Width = 95
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object SvrBuffSize: TEdit
    Left = 131
    Top = 7
    Width = 61
    Height = 21
    TabOrder = 2
    Text = '16384'
  end
  object SvrCycle: TEdit
    Left = 131
    Top = 29
    Width = 61
    Height = 21
    TabOrder = 3
    Text = '100'
  end
  object DataTimeout: TEdit
    Left = 131
    Top = 51
    Width = 61
    Height = 21
    TabOrder = 4
    Text = '10000'
  end
  object ConnectInterval: TEdit
    Left = 131
    Top = 73
    Width = 61
    Height = 21
    TabOrder = 5
    Text = '2000'
  end
  object AvePeriodRate: TEdit
    Left = 308
    Top = 7
    Width = 61
    Height = 21
    TabOrder = 6
    Text = '1000'
  end
  object TraceLevelS: TComboBox
    Left = 308
    Top = 51
    Width = 61
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 8
    Text = 'None'
    Items.Strings = (
      'None'
      'Level 1'
      'Level 2'
      'Level 3'
      'Level 4'
      'Level 5')
  end
  object AntPos2: TEdit
    Left = 193
    Top = 118
    Width = 87
    Height = 21
    TabOrder = 11
    Text = '0.000'
  end
  object AntPos1: TEdit
    Left = 104
    Top = 118
    Width = 87
    Height = 21
    TabOrder = 10
    Text = '0.000'
  end
  object NmeaCycle: TEdit
    Left = 308
    Top = 73
    Width = 61
    Height = 21
    TabOrder = 9
    Text = '0'
  end
  object AntPos3: TEdit
    Left = 282
    Top = 118
    Width = 72
    Height = 21
    TabOrder = 12
    Text = '0.000'
  end
  object BtnPos: TButton
    Left = 354
    Top = 118
    Width = 17
    Height = 21
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 13
    OnClick = BtnPosClick
  end
  object NmeaReqT: TCheckBox
    Left = 200
    Top = 75
    Width = 100
    Height = 17
    Caption = 'NMEA Cycle (ms)'
    TabOrder = 14
    OnClick = NmeaReqTClick
  end
  object LocalDir: TEdit
    Left = 104
    Top = 208
    Width = 249
    Height = 21
    TabOrder = 15
  end
  object BtnLocalDir: TButton
    Left = 353
    Top = 208
    Width = 17
    Height = 21
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 16
    OnClick = BtnLocalDirClick
  end
  object FileSwapMarginE: TEdit
    Left = 308
    Top = 29
    Width = 61
    Height = 21
    TabOrder = 7
    Text = '30'
  end
  object ProxyAddr: TEdit
    Left = 104
    Top = 230
    Width = 265
    Height = 21
    TabOrder = 17
  end
  object AntInfo: TEdit
    Left = 104
    Top = 162
    Width = 265
    Height = 21
    TabOrder = 18
    Text = 'AntInfo'
  end
  object RcvInfo: TEdit
    Left = 104
    Top = 184
    Width = 265
    Height = 21
    TabOrder = 19
    Text = 'Edit1'
  end
  object AntOff1: TEdit
    Left = 104
    Top = 140
    Width = 87
    Height = 21
    TabOrder = 20
    Text = '0.000'
  end
  object AntOff2: TEdit
    Left = 193
    Top = 140
    Width = 87
    Height = 21
    TabOrder = 21
    Text = '0.000'
  end
  object AntOff3: TEdit
    Left = 282
    Top = 140
    Width = 87
    Height = 21
    TabOrder = 22
    Text = '0.000'
  end
  object StationId: TEdit
    Left = 131
    Top = 95
    Width = 61
    Height = 21
    TabOrder = 23
    Text = '1234'
  end
  object StaInfoSel: TCheckBox
    Left = 9
    Top = 97
    Width = 77
    Height = 17
    Caption = 'Station ID'
    TabOrder = 24
    OnClick = StaInfoSelClick
  end
end
