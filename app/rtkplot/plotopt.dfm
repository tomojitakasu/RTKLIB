object PlotOptDialog: TPlotOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 421
  ClientWidth = 552
  Color = clWhite
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Msg: TLabel
    Left = 4
    Top = 212
    Width = 3
    Height = 13
  end
  object Panel2: TPanel
    Left = 0
    Top = 0
    Width = 552
    Height = 417
    Align = alTop
    BevelOuter = bvNone
    Color = clWhite
    ParentBackground = False
    TabOrder = 2
    ExplicitLeft = 4
    ExplicitTop = -4
    object Label1: TLabel
      Left = 373
      Top = 123
      Width = 84
      Height = 13
      Caption = 'Background Color'
    end
    object Label2: TLabel
      Left = 373
      Top = 145
      Width = 45
      Height = 13
      Caption = 'Plot Style'
    end
    object Label4: TLabel
      Left = 193
      Top = 35
      Width = 74
      Height = 13
      Caption = 'Direction Arrow'
    end
    object Label5: TLabel
      Left = 373
      Top = 101
      Width = 47
      Height = 13
      Caption = 'Grid Color'
    end
    object Label6: TLabel
      Left = 373
      Top = 79
      Width = 50
      Height = 13
      Caption = 'Text Color'
    end
    object Label7: TLabel
      Left = 373
      Top = 57
      Width = 47
      Height = 13
      Caption = 'Line Color'
    end
    object Label9: TLabel
      Left = 12
      Top = 57
      Width = 72
      Height = 13
      Caption = 'Show Statistics'
    end
    object Label8: TLabel
      Left = 193
      Top = 167
      Width = 68
      Height = 13
      Caption = 'Y-Range (+/-)'
    end
    object Label10: TLabel
      Left = 373
      Top = 167
      Width = 45
      Height = 13
      Caption = 'Mark Size'
    end
    object Label12: TLabel
      Left = 12
      Top = 13
      Width = 59
      Height = 13
      Caption = 'Time Format'
    end
    object Label13: TLabel
      Left = 193
      Top = 145
      Width = 38
      Height = 13
      Caption = 'Auto Fit'
    end
    object Label14: TLabel
      Left = 373
      Top = 13
      Width = 87
      Height = 13
      Caption = 'Mark Color 1 (1-6)'
    end
    object Label17: TLabel
      Left = 12
      Top = 79
      Width = 46
      Height = 13
      Caption = 'Cycle-Slip'
    end
    object Label18: TLabel
      Left = 12
      Top = 145
      Width = 87
      Height = 13
      Caption = 'Elevation Mask ('#176')'
    end
    object Label19: TLabel
      Left = 193
      Top = 13
      Width = 73
      Height = 13
      Caption = 'Error Bar/Circle'
    end
    object Label16: TLabel
      Left = 12
      Top = 101
      Width = 75
      Height = 13
      Caption = 'Parity Unknown'
    end
    object Label20: TLabel
      Left = 193
      Top = 57
      Width = 57
      Height = 13
      Caption = 'Graph Label'
    end
    object Label21: TLabel
      Left = 193
      Top = 79
      Width = 70
      Height = 13
      Caption = 'Grid/Grid Label'
    end
    object Label22: TLabel
      Left = 193
      Top = 123
      Width = 25
      Height = 13
      Caption = 'Scale'
    end
    object Label23: TLabel
      Left = 193
      Top = 101
      Width = 43
      Height = 13
      Caption = 'Compass'
    end
    object Label24: TLabel
      Left = 12
      Top = 123
      Width = 49
      Height = 13
      Caption = 'Ephemeris'
    end
    object Label11: TLabel
      Left = 373
      Top = 211
      Width = 88
      Height = 13
      Caption = 'Animation Interval'
    end
    object Label25: TLabel
      Left = 12
      Top = 189
      Width = 84
      Height = 13
      Caption = 'Hide Low Satellite'
    end
    object Label3: TLabel
      Left = 193
      Top = 233
      Width = 84
      Height = 13
      Caption = 'Coordinate Origin'
    end
    object Label26: TLabel
      Left = 12
      Top = 167
      Width = 86
      Height = 13
      Caption = 'Elev Mask Pattern'
    end
    object LabelRefPos: TLabel
      Left = 193
      Top = 257
      Width = 57
      Height = 13
      Caption = 'Lat/Lon/Hgt'
    end
    object Label28: TLabel
      Left = 12
      Top = 211
      Width = 74
      Height = 13
      Caption = 'Max NSAT/DOP'
    end
    object LabelExSats: TLabel
      Left = 11
      Top = 344
      Width = 67
      Height = 13
      Caption = 'Excluded Sats'
    end
    object Label29: TLabel
      Left = 193
      Top = 189
      Width = 68
      Height = 13
      Caption = 'RT Buffer Size'
    end
    object Label27: TLabel
      Left = 373
      Top = 233
      Width = 88
      Height = 13
      Caption = 'Update Cycle (ms)'
    end
    object Label30: TLabel
      Left = 193
      Top = 279
      Width = 39
      Height = 13
      Caption = 'QC Cmd'
    end
    object Label31: TLabel
      Left = 12
      Top = 257
      Width = 82
      Height = 13
      Caption = 'Receiver Position'
    end
    object Label32: TLabel
      Left = 373
      Top = 35
      Width = 87
      Height = 13
      Caption = 'Mark Color 2 (1-6)'
    end
    object Label15: TLabel
      Left = 12
      Top = 35
      Width = 73
      Height = 13
      Caption = 'Lat/Lon Format'
    end
    object Label33: TLabel
      Left = 193
      Top = 301
      Width = 51
      Height = 13
      Caption = 'RINEX Opt'
    end
    object Label34: TLabel
      Left = 193
      Top = 325
      Width = 43
      Height = 13
      Caption = 'TLE Data'
    end
    object Label35: TLabel
      Left = 193
      Top = 347
      Width = 52
      Height = 13
      Caption = 'TLE Sat No'
    end
    object Label36: TLabel
      Left = 8
      Top = 360
      Width = 76
      Height = 13
      Caption = '(+Sn: Included)'
    end
    object BtnTLEView: TSpeedButton
      Left = 500
      Top = 319
      Width = 21
      Height = 21
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnTLEViewClick
    end
    object BtnTLESatView: TSpeedButton
      Left = 500
      Top = 341
      Width = 21
      Height = 21
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
        FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
        FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
        0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      OnClick = BtnTLESatViewClick
    end
    object Label37: TLabel
      Left = 12
      Top = 233
      Width = 67
      Height = 13
      Caption = 'Max Multipath'
    end
    object Label38: TLabel
      Left = 193
      Top = 369
      Width = 95
      Height = 13
      Caption = 'API Key for GMView'
    end
    object Panel1: TPanel
      Left = 361
      Top = 186
      Width = 156
      Height = 27
      BevelOuter = bvNone
      TabOrder = 48
      object FontLabel: TLabel
        Left = 102
        Top = 5
        Width = 52
        Height = 13
        Alignment = taRightJustify
        Caption = 'Font Name'
      end
      object LabelFont: TLabel
        Left = 12
        Top = 4
        Width = 22
        Height = 13
        Caption = 'Font'
      end
    end
    object ShowArrow: TComboBox
      Left = 283
      Top = 32
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 14
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object PlotStyle: TComboBox
      Left = 467
      Top = 142
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 43
      Text = 'Mark/Line'
      Items.Strings = (
        'Mark/Line'
        'Mark'
        'Line'
        'None')
    end
    object Origin: TComboBox
      Left = 283
      Top = 230
      Width = 77
      Height = 21
      Style = csDropDownList
      DropDownCount = 20
      ItemIndex = 0
      TabOrder = 22
      Text = 'Start Pos'
      OnChange = OriginChange
      Items.Strings = (
        'Start Pos'
        'End Pos'
        'Average Pos'
        'Linear Fit Pos'
        'Base Station'
        'Lat/Lon/Hgt'
        'Auto Input'
        'Image Center'
        'Map Center'
        'Waypnt1'
        'Waypnt2'
        'Waypnt3'
        'Waypnt4'
        'Waypnt5'
        'Waypnt6'
        'Waypnt7'
        'Waypnt8'
        'Waypnt9'
        'Waypnt10')
    end
    object Color1: TPanel
      Left = 467
      Top = 120
      Width = 54
      Height = 21
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 35
    end
    object BtnColor1: TButton
      Left = 520
      Top = 119
      Width = 25
      Height = 22
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 36
      OnClick = BtnColor1Click
    end
    object Color2: TPanel
      Left = 467
      Top = 98
      Width = 54
      Height = 21
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clMedGray
      ParentBackground = False
      TabOrder = 37
    end
    object BtnColor2: TButton
      Left = 520
      Top = 97
      Width = 25
      Height = 22
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 38
      OnClick = BtnColor2Click
    end
    object Color3: TPanel
      Left = 467
      Top = 76
      Width = 54
      Height = 21
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 39
    end
    object BtnColor3: TButton
      Left = 520
      Top = 75
      Width = 25
      Height = 22
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 40
      OnClick = BtnColor3Click
    end
    object Color4: TPanel
      Left = 467
      Top = 54
      Width = 54
      Height = 21
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 41
    end
    object BtnColor4: TButton
      Left = 520
      Top = 53
      Width = 25
      Height = 22
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 42
      OnClick = BtnColor4Click
    end
    object ShowStats: TComboBox
      Left = 102
      Top = 54
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 2
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object TimeLabel: TComboBox
      Left = 102
      Top = 10
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 0
      Text = 'www/ssss'
      Items.Strings = (
        'www/ssss'
        'h:m:s GPST'
        'h:m:s UTC'
        'h:m:s LT')
    end
    object AutoScale: TComboBox
      Left = 283
      Top = 142
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 19
      Text = 'OFF'
      OnChange = AutoScaleChange
      Items.Strings = (
        'OFF'
        'ON')
    end
    object MColor1: TPanel
      Left = 467
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 23
      OnClick = MColorClick
    end
    object MColor2: TPanel
      Left = 479
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 24
      OnClick = MColorClick
    end
    object MColor3: TPanel
      Left = 491
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 25
      OnClick = MColorClick
    end
    object MColor4: TPanel
      Left = 503
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 26
      OnClick = MColorClick
    end
    object MColor5: TPanel
      Left = 515
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 27
      OnClick = MColorClick
    end
    object BtnFont: TButton
      Left = 520
      Top = 185
      Width = 25
      Height = 22
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 45
      OnClick = BtnFontClick
    end
    object ShowSlip: TComboBox
      Left = 102
      Top = 76
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 3
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'LG Jump'
        'LLI Flag')
    end
    object ShowErr: TComboBox
      Left = 283
      Top = 10
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 13
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'Bar/Circle'
        'Dots')
    end
    object MarkSize: TComboBox
      Left = 467
      Top = 164
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 44
      Text = '1'
      Items.Strings = (
        '1'
        '2'
        '3'
        '4'
        '5'
        '10'
        '15'
        '20')
    end
    object ShowHalfC: TComboBox
      Left = 102
      Top = 98
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 4
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object YRange: TComboBox
      Left = 283
      Top = 164
      Width = 77
      Height = 21
      DropDownCount = 20
      ItemIndex = 0
      TabOrder = 20
      Text = '0.05'
      Items.Strings = (
        '0.05'
        '0.1'
        '0.2'
        '0.5'
        '1'
        '2'
        '5'
        '10'
        '20'
        '50'
        '100'
        '500'
        '1000'
        '5000'
        '10000')
    end
    object ShowLabel: TComboBox
      Left = 283
      Top = 54
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 15
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object ShowGLabel: TComboBox
      Left = 283
      Top = 76
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 16
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'Grid'
        'Grid/Label'
        'Circles'
        'Circles/Label')
    end
    object ShowScale: TComboBox
      Left = 283
      Top = 120
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 18
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object ShowCompass: TComboBox
      Left = 283
      Top = 98
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 17
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object ShowEph: TComboBox
      Left = 102
      Top = 120
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 5
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object GroupBox1: TGroupBox
      Left = 4
      Top = 279
      Width = 174
      Height = 61
      Caption = 'Satellite System'
      TabOrder = 11
      object NavSys1: TCheckBox
        Left = 8
        Top = 19
        Width = 45
        Height = 17
        Caption = 'GPS'
        Checked = True
        State = cbChecked
        TabOrder = 0
      end
      object NavSys2: TCheckBox
        Left = 49
        Top = 19
        Width = 69
        Height = 17
        Caption = 'GLO'
        TabOrder = 1
      end
      object NavSys5: TCheckBox
        Left = 8
        Top = 41
        Width = 57
        Height = 17
        Caption = 'SBS'
        TabOrder = 4
      end
      object NavSys3: TCheckBox
        Left = 90
        Top = 19
        Width = 57
        Height = 17
        Caption = 'GAL'
        TabOrder = 2
      end
      object NavSys4: TCheckBox
        Left = 131
        Top = 19
        Width = 47
        Height = 17
        Caption = 'QZS'
        TabOrder = 3
      end
      object NavSys6: TCheckBox
        Left = 49
        Top = 41
        Width = 57
        Height = 17
        Caption = 'BDS'
        TabOrder = 5
      end
      object NavSys7: TCheckBox
        Left = 90
        Top = 41
        Width = 57
        Height = 17
        Caption = 'IRN'
        TabOrder = 6
      end
    end
    object ElMask: TComboBox
      Left = 102
      Top = 142
      Width = 77
      Height = 21
      DropDownCount = 20
      TabOrder = 6
      Text = '0'
      Items.Strings = (
        '0'
        '5'
        '10'
        '15'
        '20'
        '25'
        '30'
        '35'
        '40'
        '45')
    end
    object AnimCycle: TComboBox
      Left = 467
      Top = 208
      Width = 77
      Height = 21
      TabOrder = 46
      Text = '1'
      Items.Strings = (
        '1'
        '2'
        '5'
        '10'
        '20'
        '50'
        '100')
    end
    object HideLowSat: TComboBox
      Left = 102
      Top = 186
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 8
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object ElMaskP: TComboBox
      Left = 102
      Top = 164
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 7
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'ON')
    end
    object RefPos1: TEdit
      Left = 254
      Top = 254
      Width = 92
      Height = 21
      TabOrder = 49
      Text = '0'
    end
    object RefPos2: TEdit
      Left = 348
      Top = 254
      Width = 94
      Height = 21
      TabOrder = 50
      Text = '0'
    end
    object RefPos3: TEdit
      Left = 444
      Top = 254
      Width = 76
      Height = 21
      TabOrder = 51
      Text = '0'
    end
    object BtnRefPos: TButton
      Left = 520
      Top = 253
      Width = 25
      Height = 22
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 52
      OnClick = BtnRefPosClick
    end
    object MaxDop: TComboBox
      Left = 102
      Top = 208
      Width = 77
      Height = 21
      DropDownCount = 20
      TabOrder = 9
      Text = '30'
      Items.Strings = (
        '10'
        '30'
        '50'
        '100'
        '200'
        '500')
    end
    object ExSats: TEdit
      Left = 86
      Top = 342
      Width = 92
      Height = 21
      TabOrder = 12
    end
    object BuffSize: TEdit
      Left = 283
      Top = 186
      Width = 77
      Height = 21
      TabOrder = 21
      Text = '10800'
    end
    object RefCycle: TEdit
      Left = 467
      Top = 230
      Width = 77
      Height = 21
      TabOrder = 47
      Text = '100'
    end
    object MColor6: TPanel
      Left = 527
      Top = 11
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 28
      OnClick = MColorClick
    end
    object QcCmd: TEdit
      Left = 254
      Top = 276
      Width = 290
      Height = 21
      TabOrder = 53
      Text = 'teqc'
    end
    object RcvPos: TComboBox
      Left = 102
      Top = 253
      Width = 77
      Height = 21
      Style = csDropDownList
      DropDownCount = 20
      ItemIndex = 0
      TabOrder = 10
      Text = 'Single Solution'
      OnChange = RcvPosChange
      Items.Strings = (
        'Single Solution'
        'Lat/Lon/Hgt'
        'RINEX Header')
    end
    object MColor7: TPanel
      Left = 467
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 29
      OnClick = MColorClick
    end
    object MColor8: TPanel
      Left = 479
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 30
      OnClick = MColorClick
    end
    object MColor9: TPanel
      Left = 491
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 31
      OnClick = MColorClick
    end
    object MColor10: TPanel
      Left = 503
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 32
      OnClick = MColorClick
    end
    object MColor11: TPanel
      Left = 515
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 33
      OnClick = MColorClick
    end
    object MColor12: TPanel
      Left = 527
      Top = 33
      Width = 13
      Height = 19
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Color = clWhite
      ParentBackground = False
      TabOrder = 34
      OnClick = MColorClick
    end
    object LatLonFmt: TComboBox
      Left = 102
      Top = 32
      Width = 77
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 1
      Text = 'ddd.ddddd'
      Items.Strings = (
        'ddd.ddddd'
        'ddd mm ss.ss')
    end
    object RnxOpts: TEdit
      Left = 254
      Top = 298
      Width = 290
      Height = 21
      TabOrder = 54
    end
    object TLEFile: TEdit
      Left = 254
      Top = 320
      Width = 245
      Height = 21
      TabOrder = 55
    end
    object BtnTLEFile: TButton
      Left = 520
      Top = 319
      Width = 25
      Height = 22
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 56
      OnClick = BtnTLEFileClick
    end
    object TLESatFile: TEdit
      Left = 254
      Top = 342
      Width = 245
      Height = 21
      TabOrder = 57
    end
    object BtnTLESatFile: TButton
      Left = 520
      Top = 341
      Width = 25
      Height = 22
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 58
      OnClick = BtnTLESatFileClick
    end
    object MaxMP: TComboBox
      Left = 102
      Top = 230
      Width = 77
      Height = 21
      DropDownCount = 20
      TabOrder = 59
      Text = '10'
      Items.Strings = (
        '1'
        '2'
        '3'
        '5'
        '10'
        '20'
        '30')
    end
    object EditTimeSync: TEdit
      Left = 283
      Top = 208
      Width = 77
      Height = 21
      TabOrder = 60
      Text = '0'
    end
    object ChkTimeSync: TCheckBox
      Left = 190
      Top = 210
      Width = 87
      Height = 17
      Caption = 'Time Sync Port'
      TabOrder = 61
      OnClick = ChkTimeSyncClick
    end
    object ApiKey: TEdit
      Left = 292
      Top = 364
      Width = 253
      Height = 21
      TabOrder = 62
    end
  end
  object BtnCancel: TButton
    Left = 455
    Top = 388
    Width = 92
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object BtnOK: TButton
    Left = 363
    Top = 388
    Width = 92
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOKClick
  end
  object ColorDialog: TColorDialog
    Left = 22
    Top = 374
  end
  object FontDialog: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Options = []
    Left = 95
    Top = 373
  end
  object OpenDialog: TOpenDialog
    Filter = 
      'Text File (*.txt)|*.txt|Position File (*.pos,*.snx)|*.pos;*.snx|' +
      'All File (*.*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 60
    Top = 374
  end
end
