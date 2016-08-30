object ConnectDialog: TConnectDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Connection Settings'
  ClientHeight = 164
  ClientWidth = 283
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
  object Label1: TLabel
    Left = 30
    Top = 6
    Width = 61
    Height = 13
    Caption = 'Stream Type'
  end
  object Label2: TLabel
    Left = 117
    Top = 6
    Width = 18
    Height = 13
    Caption = 'Opt'
  end
  object Label3: TLabel
    Left = 176
    Top = 6
    Width = 75
    Height = 13
    Caption = 'Solution Format'
  end
  object Label4: TLabel
    Left = 140
    Top = 6
    Width = 21
    Height = 13
    Caption = 'Cmd'
  end
  object Label5: TLabel
    Left = 26
    Top = 72
    Width = 59
    Height = 13
    Caption = 'Time Format'
  end
  object Label6: TLabel
    Left = 124
    Top = 72
    Width = 73
    Height = 13
    Caption = 'Lat/Lon Format'
  end
  object Label7: TLabel
    Left = 224
    Top = 72
    Width = 43
    Height = 13
    Caption = 'Field Sep'
  end
  object Label8: TLabel
    Left = 10
    Top = 114
    Width = 146
    Height = 13
    Caption = 'Timeout/Re-connect Intvl (ms)'
  end
  object Label9: TLabel
    Left = 6
    Top = 26
    Width = 14
    Height = 13
    Caption = '(1)'
  end
  object Label10: TLabel
    Left = 6
    Top = 50
    Width = 14
    Height = 13
    Caption = '(2)'
  end
  object BtnOk: TButton
    Left = 94
    Top = 134
    Width = 89
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 186
    Top = 134
    Width = 89
    Height = 29
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object SelStream1: TComboBox
    Left = 22
    Top = 22
    Width = 91
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 2
    OnChange = SelStream1Change
    Items.Strings = (
      ''
      'Serial'
      'TCP Client'
      'TCP Server'
      'NTRIP Client'
      'File')
  end
  object BtnOpt1: TButton
    Left = 114
    Top = 21
    Width = 25
    Height = 23
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    OnClick = BtnOpt1Click
  end
  object SolFormat1: TComboBox
    Left = 164
    Top = 22
    Width = 110
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 5
    Text = 'Lat/Lon/Height'
    OnChange = SolFormat1Change
    Items.Strings = (
      'Lat/Lon/Height'
      'X/Y/Z-ECEF'
      'E/N/U-Baseline'
      'NMEA0183'
      'Solution Status')
  end
  object BtnCmd1: TButton
    Left = 138
    Top = 21
    Width = 25
    Height = 23
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 4
    OnClick = BtnCmd1Click
  end
  object TimeFormS: TComboBox
    Left = 8
    Top = 86
    Width = 105
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 10
    Text = 'ww ssss.ss GPST'
    OnChange = SelStream1Change
    Items.Strings = (
      'ww ssss.ss GPST'
      'hh:mm:ss GPST'
      'hh:mm:ss UTC'
      'hh:mm:ss JST')
  end
  object DegFormS: TComboBox
    Left = 114
    Top = 86
    Width = 105
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 11
    Text = 'ddd.ddddddd'
    OnChange = SelStream1Change
    Items.Strings = (
      'ddd.ddddddd'
      'ddd mm ss.sss')
  end
  object FieldSepS: TEdit
    Left = 220
    Top = 86
    Width = 53
    Height = 21
    TabOrder = 12
  end
  object TimeOutTimeE: TEdit
    Left = 164
    Top = 110
    Width = 53
    Height = 21
    TabOrder = 13
    Text = '0'
  end
  object ReConnTimeE: TEdit
    Left = 220
    Top = 110
    Width = 53
    Height = 21
    TabOrder = 14
    Text = '10000'
  end
  object SelStream2: TComboBox
    Left = 22
    Top = 46
    Width = 91
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 6
    OnChange = SelStream2Change
    Items.Strings = (
      ''
      'Serial'
      'TCP Client'
      'TCP Server'
      'NTRIP Client'
      'File')
  end
  object BtnOpt2: TButton
    Left = 114
    Top = 45
    Width = 25
    Height = 23
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 7
    OnClick = BtnOpt2Click
  end
  object BtnCmd2: TButton
    Left = 138
    Top = 45
    Width = 25
    Height = 23
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 8
    OnClick = BtnCmd2Click
  end
  object SolFormat2: TComboBox
    Left = 164
    Top = 46
    Width = 110
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 9
    Text = 'Lat/Lon/Height'
    OnChange = SolFormat2Change
    Items.Strings = (
      'Lat/Lon/Height'
      'X/Y/Z-ECEF'
      'E/N/U-Baseline'
      'NMEA0183'
      'Solution Status')
  end
end
