object MapAreaDialog: TMapAreaDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Map Image'
  ClientHeight = 122
  ClientWidth = 341
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
  object BtnClose: TButton
    Left = 256
    Top = 92
    Width = 83
    Height = 29
    Caption = '&Close'
    TabOrder = 0
    OnClick = BtnCloseClick
  end
  object BtnCenter: TButton
    Left = 86
    Top = 92
    Width = 83
    Height = 29
    Caption = '&Center'
    ModalResult = 1
    TabOrder = 2
    OnClick = BtnCenterClick
  end
  object BtnSave: TButton
    Left = 2
    Top = 92
    Width = 83
    Height = 29
    Caption = '&Save Tag'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnSaveClick
  end
  object BtnUpdate: TButton
    Left = 172
    Top = 92
    Width = 83
    Height = 29
    Caption = '&Update'
    TabOrder = 3
    OnClick = BtnUpdateClick
  end
  object Panel1: TPanel
    Left = 2
    Top = 2
    Width = 337
    Height = 29
    BevelOuter = bvNone
    TabOrder = 4
    object Label1: TLabel
      Left = 10
      Top = 7
      Width = 114
      Height = 13
      Caption = 'Image Size W/H (pixels)'
    end
    object Label2: TLabel
      Left = 226
      Top = 7
      Width = 6
      Height = 13
      Caption = 'x'
    end
    object MapSize1: TEdit
      Left = 144
      Top = 4
      Width = 75
      Height = 21
      ReadOnly = True
      TabOrder = 0
      Text = '0'
    end
    object MapSize2: TEdit
      Left = 240
      Top = 4
      Width = 75
      Height = 21
      ReadOnly = True
      TabOrder = 1
      Text = '0'
    end
  end
  object Panel2: TPanel
    Left = 2
    Top = 32
    Width = 337
    Height = 57
    BevelOuter = bvNone
    TabOrder = 5
    object Label5: TLabel
      Left = 10
      Top = 7
      Width = 121
      Height = 13
      Caption = 'Image Center Lon/Lat ('#176')'
    end
    object Label6: TLabel
      Left = 10
      Top = 35
      Width = 89
      Height = 13
      Caption = 'Scale X/Y (m/pixel)'
    end
    object ScaleX: TEdit
      Left = 144
      Top = 32
      Width = 75
      Height = 21
      NumbersOnly = True
      OEMConvert = True
      TabOrder = 0
      Text = '0.000'
      OnKeyDown = ScaleXKeyDown
    end
    object ScaleXUpDown: TUpDown
      Left = 219
      Top = 31
      Width = 21
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 1
      Wrap = True
      OnChangingEx = ScaleXUpDownChangingEx
    end
    object ScaleY: TEdit
      Left = 240
      Top = 32
      Width = 75
      Height = 21
      NumbersOnly = True
      OEMConvert = True
      TabOrder = 2
      Text = '0.000'
      OnKeyDown = ScaleYKeyDown
    end
    object ScaleYUpDown: TUpDown
      Left = 315
      Top = 31
      Width = 21
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 3
      Wrap = True
      OnChangingEx = ScaleYUpDownChangingEx
    end
    object Lat: TEdit
      Left = 240
      Top = 4
      Width = 75
      Height = 21
      NumbersOnly = True
      OEMConvert = True
      TabOrder = 4
      Text = '0.000000'
      OnKeyDown = LatKeyDown
    end
    object LatUpDown: TUpDown
      Left = 315
      Top = 3
      Width = 21
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 5
      Wrap = True
      OnChangingEx = LatUpDownChangingEx
    end
    object Lon: TEdit
      Left = 144
      Top = 4
      Width = 75
      Height = 21
      NumbersOnly = True
      OEMConvert = True
      TabOrder = 6
      Text = '0.000000'
      OnKeyDown = LonKeyDown
    end
    object LonUpDown: TUpDown
      Left = 219
      Top = 3
      Width = 21
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 7
      Wrap = True
      OnChangingEx = LonUpDownChangingEx
    end
    object ScaleEq: TCheckBox
      Left = 102
      Top = 34
      Width = 39
      Height = 17
      Caption = 'X=Y'
      TabOrder = 8
      OnClick = ScaleEqClick
    end
  end
end
