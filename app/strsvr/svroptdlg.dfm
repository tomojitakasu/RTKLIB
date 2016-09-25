object SvrOptDialog: TSvrOptDialog
  Left = 0
  Top = 0
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 351
  ClientWidth = 435
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
  object Label15: TLabel
    Left = 9
    Top = 298
    Width = 36
    Height = 13
    Caption = 'Log File'
  end
  object Label14: TLabel
    Left = 9
    Top = 276
    Width = 95
    Height = 13
    Caption = 'NTRIP Source Table'
  end
  object Label10: TLabel
    Left = 9
    Top = 254
    Width = 90
    Height = 13
    Caption = 'HTTP/NTRIP Proxy'
  end
  object Label4: TLabel
    Left = 9
    Top = 232
    Width = 90
    Height = 13
    Caption = 'FTP/HTTP Local Dir'
  end
  object Label1: TLabel
    Left = 9
    Top = 10
    Width = 90
    Height = 13
    Caption = 'Buffer Size (bytes)'
  end
  object Label2: TLabel
    Left = 9
    Top = 32
    Width = 88
    Height = 13
    Caption = 'Server Cycle  (ms)'
  end
  object Label3: TLabel
    Left = 9
    Top = 54
    Width = 104
    Height = 13
    Caption = 'Inactive Timeout (ms)'
  end
  object Label6: TLabel
    Left = 234
    Top = 10
    Width = 93
    Height = 13
    Caption = 'Period of Rate (ms)'
  end
  object Label7: TLabel
    Left = 234
    Top = 76
    Width = 82
    Height = 13
    Caption = 'Output Log Level'
  end
  object Label8: TLabel
    Left = 9
    Top = 144
    Width = 71
    Height = 13
    Caption = 'Lat/Lon/Height'
  end
  object Label9: TLabel
    Left = 234
    Top = 32
    Width = 96
    Height = 13
    Caption = 'File Swap Margin (s)'
  end
  object Label5: TLabel
    Left = 9
    Top = 76
    Width = 119
    Height = 13
    Caption = 'Reconnect Interval  (ms)'
  end
  object Label11: TLabel
    Left = 9
    Top = 166
    Width = 81
    Height = 13
    Caption = 'Offset E/N/U (m)'
  end
  object Label12: TLabel
    Left = 9
    Top = 188
    Width = 64
    Height = 13
    Caption = 'Antenna Info'
  end
  object Label13: TLabel
    Left = 9
    Top = 210
    Width = 65
    Height = 13
    Caption = 'Receiver Info'
  end
  object Label16: TLabel
    Left = 234
    Top = 54
    Width = 77
    Height = 13
    Caption = 'Relay Messages'
  end
  object Label17: TLabel
    Left = 9
    Top = 98
    Width = 118
    Height = 13
    Caption = 'Progress Bar Range (KB)'
  end
  object BtnOk: TButton
    Left = 232
    Top = 318
    Width = 95
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 331
    Top = 318
    Width = 95
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object SvrBuffSize: TEdit
    Left = 131
    Top = 7
    Width = 85
    Height = 21
    TabOrder = 2
    Text = '16384'
  end
  object SvrCycle: TEdit
    Left = 131
    Top = 29
    Width = 85
    Height = 21
    TabOrder = 3
    Text = '100'
  end
  object DataTimeout: TEdit
    Left = 131
    Top = 51
    Width = 85
    Height = 21
    TabOrder = 4
    Text = '10000'
  end
  object ConnectInterval: TEdit
    Left = 131
    Top = 73
    Width = 85
    Height = 21
    TabOrder = 5
    Text = '2000'
  end
  object AvePeriodRate: TEdit
    Left = 340
    Top = 7
    Width = 85
    Height = 21
    TabOrder = 7
    Text = '1000'
  end
  object TraceLevelS: TComboBox
    Left = 340
    Top = 73
    Width = 85
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 10
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
    Left = 205
    Top = 140
    Width = 97
    Height = 21
    TabOrder = 16
    Text = '0.000'
  end
  object AntPos1: TEdit
    Left = 108
    Top = 140
    Width = 96
    Height = 21
    TabOrder = 15
    Text = '0.000'
  end
  object NmeaCycle: TEdit
    Left = 340
    Top = 95
    Width = 85
    Height = 21
    TabOrder = 12
    Text = '0'
  end
  object AntPos3: TEdit
    Left = 303
    Top = 140
    Width = 97
    Height = 21
    TabOrder = 17
    Text = '0.000'
  end
  object BtnPos: TButton
    Left = 401
    Top = 140
    Width = 25
    Height = 21
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 18
    OnClick = BtnPosClick
  end
  object NmeaReqT: TCheckBox
    Left = 233
    Top = 97
    Width = 100
    Height = 17
    Caption = 'NMEA Cycle (ms)'
    TabOrder = 11
    OnClick = NmeaReqTClick
  end
  object LocalDir: TEdit
    Left = 108
    Top = 228
    Width = 292
    Height = 21
    TabOrder = 24
  end
  object BtnLocalDir: TButton
    Left = 401
    Top = 228
    Width = 25
    Height = 21
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 25
    OnClick = BtnLocalDirClick
  end
  object FileSwapMarginE: TEdit
    Left = 340
    Top = 29
    Width = 85
    Height = 21
    TabOrder = 8
    Text = '30'
  end
  object ProxyAddr: TEdit
    Left = 108
    Top = 250
    Width = 317
    Height = 21
    TabOrder = 26
  end
  object AntInfo: TEdit
    Left = 108
    Top = 184
    Width = 317
    Height = 21
    TabOrder = 22
  end
  object RcvInfo: TEdit
    Left = 108
    Top = 206
    Width = 317
    Height = 21
    TabOrder = 23
  end
  object AntOff1: TEdit
    Left = 108
    Top = 162
    Width = 96
    Height = 21
    TabOrder = 19
    Text = '0.000'
  end
  object AntOff2: TEdit
    Left = 205
    Top = 162
    Width = 97
    Height = 21
    TabOrder = 20
    Text = '0.000'
  end
  object AntOff3: TEdit
    Left = 303
    Top = 162
    Width = 97
    Height = 21
    TabOrder = 21
    Text = '0.000'
  end
  object StationId: TEdit
    Left = 131
    Top = 117
    Width = 85
    Height = 21
    TabOrder = 14
    Text = '1234'
  end
  object StaInfoSel: TCheckBox
    Left = 9
    Top = 119
    Width = 77
    Height = 17
    Caption = 'Station ID'
    TabOrder = 13
    OnClick = StaInfoSelClick
  end
  object SrcTblFileF: TEdit
    Left = 108
    Top = 272
    Width = 292
    Height = 21
    TabOrder = 27
  end
  object BtnSrcTblFile: TButton
    Left = 401
    Top = 272
    Width = 25
    Height = 21
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 28
    OnClick = BtnSrcTblFileClick
  end
  object LogFileF: TEdit
    Left = 108
    Top = 294
    Width = 292
    Height = 21
    TabOrder = 29
  end
  object BtnLogFile: TButton
    Left = 401
    Top = 294
    Width = 25
    Height = 21
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 30
    OnClick = BtnLogFileClick
  end
  object RelayMsg: TComboBox
    Left = 340
    Top = 51
    Width = 85
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 9
    Text = 'None'
    Items.Strings = (
      'None'
      '(1)  -> (0)'
      '(2)  -> (0)'
      '(3)  -> (0)')
  end
  object ProgBarR: TEdit
    Left = 131
    Top = 95
    Width = 85
    Height = 21
    TabOrder = 6
    Text = '2000'
  end
  object OpenDialog: TOpenDialog
    Filter = 'All (*.*)|*.*'
    Left = 113
    Top = 250
  end
end
