object OptDialog: TOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 285
  ClientWidth = 382
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
  object Label47: TLabel
    Left = 10
    Top = 183
    Width = 93
    Height = 13
    Caption = 'Station Position File'
  end
  object BtnCancel: TButton
    Left = 306
    Top = 262
    Width = 69
    Height = 21
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object BtnOk: TButton
    Left = 236
    Top = 262
    Width = 69
    Height = 21
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnSave: TButton
    Left = 160
    Top = 262
    Width = 69
    Height = 21
    Caption = '&Save'
    TabOrder = 3
    OnClick = BtnSaveClick
  end
  object BtnLoad: TButton
    Left = 90
    Top = 262
    Width = 69
    Height = 21
    Caption = '&Load'
    TabOrder = 2
    OnClick = BtnLoadClick
  end
  object Options: TPageControl
    Left = 0
    Top = 0
    Width = 382
    Height = 259
    ActivePage = TabSheet1
    Align = alTop
    TabOrder = 4
    object TabSheet1: TTabSheet
      Caption = 'Setting&1'
      object Label3: TLabel
        Left = 24
        Top = 73
        Width = 183
        Height = 13
        Caption = 'Rec Dynamics / Earth Tides Correction'
      end
      object Label9: TLabel
        Left = 24
        Top = 117
        Width = 114
        Height = 13
        Caption = 'Troposphere Correction'
      end
      object Label8: TLabel
        Left = 24
        Top = 95
        Width = 108
        Height = 13
        Caption = 'Ionosphere Correction'
      end
      object LabelPosMode: TLabel
        Left = 24
        Top = 7
        Width = 80
        Height = 13
        Caption = 'Positioning Mode'
      end
      object LabelFreq: TLabel
        Left = 24
        Top = 29
        Width = 119
        Height = 13
        Caption = 'Frequencies / Filter Type'
      end
      object LabelElMask: TLabel
        Left = 24
        Top = 51
        Width = 179
        Height = 13
        Caption = 'Elevation Mask ('#176') / SNR Mask (dbHz)'
      end
      object Label32: TLabel
        Left = 24
        Top = 139
        Width = 119
        Height = 13
        Caption = 'Satellite Ephemeris/Clock'
      end
      object Label35: TLabel
        Left = 24
        Top = 186
        Width = 176
        Height = 13
        Caption = 'Excluded Satellites (+PRN: Included)'
      end
      object NavSys1: TCheckBox
        Left = 24
        Top = 206
        Width = 49
        Height = 17
        Caption = 'GPS'
        Checked = True
        State = cbChecked
        TabOrder = 9
      end
      object DynamicModel: TComboBox
        Left = 221
        Top = 70
        Width = 69
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 3
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'ON')
      end
      object IonoOpt: TComboBox
        Left = 221
        Top = 92
        Width = 138
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 5
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'Broadcast'
          'SBAS'
          'Iono-Free LC'
          'Estimate STEC'
          'IONEX TEC'
          'QZSS Broadcast'
          'QZSS LEX')
      end
      object TropOpt: TComboBox
        Left = 221
        Top = 114
        Width = 138
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 6
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'Saastamoinen'
          'SBAS'
          'Estimate ZTD'
          'Estimate ZTD+Grad')
      end
      object PosMode: TComboBox
        Left = 221
        Top = 4
        Width = 138
        Height = 21
        Style = csDropDownList
        DropDownCount = 10
        ItemIndex = 0
        TabOrder = 0
        Text = 'Single'
        OnChange = PosModeChange
        Items.Strings = (
          'Single'
          'DGPS/DGNSS'
          'Kinematic'
          'Static'
          'Moving-Base'
          'Fixed'
          'PPP Kinematic'
          'PPP Static'
          'PPP Fixed')
      end
      object Freq: TComboBox
        Left = 221
        Top = 26
        Width = 69
        Height = 21
        Style = csDropDownList
        ItemIndex = 1
        TabOrder = 1
        Text = 'L1+L2'
        OnChange = FreqChange
        Items.Strings = (
          'L1'
          'L1+L2')
      end
      object Solution: TComboBox
        Left = 291
        Top = 26
        Width = 68
        Height = 21
        Style = csDropDownList
        Enabled = False
        ItemIndex = 0
        TabOrder = 2
        Text = 'Forward'
        Items.Strings = (
          'Forward'
          'Backward'
          'Combined')
      end
      object SatEphem: TComboBox
        Left = 221
        Top = 136
        Width = 138
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 7
        Text = 'Broadcast'
        Items.Strings = (
          'Broadcast'
          'Precise'
          'Broadcast+SBAS'
          'Broadcast+SSR APC'
          'Broadcast+SSR CoM'
          'QZSS LEX')
      end
      object ExSatsE: TEdit
        Left = 221
        Top = 183
        Width = 139
        Height = 21
        TabOrder = 8
      end
      object NavSys2: TCheckBox
        Left = 68
        Top = 206
        Width = 71
        Height = 17
        Caption = 'GLO'
        TabOrder = 10
        OnClick = NavSys2Click
      end
      object NavSys3: TCheckBox
        Left = 114
        Top = 206
        Width = 61
        Height = 17
        Caption = 'Galileo'
        TabOrder = 11
      end
      object NavSys4: TCheckBox
        Left = 168
        Top = 206
        Width = 61
        Height = 17
        Caption = 'QZSS'
        TabOrder = 12
      end
      object NavSys5: TCheckBox
        Left = 218
        Top = 206
        Width = 51
        Height = 17
        Caption = 'SBAS'
        TabOrder = 13
      end
      object TideCorr: TComboBox
        Left = 291
        Top = 70
        Width = 68
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 4
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'Solid')
      end
      object NavSys6: TCheckBox
        Left = 268
        Top = 204
        Width = 69
        Height = 19
        Caption = 'BeiDou'
        TabOrder = 14
        OnClick = NavSys6Click
      end
      object ElMask: TComboBox
        Left = 221
        Top = 48
        Width = 69
        Height = 21
        AutoComplete = False
        DropDownCount = 16
        TabOrder = 15
        Text = '15'
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
          '45'
          '50'
          '55'
          '60'
          '65'
          '70')
      end
      object PosOpt1: TCheckBox
        Left = 24
        Top = 160
        Width = 61
        Height = 17
        Caption = 'Sat PCV'
        TabOrder = 16
      end
      object PosOpt2: TCheckBox
        Left = 84
        Top = 160
        Width = 62
        Height = 17
        Caption = 'Rec PCV'
        TabOrder = 17
      end
      object PosOpt3: TCheckBox
        Left = 146
        Top = 160
        Width = 69
        Height = 17
        Caption = 'Ph-Windup'
        TabOrder = 18
      end
      object PosOpt4: TCheckBox
        Left = 218
        Top = 160
        Width = 68
        Height = 17
        Caption = 'Reject Ecl'
        TabOrder = 19
      end
      object PosOpt5: TCheckBox
        Left = 292
        Top = 160
        Width = 68
        Height = 17
        Caption = 'RAIM FDE'
        TabOrder = 20
      end
      object BtnSnrMask: TButton
        Left = 290
        Top = 47
        Width = 70
        Height = 23
        Caption = '...'
        TabOrder = 21
        OnClick = BtnSnrMaskClick
      end
    end
    object TabSheet2: TTabSheet
      Caption = 'Setting&2'
      ImageIndex = 1
      object Label25: TLabel
        Left = 24
        Top = 7
        Width = 184
        Height = 13
        Caption = 'Integer Ambiguity Res (GPS/GLO/BDS)'
      end
      object Label24: TLabel
        Left = 24
        Top = 33
        Width = 124
        Height = 13
        Caption = 'Min Ratio to Fix Ambiguity'
      end
      object Label13: TLabel
        Left = 24
        Top = 77
        Width = 164
        Height = 13
        Caption = 'Min Lock / Elevation ('#176') to Fix Amb'
      end
      object LabelHold: TLabel
        Left = 24
        Top = 99
        Width = 164
        Height = 13
        Caption = 'Min Fix / Elevation ('#176') to Hold Amb'
      end
      object Label22: TLabel
        Left = 24
        Top = 123
        Width = 179
        Height = 13
        Caption = 'Outage to Reset Amb / Slip Thres (m)'
      end
      object Label14: TLabel
        Left = 24
        Top = 147
        Width = 165
        Height = 13
        Caption = 'Max Age of Diff (s) / Sync Solution'
      end
      object Label11: TLabel
        Left = 24
        Top = 169
        Width = 176
        Height = 13
        Caption = 'Reject Threshold of GDOP/Innov (m)'
      end
      object Label37: TLabel
        Left = 24
        Top = 191
        Width = 122
        Height = 13
        Caption = 'Number of Filter Iteration'
      end
      object Label12: TLabel
        Left = 24
        Top = 55
        Width = 179
        Height = 13
        Caption = 'Min Confidence / Max FCB to Fix Amb'
      end
      object AmbRes: TComboBox
        Left = 221
        Top = 4
        Width = 47
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 0
        Text = 'OFF'
        OnChange = AmbResChange
        Items.Strings = (
          'OFF'
          'Continuous'
          'Instantaneous'
          'Fix and Hold')
      end
      object ValidThresAR: TEdit
        Left = 221
        Top = 30
        Width = 139
        Height = 21
        TabOrder = 2
        Text = '3.0'
      end
      object LockCntFixAmb: TEdit
        Left = 221
        Top = 74
        Width = 69
        Height = 21
        TabOrder = 3
        Text = '0'
      end
      object OutCntResetAmb: TEdit
        Left = 221
        Top = 122
        Width = 69
        Height = 21
        TabOrder = 5
        Text = '5'
      end
      object ElMaskAR: TEdit
        Left = 291
        Top = 74
        Width = 69
        Height = 21
        TabOrder = 4
        Text = '0'
      end
      object SlipThres: TEdit
        Left = 291
        Top = 122
        Width = 69
        Height = 21
        TabOrder = 6
        Text = '0.05'
      end
      object MaxAgeDiff: TEdit
        Left = 221
        Top = 144
        Width = 69
        Height = 21
        TabOrder = 7
        Text = '30'
      end
      object RejectThres: TEdit
        Left = 291
        Top = 166
        Width = 69
        Height = 21
        TabOrder = 10
        Text = '30'
      end
      object NumIter: TEdit
        Left = 221
        Top = 188
        Width = 139
        Height = 21
        TabOrder = 11
        Text = '1'
      end
      object GloAmbRes: TComboBox
        Left = 269
        Top = 4
        Width = 45
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 1
        Text = 'OFF'
        OnChange = AmbResChange
        Items.Strings = (
          'OFF'
          'ON'
          'Auto Calibration')
      end
      object BaselineConst: TCheckBox
        Left = 24
        Top = 212
        Width = 181
        Height = 17
        Caption = 'Baseline Length Constraint (m)'
        TabOrder = 12
        OnClick = BaselineConstClick
      end
      object BaselineLen: TEdit
        Left = 221
        Top = 210
        Width = 69
        Height = 21
        TabOrder = 13
        Text = '0.000'
      end
      object BaselineSig: TEdit
        Left = 291
        Top = 210
        Width = 69
        Height = 21
        TabOrder = 14
        Text = '0.000'
      end
      object FixCntHoldAmb: TEdit
        Left = 221
        Top = 96
        Width = 69
        Height = 21
        TabOrder = 15
        Text = '10'
      end
      object ElMaskHold: TEdit
        Left = 291
        Top = 96
        Width = 69
        Height = 21
        TabOrder = 16
        Text = '10'
      end
      object RejectGdop: TEdit
        Left = 221
        Top = 166
        Width = 69
        Height = 21
        TabOrder = 9
        Text = '30'
      end
      object ThresAR2: TEdit
        Left = 221
        Top = 52
        Width = 69
        Height = 21
        TabOrder = 17
        Text = '0.9999'
      end
      object ThresAR3: TEdit
        Left = 291
        Top = 52
        Width = 69
        Height = 21
        TabOrder = 18
        Text = '0.20'
      end
      object SyncSol: TComboBox
        Left = 291
        Top = 144
        Width = 68
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 8
        Text = 'OFF'
        OnChange = AmbResChange
        Items.Strings = (
          'OFF'
          'ON')
      end
      object BdsAmbRes: TComboBox
        Left = 315
        Top = 4
        Width = 45
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 19
        Text = 'OFF'
        OnChange = AmbResChange
        Items.Strings = (
          'OFF'
          'ON')
      end
    end
    object TabSheet3: TTabSheet
      Caption = 'O&utput'
      ImageIndex = 2
      object LabelSolFormat: TLabel
        Left = 24
        Top = 7
        Width = 75
        Height = 13
        Caption = 'Solution Format'
      end
      object LabelTimeFormat: TLabel
        Left = 24
        Top = 51
        Width = 134
        Height = 13
        Caption = 'Time Format / # of Decimals'
      end
      object LabelLatLonFormat: TLabel
        Left = 24
        Top = 73
        Width = 133
        Height = 13
        Caption = 'Latitude / Longitude Format'
      end
      object LabelFieldSep: TLabel
        Left = 24
        Top = 97
        Width = 73
        Height = 13
        Caption = 'Field Separator'
      end
      object Label2: TLabel
        Left = 24
        Top = 119
        Width = 66
        Height = 13
        Caption = 'Datum/Height'
      end
      object Label18: TLabel
        Left = 24
        Top = 141
        Width = 58
        Height = 13
        Caption = 'Geoid Model'
      end
      object Label20: TLabel
        Left = 24
        Top = 29
        Width = 167
        Height = 13
        Caption = 'Output Header/Processing Options'
      end
      object Label36: TLabel
        Left = 24
        Top = 211
        Width = 180
        Height = 13
        Caption = 'Output Solution Status / Debug Trace'
      end
      object Label17: TLabel
        Left = 24
        Top = 187
        Width = 185
        Height = 13
        Caption = 'NMEA Interval (s) RMC/GGA, GSA/GSV'
      end
      object Label21: TLabel
        Left = 24
        Top = 163
        Width = 114
        Height = 13
        Caption = 'Solution for Static Mode'
        Enabled = False
      end
      object SolFormat: TComboBox
        Left = 221
        Top = 4
        Width = 139
        Height = 21
        Style = csDropDownList
        Enabled = False
        ItemIndex = 0
        TabOrder = 0
        Text = 'Lat/Lon/Height'
        OnChange = SolFormatChange
        Items.Strings = (
          'Lat/Lon/Height'
          'X/Y/Z-ECEF'
          'E/N/U-Baseline'
          'NMEA0183')
      end
      object TimeFormat: TComboBox
        Left = 221
        Top = 50
        Width = 106
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 3
        Text = 'ww ssss GPST'
        Items.Strings = (
          'ww ssss GPST'
          'hh:mm:ss GPST'
          'hh:mm:ss UTC'
          'hh:mm:ss JST')
      end
      object LatLonFormat: TComboBox
        Left = 221
        Top = 72
        Width = 139
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 5
        Text = 'ddd.ddddddd'
        Items.Strings = (
          'ddd.ddddddd'
          'ddd mm ss.sss')
      end
      object FieldSep: TEdit
        Left = 221
        Top = 94
        Width = 139
        Height = 21
        TabOrder = 6
      end
      object OutputDatum: TComboBox
        Left = 221
        Top = 116
        Width = 69
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 7
        Text = 'WGS84'
        Items.Strings = (
          'WGS84')
      end
      object OutputHeight: TComboBox
        Left = 291
        Top = 116
        Width = 69
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 8
        Text = 'Ellipsoidal'
        OnClick = OutputHeightClick
        Items.Strings = (
          'Ellipsoidal'
          'Geodetic')
      end
      object OutputGeoid: TComboBox
        Left = 221
        Top = 138
        Width = 139
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 9
        Text = 'Internal'
        Items.Strings = (
          'Internal'
          'EGM96-BE (15")'
          'EGM2008-SE (2.5")'
          'EGM2008-SE (1")'
          'GSI2000 (1x1.5")')
      end
      object OutputHead: TComboBox
        Left = 221
        Top = 28
        Width = 69
        Height = 21
        Style = csDropDownList
        ItemIndex = 1
        TabOrder = 1
        Text = 'ON'
        Items.Strings = (
          'OFF'
          'ON')
      end
      object OutputOpt: TComboBox
        Left = 291
        Top = 28
        Width = 69
        Height = 21
        Style = csDropDownList
        Enabled = False
        ItemIndex = 0
        TabOrder = 2
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'ON')
      end
      object TimeDecimal: TEdit
        Left = 329
        Top = 50
        Width = 31
        Height = 21
        TabOrder = 4
        Text = '3'
      end
      object DebugStatus: TComboBox
        Left = 221
        Top = 208
        Width = 69
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 10
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'States'
          'Residuals')
      end
      object NmeaIntv1: TEdit
        Left = 221
        Top = 184
        Width = 69
        Height = 21
        TabOrder = 12
        Text = '0'
      end
      object NmeaIntv2: TEdit
        Left = 291
        Top = 184
        Width = 69
        Height = 21
        TabOrder = 13
        Text = '1'
      end
      object DebugTrace: TComboBox
        Left = 291
        Top = 208
        Width = 69
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
      object SolStatic: TComboBox
        Left = 221
        Top = 160
        Width = 139
        Height = 21
        Style = csDropDownList
        Enabled = False
        ItemIndex = 0
        TabOrder = 14
        Text = 'All'
        Items.Strings = (
          'All'
          'Single')
      end
    end
    object TabSheet4: TTabSheet
      Caption = 'S&tatistics'
      ImageIndex = 3
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object Label29: TLabel
        Left = 34
        Top = 213
        Width = 132
        Height = 13
        Caption = 'Satellite Clock Stability (s/s)'
      end
      object GroupBox3: TGroupBox
        Left = 2
        Top = 0
        Width = 369
        Height = 105
        Caption = 'Measurement Errors (1-sigma)'
        TabOrder = 0
        object Label6: TLabel
          Left = 35
          Top = 16
          Width = 179
          Height = 13
          Caption = 'Code/Carrier-Phase Error Ratio L1/L2'
        end
        object Label7: TLabel
          Left = 35
          Top = 38
          Width = 160
          Height = 13
          Caption = 'Carrier-Phase Error a+b/sinEl (m)'
        end
        object Label16: TLabel
          Left = 34
          Top = 60
          Width = 184
          Height = 13
          Caption = 'Carrier-Phase Error/Baseline (m/10km)'
        end
        object Label10: TLabel
          Left = 34
          Top = 82
          Width = 114
          Height = 13
          Caption = 'Doppler Frequency (Hz)'
        end
        object MeasErrR1: TEdit
          Left = 228
          Top = 14
          Width = 55
          Height = 21
          TabOrder = 0
          Text = '100.0'
        end
        object MeasErr2: TEdit
          Left = 228
          Top = 36
          Width = 55
          Height = 21
          TabOrder = 2
          Text = '0.003'
        end
        object MeasErr3: TEdit
          Left = 284
          Top = 36
          Width = 57
          Height = 21
          TabOrder = 3
          Text = '0.003'
        end
        object MeasErr4: TEdit
          Left = 228
          Top = 58
          Width = 113
          Height = 21
          TabOrder = 4
          Text = '0.000'
        end
        object MeasErr5: TEdit
          Left = 228
          Top = 80
          Width = 113
          Height = 21
          TabOrder = 5
          Text = '1.000'
        end
        object MeasErrR2: TEdit
          Left = 284
          Top = 14
          Width = 57
          Height = 21
          TabOrder = 1
          Text = '100.0'
        end
      end
      object GroupBox4: TGroupBox
        Left = 2
        Top = 104
        Width = 369
        Height = 105
        Caption = 'Process Noises (1-sigma/sqrt(s))'
        TabOrder = 1
        object Label26: TLabel
          Left = 32
          Top = 38
          Width = 123
          Height = 13
          Caption = 'Carrier-Phase Bias (cycle)'
        end
        object Label27: TLabel
          Left = 32
          Top = 60
          Width = 172
          Height = 13
          Caption = 'Vertical Ionospheric Delay (m/10km)'
        end
        object Label28: TLabel
          Left = 32
          Top = 84
          Width = 144
          Height = 13
          Caption = 'Zenith Tropospheric Delay (m)'
        end
        object Label33: TLabel
          Left = 32
          Top = 16
          Width = 170
          Height = 13
          Caption = 'Receiver Accel Horiz/Vertical (m/s2)'
        end
        object PrNoise1: TEdit
          Left = 228
          Top = 36
          Width = 113
          Height = 21
          TabOrder = 2
          Text = '1.0E-04'
        end
        object PrNoise2: TEdit
          Left = 228
          Top = 58
          Width = 113
          Height = 21
          TabOrder = 3
          Text = '1.0E-03'
        end
        object PrNoise3: TEdit
          Left = 228
          Top = 80
          Width = 113
          Height = 21
          TabOrder = 4
          Text = '1.0E-04'
        end
        object PrNoise4: TEdit
          Left = 228
          Top = 14
          Width = 55
          Height = 21
          TabOrder = 0
          Text = '1.0E-04'
        end
        object PrNoise5: TEdit
          Left = 284
          Top = 14
          Width = 57
          Height = 21
          TabOrder = 1
          Text = '1.0E-04'
        end
      end
      object SatClkStab: TEdit
        Left = 230
        Top = 210
        Width = 113
        Height = 21
        TabOrder = 2
        Text = '5.0E-12'
      end
    end
    object TabSheet5: TTabSheet
      Caption = '&Positions'
      ImageIndex = 4
      object Label4: TLabel
        Left = 12
        Top = 12
        Width = 3
        Height = 13
      end
      object Label30: TLabel
        Left = 6
        Top = 195
        Width = 93
        Height = 13
        Caption = 'Station Position File'
      end
      object BtnStaPosView: TSpeedButton
        Left = 336
        Top = 209
        Width = 17
        Height = 18
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
        OnClick = BtnStaPosViewClick
      end
      object GroupRefAnt: TGroupBox
        Left = 2
        Top = 96
        Width = 369
        Height = 99
        Caption = 'Base Station'
        TabOrder = 1
        object LabelRefAntD: TLabel
          Left = 210
          Top = 58
          Width = 76
          Height = 13
          Caption = 'Delta-E/N/U (m)'
        end
        object RefAntE: TEdit
          Left = 208
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 7
          Text = '0'
        end
        object RefAntN: TEdit
          Left = 260
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 8
          Text = '0'
        end
        object RefAntU: TEdit
          Left = 312
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 9
          Text = '0'
        end
        object RefPos1: TEdit
          Left = 6
          Top = 36
          Width = 119
          Height = 21
          TabOrder = 1
          Text = '0'
        end
        object RefPos2: TEdit
          Left = 124
          Top = 36
          Width = 119
          Height = 21
          TabOrder = 2
          Text = '0'
        end
        object RefPos3: TEdit
          Left = 244
          Top = 36
          Width = 119
          Height = 21
          TabOrder = 3
          Text = '0'
        end
        object BtnRefPos: TButton
          Left = 345
          Top = 16
          Width = 17
          Height = 17
          Caption = '...'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -9
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 4
          OnClick = BtnRefPosClick
        end
        object RefAntPcv: TCheckBox
          Left = 6
          Top = 58
          Width = 165
          Height = 17
          Caption = 'Antenna Type (*: Auto)'
          TabOrder = 5
          OnClick = RovAntPcvClick
        end
        object RefAnt: TComboBox
          Left = 6
          Top = 74
          Width = 203
          Height = 21
          DropDownCount = 16
          TabOrder = 6
        end
        object RefPosTypeP: TComboBox
          Left = 6
          Top = 14
          Width = 137
          Height = 21
          Style = csDropDownList
          TabOrder = 0
          OnChange = RefPosTypePChange
          Items.Strings = (
            'Lat/Lon/Height (deg/m)'
            'Lat/Lon/Height (dms/m)'
            'X/Y/Z-ECEF (m)'
            'RTCM Antenna Position')
        end
      end
      object StaPosFile: TEdit
        Left = 2
        Top = 209
        Width = 333
        Height = 21
        TabOrder = 2
      end
      object BtnStaPosFile: TButton
        Left = 353
        Top = 209
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
        OnClick = BtnStaPosFileClick
      end
      object GroupRovAnt: TGroupBox
        Left = 2
        Top = -2
        Width = 369
        Height = 99
        Caption = 'Rover'
        TabOrder = 0
        object LabelRovAntD: TLabel
          Left = 210
          Top = 58
          Width = 76
          Height = 13
          Caption = 'Delta-E/N/U (m)'
        end
        object RovAntE: TEdit
          Left = 208
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 7
          Text = '0'
        end
        object RovAntN: TEdit
          Left = 260
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 8
          Text = '0'
        end
        object RovAntU: TEdit
          Left = 312
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 9
          Text = '0'
        end
        object RovPos1: TEdit
          Left = 6
          Top = 36
          Width = 119
          Height = 21
          TabOrder = 1
          Text = '0'
        end
        object RovPos2: TEdit
          Left = 124
          Top = 36
          Width = 119
          Height = 21
          TabOrder = 2
          Text = '0'
        end
        object RovPos3: TEdit
          Left = 244
          Top = 36
          Width = 119
          Height = 21
          TabOrder = 3
          Text = '0'
        end
        object BtnRovPos: TButton
          Left = 345
          Top = 16
          Width = 17
          Height = 17
          Caption = '...'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -9
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 4
          OnClick = BtnRovPosClick
        end
        object RovAntPcv: TCheckBox
          Left = 6
          Top = 58
          Width = 153
          Height = 17
          Caption = 'Antenna Type (*: Auto)'
          TabOrder = 5
          OnClick = RovAntPcvClick
        end
        object RovAnt: TComboBox
          Left = 6
          Top = 74
          Width = 203
          Height = 21
          DropDownCount = 16
          TabOrder = 6
        end
        object RovPosTypeP: TComboBox
          Left = 6
          Top = 14
          Width = 137
          Height = 21
          Style = csDropDownList
          Enabled = False
          ItemIndex = 0
          TabOrder = 0
          Text = 'Lat/Lon/Height (deg/m)'
          OnChange = RovPosTypePChange
          Items.Strings = (
            'Lat/Lon/Height (deg/m)'
            'Lat/Lon/Height (dms/m)'
            'X/Y/Z-ECEF (m)')
        end
      end
    end
    object TabSheet7: TTabSheet
      Caption = '&Files'
      ImageIndex = 6
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object Label1: TLabel
        Left = 6
        Top = 93
        Width = 65
        Height = 13
        Caption = 'DCB Data File'
      end
      object BtnAntPcvView: TSpeedButton
        Left = 355
        Top = 0
        Width = 17
        Height = 17
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
        OnClick = BtnAntPcvViewClick
      end
      object Label38: TLabel
        Left = 6
        Top = 2
        Width = 250
        Height = 13
        Caption = 'Satellite/Receiver Antenna PCV File ANTEX/NGS PCV'
      end
      object BtnSatPcvView: TSpeedButton
        Left = 336
        Top = 0
        Width = 17
        Height = 17
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
        OnClick = BtnSatPcvViewClick
      end
      object Label48: TLabel
        Left = 6
        Top = 58
        Width = 72
        Height = 13
        Caption = 'Geoid Data File'
      end
      object Label31: TLabel
        Left = 6
        Top = 197
        Width = 121
        Height = 13
        Caption = 'FTP/HTTP Local Directory'
      end
      object BtnDCBView: TSpeedButton
        Left = 355
        Top = 93
        Width = 17
        Height = 17
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
        OnClick = BtnStaPosViewClick
      end
      object Label34: TLabel
        Left = 6
        Top = 163
        Width = 130
        Height = 13
        Caption = 'Ocean Loading BLQ Format'
      end
      object BtnOLFileView: TSpeedButton
        Left = 355
        Top = 162
        Width = 17
        Height = 17
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
      end
      object Label23: TLabel
        Left = 6
        Top = 128
        Width = 65
        Height = 13
        Caption = 'EOP Data File'
      end
      object BtnEOPView: TSpeedButton
        Left = 355
        Top = 127
        Width = 17
        Height = 17
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
        OnClick = BtnEOPViewClick
      end
      object AntPcvFile: TEdit
        Left = 2
        Top = 37
        Width = 353
        Height = 21
        TabOrder = 2
      end
      object BtnAntPcvFile: TButton
        Left = 355
        Top = 39
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
        OnClick = BtnAntPcvFileClick
      end
      object BtnDCBFile: TButton
        Left = 355
        Top = 109
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 7
        OnClick = BtnDCBFileClick
      end
      object DCBFile: TEdit
        Left = 2
        Top = 107
        Width = 353
        Height = 21
        TabOrder = 6
      end
      object SatPcvFile: TEdit
        Left = 2
        Top = 16
        Width = 353
        Height = 21
        TabOrder = 0
      end
      object BtnSatPcvFile: TButton
        Left = 355
        Top = 18
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnSatPcvFileClick
      end
      object GeoidDataFile: TEdit
        Left = 2
        Top = 72
        Width = 353
        Height = 21
        TabOrder = 4
      end
      object BtnGeoidDataFile: TButton
        Left = 355
        Top = 74
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnGeoidDataFileClick
      end
      object LocalDir: TEdit
        Left = 2
        Top = 211
        Width = 353
        Height = 21
        TabOrder = 12
      end
      object BtnLocalDir: TButton
        Left = 355
        Top = 212
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 13
        OnClick = BtnLocalDirClick
      end
      object OLFile: TEdit
        Left = 2
        Top = 177
        Width = 353
        Height = 21
        TabOrder = 10
      end
      object BtnOLFile: TButton
        Left = 355
        Top = 179
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 11
      end
      object EOPFile: TEdit
        Left = 2
        Top = 142
        Width = 353
        Height = 21
        TabOrder = 8
      end
      object BtnEOPFile: TButton
        Left = 355
        Top = 143
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 9
        OnClick = BtnEOPFileClick
      end
    end
    object TabSheet8: TTabSheet
      Caption = '&Misc'
      ImageIndex = 6
      object Label19: TLabel
        Left = 32
        Top = 7
        Width = 190
        Height = 13
        Caption = 'Process Cycle (ms) / Buffer Size (bytes)'
      end
      object Label40: TLabel
        Left = 147
        Top = 167
        Width = 39
        Height = 13
        Caption = 'Sol Font'
      end
      object FontLabel: TLabel
        Left = 280
        Top = 163
        Width = 67
        Height = 18
        Alignment = taRightJustify
        Caption = 'Font Label'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Label41: TLabel
        Left = 32
        Top = 74
        Width = 162
        Height = 13
        Caption = 'Solution Buffer/ Log Size (epochs)'
      end
      object Label42: TLabel
        Left = 32
        Top = 96
        Width = 142
        Height = 13
        Caption = 'Navigation Message Selection'
      end
      object Label5: TLabel
        Left = 32
        Top = 118
        Width = 193
        Height = 13
        Caption = 'SBAS Sat Selection (0: all) / Monitor Port'
      end
      object Label46: TLabel
        Left = 32
        Top = 51
        Width = 187
        Height = 13
        Caption = 'NMEA Cycle (ms) / File Swap Margin (s)'
      end
      object Label44: TLabel
        Left = 32
        Top = 29
        Width = 164
        Height = 13
        Caption = 'Timeout / Reconnect Interval (ms)'
      end
      object Label45: TLabel
        Left = 32
        Top = 142
        Width = 96
        Height = 13
        Caption = 'HTTP / NTRIP Proxy'
      end
      object Label15: TLabel
        Left = 8
        Top = 191
        Width = 43
        Height = 13
        Caption = 'TLE Data'
      end
      object Label39: TLabel
        Left = 8
        Top = 212
        Width = 32
        Height = 13
        Caption = 'Sat No'
      end
      object Label43: TLabel
        Left = 32
        Top = 167
        Width = 33
        Height = 13
        Caption = 'Layout'
      end
      object SvrCycleE: TEdit
        Left = 231
        Top = 4
        Width = 61
        Height = 21
        TabOrder = 0
        Text = '10'
      end
      object SvrBuffSizeE: TEdit
        Left = 293
        Top = 4
        Width = 61
        Height = 21
        TabOrder = 5
        Text = '32768'
      end
      object BtnFont: TButton
        Left = 354
        Top = 165
        Width = 17
        Height = 19
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 13
        OnClick = BtnFontClick
      end
      object SolBuffSizeE: TEdit
        Left = 231
        Top = 71
        Width = 61
        Height = 21
        TabOrder = 6
        Text = '1000'
      end
      object NavSelectS: TComboBox
        Left = 231
        Top = 93
        Width = 123
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 8
        Text = 'All'
        Items.Strings = (
          'All'
          '(1) Rover'
          '(2) Base Station'
          '(3) Correction')
      end
      object SbasSatE: TEdit
        Left = 231
        Top = 115
        Width = 61
        Height = 21
        TabOrder = 9
        Text = '0'
      end
      object SavedSolE: TEdit
        Left = 293
        Top = 71
        Width = 61
        Height = 21
        TabOrder = 7
        Text = '100'
      end
      object NmeaCycleE: TEdit
        Left = 231
        Top = 48
        Width = 61
        Height = 21
        TabOrder = 3
        Text = '5000'
      end
      object TimeoutTimeE: TEdit
        Left = 231
        Top = 26
        Width = 61
        Height = 21
        TabOrder = 1
        Text = '10000'
      end
      object ReconTimeE: TEdit
        Left = 293
        Top = 26
        Width = 61
        Height = 21
        TabOrder = 2
        Text = '10000'
      end
      object MoniPortE: TEdit
        Left = 293
        Top = 115
        Width = 61
        Height = 21
        TabOrder = 10
        Text = '0'
      end
      object FileSwapMarginE: TEdit
        Left = 293
        Top = 48
        Width = 61
        Height = 21
        TabOrder = 4
        Text = '30'
      end
      object ProxyAddrE: TEdit
        Left = 140
        Top = 138
        Width = 214
        Height = 21
        TabOrder = 11
      end
      object TLEFile: TEdit
        Left = 55
        Top = 187
        Width = 298
        Height = 21
        TabOrder = 14
      end
      object BtnTLEFile: TButton
        Left = 354
        Top = 188
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 15
        OnClick = BtnTLEFileClick
      end
      object TLESatFile: TEdit
        Left = 55
        Top = 209
        Width = 298
        Height = 21
        TabOrder = 16
      end
      object BtnTLESatFile: TButton
        Left = 354
        Top = 210
        Width = 17
        Height = 18
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 17
        OnClick = BtnTLESatFileClick
      end
      object PanelStackE: TComboBox
        Left = 70
        Top = 163
        Width = 71
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 12
        Text = 'Horizontal'
        Items.Strings = (
          'Horizontal'
          'Vertical')
      end
    end
  end
  object OpenDialog: TOpenDialog
    Filter = 
      'All (*.*)|*.*|PCV File (*.pcv,*.atx)|*.pcv;*.atx|Position File (' +
      '*.pos)|*.pos|Options File (*.conf)|*.conf'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Load File'
    Left = 4
    Top = 258
  end
  object SaveDialog: TSaveDialog
    Filter = 'All (*.*)|*.*|Options File (*.conf)|*.conf'
    Title = 'Save File'
    Left = 34
    Top = 258
  end
  object FontDialog: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Left = 60
    Top = 258
  end
end
