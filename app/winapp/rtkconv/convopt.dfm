object ConvOptDialog: TConvOptDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 357
  ClientWidth = 425
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
  object Label3: TLabel
    Left = 153
    Top = 332
    Width = 31
    Height = 13
    Caption = 'Debug'
  end
  object Label8: TLabel
    Left = 10
    Top = 305
    Width = 82
    Height = 13
    Caption = 'Receiver Options'
  end
  object Label11: TLabel
    Left = 323
    Top = 229
    Width = 89
    Height = 13
    Caption = 'Excluded Satellites'
  end
  object Label9: TLabel
    Left = 10
    Top = 6
    Width = 49
    Height = 13
    Caption = 'RINEX Ver'
  end
  object Label12: TLabel
    Left = 204
    Top = 6
    Width = 48
    Height = 13
    Caption = 'Station ID'
  end
  object Label13: TLabel
    Left = 10
    Top = 332
    Width = 88
    Height = 13
    Caption = 'Time Torelance (s)'
  end
  object BtnOk: TButton
    Left = 255
    Top = 325
    Width = 80
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 339
    Top = 325
    Width = 80
    Height = 29
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object GroupBox1: TGroupBox
    Left = 1
    Top = 24
    Width = 417
    Height = 204
    TabOrder = 6
    object Label1: TLabel
      Left = 10
      Top = 72
      Width = 99
      Height = 13
      Caption = 'Maker Name/#/Type'
    end
    object Label2: TLabel
      Left = 10
      Top = 6
      Width = 104
      Height = 13
      Caption = 'RunBy/Obsrv/Agency'
    end
    object Label4: TLabel
      Left = 10
      Top = 94
      Width = 82
      Height = 13
      Caption = 'Rec #/Type/Vers'
    end
    object Label5: TLabel
      Left = 10
      Top = 116
      Width = 56
      Height = 13
      Caption = 'Ant #/Type'
    end
    object Label6: TLabel
      Left = 10
      Top = 138
      Width = 76
      Height = 13
      Caption = 'Approx Pos XYZ'
    end
    object Label7: TLabel
      Left = 10
      Top = 160
      Width = 100
      Height = 13
      Caption = 'Antenna Delta H/E/N'
    end
    object Label10: TLabel
      Left = 10
      Top = 37
      Width = 45
      Height = 13
      Caption = 'Comment'
    end
    object Marker: TEdit
      Left = 128
      Top = 69
      Width = 95
      Height = 21
      TabOrder = 5
    end
    object RunBy: TEdit
      Left = 128
      Top = 3
      Width = 95
      Height = 21
      TabOrder = 0
    end
    object Name0: TEdit
      Left = 224
      Top = 3
      Width = 95
      Height = 21
      TabOrder = 1
    end
    object Name1: TEdit
      Left = 320
      Top = 3
      Width = 95
      Height = 21
      TabOrder = 2
    end
    object Rec2: TEdit
      Left = 320
      Top = 91
      Width = 95
      Height = 21
      TabOrder = 10
    end
    object Rec1: TEdit
      Left = 224
      Top = 91
      Width = 95
      Height = 21
      TabOrder = 9
    end
    object Rec0: TEdit
      Left = 128
      Top = 91
      Width = 95
      Height = 21
      TabOrder = 8
    end
    object Ant0: TEdit
      Left = 128
      Top = 113
      Width = 95
      Height = 21
      TabOrder = 11
    end
    object Ant1: TEdit
      Left = 224
      Top = 113
      Width = 95
      Height = 21
      TabOrder = 12
    end
    object AppPos2: TEdit
      Left = 320
      Top = 135
      Width = 95
      Height = 21
      TabOrder = 15
      Text = '0.0000'
    end
    object AntDel2: TEdit
      Left = 320
      Top = 157
      Width = 95
      Height = 21
      TabOrder = 18
      Text = '0.0000'
    end
    object AppPos1: TEdit
      Left = 224
      Top = 135
      Width = 95
      Height = 21
      TabOrder = 14
      Text = '0.0000'
    end
    object AntDel1: TEdit
      Left = 224
      Top = 157
      Width = 95
      Height = 21
      TabOrder = 17
      Text = '0.0000'
    end
    object AppPos0: TEdit
      Left = 128
      Top = 135
      Width = 95
      Height = 21
      TabOrder = 13
      Text = '0.0000'
    end
    object AntDel0: TEdit
      Left = 128
      Top = 157
      Width = 95
      Height = 21
      TabOrder = 16
      Text = '0.0000'
    end
    object Comment0: TEdit
      Left = 128
      Top = 25
      Width = 287
      Height = 21
      TabOrder = 3
    end
    object Comment1: TEdit
      Left = 128
      Top = 47
      Width = 287
      Height = 21
      TabOrder = 4
    end
    object MarkerType: TEdit
      Left = 320
      Top = 69
      Width = 95
      Height = 21
      TabOrder = 7
    end
    object MarkerNo: TEdit
      Left = 224
      Top = 69
      Width = 95
      Height = 21
      TabOrder = 6
    end
    object OutIono: TCheckBox
      Left = 187
      Top = 182
      Width = 68
      Height = 18
      Caption = 'Iono Corr'
      TabOrder = 22
    end
    object OutTime: TCheckBox
      Left = 263
      Top = 182
      Width = 65
      Height = 18
      Caption = 'Time Corr'
      TabOrder = 23
    end
    object OutLeaps: TCheckBox
      Left = 341
      Top = 182
      Width = 62
      Height = 18
      Caption = 'Leap Sec'
      TabOrder = 24
    end
    object AutoPos: TCheckBox
      Left = 109
      Top = 137
      Width = 18
      Height = 18
      TabOrder = 19
      OnClick = AutoPosClick
    end
    object HalfCyc: TCheckBox
      Left = 96
      Top = 183
      Width = 82
      Height = 18
      Caption = 'Half Cyc Corr'
      TabOrder = 21
    end
    object PhaseShift: TCheckBox
      Left = 10
      Top = 182
      Width = 76
      Height = 18
      Caption = 'Phase Shift'
      TabOrder = 20
    end
  end
  object GroupBox3: TGroupBox
    Left = 1
    Top = 263
    Width = 132
    Height = 35
    Caption = 'Observation Types'
    TabOrder = 8
    object Obs1: TCheckBox
      Left = 10
      Top = 14
      Width = 30
      Height = 15
      Caption = 'C'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object Obs2: TCheckBox
      Left = 41
      Top = 14
      Width = 30
      Height = 15
      Caption = 'L'
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
    object Obs3: TCheckBox
      Left = 72
      Top = 14
      Width = 30
      Height = 15
      Caption = 'D'
      Checked = True
      State = cbChecked
      TabOrder = 2
    end
    object Obs4: TCheckBox
      Left = 103
      Top = 14
      Width = 30
      Height = 15
      Caption = 'S'
      Checked = True
      State = cbChecked
      TabOrder = 3
    end
  end
  object TraceLevel: TComboBox
    Left = 189
    Top = 329
    Width = 60
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 11
    Text = 'OFF'
    Items.Strings = (
      'OFF'
      'Level 1'
      'Level 2'
      'Level 3'
      'Level 4'
      'Level 5')
  end
  object RcvOption: TEdit
    Left = 103
    Top = 301
    Width = 315
    Height = 21
    TabOrder = 10
  end
  object GroupBox4: TGroupBox
    Left = 137
    Top = 263
    Width = 176
    Height = 35
    Caption = 'GNSS Signals       '
    TabOrder = 9
    object BtnFreq: TSpeedButton
      Left = 71
      Top = -2
      Width = 16
      Height = 16
      Caption = '?'
      Flat = True
      Spacing = 0
      OnClick = BtnFreqClick
    end
    object Freq1: TCheckBox
      Left = 10
      Top = 14
      Width = 30
      Height = 17
      Caption = 'L1'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object Freq2: TCheckBox
      Left = 43
      Top = 14
      Width = 30
      Height = 17
      Caption = 'L2'
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
    object Freq3: TCheckBox
      Left = 76
      Top = 14
      Width = 30
      Height = 17
      Caption = 'L3'
      TabOrder = 2
    end
    object Freq4: TCheckBox
      Left = 109
      Top = 14
      Width = 30
      Height = 17
      Caption = 'L4'
      TabOrder = 3
    end
    object Freq5: TCheckBox
      Left = 142
      Top = 14
      Width = 30
      Height = 17
      Caption = 'L5'
      TabOrder = 4
    end
  end
  object ExSats: TEdit
    Left = 317
    Top = 243
    Width = 101
    Height = 21
    TabOrder = 12
  end
  object RnxVer: TComboBox
    Left = 67
    Top = 2
    Width = 60
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 2
    Text = '2.10'
    OnChange = RnxVerChange
    Items.Strings = (
      '2.10'
      '2.11'
      '2.12'
      '3.00'
      '3.01'
      '3.02'
      '3.03'
      '3.04')
  end
  object RnxFile: TCheckBox
    Left = 327
    Top = 5
    Width = 84
    Height = 17
    Caption = 'RINEX2 Name'
    TabOrder = 5
    OnClick = RnxFileClick
  end
  object RnxCode: TEdit
    Left = 260
    Top = 3
    Width = 60
    Height = 21
    TabOrder = 4
    Text = '0000'
  end
  object GroupBox2: TGroupBox
    Left = 1
    Top = 228
    Width = 312
    Height = 35
    Caption = 'Satellite Systems'
    TabOrder = 7
    object Nav1: TCheckBox
      Left = 10
      Top = 14
      Width = 40
      Height = 15
      Caption = 'GPS'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object Nav2: TCheckBox
      Left = 52
      Top = 14
      Width = 40
      Height = 15
      Caption = 'GLO'
      TabOrder = 1
    end
    object Nav3: TCheckBox
      Left = 94
      Top = 14
      Width = 40
      Height = 15
      Caption = 'GAL'
      TabOrder = 2
    end
    object Nav4: TCheckBox
      Left = 136
      Top = 14
      Width = 40
      Height = 15
      Caption = 'QZS'
      TabOrder = 3
    end
    object Nav5: TCheckBox
      Left = 270
      Top = 14
      Width = 40
      Height = 15
      Caption = 'SBS'
      TabOrder = 6
    end
    object Nav6: TCheckBox
      Left = 178
      Top = 14
      Width = 40
      Height = 15
      Caption = 'BDS'
      TabOrder = 4
    end
    object Nav7: TCheckBox
      Left = 220
      Top = 14
      Width = 45
      Height = 15
      Caption = 'NavIC'
      TabOrder = 5
    end
  end
  object BtnMask: TButton
    Left = 316
    Top = 268
    Width = 50
    Height = 30
    Caption = 'Mask...'
    TabOrder = 13
    OnClick = BtnMaskClick
  end
  object ChkSepNav: TCheckBox
    Left = 137
    Top = 5
    Width = 66
    Height = 17
    Caption = 'Sep NAV'
    TabOrder = 3
  end
  object TimeTol: TEdit
    Left = 103
    Top = 329
    Width = 45
    Height = 21
    TabOrder = 14
    Text = '0.005'
  end
  object BtnFcn: TButton
    Left = 369
    Top = 268
    Width = 50
    Height = 30
    Caption = 'FCN...'
    TabOrder = 15
    OnClick = BtnFcnClick
  end
end
