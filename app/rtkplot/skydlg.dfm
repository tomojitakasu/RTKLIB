object SkyImgDialog: TSkyImgDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Sky Image'
  ClientHeight = 209
  ClientWidth = 366
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
    Left = 289
    Top = 180
    Width = 78
    Height = 29
    Caption = '&Close'
    TabOrder = 0
    OnClick = BtnCloseClick
  end
  object BtnSave: TButton
    Left = -1
    Top = 180
    Width = 68
    Height = 29
    Caption = '&Save Tag'
    ModalResult = 1
    TabOrder = 4
    OnClick = BtnSaveClick
  end
  object BtnUpdate: TButton
    Left = 212
    Top = 180
    Width = 78
    Height = 29
    Caption = '&Update'
    TabOrder = 1
    OnClick = BtnUpdateClick
  end
  object Panel1: TPanel
    Left = 1
    Top = 2
    Width = 366
    Height = 28
    BevelOuter = bvNone
    TabOrder = 2
    object Label1: TLabel
      Left = 10
      Top = 7
      Width = 117
      Height = 13
      Caption = 'Image Size W/ H (pixels)'
    end
    object Label2: TLabel
      Left = 212
      Top = 7
      Width = 6
      Height = 13
      Caption = 'x'
    end
    object SkySize1: TEdit
      Left = 144
      Top = 4
      Width = 60
      Height = 21
      ReadOnly = True
      TabOrder = 0
      Text = '0'
    end
    object SkySize2: TEdit
      Left = 226
      Top = 4
      Width = 60
      Height = 21
      ReadOnly = True
      TabOrder = 1
      Text = '0'
    end
    object SkyFlip: TCheckBox
      Left = 316
      Top = 7
      Width = 97
      Height = 15
      Caption = 'Flip'
      TabOrder = 2
      OnMouseUp = SkyFlipMouseUp
    end
  end
  object Panel2: TPanel
    Left = 2
    Top = 31
    Width = 365
    Height = 147
    BevelOuter = bvNone
    TabOrder = 3
    object Label6: TLabel
      Left = 11
      Top = 46
      Width = 17
      Height = 13
      Caption = 'Roll'
    end
    object Label3: TLabel
      Left = 128
      Top = 45
      Width = 23
      Height = 13
      Caption = 'Pitch'
    end
    object Label4: TLabel
      Left = 245
      Top = 45
      Width = 20
      Height = 13
      Caption = 'Yaw'
    end
    object Label7: TLabel
      Left = 78
      Top = 2
      Width = 92
      Height = 13
      Caption = 'Plot Center (pixels)'
    end
    object Label5: TLabel
      Left = 143
      Top = 21
      Width = 6
      Height = 13
      Caption = 'Y'
    end
    object Label9: TLabel
      Left = 20
      Top = 21
      Width = 6
      Height = 13
      Caption = 'X'
    end
    object Label10: TLabel
      Left = 258
      Top = 2
      Width = 91
      Height = 13
      Caption = 'Plot Radius (pixels)'
    end
    object Label11: TLabel
      Left = 21
      Top = 85
      Width = 12
      Height = 13
      Caption = '80'
    end
    object Label12: TLabel
      Left = 60
      Top = 85
      Width = 12
      Height = 13
      Caption = '70'
    end
    object Label13: TLabel
      Left = 99
      Top = 85
      Width = 12
      Height = 13
      Caption = '60'
    end
    object Label14: TLabel
      Left = 138
      Top = 85
      Width = 12
      Height = 13
      Caption = '50'
    end
    object Label15: TLabel
      Left = 176
      Top = 85
      Width = 12
      Height = 13
      Caption = '40'
    end
    object Label16: TLabel
      Left = 215
      Top = 85
      Width = 12
      Height = 13
      Caption = '30'
    end
    object Label17: TLabel
      Left = 254
      Top = 85
      Width = 12
      Height = 13
      Caption = '20'
    end
    object Label18: TLabel
      Left = 293
      Top = 85
      Width = 12
      Height = 13
      Caption = '10'
    end
    object Label8: TLabel
      Left = 105
      Top = 129
      Width = 46
      Height = 13
      Caption = 'Resample'
    end
    object Label19: TLabel
      Left = 336
      Top = 85
      Width = 6
      Height = 13
      Caption = '0'
    end
    object Label20: TLabel
      Left = 324
      Top = 67
      Width = 26
      Height = 13
      Caption = '(deg)'
    end
    object SkyFov1: TEdit
      Left = 38
      Top = 42
      Width = 60
      Height = 21
      TabOrder = 6
      Text = '0.00'
    end
    object SkyFov1UpDown: TUpDown
      Left = 98
      Top = 42
      Width = 17
      Height = 21
      Min = -32000
      Max = 32000
      TabOrder = 7
      Wrap = True
      OnChangingEx = SkyFov1UpDownChangingEx
    end
    object SkyFov2: TEdit
      Left = 159
      Top = 42
      Width = 60
      Height = 21
      TabOrder = 8
      Text = '0.00'
    end
    object SkyFov2UpDown: TUpDown
      Left = 219
      Top = 42
      Width = 17
      Height = 21
      Min = -32000
      Max = 32000
      TabOrder = 9
      Wrap = True
      OnChangingEx = SkyFov2UpDownChangingEx
    end
    object SkyElMask: TCheckBox
      Left = 10
      Top = 127
      Width = 93
      Height = 17
      Caption = 'Elevation Mask'
      Checked = True
      State = cbChecked
      TabOrder = 22
      OnMouseUp = SkyElMaskMouseUp
    end
    object SkyFov3: TEdit
      Left = 273
      Top = 42
      Width = 60
      Height = 21
      TabOrder = 10
      Text = '0.00'
    end
    object SkyFov3UpDown: TUpDown
      Left = 333
      Top = 42
      Width = 17
      Height = 21
      Min = -32000
      Max = 32000
      TabOrder = 11
      Wrap = True
      OnChangingEx = SkyFov3UpDownChangingEx
    end
    object SkyCent1: TEdit
      Left = 38
      Top = 18
      Width = 60
      Height = 21
      TabOrder = 0
      Text = '0.00'
    end
    object SkyCent2: TEdit
      Left = 159
      Top = 18
      Width = 60
      Height = 21
      TabOrder = 2
      Text = '0.00'
    end
    object SkyScale: TEdit
      Left = 273
      Top = 18
      Width = 60
      Height = 21
      TabOrder = 4
      Text = '0.00'
    end
    object SkyCent1UpDown: TUpDown
      Left = 98
      Top = 18
      Width = 17
      Height = 21
      Min = -32000
      Max = 32000
      TabOrder = 1
      Wrap = True
      OnChangingEx = SkyCent1UpDownChangingEx
    end
    object SkyScaleUpDown: TUpDown
      Left = 333
      Top = 18
      Width = 17
      Height = 21
      Min = -32000
      Max = 32000
      TabOrder = 5
      Wrap = True
      OnChangingEx = SkyScaleUpDownChangingEx
    end
    object SkyCent2UpDown: TUpDown
      Left = 219
      Top = 18
      Width = 17
      Height = 21
      Min = -32000
      Max = 32000
      TabOrder = 3
      Wrap = True
      OnChangingEx = SkyCent2UpDownChangingEx
    end
    object SkyDestCorr: TCheckBox
      Left = 10
      Top = 67
      Width = 280
      Height = 17
      Caption = 'Pixels from Center - Elevation'
      Checked = True
      State = cbChecked
      TabOrder = 12
      OnMouseUp = SkyDestCorrMouseUp
    end
    object SkyDest1: TEdit
      Left = 9
      Top = 99
      Width = 38
      Height = 21
      TabOrder = 13
      Text = '0.0'
    end
    object SkyDest2: TEdit
      Left = 48
      Top = 99
      Width = 38
      Height = 21
      TabOrder = 14
      Text = '0.0'
    end
    object SkyDest3: TEdit
      Left = 87
      Top = 99
      Width = 38
      Height = 21
      TabOrder = 15
      Text = '0.0'
    end
    object SkyDest4: TEdit
      Left = 126
      Top = 99
      Width = 38
      Height = 21
      TabOrder = 16
      Text = '0.0'
    end
    object SkyDest5: TEdit
      Left = 165
      Top = 99
      Width = 38
      Height = 21
      TabOrder = 17
      Text = '0.0'
    end
    object SkyDest6: TEdit
      Left = 204
      Top = 99
      Width = 38
      Height = 21
      TabOrder = 18
      Text = '0.0'
    end
    object SkyDest7: TEdit
      Left = 243
      Top = 99
      Width = 38
      Height = 21
      TabOrder = 19
      Text = '0.0'
    end
    object SkyDest8: TEdit
      Left = 282
      Top = 99
      Width = 38
      Height = 21
      TabOrder = 20
      Text = '0.0'
    end
    object SkyRes: TComboBox
      Left = 156
      Top = 125
      Width = 37
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 23
      Text = 'NN'
      OnChange = SkyResChange
      Items.Strings = (
        'NN'
        'BL')
    end
    object SkyDest9: TEdit
      Left = 321
      Top = 99
      Width = 38
      Height = 21
      TabOrder = 21
      Text = '0.0'
    end
    object SkyBinarize: TCheckBox
      Left = 207
      Top = 127
      Width = 61
      Height = 17
      Caption = 'Binarize'
      TabOrder = 24
      OnMouseUp = SkyBinarizeMouseUp
    end
    object SkyBinThres2: TEdit
      Left = 313
      Top = 126
      Width = 30
      Height = 21
      TabOrder = 27
      Text = '0.00'
    end
    object SkyBinThres2UpDown: TUpDown
      Left = 343
      Top = 126
      Width = 17
      Height = 21
      Min = -32000
      Max = 32000
      TabOrder = 28
      Wrap = True
      OnChangingEx = SkyBinThres2UpDownChangingEx
    end
    object SkyBinThres1: TEdit
      Left = 266
      Top = 126
      Width = 30
      Height = 21
      TabOrder = 25
      Text = '0.00'
    end
    object SkyBinThres1UpDown: TUpDown
      Left = 296
      Top = 126
      Width = 17
      Height = 21
      Min = -32000
      Max = 32000
      TabOrder = 26
      Wrap = True
      OnChangingEx = SkyBinThres1UpDownChangingEx
    end
  end
  object BtnLoad: TButton
    Left = 66
    Top = 180
    Width = 68
    Height = 29
    Caption = '&Load Tag...'
    ModalResult = 1
    TabOrder = 5
    OnClick = BtnLoadClick
  end
  object BtnGenMask: TButton
    Left = 139
    Top = 180
    Width = 68
    Height = 29
    Caption = '&Gen Mask'
    ModalResult = 1
    TabOrder = 6
    OnClick = BtnGenMaskClick
  end
  object OpenTagDialog: TOpenDialog
    Filter = 'Tag File (*.tag)|*.tag|All (*.*)|*.*'
    Title = 'Load Tag'
    Left = 5
    Top = 4
  end
end
