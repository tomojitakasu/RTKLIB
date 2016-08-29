object CodeOptDialog: TCodeOptDialog
  Left = 158
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Signal Mask'
  ClientHeight = 333
  ClientWidth = 397
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
  object GroupBox5: TGroupBox
    Left = 0
    Top = 296
    Width = 166
    Height = 34
    Caption = 'SBAS'
    TabOrder = 6
    object S01: TCheckBox
      Left = 10
      Top = 14
      Width = 44
      Height = 16
      Caption = '1C'
      TabOrder = 0
    end
    object S24: TCheckBox
      Left = 48
      Top = 14
      Width = 44
      Height = 16
      Caption = '5I'
      TabOrder = 1
    end
    object S25: TCheckBox
      Left = 86
      Top = 14
      Width = 44
      Height = 16
      Caption = '5Q'
      TabOrder = 2
    end
    object S26: TCheckBox
      Left = 124
      Top = 14
      Width = 34
      Height = 16
      Caption = '5X'
      TabOrder = 3
    end
  end
  object BtnOk: TButton
    Left = 253
    Top = 301
    Width = 71
    Height = 29
    Caption = 'OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 324
    Top = 301
    Width = 71
    Height = 29
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object GroupBox1: TGroupBox
    Left = 0
    Top = 1
    Width = 394
    Height = 70
    Caption = 'GPS'
    TabOrder = 2
    object G01: TCheckBox
      Left = 10
      Top = 14
      Width = 44
      Height = 16
      Caption = '1C'
      TabOrder = 0
    end
    object G02: TCheckBox
      Left = 48
      Top = 14
      Width = 44
      Height = 16
      Caption = '1P'
      TabOrder = 1
    end
    object G03: TCheckBox
      Left = 86
      Top = 14
      Width = 44
      Height = 16
      Caption = '1W'
      TabOrder = 2
    end
    object G04: TCheckBox
      Left = 124
      Top = 14
      Width = 44
      Height = 16
      Caption = '1Y'
      TabOrder = 3
    end
    object G05: TCheckBox
      Left = 162
      Top = 14
      Width = 44
      Height = 16
      Caption = '1M'
      TabOrder = 4
    end
    object G14: TCheckBox
      Left = 10
      Top = 32
      Width = 44
      Height = 16
      Caption = '2C'
      TabOrder = 8
    end
    object G15: TCheckBox
      Left = 48
      Top = 32
      Width = 44
      Height = 16
      Caption = '2D'
      TabOrder = 9
    end
    object G16: TCheckBox
      Left = 86
      Top = 32
      Width = 44
      Height = 16
      Caption = '2S'
      TabOrder = 10
    end
    object G17: TCheckBox
      Left = 124
      Top = 32
      Width = 44
      Height = 16
      Caption = '2L'
      TabOrder = 11
    end
    object G18: TCheckBox
      Left = 162
      Top = 32
      Width = 44
      Height = 16
      Caption = '2X'
      TabOrder = 12
    end
    object G24: TCheckBox
      Left = 10
      Top = 50
      Width = 44
      Height = 16
      Caption = '5I'
      TabOrder = 18
    end
    object G25: TCheckBox
      Left = 48
      Top = 50
      Width = 44
      Height = 16
      Caption = '5Q'
      TabOrder = 19
    end
    object G26: TCheckBox
      Left = 86
      Top = 50
      Width = 44
      Height = 16
      Caption = '5X'
      TabOrder = 20
    end
    object G19: TCheckBox
      Left = 200
      Top = 32
      Width = 44
      Height = 16
      Caption = '2P'
      TabOrder = 13
    end
    object G06: TCheckBox
      Left = 200
      Top = 14
      Width = 44
      Height = 16
      Caption = '1N'
      TabOrder = 5
    end
    object G20: TCheckBox
      Left = 238
      Top = 32
      Width = 44
      Height = 16
      Caption = '2W'
      TabOrder = 14
    end
    object G07: TCheckBox
      Left = 238
      Top = 14
      Width = 44
      Height = 16
      Caption = '1S'
      TabOrder = 6
    end
    object G21: TCheckBox
      Left = 276
      Top = 32
      Width = 44
      Height = 16
      Caption = '2Y'
      TabOrder = 15
    end
    object G22: TCheckBox
      Left = 314
      Top = 32
      Width = 40
      Height = 16
      Caption = '2M'
      TabOrder = 16
    end
    object G08: TCheckBox
      Left = 276
      Top = 14
      Width = 44
      Height = 16
      Caption = '1L'
      TabOrder = 7
    end
    object G23: TCheckBox
      Left = 352
      Top = 32
      Width = 40
      Height = 16
      Caption = '2N'
      TabOrder = 17
    end
  end
  object GroupBox2: TGroupBox
    Left = 0
    Top = 71
    Width = 394
    Height = 36
    Caption = 'GLONASS'
    TabOrder = 3
    object R01: TCheckBox
      Left = 10
      Top = 14
      Width = 44
      Height = 16
      Caption = '1C'
      TabOrder = 0
    end
    object R02: TCheckBox
      Left = 48
      Top = 14
      Width = 44
      Height = 16
      Caption = '1P'
      TabOrder = 1
    end
    object R14: TCheckBox
      Left = 86
      Top = 14
      Width = 44
      Height = 16
      Caption = '2C'
      TabOrder = 2
    end
    object R19: TCheckBox
      Left = 124
      Top = 14
      Width = 44
      Height = 16
      Caption = '2P'
      TabOrder = 3
    end
    object R44: TCheckBox
      Left = 162
      Top = 14
      Width = 44
      Height = 16
      Caption = '3I'
      TabOrder = 4
    end
    object R45: TCheckBox
      Left = 200
      Top = 14
      Width = 44
      Height = 16
      Caption = '3Q'
      TabOrder = 5
    end
    object R46: TCheckBox
      Left = 238
      Top = 14
      Width = 44
      Height = 16
      Caption = '3X'
      TabOrder = 6
    end
  end
  object GroupBox3: TGroupBox
    Left = 0
    Top = 107
    Width = 394
    Height = 69
    Caption = 'Galileo'
    TabOrder = 4
    object E01: TCheckBox
      Left = 10
      Top = 14
      Width = 44
      Height = 16
      Caption = '1C'
      TabOrder = 0
    end
    object E10: TCheckBox
      Left = 48
      Top = 14
      Width = 44
      Height = 16
      Caption = '1A'
      TabOrder = 1
    end
    object E11: TCheckBox
      Left = 86
      Top = 14
      Width = 44
      Height = 16
      Caption = '1B'
      TabOrder = 2
    end
    object E12: TCheckBox
      Left = 124
      Top = 14
      Width = 44
      Height = 16
      Caption = '1X'
      TabOrder = 3
    end
    object E13: TCheckBox
      Left = 162
      Top = 14
      Width = 44
      Height = 16
      Caption = '1Z'
      TabOrder = 4
    end
    object E24: TCheckBox
      Left = 10
      Top = 32
      Width = 44
      Height = 16
      Caption = '5I'
      TabOrder = 5
    end
    object E25: TCheckBox
      Left = 48
      Top = 32
      Width = 44
      Height = 16
      Caption = '5Q'
      TabOrder = 6
    end
    object E26: TCheckBox
      Left = 86
      Top = 32
      Width = 44
      Height = 16
      Caption = '5X'
      TabOrder = 7
    end
    object E27: TCheckBox
      Left = 10
      Top = 50
      Width = 44
      Height = 16
      Caption = '7I'
      TabOrder = 13
    end
    object E28: TCheckBox
      Left = 48
      Top = 50
      Width = 44
      Height = 16
      Caption = '7Q'
      TabOrder = 14
    end
    object E29: TCheckBox
      Left = 86
      Top = 50
      Width = 44
      Height = 16
      Caption = '7X'
      TabOrder = 15
    end
    object E30: TCheckBox
      Left = 124
      Top = 32
      Width = 44
      Height = 16
      Caption = '6A'
      TabOrder = 8
    end
    object E31: TCheckBox
      Left = 162
      Top = 32
      Width = 35
      Height = 16
      Caption = '6B'
      TabOrder = 9
    end
    object E37: TCheckBox
      Left = 124
      Top = 50
      Width = 44
      Height = 16
      Caption = '8I'
      TabOrder = 16
    end
    object E38: TCheckBox
      Left = 162
      Top = 50
      Width = 44
      Height = 16
      Caption = '8Q'
      TabOrder = 17
    end
    object E32: TCheckBox
      Left = 200
      Top = 32
      Width = 44
      Height = 16
      Caption = '6C'
      TabOrder = 10
    end
    object E39: TCheckBox
      Left = 200
      Top = 50
      Width = 44
      Height = 16
      Caption = '8X'
      TabOrder = 18
    end
    object E33: TCheckBox
      Left = 238
      Top = 32
      Width = 44
      Height = 16
      Caption = '6X'
      TabOrder = 11
    end
    object E34: TCheckBox
      Left = 276
      Top = 32
      Width = 44
      Height = 16
      Caption = '6Z'
      TabOrder = 12
    end
  end
  object GroupBox4: TGroupBox
    Left = 0
    Top = 176
    Width = 394
    Height = 52
    Caption = 'QZSS'
    TabOrder = 5
    object J01: TCheckBox
      Left = 10
      Top = 14
      Width = 44
      Height = 16
      Caption = '1C'
      TabOrder = 0
    end
    object J07: TCheckBox
      Left = 48
      Top = 14
      Width = 44
      Height = 16
      Caption = '1S'
      TabOrder = 1
    end
    object J08: TCheckBox
      Left = 86
      Top = 14
      Width = 44
      Height = 16
      Caption = '1L'
      TabOrder = 2
    end
    object J13: TCheckBox
      Left = 162
      Top = 14
      Width = 38
      Height = 16
      Caption = '1Z'
      TabOrder = 4
    end
    object J12: TCheckBox
      Left = 124
      Top = 14
      Width = 32
      Height = 16
      Caption = '1X'
      TabOrder = 3
    end
    object J24: TCheckBox
      Left = 10
      Top = 32
      Width = 44
      Height = 16
      Caption = '5I'
      TabOrder = 8
    end
    object J25: TCheckBox
      Left = 48
      Top = 32
      Width = 44
      Height = 16
      Caption = '5Q'
      TabOrder = 9
    end
    object J26: TCheckBox
      Left = 86
      Top = 32
      Width = 44
      Height = 16
      Caption = '5X'
      TabOrder = 10
    end
    object J35: TCheckBox
      Left = 124
      Top = 32
      Width = 44
      Height = 16
      Caption = '6S'
      TabOrder = 11
    end
    object J36: TCheckBox
      Left = 162
      Top = 32
      Width = 44
      Height = 16
      Caption = '6L'
      TabOrder = 12
    end
    object J16: TCheckBox
      Left = 200
      Top = 14
      Width = 44
      Height = 16
      Caption = '2S'
      TabOrder = 5
    end
    object J17: TCheckBox
      Left = 238
      Top = 14
      Width = 44
      Height = 16
      Caption = '2L'
      TabOrder = 6
    end
    object J18: TCheckBox
      Left = 276
      Top = 14
      Width = 44
      Height = 16
      Caption = '2X'
      TabOrder = 7
    end
    object J33: TCheckBox
      Left = 200
      Top = 32
      Width = 40
      Height = 16
      Caption = '6X'
      TabOrder = 13
    end
  end
  object BtnSetAll: TButton
    Left = 180
    Top = 301
    Width = 61
    Height = 29
    Caption = 'Set All'
    TabOrder = 7
    OnClick = BtnSetAllClick
  end
  object GroupBox6: TGroupBox
    Left = 0
    Top = 228
    Width = 393
    Height = 34
    Caption = 'BeiDou'
    TabOrder = 8
    object C47: TCheckBox
      Left = 10
      Top = 14
      Width = 35
      Height = 16
      Caption = '2I'
      Enabled = False
      TabOrder = 2
    end
    object C42: TCheckBox
      Left = 238
      Top = 14
      Width = 32
      Height = 16
      Caption = '6I'
      Enabled = False
      TabOrder = 0
    end
    object C27: TCheckBox
      Left = 124
      Top = 14
      Width = 34
      Height = 16
      Caption = '7I'
      Enabled = False
      TabOrder = 3
    end
    object C28: TCheckBox
      Left = 162
      Top = 14
      Width = 44
      Height = 16
      Caption = '7Q'
      Enabled = False
      TabOrder = 4
    end
    object C29: TCheckBox
      Left = 201
      Top = 14
      Width = 34
      Height = 16
      Caption = '7X'
      Enabled = False
      TabOrder = 1
    end
    object C48: TCheckBox
      Left = 48
      Top = 14
      Width = 34
      Height = 16
      Caption = '2Q'
      Enabled = False
      TabOrder = 5
    end
    object C12: TCheckBox
      Left = 86
      Top = 14
      Width = 34
      Height = 16
      Caption = '2X'
      Enabled = False
      TabOrder = 6
    end
    object C43: TCheckBox
      Left = 276
      Top = 14
      Width = 32
      Height = 16
      Caption = '6Q'
      Enabled = False
      TabOrder = 7
    end
    object C33: TCheckBox
      Left = 314
      Top = 14
      Width = 32
      Height = 16
      Caption = '6X'
      Enabled = False
      TabOrder = 8
    end
  end
  object GroupBox7: TGroupBox
    Left = 0
    Top = 262
    Width = 393
    Height = 34
    Caption = 'IRNSS'
    TabOrder = 9
    object I49: TCheckBox
      Left = 10
      Top = 14
      Width = 35
      Height = 16
      Caption = '5A'
      Enabled = False
      TabOrder = 2
    end
    object I54: TCheckBox
      Left = 238
      Top = 14
      Width = 32
      Height = 16
      Caption = '9C'
      Enabled = False
      TabOrder = 0
    end
    object I26: TCheckBox
      Left = 124
      Top = 14
      Width = 34
      Height = 16
      Caption = '5X'
      Enabled = False
      TabOrder = 3
    end
    object I52: TCheckBox
      Left = 162
      Top = 14
      Width = 44
      Height = 16
      Caption = '9A'
      Enabled = False
      TabOrder = 4
    end
    object I53: TCheckBox
      Left = 201
      Top = 14
      Width = 34
      Height = 16
      Caption = '9B'
      Enabled = False
      TabOrder = 1
    end
    object I50: TCheckBox
      Left = 48
      Top = 14
      Width = 34
      Height = 16
      Caption = '5B'
      Enabled = False
      TabOrder = 5
    end
    object I51: TCheckBox
      Left = 86
      Top = 14
      Width = 34
      Height = 16
      Caption = '5C'
      Enabled = False
      TabOrder = 6
    end
    object I55: TCheckBox
      Left = 276
      Top = 14
      Width = 32
      Height = 16
      Caption = '9X'
      Enabled = False
      TabOrder = 7
    end
  end
end
