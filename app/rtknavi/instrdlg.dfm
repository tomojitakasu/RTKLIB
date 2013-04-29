object InputStrDialog: TInputStrDialog
  Left = 0
  Top = 3
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Input Streams'
  ClientHeight = 232
  ClientWidth = 397
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
  object Label5: TLabel
    Left = 164
    Top = 3
    Width = 24
    Height = 13
    Caption = 'Type'
  end
  object Label6: TLabel
    Left = 232
    Top = 3
    Width = 18
    Height = 13
    Caption = 'Opt'
  end
  object Label10: TLabel
    Left = 40
    Top = 3
    Width = 63
    Height = 13
    Caption = 'Input Stream'
  end
  object Label7: TLabel
    Left = 306
    Top = 3
    Width = 34
    Height = 13
    Caption = 'Format'
  end
  object Label11: TLabel
    Left = 254
    Top = 3
    Width = 21
    Height = 13
    Caption = 'Cmd'
  end
  object LabelF1: TLabel
    Left = 10
    Top = 124
    Width = 75
    Height = 13
    Caption = 'Input File Paths'
  end
  object LabelF2: TLabel
    Left = 166
    Top = 209
    Width = 5
    Height = 13
    Caption = 's'
  end
  object LabelF3: TLabel
    Left = 106
    Top = 209
    Width = 8
    Height = 13
    Caption = '+'
  end
  object LabelNmea: TLabel
    Left = 8
    Top = 86
    Width = 185
    Height = 13
    Caption = 'Transmit NMEA GPGGA to Base Station'
  end
  object Label1: TLabel
    Left = 372
    Top = 3
    Width = 18
    Height = 13
    Caption = 'Opt'
  end
  object StreamC2: TCheckBox
    Left = 8
    Top = 42
    Width = 121
    Height = 17
    Caption = '(2) Base Station'
    TabOrder = 8
    OnClick = StreamC2Click
  end
  object StreamC1: TCheckBox
    Left = 8
    Top = 20
    Width = 119
    Height = 17
    Caption = '(1) Rover'
    TabOrder = 2
    OnClick = StreamC1Click
  end
  object TimeTagC: TCheckBox
    Left = 10
    Top = 208
    Width = 47
    Height = 17
    Caption = 'Time'
    TabOrder = 28
    OnClick = TimeTagCClick
  end
  object BtnCancel: TButton
    Left = 308
    Top = 206
    Width = 83
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BtnOk: TButton
    Left = 224
    Top = 206
    Width = 83
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object Stream1: TComboBox
    Left = 128
    Top = 18
    Width = 107
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 3
    Text = 'Serial'
    OnChange = Stream1Change
    Items.Strings = (
      'Serial'
      'TCP Client'
      'TCP Server'
      'NTRIP Client'
      'File')
  end
  object BtnStr1: TButton
    Left = 236
    Top = 18
    Width = 19
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 4
    OnClick = BtnStr1Click
  end
  object Stream2: TComboBox
    Left = 128
    Top = 40
    Width = 107
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 9
    Text = 'Serial'
    OnChange = Stream2Change
    Items.Strings = (
      'Serial'
      'TCP Client'
      'TCP Server'
      'NTRIP Client'
      'File')
  end
  object BtnStr2: TButton
    Left = 236
    Top = 40
    Width = 19
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 10
    OnClick = BtnStr2Click
  end
  object Format1: TComboBox
    Left = 276
    Top = 18
    Width = 97
    Height = 21
    Style = csDropDownList
    DropDownCount = 16
    TabOrder = 6
  end
  object Format2: TComboBox
    Left = 276
    Top = 40
    Width = 97
    Height = 21
    Style = csDropDownList
    DropDownCount = 16
    TabOrder = 12
  end
  object BtnCmd1: TButton
    Left = 256
    Top = 18
    Width = 19
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 5
    OnClick = BtnCmd1Click
  end
  object BtnCmd2: TButton
    Left = 256
    Top = 40
    Width = 19
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 11
    OnClick = BtnCmd2Click
  end
  object FilePath1: TEdit
    Left = 8
    Top = 138
    Width = 365
    Height = 21
    TabOrder = 24
  end
  object FilePath2: TEdit
    Left = 8
    Top = 160
    Width = 365
    Height = 21
    TabOrder = 26
  end
  object TimeStartE: TEdit
    Left = 116
    Top = 206
    Width = 47
    Height = 21
    TabOrder = 30
    Text = '0'
  end
  object BtnFile1: TButton
    Left = 374
    Top = 139
    Width = 17
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 25
    OnClick = BtnFile1Click
  end
  object BtnFile2: TButton
    Left = 374
    Top = 161
    Width = 17
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 27
    OnClick = BtnFile2Click
  end
  object NmeaReqL: TComboBox
    Left = 8
    Top = 100
    Width = 117
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 20
    Text = 'OFF'
    OnChange = NmeaReqLChange
    Items.Strings = (
      'OFF'
      'Latitude/Longitude'
      'Single Solution')
  end
  object TimeSpeedL: TComboBox
    Left = 58
    Top = 206
    Width = 49
    Height = 21
    DropDownCount = 20
    TabOrder = 29
    Text = 'x1'
    Items.Strings = (
      ''
      'x0.1'
      'x0.2'
      'x0.5'
      'x1'
      'x2'
      'x5'
      'x10')
  end
  object NmeaPos1: TEdit
    Left = 158
    Top = 100
    Width = 107
    Height = 21
    TabOrder = 21
    Text = '0.000000000'
  end
  object NmeaPos2: TEdit
    Left = 266
    Top = 100
    Width = 107
    Height = 21
    TabOrder = 22
    Text = '0.000000000'
  end
  object BtnPos: TButton
    Left = 374
    Top = 101
    Width = 17
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 23
    OnClick = BtnPosClick
  end
  object FilePath3: TEdit
    Left = 8
    Top = 182
    Width = 365
    Height = 21
    TabOrder = 31
  end
  object BtnFile3: TButton
    Left = 374
    Top = 183
    Width = 17
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 32
    OnClick = BtnFile3Click
  end
  object StreamC3: TCheckBox
    Left = 8
    Top = 64
    Width = 121
    Height = 17
    Caption = '(3) Correction'
    TabOrder = 14
    OnClick = StreamC2Click
  end
  object Stream3: TComboBox
    Left = 128
    Top = 62
    Width = 107
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 15
    Text = 'Serial'
    OnChange = Stream3Change
    Items.Strings = (
      'Serial'
      'TCP Client'
      'TCP Server'
      'NTRIP Client'
      'File'
      'FTP'
      'HTTP')
  end
  object BtnStr3: TButton
    Left = 236
    Top = 62
    Width = 19
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 16
    OnClick = BtnStr3Click
  end
  object Format3: TComboBox
    Left = 276
    Top = 62
    Width = 97
    Height = 21
    Style = csDropDownList
    DropDownCount = 16
    TabOrder = 18
  end
  object BtnCmd3: TButton
    Left = 256
    Top = 62
    Width = 19
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 17
    OnClick = BtnCmd3Click
  end
  object BtnRcvOpt1: TButton
    Left = 374
    Top = 18
    Width = 17
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 7
    OnClick = BtnRcvOpt1Click
  end
  object BtnRcvOpt2: TButton
    Left = 374
    Top = 40
    Width = 17
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 13
    OnClick = BtnRcvOpt2Click
  end
  object BtnRcvOpt3: TButton
    Left = 374
    Top = 62
    Width = 17
    Height = 20
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 19
    OnClick = BtnRcvOpt3Click
  end
  object OpenDialog: TOpenDialog
    Filter = 
      'All File (*.*)|*.*|Log File (*.log)|*.log|RTCM 2 File (*.rtcm2)|' +
      '*.rtcm2|RTCM 3 File (*.rtcm3)|*.rtcm3'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Input File Path'
    Left = 178
    Top = 202
  end
end
