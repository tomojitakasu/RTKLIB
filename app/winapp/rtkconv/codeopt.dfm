object CodeOptDialog: TCodeOptDialog
  Left = 158
  Top = 52
  BorderStyle = bsDialog
  Caption = 'Signal Mask'
  ClientHeight = 402
  ClientWidth = 400
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
    Left = 4
    Top = 362
    Width = 162
    Height = 35
    Caption = 'SBAS'
    TabOrder = 8
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
    Left = 242
    Top = 368
    Width = 74
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 320
    Top = 368
    Width = 74
    Height = 29
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object GroupBox1: TGroupBox
    Left = 4
    Top = 0
    Width = 390
    Height = 72
    Caption = 'GPS'
    TabOrder = 2
    object G01: TCheckBox
      Left = 10
      Top = 15
      Width = 44
      Height = 16
      Caption = '1C'
      TabOrder = 0
    end
    object G02: TCheckBox
      Left = 48
      Top = 15
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
      Top = 33
      Width = 44
      Height = 16
      Caption = '2C'
      TabOrder = 8
    end
    object G15: TCheckBox
      Left = 48
      Top = 33
      Width = 44
      Height = 16
      Caption = '2D'
      TabOrder = 9
    end
    object G16: TCheckBox
      Left = 86
      Top = 33
      Width = 44
      Height = 16
      Caption = '2S'
      TabOrder = 10
    end
    object G17: TCheckBox
      Left = 124
      Top = 33
      Width = 44
      Height = 16
      Caption = '2L'
      TabOrder = 11
    end
    object G18: TCheckBox
      Left = 162
      Top = 33
      Width = 44
      Height = 16
      Caption = '2X'
      TabOrder = 12
    end
    object G24: TCheckBox
      Left = 10
      Top = 52
      Width = 44
      Height = 16
      Caption = '5I'
      TabOrder = 18
    end
    object G25: TCheckBox
      Left = 48
      Top = 52
      Width = 44
      Height = 16
      Caption = '5Q'
      TabOrder = 19
    end
    object G26: TCheckBox
      Left = 86
      Top = 52
      Width = 44
      Height = 16
      Caption = '5X'
      TabOrder = 20
    end
    object G19: TCheckBox
      Left = 200
      Top = 33
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
      Top = 33
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
      Top = 33
      Width = 44
      Height = 16
      Caption = '2Y'
      TabOrder = 15
    end
    object G22: TCheckBox
      Left = 314
      Top = 33
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
      Top = 33
      Width = 35
      Height = 16
      Caption = '2N'
      TabOrder = 17
    end
  end
  object GroupBox2: TGroupBox
    Left = 4
    Top = 73
    Width = 390
    Height = 52
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
      Left = 239
      Top = 14
      Width = 44
      Height = 16
      Caption = '3X'
      TabOrder = 6
    end
    object R66: TCheckBox
      Left = 276
      Top = 14
      Width = 44
      Height = 16
      Caption = '4A'
      TabOrder = 7
    end
    object R67: TCheckBox
      Left = 314
      Top = 14
      Width = 44
      Height = 16
      Caption = '4B'
      TabOrder = 8
    end
    object R68: TCheckBox
      Left = 352
      Top = 14
      Width = 44
      Height = 16
      Caption = '4X'
      TabOrder = 9
    end
    object R30: TCheckBox
      Left = 10
      Top = 33
      Width = 44
      Height = 16
      Caption = '6A'
      TabOrder = 10
    end
    object R31: TCheckBox
      Left = 48
      Top = 33
      Width = 44
      Height = 16
      Caption = '6B'
      TabOrder = 11
    end
    object R33: TCheckBox
      Left = 86
      Top = 33
      Width = 44
      Height = 16
      Caption = '6X'
      TabOrder = 12
    end
  end
  object GroupBox3: TGroupBox
    Left = 4
    Top = 126
    Width = 390
    Height = 72
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
      Top = 33
      Width = 44
      Height = 16
      Caption = '5I'
      TabOrder = 5
    end
    object E25: TCheckBox
      Left = 48
      Top = 33
      Width = 44
      Height = 16
      Caption = '5Q'
      TabOrder = 6
    end
    object E26: TCheckBox
      Left = 86
      Top = 33
      Width = 44
      Height = 16
      Caption = '5X'
      TabOrder = 7
    end
    object E27: TCheckBox
      Left = 10
      Top = 52
      Width = 44
      Height = 16
      Caption = '7I'
      TabOrder = 13
    end
    object E28: TCheckBox
      Left = 48
      Top = 52
      Width = 44
      Height = 16
      Caption = '7Q'
      TabOrder = 14
    end
    object E29: TCheckBox
      Left = 86
      Top = 52
      Width = 44
      Height = 16
      Caption = '7X'
      TabOrder = 15
    end
    object E30: TCheckBox
      Left = 124
      Top = 33
      Width = 44
      Height = 16
      Caption = '6A'
      TabOrder = 8
    end
    object E31: TCheckBox
      Left = 162
      Top = 33
      Width = 35
      Height = 16
      Caption = '6B'
      TabOrder = 9
    end
    object E37: TCheckBox
      Left = 124
      Top = 52
      Width = 44
      Height = 16
      Caption = '8I'
      TabOrder = 16
    end
    object E38: TCheckBox
      Left = 162
      Top = 52
      Width = 44
      Height = 16
      Caption = '8Q'
      TabOrder = 17
    end
    object E32: TCheckBox
      Left = 200
      Top = 33
      Width = 44
      Height = 16
      Caption = '6C'
      TabOrder = 10
    end
    object E39: TCheckBox
      Left = 200
      Top = 52
      Width = 44
      Height = 16
      Caption = '8X'
      TabOrder = 18
    end
    object E33: TCheckBox
      Left = 237
      Top = 33
      Width = 44
      Height = 16
      Caption = '6X'
      TabOrder = 11
    end
    object E34: TCheckBox
      Left = 276
      Top = 33
      Width = 44
      Height = 16
      Caption = '6Z'
      TabOrder = 12
    end
  end
  object GroupBox4: TGroupBox
    Left = 4
    Top = 199
    Width = 390
    Height = 55
    Caption = 'QZSS'
    TabOrder = 5
    object J01: TCheckBox
      Left = 10
      Top = 14
      Width = 34
      Height = 16
      Caption = '1C'
      TabOrder = 0
    end
    object J07: TCheckBox
      Left = 48
      Top = 14
      Width = 34
      Height = 16
      Caption = '1S'
      TabOrder = 1
    end
    object J08: TCheckBox
      Left = 86
      Top = 14
      Width = 34
      Height = 16
      Caption = '1L'
      TabOrder = 2
    end
    object J13: TCheckBox
      Left = 162
      Top = 14
      Width = 34
      Height = 16
      Caption = '1Z'
      TabOrder = 4
    end
    object J12: TCheckBox
      Left = 124
      Top = 14
      Width = 34
      Height = 16
      Caption = '1X'
      TabOrder = 3
    end
    object J24: TCheckBox
      Left = 314
      Top = 14
      Width = 34
      Height = 16
      Caption = '5I'
      TabOrder = 8
    end
    object J25: TCheckBox
      Left = 352
      Top = 14
      Width = 34
      Height = 16
      Caption = '5Q'
      TabOrder = 9
    end
    object J26: TCheckBox
      Left = 10
      Top = 33
      Width = 34
      Height = 16
      Caption = '5X'
      TabOrder = 10
    end
    object J35: TCheckBox
      Left = 161
      Top = 33
      Width = 34
      Height = 16
      Caption = '6S'
      TabOrder = 14
    end
    object J36: TCheckBox
      Left = 200
      Top = 33
      Width = 34
      Height = 16
      Caption = '6L'
      TabOrder = 15
    end
    object J16: TCheckBox
      Left = 200
      Top = 14
      Width = 34
      Height = 16
      Caption = '2S'
      TabOrder = 5
    end
    object J17: TCheckBox
      Left = 237
      Top = 14
      Width = 34
      Height = 16
      Caption = '2L'
      TabOrder = 6
    end
    object J18: TCheckBox
      Left = 276
      Top = 14
      Width = 34
      Height = 16
      Caption = '2X'
      TabOrder = 7
    end
    object J33: TCheckBox
      Left = 237
      Top = 33
      Width = 34
      Height = 16
      Caption = '6X'
      TabOrder = 16
    end
    object J57: TCheckBox
      Left = 47
      Top = 33
      Width = 34
      Height = 16
      Caption = '5D'
      TabOrder = 11
    end
    object J58: TCheckBox
      Left = 86
      Top = 33
      Width = 34
      Height = 16
      Caption = '5P'
      TabOrder = 12
    end
    object J59: TCheckBox
      Left = 124
      Top = 33
      Width = 34
      Height = 16
      Caption = '5Z'
      TabOrder = 13
    end
    object J60: TCheckBox
      Left = 276
      Top = 33
      Width = 34
      Height = 16
      Caption = '6E'
      TabOrder = 17
    end
    object J34: TCheckBox
      Left = 314
      Top = 33
      Width = 34
      Height = 16
      Caption = '6Z'
      TabOrder = 18
    end
  end
  object BtnSetAll: TButton
    Left = 170
    Top = 368
    Width = 66
    Height = 29
    Caption = 'Set &All'
    TabOrder = 9
    OnClick = BtnSetAllClick
  end
  object GroupBox6: TGroupBox
    Left = 4
    Top = 255
    Width = 390
    Height = 72
    Caption = 'BDS'
    TabOrder = 6
    object C40: TCheckBox
      Left = 10
      Top = 15
      Width = 35
      Height = 16
      Caption = '2I'
      Enabled = False
      TabOrder = 0
    end
    object C42: TCheckBox
      Left = 238
      Top = 15
      Width = 32
      Height = 16
      Caption = '6I'
      Enabled = False
      TabOrder = 6
    end
    object C27: TCheckBox
      Left = 124
      Top = 15
      Width = 34
      Height = 16
      Caption = '7I'
      Enabled = False
      TabOrder = 3
    end
    object C28: TCheckBox
      Left = 162
      Top = 15
      Width = 44
      Height = 16
      Caption = '7Q'
      Enabled = False
      TabOrder = 4
    end
    object C29: TCheckBox
      Left = 201
      Top = 15
      Width = 34
      Height = 16
      Caption = '7X'
      Enabled = False
      TabOrder = 5
    end
    object C41: TCheckBox
      Left = 48
      Top = 15
      Width = 34
      Height = 16
      Caption = '2Q'
      Enabled = False
      TabOrder = 1
    end
    object C18: TCheckBox
      Left = 85
      Top = 15
      Width = 34
      Height = 16
      Caption = '2X'
      Enabled = False
      TabOrder = 2
    end
    object C43: TCheckBox
      Left = 276
      Top = 15
      Width = 32
      Height = 16
      Caption = '6Q'
      Enabled = False
      TabOrder = 7
    end
    object C33: TCheckBox
      Left = 314
      Top = 15
      Width = 32
      Height = 16
      Caption = '6X'
      Enabled = False
      TabOrder = 8
    end
    object C56: TCheckBox
      Left = 10
      Top = 33
      Width = 35
      Height = 16
      Caption = '1D'
      Enabled = False
      TabOrder = 9
    end
    object C02: TCheckBox
      Left = 48
      Top = 33
      Width = 34
      Height = 16
      Caption = '1P'
      Enabled = False
      TabOrder = 10
    end
    object C12: TCheckBox
      Left = 85
      Top = 33
      Width = 34
      Height = 16
      Caption = '1X'
      Enabled = False
      TabOrder = 11
    end
    object C10: TCheckBox
      Left = 124
      Top = 33
      Width = 34
      Height = 16
      Caption = '1A'
      Enabled = False
      TabOrder = 12
    end
    object C06: TCheckBox
      Left = 162
      Top = 33
      Width = 44
      Height = 16
      Caption = '1N'
      Enabled = False
      TabOrder = 13
    end
    object C57: TCheckBox
      Left = 201
      Top = 33
      Width = 34
      Height = 16
      Caption = '5D'
      Enabled = False
      TabOrder = 14
    end
    object C58: TCheckBox
      Left = 238
      Top = 33
      Width = 32
      Height = 16
      Caption = '5P'
      Enabled = False
      TabOrder = 15
    end
    object C26: TCheckBox
      Left = 276
      Top = 33
      Width = 32
      Height = 16
      Caption = '5X'
      Enabled = False
      TabOrder = 16
    end
    object C61: TCheckBox
      Left = 10
      Top = 52
      Width = 35
      Height = 16
      Caption = '7D'
      Enabled = False
      TabOrder = 17
    end
    object C62: TCheckBox
      Left = 48
      Top = 52
      Width = 34
      Height = 16
      Caption = '7P'
      Enabled = False
      TabOrder = 18
    end
    object C63: TCheckBox
      Left = 85
      Top = 52
      Width = 34
      Height = 16
      Caption = '7Z'
      Enabled = False
      TabOrder = 19
    end
    object C64: TCheckBox
      Left = 124
      Top = 52
      Width = 34
      Height = 16
      Caption = '8D'
      Enabled = False
      TabOrder = 20
    end
    object C65: TCheckBox
      Left = 162
      Top = 52
      Width = 44
      Height = 16
      Caption = '8P'
      Enabled = False
      TabOrder = 21
    end
    object C39: TCheckBox
      Left = 201
      Top = 52
      Width = 34
      Height = 16
      Caption = '8X'
      Enabled = False
      TabOrder = 22
    end
    object C30: TCheckBox
      Left = 238
      Top = 52
      Width = 32
      Height = 16
      Caption = '6A'
      Enabled = False
      TabOrder = 23
    end
  end
  object GroupBox7: TGroupBox
    Left = 4
    Top = 328
    Width = 390
    Height = 34
    Caption = 'NavIC'
    TabOrder = 7
    object I49: TCheckBox
      Left = 10
      Top = 14
      Width = 35
      Height = 16
      Caption = '5A'
      Enabled = False
      TabOrder = 0
    end
    object I54: TCheckBox
      Left = 238
      Top = 14
      Width = 32
      Height = 16
      Caption = '9C'
      Enabled = False
      TabOrder = 6
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
      TabOrder = 5
    end
    object I50: TCheckBox
      Left = 48
      Top = 14
      Width = 34
      Height = 16
      Caption = '5B'
      Enabled = False
      TabOrder = 1
    end
    object I51: TCheckBox
      Left = 86
      Top = 14
      Width = 34
      Height = 16
      Caption = '5C'
      Enabled = False
      TabOrder = 2
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
