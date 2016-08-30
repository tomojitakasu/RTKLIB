object SatDialog: TSatDialog
  Left = 0
  Top = 0
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Satellites'
  ClientHeight = 187
  ClientWidth = 349
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
  object BtnCancel: TButton
    Left = 272
    Top = 162
    Width = 73
    Height = 23
    Caption = '&Cancel'
    TabOrder = 0
    OnClick = BtnCancelClick
  end
  object BtnOk: TButton
    Left = 196
    Top = 162
    Width = 73
    Height = 23
    Caption = '&OK'
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object Panel1: TPanel
    Left = 6
    Top = 4
    Width = 65
    Height = 153
    BevelOuter = bvNone
    TabOrder = 2
    object PRN01: TCheckBox
      Left = 8
      Top = 6
      Width = 59
      Height = 17
      Caption = 'PRN01'
      TabOrder = 0
    end
    object PRN02: TCheckBox
      Left = 8
      Top = 24
      Width = 59
      Height = 17
      Caption = 'PRN02'
      TabOrder = 1
    end
    object PRN03: TCheckBox
      Left = 8
      Top = 42
      Width = 59
      Height = 17
      Caption = 'PRN03'
      TabOrder = 2
    end
    object PRN04: TCheckBox
      Left = 8
      Top = 60
      Width = 59
      Height = 17
      Caption = 'PRN04'
      TabOrder = 3
    end
    object PRN05: TCheckBox
      Left = 8
      Top = 78
      Width = 59
      Height = 17
      Caption = 'PRN05'
      TabOrder = 4
    end
    object PRN06: TCheckBox
      Left = 8
      Top = 96
      Width = 59
      Height = 17
      Caption = 'PRN06'
      TabOrder = 5
    end
    object PRN07: TCheckBox
      Left = 8
      Top = 114
      Width = 59
      Height = 17
      Caption = 'PRN07'
      TabOrder = 6
    end
    object PRN08: TCheckBox
      Left = 8
      Top = 132
      Width = 59
      Height = 17
      Caption = 'PRN08'
      TabOrder = 7
    end
  end
  object Panel2: TPanel
    Left = 74
    Top = 4
    Width = 65
    Height = 153
    BevelOuter = bvNone
    TabOrder = 3
    object PRN09: TCheckBox
      Left = 8
      Top = 6
      Width = 59
      Height = 17
      Caption = 'PRN09'
      TabOrder = 0
    end
    object PRN10: TCheckBox
      Left = 8
      Top = 24
      Width = 59
      Height = 17
      Caption = 'PRN10'
      TabOrder = 1
    end
    object PRN11: TCheckBox
      Left = 8
      Top = 42
      Width = 59
      Height = 17
      Caption = 'PRN11'
      TabOrder = 2
    end
    object PRN12: TCheckBox
      Left = 8
      Top = 60
      Width = 59
      Height = 17
      Caption = 'PRN12'
      TabOrder = 3
    end
    object PRN13: TCheckBox
      Left = 8
      Top = 78
      Width = 59
      Height = 17
      Caption = 'PRN13'
      TabOrder = 4
    end
    object PRN14: TCheckBox
      Left = 8
      Top = 96
      Width = 59
      Height = 17
      Caption = 'PRN14'
      TabOrder = 5
    end
    object PRN15: TCheckBox
      Left = 8
      Top = 114
      Width = 59
      Height = 17
      Caption = 'PRN15'
      TabOrder = 6
    end
    object PRN16: TCheckBox
      Left = 8
      Top = 132
      Width = 59
      Height = 17
      Caption = 'PRN16'
      TabOrder = 7
    end
  end
  object Panel3: TPanel
    Left = 142
    Top = 4
    Width = 65
    Height = 153
    BevelOuter = bvNone
    TabOrder = 4
    object PRN17: TCheckBox
      Left = 8
      Top = 6
      Width = 59
      Height = 17
      Caption = 'PRN17'
      TabOrder = 0
    end
    object PRN18: TCheckBox
      Left = 8
      Top = 24
      Width = 59
      Height = 17
      Caption = 'PRN18'
      TabOrder = 1
    end
    object PRN19: TCheckBox
      Left = 8
      Top = 42
      Width = 59
      Height = 17
      Caption = 'PRN19'
      TabOrder = 2
    end
    object PRN20: TCheckBox
      Left = 8
      Top = 60
      Width = 59
      Height = 17
      Caption = 'PRN20'
      TabOrder = 3
    end
    object PRN21: TCheckBox
      Left = 8
      Top = 78
      Width = 59
      Height = 17
      Caption = 'PRN21'
      TabOrder = 4
    end
    object PRN22: TCheckBox
      Left = 8
      Top = 96
      Width = 59
      Height = 17
      Caption = 'PRN22'
      TabOrder = 5
    end
    object PRN23: TCheckBox
      Left = 8
      Top = 114
      Width = 59
      Height = 17
      Caption = 'PRN23'
      TabOrder = 6
    end
    object PRN24: TCheckBox
      Left = 8
      Top = 132
      Width = 59
      Height = 17
      Caption = 'PRN24'
      TabOrder = 7
    end
  end
  object Panel4: TPanel
    Left = 278
    Top = 4
    Width = 65
    Height = 153
    BevelOuter = bvNone
    TabOrder = 5
    object SBAS: TCheckBox
      Left = 8
      Top = 6
      Width = 59
      Height = 17
      Caption = 'SBAS'
      TabOrder = 0
    end
    object GLO: TCheckBox
      Left = 8
      Top = 24
      Width = 59
      Height = 17
      Caption = 'GLO'
      TabOrder = 1
    end
    object GAL: TCheckBox
      Left = 8
      Top = 42
      Width = 59
      Height = 17
      Caption = 'GAL'
      TabOrder = 2
    end
    object PRN33: TCheckBox
      Left = 8
      Top = 60
      Width = 59
      Height = 17
      Caption = 'PRN33-'
      TabOrder = 3
    end
  end
  object Panel5: TPanel
    Left = 210
    Top = 4
    Width = 65
    Height = 153
    BevelOuter = bvNone
    TabOrder = 6
    object PRN25: TCheckBox
      Left = 8
      Top = 6
      Width = 59
      Height = 17
      Caption = 'PRN25'
      TabOrder = 0
    end
    object PRN26: TCheckBox
      Left = 8
      Top = 24
      Width = 59
      Height = 17
      Caption = 'PRN26'
      TabOrder = 1
    end
    object PRN27: TCheckBox
      Left = 8
      Top = 42
      Width = 59
      Height = 17
      Caption = 'PRN27'
      TabOrder = 2
    end
    object PRN28: TCheckBox
      Left = 8
      Top = 60
      Width = 59
      Height = 17
      Caption = 'PRN28'
      TabOrder = 3
    end
    object PRN29: TCheckBox
      Left = 8
      Top = 78
      Width = 59
      Height = 17
      Caption = 'PRN29'
      TabOrder = 4
    end
    object PRN30: TCheckBox
      Left = 8
      Top = 96
      Width = 59
      Height = 17
      Caption = 'PRN30'
      TabOrder = 5
    end
    object PRN31: TCheckBox
      Left = 8
      Top = 114
      Width = 59
      Height = 17
      Caption = 'PRN31'
      TabOrder = 6
    end
    object PRN32: TCheckBox
      Left = 8
      Top = 132
      Width = 59
      Height = 17
      Caption = 'PRN32'
      TabOrder = 7
    end
  end
  object BtnChkAll: TButton
    Left = 6
    Top = 162
    Width = 67
    Height = 23
    Caption = '&Check All'
    TabOrder = 7
    OnClick = BtnChkAllClick
  end
  object BtnUnchkAll: TButton
    Left = 74
    Top = 162
    Width = 69
    Height = 23
    Caption = '&Uncheck All'
    TabOrder = 8
    OnClick = BtnUnchkAllClick
  end
end
