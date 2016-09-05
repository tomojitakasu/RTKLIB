object OutputStrDialog: TOutputStrDialog
  Left = 0
  Top = 3
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Output Streams'
  ClientHeight = 156
  ClientWidth = 390
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
  object Label5: TLabel
    Left = 169
    Top = 3
    Width = 24
    Height = 13
    Caption = 'Type'
  end
  object Label6: TLabel
    Left = 234
    Top = 3
    Width = 32
    Height = 13
    Caption = 'Option'
  end
  object Label10: TLabel
    Left = 44
    Top = 3
    Width = 71
    Height = 13
    Caption = 'Output Stream'
  end
  object Label7: TLabel
    Left = 308
    Top = 5
    Width = 34
    Height = 13
    Caption = 'Format'
  end
  object LabelF1: TLabel
    Left = 8
    Top = 65
    Width = 83
    Height = 13
    Caption = 'Output File Paths'
  end
  object BtnKey: TSpeedButton
    Left = 190
    Top = 128
    Width = 21
    Height = 23
    Caption = '?'
    Flat = True
    OnClick = BtnKeyClick
  end
  object Label1: TLabel
    Left = 78
    Top = 132
    Width = 49
    Height = 13
    Caption = 'Swap Intv'
  end
  object Label2: TLabel
    Left = 178
    Top = 132
    Width = 7
    Height = 13
    Caption = 'H'
  end
  object BtnCancel: TButton
    Left = 301
    Top = 127
    Width = 85
    Height = 27
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BtnOk: TButton
    Left = 213
    Top = 127
    Width = 85
    Height = 27
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object Stream1: TComboBox
    Left = 133
    Top = 18
    Width = 103
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
      'NTRIP Server'
      'NTRIP Caster'
      'File')
  end
  object BtnStr1: TButton
    Left = 237
    Top = 17
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
    OnClick = BtnStr1Click
  end
  object BtnStr2: TButton
    Left = 237
    Top = 40
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
    OnClick = BtnStr2Click
  end
  object Stream2: TComboBox
    Left = 133
    Top = 41
    Width = 103
    Height = 21
    Style = csDropDownList
    TabOrder = 7
    OnChange = Stream2Change
    Items.Strings = (
      'Serial'
      'TCP Client'
      'TCP Server'
      'NTRIP Server'
      'NTRIP Caster'
      'File')
  end
  object Format1: TComboBox
    Left = 278
    Top = 18
    Width = 107
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 5
    Text = 'Lat/Lon/Height'
    Items.Strings = (
      'Lat/Lon/Height'
      'X/Y/Z-ECEF'
      'E/N/U-Baseline'
      'NMEA0183'
      'Solution Status')
  end
  object Format2: TComboBox
    Left = 278
    Top = 41
    Width = 107
    Height = 21
    Style = csDropDownList
    TabOrder = 9
    Items.Strings = (
      'Lat/Lon/Height'
      'X/Y/Z-ECEF'
      'E/N/U-Baseline'
      'NMEA0183'
      'Solution Status')
  end
  object FilePath1: TEdit
    Left = 6
    Top = 80
    Width = 355
    Height = 21
    TabOrder = 10
  end
  object BtnFile1: TButton
    Left = 361
    Top = 79
    Width = 25
    Height = 23
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 11
    OnClick = BtnFile1Click
  end
  object FilePath2: TEdit
    Left = 6
    Top = 102
    Width = 355
    Height = 21
    TabOrder = 12
  end
  object BtnFile2: TButton
    Left = 361
    Top = 101
    Width = 25
    Height = 23
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 13
    OnClick = BtnFile2Click
  end
  object TimeTagC: TCheckBox
    Left = 8
    Top = 130
    Width = 65
    Height = 17
    Caption = 'Time-Tag'
    TabOrder = 14
  end
  object Stream1C: TCheckBox
    Left = 8
    Top = 20
    Width = 120
    Height = 17
    Caption = '(4) Solution 1'
    TabOrder = 2
    OnClick = Stream1CClick
  end
  object Stream2C: TCheckBox
    Left = 8
    Top = 43
    Width = 120
    Height = 17
    Caption = '(5) Solution 2'
    TabOrder = 6
    OnClick = Stream2CClick
  end
  object SwapIntv: TComboBox
    Left = 130
    Top = 129
    Width = 45
    Height = 21
    TabOrder = 15
    Items.Strings = (
      ''
      '0.25'
      '0.5'
      '1'
      '3'
      '6'
      '12'
      '24')
  end
  object SaveDialog: TSaveDialog
    Filter = 
      'All File (*.*)|*.*|Log File (*.log)|*.log|RTCM2 File (*.rtcm2)|*' +
      '.rtcm2|RTCM3 File (*.rtcm3)|*.rtcm3'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Output File Path'
    Left = 314
    Top = 60
  end
end
