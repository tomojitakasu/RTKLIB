object LogStrDialog: TLogStrDialog
  Left = 0
  Top = 3
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Log Streams'
  ClientHeight = 196
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
    Left = 240
    Top = 3
    Width = 18
    Height = 13
    Caption = 'Opt'
  end
  object Label10: TLabel
    Left = 50
    Top = 3
    Width = 54
    Height = 13
    Caption = 'Log Stream'
  end
  object LabelF1: TLabel
    Left = 12
    Top = 84
    Width = 66
    Height = 13
    Caption = 'Log File Paths'
  end
  object BtnKey: TSpeedButton
    Left = 188
    Top = 168
    Width = 21
    Height = 23
    Caption = '?'
    Flat = True
    OnClick = BtnKeyClick
  end
  object Label1: TLabel
    Left = 76
    Top = 173
    Width = 49
    Height = 13
    Caption = 'Swap Intv'
  end
  object Label2: TLabel
    Left = 175
    Top = 173
    Width = 7
    Height = 13
    Caption = 'H'
  end
  object BtnCancel: TButton
    Left = 302
    Top = 167
    Width = 85
    Height = 27
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BtnOk: TButton
    Left = 212
    Top = 167
    Width = 85
    Height = 27
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object Stream1: TComboBox
    Left = 133
    Top = 17
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
  object Stream2: TComboBox
    Left = 133
    Top = 40
    Width = 103
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 6
    Text = 'Serial'
    OnChange = Stream2Change
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
    Top = 16
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
    Top = 39
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
    OnClick = BtnStr2Click
  end
  object FilePath1: TEdit
    Left = 8
    Top = 98
    Width = 355
    Height = 21
    TabOrder = 8
  end
  object FilePath2: TEdit
    Left = 8
    Top = 120
    Width = 355
    Height = 21
    TabOrder = 10
  end
  object BtnFile1: TButton
    Left = 363
    Top = 97
    Width = 25
    Height = 23
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 9
    OnClick = BtnFile1Click
  end
  object BtnFile2: TButton
    Left = 363
    Top = 119
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
    OnClick = BtnFile2Click
  end
  object TimeTagC: TCheckBox
    Left = 10
    Top = 170
    Width = 63
    Height = 17
    Caption = 'Time-Tag'
    TabOrder = 12
  end
  object Stream1C: TCheckBox
    Left = 8
    Top = 19
    Width = 117
    Height = 17
    Caption = '(6) Rover'
    TabOrder = 2
    OnClick = Stream1CClick
  end
  object Stream2C: TCheckBox
    Left = 8
    Top = 42
    Width = 117
    Height = 17
    Caption = '(7) Base Station'
    TabOrder = 5
    OnClick = Stream2CClick
  end
  object Stream3C: TCheckBox
    Left = 8
    Top = 65
    Width = 117
    Height = 17
    Caption = '(8) Correction'
    TabOrder = 13
    OnClick = Stream3CClick
  end
  object Stream3: TComboBox
    Left = 133
    Top = 63
    Width = 103
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 14
    Text = 'Serial'
    OnChange = Stream3Change
    Items.Strings = (
      'Serial'
      'TCP Client'
      'TCP Server'
      'NTRIP Server'
      'NTRIP Caster'
      'File')
  end
  object BtnStr3: TButton
    Left = 237
    Top = 62
    Width = 25
    Height = 23
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 15
    OnClick = BtnStr3Click
  end
  object FilePath3: TEdit
    Left = 8
    Top = 142
    Width = 355
    Height = 21
    TabOrder = 16
  end
  object BtnFile3: TButton
    Left = 363
    Top = 141
    Width = 25
    Height = 23
    Caption = '...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 17
    OnClick = BtnFile3Click
  end
  object SwapIntv: TComboBox
    Left = 128
    Top = 170
    Width = 45
    Height = 21
    TabOrder = 18
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
  object OutEventC: TCheckBox
    Left = 276
    Top = 19
    Width = 105
    Height = 17
    Caption = 'Output Event'
    TabOrder = 19
  end
  object SaveDialog: TSaveDialog
    Filter = 
      'All File (*.*)|*.*|Log File (*.log)|*.log|RTCM2 File (*.rtcm2)|*' +
      '.rtcm2|RTCM3 File (*.rtcm3)|*.rtcm3'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Output File Path'
    Left = 307
    Top = 64
  end
end
