object ExtOptDialog: TExtOptDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Extended Options'
  ClientHeight = 246
  ClientWidth = 371
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
  object BtnOk: TButton
    Left = 231
    Top = 225
    Width = 69
    Height = 21
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 300
    Top = 225
    Width = 69
    Height = 21
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object Panel6: TPanel
    Left = -1
    Top = 1
    Width = 371
    Height = 90
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 2
    object Label23: TLabel
      Left = 8
      Top = 22
      Width = 43
      Height = 13
      Caption = 'GPS/QZS'
    end
    object Label48: TLabel
      Left = 8
      Top = 44
      Width = 46
      Height = 13
      Caption = 'GLONASS'
    end
    object Label49: TLabel
      Left = 8
      Top = 66
      Width = 31
      Height = 13
      Caption = 'Galileo'
    end
    object Label59: TLabel
      Left = 92
      Top = 3
      Width = 6
      Height = 13
      Caption = 'a'
    end
    object Label61: TLabel
      Left = 138
      Top = 3
      Width = 6
      Height = 13
      Caption = 'b'
    end
    object Label84: TLabel
      Left = 244
      Top = 3
      Width = 6
      Height = 13
      Caption = 'b'
    end
    object Label85: TLabel
      Left = 198
      Top = 3
      Width = 6
      Height = 13
      Caption = 'a'
    end
    object Label88: TLabel
      Left = 346
      Top = 3
      Width = 6
      Height = 13
      Caption = 'b'
    end
    object Label89: TLabel
      Left = 300
      Top = 3
      Width = 6
      Height = 13
      Caption = 'a'
    end
    object ExtEna0: TCheckBox
      Left = 6
      Top = 3
      Width = 73
      Height = 17
      Caption = 'Code Error'
      TabOrder = 0
      OnClick = ExtEna0Click
    end
    object Panel4: TPanel
      Left = 58
      Top = 62
      Width = 309
      Height = 24
      BevelOuter = bvNone
      TabOrder = 3
      object Label71: TLabel
        Left = 2
        Top = 4
        Width = 12
        Height = 13
        Caption = 'E1'
      end
      object Label72: TLabel
        Left = 103
        Top = 4
        Width = 18
        Height = 13
        Caption = 'E5a'
      end
      object Label73: TLabel
        Left = 207
        Top = 4
        Width = 18
        Height = 13
        Caption = 'E5b'
      end
      object CodeErr20: TEdit
        Left = 18
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 0
        Text = '0.300'
      end
      object CodeErr21: TEdit
        Left = 60
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 1
        Text = '0.300'
      end
      object CodeErr22: TEdit
        Left = 122
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 2
        Text = '0.300'
      end
      object CodeErr23: TEdit
        Left = 164
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 3
        Text = '0.300'
      end
      object CodeErr24: TEdit
        Left = 226
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 4
        Text = '0.300'
      end
      object CodeErr25: TEdit
        Left = 268
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 5
        Text = '0.300'
      end
    end
    object Panel3: TPanel
      Left = 58
      Top = 40
      Width = 309
      Height = 24
      BevelOuter = bvNone
      TabOrder = 2
      object Label52: TLabel
        Left = 2
        Top = 4
        Width = 11
        Height = 13
        Caption = 'L1'
      end
      object Label70: TLabel
        Left = 106
        Top = 4
        Width = 11
        Height = 13
        Caption = 'L2'
      end
      object CodeErr10: TEdit
        Left = 18
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 0
        Text = '0.300'
      end
      object CodeErr11: TEdit
        Left = 60
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 1
        Text = '0.300'
      end
      object CodeErr12: TEdit
        Left = 122
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 2
        Text = '0.300'
      end
      object CodeErr13: TEdit
        Left = 164
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 3
        Text = '0.300'
      end
      object CodeErr14: TEdit
        Left = 226
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 4
        Text = '0.300'
        Visible = False
      end
      object CodeErr15: TEdit
        Left = 268
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 5
        Text = '0.300'
        Visible = False
      end
    end
    object Panel2: TPanel
      Left = 58
      Top = 18
      Width = 309
      Height = 24
      BevelOuter = bvNone
      TabOrder = 1
      object Label69: TLabel
        Left = 210
        Top = 4
        Width = 11
        Height = 13
        Caption = 'L5'
      end
      object Label50: TLabel
        Left = 106
        Top = 4
        Width = 11
        Height = 13
        Caption = 'L2'
      end
      object Label51: TLabel
        Left = 2
        Top = 4
        Width = 11
        Height = 13
        Caption = 'L1'
      end
      object CodeErr00: TEdit
        Left = 18
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 0
        Text = '0.300'
      end
      object CodeErr01: TEdit
        Left = 60
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 1
        Text = '0.300'
      end
      object CodeErr02: TEdit
        Left = 122
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 2
        Text = '0.300'
      end
      object CodeErr03: TEdit
        Left = 164
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 3
        Text = '0.300'
      end
      object CodeErr04: TEdit
        Left = 226
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 4
        Text = '0.300'
      end
      object CodeErr05: TEdit
        Left = 268
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 5
        Text = '0.300'
      end
    end
  end
  object Panel7: TPanel
    Left = -1
    Top = 91
    Width = 371
    Height = 90
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 3
    object Label12: TLabel
      Left = 8
      Top = 24
      Width = 43
      Height = 13
      Caption = 'GPS/QZS'
    end
    object Label53: TLabel
      Left = 8
      Top = 46
      Width = 46
      Height = 13
      Caption = 'GLONASS'
    end
    object Label58: TLabel
      Left = 8
      Top = 68
      Width = 31
      Height = 13
      Caption = 'Galileo'
    end
    object Label90: TLabel
      Left = 92
      Top = 3
      Width = 6
      Height = 13
      Caption = 'a'
    end
    object Label91: TLabel
      Left = 138
      Top = 3
      Width = 6
      Height = 13
      Caption = 'b'
    end
    object Label92: TLabel
      Left = 198
      Top = 3
      Width = 6
      Height = 13
      Caption = 'a'
    end
    object Label93: TLabel
      Left = 244
      Top = 3
      Width = 6
      Height = 13
      Caption = 'b'
    end
    object Label94: TLabel
      Left = 300
      Top = 3
      Width = 6
      Height = 13
      Caption = 'a'
    end
    object Label95: TLabel
      Left = 346
      Top = 3
      Width = 6
      Height = 13
      Caption = 'b'
    end
    object ExtEna1: TCheckBox
      Left = 6
      Top = 3
      Width = 73
      Height = 17
      Caption = 'Phase Error'
      TabOrder = 0
      OnClick = ExtEna1Click
    end
    object Panel10: TPanel
      Left = 58
      Top = 62
      Width = 309
      Height = 24
      BevelOuter = bvNone
      TabOrder = 3
      object Label76: TLabel
        Left = 2
        Top = 6
        Width = 12
        Height = 13
        Caption = 'E1'
      end
      object Label79: TLabel
        Left = 103
        Top = 6
        Width = 18
        Height = 13
        Caption = 'E5a'
      end
      object Label81: TLabel
        Left = 206
        Top = 6
        Width = 18
        Height = 13
        Caption = 'E5b'
      end
      object PhaseErr20: TEdit
        Left = 18
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 0
        Text = '0.003'
      end
      object PhaseErr21: TEdit
        Left = 60
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 1
        Text = '0.003'
      end
      object PhaseErr22: TEdit
        Left = 122
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 2
        Text = '0.003'
      end
      object PhaseErr23: TEdit
        Left = 164
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 3
        Text = '0.003'
      end
      object PhaseErr24: TEdit
        Left = 226
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 4
        Text = '0.003'
      end
      object PhaseErr25: TEdit
        Left = 268
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 5
        Text = '0.003'
      end
    end
    object Panel9: TPanel
      Left = 58
      Top = 40
      Width = 309
      Height = 24
      BevelOuter = bvNone
      TabOrder = 2
      object Label75: TLabel
        Left = 2
        Top = 6
        Width = 11
        Height = 13
        Caption = 'L1'
      end
      object Label78: TLabel
        Left = 106
        Top = 6
        Width = 11
        Height = 13
        Caption = 'L2'
      end
      object PhaseErr10: TEdit
        Left = 18
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 0
        Text = '0.003'
      end
      object PhaseErr11: TEdit
        Left = 60
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 1
        Text = '0.003'
      end
      object PhaseErr12: TEdit
        Left = 122
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 2
        Text = '0.003'
      end
      object PhaseErr13: TEdit
        Left = 164
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 3
        Text = '0.003'
      end
      object PhaseErr14: TEdit
        Left = 226
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 4
        Text = '0.003'
        Visible = False
      end
      object PhaseErr15: TEdit
        Left = 268
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 5
        Text = '0.003'
        Visible = False
      end
    end
    object Panel8: TPanel
      Left = 58
      Top = 18
      Width = 309
      Height = 24
      BevelOuter = bvNone
      TabOrder = 1
      object Label74: TLabel
        Left = 2
        Top = 6
        Width = 11
        Height = 13
        Caption = 'L1'
      end
      object Label77: TLabel
        Left = 106
        Top = 6
        Width = 11
        Height = 13
        Caption = 'L2'
      end
      object Label80: TLabel
        Left = 210
        Top = 6
        Width = 11
        Height = 13
        Caption = 'L5'
      end
      object PhaseErr00: TEdit
        Left = 18
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 0
        Text = '0.003'
      end
      object PhaseErr01: TEdit
        Left = 60
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 1
        Text = '0.003'
      end
      object PhaseErr02: TEdit
        Left = 122
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 2
        Text = '0.003'
      end
      object PhaseErr03: TEdit
        Left = 164
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 3
        Text = '0.003'
      end
      object PhaseErr04: TEdit
        Left = 226
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 4
        Text = '0.003'
      end
      object PhaseErr05: TEdit
        Left = 268
        Top = 2
        Width = 41
        Height = 21
        TabOrder = 5
        Text = '0.003'
      end
    end
  end
  object Panel5: TPanel
    Left = -1
    Top = 181
    Width = 371
    Height = 45
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 4
    object Label54: TLabel
      Left = 200
      Top = 23
      Width = 11
      Height = 13
      Caption = 'L1'
    end
    object Label56: TLabel
      Left = 264
      Top = 23
      Width = 11
      Height = 13
      Caption = 'L2'
    end
    object Label57: TLabel
      Left = 326
      Top = 23
      Width = 37
      Height = 13
      Caption = 'cm/FCN'
    end
    object Label55: TLabel
      Left = 10
      Top = 23
      Width = 11
      Height = 13
      Caption = 'L1'
    end
    object Label67: TLabel
      Left = 74
      Top = 23
      Width = 11
      Height = 13
      Caption = 'L2'
    end
    object Label68: TLabel
      Left = 136
      Top = 23
      Width = 8
      Height = 13
      Caption = 'm'
    end
    object ExtEna2: TCheckBox
      Left = 198
      Top = 3
      Width = 159
      Height = 17
      Caption = 'GLONASS Inter Freq Bias'
      TabOrder = 0
      OnClick = ExtEna2Click
    end
    object GloICB0: TEdit
      Left = 216
      Top = 20
      Width = 43
      Height = 21
      TabOrder = 1
      Text = '0.000'
    end
    object GloICB1: TEdit
      Left = 280
      Top = 20
      Width = 43
      Height = 21
      TabOrder = 2
      Text = '0.000'
    end
    object ExtEna3: TCheckBox
      Left = 8
      Top = 3
      Width = 149
      Height = 17
      Caption = 'GPS-GLO Inter System Bias'
      TabOrder = 3
      OnClick = ExtEna3Click
    end
    object GpsGloB0: TEdit
      Left = 26
      Top = 20
      Width = 43
      Height = 21
      TabOrder = 4
      Text = '0.000'
    end
    object GpsGloB1: TEdit
      Left = 90
      Top = 20
      Width = 43
      Height = 21
      TabOrder = 5
      Text = '0.000'
    end
  end
end
