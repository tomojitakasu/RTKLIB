object ConvOptDialog: TConvOptDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 321
  ClientWidth = 421
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
  object Label3: TLabel
    Left = 151
    Top = 302
    Width = 31
    Height = 13
    Caption = 'Debug'
  end
  object Label8: TLabel
    Left = 6
    Top = 301
    Width = 32
    Height = 13
    Caption = 'Option'
  end
  object Label11: TLabel
    Left = 322
    Top = 229
    Width = 89
    Height = 13
    Caption = 'Excluded Satellites'
  end
  object Label9: TLabel
    Left = 10
    Top = 6
    Width = 68
    Height = 13
    Caption = 'RINEX Version'
  end
  object Label12: TLabel
    Left = 172
    Top = 6
    Width = 48
    Height = 13
    Caption = 'Station ID'
  end
  object BtnOk: TButton
    Left = 256
    Top = 297
    Width = 81
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 338
    Top = 297
    Width = 81
    Height = 23
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object GroupBox1: TGroupBox
    Left = 1
    Top = 25
    Width = 417
    Height = 204
    TabOrder = 5
    object Label1: TLabel
      Left = 10
      Top = 71
      Width = 99
      Height = 13
      Caption = 'Maker Name/#/Type'
    end
    object Label2: TLabel
      Left = 10
      Top = 6
      Width = 100
      Height = 13
      Caption = 'RunBy/Obsv/Agency'
    end
    object Label4: TLabel
      Left = 10
      Top = 93
      Width = 82
      Height = 13
      Caption = 'Rec #/Type/Vers'
    end
    object Label5: TLabel
      Left = 10
      Top = 115
      Width = 56
      Height = 13
      Caption = 'Ant #/Type'
    end
    object Label6: TLabel
      Left = 10
      Top = 137
      Width = 76
      Height = 13
      Caption = 'Approx Pos XYZ'
    end
    object Label7: TLabel
      Left = 10
      Top = 159
      Width = 76
      Height = 13
      Caption = 'Ant Delta H/E/N'
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
    object Ant2: TEdit
      Left = 320
      Top = 113
      Width = 95
      Height = 21
      TabOrder = 13
    end
    object AppPos2: TEdit
      Left = 320
      Top = 135
      Width = 95
      Height = 21
      TabOrder = 16
      Text = '0.0000'
    end
    object AntDel2: TEdit
      Left = 320
      Top = 157
      Width = 95
      Height = 21
      TabOrder = 19
      Text = '0.0000'
    end
    object AppPos1: TEdit
      Left = 224
      Top = 135
      Width = 95
      Height = 21
      TabOrder = 15
      Text = '0.0000'
    end
    object AntDel1: TEdit
      Left = 224
      Top = 157
      Width = 95
      Height = 21
      TabOrder = 18
      Text = '0.0000'
    end
    object AppPos0: TEdit
      Left = 128
      Top = 135
      Width = 95
      Height = 21
      TabOrder = 14
      Text = '0.0000'
    end
    object AntDel0: TEdit
      Left = 128
      Top = 157
      Width = 95
      Height = 21
      TabOrder = 17
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
    object ScanObs: TCheckBox
      Left = 10
      Top = 183
      Width = 97
      Height = 18
      Caption = 'Scan Obs Types'
      TabOrder = 20
    end
    object OutIono: TCheckBox
      Left = 209
      Top = 183
      Width = 68
      Height = 18
      Caption = 'Iono Corr'
      TabOrder = 21
    end
    object OutTime: TCheckBox
      Left = 281
      Top = 183
      Width = 65
      Height = 18
      Caption = 'Time Corr'
      TabOrder = 22
    end
    object OutLeaps: TCheckBox
      Left = 351
      Top = 183
      Width = 62
      Height = 18
      Caption = 'Leap Sec'
      TabOrder = 23
    end
    object AutoPos: TCheckBox
      Left = 109
      Top = 137
      Width = 18
      Height = 18
      TabOrder = 24
      OnClick = AutoPosClick
    end
  end
  object GroupBox3: TGroupBox
    Left = 1
    Top = 262
    Width = 142
    Height = 35
    Caption = 'Observation Types'
    TabOrder = 7
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
      Left = 42
      Top = 14
      Width = 30
      Height = 15
      Caption = 'L'
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
    object Obs3: TCheckBox
      Left = 76
      Top = 14
      Width = 30
      Height = 15
      Caption = 'D'
      Checked = True
      State = cbChecked
      TabOrder = 2
    end
    object Obs4: TCheckBox
      Left = 109
      Top = 14
      Width = 27
      Height = 15
      Caption = 'S'
      Checked = True
      State = cbChecked
      TabOrder = 3
    end
  end
  object TraceLevel: TComboBox
    Left = 185
    Top = 298
    Width = 63
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 10
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
    Left = 40
    Top = 297
    Width = 103
    Height = 21
    TabOrder = 9
  end
  object GroupBox4: TGroupBox
    Left = 144
    Top = 262
    Width = 274
    Height = 35
    Caption = 'Frequencies'
    TabOrder = 8
    object Freq1: TCheckBox
      Left = 10
      Top = 14
      Width = 51
      Height = 17
      Caption = 'L1'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object Freq2: TCheckBox
      Left = 43
      Top = 14
      Width = 37
      Height = 17
      Caption = 'L2'
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
    object Freq3: TCheckBox
      Left = 76
      Top = 14
      Width = 53
      Height = 17
      Caption = 'L5/L3'
      TabOrder = 2
    end
    object Freq4: TCheckBox
      Left = 124
      Top = 14
      Width = 45
      Height = 17
      Caption = 'L6'
      TabOrder = 3
    end
    object Freq5: TCheckBox
      Left = 157
      Top = 14
      Width = 39
      Height = 17
      Caption = 'L7'
      TabOrder = 4
    end
    object Freq6: TCheckBox
      Left = 189
      Top = 14
      Width = 31
      Height = 17
      Caption = 'L8'
      TabOrder = 5
    end
    object BtnMask: TButton
      Left = 223
      Top = 9
      Width = 48
      Height = 23
      Caption = 'Mask...'
      TabOrder = 6
      OnClick = BtnMaskClick
    end
  end
  object ExSats: TEdit
    Left = 315
    Top = 243
    Width = 102
    Height = 21
    TabOrder = 11
  end
  object RnxVer: TComboBox
    Left = 84
    Top = 2
    Width = 83
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
      '3.02')
  end
  object RnxFile: TCheckBox
    Left = 327
    Top = 5
    Width = 84
    Height = 17
    Caption = 'RINEX Name'
    TabOrder = 3
    OnClick = RnxFileClick
  end
  object RnxCode: TEdit
    Left = 225
    Top = 4
    Width = 95
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
    TabOrder = 6
    object Nav1: TCheckBox
      Left = 10
      Top = 14
      Width = 47
      Height = 15
      Caption = 'GPS'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object Nav2: TCheckBox
      Left = 56
      Top = 14
      Width = 71
      Height = 15
      Caption = 'GLO'
      TabOrder = 1
    end
    object Nav3: TCheckBox
      Left = 102
      Top = 14
      Width = 59
      Height = 15
      Caption = 'Galileo'
      TabOrder = 2
    end
    object Nav4: TCheckBox
      Left = 157
      Top = 14
      Width = 55
      Height = 15
      Caption = 'QZSS'
      TabOrder = 3
    end
    object Nav5: TCheckBox
      Left = 208
      Top = 14
      Width = 49
      Height = 15
      Caption = 'SBAS'
      TabOrder = 4
    end
    object Nav6: TCheckBox
      Left = 257
      Top = 14
      Width = 49
      Height = 15
      Caption = 'BeiDou'
      TabOrder = 5
    end
  end
end
